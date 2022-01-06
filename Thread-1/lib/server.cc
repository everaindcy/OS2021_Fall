
#include <vector>
#include <thread>
#include <iostream>
#include <mutex>

#include "server.h"
#include "model.h"
#include "utils.h"

#define TIMER(s) // proj1::AutoTimer timer(s)

namespace proj1 {

void Server::write_to_stdout() {
    users.write_to_stdout();
    items.write_to_stdout();
}

void Server::do_instruction(Instruction inst) {
    switch(inst.order) {
        case INIT_EMB: {
            if (!useLock) {
                do_init(inst);
            } else if (!initParall) {
                do_init_safe(inst);
            } else {
                do_init_parall(inst);
            }
            break;
        }
        case UPDATE_EMB: {
            if (!useLock) {
                do_update(inst);
            } else if (!useEpoch) {
                do_update_safe(inst);
            } else {
                if (!changeInplace) {
                    do_update_epoch(inst);
                } else {
                    do_update_inplace(inst);
                }
            }
            break;
        }
        case RECOMMEND: {
            if (!useLock) {
                Embedding* emb = do_recommend(inst);
                emb->write_to_stdout();
            } else if (!useEpoch) {
                Embedding* emb = do_recommend_safe(inst);
                emb->write_to_stdout();
            } else {
                if (!changeInplace) {
                    Embedding* emb = do_recommend_epoch(inst);
                    emb->write_to_stdout();
                } else {
                    Embedding* emb = do_recommend_non_inplace(inst);
                    emb->write_to_stdout();
                }
            }
        }
    }
}

void Server::do_init(Instruction inst) {
    TIMER("do_init");

    int length = users.get_emb_length();
    Embedding* new_user = new Embedding(length);
    int user_idx = users.append(new_user);

    for (int item_index: inst.payloads) {
        Embedding* item_emb = items.get_embedding(item_index);

        EmbeddingGradient* gradient = cold_start(new_user, item_emb);
        users.update_embedding(user_idx, gradient, 0.01);
        delete gradient;
    }
}

void Server::do_init_safe(Instruction inst) {
    TIMER("do_init_safe");

    int length = users.get_emb_length();
    Embedding* new_user = new Embedding(length);
    int user_idx = users.append(new_user);

    for (int item_index: inst.payloads) {
        Embedding* item_emb = items.get_embedding(item_index);

        Embedding* user = new Embedding(new_user);
        Embedding* item = new Embedding(item_emb);
        EmbeddingGradient* gradient = cold_start(user, item);
        delete user, item;

        users.update_embedding(user_idx, gradient, 0.01);
        delete gradient;
    }
}

void Server::do_init_parall(Instruction inst) {
    TIMER("do_init_parall");

    int length = users.get_emb_length();
    Embedding* new_user = new Embedding(length);
    int user_idx = users.append(new_user);

    std::vector<std::thread*> threads;
    for (int item_index: inst.payloads) {
        std::thread* t = new std::thread(
            [&, item_index]() {
                Embedding* item_emb = items.get_embedding(item_index);

                Embedding* user = new Embedding(new_user);
                Embedding* item = new Embedding(item_emb);
                EmbeddingGradient* gradient = cold_start(user, item);
                delete user, item;

                users.update_embedding(user_idx, gradient, 0.01);
                delete gradient;
            }
        );
        threads.push_back(t);
    }
    for (std::thread* t: threads) {
        t->join();
        delete t;
    }
}

void Server::do_update(Instruction inst) {
    TIMER("do_update");

    std::cout << "do_update" << std::endl;
    int user_idx = inst.payloads[0];
    int item_idx = inst.payloads[1];
    int label = inst.payloads[2];

    Embedding* user_emb = users.get_embedding(user_idx);
    Embedding* item_emb = items.get_embedding(item_idx);

    EmbeddingGradient* gradient = calc_gradient(user_emb, item_emb, label);
    users.update_embedding(user_idx, gradient, 0.01);
    delete gradient;

    gradient = calc_gradient(item_emb, user_emb, label);
    items.update_embedding(item_idx, gradient, 0.001);
    delete gradient;
}

void Server::do_update_safe(Instruction inst) {
    TIMER("do_update_safe");

    int user_idx = inst.payloads[0];
    int item_idx = inst.payloads[1];
    int label = inst.payloads[2];

    Embedding* user_emb = users.get_embedding(user_idx);
    Embedding* item_emb = items.get_embedding(item_idx);

    auto user = new Embedding(user_emb);
    auto item = new Embedding(item_emb);
    EmbeddingGradient* gradient = calc_gradient(user, item, label);
    delete user, item;

    users.update_embedding(user_idx, gradient, 0.01);
    delete gradient;

    user = new Embedding(user_emb);
    item = new Embedding(item_emb);
    gradient = calc_gradient(item, user, label);
    delete user, item;

    items.update_embedding(item_idx, gradient, 0.001);
    delete gradient;
}

void Server::do_update_epoch(Instruction inst) {
    int iter_idx = inst.payloads[3];
{
    std::unique_lock<std::mutex> lock(this->mux);
    while (!(this->epoch >= iter_idx || (this->epoch == iter_idx - 1 && this->num_threads == 0))) {
        this->cv.wait(lock);
    }
    if (this->epoch == iter_idx - 1 && this->num_threads == 0) {
        this->epoch = iter_idx;
    }
    this->num_threads++;
}

    TIMER("do_update_epoch");

    int user_idx = inst.payloads[0];
    int item_idx = inst.payloads[1];
    int label = inst.payloads[2];

    Embedding* user_emb = users.get_embedding(user_idx);
    Embedding* item_emb = items.get_embedding(item_idx);

    auto user = new Embedding(user_emb);
    auto item = new Embedding(item_emb);
    EmbeddingGradient* gradient = calc_gradient(user, item, label);
    delete user, item;

    users.update_embedding(user_idx, gradient, 0.01);
    delete gradient;

    user = new Embedding(user_emb);
    item = new Embedding(item_emb);
    gradient = calc_gradient(item, user, label);
    delete user, item;

    items.update_embedding(item_idx, gradient, 0.001);
    delete gradient;

{
    std::unique_lock<std::mutex> lock(this->mux);
    this->num_threads--;
    this->cv.notify_all();
}
}

void Server::do_update_inplace(Instruction inst) {
    int iter_idx = inst.payloads[3];
{
    std::unique_lock<std::mutex> lock(this->mux);
    while (!(this->epoch >= iter_idx || (this->epoch == iter_idx - 1 && this->num_threads == 0))) {
        this->cv.wait(lock);
    }
    if (this->epoch == iter_idx - 1 && this->num_threads == 0) {
        this->epoch = iter_idx;
    }
    this->num_threads++;
}

    TIMER("do_update_inplace");

    int user_idx = inst.payloads[0];
    int item_idx = inst.payloads[1];
    int label = inst.payloads[2];

    Embedding* user_emb = users.get_embedding(user_idx);
    Embedding* item_emb = items.get_embedding(item_idx);

{
    std::lock_guard<std::mutex> userlock(user_emb->wmux);
    std::lock_guard<std::mutex> itemlock(item_emb->wmux);
    EmbeddingGradient* gradient = calc_gradient(user_emb, item_emb, label);
    users.update_embedding(user_idx, gradient, 0.01);
    delete gradient;

    gradient = calc_gradient(item_emb, user_emb, label);
    items.update_embedding(item_idx, gradient, 0.001);
    delete gradient;
}

{
    std::unique_lock<std::mutex> lock(this->mux);
    this->num_threads--;
    this->cv.notify_all();
}
}

Embedding* Server::do_recommend(Instruction inst) {
    TIMER("do_recommend");

    int user_idx = inst.payloads[0];
    Embedding* user = users.get_embedding(user_idx);

    std::vector<Embedding*> item_pool;
    for (unsigned int i = 2; i < inst.payloads.size(); i++) {
        int item_idx = inst.payloads[i];
        item_pool.push_back(items.get_embedding(item_idx));
    }

    Embedding* recommendation = recommend(user, item_pool);
    return recommendation;
}

Embedding* Server::do_recommend_safe(Instruction inst) {
    TIMER("do_recommend_safe");

    int user_idx = inst.payloads[0];
    Embedding* user = users.get_embedding(user_idx);
    std::lock_guard<std::mutex> lock(user->mux);

    std::vector<Embedding*> item_pool;
    for (unsigned int i = 2; i < inst.payloads.size(); i++) {
        int item_idx = inst.payloads[i];
        Embedding* item = items.get_embedding(item_idx);
        std::lock_guard<std::mutex> lock(item->mux);
        item_pool.push_back(item);
    }

    Embedding* recommendation = recommend(user, item_pool);
    return recommendation;
}

Embedding* Server::do_recommend_epoch(Instruction inst) {
    int iter_idx = inst.payloads[1];
{
    std::unique_lock<std::mutex> lock(this->mux);
    while (!(this->epoch > iter_idx | (this->epoch == iter_idx && this->num_threads == 0))) {
        this->cv.wait(lock);
    }
}

    TIMER("do_recommend_epoch");

    int user_idx = inst.payloads[0];
    Embedding* user = users.get_embedding(user_idx);
    std::lock_guard<std::mutex> lock(user->mux);

    std::vector<Embedding*> item_pool;
    for (unsigned int i = 2; i < inst.payloads.size(); i++) {
        int item_idx = inst.payloads[i];
        Embedding* item = items.get_embedding(item_idx);
        std::lock_guard<std::mutex> lock(item->mux);
        item_pool.push_back(item);
    }

    Embedding* recommendation = recommend(user, item_pool);
    return recommendation;
}

Embedding* Server::do_recommend_non_inplace(Instruction inst) {
    int iter_idx = inst.payloads[1];
{
    std::unique_lock<std::mutex> lock(this->mux);
    while (!(this->epoch > iter_idx | (this->epoch == iter_idx && this->num_threads == 0))) {
        this->cv.wait(lock);
    }
}

    TIMER("do_recommend_non_inplace");

    int user_idx = inst.payloads[0];
    Embedding* user_emb = users.get_embedding(user_idx);
    Embedding* user = new Embedding(user_emb);

    std::vector<Embedding*> item_pool;
    for (unsigned int i = 2; i < inst.payloads.size(); i++) {
        int item_idx = inst.payloads[i];
        Embedding* item_emb = items.get_embedding(item_idx);
        Embedding* item = new Embedding(item_emb);
        item_pool.push_back(item);
    }

    Embedding* recommendation = recommend(user, item_pool);

    delete user;
    for(auto item : item_pool) {
        delete item;
    }

    return recommendation;
}

bool Server::operator==(Server& svr) {
    return this->users == svr.users && this->items == svr.items;
}

} // namespace proj1

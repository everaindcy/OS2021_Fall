
#include <vector>
#include <thread>
#include <iostream>
#include "server.h"
#include "model.h"

#include "utils.h"
#define TIMER(s) proj1::AutoTimer timer(s)

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

    users.lock();
    int user_idx = users.append(new_user);
    users.unlock();

    for (int item_index: inst.payloads) {
        items.lock();
        Embedding* item_emb = items.get_embedding(item_index);
        items.unlock();

        Embedding* user = new Embedding(new_user);
        Embedding* item = new Embedding(item_emb);
        EmbeddingGradient* gradient = cold_start(user, item);
        delete user, item;

        new_user->lock();
        users.update_embedding(user_idx, gradient, 0.01);
        new_user->unlock();

        delete gradient;
    }
}

void Server::do_init_parall(Instruction inst) {
    TIMER("do_init_parall");

    int length = users.get_emb_length();
    Embedding* new_user = new Embedding(length);

    users.lock();
    int user_idx = users.append(new_user);
    users.unlock();

    std::vector<std::thread*> threads;
    for (int item_index: inst.payloads) {
        std::thread* t = new std::thread(
            [&]() {
                items.lock();
                Embedding* item_emb = items.get_embedding(item_index);
                items.unlock();

                Embedding* user = new Embedding(new_user);
                Embedding* item = new Embedding(item_emb);
                EmbeddingGradient* gradient = cold_start(user, item);
                delete user, item;

                new_user->lock();
                users.update_embedding(user_idx, gradient, 0.01);
                new_user->unlock();

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

    users.lock();
    Embedding* user_emb = users.get_embedding(user_idx);
    users.unlock();

    items.lock();
    Embedding* item_emb = items.get_embedding(item_idx);
    items.unlock();

    auto user = new Embedding(user_emb);
    auto item = new Embedding(item_emb);
    EmbeddingGradient* gradient = calc_gradient(user, item, label);
    delete user, item;

    user_emb->lock();
    users.update_embedding(user_idx, gradient, 0.01);
    user_emb->unlock();
    delete gradient;

    user = new Embedding(user_emb);
    item = new Embedding(item_emb);
    gradient = calc_gradient(item, user, label);
    delete user, item;

    item_emb->lock();
    items.update_embedding(item_idx, gradient, 0.001);
    item_emb->unlock();
    delete gradient;
}

Embedding* Server::do_recommend(Instruction inst) {
    TIMER("do_recommend");

    int user_idx = inst.payloads[0];
    Embedding* user = users.get_embedding(user_idx);

    int iter_idx = inst.payloads[1];

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

    int iter_idx = inst.payloads[1];

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

} // namespace proj1

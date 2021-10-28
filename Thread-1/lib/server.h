#ifndef THREAD_LIB_SERVER_H_
#define THREAD_LIB_SERVER_H_

#include <mutex>
#include <condition_variable>
#include "instruction.h"
#include "embedding.h"

namespace proj1 {

class Server {
public:
    Server(std::string user_filename, std::string item_filename) : users(user_filename), items(item_filename) {}
    Server(std::string user_filename, std::string item_filename, bool initParall, bool useLock, bool useEpoch)
        : users(user_filename), items(item_filename), initParall(initParall), useLock(useLock), useEpoch(useEpoch) {}
    void do_instruction(Instruction);
    void write_to_stdout();

    void do_init(Instruction);
    void do_init_safe(Instruction);
    void do_init_parall(Instruction);

    void do_update(Instruction);
    void do_update_safe(Instruction);
    void do_update_epoch(Instruction);

    Embedding* do_recommend(Instruction);
    Embedding* do_recommend_safe(Instruction);
    Embedding* do_recommend_epoch(Instruction);

    bool operator==(Server&);

private:
    bool initParall = false;
    bool useLock = false;
    bool useEpoch = false;

    int epoch = -1;
    int num_threads = 0; // number of update threads running in this epoch
    std::mutex mux;
    std::condition_variable cv;

    EmbeddingHolder users;
    EmbeddingHolder items;
};

} // namespace proj1
#endif // THREAD_LIB_SERVER_H_

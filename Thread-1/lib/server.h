#ifndef THREAD_LIB_SERVER_H_
#define THREAD_LIB_SERVER_H_

#include <mutex>
#include <condition_variable>
#include "instruction.h"
#include "embedding.h"

namespace proj1 {

class server {
public:
    server(std::string user_filename, std::string item_filename) : users(user_filename), items(item_filename) {}
    server(std::string user_filename, std::string item_filename,
            bool initParallilized, bool useLock, bool useEpoch) : users(user_filename), items(item_filename) {
        this->initParallelized = initParallelized;
        this->useLock = useLock;
        this->useEpoch = useEpoch;
    }
    void do_instruction(Instruction);

private:
    bool initParallelized = false;
    bool useLock = false;
    bool useEpoch = false;

    int epoch = -1;
    std::mutex mux;
    std::condition_variable cv;

    EmbeddingHolder users;
    EmbeddingHolder items;
};

} // namespace proj1
#endif // THREAD_LIB_SERVER_H_

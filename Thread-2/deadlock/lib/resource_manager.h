#ifndef DEADLOCK_LIB_RESOURCE_MANAGER_H_
#define DEADLOCK_LIB_RESOURCE_MANAGER_H_

#include <map>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "thread_manager.h"

namespace proj2 {

enum RESOURCE {
    GPU = 0,
    MEMORY,
    DISK,
    NETWORK
};

class ResourceManager {
public:
    ResourceManager(ThreadManager *t, std::map<RESOURCE, int> init_count): \
        resource_amount(init_count), tmgr(t), total_thread(0) {
            std::map<RESOURCE, int>::iterator iter;
            for (iter = resource_amount.begin(); iter != resource_amount.end(); iter++)
                printf("Resource %d: %d\n", iter->first, iter->second);
    }
    void budget_claim(std::map<RESOURCE, int> budget);
    int request(RESOURCE, int amount);
    void release(RESOURCE, int amount);
private:
    bool check_security(RESOURCE, int amount);

    std::vector<std::thread::id> threadList;
    std::map<std::thread::id, bool> unfinished;
    std::map<RESOURCE, int> resource_amount;
    std::map<RESOURCE, std::mutex> resource_mutex;
    std::map<RESOURCE, std::condition_variable> resource_cv;
    ThreadManager *tmgr;

    std::mutex data_lock;
    std::map<std::thread::id, int*> max;
    // std::map<RESOURCE, int> available;
    std::map<std::thread::id, int*> allocation;

    std::map<std::thread::id, int> debug_thread_id;

    int total_thread;
};

}  // namespce: proj2

#endif
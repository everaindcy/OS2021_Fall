#include <stdio.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include "resource_manager.h"

namespace proj2 {

int ResourceManager::request(RESOURCE r, int amount) {

    //改成总共只需要一个锁！


    std::thread::id this_id = std::this_thread::get_id();
    printf("%7d request %d : %d\n", debug_thread_id[this_id], r, amount);

    if (amount <= 0)  {
        printf("%7d request with WRONG parameter.\n",debug_thread_id[this_id]);
        return 1;
    }
    
    // std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    std::unique_lock<std::mutex> lk(data_lock);
    while (true) {
        if (this->resource_cv[r].wait_for( lk,  std::chrono::milliseconds(100), 
            // [this, r, amount] { return this->resource_amount[r] >= amount; }
            [this, r, amount]{return this->check_security(r, amount);}
        )) {
            break;
        } else {
            auto this_id = std::this_thread::get_id();
            /* HINT: If you choose to detect the deadlock and recover, implement your code here to kill and restart threads. Note that you should release this thread's resources properly. */
            if (tmgr->is_killed(this_id)) {
                return -1;
            }
        }
    }
    printf("%7d get %d : %d\n", debug_thread_id[this_id], r, amount);
    this->resource_amount[r] -= amount;
    this->allocation[this_id][r] += amount;
    // this->available[r] -= amount;
    // this->resource_mutex[r].unlock();
    this->data_lock.unlock();
    return 0;
}


void ResourceManager::release(RESOURCE r, int amount) {
    std::thread::id this_id = std::this_thread::get_id();
    printf("%7d release %d : %d\n", debug_thread_id[this_id], r, amount);
    if (amount <= 0)  return;
    // std::unique_lock<std::mutex> lk(this->resource_mutex[r]);
    std::unique_lock<std::mutex> lk(data_lock);
    this->resource_amount[r] += amount;
    this->allocation[this_id][r] -= amount;
    this->resource_cv[r].notify_all();
}

bool ResourceManager::check_security(RESOURCE r, int amount) {

    std::map<RESOURCE, int> available;
    std::map<std::thread::id, int*> cur_allocation;
    available = resource_amount;
    cur_allocation = allocation;

    std::thread::id this_id = std::this_thread::get_id();
    // printf("%7d check %d : %d (now available %d %d %d %d)\n", debug_thread_id[this_id], r, amount, available[GPU], available[MEMORY], available[DISK], available[NETWORK]);
    // for (int n = 0; n < threadList.size(); n++) {
    //     printf("%7d check (cur_alloc %d : %d %d %d %d)\n", (*(uint32_t*)&this_id), (*(uint32_t*)&threadList[n]), allocation[threadList[n]][GPU], allocation[threadList[n]][MEMORY], allocation[threadList[n]][DISK], allocation[threadList[n]][NETWORK]);
    // }

    if (available[r] < amount) {
        // printf("FAILED more than available\n");
        return false;
    }
    if (cur_allocation[this_id][r] + amount > max[this_id][r]) {
        // printf("FAILED more than max\n");
        return false;
    }
    // printf("\n");

    available[r] -= amount;
    cur_allocation[this_id][r] += amount;


    for (int i = 0; i < threadList.size(); i++)
        unfinished[threadList[i]] = true;

    bool done = false;
    while(!done){
        done = true;
        for (int n = 0; n < threadList.size(); n++) {
            if (!unfinished[threadList[n]]) continue;
            bool is_available = true;
            for (int i = 0; i < 4; i++) {
                // printf("%7d check (%d,%d) : max:%d, alloc:%d, avail:%d\n", (*(uint32_t*)&this_id), (*(uint32_t*)&threadList[n]), i, max[threadList[n]][i], allocation[threadList[n]][i], available[(proj2::RESOURCE)i]);
                if (max[threadList[n]][i] - cur_allocation[threadList[n]][i] > available[(proj2::RESOURCE)i]) {
                    is_available = false;
                    break;
                }
            }
            // printf("%7d check %d : %s\n", (*(uint32_t*)&this_id), (*(uint32_t*)&threadList[n]), is_available?"True":"False");
            if (is_available) {
                done = false;
                for (int i = 0; i < 4; i++) {
                    available[(proj2::RESOURCE)i] += cur_allocation[threadList[n]][i];
                }
                unfinished[threadList[n]] = false;
            }
        }
    }

    available[r] += amount;
    cur_allocation[this_id][r] -= amount;

    for (int n = 0; n < threadList.size(); n++) {
        if (unfinished[threadList[n]]) {
            // printf("%7d check FAILED\n", (*(uint32_t*)&this_id));
            return false;
        }
    }

    // printf("%7d check PASS\n", debug_thread_id[this_id]);
    return true;
}



void ResourceManager::budget_claim(std::map<RESOURCE, int> budget) {
    // This function is called when some workload starts.
    // The workload will eventually consume all resources it claimss
    std::thread::id this_id = std::this_thread::get_id();

    std::unique_lock<std::mutex> lk(data_lock);
    this->total_thread++;
    debug_thread_id[this_id] = total_thread;
    printf("%7d Total thread: %d\n",debug_thread_id[this_id], this->total_thread);

    this->threadList.push_back(this_id);

    this->max[this_id] = new int [4];
    this->allocation[this_id] = new int [4];

    for (int i = 0; i < 4; i++) {
        this->max[this_id][i] = 0;
        this->allocation[this_id][i] = 0;
    }

    std::map<RESOURCE, int>::iterator iter;
    for (iter = budget.begin(); iter != budget.end(); iter++) {
        this->max[this_id][iter->first] = iter->second;
        // printf("%7d claim %d : %d\n", debug_thread_id[this_id], iter->first, iter->second);
    }
}

} // namespace: proj2

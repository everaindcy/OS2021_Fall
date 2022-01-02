#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <assert.h>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <cstdlib>
#include <cstdio>

#define PageSize 1024

namespace proj4 {

class PageFrame {
public:
    PageFrame();
    int& operator[] (unsigned long);
    void WriteDisk(std::string);
    void ReadDisk(std::string);
    void Clear();
private:
    int mem[PageSize];
};

class PageInfo {
public:
    PageInfo();
    void SetInfo(int,int);
    void ClearInfo();
    int GetHolder();
    int GetVid();
    void lock() { page_lock.lock(); }
    void unlock() { page_lock.unlock(); }
    int used;

private:
    int holder; //page holder id (array_id)
    int virtual_page_id; // page virtual #
    mutable std::mutex page_lock;
    /*add your extra states here freely for implementation*/

};

class MemoryManager {
public:
    // you should not modify the public interfaces used in tests
    MemoryManager(size_t);
    int ReadPage(int array_id, int virtual_page_id, int offset);
    void WritePage(int array_id, int virtual_page_id, int offset, int value);
    void ClearPage(int array_id, int virtual_page_id);
    int Allocate(size_t);
    void Release(int array_id);
    ~MemoryManager();
private:
    std::map<int, std::map<int, int>> page_map; // // mapping from ArrayList's virtual page # to physical page #
    std::map<int, std::map<int, std::mutex*>> page_mutex;
    PageFrame** mem; // physical pages, using 'PageFrame* mem' is also acceptable 
    PageInfo** page_info; // physical page info
    unsigned int free_list;  // use bitmap implementation to identify and search for free pages
    int next_array_id;
    size_t mma_sz;
    /*add your extra states here freely for implementation*/

    void PageIn(int array_id, int virtual_page_id, int physical_page_id);
    void PageOut(int physical_page_id, int holder, int virtual_page_id);
    int PageReplace(int array_id, int virtual_page_id);

    int get_empty_page();
    int ReplacementPolicyFIFO();
    int ReplacementPolicyClock();
    std::queue<int> Q_FIFO;
    int CLOCK_HAND;

    std::mutex mma_lock;
};

}  // namespce: proj4

#endif


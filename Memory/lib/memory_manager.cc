#include "memory_manager.h"

#include "array_list.h"
#include <iostream>
#include <fstream>

namespace proj3 {
    PageFrame::PageFrame(){
    }
    int& PageFrame::operator[] (unsigned long idx){
        //each page should provide random access like an array
        return mem[idx];
    }
    void PageFrame::WriteDisk(std::string filename) {
        // write page content into disk files
        std::string path = "disk/";
        auto fin = fopen((path + filename).c_str(), "w");
        for (int i; i < PageSize; i++) {
            fprintf(fin, "%d\n", mem[i]);
        }
        fclose(fin);
    }
    void PageFrame::ReadDisk(std::string filename) {
        // read page content from disk files
        std::string path = "disk/";
        auto fin = fopen((path + filename).c_str(), "r");
        for (int i; i < PageSize; i++) {
            fscanf(fin, "%d", &mem[i]);
        }
        fclose(fin);
    }

    PageInfo::PageInfo(){
        holder = -1;
        virtual_page_id = -1;
    }
    void PageInfo::SetInfo(int cur_holder, int cur_vid){
        //modify the page states
        //you can add extra parameters if needed
        holder = cur_holder;
        virtual_page_id = cur_vid;
    }
    void PageInfo::ClearInfo(){
        //clear the page states
        //you can add extra parameters if needed
        holder = -1;
        virtual_page_id = -1;
    }

    int PageInfo::GetHolder(){ return holder; }
    int PageInfo::GetVid(){ return virtual_page_id; }
    

    MemoryManager::MemoryManager(size_t sz){
        //mma should build its memory space with given space size
        //you should not allocate larger space than 'sz' (the number of physical pages) 
        next_array_id = 0;
        mma_sz = sz;
        free_list = 0;
        for (int i = 0; i < sz; i++){
            auto *a_page = new PageFrame();
            auto *a_pageinfo = new PageInfo();
            mem[i] = a_page;
            page_info[i] = a_pageinfo;
        }
    }
    MemoryManager::~MemoryManager(){
    }
    void MemoryManager::PageOut(int physical_page_id){
        //swap out the physical page with the indx of 'physical_page_id out' into a disk file
    }
    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id){
        //swap the target page from the disk file into a physical page with the index of 'physical_page_id out'
    }
    void MemoryManager::PageReplace(int array_id, int virtual_page_id){
        //implement your page replacement policy here
    }
    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset){
        // for arrayList of 'array_id', return the target value on its virtual space
        int phy_Pageidx = page_map[array_id][virtual_page_id];
        if (phy_Pageidx == -1) {
            PageReplace(array_id, virtual_page_id);
        }
        phy_Pageidx = page_map[array_id][virtual_page_id];
        PageFrame* page = mem[phy_Pageidx];
        return (*page)[offset];
    }
    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value){
        // for arrayList of 'array_id', write 'value' into the target position on its virtual space
        int phy_Page_idx = page_map[array_id][virtual_page_id];
        if (phy_Page_idx == -1) {
            PageReplace(array_id, virtual_page_id);
        }
        phy_Page_idx = page_map[array_id][virtual_page_id];
        PageFrame* page = mem[phy_Page_idx];
        (*page)[offset] = value;
    }
    ArrayList* MemoryManager::Allocate(size_t sz){
        // when an application requires for memory, create an ArrayList and record mappings from its virtual memory space to the physical memory space
        int num_page = sz / PageSize;
        if (sz % PageSize != 0) num_page ++;
        ArrayList *a_ArrayList = new ArrayList(sz, this, next_array_id);
        std::map<int, int> a_trans_map;
        for (int i = 0; i < num_page; i++) a_trans_map[i] = -1;
        page_map[next_array_id] = a_trans_map;
        next_array_id++;
        return a_ArrayList;

    }
    void MemoryManager::Release(ArrayList* arr){
        // an application will call release() function when destroying its arrayList
        // release the virtual space of the arrayList and erase the corresponding mappings

    }
} // namespce: proj3
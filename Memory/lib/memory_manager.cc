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
        // printf("disk write : %s, mem[1] = %d\n", filename, mem[1]);
        std::string path = ".\\disk\\";
        auto fin = fopen((path + filename + ".txt").c_str(), "w");
        for (int i = 0; i < PageSize; i++) {
            fprintf(fin, "%d\n", mem[i]);
        }
        fclose(fin);
    }
    void PageFrame::ReadDisk(std::string filename) {
        // read page content from disk files
        // printf("disk read : %s\n", filename);
        std::string path = ".\\disk\\";
        auto fin = fopen((path + filename + ".txt").c_str(), "r");
        if (fin==NULL) {
            // printf("page not find. clear.\n");
            Clear();
        } else {
            for (int i = 0; i < PageSize; i++) {
                fscanf(fin, "%d", &mem[i]);
            }
            fclose(fin);
        }
    }
    void PageFrame::Clear(){
        for (int i = 0; i < PageSize; i++) mem[i] = 0;
    }

    PageInfo::PageInfo(){
        holder = -1;
        virtual_page_id = -1;
        used = 0;
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
        mem = new PageFrame*[sz];
        page_info = new PageInfo*[sz];
        for (int i = 0; i < sz; i++){
            auto *a_page = new PageFrame();
            auto *a_pageinfo = new PageInfo();
            mem[i] = a_page;
            page_info[i] = a_pageinfo;
            // M_CLOCK[i] = 0;
            CLOCK_HAND = 0;
        }
    }
    MemoryManager::~MemoryManager(){
        for (int i = 0; i < mma_sz; i++){
            delete mem[i];
            delete page_info[i];
        }
        delete[] mem;
        delete[] page_info;
    }
    void MemoryManager::PageOut(int physical_page_id, int holder = -2, int virtual_page_id = -2){
        //swap out the physical page with the indx of 'physical_page_id out' into a disk file
        auto a_page = mem[physical_page_id];
        auto a_info = page_info[physical_page_id];
        int a_holder = (holder != -2) ? holder : a_info->GetHolder();
        int a_vid = (virtual_page_id != -2) ? virtual_page_id : a_info->GetVid();
        if(a_holder == -1) return;
        // page_map[a_info->GetHolder()][a_info->GetVid()] = -1;
        auto filename = std::to_string(a_holder) + "-" + std::to_string(a_vid);
        // printf("page out   : id %d, vid %d, phy %d\n", a_holder, a_vid, physical_page_id);
        a_page->WriteDisk(filename);
        // a_info->ClearInfo();
    }
    void MemoryManager::PageIn(int array_id, int virtual_page_id, int physical_page_id){
        //swap the target page from the disk file into a physical page with the index of 'physical_page_id out'
        // printf("page in    : id %d, vid %d, phy %d\n", array_id, virtual_page_id, physical_page_id);
        auto filename = std::to_string(array_id) + "-" + std::to_string(virtual_page_id);
        auto phy_Page = mem[physical_page_id];
        phy_Page->ReadDisk(filename);
        // page_info[physical_page_id]->SetInfo(array_id, virtual_page_id);
    }
    int MemoryManager::PageReplace(int array_id, int virtual_page_id){
        //implement your page replacement policy here
        // int phy_page_id = ReplacementPolicyClock();
        return ReplacementPolicyClock();
        // PageOut(phy_page_id);
        // PageIn(array_id, virtual_page_id, phy_page_id);

        // page_map[page_info[phy_page_id]->GetHolder()][page_info[phy_page_id]->GetVid()] = -1;
        // page_map[array_id][virtual_page_id] = phy_page_id;
        // page_info[phy_page_id]->SetInfo(array_id, virtual_page_id);
    }
    int MemoryManager::ReadPage(int array_id, int virtual_page_id, int offset){
        // for arrayList of 'array_id', return the target value on its virtual space
        // if(offset==0) printf("read page  : id %d, vid %d, offset %d\n", array_id, virtual_page_id, offset);
        mma_lock.lock();
        bool need_replace = false;
        int old_holder, old_virtual_page_id;
        int phy_page_id = page_map[array_id][virtual_page_id];
        if (phy_page_id == -1) {
            need_replace = true;
            phy_page_id = PageReplace(array_id, virtual_page_id);
            old_holder = page_info[phy_page_id]->GetHolder();
            old_virtual_page_id = page_info[phy_page_id]->GetVid();
            page_map[old_holder][old_virtual_page_id] = -1;
            page_map[array_id][virtual_page_id] = phy_page_id;
            page_info[phy_page_id]->SetInfo(array_id, virtual_page_id);
        }
        phy_page_id = page_map[array_id][virtual_page_id];
        page_info[phy_page_id]->used = 1;
        page_info[phy_page_id]->lock();
        mma_lock.unlock();
        if (need_replace){
            PageOut(phy_page_id, old_holder, old_virtual_page_id);
            PageIn(array_id, virtual_page_id, phy_page_id);
        }
        PageFrame* page = mem[phy_page_id];
        int result = (*page)[offset];
        page_info[phy_page_id]->unlock();
        return result;
    }
    void MemoryManager::WritePage(int array_id, int virtual_page_id, int offset, int value){
        // for arrayList of 'array_id', write 'value' into the target position on its virtual space
        // if(offset==0) printf("write page : id %d, vid %d, offset %d, value %d\n", array_id, virtual_page_id, offset, value);
        mma_lock.lock();
        bool need_replace = false;
        int old_holder, old_virtual_page_id;
        int phy_page_id = page_map[array_id][virtual_page_id];
        if (phy_page_id == -1) {
            need_replace = true;
            // printf("write page need replace\n");
            phy_page_id = PageReplace(array_id, virtual_page_id);
            old_holder = page_info[phy_page_id]->GetHolder();
            old_virtual_page_id = page_info[phy_page_id]->GetVid();
            page_map[old_holder][old_virtual_page_id] = -1;
            page_map[array_id][virtual_page_id] = phy_page_id;
            page_info[phy_page_id]->SetInfo(array_id, virtual_page_id);
            PageOut(phy_page_id, old_holder, old_virtual_page_id);
            PageIn(array_id, virtual_page_id, phy_page_id);
        }
        // phy_page_id = page_map[array_id][virtual_page_id];
        page_info[phy_page_id]->used = 1;
        page_info[phy_page_id]->lock();
        mma_lock.unlock();
        if (need_replace){
            // PageOut(phy_page_id, old_holder, old_virtual_page_id);
            // PageIn(array_id, virtual_page_id, phy_page_id);
        }
        PageFrame* page = mem[phy_page_id];
        (*page)[offset] = value;
        page_info[phy_page_id]->unlock();
    }
    void MemoryManager::ClearPage(int array_id, int virtual_page_id){
        int phy_Page_idx = page_map[array_id][virtual_page_id];
        auto filename = std::to_string(array_id) + "-" + std::to_string(virtual_page_id);
        std::string path = ".\\disk\\";
        auto fin = fopen((path + filename + ".txt").c_str(), "w");
        if (fin != NULL) {
            for (int i = 0; i < PageSize; i++) {
                fprintf(fin, "%d\n", 0);
            }
        }
        fclose(fin);
        if (phy_Page_idx != -1) { mem[phy_Page_idx]->Clear(); }
    }
    ArrayList* MemoryManager::Allocate(size_t sz){
        // when an application requires for memory, create an ArrayList and record mappings from its virtual memory space to the physical memory space
        mma_lock.lock();
        int num_page = sz / PageSize;
        if (sz % PageSize != 0) num_page ++;
        ArrayList *a_ArrayList = new ArrayList(sz, this, next_array_id);
        std::map<int, int> a_trans_map;
        for (int i = 0; i < num_page; i++) a_trans_map[i] = -1;
        page_map[next_array_id] = a_trans_map;
        // printf("allocate : id %d : size %d\n", next_array_id, sz);
        next_array_id++;
        mma_lock.unlock();
        return a_ArrayList;

    }
    void MemoryManager::Release(ArrayList* arr){
        // an application will call release() function when destroying its arrayList
        // release the virtual space of the arrayList and erase the corresponding mappings
        mma_lock.lock();
        int array_id = arr->array_id;
        // printf("release : %d\n", arr->array_id);
        for (int i = 0; i < page_map[array_id].size(); i++) {
            ClearPage(array_id, i);
        }
        mma_lock.unlock();
    }

    int MemoryManager::get_empty_page(){
        int dict = free_list;
        for (int i = 0; i < mma_sz; i++){
            if(dict % 2 == 0) {
                free_list += 1<<i;
                return i;
            }
            dict >>= 1;
        }
        return -1;
    }

    int MemoryManager::ReplacementPolicyFIFO(){
        int empty_id = get_empty_page();
        int phy_page_id = 0;
        if(empty_id != -1){
            phy_page_id = empty_id;
            Q_FIFO.push(empty_id);
        } else {
            phy_page_id = Q_FIFO.front();
            // PageOut(phy_page_id);
            Q_FIFO.pop();
            Q_FIFO.push(phy_page_id);
        }
        return phy_page_id;
    }

    int MemoryManager::ReplacementPolicyClock(){
        int empty_id = get_empty_page();
        int phy_page_id = 0;
        if(empty_id != -1){
            phy_page_id = empty_id;
        } else {
            while(true){
                CLOCK_HAND++;
                if(CLOCK_HAND == mma_sz) CLOCK_HAND = 0;

                if(page_info[CLOCK_HAND]->used == 0){
                    phy_page_id = CLOCK_HAND;
                    // PageOut(phy_page_id);
                    break;
                } else {
                    page_info[CLOCK_HAND]->used = 0;
                }
            }
        }
        return phy_page_id;
    }
} // namespce: proj3

#include "array_list.h"
#include "mma_client.h"

namespace proj4 {
    ArrayList::ArrayList (size_t sz, MmaClient* Mma, int id) {
        size = sz;
        mma = Mma;
        array_id = id;
    }
    int ArrayList::Read (unsigned long idx) {
        //read the value in the virtual index of 'idx' from mma's memory space
        size_t virtual_page_id = idx/PageSize;
        size_t offset = idx%PageSize;
        return mma->ReadPage(array_id, virtual_page_id, offset);
    }
    void ArrayList::Write (unsigned long idx, int value) {
        //write 'value' in the virtual index of 'idx' into mma's memory space
        size_t virtual_page_id = idx/PageSize;
        size_t offset = idx%PageSize;
        mma->WritePage(array_id, virtual_page_id, offset, value);
    }
    ArrayList::~ArrayList(){
    }
} // namespce: proj4
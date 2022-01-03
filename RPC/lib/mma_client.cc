
#include "mma_client.h"
#include "array_list.h"
#include <thread>

namespace proj4 {

ArrayList* MmaClient::Allocate(size_t size) {
    ClientContext context;
    AllocateArgs args;
    AllocateReply reply;
    args.set_size(size);
    
    Status status = stub_->Allocate(&context, args, &reply);
    while (!status.ok()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        status = stub_->Allocate(&context, args, &reply);
    }

    return new ArrayList(size, this, reply.arrayid());
}

void MmaClient::Free(ArrayList* al) {
    ClientContext context;
    ReleaseArgs args;
    ReleaseReply reply;
    args.set_arrayid(al->array_id);

    Status status = stub_->Release(&context, args, &reply);
}

int MmaClient::ReadPage(int array_id, int virtual_page_id, int offset) {
    ClientContext context;
    ReadPageArgs args;
    ReadPageReply reply;
    args.set_arrayid(array_id);
    args.set_virtulpageid(virtual_page_id);
    args.set_offset(offset);

    Status status = stub_->ReadPage(&context, args, &reply);

    return reply.value();
}

void MmaClient::WritePage(int array_id, int virtual_page_id, int offset, int value) {
    ClientContext context;
    WritePageArgs args;
    WritePageReply reply;
    args.set_arrayid(array_id);
    args.set_virtulpageid(virtual_page_id);
    args.set_offset(offset);
    args.set_value(value);

    Status status = stub_->WritePage(&context, args, &reply);
}

} //namespace proj4

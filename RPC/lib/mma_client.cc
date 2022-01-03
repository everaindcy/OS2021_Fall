
#include "mma_client.h"
#include "array_list.h"
#include <thread>

namespace proj4 {

ArrayList* MmaClient::Allocate(size_t size) {
    ClientContext context;
    AllocateArgs args;
    AllocateReply reply;
    args.set_size(size);

    // std::cout << "Allocate " << size << std::endl;
    
    Status status = stub_->Allocate(&context, args, &reply);
    while (!status.ok()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        status = stub_->Allocate(&context, args, &reply);
    }

    // std::cout << "Allocate " << size << " success, arrayID = " << reply.arrayid() << std::endl;

    return new ArrayList(size, this, reply.arrayid());
}

void MmaClient::Free(ArrayList* al) {
    ClientContext context;
    ReleaseArgs args;
    ReleaseReply reply;
    args.set_arrayid(al->array_id);

    // std::cout << "Free " << al->array_id << std::endl;

    Status status = stub_->Release(&context, args, &reply);

    // std::cout << "Free " << al->array_id << " success" << std::endl;

    delete al;
}

int MmaClient::ReadPage(int array_id, int virtual_page_id, int offset) {
    ClientContext context;
    ReadPageArgs args;
    ReadPageReply reply;
    args.set_arrayid(array_id);
    args.set_virtulpageid(virtual_page_id);
    args.set_offset(offset);

    // std::cout << "ReadPage " << array_id << ", " << virtual_page_id << ", " << offset << std::endl;

    Status status = stub_->ReadPage(&context, args, &reply);

    // std::cout << "ReadPage " << array_id << ", " << virtual_page_id << ", " << offset << " success, value = " << reply.value() << std::endl;

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

    // std::cout << "WritePage " << array_id << ", " << virtual_page_id << ", " << offset << ", " << value << std::endl;

    Status status = stub_->WritePage(&context, args, &reply);

    // std::cout << "WritePage " << array_id << ", " << virtual_page_id << ", " << offset << ", " << value << " success" << std::endl;
}

} //namespace proj4

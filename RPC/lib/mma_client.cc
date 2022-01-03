
#include "mma_client.h"
#include "array_list.h"
#include <thread>

namespace proj4 {

ArrayList* MmaClient::Allocate(size_t size) {
    // std::cout << "Allocate " + std::to_string(size) + "\n";

    for(;;) {
        ClientContext context;
        AllocateArgs args;
        AllocateReply reply;
        args.set_size(size);

        Status status = stub_->Allocate(&context, args, &reply);

        if (status.ok()) {
            // std::cout << "Allocate " + std::to_string(size) + " success, arrayID = " + std::to_string(reply.arrayid()) + "\n";

            return new ArrayList(size, this, reply.arrayid());
        } else {
            // std::cout << "Allocate " + std::to_string(size) + " cancled\n";

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

void MmaClient::Free(ArrayList* al) {
    ClientContext context;
    ReleaseArgs args;
    ReleaseReply reply;
    args.set_arrayid(al->array_id);

    // std::cout << "Free " + std::to_string(al->array_id) + "\n";

    Status status = stub_->Release(&context, args, &reply);

    // std::cout << "Free " + std::to_string(al->array_id) + " success\n";

    delete al;
}

int MmaClient::ReadPage(int array_id, int virtual_page_id, int offset) {
    ClientContext context;
    ReadPageArgs args;
    ReadPageReply reply;
    args.set_arrayid(array_id);
    args.set_virtulpageid(virtual_page_id);
    args.set_offset(offset);

    // std::cout << "ReadPage " + std::to_string(array_id) + ", " + std::to_string(virtual_page_id) + ", " + std::to_string(offset) + "\n";

    Status status = stub_->ReadPage(&context, args, &reply);

    // std::cout << "ReadPage " + std::to_string(array_id) + ", " + std::to_string(virtual_page_id) + ", " + std::to_string(offset) + " success, value = " + std::to_string(reply.value()) + "\n";

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

    // std::cout << "WritePage " + std::to_string(array_id) + ", " + std::to_string(virtual_page_id) + ", " + std::to_string(offset) + ", " + std::to_string(value) + "\n";

    Status status = stub_->WritePage(&context, args, &reply);

    // std::cout << "WritePage " + std::to_string(array_id) + ", " + std::to_string(virtual_page_id) + ", " + std::to_string(offset) + ", " + std::to_string(value) + " success\n";
}

} //namespace proj4

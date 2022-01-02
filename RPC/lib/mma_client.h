#ifndef MMA_CLIENT_H
#define MMA_CLIENT_H

#include <memory>
#include <cstdlib>

#include <grpc++/grpc++.h>

#ifdef BAZEL_BUILD
#include "proto/mma.grpc.pb.h"
#else
#include "mma.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using mma::MemoryManager;
using mma::AllocateArgs;
using mma::ReleaseArgs;
using mma::ReadPageArgs;
using mma::WritePageArgs;
using mma::AllocateReply;
using mma::ReleaseReply;
using mma::ReadPageReply;
using mma::WritePageReply;


namespace proj4 {

class ArrayList;

class MmaClient {
public:
    MmaClient(std::shared_ptr<Channel> channel) : stub_(MemoryManager::NewStub(channel)) {}
    int ReadPage(int array_id, int virtual_page_id, int offset);
    void WritePage(int array_id, int virtual_page_id, int offset, int value);
    void ClearPage(int array_id, int virtual_page_id);
    ArrayList* Allocate(size_t);
    void Release(ArrayList*);

private:
    std::unique_ptr<MemoryManager::Stub> stub_;
};

} //namespace proj4

#endif
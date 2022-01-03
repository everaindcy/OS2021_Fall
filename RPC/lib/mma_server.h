#ifndef MMA_SERVER_H
#define MMA_SERVER_H

#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <mutex>

#include <grpc++/grpc++.h>
#include <grpc++/ext/proto_server_reflection_plugin.h>
#include <grpc++/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "proto/mma.grpc.pb.h"
#else
#include "mma.grpc.pb.h"
#endif

#include "memory_manager.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using mma::memoryManager;
using mma::AllocateArgs;
using mma::ReleaseArgs;
using mma::ReadPageArgs;
using mma::WritePageArgs;
using mma::AllocateReply;
using mma::ReleaseReply;
using mma::ReadPageReply;
using mma::WritePageReply;


// Logic and data behind the server's behavior.

namespace proj4 {

class MmaServer final : public memoryManager::Service {
public:
    MmaServer(size_t phy_page_num) : mma(new MemoryManager(phy_page_num)) {}
    MmaServer(size_t phy_page_num, size_t max_vir_page_num)
        : mma(new MemoryManager(phy_page_num)), max_vir_page_num(max_vir_page_num) {}
    ~MmaServer() {delete mma;}

    Status Allocate (ServerContext* context, const AllocateArgs*  args, AllocateReply*  reply) override;
    Status Release  (ServerContext* context, const ReleaseArgs*   args, ReleaseReply*   reply) override;
    Status ReadPage (ServerContext* context, const ReadPageArgs*  args, ReadPageReply*  reply) override;
    Status WritePage(ServerContext* context, const WritePageArgs* args, WritePageReply* reply) override;

private:
    MemoryManager* mma;
    size_t max_vir_page_num = 0;
    mutable std::mutex mux;
    size_t total_vir_page_num = 0;
};

// setup a server with UnLimited virtual memory space
void RunServerUL(size_t phy_page_num);

// setup a server with Limited virtual memory space
void RunServerL(size_t phy_page_num, size_t max_vir_page_num);

// shutdown the server setup by RunServerUL or RunServerL
void ShutdownServer();

} //namespace proj4

#endif
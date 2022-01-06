#include "mma_server.h"

namespace proj4 {

Status MmaServer::Allocate(ServerContext* context, const AllocateArgs* args, AllocateReply* reply) {
    size_t pageNeed = (args->size() + PageSize - 1) / PageSize;
    this->mux.lock();
    if ((max_vir_page_num != 0) && (total_vir_page_num >= max_vir_page_num)) {
        this->mux.unlock();
        return Status::CANCELLED;
    }
    total_vir_page_num += pageNeed;
    // std::cout << std::to_string(total_vir_page_num) + "/" + std::to_string(max_vir_page_num) + "\n";
    this->mux.unlock();
    int ArrayID = mma->Allocate(args->size());
    reply->set_arrayid(ArrayID);
    return Status::OK;
}

Status MmaServer::Release(ServerContext* context, const ReleaseArgs* args, ReleaseReply* reply) {
    size_t release_page_num = mma->Release(args->arrayid());
    this->mux.lock();
    total_vir_page_num -= release_page_num;
    // std::cout << std::to_string(total_vir_page_num) + "/" + std::to_string(max_vir_page_num) + "\n";
    this->mux.unlock();
    return Status::OK;
}

Status MmaServer::ReadPage(ServerContext* context, const ReadPageArgs* args, ReadPageReply* reply) {
    int value = mma->ReadPage(args->arrayid(), args->virtulpageid(), args->offset());
    reply->set_value(value);
    return Status::OK;
}

Status MmaServer::WritePage(ServerContext* context, const WritePageArgs* args, WritePageReply* reply) {
    mma->WritePage(args->arrayid(), args->virtulpageid(), args->offset(), args->value());
    return Status::OK;
}

std::unique_ptr<Server> server;

void RunServerUL(size_t phy_page_num) {
    std::string server_address("0.0.0.0:50051");
    MmaServer service(phy_page_num);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    server = std::unique_ptr<Server>(builder.BuildAndStart());
    // std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

void RunServerL(size_t phy_page_num, size_t max_vir_page_num) {
    std::string server_address("0.0.0.0:50051");
    MmaServer service(phy_page_num, max_vir_page_num);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    server = std::unique_ptr<Server>(builder.BuildAndStart());
    // std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server->Wait();
}

void ShutdownServer() {
    server->Shutdown();
}

} //namespace proj4

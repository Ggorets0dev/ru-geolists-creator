#include <memory>
#include <string>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <grpcpp/grpcpp.h>

#define CALLBACK_UNIMPLEMENTED_ERROR_MSG        "Requested callback is not implemented by service"

#include "log.hpp"
#include "service.hpp"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using rglc::KeepAliveRequest;
using rglc::KeepAliveResponse;
using rglc::RGLC_Service;

class RGLC_ServiceImpl final : public RGLC_Service::Service {
private:
    std::mutex mtx_;
    std::condition_variable cv_;
    bool keep_alive_received_ = false;
    bool running_ = true;
    Server* server_ptr_ = nullptr;
    ServiceCallbacks callbacks_ = {};

    void resetWatchDog() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            keep_alive_received_ = true;
        }
        cv_.notify_one();
    }

public:
    void SetServerInstance(Server* server) {
        server_ptr_ = server;
    }

    void SetCallbacks(const ServiceCallbacks& callbacks) {
        callbacks_ = callbacks;
    }

    Status SendKeepAlive(ServerContext* context, const KeepAliveRequest* request,
                         KeepAliveResponse* response) override {
        resetWatchDog();

        LOG_MSG(LOG_LEVEL_INFO, LOG_MODULE_SERVICE, "Got \"keep alive\" request");
        return Status::OK;
    }

    void runWatchDog(const time_t timeout_sec) {
        while (running_) {
            std::unique_lock<std::mutex> lock(mtx_);
            const auto status = cv_.wait_for(lock, std::chrono::seconds(timeout_sec), [this] {
                return keep_alive_received_ || !running_; 
            });

            if (!status && running_) {
                running_ = false;
                if (server_ptr_) {
                    LOG_MSG(LOG_LEVEL_INFO, LOG_MODULE_SERVICE, "WatchDog requested shutdown");
                    server_ptr_->Shutdown();
                }
            } else {
                keep_alive_received_ = false;
            }
        }
    }

    void StopWatchdog() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            running_ = false;
        }
        cv_.notify_one();
    }

    Status BuildGeoLists(ServerContext *context, const rglc::BuildGeoListsRequest *request, rglc::BuildGeoListsResponse *response) override {
        resetWatchDog();
        LOG_MSG(LOG_LEVEL_INFO, LOG_MODULE_SERVICE, "Got \"build geolists\" request");

        if (callbacks_.buildGeoLists) {
            return callbacks_.buildGeoLists(request, response);
        }

        Status status(grpc::StatusCode::UNIMPLEMENTED, CALLBACK_UNIMPLEMENTED_ERROR_MSG);

        return status;
    }
};

void runService(const ServiceSettings& settings, const ServiceCallbacks& callbacks) {
    RGLC_ServiceImpl service;
    std::thread watchdog_thread;

    const std::string server_address = fmt::format("{}:{}", settings.addr, settings.port);

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    const std::unique_ptr server(builder.BuildAndStart());

    service.SetServerInstance(server.get());
    service.SetCallbacks(callbacks);

    LOG_MSG(LOG_LEVEL_INFO, LOG_MODULE_SERVICE, "Service is started, listening on " + server_address);

    if (settings.timeout_sec) {
        LOG_MSG(LOG_LEVEL_INFO, LOG_MODULE_SERVICE, "WatchDog for server is launched: {} sec", settings.timeout_sec);
        watchdog_thread = std::thread(&RGLC_ServiceImpl::runWatchDog, &service, settings.timeout_sec);
    }

    server->Wait();

    if (settings.timeout_sec) {
        service.StopWatchdog();
        if (watchdog_thread.joinable()) {
            watchdog_thread.join();
        }
    }
}
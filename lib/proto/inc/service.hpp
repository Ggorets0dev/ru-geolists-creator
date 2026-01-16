#ifndef RGLC_SERVICE_HPP
#define RGLC_SERVICE_HPP

#include <netinet/in.h>

#include "rglc_service.grpc.pb.h"

using BuildGeoListsCallback = std::function<grpc::Status(const rglc::BuildGeoListsRequest *request, rglc::BuildGeoListsResponse *response)>;

struct ServiceCallbacks {
    BuildGeoListsCallback buildGeoLists;
};

struct ServiceSettings {
    std::string addr = "0.0.0.0";
    in_port_t port = 50051;
    time_t timeout_sec = 10;
};

void runService(const ServiceSettings& settings, const ServiceCallbacks& callbacks);

#endif //RGLC_SERVICE_HPP
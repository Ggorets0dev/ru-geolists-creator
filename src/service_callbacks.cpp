#include "service_callbacks.hpp"
#include "log.hpp"
#include "build_lists_handler.hpp"

ServiceCallbacks gServiceCallbacks;

static CmdArgs parseCmdArgs(const rglc::BuildGeoListsRequest& request) {
    CmdArgs args = {};

    args.isForceCreation = request.is_force();
    args.isNoExtra = !request.is_use_extra();
    args.outDirPath = request.out_dir();
    args.isUseWhitelist = request.is_use_whitelist();
    std::copy(request.formats().begin(), request.formats().end(), std::back_inserter(args.formats));

    return args;
}

static const BuildGeoListsCallback BuildGeoLists = [](const rglc::BuildGeoListsRequest *request, rglc::BuildGeoListsResponse *response) {
    const auto args = parseCmdArgs(*request);

    const auto releases = buildListsHandler(args);

    if (!releases) {
        grpc::Status status(grpc::StatusCode::UNKNOWN, "Failed to build geolists");
        return status;
    }

    if (releases->isEmpty) {
        response->set_is_empty(true);
        grpc::Status status(grpc::StatusCode::OK, "No need to build lists, no source update available");
        return status;
    }

    response->set_release_notes_path(releases->releaseNotes);

    for (const auto& release : releases->packs) {
        response->add_geoip_paths(release.listIP);
        response->add_geosite_paths(release.listDomain);
    }

    return grpc::Status::OK;
};

void fillServiceCallbacks(ServiceCallbacks& callbacks) {
    callbacks.buildGeoLists = BuildGeoLists;
}
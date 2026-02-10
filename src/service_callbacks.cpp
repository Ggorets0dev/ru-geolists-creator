#include "service_callbacks.hpp"
#include "log.hpp"
#include "build_lists_handler.hpp"

ServiceCallbacks gServiceCallbacks;

static CmdArgs parseCmdArgs(const rglc::BuildGeoListsRequest& request) {
    CmdArgs args = {};

    args.outDirPath = request.out_dir();
    args.isUseWhitelist = request.is_use_whitelist();
    std::copy(request.formats().begin(), request.formats().end(), std::back_inserter(args.formats));
    std::copy(request.presets().begin(), request.presets().end(), std::back_inserter(args.presets));

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
        grpc::Status status(grpc::StatusCode::OK, "No need to build lists, no source update available");
        return status;
    }

    response->set_release_notes_path(releases->releaseNotes);

    for (const auto& release : releases->packs) {
        auto* pack_msg = response->add_packs();
        pack_msg->set_preset_label(release.presetLabel);

        if (release.listDomain.has_value()) {
            pack_msg->set_list_domain(release.listDomain->string());
        }

        if (release.listIP.has_value()) {
            pack_msg->set_list_ip(release.listIP->string());
        }
    }

    return grpc::Status::OK;
};

void fillServiceCallbacks(ServiceCallbacks& callbacks) {
    callbacks.buildGeoLists = BuildGeoLists;
}
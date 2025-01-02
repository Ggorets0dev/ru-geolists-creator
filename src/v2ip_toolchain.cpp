#include "v2ip_toolchain.hpp"

#define V2IP_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/geoip/releases/latest"

namespace fs = std::filesystem;

std::optional<std::string>
downloadV2ipSourceCode() {
    bool status = 1;
    const uint8_t attempt_cnt = 5;

    status = downloadFile(V2IP_API_LAST_RELEASE_URL, V2IP_RELEASE_REQ_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to get data on the V2IP repository. Check your internet connection");
        return std::nullopt;
    }

    Json::Value request;
    status = readJsonFromFile(V2IP_RELEASE_REQ_FILE_NAME, request);

    if (!status) {
        LOG_ERROR("Deserialization of the repository request failed");
        return std::nullopt;
    }
    fs::remove(V2IP_RELEASE_REQ_FILE_NAME);

    std::string lastReleaseUrl = request["tarball_url"].asString();

    for(uint8_t i(0); i < attempt_cnt; ++i) {
        status = downloadFile(lastReleaseUrl, V2IP_SRC_FILE_NAME);

        if (status) {
            LOG_INFO("V2IP source code was successfully downloaded");
            break;
        } else {
            LOG_ERROR("Failed to download the V2IP source code, performing another attempt...");
        }

        std::this_thread::sleep_for(std::chrono::seconds(DOWNLOAD_TRY_DELAY_SEC));
    }

    if (!status) {
        LOG_ERROR("V2IP source code could not be downloaded after several attempts. Check your internet connection");
        return std::nullopt;
    }

    return V2IP_SRC_FILE_NAME;
}

bool
runToolchain(const std::string& rootPath) {
    const fs::path kCurrentDir = fs::current_path();

    fs::current_path(rootPath.c_str());

    int result = std::system("go run ./");

    if (result == 0) {
        LOG_INFO("IP address list building with XRay tools has been successfully completed");
    } else {
        LOG_ERROR("Failed to build IP address list using XRay tools");
    }

    fs::current_path(kCurrentDir);

    return !result;
}

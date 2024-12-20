#include "v2ip_toolchain.hpp"

#define V2IP_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/geoip/releases/latest"

namespace fs = std::filesystem;

std::optional<std::string>
downloadV2ipSourceCode() {
    bool status = 1;
    const uint8_t attempt_cnt = 5;

    status = downloadFile(V2IP_API_LAST_RELEASE_URL, V2IP_RELEASE_REQ_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to get data on the DLC repository. Check your internet connection");
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
            LOG_INFO("DLC source code was successfully downloaded");
            break;
        } else {
            LOG_ERROR("Failed to download the DLC source code, performing another attempt...");
        }

        std::this_thread::sleep_for(std::chrono::seconds(DOWNLOAD_TRY_DELAY_SEC));
    }

    if (!status) {
        LOG_ERROR("DLC source code could not be downloaded after several attempts. Check your internet connection");
        return std::nullopt;
    }

    return V2IP_SRC_FILE_NAME;
}

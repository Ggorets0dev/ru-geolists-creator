#include "dlc_toolchain.hpp"

#define DLC_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/domain-list-community/releases/latest"
#define DLC_SRC_FILE_NAME           "dlc_src.tar.gz"
#define DLC_RELEASE_REQ_FILE_NAME   "dlc_req.json"

namespace fs = std::filesystem;

bool
downloadDlcSourceCode() {
    bool status = 1;
    const uint8_t attempt_cnt = 5;

    status = downloadFile(DLC_API_LAST_RELEASE_URL, DLC_RELEASE_REQ_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to get data on the DLC repository. Check your internet connection");
        return false;
    }

    Json::Value request;
    status = readJsonFromFile(DLC_RELEASE_REQ_FILE_NAME, request);

    if (!status) {
        LOG_ERROR("Deserialization of the repository request failed");
        return false;
    }

    std::string lastReleaseUrl = request["tarball_url"].asString();

    for(uint8_t i(0); i < attempt_cnt; ++i) {
        status = downloadFile(lastReleaseUrl, DLC_SRC_FILE_NAME);

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
        return false;
    }

    return true;
}

bool
clearDlcDataSection(std::string dlcRootPath) {
    dlcRootPath += "/data"; // Now it points to data section

    try {
        if (fs::exists(dlcRootPath) && fs::is_directory(dlcRootPath)) {
            for (const auto& entry : fs::directory_iterator(dlcRootPath)) {
                if (fs::is_regular_file(entry.path()) || fs::is_symlink(entry.path())) {
                    fs::remove(entry.path()); // Удаляем файл или символическую ссылку
                } else if (fs::is_directory(entry.path())) {
                    fs::remove_all(entry.path()); // Удаляем вложенную директорию
                }
            }
            LOG_INFO("Data section of DLC was cleared");
        } else {
            LOG_ERROR("Failed to clear DLC data section. Path is not a directory or does not exist: " + dlcRootPath);
            return false;
        }
    } catch (const fs::filesystem_error& e) {
        log(LogType::ERROR, "Filesystem error:", e.what());
        return false;
    }

    return true;
}

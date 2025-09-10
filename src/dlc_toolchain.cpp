#include "dlc_toolchain.hpp"
#include "log.hpp"
#include "json_io.hpp"
#include "network.hpp"

#define DLC_API_LAST_RELEASE_URL    "https://api.github.com/repos/v2fly/domain-list-community/releases/latest"

const fs::path gkDlcToolchainDir = fs::path(std::getenv("HOME")) / ".local" / "lib";

std::optional<std::string> downloadDlcSourceCode() {
    bool status = true;

    if (!tryDownloadFile(DLC_API_LAST_RELEASE_URL, DLC_RELEASE_REQ_FILE_NAME)) {
        LOG_ERROR("Failed to perform API request for DLC repository");
        return std::nullopt;
    }

    Json::Value request;
    status = readJsonFromFile(DLC_RELEASE_REQ_FILE_NAME, request);

    if (!status) {
        LOG_ERROR("Deserialization of the repository request failed");
        return std::nullopt;
    }
    fs::remove(DLC_RELEASE_REQ_FILE_NAME);

    std::string lastReleaseUrl = request["tarball_url"].asString();

    if (tryDownloadFile(lastReleaseUrl, DLC_SRC_FILE_NAME)) {
        LOG_INFO("DLC source code was successfully downloaded");
    } else {
        LOG_ERROR("DLC source code could not be downloaded after several attempts");
        return std::nullopt;
    }

    return DLC_SRC_FILE_NAME;
}

bool clearDlcDataSection(std::string dlcRootPath) {
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
        LOG_ERROR("Filesystem error:" + std::string(e.what()));
        return false;
    }

    return true;
}

bool addDomainSource(const std::string& dlcRootPath, const fs::path& sourceFilePath, const std::string& section) {
    fs::path newFilePath = dlcRootPath;

    newFilePath /= "data";
    newFilePath /= section;
    newFilePath.replace_extension("");

    try {
        fs::copy(sourceFilePath, newFilePath);
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error:" + std::string(e.what()));
        return false;
    }

    return true;
}

std::optional<fs::path> runDlcToolchain(const std::string& rootPath) {
    const fs::path kCurrentDir = fs::current_path();
    std::optional<fs::path> outFilePath;

    fs::current_path(rootPath.c_str());

    int result = std::system("go run ./");

    if (result == 0) {
        outFilePath = fs::current_path() / "dlc.dat";
        LOG_INFO("Domain address list building with XRay tools has been successfully completed");
    } else {
        outFilePath = std::nullopt;
        LOG_ERROR("Failed to build Domain address list using XRay tools");
    }

    fs::current_path(kCurrentDir);

    return outFilePath;
}

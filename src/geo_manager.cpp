#include "geo_manager.hpp"
#include "url_handle.hpp"
#include "log.hpp"
#include "fs_utils_temp.hpp"

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define GEO_MGR_LAST_RELEASE_URL        "https://api.github.com/repos/MetaCubeX/geo/releases/latest"

const fs::path gkGeoManagerDir = fs::path(std::getenv("HOME")) / ".local" / "lib";

std::optional<std::string> setupGeoManagerBinary() {
    bool status;
    std::string geoMgrBinary;
    std::vector<std::string> downloads;

#if defined(__x86_64__) || defined(_M_AMD64)
    geoMgrBinary = "geo-linux-amd64";
#elif defined(__aarch64__) || defined(__arm64__)
    geoMgrBinary = "geo-linux-arm64";
#endif

    std::string geoMgrTargetPath = gkGeoManagerDir / geoMgrBinary;

    // ========= Temp files control
    FS::Utils::Temp::SessionTempFileRegistry tempFileReg;
    auto geoMgrReqFile = tempFileReg.createTempFile("json");
    // =========

    LOG_INFO("Starting to setup Geo manager binary...");

    if (!NetUtils::tryDownloadFile(GEO_MGR_LAST_RELEASE_URL, geoMgrReqFile.lock()->path)) {
        LOG_ERROR("Failed to perform API request for V2IP repository");
        return std::nullopt;
    }

    Json::Value request;
    status = readJsonFromFile(geoMgrReqFile.lock()->path, request);

    if (!status) {
        LOG_ERROR("Failed to read JSON from API request (Geo manager)");
        return std::nullopt;
    }

    downloads = NetUtils::downloadGithubReleaseAssets(request, { geoMgrBinary }, tempFileReg.getTempDir());

    if (downloads.empty()) {
        LOG_ERROR("Failed to download Geo manager release assets");
        return std::nullopt;
    }

    fs::copy_file(geoMgrBinary, geoMgrTargetPath, fs::copy_options::overwrite_existing);

    fs::permissions(geoMgrTargetPath,
                    fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec,
                    fs::perm_options::add);

    LOG_INFO("Geo manager installed successfully");

    return geoMgrTargetPath;
}

bool convertGeolist(const std::string& binPath, const Source::Type type,
                    const std::string& inFormat, const std::string& outFormat,
                    const std::string& inPath, const std::string& outPath) {

    const pid_t pid = fork();
    int status;

    const std::string sourceType = (type == Source::Type::DOMAIN) ? "site" : "ip";

    if (pid == 0) {
        // Child process, running Geo manager...

        LOG_INFO("Fork for running Geo manager is completed, child process created");

        suppressConsoleOutput();

        execl(binPath.c_str(), fs::path(binPath).filename().string().c_str(), "convert", sourceType.c_str(), "-i", inFormat.c_str(),
              "-o", outFormat.c_str(), "-f", outPath.c_str(), inPath.c_str(), nullptr);

        LOG_ERROR("Failed to run Geo manager for converting list");
        exit(1);
    }

    if (pid > 0) {
        // Waiting for Geo manager to convert list
        waitpid(pid, &status, 0);

        if (status != 0) {
            LOG_ERROR("Failed to convert list using Geo manager, process returned error code status");
            return false;
        }

        return true;
    }

    LOG_ERROR("Incorrect PID returned from OS");
    return false;
}

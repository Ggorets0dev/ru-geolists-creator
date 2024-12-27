#include "handlers.hpp"

#define VALIDATE_INIT_PART_RESULT(condition) \
    if (!condition) { \
        LOG_ERROR(SOFTWARE_INIT_FAIL_MSG); \
        exit(1); \
    }

#define VALIDATE_CHECK_UPDATES_PART_RESULT(condition) \
    if (!condition) { \
        LOG_ERROR(CHECK_UPDATES_FAIL_MSG); \
        return std::make_tuple(false, false); \
    }

#define VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(condition) \
    if (!condition) { \
        LOG_ERROR(DOWNLOAD_UPDATES_FAIL_MSG); \
        return false; \
    }

void
initSoftware() {
    bool status;

    // SECTION - Download DLC
    auto dlcArchivePath = downloadDlcSourceCode();
    VALIDATE_INIT_PART_RESULT(dlcArchivePath);

    auto dlcRootPath = extractTarGz(*dlcArchivePath, "./");
    VALIDATE_INIT_PART_RESULT(dlcRootPath);

    status = clearDlcDataSection(*dlcRootPath);
    VALIDATE_INIT_PART_RESULT(status);

    fs::remove(*dlcArchivePath);
    // !SECTION

    // SECTION - Download V2IP
    auto v2ipArchivePath = downloadV2ipSourceCode();
    VALIDATE_INIT_PART_RESULT(v2ipArchivePath);

    auto v2ipRootPath = extractTarGz(*v2ipArchivePath, "./");
    VALIDATE_INIT_PART_RESULT(v2ipRootPath);

    fs::remove(*v2ipArchivePath);
    // !SECTION

    // SECTION - Create config
    RgcConfig config;

    config.dlcRootPath = std::move(*dlcRootPath);
    config.v2ipRootPath = std::move(*v2ipRootPath);

    config.refilterTime = 0u;
    config.v2rayTime = 0u;
    config.ruadlistVersion = "";

    status = writeConfig(config);
    VALIDATE_INIT_PART_RESULT(status);
    // !SECTION

    if (status) {
        LOG_INFO("Initalization successfully completed, all components installed");
    } else {
        LOG_ERROR(SOFTWARE_INIT_FAIL_MSG);
        exit(1);
    }
}

std::tuple<bool, bool>
checkForUpdates(const RgcConfig& config) {
    bool status;
    bool isUpdateFound;
    Json::Value value;
    std::string ruadlistVersion;
    std::optional<std::time_t> lastReleaseTime;

    // SECTION - Check ReFilter for updates
    status = downloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    fs::remove(REFILTER_RELEASE_REQ_FILE_NAME);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(lastReleaseTime);

    isUpdateFound = config.refilterTime < *lastReleaseTime;

    if (isUpdateFound) {
        LOG_INFO("An update to the ReFilter lists has been detected");
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    // SECTION - Check XRAY rules for updates
    status = downloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(XRAY_RULES_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    fs::remove(REFILTER_RELEASE_REQ_FILE_NAME);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(lastReleaseTime);

    isUpdateFound = config.refilterTime < *lastReleaseTime;

    if (isUpdateFound) {
        LOG_INFO("An update to the XRAY rules has been detected");
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    // SECTION - Check RUADLIST for updates
    status = downloadFile(RUADLIST_URL, RUADLIST_FILE_NAME);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = parseRuadlistVersion(RUADLIST_FILE_NAME, ruadlistVersion);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    isUpdateFound = config.ruadlistVersion != ruadlistVersion;

    std::cout << ruadlistVersion;

    if (isUpdateFound) {
        LOG_INFO("An update to the RUADLIST has been detected");
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    return std::make_tuple(status, isUpdateFound);
}

bool
downloadNewestSources(RgcConfig& config) {
    bool status;
    Json::Value value;
    std::optional<std::time_t> lastReleaseTime;
    std::string ruadlistVersion;
    std::vector<std::string> assetsNames;

    // SECTION - Download newest ReFilter rules
    status = downloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.refilterTime = *lastReleaseTime;

    assetsNames = {"domains_all.lst", "ipsum.lst"};
    status = downloadGithubReleaseAssets(value, assetsNames);\
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);
    // !SECTION

    // SECTION - Download newest XRay rules
    status = downloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(XRAY_RULES_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.v2rayTime = *lastReleaseTime;

    assetsNames = {"reject-list.txt"};
    status = downloadGithubReleaseAssets(value, assetsNames);\
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);
    // !SECTION

    // SECTION - Download newest RUADLIST rules
    status = downloadFile(RUADLIST_URL, RUADLIST_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = parseRuadlistVersion(RUADLIST_FILE_NAME, ruadlistVersion);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    config.ruadlistVersion = std::move(ruadlistVersion);
    // !SECTION

    return status;
}

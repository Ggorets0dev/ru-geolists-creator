#include "handlers.hpp"

#define TEMP_DIR_PATH   "./temp"

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
checkForUpdates() {
    bool status;
    bool isUpdateFound;
    Json::Value value;
    RgcConfig config;
    std::optional<std::time_t> lastReleaseTime;

    status = readConfig(config);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    if (!fs::exists(TEMP_DIR_PATH)) {
        mkdir(TEMP_DIR_PATH, 0755);
    }
    chdir(TEMP_DIR_PATH);

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
    // !SECTION

    return std::make_tuple(status, isUpdateFound);
}

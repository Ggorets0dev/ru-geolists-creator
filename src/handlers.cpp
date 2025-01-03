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

static std::string
parseUnixTime(const std::time_t& timeU) {
    // Преобразуйте unix-время в структуру tm
    std::tm* time_info = std::localtime(&timeU);

    // Создайте строку формата день.месяц.год часы:минуты:секунды
    std::stringstream ss;
    ss << time_info->tm_mday << '.' << (time_info->tm_mon + 1) << '.' << (time_info->tm_year + 1900) << ' ';
    ss << time_info->tm_hour << ':' << time_info->tm_min << ':' << time_info->tm_sec;

    // Выведите строку
    return ss.str();
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

    config.refilterTime = CFG_DEFAULT_NUM_VALUE;
    config.v2rayTime = CFG_DEFAULT_NUM_VALUE;
    config.ruadlistVersion = CFG_DEFAULT_STR_VALUE;
    config.apiToken = CFG_DEFAULT_STR_VALUE;

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
    std::string logMsg;
    std::optional<std::time_t> lastReleaseTime;

    std::string gitHttpHeader = "Authorization: token " + config.apiToken;

    // SECTION - Check ReFilter for updates
    if (!config.apiToken.empty()) {
        status = downloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME, gitHttpHeader.c_str());
    } else {
        status = downloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);
    }
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    fs::remove(REFILTER_RELEASE_REQ_FILE_NAME);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(lastReleaseTime);

    isUpdateFound = config.refilterTime < *lastReleaseTime;

    if (isUpdateFound) {
        logMsg = "An update to the ReFilter lists has been detected: ";
        logMsg += parseUnixTime(*lastReleaseTime);
        logMsg += " vs ";
        logMsg += parseUnixTime(config.refilterTime);

        LOG_WARNING(logMsg);
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    // SECTION - Check XRAY rules for updates
    if (!config.apiToken.empty()) {
        status = downloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME, gitHttpHeader.c_str());
    } else {
        status = downloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME);
    }
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(XRAY_RULES_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    fs::remove(XRAY_RULES_RELEASE_REQ_FILE_NAME);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(lastReleaseTime);

    isUpdateFound = config.v2rayTime < *lastReleaseTime;

    if (isUpdateFound) {
        logMsg = "An update to the XRay lists has been detected: ";
        logMsg += parseUnixTime(*lastReleaseTime);
        logMsg += " vs ";
        logMsg += parseUnixTime(config.refilterTime);

        LOG_WARNING(logMsg);
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    // SECTION - Check RUADLIST for updates
    status = downloadFile(RUADLIST_FULL_URL, RUADLIST_FILE_NAME);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = parseRuadlistVersion(RUADLIST_FILE_NAME, ruadlistVersion);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    isUpdateFound = config.ruadlistVersion != ruadlistVersion;

    if (isUpdateFound) {
        logMsg = "An update to the RUADLIST has been detected: ";
        logMsg += (ruadlistVersion + " vs " + config.ruadlistVersion);

        LOG_WARNING(logMsg);
        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    return std::make_tuple(status, isUpdateFound);
}

bool
downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles) {
    bool status;
    Json::Value value;
    std::optional<std::time_t> lastReleaseTime;
    std::string latestRuadlistVersion;
    std::vector<std::string> assetsNames;

    const fs::path kCurrentDir = fs::current_path();

    // SECTION - Download newest ReFilter rules
    status = downloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.refilterTime = *lastReleaseTime;

    assetsNames = {"domains_all.lst", "ipsum.lst"};
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[0]));
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::IP, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[1]));
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
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, "xray_reject"), kCurrentDir / assetsNames[0]));
    // !SECTION

    // SECTION - Download newest RUADLIST rules
    status = downloadFile(RUADLIST_FULL_URL, RUADLIST_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = parseRuadlistVersion(RUADLIST_FILE_NAME, latestRuadlistVersion);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = downloadFile(RUADLIST_ADSERVERS_URL, RUADLIST_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = extractDomainsFromFile(RUADLIST_FILE_NAME, RUADLIST_EXTRACTED_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = removeDuplicateDomains(RUADLIST_EXTRACTED_FILE_NAME, assetsNames[0]); // reject-list.txt

    config.ruadlistVersion = std::move(latestRuadlistVersion);
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, RUADLIST_SECTION_NAME), kCurrentDir / RUADLIST_EXTRACTED_FILE_NAME));
    // !SECTION

    // SECTION - Download extra sources
    for (const auto& source : config.extraSources) {
        status = downloadFile(source.url, source.section);
        VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);
        downloadedFiles.push_back(DownloadedSourcePair(Source(source.type, source.section), kCurrentDir / source.section));
    }
    // !SECTION

    return status;
}


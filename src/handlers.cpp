#include "handlers.hpp"
#include "time_tools.hpp"
#include "ruadlist.hpp"
#include "archive.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "network.hpp"
#include "software_info.hpp"
#include "cli_args.hpp"

#include <string>

#define PRINT_DELIMETER "============================"

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

void printSoftwareInfo() {
    std::cout << PRINT_DELIMETER << std::endl;
    std::cout << "ru-geolists-creator v" << RGC_VERSION_STRING << std::endl;
    std::cout << "Developer: " << RGC_DEVELOPER << std::endl;
    std::cout << "License: " << RGC_LICENSE << std::endl;
    std::cout << "GitHub: " << RGC_REPOSITORY << std::endl;
    std::cout << PRINT_DELIMETER << std::endl;
}

void showExtraSources() {
    RgcConfig config;
    bool status;
    uint16_t recordId(1);

    status = readConfig(config);

    if (!status) {
        LOG_ERROR("Failed to show all extra sources");
        exit(1);
    }

    if (std::distance(config.extraSources.begin(), config.extraSources.end()) == 0) {
        LOG_INFO("No extra sources were found in config file");
        return;
    }

    std::cout << "\nExtra sources specified in config: \n\n";

    std::cout << PRINT_DELIMETER << std::endl;

    for (const auto& source : config.extraSources) {
        std::cout << "[ID = " << recordId << "]\n\n";

        source.print(std::cout);
        std::cout << PRINT_DELIMETER << std::endl;

        ++recordId;
    }
}

void addExtraSource() {
    RgcConfig config;
    ExtraSource source;
    bool status;

    std::string buffer;
    buffer.reserve(10);

    status = readConfig(config);

    if (!status) {
        LOG_ERROR(ADD_EXTRA_FAIL_MSG);
        exit(1);
    }

    LOG_INFO("Information about new source is required");

    getStringInput("Type (ip/domain)", buffer, false);
    source.type = sourceStringToType(buffer);

    getStringInput("Section", source.section, false);
    getStringInput("URL", source.url, false);

    status = tryAccessUrl(source.url);

    if (!status) {
        LOG_WARNING("Unable to access the list at the specified URL, resource not added");
        return;
    }

    config.extraSources.push_front(source);

    status = writeConfig(config);

    if (!status) {
        LOG_ERROR(ADD_EXTRA_FAIL_MSG);
        exit(1);
    }

    LOG_INFO("Successfully added source to config file");
}

void removeExtraSource(SourceId id) {
    RgcConfig config;
    bool status;

    SourceId currId(0);

    SourceId beforeSize(0);
    SourceId afterSize(0);

    status = readConfig(config);

    if (!status) {
        LOG_ERROR(REMOVE_EXTRA_FAIL_MSG);
        exit(1);
    }

    // Get size before deleting
    for (const auto& source : config.extraSources) {
        ++beforeSize;
    }

    config.extraSources.remove_if([&currId, &id](const auto& source) {
        ++currId;
        return currId == id;
    });

    // Get size after deleting
    for (const auto& source : config.extraSources) {
        ++afterSize;
    }

    if (beforeSize == afterSize) {
        LOG_WARNING("Failed to find source with specified ID for removing");
        return;
    }

    status = writeConfig(config);

    if (!status) {
        LOG_ERROR(REMOVE_EXTRA_FAIL_MSG);
        exit(1);
    }

    LOG_INFO("Successfully removed source from config file");
}

void checkUrlsAccess() {
    RgcConfig config;
    bool isAccessed;

    // Adding all main sources
    std::vector<std::string> urls = {
        REFILTER_API_LAST_RELEASE_URL,
        XRAY_RULES_API_LAST_RELEASE_URL,
        RUADLIST_API_MASTER_URL,
        RUADLIST_ADSERVERS_URL,
        ANTIFILTER_AYN_IPS_URL
    };

    LOG_INFO("Check for all soirce's URLs is requested");

    isAccessed = readConfig(config);

    if (isAccessed) {
        std::transform(config.extraSources.begin(), config.extraSources.end(), std::back_inserter(urls),
                       [](const ExtraSource& source) { return source.url; });
    } else {
        LOG_WARNING("Configuration file could not be read, extra sources wont be checked");
    }

    for (const std::string& url : urls) {
        isAccessed = tryAccessUrl(url);
        logUrlAccess(url, isAccessed);
    }
}

void deinitSoftware() {
    bool status;
    RgcConfig config;

    status = readConfig(config);

    if (!status) {
        LOG_ERROR(SOFTWARE_DEINIT_FAIL_MSG);
        exit(1);
    }

    try {
        fs::remove(gkConfigPath);

        fs::remove_all(config.dlcRootPath);
        fs::remove_all(config.v2ipRootPath);
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR(SOFTWARE_DEINIT_FAIL_MSG);
        exit(1);
    }

    LOG_INFO("Deinitalization successfully completed, all components deleted");
}

void initSoftware() {
    bool status;
    std::string apiToken;
    RgcConfig config;

    // Create dirs for DLC toolchain deploy
    try {
        fs::create_directories(gkDlcToolchainDir);
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR(SOFTWARE_INIT_FAIL_MSG);
        exit(1);
    }

    // Create dirs for V2IP toolchain deploy
    try {
        fs::create_directories(gkV2ipToolchainDir);
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR(SOFTWARE_INIT_FAIL_MSG);
        exit(1);
    }

    // SECTION - Download DLC
    auto dlcArchivePath = downloadDlcSourceCode();
    VALIDATE_INIT_PART_RESULT(dlcArchivePath);

    auto dlcRootPath = extractTarGz(*dlcArchivePath, gkDlcToolchainDir);
    VALIDATE_INIT_PART_RESULT(dlcRootPath);

    status = clearDlcDataSection(*dlcRootPath);
    VALIDATE_INIT_PART_RESULT(status);

    fs::remove(*dlcArchivePath);
    // !SECTION

    // SECTION - Download V2IP
    auto v2ipArchivePath = downloadV2ipSourceCode();
    VALIDATE_INIT_PART_RESULT(v2ipArchivePath);

    auto v2ipRootPath = extractTarGz(*v2ipArchivePath, gkV2ipToolchainDir);
    VALIDATE_INIT_PART_RESULT(v2ipRootPath);

    fs::remove(*v2ipArchivePath);
    // !SECTION

    // SECTION - Get user's GitHub API token
    getStringInput("Specify your GitHub API token for requests (may be left empty)", config.apiToken, true);
    // !SECTION

    // SECTION - Create config
    config.dlcRootPath = std::move(*dlcRootPath);
    config.v2ipRootPath = std::move(*v2ipRootPath);

    config.refilterTime = CFG_DEFAULT_NUM_VALUE;
    config.v2rayTime = CFG_DEFAULT_NUM_VALUE;
    config.ruadlistTime = CFG_DEFAULT_NUM_VALUE;
    config.apiToken = apiToken;

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

std::tuple<bool, bool> checkForUpdates(const RgcConfig& config) {
    bool status;
    bool isUpdateFound;
    Json::Value value;
    std::string ruadlistVersion;
    std::string logMsg;
    std::string gitHttpHeader;
    std::optional<std::time_t> lastReleaseTime;

    gitHttpHeader = "Authorization: token " + config.apiToken;

    // SECTION - Check ReFilter for updates
    if (!config.apiToken.empty()) {
        status = tryDownloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME, gitHttpHeader.c_str());
    } else {
        status = tryDownloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);
    }

    if (!status) {
        LOG_ERROR("Failed to fetch updates for ReFilter lists");
        return std::make_tuple(false, false);
    }

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
        status = tryDownloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME, gitHttpHeader.c_str());
    } else {
        status = tryDownloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME);
    }

    if (!status) {
        LOG_ERROR("Failed to fetch updates for XRay lists");
        return std::make_tuple(false, false);
    }

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
    status = tryDownloadFile(RUADLIST_API_MASTER_URL, RUADLIST_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to fetch updates for RuAdList");
        return std::make_tuple(false, false);
    }

    status = readJsonFromFile(RUADLIST_FILE_NAME, value);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    status = parseRuadlistUpdateDatetime(value, *lastReleaseTime);
    VALIDATE_CHECK_UPDATES_PART_RESULT(status);

    isUpdateFound = config.ruadlistTime < lastReleaseTime;

    if (isUpdateFound) {
        logMsg = "An update to the RUADLIST has been detected: ";
        logMsg += parseUnixTime(*lastReleaseTime);
        logMsg += " vs ";
        logMsg += parseUnixTime(config.ruadlistTime);

        LOG_WARNING(logMsg);

        return std::make_tuple(status, isUpdateFound);
    }
    // !SECTION

    return std::make_tuple(status, isUpdateFound);
}

bool downloadNewestSources(RgcConfig& config, bool useExtraSources, std::vector<DownloadedSourcePair>& downloadedFiles) {
    bool status;
    Json::Value value;
    std::optional<std::time_t> lastReleaseTime;
    std::string latestRuadlistVersion;
    std::vector<std::string> assetsNames;
    size_t dupeCnt;

    const fs::path kCurrentDir = fs::current_path();

    // SECTION - Download newest ReFilter rules
    status = tryDownloadFile(REFILTER_API_LAST_RELEASE_URL, REFILTER_RELEASE_REQ_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to download ReFilter lists API response");
        return false;
    }

    status = readJsonFromFile(REFILTER_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.refilterTime = *lastReleaseTime;

    assetsNames = {REFILTER_DOMAIN_ASSET_FILE_NAME, REFILTER_IP_ASSET_FILE_NAME};
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[0]));
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::IP, REFILTER_SECTION_NAME), kCurrentDir / assetsNames[1]));

    LOG_INFO("ReFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest XRay rules    
    status = tryDownloadFile(XRAY_RULES_API_LAST_RELEASE_URL, XRAY_RULES_RELEASE_REQ_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to download XRay lists API response");
        return false;
    }

    status = readJsonFromFile(XRAY_RULES_RELEASE_REQ_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    lastReleaseTime = parsePublishTime(value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(lastReleaseTime);
    config.v2rayTime = *lastReleaseTime;

    assetsNames = {"reject-list.txt"};
    status = downloadGithubReleaseAssets(value, assetsNames);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, XRAY_REJECT_SECTION_NAME), kCurrentDir / assetsNames[0]));

    LOG_INFO("XRay rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest RUADLIST rules    
    status = tryDownloadFile(RUADLIST_API_MASTER_URL, RUADLIST_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList API response");
        return false;
    }

    status = readJsonFromFile(RUADLIST_FILE_NAME, value);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = parseRuadlistUpdateDatetime(value, *lastReleaseTime);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    status = tryDownloadFile(RUADLIST_ADSERVERS_URL, RUADLIST_FILE_NAME);

    if (!status) {
        LOG_ERROR("Failed to download RuAdList adservers");
        return false;
    }

    status = extractDomainsFromFile(RUADLIST_FILE_NAME, RUADLIST_EXTRACTED_FILE_NAME);
    VALIDATE_DOWNLOAD_UPDATES_PART_RESULT(status);

    removeDuplicateLines(RUADLIST_EXTRACTED_FILE_NAME, assetsNames[0]); // RuAdList vs reject-list.txt from XRay

    config.ruadlistTime = *lastReleaseTime;
    downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::DOMAIN, RUADLIST_SECTION_NAME), kCurrentDir / RUADLIST_EXTRACTED_FILE_NAME));

    LOG_INFO("RuAdList rules were downloaded successfully");
    // !SECTION

    // SECTION - Download newest ANTIFILTER rules
    try {
        tryDownloadFile(ANTIFILTER_AYN_IPS_URL, ANTIFILTER_FILE_NAME);

        dupeCnt = removeDuplicateLines(REFILTER_IP_ASSET_FILE_NAME, ANTIFILTER_FILE_NAME);

        if (dupeCnt) {
            LOG_INFO("Count of duplicates found between REFILTER and ANTIFILTER: " + std::to_string(dupeCnt));
        } else {
            LOG_INFO("No duplicates were found between REFILTER AND ANTIFILTER");
        }

        downloadedFiles.push_back(DownloadedSourcePair(Source(Source::Type::IP, ANTIFILTER_SECTION_NAME), kCurrentDir / ANTIFILTER_FILE_NAME));
    }  catch (std::exception& e) {
        LOG_ERROR(e.what());
        LOG_WARNING("Failed add Antifilter rules to Geolists");
    }

    LOG_INFO("AntiFilter rules were downloaded successfully");
    // !SECTION

    // SECTION - Download extra sources
    std::string fileName;

    for (const auto& source : config.extraSources) {
        status = tryDownloadFile(source.url, fileName);

        if (!status) {
            LOG_WARNING("Failed to downloaded extra source: " + source.url);
            continue;
        }

        downloadedFiles.push_back(DownloadedSourcePair(Source(source.type, source.section), kCurrentDir / fileName));
        LOG_INFO("Extra source was downloaded successfully: " + source.url);
    }
    // !SECTION

    return status;
}


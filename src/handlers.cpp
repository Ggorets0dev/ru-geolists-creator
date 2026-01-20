#include "handlers.hpp"
#include "time_tools.hpp"
#include "ruadlist.hpp"
#include "archive.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "url_handle.hpp"
#include "geo_manager.hpp"
#include "fs_utils_temp.hpp"

#include <string>

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

void printHelp(const CLI::App& app) {
    std::cout << app.help() << std::endl; // Standard help
    // TODO: Add some other info for usage
}

void showPresets(const CmdArgs& args) {
    size_t shownPresetsCount = 0;
    const auto config = getCachedConfig();

    std::cout << "Available presets from configuration: \n" << std::endl;

    for (const auto&[fst, snd] : config->presets) {
        if (!args.presets.empty() && std::find(args.presets.begin(), args.presets.end(), snd.label) == args.presets.end()) {
            // Preset is not requested for check
            continue;
        }

        SourcePreset::SortType type = SourcePreset::SORT_BY_ID;

        if (args.isSortBySections) {
            type = SourcePreset::SORT_BY_SECTION;
        } else if (args.isSortByeInetTypes) {
            type = SourcePreset::SORT_BY_INET_TYPE;
        } else if (args.isSortByStorageTypes) {
            type = SourcePreset::SORT_BY_STORAGE_TYPE;
        }

        snd.print(std::cout, type);
        ++shownPresetsCount;
    }

    if (!shownPresetsCount) {
        LOG_INFO("No presets detected in config, nothing to show");
    }
}

void checkUrlsAccess(const CmdArgs& args) {
    LOG_INFO("Check for all sources's URLs is requested");

    const auto config = getCachedConfig();

    for (const auto& pair : config->presets) {
        if (!args.presets.empty() && std::find(args.presets.begin(), args.presets.end(), pair.second.label) == args.presets.end()) {
            // Preset is not requested for check
            continue;
        }

        for (const auto& sourceId : pair.second.sourceIds) {
            bool isAccessed = false;
            auto sourceObj = config->sources.find(sourceId);

            if (sourceObj == config->sources.end()) {
                continue;
            }

            auto source = sourceObj->second;

            if (source.storageType == Source::GITHUB_RELEASE) {
                if (!source.assets || source.assets->empty()) {
                    isAccessed = false;
                } else {
                    try {
                        isAccessed = NetUtils::tryAccessGithubReleaseAssets(source.url, *source.assets, config->apiToken);
                    } catch (...) {
                        isAccessed = false;
                    }
                }
            } else if (source.storageType == Source::REGULAR_FILE_LOCAL) {
                try {
                    isAccessed = fs::exists(source.url);
                } catch (const fs::filesystem_error& e) {
                    LOG_ERROR("Filesystem error: " + std::string(e.what()));
                    isAccessed = false;
                }
            } else if (source.storageType == Source::REGULAR_FILE_REMOTE) {
                isAccessed = NetUtils::tryAccessUrl(source.url);
            }

            logUrlAccess(source.url, isAccessed);
        }
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
        fs::remove(config.geoMgrBinaryPath);
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR(SOFTWARE_DEINIT_FAIL_MSG);
        exit(1);
    }

    LOG_INFO("Deinitialization successfully completed, all components deleted");
}

void initSoftware() {
    bool status;
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

    // Create dirs for Geo Manager deploy
    try {
        fs::create_directories(gkGeoManagerDir);
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

    fs::path bufferPath = *dlcRootPath;
    bufferPath = bufferPath.parent_path() / DLC_TOOLCHAIN_DIRNAME;

    if (fs::exists(bufferPath)) {
        // If toolchain exists, delete it
        fs::remove_all(bufferPath);
    }

    fs::rename(*dlcRootPath, bufferPath);
    dlcRootPath = bufferPath.string();

    LOG_INFO("DLC toolchain renaming has been completed for improved universality: {}", bufferPath.string());

    status = clearDlcDataSection(*dlcRootPath);
    VALIDATE_INIT_PART_RESULT(status);
    // !SECTION

    // SECTION - Download V2IP
    auto v2ipArchivePath = downloadV2ipSourceCode();
    VALIDATE_INIT_PART_RESULT(v2ipArchivePath);

    auto v2ipRootPath = extractTarGz(*v2ipArchivePath, gkV2ipToolchainDir);
    VALIDATE_INIT_PART_RESULT(v2ipRootPath);

    bufferPath = *v2ipRootPath;
    bufferPath = bufferPath.parent_path() / V2IP_TOOLCHAIN_DIRNAME;

    if (fs::exists(bufferPath)) {
        // If toolchain exists, delete it
        fs::remove_all(bufferPath);
    }

    fs::rename(*v2ipRootPath, bufferPath);
    v2ipRootPath = bufferPath.string();

    LOG_INFO("V2IP toolchain renaming has been completed for improved universality: {}", bufferPath.string());
    // !SECTION

    // SECTION - Download GeoManager
    auto geoManagerPath = setupGeoManagerBinary();
    VALIDATE_INIT_PART_RESULT(geoManagerPath);
    // !SECTION

    // SECTION - Get user's GitHub API token
    getStringInput("Specify your GitHub API token for requests (may be left empty)", config.apiToken, true);
    // !SECTION

    // SECTION - Create config
    config.dlcRootPath = std::move(*dlcRootPath);
    config.v2ipRootPath = std::move(*v2ipRootPath);
    config.geoMgrBinaryPath = std::move(*geoManagerPath);

    status = writeConfig(config);
    VALIDATE_INIT_PART_RESULT(status);
    // !SECTION

    LOG_INFO("Initialization successfully completed, all components installed");
}


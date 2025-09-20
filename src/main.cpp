#include "handlers.hpp"
#include "config.hpp"
#include "temp.hpp"
#include "main_sources.hpp"
#include "cli_args.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "build_tools.hpp"
#include "ipc_chain.hpp"
#include "geo_manager.hpp"

#include <unistd.h>
#include <signal.h>

#define EXIT_WITH_CLEANUP(code) \
    performCleanup();           \
    return code;

static void performCleanup() {
    bool status;

    if (fs::exists(TEMP_DIR_NAME) && fs::is_directory(TEMP_DIR_NAME)) {
        std::error_code ec;

        fs::remove_all(TEMP_DIR_NAME, ec);

        if (ec) {
            LOG_ERROR("Failed to delete TEMP dir before exiting: " + ec.message());
        }
    }

    if (fs::exists(RGC_RELEASE_NOTES_FIFO_PATH)) {
        status = fs::remove(RGC_RELEASE_NOTES_FIFO_PATH);

        if (!status) {
            LOG_ERROR("Failed to delete FIFO for IPC before exiting");
        }
    }
}

int main(int argc, char** argv) {
    CLI::App app;
    int cmdArgsParseStatus;
    bool status;
    RgcConfig config;
    std::vector<std::string> v2ipSections;
    Json::Value v2ipInputRules(Json::arrayValue);
    std::vector<DownloadedSourcePair> downloadedSources;
    std::optional<fs::path> outGeoipPath, outGeositePath;
    GeoReleases releases;

    // Init RAND
    std::srand(std::time(0));

    // Init logging
    initLogging();

    // SECTION - Parse CMD args using CLI11 lib
    prepareCmdArgs(app, argc, argv);

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        printHelp(app);
        return 0;
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }
    // !SECTION

    // Show about and close software
    if (gCmdArgs.isInit) {
        if (fs::exists(gkConfigPath)) {
            bool isInitAgain = askYesNo("Initialization is already performed, delete config and run it again?", false);

            if (isInitAgain) {
                deinitSoftware();
            } else {
                LOG_INFO("Re-initialization canceled");
                return 0;
            }
        }

        LOG_INFO("Launching software initialization...");

        initSoftware(); // Download all toolchains and create config

        EXIT_WITH_CLEANUP(0);
    }

    // Print software information
    if (gCmdArgs.isShowAbout) {
        printSoftwareInfo();
        return 0;
    }

    if (!fs::exists(gkConfigPath)) {
        LOG_WARNING("Configuration file is not found, perform software initialization using --init");
        return 0;
    }

    // Add extra source
    if (gCmdArgs.isAddExtra) {
        addExtraSource();
        return 0;
    }

    // Remove extra source
    if (gRemoveExtraOption->count() == 1) {
        removeExtraSource(gCmdArgs.extraSourceId);
        return 0;
    }

    // Show all extra sources
    if (gCmdArgs.isShowExtras) {
        showExtraSources();
        return 0;
    }

    // Check access for URLs and close software
    if (gCmdArgs.isCheckUrls) {
        checkUrlsAccess();
        return 0;
    }

    // ---> Download sources --->

    status = validateParsedFormats();

    if (!status) {
        LOG_ERROR("Build could not be started because the requested formats do not match the supported formats");
        return 1;
    }

    // Check if out path is set correctly
    if (gOutPathOption->count() == 0) {
        LOG_ERROR("Out path for geolists was not specified");
        return 1;
    }

    status = readConfig(config);
    if (!status) {
        LOG_ERROR(READ_CFG_FAIL_MSG);

        EXIT_WITH_CLEANUP(1);
    }

    CREATE_TEMP_DIR();
    ENTER_TEMP_DIR();
    CLEAR_TEMP_DIR();

    if (!gCmdArgs.isForceCreation) {
        LOG_INFO("Updates will be searched for, and built if available");

        auto [checkStatus, isUpdateFound] = checkForUpdates(config);

        if (!checkStatus) {
            // An additional log can be posted here

            performCleanup();
            return 1; // Failed to check updates. Exit
        } else if (!isUpdateFound) {
            LOG_INFO("No need to update sources, stopping program...");

            if (gCmdArgs.isChild) {
                // Signaling that there's nothing to do
                kill(getppid(), SIGUSR2);
            }

            EXIT_WITH_CLEANUP(0);
        }
    } else {
        LOG_INFO("No check for updates required, forced download and build");
    }

    // SECTION - Download latest available sources
    LOG_INFO("Process of downloading the latest versions of the sources begins...");
    status = downloadNewestSources(config, true, downloadedSources);

    if (!status) {
        LOG_ERROR("Failed to download newest sources to build lists");
        EXIT_WITH_CLEANUP(1);
    }
    // !SECTION

    std::cout << "\nSuccessfully downloaded sources: \n" << std::endl;
    printDownloadedSources(std::cout, downloadedSources);

    // Join similar sources if they exist
    joinSimilarSources(downloadedSources);

    // SECTION - Move sources to toolchains
    clearDlcDataSection(config.dlcRootPath);

    v2ipSections.reserve(downloadedSources.size());

    for (const auto& source : downloadedSources) {
        if (source.first.type == Source::Type::DOMAIN) {
            status &= addDomainSource(config.dlcRootPath, source.second, source.first.section);
        } else { // IP
            addIPSource(source, v2ipInputRules);
            v2ipSections.push_back(source.first.section);
        }
    }

    status &= saveIPSources(config.v2ipRootPath, v2ipInputRules, v2ipSections);

    if (!status) {
        LOG_ERROR("Failed to correctly place sources in toolchains");

        EXIT_WITH_CLEANUP(1);
    }

    LOG_INFO("Successfully deployed source files to toolchain environments");
    // !SECTION

    // SECTION - Run toolchains to create lists
    LOG_INFO("Build process of Domain lists is started using DLC...");
    outGeositePath = runDlcToolchain(config.dlcRootPath);

    LOG_INFO("Build process of IP lists is started using V2IP...");
    outGeoipPath = runV2ipToolchain(config.v2ipRootPath);

    if (!outGeositePath || !outGeoipPath) {
        LOG_ERROR("Building one or more lists failed due to errors within the toolchains");

        EXIT_WITH_CLEANUP(1);
    }
    // !SECTION

    // SECTION - Copy created files to destination

    // Setting paths by default
    try {
        fs::create_directories(gCmdArgs.outDirPath);

        if (IS_FORMAT_REQUESTED(GEO_FORMAT_V2RAY_CAPTION)) {
            // Deploying V2Ray rules in .dat extension
            releases.packs.push_back(
                GeoReleasePack(fs::path(gCmdArgs.outDirPath) / GEOSITE_FILENAME_DAT, fs::path(gCmdArgs.outDirPath) / GEOIP_FILENAME_DAT)
            );

            fs::copy(*outGeositePath, releases.packs[0].listDomain, fs::copy_options::overwrite_existing);
            fs::copy(*outGeoipPath, releases.packs[0].listIP, fs::copy_options::overwrite_existing);
        }

        if (IS_FORMAT_REQUESTED(GEO_FORMAT_SING_CAPTION)) {
            // Deploying Sing rules in .db extension

            std::string singGeositePath = fs::path(gCmdArgs.outDirPath) / GEOSITE_FILENAME_DB;
            std::string singGeoipPath = fs::path(gCmdArgs.outDirPath) / GEOIP_FILENAME_DB;

            status = convertGeolist(config.geoMgrBinaryPath, Source::Type::DOMAIN, GEO_FORMAT_V2RAY_CAPTION, GEO_FORMAT_SING_CAPTION, *outGeositePath, singGeositePath);

            if (!status) {
                auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_SING_CAPTION);
                LOG_WARNING(log);
            }

            status = convertGeolist(config.geoMgrBinaryPath, Source::Type::IP, GEO_FORMAT_V2RAY_CAPTION, GEO_FORMAT_SING_CAPTION, *outGeoipPath, singGeoipPath);

            if (!status) {
                auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_SING_CAPTION);
                LOG_WARNING(log);
            }

            releases.packs.push_back(
                GeoReleasePack(singGeositePath, singGeoipPath)
            );
        }

        releases.releaseNotes = fs::path(gCmdArgs.outDirPath) / RELEASE_NOTES_FILENAME;
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error:" + std::string(e.what()));

        EXIT_WITH_CLEANUP(1);
    }

    try {
        createReleaseNotes(releases, config, downloadedSources);
    } catch (const std::runtime_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR("Failed to create release notes for parent proccess");
    }

    LOG_INFO("Domain address list(s) successfully created");
    LOG_INFO("IP address list(s) successfully created");
    // !SECTION

    performCleanup();

    status = writeConfig(config);

    if (!status) {
        LOG_ERROR(WRITE_CFG_FAIL_MSG);
        return 1;
    } else {
        return 0;
    }
}

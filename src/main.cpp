#include "handlers.hpp"
#include "config.hpp"
#include "temp.hpp"
#include "main_sources.hpp"
#include "cli_args.hpp"
#include "log.hpp"
#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "build_tools.hpp"

static void performCleanup() {
    if (fs::exists(TEMP_DIR_NAME) && fs::is_directory(TEMP_DIR_NAME)) {
        std::error_code ec;

        fs::remove_all(TEMP_DIR_NAME, ec);

        if (ec) {
            LOG_ERROR("Failed to delete TEMP dir before exiting: " + ec.message());
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
    GeoListsPaths release_paths;

    // Init RAND
    std::srand(std::time(0));

    // Init logging
    initLogging();

    // SECTION - Parse CMD args using CLI11 lib
    prepareCmdArgs(app, argc, argv);

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp &e) {
        std::cout << app.help() << std::endl; // Standard help
        std::cout << "Notice: When running without arguments, the update-checked mode is used" << std::endl;
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

        performCleanup();
        return 0;
    }

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

        performCleanup();
        return 0;
    }

    // Remove extra source
    if (gRemoveExtraOption->count() == 1) {
        removeExtraSource(gCmdArgs.extraSourceId);

        performCleanup();
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

    status = readConfig(config);
    if (!status) {
        LOG_ERROR(READ_CFG_FAIL_MSG);

        performCleanup();
        return 1;
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
            LOG_INFO("No need to update sources, exit the program");

            performCleanup();
            return 0;
        }
    } else {
        LOG_INFO("No check for updates required, forced download and build");
    }

    // SECTION - Download latest available sources
    LOG_INFO("Process of downloading the latest versions of the sources begins...");
    status = downloadNewestSources(config, true, downloadedSources);

    if (!status) {
        // An additional log can be posted here

        performCleanup();
        return 1; // Failed to download newest releases. Exit
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

        performCleanup();
        return 1;
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

        performCleanup();
        return 1;
    }
    // !SECTION

    // SECTION - Copy created files to destination

    // Setting paths by default
    release_paths.listDomain = fs::current_path() / OUTPUT_FOLDER_NAME / GEOSITE_FILE_NAME;
    release_paths.listIP = fs::current_path() / OUTPUT_FOLDER_NAME / GEOIP_FILE_NAME;

    try {
        if (!fs::exists(OUTPUT_FOLDER_NAME)) {
            fs::create_directory(OUTPUT_FOLDER_NAME);
        }

        fs::copy(*outGeositePath, release_paths.listDomain, fs::copy_options::overwrite_existing);
        fs::copy(*outGeoipPath, release_paths.listIP, fs::copy_options::overwrite_existing);

    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error:" + std::string(e.what()));

        performCleanup();
        return 1;
    }

    try {
        createReleaseNotes(release_paths, config, downloadedSources);
    } catch (const std::runtime_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR("Failed to create release notes for parent proccess");
    }

    LOG_INFO("Domain address list successfully created:" + release_paths.listDomain.string());
    LOG_INFO("IP address list successfully created:" + release_paths.listIP.string());
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

#include "handlers.hpp"
#include "config.hpp"
#include "temp.hpp"
#include "main_sources.hpp"
#include "cli_args.hpp"

static const fs::path gkGeositeDestPath = fs::current_path() / OUTPUT_FOLDER_NAME / GEOSITE_FILE_NAME;
static const fs::path gkGeoipDestPath   = fs::current_path() / OUTPUT_FOLDER_NAME / GEOIP_FILE_NAME;

static void performCleanup() {
    if (fs::exists(TEMP_DIR_NAME) && fs::is_directory(TEMP_DIR_NAME)) {
        std::error_code ec;  // Для подавления исключений (если не хочется try-catch)

        // Удаляем папку и всё её содержимое
        fs::remove_all(TEMP_DIR_NAME, ec);

        if (ec) {
            LOG_ERROR("Failed to delete TEMP dir before exiting: " + ec.message());
        }
    }
}

int
main(int argc, char** argv) {
    CLI::App app;
    int cmdArgsParseStatus;
    bool status;
    RgcConfig config;
    std::vector<std::string> v2ipSections;
    Json::Value v2ipInputRules(Json::arrayValue);
    std::vector<DownloadedSourcePair> downloadedSources;
    std::optional<fs::path> outGeoipPath, outGeositePath;

    // Init RAND
    std::srand(std::time(0));

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
    if (gCmdArgs.isShowAbout) {
        printSoftwareInfo();
        return 0;
    }

    if (!fs::exists(RGC_CONFIG_PATH)) {
        LOG_WARNING("Configuration file is not detected, initialization is performed");
        initSoftware(); // Download all toolchains and create config

        LOG_INFO("You can add a GitHub API access key before running the software. Restart the application with the token added if desired");

        performCleanup();
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
        LOG_ERROR("Configuration file could not be read, operation cannot be continued");

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

    EXIT_TEMP_DIR();

    LOG_INFO("Successfully downloaded all sources: \n");
    printDownloadedSources(std::cout, downloadedSources);

    writeConfig(config);

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
    try {
        if (!fs::exists(OUTPUT_FOLDER_NAME)) { \
            fs::create_directory(OUTPUT_FOLDER_NAME); \
        }

        fs::copy(*outGeositePath, gkGeositeDestPath, fs::copy_options::overwrite_existing);
        fs::copy(*outGeoipPath, gkGeoipDestPath, fs::copy_options::overwrite_existing);
        createReleaseNotes(config, downloadedSources);
    } catch (const fs::filesystem_error& e) {
        log(LogType::ERROR, "Filesystem error:", e.what());

        performCleanup();
        return 1;
    }

    log(LogType::INFO, "Domain address list successfully created:", gkGeositeDestPath.string());
    log(LogType::INFO, "IP address list successfully created:", gkGeoipDestPath.string());
    // !SECTION

    performCleanup();
    return 0;
}

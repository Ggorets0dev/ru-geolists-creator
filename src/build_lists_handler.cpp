#include "build_lists_handler.hpp"

#include "build_tools.hpp"
#include "log.hpp"
#include "json_io.hpp"
#include "config.hpp"
#include "dlc_toolchain.hpp"
#include "geo_manager.hpp"
#include "handlers.hpp"
#include "libnetwork_settings.hpp"
#include "v2ip_toolchain.hpp"

std::optional<GeoReleases> buildListsHandler(const CmdArgs& args) {
    bool status = validateParsedFormats(args);
    std::vector<DownloadedSourcePair> downloadedSources;
    RgcConfig config;
    std::vector<std::string> v2ipSections;
    Json::Value v2ipInputRules(Json::arrayValue);
    std::optional<fs::path> outGeoipPath, outGeositePath;

    GeoReleases releases = {
        .isEmpty = true
    };

    if (!status) {
        LOG_ERROR("Build could not be started because the requested formats do not match the supported formats");
        return std::nullopt;
    }

    status = readConfig(config);

    if (!status) {
        LOG_ERROR(READ_CFG_FAIL_MSG);
        return std::nullopt;
    }

    // ======== Init network lib settings
    gLibNetworkSettings.isSearchSubnetByBGP = true; // FIXME: Add ability to choose for user
    gLibNetworkSettings.bgpDumpPath = config.bgpDumpPath;
    // ========

    if (!args.isForceCreation) {
        LOG_INFO("Updates will be searched for, and built if available");

        auto [checkStatus, isUpdateFound] = checkForUpdates(config);

        if (!checkStatus) {
            // An additional log can be posted here

            return std::nullopt; // Failed to check updates. Exit
        }

        if (!isUpdateFound) {
            LOG_INFO("No need to build lists, no source update available");

            return releases;
        }
    } else {
        LOG_INFO("No check for updates required, forced download and build");
    }

    // SECTION - Download latest available sources
    LOG_INFO("Process of downloading the latest versions of the sources begins...");

    // TODO: Get extra sources flag from user-config
    status = downloadNewestSources(config, !args.isNoExtra, args.isUseWhitelist, downloadedSources);

    if (!status) {
        LOG_ERROR("Failed to download newest sources to build lists");
        return std::nullopt;
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
        return std::nullopt;
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
        return std::nullopt;
    }
    // !SECTION

    // SECTION - Copy created files to destination

    // Setting paths by default
    try {
        fs::create_directories(args.outDirPath);

        if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_V2RAY_CAPTION)) {
            // Deploying V2Ray rules in .dat extension
            releases.packs.emplace_back(
                fs::path(args.outDirPath) / GEOSITE_FILENAME_DAT,
                fs::path(args.outDirPath) / GEOIP_FILENAME_DAT
            );

            fs::copy(*outGeositePath, releases.packs[0].listDomain, fs::copy_options::overwrite_existing);
            fs::copy(*outGeoipPath, releases.packs[0].listIP, fs::copy_options::overwrite_existing);
        }

        if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_SING_CAPTION)) {
            // Deploying Sing rules in .db extension

            std::string singGeositePath = fs::path(args.outDirPath) / GEOSITE_FILENAME_DB;
            std::string singGeoipPath = fs::path(args.outDirPath) / GEOIP_FILENAME_DB;

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

            releases.packs.emplace_back(singGeositePath, singGeoipPath
            );
        }

        releases.releaseNotes = fs::path(args.outDirPath) / RELEASE_NOTES_FILENAME;
    } catch (const fs::filesystem_error& e) {
        LOG_ERROR("Filesystem error:" + std::string(e.what()));
        return std::nullopt;
    }

    try {
        createReleaseNotes(releases, config, downloadedSources);
    } catch (const std::runtime_error& e) {
        LOG_ERROR(e.what());
        LOG_ERROR("Failed to create release notes for parent process");
    }

    LOG_INFO("Domain address list(s) successfully created");
    LOG_INFO("IP address list(s) successfully created");
    // !SECTION

    status = writeConfig(config);

    if (!status) {
        LOG_ERROR(WRITE_CFG_FAIL_MSG);
        return std::nullopt;
    }

    // Set special flag, mark that job done successfully
    releases.isEmpty = false;

    return releases;
}

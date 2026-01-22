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

    const auto config = getCachedConfig();

    // ======== Init network lib settings
    gLibNetworkSettings.isSearchSubnetByBGP = args.isUseWhitelist;
    gLibNetworkSettings.bgpDumpPath = config->bgpDumpPath;
    // ========

    const auto outDirPath = fs::path(args.outDirPath);
    releases.releaseNotes = outDirPath / RELEASE_NOTES_FILENAME;
    std::ofstream releaseNotesFile(releases.releaseNotes);

    for (const auto& pair : config->presets) {
        if (!args.presets.empty() && std::find(args.presets.begin(), args.presets.end(), pair.second.label) == args.presets.end()) {
            // Preset is not requested for check
            continue;
        }

        const auto& preset = pair.second;
        auto downloads = preset.downloadSources();

        if (!downloads.has_value()) {
            LOG_WARNING("Failed to download all sources for preset with label {}, aborting build", preset.label);
            return std::nullopt;
        }

        // Join similar sources if they exist
        joinSimilarSources(*downloads);

        // SECTION - Move sources to toolchains
        clearDlcDataSection(config->dlcRootPath);

        v2ipSections.reserve(downloads->size());

        for (const auto& sourcePair : *downloads) {
            if (const auto& source = config->sources.at(sourcePair.first); source.inetType == Source::InetType::DOMAIN) {
                status &= addDomainSource(config->dlcRootPath, sourcePair.second, source.section);
            } else { // IP
                addIPSource(sourcePair, v2ipInputRules);
                v2ipSections.push_back(source.section);
            }
        }

        status &= saveIPSources(config->v2ipRootPath, v2ipInputRules, v2ipSections);

        if (!status) {
            LOG_ERROR("Failed to correctly place sources in toolchains");
            return std::nullopt;
        }

        LOG_INFO("Successfully deployed source files to toolchain environments");
        // !SECTION

        // SECTION - Run toolchains to create lists
        LOG_INFO("Build process of Domain lists is started using DLC...");
        outGeositePath = runDlcToolchain(config->dlcRootPath);

        LOG_INFO("Build process of IP lists is started using V2IP...");
        outGeoipPath = runV2ipToolchain(config->v2ipRootPath);

        if (!outGeositePath || !outGeoipPath) {
            LOG_ERROR("Building one or more lists failed due to errors within the toolchains");
            return std::nullopt;
        }
        // !SECTION

        // SECTION - Copy created files to destination

        // Setting paths by default
        try {
            const fs::path targetPath = outDirPath / fmt::format("preset-{}", preset.label);

            fs::create_directories(targetPath);

            // ============
            // Build releases
            // ============
            if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_V2RAY_CAPTION)) {
                // Deploying V2Ray rules in .dat extension
                const std::string geoipFilename = fmt::format("{}-{}.{}",
                    GEOIP_BASE_FILENAME,
                    preset.label,
                    V2RAY_FILES_EXT);

                const std::string geositeFilename = fmt::format("{}-{}.{}",
                    GEOSITE_BASE_FILENAME,
                    preset.label,
                    V2RAY_FILES_EXT);

                releases.packs.emplace_back(
                    targetPath / geoipFilename,
                    targetPath / geositeFilename
                );

                fs::copy(*outGeositePath, releases.packs[0].listDomain, fs::copy_options::overwrite_existing);
                fs::copy(*outGeoipPath, releases.packs[0].listIP, fs::copy_options::overwrite_existing);
            }

            if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_SING_CAPTION)) {
                // Deploying Sing rules in .db extension
                const std::string geoipFilename = fmt::format("{}-{}.{}",
                     GEOIP_BASE_FILENAME,
                     preset.label,
                     SING_FILES_EXT);

                const std::string geositeFilename = fmt::format("{}-{}.{}",
                    GEOSITE_BASE_FILENAME,
                    preset.label,
                    SING_FILES_EXT);

                const fs::path singGeositePath = targetPath / geositeFilename;
                const fs::path singGeoipPath = targetPath / geoipFilename;

                status = convertGeolist(config->geoMgrBinaryPath, Source::InetType::DOMAIN, GEO_FORMAT_V2RAY_CAPTION, GEO_FORMAT_SING_CAPTION, *outGeositePath, singGeositePath);

                if (!status) {
                    auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_SING_CAPTION);
                    LOG_WARNING(log);
                    continue;
                }

                status = convertGeolist(config->geoMgrBinaryPath, Source::InetType::IP, GEO_FORMAT_V2RAY_CAPTION, GEO_FORMAT_SING_CAPTION, *outGeoipPath, singGeoipPath);

                if (!status) {
                    auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_SING_CAPTION);
                    LOG_WARNING(log);
                    continue;
                }

                releases.packs.emplace_back(singGeositePath, singGeoipPath);
            }
            // ============


            // ============
            // Save files if release folder
            // ============
            if (!releases.packs.empty()) {
                const fs::path componentsDirPath = targetPath / "components";
                fs::create_directories(componentsDirPath);

                for (const auto&[id, path] : *downloads) {
                    const auto& source = config->sources.at(id);
                    const std::string filename = source.section + path.extension().string();

                    fs::copy(path, componentsDirPath / filename, fs::copy_options::overwrite_existing);
                    LOG_INFO("Source with ID {} and filename {} copied to components in release folder", id, filename);
                }
            }
            // ============

            addPresetToRelNotes(releaseNotesFile, pair.second);
        } catch (const fs::filesystem_error& e) {
            LOG_ERROR("Filesystem error:" + std::string(e.what()));
            return std::nullopt;
        }

        LOG_INFO("Domain address list(s) successfully created for preset \"{}\"", preset.label);
        LOG_INFO("IP address list(s) successfully created for preset \"{}\"", preset.label);
        // !SECTION

        // SECTION - Add records to release notes
        setBuildInfoToRelNotes(releaseNotesFile);
        // !SECTION
    }

    releaseNotesFile.close();

    // Set special flag, mark that job done successfully
    releases.isEmpty = false;

    return releases;
}

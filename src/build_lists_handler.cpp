#include "build_lists_handler.hpp"

#include <netdb.h>
#include <sys/stat.h>

#include "archive.hpp"
#include "build_tools.hpp"
#include "log.hpp"
#include "json_io.hpp"
#include "config.hpp"
#include "dlc_toolchain.hpp"
#include "filter.hpp"
#include "geo_manager.hpp"
#include "handlers.hpp"
#include "libnetwork_settings.hpp"
#include "sing_box.hpp"
#include "v2ip_toolchain.hpp"
#include "time_tools.hpp"

std::optional<GeoReleases> buildListsHandler(const CmdArgs& args) {
    bool status = validateParsedFormats(args);
    std::optional<fs::path> outGeoipPath, outGeositePath;
    size_t builtPresetsCount = 0;
    std::forward_list<SourcePreset> reqPresets = {};
    size_t reqPresetsCount = 0;
    BuildStats buildStats = {0};

    GeoReleases releases = {
        .isEmpty = true
    };

    if (!status) {
        LOG_ERROR("Build could not be started because the requested formats do not match the supported formats");
        return std::nullopt;
    }

    LOG_INFO("Lists building process is started");

    const auto config = getCachedConfig();

    // ======== Init network lib settings
    gLibNetworkSettings.isSearchSubnetByBGP = args.isUseWhitelist;
    gLibNetworkSettings.bgpDumpPath = config->bgpDumpPath;
    // ========

    const auto outDirPath = fs::path(args.outDirPath);

    if (!isDirEmpty(outDirPath, true)) {
        LOG_ERROR("Specified output path ({}) is not empty, files may be overwritten", outDirPath.string());
        return std::nullopt;
    }

    fs::create_directories(outDirPath);

    releases.releaseNotes = outDirPath / RELEASE_NOTES_FILENAME;
    std::ofstream releaseNotesFile(releases.releaseNotes);

    // ================
    // Prepare presets (create grouped and etc)
    // ================
    for (const auto& pair : config->presets) {
        const bool isFound = args.presets.empty() || std::find(args.presets.begin(), args.presets.end(), pair.second.label) != args.presets.end();

        if (!isFound) {
            // Preset is not requested for build
            continue;
        }

        if (args.isUseGrouping) {
            auto groupingPreset = pair.second;
            groupingPreset.isGrouped = true;
            reqPresets.push_front(groupingPreset);
        } else {
            reqPresets.push_front(pair.second);
        }

        ++reqPresetsCount;
    }
    // ================

    if (!reqPresetsCount) {
        LOG_INFO("No presets found in config file for build");
        return std::nullopt;
    }

    // ================
    // Process requested presets
    // ================
    for (const auto& preset : reqPresets) {
        // ============ V2IP toolchain vars
        std::vector<std::string> v2ipSections;
        Json::Value v2ipInputRules(Json::arrayValue);
        // ============

        auto sourcesStorage = config->sources;
        auto downloads = preset.downloadSources();

        LOG_INFO("Build of preset \"{}\" is requested and started", preset.label);

        if (!downloads.has_value()) {
            LOG_WARNING("Failed to download all sources for preset  \"{}\", aborting it's building", preset.label);
            continue;
        }

        // Apply preprocessing
        for (const auto&[fst, snd] : *downloads) {
            const auto& src = sourcesStorage.at(fst);

            status = true;
            if (src.preprocType == Source::EXTRACT_DOMAINS) {
                status = extractDomainsInPlace(snd);
            }

            if (!status) {
                break;
            }
        }

        if (!status) {
            LOG_ERROR("Failed to perform preprocessing for source ID {} while building preset \"{}\", aborting preset");
            continue;
        }

        if (preset.isGrouped) {
            groupSourcesByInetType(*downloads, sourcesStorage);
        } else if (preset.isGroupRequested(sourcesStorage)) {
            groupSourcesByGroups(*downloads, sourcesStorage);
        } else {
            // Join similar sources if they exist
            groupSourcesBySections(*downloads);

            if (args.isUseWhitelist) {
                filterDownloadsByWhitelist(*downloads);
            }
        }

        // SECTION - Preprocessing for removing duplicates
        for (const auto& pair : *downloads) {
            const auto& source = sourcesStorage.at(pair.first);
            const auto& path = pair.second.string();
            try {
                const size_t removedCount = removeDuplicateLines(path);
                LOG_INFO("Count of removed duplicates (id: {}, file: {}): {}", source.id, path, removedCount);
            } catch (std::ios_base::failure& e) {
                LOG_WARNING("Failed to remove duplicates (id: {}, file: {}): {}", source.id, path, std::string(e.what()));
            }
        }

        // SECTION - Move sources to toolchains
        clearDlcDataSection(config->dlcRootPath);
        v2ipSections.reserve(downloads->size());

        size_t ipSrcAdded = 0;
        size_t domainSrcAdded = 0;

        for (const auto& sourcePair : *downloads) {
            if (const auto& source = sourcesStorage.at(sourcePair.first); source.inetType == Source::InetType::DOMAIN) {
                status &= addDomainSource(config->dlcRootPath, sourcePair.second, source.section);
                ++domainSrcAdded;
            } else { // IP
                addIPSource(sourcePair, v2ipInputRules, sourcesStorage);
                v2ipSections.push_back(source.section);
                ++ipSrcAdded;
            }
        }

        if (ipSrcAdded) {
            // If any IP source added, prepare V2IP
            status &= saveIPSources(config->v2ipRootPath, v2ipInputRules, v2ipSections);
        }

        if (!status) {
            LOG_ERROR("Failed to correctly place sources in toolchains");
            return std::nullopt;
        }

        LOG_INFO("Successfully deployed source files to toolchain environments");
        // !SECTION

        // SECTION - Run toolchains to create lists
        LOG_INFO("Build process of Domain lists is started using DLC...");
        outGeositePath = domainSrcAdded ? runDlcToolchain(config->dlcRootPath) : std::nullopt;

        LOG_INFO("Build process of IP lists is started using V2IP...");
        outGeoipPath = ipSrcAdded ? runV2ipToolchain(config->v2ipRootPath) : std::nullopt;

        if ((domainSrcAdded && !outGeositePath) || (ipSrcAdded && !outGeoipPath)) {
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
            if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_DAT_CAPTION)) {
                // Deploying V2Ray rules in .dat extension
                const std::string geoipFilename = fmt::format("{}-{}.{}",
                    GEOIP_BASE_FILENAME,
                    preset.label,
                    V2RAY_FILES_EXT);

                const std::string geositeFilename = fmt::format("{}-{}.{}",
                    GEOSITE_BASE_FILENAME,
                    preset.label,
                    V2RAY_FILES_EXT);

                GeoReleasePack pack;

                pack.presetLabel = preset.label;

                if (outGeositePath.has_value()) {
                    pack.listDomain = targetPath / geositeFilename;
                    fs::copy(*outGeositePath, *pack.listDomain, fs::copy_options::overwrite_existing);
                }
                if (outGeoipPath.has_value()) {
                    pack.listIP = targetPath / geoipFilename;
                    fs::copy(*outGeoipPath, *pack.listIP, fs::copy_options::overwrite_existing);
                }

                releases.packs.push_back(std::move(pack));
            }

            if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_DB_CAPTION)) {
                // Deploying Sing rules in .db extension
                const std::string geoipFilename = fmt::format("{}-{}.{}",
                     GEOIP_BASE_FILENAME,
                     preset.label,
                     SING_DB_FILES_EXT);

                const std::string geositeFilename = fmt::format("{}-{}.{}",
                    GEOSITE_BASE_FILENAME,
                    preset.label,
                    SING_DB_FILES_EXT);

                GeoReleasePack pack;
                const fs::path singGeositePath = targetPath / geositeFilename;
                const fs::path singGeoipPath = targetPath / geoipFilename;

                pack.presetLabel = preset.label;

                if (outGeositePath.has_value()) {
                    status = convertGeolist(config->geoMgrBinaryPath,
                        Source::InetType::DOMAIN,
                        GEO_FORMAT_DAT_CAPTION,
                        GEO_FORMAT_DB_CAPTION,
                        *outGeositePath,
                        singGeositePath);

                    if (!status) {
                        auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_DB_CAPTION);
                        LOG_WARNING(log);
                        continue;
                    }

                    pack.listDomain = singGeositePath;
                }

                if (outGeoipPath.has_value()) {
                    status = convertGeolist(config->geoMgrBinaryPath,
                        Source::InetType::IP,
                        GEO_FORMAT_DAT_CAPTION,
                        GEO_FORMAT_DB_CAPTION,
                        *outGeoipPath,
                        singGeoipPath);

                    if (!status) {
                        auto log = std::string(GEO_FORMAT_CONVERT_FAIL_MSG) + std::string(GEO_FORMAT_DB_CAPTION);
                        LOG_WARNING(log);
                        continue;
                    }

                    pack.listIP = singGeoipPath;
                }

                releases.packs.push_back(std::move(pack));
            }

            if (IS_FORMAT_REQUESTED(args, GEO_FORMAT_SRS_CAPTION)) {
                if (auto jsonRulesets = generateSingBoxRuleSets(*downloads, sourcesStorage)) {
                    if (!config->singBoxBinaryPath.empty()) {
                        if (const auto srsRulesets = compileSingBoxRuleSets(config->singBoxBinaryPath, targetPath, *jsonRulesets)) {
                            GeoReleasePack pack;
                            pack.presetLabel = preset.label;
                            pack.listsRuleSet = srsRulesets;
                            releases.packs.push_back(std::move(pack));
                        } else {
                            LOG_WARNING("Failed to compile JSON file to SRS");
                        }
                    } else {
                        LOG_WARNING("Impossible to create SRS file because path to sing-box binary is not specified in config");
                    }
                } else {
                    LOG_ERROR("Failed to create RuleSet in JSON format before compilation");
                }
            }
            // ============

            // ============
            // Save files if release folder
            // ============
            if (!releases.packs.empty()) {
                const fs::path componentsDirPath = targetPath / "components";
                fs::create_directories(componentsDirPath);

                for (const auto&[id, path] : *downloads) {
                    const auto& source = sourcesStorage.at(id);

                    if (source.inetType == Source::IP) {
                        buildStats.subnetsCount += countLinesInFile(path);
                        ++buildStats.subnetsFilesCount;
                    } else {
                        buildStats.domainsCount += countLinesInFile(path);
                        ++buildStats.domainsFilesCount;
                    }

                    const std::string filename = fmt::format("{}-{}{}",
                        source.section,
                        sourceInetTypeToString(source.inetType),
                        path.extension().string());

                    fs::copy(path, componentsDirPath / filename, fs::copy_options::overwrite_existing);
                    LOG_INFO("Source with ID {} and filename {} copied to components in release folder", id, filename);
                }

                addPresetToRelNotes(releaseNotesFile, preset);
            }
            // ============
        } catch (const fs::filesystem_error& e) {
            LOG_ERROR("Filesystem error:" + std::string(e.what()));
            releaseNotesFile.close();
            return std::nullopt;
        }

        LOG_INFO("Domain address list(s) successfully created for preset \"{}\"", preset.label);
        LOG_INFO("IP address list(s) successfully created for preset \"{}\"", preset.label);
        // !SECTION

        ++builtPresetsCount;
    }
    // ========

    // ============
    // Filling build stats
    // ============
    buildStats.formats = args.formats;
    // ============

    if (!builtPresetsCount) {
        LOG_ERROR("Failed to build any preset, release notes are empty");
        releaseNotesFile.close();
        return std::nullopt;
    }

    // SECTION - Add records to release notes
    setBuildInfoToRelNotes(releaseNotesFile, buildStats);
    // !SECTION

    releaseNotesFile.close();
    LOG_INFO("Release notes file is saved at path: {}", releases.releaseNotes.string());

    // SECTION - Creating archive for deploy
    const auto archiveName = fmt::format("rglc_geofiles_release_{}", getCurrentUnixTimestamp());
    createZipArchive(outDirPath, archiveName);
    // !SECTION

    // Set special flag, mark that job done successfully
    releases.isEmpty = false;

    return releases;
}

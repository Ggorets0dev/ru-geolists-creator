#include "main_sources.hpp"
#include "cli_draw.hpp"

#include <algorithm>
#include <set>

#include "config.hpp"
#include "filter.hpp"
#include "fs_utils_temp.hpp"
#include "log.hpp"
#include "url_handle.hpp"

static SourceObjectId getMetaSourceId(SourcesStorage& storage) {
    for (SourceObjectId i = 1; i < std::numeric_limits<SourceObjectId>::max(); i++) {
        if (storage.find(i) == storage.end()) {
            return i;
        }
    }

    return 0;
}

Source::Source(const Json::Value& value) {
    this->id = JsonValidator::getRequired<int>(value, "id");
    this->section = JsonValidator::getRequired<std::string>(value, "section");
    this->url = JsonValidator::getRequired<std::string>(value, "url");
    this->storageType = sourceStringToStorageType(JsonValidator::getRequired<std::string>(value, "storage_type"));
    this->inetType = sourceStringToInetType(JsonValidator::getRequired<std::string>(value, "inet_type"));
    this->group = JsonValidator::getOptional<std::string>(value, "group");

    auto preprocTypeStr = JsonValidator::getOptional<std::string>(value, "preproc_type");
    this->preprocType = preprocTypeStr.has_value() ? sourceStringToPreprocType(preprocTypeStr.value()) : PREPROCESSING_TYPE_UNKNOWN;

    // GITHUB release requires extra fields
    if (this->storageType != GITHUB_RELEASE) {
        this->assets = std::nullopt;
        return;
    }

    this->assets = JsonValidator::getRequiredArray<std::string>(value, "assets");
}

std::string sourceInetTypeToString(const Source::InetType type) {
    switch(type) {
    case Source::InetType::IP:
        return "ip";
    case Source::InetType::DOMAIN:
        return "domain";
    default:
        return "NONE";
    }
}

Source::InetType sourceStringToInetType(const std::string_view str) {
    if (str == "ip") {
        return Source::InetType::IP;
    }

    if (str == "domain") { //  domain
        return Source::InetType::DOMAIN;
    }

    return Source::InetType::INET_TYPE_UNKNOWN;
}

Source::PreprocessingType sourceStringToPreprocType(const std::string_view str) {
    if (str == "extract_domains") {
        return Source::PreprocessingType::EXTRACT_DOMAINS;
    }

    return Source::PreprocessingType::PREPROCESSING_TYPE_UNKNOWN;
}

std::string sourceStorageTypeToString(const Source::StorageType type) {
    switch(type) {
        case Source::StorageType::REGULAR_FILE_LOCAL:
            return "file_loc";
        case Source::StorageType::REGULAR_FILE_REMOTE:
            return "file_remote";
        case Source::StorageType::GITHUB_RELEASE:
            return "github_release";
        default:
            return "NONE";
    }
}

Source::StorageType sourceStringToStorageType(const std::string_view str) {
    if (str == "file_loc") {
        return Source::StorageType::REGULAR_FILE_LOCAL;
    }

    if (str == "file_remote") {
        return Source::StorageType::REGULAR_FILE_REMOTE;
    }

    if (str == "github_release") {
        return Source::StorageType::GITHUB_RELEASE;
    }

    return Source::StorageType::STORAGE_TYPE_UNKNOWN;
}

void groupSourcesByGroups(std::vector<DownloadedSourcePair>& sources, SourcesStorage& sourcesStorage) {
    if (sources.empty()) return;

    std::vector<DownloadedSourcePair> groupedSources;
    std::vector<DownloadedSourcePair> metaSources;

    const FS::Utils::Temp::SessionTempFileRegistry registry;

    for (const auto& pair : sources) {
        const Source& source = sourcesStorage.at(pair.first);
        if (!source.group) {
            // No grup, just add and conitnue
            groupedSources.push_back(pair);
            continue;
        }

        // Group is defined, create meta source or add source to existing
        auto it = std::find_if(metaSources.begin(), metaSources.end(),
                [&source, &sourcesStorage](const auto& innerPair) {
                    const Source& innerSource = sourcesStorage.at(innerPair.first);
                    return source.group == innerSource.section && source.inetType == innerSource.inetType; // 'first' is SourceObjectId
                });

        if (it == metaSources.end()) {
            // Meta source with such group does not exist, need creation
            Source metaSource(getMetaSourceId(sourcesStorage), source.inetType, *source.group);
            const auto path = registry.createTempFileDetached("lst")->path;
            sourcesStorage.emplace(metaSource.id, metaSource);
            metaSources.emplace_back(metaSource.id, path);

            it = std::prev(metaSources.end());
        }

        joinTwoFiles(it->second, pair.second);
    }

    sources.clear();
    sources.insert(sources.end(), groupedSources.begin(), groupedSources.end());
    sources.insert(sources.end(), metaSources.begin(), metaSources.end());
}

void groupSourcesByInetType(std::vector<DownloadedSourcePair>& sources,
                                                         SourcesStorage& sourcesStorage) {
    if (sources.empty()) return;

    // ===============
    // Create meta sources
    // ===============
    std::vector<DownloadedSourcePair> metaSources;
    metaSources.reserve(2);

    const FS::Utils::Temp::SessionTempFileRegistry registry;
    const auto domainsFilePath = registry.createTempFileDetached("lst")->path;
    const auto IpsFilePath = registry.createTempFileDetached("lst")->path;

    Source metaJoinedDomains(getMetaSourceId(sourcesStorage),
        Source::InetType::DOMAIN,
        "rglc");

    sourcesStorage.emplace(metaJoinedDomains.id, metaJoinedDomains);
    createEmptyFile(domainsFilePath);

    Source metaJoinedIPs(getMetaSourceId(sourcesStorage),
        Source::InetType::IP,
        "rglc");

    sourcesStorage.emplace(metaJoinedIPs.id, metaJoinedIPs);
    createEmptyFile(IpsFilePath);
    // ===============

    size_t domainSourcesCount = 0;
    size_t ipSourcesCount = 0;

    for (const auto& [sourceId, filePath] : sources) {
        const auto& sourceConfig = sourcesStorage.at(sourceId);

        if (sourceConfig.inetType == Source::InetType::DOMAIN) {
            joinTwoFiles(domainsFilePath, filePath);
            ++domainSourcesCount;
        } else { // Source::InetType::IP
            joinTwoFiles(IpsFilePath, filePath);
            ++ipSourcesCount;
        }
    }

    if (domainSourcesCount) {
        metaSources.emplace_back(metaJoinedDomains.id, domainsFilePath);
    } else {
        fs::remove(domainsFilePath);
    }

    if (ipSourcesCount) {
        metaSources.emplace_back(metaJoinedIPs.id, IpsFilePath);
    } else {
        fs::remove(IpsFilePath);
    }

    sources = std::move(metaSources);
}

void groupSourcesBySections(std::vector<DownloadedSourcePair>& sources) {
    std::vector<bool> removeMarkers(sources.size());
    const auto config = getCachedConfig();

    for (size_t i(0); i < sources.size() - 1; ++i) {
        const auto& parentSource = config->sources.at(sources[i].first);

        if (removeMarkers[i]) {
            // Source is already joined
            continue;
        }

        for (size_t j(i + 1); j < sources.size(); ++j) {
            if (const auto& childSource = config->sources.at(sources[j].first); parentSource.section != childSource.section || parentSource.inetType != childSource.inetType || removeMarkers[j]) {
                // Sources cant be joined
                continue;
            }

            joinTwoFiles(sources[i].second, sources[j].second);
            removeMarkers[j] = true;
        }
    }

    size_t index(0);
    auto removeIter = std::remove_if(sources.begin(), sources.end(), [&index, &removeMarkers](const auto& pair) {
        ++index;
        return removeMarkers[index - 1];
    });

    sources.erase(removeIter, sources.end());
}

std::optional<std::vector<DownloadedSourcePair>> SourcePreset::downloadSources() const {
    std::vector<DownloadedSourcePair> downloads;
    const auto config = getCachedConfig();

    for (const auto& id : sourceIds) {
        const auto source = config->sources.at(id);

        if (const bool status = source.getData(downloads); !status) {
            LOG_WARNING("Failed to fully load the preset with label \"{}\"", this->label);
            return std::nullopt;
        }
    }

    LOG_INFO("Sources from preset \"{}\" are collected", this->label);
    return downloads;
}

bool Source::getData(std::vector<DownloadedSourcePair>& downloads) const {
    if (this->storageType == REGULAR_FILE_LOCAL) {
        if (!fs::exists(this->url)) {
            LOG_WARNING("Failed to get data from local source ID {}", this->id);
            return false;
        }

        downloads.emplace_back(this->id, this->url);
    } else if (this->storageType == REGULAR_FILE_REMOTE) {
        const FS::Utils::Temp::SessionTempFileRegistry registry;
        const auto file = registry.createTempFileDetached("lst");

        if (const bool status = NetUtils::tryDownloadFile(this->url, file->path, nullptr); !status) {
            LOG_WARNING("Failed to get data from remote source ID {}", this->id);
            return false;
        }

        downloads.emplace_back(this->id, file->path);
        LOG_INFO("Source with ID {} and URL {} was downloaded as regular file", this->id, this->url);
    } else if (this->storageType == GITHUB_RELEASE) {
        if (!this->assets.has_value()) {
            LOG_WARNING("Failed to get data from remote GitHub source ID {}", this->id);
            return false;
        }

        const auto downloadsGithub = NetUtils::downloadGithubReleaseAssets(this->url, *this->assets, FS::Utils::Temp::getSessionTempDir(), getCachedConfig()->apiToken);

        if (downloadsGithub.size() != this->assets->size()) {
            LOG_WARNING("Failed to get data from remote GitHub source ID {}", this->id);
            return false;
        }

        std::for_each(downloadsGithub.begin(), downloadsGithub.end(), [&](const auto& path) {
            downloads.emplace_back(this->id, path);
            LOG_INFO("Asset {} of source with ID {} and URL {} was downloaded as regular file", path, this->id, this->url);
        });
    } else {
        return false;
    }

    LOG_INFO("Source with ID {} is collected", this->id);

    return true;
}

bool SourcePreset::isGroupRequested(const SourcesStorage& storage) const {
    bool isRequested = false;

    std::for_each(sourceIds.begin(), sourceIds.end(), [&isRequested, &storage](auto& id) {
        const auto& source = storage.at(id);
        isRequested |= source.group != std::nullopt;
    });

    return isRequested;
}

void SourcePreset::print(std::ostream& stream, const SortType sortType) const {
    const auto config = getCachedConfig();
    TablePrinter table({"ID", "Group/Section", "Storage", "Inet", "URL (Sections)"});

    stream << "PRESET: " << this->label;
    if (isGrouped) {
        stream << " [GROUPED]\n";
        table.addRow({"N/A", "rglc (domain)", "N/A", "domain", "N/A"});
        table.addRow({"N/A", "rglc (ip)", "N/A", "ip", "N/A"});
        table.print(stream);
        stream << "\n";
        return;
    }
    stream << "\n";

    std::vector<SourceObjectId> sourceIdsSorted(sourceIds.begin(), sourceIds.end());
    // ... (Keep your existing std::sort logic here) ...

    struct GroupData {
        std::string name;
        std::string sections;
        Source::StorageType firstStorage;
        Source::InetType inetType; // Track the inet type for this specific row
        bool mixedStorage = false;
        const Source* primary = nullptr;
    };

    std::vector<GroupData> displayRows;
    // Map key is now a pair: {Group Name, Inet Type}
    std::map<std::pair<std::string, Source::InetType>, size_t> groupMap;

    for (const auto& sid : sourceIdsSorted) {
        const Source& src = config->sources.at(sid);

        if (src.group.has_value()) {
            const std::string& gName = src.group.value();
            auto key = std::make_pair(gName, src.inetType);
            auto it = groupMap.find(key);

            if (it != groupMap.end()) {
                GroupData& data = displayRows[it->second];
                if (data.sections.find(src.section) == std::string::npos) {
                    data.sections += ", " + src.section;
                }

                if (src.storageType != data.firstStorage) {
                    data.mixedStorage = true;
                }
                continue;
            }

            // New row for this specific Group + InetType combination
            groupMap[key] = displayRows.size();
            displayRows.push_back({gName, src.section, src.storageType, src.inetType, false, &src});
        } else {
            // Standalone source - no group, always a unique row
            displayRows.push_back({src.section, src.section, src.storageType, src.inetType, false, &src});
        }
    }

    auto truncate = [](std::string s, const size_t width) -> std::string {
        if (s.length() > width) return s.substr(0, width - 3) + "...";
        return s;
    };

    for (const auto& row : displayRows) {
        std::string storageStr = row.mixedStorage
            ? "MIXED"
            : sourceStorageTypeToString(row.firstStorage);

        table.addRow({
            row.primary->group.has_value() ? "GRP" : std::to_string(row.primary->id),
            truncate(row.name, 20),
            truncate(storageStr, 18),
            truncate(sourceInetTypeToString(row.inetType), 8), // Uses row's specific inet type
            truncate(row.sections, 70)
        });
    }

    table.print(stream);
    stream << "\n";
}
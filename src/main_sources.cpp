#include "main_sources.hpp"
#include "cli_draw.hpp"

#include <algorithm>

#include "config.hpp"
#include "fs_utils_temp.hpp"
#include "log.hpp"
#include "url_handle.hpp"

Source::Source(const Json::Value& value) {
    this->id = JsonValidator::getRequired<int>(value, "id");
    this->section = JsonValidator::getRequired<std::string>(value, "section");
    this->url = JsonValidator::getRequired<std::string>(value, "url");
    this->storageType = sourceStringToStorageType(JsonValidator::getRequired<std::string>(value, "storage_type"));
    this->inetType = sourceStringToInetType(JsonValidator::getRequired<std::string>(value, "inet_type"));

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

void joinSimilarSources(std::vector<DownloadedSourcePair>& sources) {
    std::vector<bool> removeMarkers(sources.size());
    const auto config = getCachedConfig();

    for (size_t i(0); i < sources.size(); ++i) {
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

void SourcePreset::print(std::ostream& stream, const SortType sortType) const {
    const auto config = getCachedConfig();

    std::vector<SourceObjectId> sourceIdsSorted(sourceIds.begin(), sourceIds.end());
    std::sort(sourceIdsSorted.begin(), sourceIdsSorted.end(), [&](const SourceObjectId& a, const SourceObjectId& b) {
        const auto& sA = config->sources.at(a);
        const auto& sB = config->sources.at(b);
        switch (sortType) {
            case SORT_BY_ID:           return sA.id < sB.id;
            case SORT_BY_SECTION:      return sA.section < sB.section;
            case SORT_BY_INET_TYPE:    return sA.inetType < sB.inetType;
            case SORT_BY_STORAGE_TYPE: return sA.storageType < sB.storageType;
            default:                   return false;
        }
    });

    TablePrinter table({"ID", "Section", "Storage", "Inet", "URL"});

    auto truncate = [](std::string s, size_t width) -> std::string {
        if (s.length() > width) return s.substr(0, width - 3) + "...";
        return s;
    };

    const std::vector<size_t> widths = {8, 13, 18, 8, 70};

    for (const auto& sid : sourceIdsSorted) {
        const Source& src = config->sources.at(sid);

        table.addRow({
            std::to_string(src.id),
            truncate(src.section, widths[1]),
            truncate(sourceStorageTypeToString(src.storageType), widths[2]),
            truncate(sourceInetTypeToString(src.inetType), widths[3]),
            truncate(src.url, widths[4])
        });
    }

    stream << "PRESET: " << this->label << "\n";
    table.print(stream);
    stream << "\n";
}
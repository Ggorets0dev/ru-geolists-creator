#ifndef MAIN_SOURCES_HPP
#define MAIN_SOURCES_HPP

#include <string>
#include <forward_list>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fs_utils.hpp"
#include "json_io.hpp"

class Source;
class SourcePreset;

// ID of source saved in configuration file
using SourceObjectId = uint16_t;
using DownloadedSourcePair = std::pair<SourceObjectId, fs::path>;

using SourcesStorage = std::unordered_map<SourceObjectId, Source>;
using SourcePresetsStorage = std::unordered_map<std::string, SourcePreset>;

class Source final {
public:
    // EXAMPLE:
    // {
    //     "id": 1,
    //     "storage_type": "file_loc",
    //     "inet_type": "domain",
    //     "url": "/tmp/file.txt",
    //     "section": "ru-whitelist"
    // }

    enum StorageType {
        REGULAR_FILE_LOCAL,
        REGULAR_FILE_REMOTE,
        GITHUB_RELEASE,
        STORAGE_TYPE_UNKNOWN
    };

    enum InetType {
        DOMAIN,
        IP,
        INET_TYPE_UNKNOWN
    };

    enum PreprocessingType {
        EXTRACT_DOMAINS,
        PREPROCESSING_TYPE_UNKNOWN
    };

    Source(const SourceObjectId id, const InetType inetType, std::string  section) :
        id(id), section(std::move(section)), inetType(inetType) {}

    ~Source() = default;
    Source(const Source& other) = default;
    explicit Source(const Json::Value& value);
    bool getData(std::vector<DownloadedSourcePair>& downloads) const;

    SourceObjectId id;
    std::string section;
    std::string url;

    StorageType storageType;
    InetType inetType;
    std::optional<PreprocessingType> preprocType;
    std::optional<std::string> group;

    std::optional<std::vector<std::string>> assets; // For GitHub release
};

class SourcePreset final {
public:
    // EXAMPLE:
    // {
    //     "id": 1,
    //     "label": "base"
    //     "source_ids": [1]
    // }

    enum SortType {
        SORT_BY_ID,
        SORT_BY_SECTION,
        SORT_BY_INET_TYPE,
        SORT_BY_STORAGE_TYPE
    };

    ~SourcePreset() = default;

    std::string label;
    bool isGrouped = false;
    std::forward_list<SourceObjectId> sourceIds;

    [[nodiscard]] bool isGroupRequested(const SourcesStorage& storage) const;
    void print(std::ostream& stream, SortType sortType) const;
    [[nodiscard]] std::optional<std::vector<DownloadedSourcePair>> downloadSources() const;
};

// ===============
// TYPE CONVERTERS
// ===============
std::string sourceInetTypeToString(Source::InetType type);

Source::InetType sourceStringToInetType(std::string_view str);

std::string sourceStorageTypeToString(Source::StorageType type);

Source::StorageType sourceStringToStorageType(std::string_view str);

Source::PreprocessingType sourceStringToPreprocType(std::string_view str);
// ===============

void groupSourcesBySections(std::vector<DownloadedSourcePair>& downloadedSources);

void groupSourcesByInetType(std::vector<DownloadedSourcePair>& sources,
                                                         std::unordered_map<SourceObjectId, Source>& sourcesStorage);

void groupSourcesByGroups(std::vector<DownloadedSourcePair>& sources, SourcesStorage& sourcesStorage);

#endif // MAIN_SOURCES_HPP

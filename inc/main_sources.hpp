#ifndef MAIN_SOURCES_HPP
#define MAIN_SOURCES_HPP

#include <string>
#include <forward_list>
#include <vector>

#include "fs_utils.hpp"
#include "json_io.hpp"

#define REFILTER_SECTION_NAME               "refilter"
#define REFILTER_DOMAIN_ASSET_FILE_NAME     "domains_all.lst"
#define REFILTER_IP_ASSET_FILE_NAME         "ipsum.lst"
#define REFILTER_API_LAST_RELEASE_URL       "https://api.github.com/repos/1andrevich/Re-filter-lists/releases/latest"

#define XRAY_RULES_API_LAST_RELEASE_URL     "https://api.github.com/repos/Loyalsoldier/v2ray-rules-dat/releases/latest"
#define XRAY_REJECT_SECTION_NAME            "v2ray_reject"

#define RUADLIST_SECTION_NAME               "ruadlist"
#define RUADLIST_API_MASTER_URL             "https://api.github.com/repos/easylist/ruadlist/branches/master"
#define RUADLIST_ADSERVERS_URL              "https://raw.githubusercontent.com/easylist/ruadlist/refs/heads/master/advblock/adservers.txt"

#define ANTIFILTER_SECTION_NAME             "antifilter"
#define ANTIFILTER_AYN_IPS_URL              "https://antifilter.download/list/allyouneed.lst"

class Source;

// ID of source saved in configuration file
using SourceObjectId = uint16_t;
using DownloadedSourcePair = std::pair<SourceObjectId, fs::path>;

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

    ~Source() = default;
    Source(const Source& other) = default;
    explicit Source(const Json::Value& value);
    bool getData(std::vector<DownloadedSourcePair>& downloads) const;

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

    SourceObjectId id;
    std::string section;
    std::string url;

    StorageType storageType;
    InetType inetType;
    std::optional<PreprocessingType> preprocType;

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
    std::forward_list<SourceObjectId> sourceIds;

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

void joinSimilarSources(std::vector<DownloadedSourcePair>& downloadedSources);

#endif // MAIN_SOURCES_HPP

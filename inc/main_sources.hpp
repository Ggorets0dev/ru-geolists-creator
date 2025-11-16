#ifndef MAIN_SOURCES_HPP
#define MAIN_SOURCES_HPP

#include <string>
#include <iostream>
#include <vector>

#include "fs_utils.hpp"

#define REFILTER_SECTION_NAME               "refilter"
#define REFILTER_DOMAIN_ASSET_FILE_NAME     "domains_all.lst"
#define REFILTER_IP_ASSET_FILE_NAME         "ipsum.lst"
#define REFILTER_API_LAST_RELEASE_URL       "https://api.github.com/repos/1andrevich/Re-filter-lists/releases/latest"
#define REFILTER_RELEASE_REQ_FILE_NAME      "refilter_github_release_api_req.json"

#define XRAY_RULES_API_LAST_RELEASE_URL     "https://api.github.com/repos/Loyalsoldier/v2ray-rules-dat/releases/latest"
#define XRAY_RULES_RELEASE_REQ_FILE_NAME    "xray_rules_github_release_api_req.json"
#define XRAY_REJECT_SECTION_NAME            "v2ray_reject"

#define RUADLIST_SECTION_NAME               "ruadlist"
#define RUADLIST_API_MASTER_URL             "https://api.github.com/repos/easylist/ruadlist/branches/master"
#define RUADLIST_ADSERVERS_URL              "https://raw.githubusercontent.com/easylist/ruadlist/refs/heads/master/advblock/adservers.txt"
#define RUADLIST_FILE_NAME                  "ruadlist.txt"
#define RUADLIST_EXTRACTED_FILE_NAME        "ruadlist_extracted.txt"

#define ANTIFILTER_SECTION_NAME             "antifilter"
#define ANTIFILTER_FILE_NAME                "antifilter.txt"
#define ANTIFILTER_AYN_IPS_URL              "https://antifilter.download/list/allyouneed.lst"

// ID of source saved in configuration file
using SourceId = uint16_t;

class Source {

public:
    enum Type {
        DOMAIN,
        IP
    };

    std::string section;
    Type type;

    Source() = default;
    Source(Type type, const std::string& section);

    virtual void print(std::ostream& stream) const;
};

using DownloadedSourcePair = std::pair<Source, fs::path>;

std::string sourceTypeToString(Source::Type type);

Source::Type sourceStringToType(std::string_view str);

void printDownloadedSources(std::ostream& stream, const std::vector<DownloadedSourcePair>& downloadedSources);

void joinSimilarSources(std::vector<DownloadedSourcePair>& downloadedSources);

std::string genSourceFileName(const Source& source);

#endif // MAIN_SOURCES_HPP

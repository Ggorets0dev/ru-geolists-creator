#ifndef MAIN_SOURCES_HPP
#define MAIN_SOURCES_HPP

#define REFILTER_SECTION_NAME               "refilter"
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

#include <string>
#include <iostream>
#include <filesystem>
#include <vector>

class Source {

public:
    enum Type {
        DOMAIN,
        IP
    };

    Source(Source::Type type, const std::string& section);
    void print(std::ostream& stream) const;

    std::string section;
    Source::Type type;
};

namespace fs = std::filesystem;
using DownloadedSourcePair = std::pair<Source, fs::path>;

extern std::string
sourceTypeToString(Source::Type type);

extern Source::Type
sourceStringToType(std::string_view str);

extern void
printDownloadedSources(std::ostream& stream, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // MAIN_SOURCES_HPP

#ifndef MAIN_SOURCES_HPP
#define MAIN_SOURCES_HPP

#include <string>
#include <iostream>

#define REFILTER_SECTION_NAME               "refilter"
#define REFILTER_API_LAST_RELEASE_URL       "https://api.github.com/repos/1andrevich/Re-filter-lists/releases/latest"
#define REFILTER_RELEASE_REQ_FILE_NAME      "refilter_github_release_api_req.json"

#define XRAY_RULES_API_LAST_RELEASE_URL     "https://api.github.com/repos/Loyalsoldier/v2ray-rules-dat/releases/latest"
#define XRAY_RULES_RELEASE_REQ_FILE_NAME    "xray_rules_github_release_api_req.json"

#define RUADLIST_SECTION_NAME               "ruadlist"
#define RUADLIST_URL                        "https://easylist-downloads.adblockplus.org/ruadlist.txt"
#define RUADLIST_FILE_NAME                  "ruadlist.txt"
#define RUADLIST_EXTRACTED_FILE_NAME        "ruadlist_extracted.txt"

class Source {

public:
    enum Type {
        DOMAIN,
        IP
    };

    Source(Source::Type type, const std::string& section);
    void print() const;

    std::string section;
    Source::Type type;
};

extern std::string
sourceTypeToString(Source::Type type);

extern Source::Type
sourceStringToType(std::string_view str);

#endif // MAIN_SOURCES_HPP

#include "filter.hpp"
#include <regex>

const std::vector<std::string> kKeywordWhitelist = {
    "google",
    "yandex",
    "vk.com",
    "ya.ru",
    "ok.ru",
    "yastatic.net",
    "alfabank.ru"
};

bool isUrl(const std::string& str) {
    static const std::regex url_regex(
        R"(^[a-zA-Z][a-zA-Z0-9+.\-]*://)"
    );
    return std::regex_search(str, url_regex);
}

bool checkKeywordWhitelist(std::string_view domain) {
    for (uint8_t i(0); i < kKeywordWhitelist.size(); ++i) {
        if (domain.find(kKeywordWhitelist[i]) != std::string::npos) {
            return true;
        }
    }

    return false;
}

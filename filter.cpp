#include "filter.hpp"

const std::vector<std::string> kKeywordWhitelist = {
    "google",
    "yandex",
    "vk.com",
    "ya.ru",
    "ok.ru"
};

bool
checkKeywordWhitelist(std::string_view domain) {
    for (uint8_t i(0); i < kKeywordWhitelist.size(); ++i) {
        if (domain.find(kKeywordWhitelist[i]) != std::string::npos) {
            return false;
        }
    }

    return true;
}

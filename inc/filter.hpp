#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <vector>
#include <fs_utils.hpp>

extern const std::vector<std::string> kKeywordWhitelist;

bool isUrl(const std::string& str);

bool checkKeywordWhitelist(std::string_view domain);

#endif // FILTER_HPP

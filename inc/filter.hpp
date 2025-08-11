#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <fs_utils.hpp>

#include "log.hpp"

extern const std::vector<std::string> kKeywordWhitelist;

extern bool
checkKeywordWhitelist(std::string_view domain);

extern void
removeDuplicateDomains(const std::string& fileAPath, const std::string& fileBPath);

#endif // FILTER_HPP

#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <filesystem>

#include "log.hpp"

namespace fs = std::filesystem;

extern const std::vector<std::string> kKeywordWhitelist;

extern bool
checkKeywordBlacklist(std::string_view domain);

extern bool
removeDuplicateDomains(const std::string& fileAPath, const std::string& fileBPath);

#endif // FILTER_HPP

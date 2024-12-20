#ifndef FILTER_H
#define FILTER_H

#include <string>
#include <vector>

extern const std::vector<std::string> kKeywordWhitelist;

bool
checkKeywordBlacklist(std::string_view domain);

#endif // FILTER_H

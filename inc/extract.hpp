#ifndef EXTRACT_H
#define EXTRACT_H

#include <string>
#include <iostream>
#include <fstream>
#include <regex>
#include <unordered_set>

#include "filter.hpp"
#include "log.hpp"

bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath);

bool
removeDuplicateDomains(const std::string& fileAPath, const std::string& fileBPath);

bool
removeDuplicateDomains(const std::string& filePath);

#endif // EXTRACT_H

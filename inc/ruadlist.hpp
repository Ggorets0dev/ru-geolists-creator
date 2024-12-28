#ifndef RUADLIST_HPP
#define RUADLIST_HPP

#include <filesystem>
#include <fstream>
#include <regex>

#include "log.hpp"
#include "filter.hpp"

#define RUADLIST_VERSION_SIZE 12

namespace fs = std::filesystem;

extern bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath);

extern bool
parseRuadlistVersion(const std::string& inputFilePath, std::string& versionOut);

#endif /* RUADLIST_HPP */

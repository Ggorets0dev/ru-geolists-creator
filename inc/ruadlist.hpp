#include <filesystem>
#include <fstream>
#include <regex>

#include "log.hpp"
#include "filter.hpp"

namespace fs = std::filesystem;

extern bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath);

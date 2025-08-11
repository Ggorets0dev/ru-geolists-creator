#ifndef RUADLIST_HPP
#define RUADLIST_HPP

#include <fstream>
#include <regex>

#include "json_io.hpp"
#include "log.hpp"
#include "filter.hpp"

#define RUADLIST_VERSION_SIZE 12

extern bool
extractDomainsFromFile(const std::string& inputFilePath, const std::string& outputFilePath);

extern bool
parseRuadlistUpdateDatetime(const Json::Value &value, std::time_t& dtOut);

#endif /* RUADLIST_HPP */

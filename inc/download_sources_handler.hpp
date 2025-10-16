#ifndef DOWNLOAD_SOURCES_HANDLER_HPP
#define DOWNLOAD_SOURCES_HANDLER_HPP

#include "config.hpp"

bool downloadNewestSources(RgcConfig& config, bool useExtraSources, bool useFilter, std::vector<DownloadedSourcePair>& downloadedFiles);

#endif // DOWNLOAD_SOURCES_HANDLER_HPP

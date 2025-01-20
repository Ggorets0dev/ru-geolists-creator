#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include "dlc_toolchain.hpp"
#include "v2ip_toolchain.hpp"
#include "config.hpp"

#define RELEASE_MOTES_FILENAME  "release_notes.txt"

extern void
createReleaseNotes(const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP

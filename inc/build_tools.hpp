#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include <utility>

#include "config.hpp"

#define GEOSITE_FILENAME_DAT        "geosite.dat"
#define GEOIP_FILENAME_DAT          "geoip.dat"

#define GEOSITE_FILENAME_DB         "geosite.db"
#define GEOIP_FILENAME_DB           "geoip.db"

#define RELEASE_NOTES_FILENAME      "release_notes.txt"

struct GeoReleasePack {
    GeoReleasePack(fs::path  listDomain, fs::path  listIP) :
        listDomain(std::move(listDomain)), listIP(std::move(listIP)) {}

    fs::path listDomain;
    fs::path listIP;
};

struct GeoReleases {
    std::vector<GeoReleasePack> packs;
    fs::path releaseNotes;
    bool isEmpty;
};

void createReleaseNotes(const GeoReleases& paths, const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources);

#endif // BUILD_TOOLS_HPP

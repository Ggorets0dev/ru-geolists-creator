#ifndef BUILD_TOOLS_HPP
#define BUILD_TOOLS_HPP

#include <utility>

#include "config.hpp"

#define GEOSITE_BASE_FILENAME        "geosite"
#define GEOIP_BASE_FILENAME          "geoip"
#define RULESET_BASE_FILENAME        "ruleset"

#define V2RAY_FILES_EXT              "dat"
#define SING_DB_FILES_EXT            "db"
#define SING_RS_FILES_EXT            "srs"

#define RELEASE_NOTES_FILENAME       "release_notes.txt"

struct GeoReleasePack {
    GeoReleasePack() {
        presetLabel = "";
        listDomain = std::nullopt;
        listIP = std::nullopt;
        listsRuleSet = std::nullopt;
    }

    GeoReleasePack(std::string presetLabel, fs::path  listDomain, fs::path  listIP, std::vector<fs::path> listRuleSet) :
        presetLabel(std::move(presetLabel)), listDomain(std::move(listDomain)), listIP(std::move(listIP)), listsRuleSet(std::move(listRuleSet)) {}

    std::string presetLabel;
    std::optional<fs::path> listDomain;
    std::optional<fs::path> listIP;
    std::optional<std::vector<fs::path>> listsRuleSet;
};

struct GeoReleases {
    std::vector<GeoReleasePack> packs;
    fs::path releaseNotes;
    bool isEmpty;
};

struct BuildStats {
    size_t subnetsFilesCount = 0;
    size_t subnetsCount = 0;
    size_t domainsFilesCount = 0;
    size_t domainsCount = 0;
    std::vector<std::string> formats;
};

bool setBuildInfoToRelNotes(std::ofstream& file, const BuildStats& stats, std::string_view message);

bool addPresetToRelNotes(std::ofstream& file, const SourcePreset& preset);

#endif // BUILD_TOOLS_HPP

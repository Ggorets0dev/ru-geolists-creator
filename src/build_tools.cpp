#include "build_tools.hpp"
#include "time_tools.hpp"

void
createReleaseNotes(const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources) {
    fs::path notesPath = fs::current_path() / OUTPUT_FOLDER_NAME / RELEASE_MOTES_FILENAME;
    std::ofstream releaseNotes(notesPath);

    releaseNotes << "Added IP/Domain sources:" << std::endl;
    printDownloadedSources(releaseNotes, downloadedSources);

    releaseNotes << "\n\n";

    releaseNotes << "ReFilter lists datetime: " << parseUnixTime(config.refilterTime) << std::endl;
    releaseNotes << "V2Ray lists datetime: " << parseUnixTime(config.v2rayTime) << std::endl;
    releaseNotes << "RuAdlist version: " << config.ruadlistVersion << std::endl;

    releaseNotes << "\n\n";

    releaseNotes << "Build datetime: " << parseUnixTime(std::time(nullptr));
}

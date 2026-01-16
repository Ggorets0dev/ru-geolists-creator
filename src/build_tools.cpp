#include "build_tools.hpp"
#include "time_tools.hpp"
#include "cli_args.hpp"
#include "cli_draw.hpp"
#include "log.hpp"

#include <sys/stat.h>
#include <csignal>

static void saveToText(const RgcConfig& config, const std::vector<DownloadedSourcePair>& downloadedSources, const fs::path& path) {
    std::ofstream releaseNotes(path);

    constexpr int labelCol = 25;
    constexpr int valueCol = 30;
    const std::vector<int> mainWidths = {labelCol, valueCol};

    releaseNotes << "BUILD ENVIRONMENT & LISTS\n";
    printTableLine(releaseNotes, mainWidths);

    releaseNotes << "| " << std::left << std::setw(labelCol) << "Component"
                 << " | " << std::left << std::setw(valueCol) << "Version / Time" << " |\n";
    printTableLine(releaseNotes, mainWidths);

    auto addRow = [&](const std::string& label, const std::string& value) {
        releaseNotes << "| " << std::left << std::setw(labelCol) << label
                     << " | " << std::left << std::setw(valueCol) << value << " |\n";
    };

    addRow("ReFilter lists", parseUnixTime(config.refilterTime));
    addRow("V2Ray lists", parseUnixTime(config.v2rayTime));
    addRow("RuAdList", parseUnixTime(config.ruadlistTime));

    printTableLine(releaseNotes, mainWidths);
    addRow("Build datetime", parseUnixTime(std::time(nullptr)));
    printTableLine(releaseNotes, mainWidths);

    releaseNotes << "\nDOWNLOADED RESOURCES\n";
    printDownloadedSources(releaseNotes, downloadedSources, false);

    releaseNotes.close();
}

void createReleaseNotes(const GeoReleases& paths,
                        const RgcConfig& config,
                        const std::vector<DownloadedSourcePair>& downloadedSources) {

    // Form TXT release notes for user
    saveToText(config, downloadedSources, paths.releaseNotes);
}

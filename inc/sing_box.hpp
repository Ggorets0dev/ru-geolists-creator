#ifndef SING_BOX_HPP
#define SING_BOX_HPP

#include <vector>
#include "main_sources.hpp"

bool generateSingBoxRuleSet(
    const std::vector<DownloadedSourcePair>& sources,
    const fs::path& savePath,
    const SourcesStorage& storage
);

bool compileSingBoxRuleSet(
    const fs::path& binaryPath,
    const fs::path& jsonPath,
    const fs::path& srsPath
);

#endif // SING_BOX_HPP
#ifndef SING_BOX_HPP
#define SING_BOX_HPP

#include <vector>
#include "main_sources.hpp"

std::optional<std::vector<fs::path>> generateSingBoxRuleSets(
    const std::vector<DownloadedSourcePair>& sources,
    const SourcesStorage& storage
);

std::optional<std::vector<fs::path>> compileSingBoxRuleSets(
    const fs::path& binaryPath,
    const fs::path& targetDir,
    const std::vector<fs::path>& jsonPaths
);

#endif // SING_BOX_HPP
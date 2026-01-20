#ifndef URL_HANDLE_HPP
#define URL_HANDLE_HPP

#include <string>
#include <json_io.hpp>

#include "net_types_base.hpp"
#include "fs_utils.hpp"

namespace NetUtils {
    NetTypes::AddressType getAddressType(const std::string& input);

    std::vector<std::string> downloadGithubReleaseAssets(const std::string& url,
        const std::vector<std::string>& fileNames,
        const fs::path& dirPath,
        const std::string& apiToken);

    bool tryAccessUrl(const std::string& url, const char* httpHeader = nullptr);

    bool tryDownloadFile(const std::string& url, const std::string& filePath, const char* httpHeader = nullptr);

    bool tryAccessGithubReleaseAssets(const std::string& url, const std::vector<std::string>& assets, const std::string& apiToken);

    std::string convertToGithubApi(const std::string& repoUrl);

    bool tryDownloadFromGithub(const std::string& url, const std::string& filePath, const std::string& apiToken);
}

#endif //URL_HANDLE_HPP
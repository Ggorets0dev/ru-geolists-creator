#include "network.hpp"

#define USER_AGENT "C++ App"

static size_t
writeToFileCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* outFile = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;

    if (outFile->is_open()) {
        outFile->write(static_cast<char*>(contents), totalSize);
    }

    return totalSize;
}

bool
downloadFile(const std::string& url, const std::string& filePath) {
    CURL* curl;
    CURLcode res;

    // Открываем файл для записи
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + filePath);
        return false;
    }

    curl = curl_easy_init();
    if (curl) {
        // Настраиваем URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

        // Настраиваем обратный вызов для записи данных
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFileCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Следовать за редиректами
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);

        // Выполняем запрос
        res = curl_easy_perform(curl);

        // Проверяем на ошибки
        if (res != CURLE_OK) {
            log(LogType::ERROR, "Failed to download file because of error:", curl_easy_strerror(res));
            return false;
        }

        // Освобождаем ресурсы
        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Failed to initialize CURL." << std::endl;
        return false;
    }

    outFile.close();

    return true;
}

std::optional<std::time_t>
parsePublishTime(const Json::Value& value) {
    std::tm tm = {};
    std::istringstream ss(value["published_at"].asString());

    // Парсим строку в формате ISO 8601 (например, 2024-12-20T14:11:25Z)
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    if (ss.fail()) {
        LOG_ERROR("Failed to parse publish time from release");
        return std::nullopt;
    }

    // Конвертируем в time_t (UNIX-время)
    return std::mktime(&tm);
}

bool
downloadGithubReleaseAssets(const Json::Value& value, const std::vector<std::string>& fileNames) {
    bool status = false;

    if (value.isMember("assets") && value["assets"].isArray()) {
        const Json::Value& assets = value["assets"];
        for (const auto& asset : assets) {
            if (std::find(fileNames.begin(), fileNames.end(), asset["name"].asString()) == fileNames.end()) {
                continue;
            }

            status = downloadFile(asset["browser_download_url"].asString(), asset["name"].asString());
        }
    }

    return status;
}

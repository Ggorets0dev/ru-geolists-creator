#include "network.hpp"

#define USER_AGENT "C++ App"

// Функция обратного вызова для записи данных в файл
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

        // Выполняем запрос
        res = curl_easy_perform(curl);

        // Проверяем на ошибки
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        } else {
            std::cout << "File downloaded successfully." << std::endl;
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

#include "json_io.hpp"

bool
readJsonFromFile(const std::string& filePath, Json::Value& outValue) {
    // Открываем файл для чтения
    std::ifstream file(filePath, std::ifstream::binary);
    if (!file.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + filePath);
        return false;
    }

    // Парсим JSON из файла
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;

    if (!Json::parseFromStream(readerBuilder, file, &outValue, &errs)) {
        LOG_ERROR("Failed to parse JSON: " + errs);

        file.close();
        return false;
    }

    file.close();

    return true;
}

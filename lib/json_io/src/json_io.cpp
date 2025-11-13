#include "json_io.hpp"
#include "log.hpp"

#include <iomanip>
#include <fstream>

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

bool
writeJsonToFile(const std::string& filePath, const Json::Value& value) {
    std::ofstream fileOut(filePath, std::ofstream::binary);
    if (!fileOut.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + filePath);
        return false;
    }

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(value, &fileOut);
    fileOut.close();

    return true;
}

template<typename Type>
bool
updateJsonValue(const std::string& filePath, const std::string& key, Type value) {
    bool status;
    Json::Value outValue;

    status = readJsonFromFile(filePath, outValue);

    if (!status) {
        return false;
    }

    outValue[key] = value;

    std::ofstream fileOut(filePath, std::ofstream::binary);
    if (!fileOut.is_open()) {
        LOG_ERROR(FILE_OPEN_ERROR_MSG + filePath);
        return false;
    }

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(outValue, &fileOut);
    fileOut.close();

    return true;
}

std::optional<std::time_t>
parsePublishTime(const Json::Value& value) {
    std::tm tm = {};
    std::istringstream ss(value["published_at"].asString());

    // Parse date and time with ISO 8601 (example 2024-12-20T14:11:25Z)
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

    if (ss.fail()) {
        LOG_ERROR("Failed to parse publish time from release");
        return std::nullopt;
    }

    // Convert to time_t (UNIX-time)
    return std::mktime(&tm);
}

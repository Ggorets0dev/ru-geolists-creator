#ifndef JSON_IO_HPP
#define JSON_IO_HPP

#include <ctime>
#include <string>
#include <optional>

#if __has_include(<jsoncpp/json/json.h>)
#include <jsoncpp/json/json.h>
#elif __has_include(<json/json.h>)
#include <json/json.h>
#else
#error "JSONCPP header not found"
#endif

#include <json/json.h>
#include <optional>
#include <string>
#include <stdexcept>
#include <vector>
#include <type_traits>

class JsonValidator {
public:
    template<typename T>
    static T getRequired(const Json::Value& node, const std::string& key) {
        auto res = checkAndGet<T>(node, key);
        if (!res) throw std::runtime_error("Invalid or missing field: '" + key + "'");
        return *res;
    }

    template<typename T>
    static std::optional<T> getOptional(const Json::Value& node, const std::string& key) {
        return checkAndGet<T>(node, key);
    }

    template<typename T>
    static std::vector<T> getRequiredArray(const Json::Value& node, const std::string& key) {
        if (!node.isMember(key) || !node[key].isArray()) {
            throw std::runtime_error("Missing or invalid array: '" + key + "'");
        }
        return convertToVector<T>(node[key]);
    }

    template<typename T>
    static std::optional<std::vector<T>> getOptionalArray(const Json::Value& node, const std::string& key) {
        if (!node.isMember(key) || !node[key].isArray()) {
            return std::nullopt;
        }
        return convertToVector<T>(node[key]);
    }

private:
    template<typename T>
    static std::optional<T> checkAndGet(const Json::Value& node, const std::string& key) {
        if (!node.isMember(key)) return std::nullopt;
        const Json::Value& val = node[key];

        if constexpr (std::is_enum_v<T>) {
            if (val.isInt()) return static_cast<T>(val.asInt());
        } else if constexpr (std::is_same_v<T, int>) {
            if (val.isInt()) return val.asInt();
        } else if constexpr (std::is_same_v<T, std::string>) {
            if (val.isString()) return val.asString();
        } else if constexpr (std::is_same_v<T, bool>) {
            if (val.isBool()) return val.asBool();
        } else if constexpr (std::is_same_v<T, double>) {
            if (val.isNumeric()) return val.asDouble();
        }
        return std::nullopt;
    }

    template<typename T>
    static std::vector<T> convertToVector(const Json::Value& arrayNode) {
        std::vector<T> result;
        result.reserve(arrayNode.size());
        for (Json::ArrayIndex i = 0; i < arrayNode.size(); ++i) {
            Json::Value tempRoot;
            tempRoot["item"] = arrayNode[i];
            auto val = checkAndGet<T>(tempRoot, "item");
            if (!val) throw std::runtime_error("Invalid type in array at index " + std::to_string(i));
            result.push_back(*val);
        }
        return result;
    }
};

bool readJsonFromFile(const std::string& filePath, Json::Value& outValue);

bool writeJsonToFile(const std::string& filePath, const Json::Value& value);

template<typename Type>
bool updateJsonValue(const std::string& filePath, const std::string& key, const Type& value);

std::optional<std::time_t> parsePublishTime(const Json::Value& value);

#endif // JSON_IO_HPP

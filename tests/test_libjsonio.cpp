#include <catch2/catch_all.hpp>

#include "json_io.hpp"
#include "fs_utils.hpp"
#include "log.hpp"

#include <fstream>
#include <unistd.h>

static std::string createTempJSON(const std::string& content) {
    fs::path tmp = fs::temp_directory_path() / ("test_json_XXXXXX.json");
    std::string path = tmp.string();

    char templ[] = "/tmp/catch2_json_test_XXXXXX";
    int fd = mkstemp(templ);
    close(fd);
    path = std::string(templ);

    std::ofstream f(path);
    f << content;
    f.close();

    // Will be deleted after test
    REQUIRE(fs::exists(path));
    return path;
}

TEST_CASE("readJsonFromFile: valid JSON", "[json_io]") {
    Json::Value expected;
    expected["name"] = "TestApp";
    expected["version"] = "1.2.3";
    expected["enabled"] = true;

    std::ostringstream oss;
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(expected, &oss);

    std::string tempFile = createTempJSON(oss.str());

    Json::Value result;
    bool success = readJsonFromFile(tempFile, result);

    REQUIRE(success == true);
    REQUIRE(result["name"].asString() == "TestApp");
    REQUIRE(result["version"].asString() == "1.2.3");
    REQUIRE(result["enabled"].asBool() == true);

    removePath(tempFile);
}

TEST_CASE("readJsonFromFile: file not exists", "[json_io]") {
    Json::Value root;
    const bool success = readJsonFromFile("/this/path/should/not/exist/12345.json", root);

    REQUIRE(success == false);
    // Лог уже должен был отработать, проверяем только возврат
}

TEST_CASE("readJsonFromFile: invalid JSON", "[json_io]") {
    const std::string tempFile = createTempJSON(R"({"key": "value", bad json})");

    Json::Value root;
    const bool success = readJsonFromFile(tempFile, root);

    REQUIRE(success == false);

    removePath(tempFile);
}

TEST_CASE("writeJsonToFile: writes normal JSON", "[json_io]") {
    std::string tempFile;
    {
        // Создаём временный файл вручную
        char templ[] = "/tmp/json_write_test_XXXXXX.json";
        int fd = mkstemp(templ);
        close(fd);
        tempFile = templ;
    }

    Json::Value value;
    value["hello"] = "world";
    value["answer"] = 42;

    bool success = writeJsonToFile(tempFile, value);
    REQUIRE(success == true);

    // Прочитаем обратно и проверим
    Json::Value readBack;
    REQUIRE(readJsonFromFile(tempFile, readBack) == true);
    REQUIRE(readBack["hello"].asString() == "world");
    REQUIRE(readBack["answer"].asInt() == 42);

    removePath(tempFile);
}

TEST_CASE("updateJsonValue: updates existing key", "[json_io]") {
    Json::Value orig;
    orig["version"] = "1.0.0";
    orig["count"] = 5;

    std::ostringstream oss;
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "  ";
    std::unique_ptr<Json::StreamWriter> writer(wbuilder.newStreamWriter());
    writer->write(orig, &oss);

    std::string tempFile = createTempJSON(oss.str());

    bool ok = updateJsonValue(tempFile, "version", std::string("2.0.0"));
    REQUIRE(ok == true);

    Json::Value updated;
    REQUIRE(readJsonFromFile(tempFile, updated) == true);
    REQUIRE(updated["version"].asString() == "2.0.0");
    REQUIRE(updated["count"].asInt() == 5);  // старое значение осталось

    removePath(tempFile);
}

TEST_CASE("updateJsonValue: adds new key", "[json_io]") {
    std::string json = R"({
  "existing": true
})";

    std::string tempFile = createTempJSON(json);

    bool ok = updateJsonValue(tempFile, "new_key", 12345);
    REQUIRE(ok == true);

    Json::Value root;
    REQUIRE(readJsonFromFile(tempFile, root) == true);
    REQUIRE(root["existing"].asBool() == true);
    REQUIRE(root["new_key"].asInt() == 12345);

    removePath(tempFile);
}

TEST_CASE("parsePublishTime: valid ISO 8601", "[json_io]") {
    Json::Value obj;
    obj["published_at"] = "2024-12-20T14:11:25Z";

    const auto opt = parsePublishTime(obj);
    REQUIRE(opt.has_value());

    std::tm expected = {};
    expected.tm_year = 2024 - 1900;
    expected.tm_mon = 11;   // December
    expected.tm_mday = 20;
    expected.tm_hour = 14;
    expected.tm_min = 11;
    expected.tm_sec = 25;

    const std::time_t expected_t = std::mktime(&expected);

    // На некоторых системах mktime может немного отличаться из-за часовых поясов,
    // но в UTC должно быть точно
    REQUIRE(*opt == expected_t);
}

TEST_CASE("parsePublishTime: invalid format", "[json_io]") {
    Json::Value obj;
    obj["published_at"] = "this-is-not-a-date";

    const auto opt = parsePublishTime(obj);
    REQUIRE_FALSE(opt.has_value());
}

TEST_CASE("parsePublishTime: missing key", "[json_io]") {
    const Json::Value obj;  // no published_at

    const auto opt = parsePublishTime(obj);
    REQUIRE_FALSE(opt.has_value());
}
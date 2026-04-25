#include "catch2/catch_all.hpp"
#include <fstream>
#include <iostream>

#include "fs_utils.hpp"
#include "fs_utils_temp.hpp"
#include "log.hpp"

void createTempFile(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream f(path);
    for (const auto& line : lines) {
        f << line << '\n';
    }
    std::cout << "Temp file created: " << path << '\n';
}

// RAII wrapper to auto-clean everything
struct TempFiles {
    std::vector<fs::path> paths;
    ~TempFiles() {
        for (const auto& p : paths) {
            if (!p.empty() && fs::exists(p)) {
                std::error_code ec;
                fs::remove_all(p, ec);  // silently ignore errors in destructor
            }
        }
    }
    void add(const fs::path& p) { paths.push_back(p); }
};

TEST_CASE("countLinesInFile", "[fs]")
{
    FS::Utils::Temp::SessionTempFileRegistry tfr("countLinesInFile_TEST");

    SECTION("normal file")
    {
        auto filePathA = tfr.createTempFile("txt").lock()->path;
        createTempFile(filePathA, {"a", "bb", "ccc"});
        REQUIRE(countLinesInFile(filePathA) == 3);
    }

    SECTION("empty file")
    {
        auto filePathA = tfr.createTempFile("txt").lock()->path;
        createTempFile(filePathA, {});
        REQUIRE(countLinesInFile(filePathA) == 0);
    }

    SECTION("file does not exist -> throws")
    {
        REQUIRE_THROWS_AS(countLinesInFile("/this/does/not/exist.txt"),
                          std::ios_base::failure);
    }
}

TEST_CASE("removeDuplicateLines", "[fs]")
{
    SECTION("removes duplicates from larger file (A > B)")
    {
        FS::Utils::Temp::SessionTempFileRegistry tfr("removeDuplicateLines_TEST");

        auto filePathA = tfr.createTempFile("txt").lock()->path;
        auto filePathB = tfr.createTempFile("txt").lock()->path;

        createTempFile(filePathA,   {"line1", "line2", "line3", "line2", "line4"});
        createTempFile(filePathB, {"line2", "line4"});

        size_t removed = removeDuplicateLines(filePathA, filePathB);

        REQUIRE(removed == 3);

        std::ifstream f(filePathA);
        std::string content((std::istreambuf_iterator(f)), {});
        REQUIRE(content == "line1\nline3\n");
    }

    SECTION("removes duplicates from larger file (B > A)")
    {
        FS::Utils::Temp::SessionTempFileRegistry tfr("removeDuplicateLines_TEST");

        auto filePathA = tfr.createTempFile("txt").lock()->path;
        auto filePathB = tfr.createTempFile("txt").lock()->path;

        createTempFile(filePathA,   {"dup", "unique"});
        createTempFile(filePathB, {"dup", "dup", "other"});

        size_t removed = removeDuplicateLines(filePathA, filePathB);

        REQUIRE(removed == 2);

        std::ifstream f(filePathB);
        std::string content((std::istreambuf_iterator<char>(f)), {});
        REQUIRE(content == "other\n");
    }

    SECTION("no duplicates")
    {
        FS::Utils::Temp::SessionTempFileRegistry tfr("removeDuplicateLines_TEST");

        auto filePathA = tfr.createTempFile("txt").lock()->path;
        auto filePathB = tfr.createTempFile("txt").lock()->path;

        createTempFile(filePathA,   {"x", "y"});
        createTempFile(filePathB, {"z"});

        REQUIRE(removeDuplicateLines(filePathA, filePathB) == 0);
        REQUIRE(countLinesInFile(filePathA) == 2);
    }
}

TEST_CASE("joinTwoFiles", "[fs]")
{
    FS::Utils::Temp::SessionTempFileRegistry tfr("removeDuplicateLines_TEST");

    auto filePathA = tfr.createTempFile("txt").lock()->path;
    auto filePathB = tfr.createTempFile("txt").lock()->path;

    createTempFile(filePathA, {"first part"});
    createTempFile(filePathB, {"second part", "third part"});

    joinTwoFiles(filePathA, filePathB);

    std::ifstream f(filePathA);
    std::string content((std::istreambuf_iterator<char>(f)), {});

    // Note: stream copy via rdbuf() does NOT add extra newline between files
    REQUIRE(content == "first part\nsecond part\nthird part\n");
}

TEST_CASE("addPathPostfix", "[fs]")
{
    REQUIRE(addPathPostfix("/tmp/file.json",        "backup")   == "/tmp/file_backup.json");
    REQUIRE(addPathPostfix("noext",                 "tmp")      == "noext");
    REQUIRE(addPathPostfix("/path/with.dots/file.tar.gz", "old")
            == "/path/with.dots/file_old.tar.gz");
}

TEST_CASE("removePath removes files and directories", "[fs]")
{
    TempFiles cleanup;  // we still track them, but test removePath explicitly

    auto dir  = fs::temp_directory_path() / "test_dir_to_remove";
    auto file = dir / "some.txt";

    fs::create_directories(dir);
    std::ofstream(file) << "data";

    REQUIRE(fs::exists(dir));
    REQUIRE(fs::exists(file));

    removePath(dir.string());

    REQUIRE(!fs::exists(dir));
}
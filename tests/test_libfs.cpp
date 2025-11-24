#include "catch2/catch_all.hpp"
#include <fstream>

#include "fs_utils.hpp"
#include "log.hpp"

fs::path createTempFile(const std::string& name, const std::vector<std::string>& lines) {
    fs::path path = fs::temp_directory_path() / name;
    std::ofstream f(path);
    for (const auto& line : lines) {
        f << line << '\n';
    }
    return path;
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
    TempFiles cleanup;

    SECTION("normal file")
    {
        auto p = createTempFile("lines_test1.txt", {"a", "bb", "ccc", ""});
        cleanup.add(p);
        REQUIRE(countLinesInFile(p) == 4);
    }

    SECTION("empty file")
    {
        auto p = createTempFile("empty.txt", {});
        cleanup.add(p);
        REQUIRE(countLinesInFile(p) == 0);
    }

    SECTION("file does not exist -> throws")
    {
        REQUIRE_THROWS_AS(countLinesInFile("/this/does/not/exist.txt"),
                          std::ios_base::failure);
    }
}

TEST_CASE("removeDuplicateLines", "[fs]")
{
    TempFiles cleanup;

    SECTION("removes duplicates from larger file (A > B)")
    {
        auto fileA = createTempFile("big.txt",   {"line1", "line2", "line3", "line2", "line4"});
        auto fileB = createTempFile("small.txt", {"line2", "line4"});
        cleanup.add(fileA);
        cleanup.add(fileB);

        size_t removed = removeDuplicateLines(fileA.string(), fileB.string());

        REQUIRE(removed == 3);

        std::ifstream f(fileA);
        std::string content((std::istreambuf_iterator<char>(f)), {});
        REQUIRE(content == "line1\nline3\n");
    }

    SECTION("removes duplicates from larger file (B > A)")
    {
        auto fileA = createTempFile("small2.txt", {"dup", "unique"});
        auto fileB = createTempFile("big2.txt",   {"dup", "dup", "other"});
        cleanup.add(fileA);
        cleanup.add(fileB);

        size_t removed = removeDuplicateLines(fileA.string(), fileB.string());

        REQUIRE(removed == 2);

        std::ifstream f(fileB);
        std::string content((std::istreambuf_iterator<char>(f)), {});
        REQUIRE(content == "other\n");
    }

    SECTION("no duplicates")
    {
        auto a = createTempFile("a.txt", {"x", "y"});
        auto b = createTempFile("b.txt", {"z"});
        cleanup.add(a); cleanup.add(b);

        REQUIRE(removeDuplicateLines(a.string(), b.string()) == 0);
        REQUIRE(countLinesInFile(a) == 2);
    }
}

TEST_CASE("joinTwoFiles", "[fs]")
{
    TempFiles cleanup;

    auto fileA = createTempFile("target.txt", {"first part"});
    auto fileB = createTempFile("source.txt", {"second part", "third part"});
    cleanup.add(fileA);
    cleanup.add(fileB);

    joinTwoFiles(fileA.string(), fileB.string());

    std::ifstream f(fileA);
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
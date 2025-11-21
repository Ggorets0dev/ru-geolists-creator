#include "catch2/catch_all.hpp"
#include <vector>
#include <forward_list>

#include "common.hpp"

template<typename T>
std::vector<T> to_vec(const std::forward_list<T>& fl) {
    return {fl.begin(), fl.end()};
}

TEST_CASE("removeListDuplicates removes duplicates preserving first occurrence", "[common][dup]")
{
    std::forward_list<int> list = {1, 5, 3, 5, 7, 1, 3, 9};
    removeListDuplicates(list);
    REQUIRE(to_vec(list) == std::vector{1, 5, 3, 7, 9});
}

TEST_CASE("removeListDuplicates edge cases", "[common][dup]")
{
    SECTION("empty list")
    {
        std::forward_list<int> list;
        removeListDuplicates(list);
        REQUIRE(list.empty());
    }

    SECTION("single element")
    {
        std::forward_list<int> list = {42};
        removeListDuplicates(list);
        REQUIRE(to_vec(list) == std::vector{42});
    }
}

TEST_CASE("removeListItemsForInxs works correctly", "[common][idx]")
{
    SECTION("remove middle elements (sorted indexes)")
    {
        std::forward_list<int> list = {10, 20, 30, 40, 50};
        removeListItemsForInxs(list, {1, 3});           // remove 20 and 40
        REQUIRE(to_vec(list) == std::vector{10, 30, 50});
    }

    SECTION("remove first element")
    {
        std::forward_list<int> list = {100, 200, 300};
        removeListItemsForInxs(list, {0});
        REQUIRE(to_vec(list) == std::vector{200, 300});
    }

    SECTION("empty index list - no changes")
    {
        std::forward_list<int> list = {1, 2, 3};
        removeListItemsForInxs(list, {});
        REQUIRE(to_vec(list) == std::vector{1, 2, 3});
    }

    SECTION("indexes beyond size are ignored")
    {
        std::forward_list<int> list = {7, 8};
        removeListItemsForInxs(list, {0, 1, 99});
        REQUIRE(list.empty());
    }
}
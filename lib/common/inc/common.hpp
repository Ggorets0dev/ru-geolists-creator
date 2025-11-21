#ifndef COMMON_HPP
#define COMMON_HPP

#include <forward_list>
#include <unordered_set>
#include <cstdint>

template <typename T>
void removeListDuplicates(std::forward_list<T>& list) {
    std::unordered_set<T> seen;
    auto prev = list.before_begin();
    auto curr = list.begin();

    while (curr != list.end()) {
        if (seen.find(*curr) != seen.end()) {
            curr = list.erase_after(prev);
        } else {
            seen.insert(*curr);
            prev = curr;
            ++curr;
        }
    }
}

template <typename T>
void removeListItemsForInxs(std::forward_list<T>& lst, const std::forward_list<uint32_t>& inxs) {
    if (inxs.empty()) {
        return;
    }

    auto prev = lst.before_begin();
    auto curr = lst.begin();

    auto idx_it = inxs.begin(); // iterator over indexes to remove
    size_t i = 0;                  // current element index

    while (curr != lst.end() && idx_it != inxs.end()) {
        if (i == *idx_it) {
            // Remove current element
            curr = lst.erase_after(prev);
            ++idx_it; // move to next index to remove
        } else {
            ++prev;
            ++curr;
        }
        ++i;
    }
}

#endif // COMMON_HPP

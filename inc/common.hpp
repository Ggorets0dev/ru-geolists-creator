#ifndef COMMON_HPP
#define COMMON_HPP

#include "forward_list"
#include <unordered_set>

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

#endif // COMMON_HPP

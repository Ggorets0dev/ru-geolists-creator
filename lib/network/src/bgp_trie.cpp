#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include "bgp_trie.hpp"

namespace NetTypes {
    // ──────────────────────────────────────────────────────────────
    // Внутренняя структура Node
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    struct BGPRadixTrie<BITS>::Node {
        std::array<Node*, 2> child{nullptr, nullptr};
        std::optional<IPvxT> route;
        ~Node() { delete child[0]; delete child[1]; }
    };

    // ──────────────────────────────────────────────────────────────
    // ctor/~ctor
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    BGPRadixTrie<BITS>::BGPRadixTrie() : root(new Node()) {}

    template <size_t BITS>
    BGPRadixTrie<BITS>::~BGPRadixTrie() { delete root; }

    // ──────────────────────────────────────────────────────────────
    // Utils
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    int BGPRadixTrie<BITS>::mask_length(const Bitset& mask) {
        int len = 0;
        auto m = mask.to_ullong();
        while (m) { len++; m <<= 1; }
        return BITS - len;
    }

    // ──────────────────────────────────────────────────────────────
    // Insert
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    void BGPRadixTrie<BITS>::insert(const IPvxT& subnet) {
        Node* cur = root;
        auto addr = subnet.ip.to_ullong();
        const int prefix_len = mask_length(subnet.mask);  // например, 24

        int bit_index = 0;
        for (int i = BITS - 1; i >= 0; --i, ++bit_index) {
            // Если мы уже прошли все биты префикса → сохраняем маршрут здесь
            if (bit_index >= prefix_len) {
                cur->route = subnet;
                return;
            }

            int bit = (addr >> i) & 1;
            if (!cur->child[bit]) {
                cur->child[bit] = new Node();
            }
            cur = cur->child[bit];
        }

        // Если дошли до конца (например, /32 или /128)
        cur->route = subnet;
    }

    // ──────────────────────────────────────────────────────────────
    // Search (LPM)
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    std::optional<typename BGPRadixTrie<BITS>::IPvxT>
    BGPRadixTrie<BITS>::lookup(const Bitset& ip) const {
        auto addr = ip.to_ullong();
        Node* cur = root;
        std::optional<IPvxT> best;

        for (int i = BITS - 1; i >= 0; --i) {
            if (cur->route) best = cur->route;
            int bit = (addr >> i) & 1;
            if (!cur->child[bit]) break;
            cur = cur->child[bit];
        }
        if (cur->route) best = cur->route;
        return best;
    }

    // Явная инстанциация для компиляции
    template class BGPRadixTrie<32>;
    template class BGPRadixTrie<128>;
}
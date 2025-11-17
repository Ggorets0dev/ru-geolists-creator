#include <array>

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
    int BGPRadixTrie<BITS>::getMaskLength(const Bitset& mask) {
        int n = 0;
        for (int i = 0; i < BITS; ++i) {
            if (!mask.test(BITS - 1 - i)) {
                break;
            }
            ++n;
        }
        return n;
    }

    // ──────────────────────────────────────────────────────────────
    // Insert
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    void BGPRadixTrie<BITS>::insert(const IPvxT& subnet) {
        Node* cur = root;

        const int prefix_len = getMaskLength(subnet.mask);

        int bit_index = 0;
        for (int i = BITS - 1; i >= 0; --i, ++bit_index) {

            // If we already passed prefix, then save
            if (bit_index >= prefix_len) {
                cur->route = subnet;
                return;
            }

            int bit = subnet.ip.test(i) ? 1 : 0;

            if (!cur->child[bit]) {
                cur->child[bit] = new Node();
            }
            cur = cur->child[bit];
        }

        // If prefix is /32 or /128
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
            if (cur->route) {
                best = cur->route;
            }

            int bit = (addr >> i) & 1;

            if (!cur->child[bit]) {
                break;
            }

            cur = cur->child[bit];
        }

        if (cur->route) {
            best = cur->route;
        }

        return best;
    }

    // ──────────────────────────────────────────────────────────────
    // Other
    // ──────────────────────────────────────────────────────────────
    template <size_t BITS>
    bool BGPRadixTrie<BITS>::isEmpty() const {
        return root->child[0] == nullptr && root->child[1] == nullptr;
    }

    bool TriePair::isEmpty() const {
        return v4.isEmpty() && v6.isEmpty();
    }

    // Явная инстанциация для компиляции
    template class BGPRadixTrie<32>;
    template class BGPRadixTrie<128>;

}
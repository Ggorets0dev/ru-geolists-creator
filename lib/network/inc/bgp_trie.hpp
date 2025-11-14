#ifndef BGP_TRIE_HPP
#define BGP_TRIE_HPP

#include <bitset>
#include <optional>
#include <array>

#include <net_types_base.hpp>

namespace NetTypes {
    template <size_t BITS>
    class BGPRadixTrie {
        using Bitset = std::bitset<BITS>;
        using IPvxT  = IPvx<Bitset>;

        // Forward declaration
        struct Node;

    public:
        BGPRadixTrie();
        ~BGPRadixTrie();

        // Copy ctor is blocked
        BGPRadixTrie(const BGPRadixTrie&) = delete;
        BGPRadixTrie& operator=(const BGPRadixTrie&) = delete;

        // Insert Subnet
        void insert(const IPvxT& subnet);

        // Longest Prefix Match
        std::optional<IPvxT> lookup(const Bitset& ip) const;

    private:
        Node* root;

        // UTILS
        static int mask_length(const Bitset& mask);
    };

    using IPv4Trie = BGPRadixTrie<32>;
    using IPv6Trie = BGPRadixTrie<128>;

    struct TriePair {
        IPv4Trie v4;
        IPv6Trie v6;
    };
}

#endif // BGP_TRIE_HPP
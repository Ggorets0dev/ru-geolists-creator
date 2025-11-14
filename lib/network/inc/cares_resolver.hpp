#ifndef CARES_RESOLVER_HPP
#define CARES_RESOLVER_HPP

#include <future>
#include <ares.h>
#include <vector>

#include "net_types_base.hpp"

// ====================
// Settings for C-Ares resolving
// ====================
#define RESOLVE_BATCH_SIZE              300u
// ====================

namespace NetUtils {
    class CAresResolver {
    public:
        struct ResolveQueryData {
            std::string host;
            std::promise<NetTypes::ListAddress> promise;
        };

        CAresResolver(unsigned int timeoutMs = 2000)
            : m_timeoutMs(timeoutMs), m_initialized(false)
        {
            m_initialized = init();
        }

        ~CAresResolver() {
            cleanup();
        }

        [[nodiscard]]
        bool isInitialized() const {
            return m_initialized;
        }

        bool resolveDomains(const NetTypes::ListAddress& hosts, NetTypes::ListAddress& uniqueIPs);

    private:
        ares_channel m_channel{};
        unsigned int m_timeoutMs;
        bool m_initialized;

        static void resolveCallback(void *arg, int status, int, struct ares_addrinfo *res);

        bool init();

        void cleanup();

        void runEventLoop(std::vector<std::future<std::forward_list<std::string>>>& futures);
    };
}

#endif //CARES_RESOLVER_HPP
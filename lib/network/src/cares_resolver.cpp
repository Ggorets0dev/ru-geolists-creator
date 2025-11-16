#include "cares_resolver.hpp"
#include "log.hpp"
#include "common.hpp"

#include <arpa/inet.h>

void NetUtils::CAresResolver::resolveCallback(void *arg, int status, int, struct ares_addrinfo *res) {
    auto *qd = static_cast<ResolveQueryData*>(arg);
    NetTypes::ListAddress ips;
    if (status == ARES_SUCCESS) {
        char ipstr[INET6_ADDRSTRLEN];
        for (auto *node = res->nodes; node; node = node->ai_next) {
            void *addr = (node->ai_family == AF_INET)
            ? (void*)&((struct sockaddr_in*)node->ai_addr)->sin_addr
            : (void*)&((struct sockaddr_in6*)node->ai_addr)->sin6_addr;
            if (inet_ntop(node->ai_family, addr, ipstr, sizeof(ipstr)))
                ips.emplace_front(ipstr);
        }
        ares_freeaddrinfo(res);
    }

    qd->promise.set_value(std::move(ips));
}

bool NetUtils::CAresResolver::resolveDomains(const NetTypes::ListAddress& hosts, NetTypes::ListAddress& uniqueIPs) {
    if (!m_initialized) {
        LOG_ERROR("Tried to call not initialized CAres domain reslover");
        return false;
    }

    std::vector<ResolveQueryData> queries;
    std::vector<std::future<std::forward_list<std::string>>> futures;

    const size_t hostsSize = std::distance(hosts.begin(), hosts.end());

    queries.reserve(hostsSize);
    futures.reserve(hostsSize);

    for (auto &h : hosts) {
        queries.push_back({h});
        futures.push_back(queries.back().promise.get_future());

        ares_addrinfo_hints hints{};
        hints.ai_family = AF_UNSPEC;
        ares_getaddrinfo(m_channel, h.c_str(), nullptr, &hints, resolveCallback, &queries.back());
    }

    runEventLoop(futures);

    for (auto &f : futures) {
        auto ips = f.get();
        for (auto &ip : ips) {
            uniqueIPs.push_front(ip);
        }
    }

    removeListDuplicates(uniqueIPs);

    return (std::distance(uniqueIPs.begin(), uniqueIPs.end()) != 0);
}

bool NetUtils::CAresResolver::init() {
    int status = ares_library_init(ARES_LIB_INIT_ALL);

    if (status != ARES_SUCCESS) {
        LOG_ERROR("Failed to init CAres library: " + std::string(ares_strerror(status)));
        return false;
    }

    ares_options options{};

    int optmask = 0;
    options.timeout = m_timeoutMs;
    optmask |= ARES_OPT_TIMEOUTMS;

    status = ares_init_options(&m_channel, &options, optmask);
    if (status != ARES_SUCCESS) {
        LOG_ERROR("Failed to init C-Ares control block: " + std::string(ares_strerror(status)));
        ares_library_cleanup();
        return false;
    }

    return true;
}

void NetUtils::CAresResolver::runEventLoop(std::vector<std::future<std::forward_list<std::string>>>& futures) {
    bool done = false;
    while (!done) {
        fd_set rfds, wfds;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        int socks[ARES_GETSOCK_MAXNUM];
        int bitmask = ares_getsock(m_channel, socks, ARES_GETSOCK_MAXNUM);
        int maxfd = -1;

        for (int i = 0; i < ARES_GETSOCK_MAXNUM; i++) {
            if (ARES_GETSOCK_READABLE(bitmask, i)) {
                FD_SET(socks[i], &rfds);
                if (socks[i] > maxfd) maxfd = socks[i];
            }
            if (ARES_GETSOCK_WRITABLE(bitmask, i)) {
                FD_SET(socks[i], &wfds);
                if (socks[i] > maxfd) maxfd = socks[i];
            }
        }

        timeval tv{};
        auto tvp = ares_timeout(m_channel, nullptr, &tv);

        int nfds = (maxfd >= 0) ? maxfd + 1 : 0;
        int rc = select(nfds, &rfds, &wfds, nullptr, tvp);

        if (rc >= 0) {
            ares_process(m_channel, &rfds, &wfds);
        }

        done = std::all_of(futures.begin(), futures.end(),
                           [](auto &f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; });
    }
}

void NetUtils::CAresResolver::cleanup() {
    if (m_initialized) {
        ares_destroy(m_channel);
        ares_library_cleanup();
        m_initialized = false;
    }
}
#ifndef LIBNETWORK_SETTINGS
#define LIBNETWORK_SETTINGS

#include <string>

/**
 * @brief Network library configuration structure.
 */
struct LibNetworkSettings {

    /** @brief Whether to discover subnet using BGP data. */
    bool isSearchSubnetByBGP = false;

    /** @brief Path to the BGP dump file (used when isSearchSubnetByBGP == true). */
    std::string bgpDumpPath;

    /** @brief Limit of IP mask, which can be fixed using BGP dump */
    struct AutoFixMaskLimitByBGP {
        unsigned int v4 = 20u;
        unsigned int v6 = 32u;
    };

    AutoFixMaskLimitByBGP autoFixMaskLimitByBGP;

    /** @brief Maximum time (seconds) for the whole cURL operation to complete. */
    unsigned int curlOperationTimeoutSec = 10u;

    /** @brief Maximum time (seconds) to establish a connection with the server. */
    unsigned int curlConnectionTimeoutSec = 5u;

    /** @brief Number of attempts to establish a network connection. */
    unsigned int connectAttemptsCount = 3u;

    /** @brief Delay (seconds) between successive connection attempts. */
    unsigned int connectAttemptDelaySec = 3u;

    /** @brief Number of attempts to download a resource. */
    unsigned int downloadAttemptCount = 3u;

    /** @brief Delay (seconds) between successive download attempts. */
    unsigned int downloadAttemptDelaySec = 3u;
};

extern LibNetworkSettings gLibNetworkSettings;

#endif //LIBNETWORK_SETTINGS
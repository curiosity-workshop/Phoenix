#pragma once

#include <phoenix/discovery/DiscoveredDevice.h>
#include <phoenix/transport/IByteTransport.h>

#include <chrono>
#include <optional>

namespace phoenix::discovery
{
    class LegacyXplproProbe
    {
    public:
        explicit LegacyXplproProbe(
            std::chrono::milliseconds retryInterval =
            std::chrono::milliseconds{ 250 },
            std::chrono::milliseconds timeout =
            std::chrono::seconds{ 4 });

        std::optional<DiscoveredDevice> probe(
            transport::IByteTransport& transport) const;

    private:
        std::chrono::milliseconds retryInterval_;
        std::chrono::milliseconds timeout_;
    };
}
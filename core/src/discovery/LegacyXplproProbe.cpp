#include <phoenix/discovery/LegacyXplproProbe.h>
#include <phoenix/protocol/legacy/LegacyFrame.h>
#include <phoenix/protocol/legacy/LegacyFrameParser.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <thread>

namespace phoenix::discovery
{
    namespace
    {
        std::string extractQuotedString(std::string_view payload)
        {
            const std::size_t firstQuote = payload.find('"');

            if (firstQuote == std::string_view::npos)
            {
                return {};
            }

            const std::size_t secondQuote =
                payload.find('"', firstQuote + 1);

            if (secondQuote == std::string_view::npos)
            {
                return {};
            }

            return std::string{
                payload.substr(
                firstQuote + 1,
                secondQuote - firstQuote - 1)
            };
        }
    }

    LegacyXplproProbe::LegacyXplproProbe(
        std::chrono::milliseconds retryInterval,
        std::chrono::milliseconds timeout)
        : retryInterval_(retryInterval),
        timeout_(timeout)
    {
    }

    std::optional<DiscoveredDevice>
        LegacyXplproProbe::probe(
            transport::IByteTransport& transport) const
    {
        if (!transport.isOpen())
        {
            return std::nullopt;
        }

        DiscoveredDevice discoveredDevice;

        std::array<std::byte, 256> readBuffer{};
        protocol::legacy::LegacyFrameParser parser;

        const auto request =
            protocol::legacy::makeFrame(
                protocol::legacy::sendNameCommand);

        const auto startTime =
            std::chrono::steady_clock::now();

        const auto deadline =
            startTime + timeout_;

        auto nextRequestTime = startTime;

        while (std::chrono::steady_clock::now() < deadline)
        {
            const auto currentTime =
                std::chrono::steady_clock::now();

            if (currentTime >= nextRequestTime)
            {
                transport.write(request);

                nextRequestTime =
                    currentTime + retryInterval_;
            }

            const std::size_t bytesRead =
                transport.read(readBuffer);

            const auto frames =
                parser.push(
                    std::span<const std::byte>{
                        readBuffer.data(),
                        bytesRead
                    });

            for (const auto& frame : frames)
            {
                if (frame.command ==
                    protocol::legacy::nameResponseCommand)
                {
                    discoveredDevice.name =
                        extractQuotedString(frame.payload);
                }
                else if (frame.command ==
                    protocol::legacy::versionResponseCommand)
                {
                    discoveredDevice.version =
                        extractQuotedString(frame.payload);
                }

                /*
                 * The name response is enough to prove that this is
                 * an XPLPro device. Version is useful but optional.
                 */
                if (!discoveredDevice.name.empty())
                {
                    return discoveredDevice;
                }
            }

            /*
             * Avoid consuming an entire processor core while still
             * checking often enough for responsive serial handling.
             */
            std::this_thread::sleep_for(
                std::chrono::milliseconds{ 10 });
        }

        return std::nullopt;
    }
}

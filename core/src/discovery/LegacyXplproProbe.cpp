#include <phoenix/discovery/LegacyXplproProbe.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <thread>

namespace phoenix::discovery
{
    namespace
    {
        constexpr char packetHeader = '[';
        constexpr char packetTrailer = ']';

        constexpr char sendNameCommand = 'N';

        // Verify these against the legacy XPLPro command definitions.
        constexpr char nameResponseCommand = 'n';
        constexpr char versionResponseCommand = 'v';

        constexpr std::size_t maximumFrameSize = 256;

        std::array<std::byte, 3> makeIdentityRequest()
        {
            return {
                static_cast<std::byte>(packetHeader),
                static_cast<std::byte>(sendNameCommand),
                static_cast<std::byte>(packetTrailer)
            };
        }

        std::string extractQuotedString(const std::string& frame)
        {
            const std::size_t firstQuote = frame.find('"');

            if (firstQuote == std::string::npos)
            {
                return {};
            }

            const std::size_t secondQuote =
                frame.find('"', firstQuote + 1);

            if (secondQuote == std::string::npos)
            {
                return {};
            }

            return frame.substr(
                firstQuote + 1,
                secondQuote - firstQuote - 1);
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
        std::string receiveBuffer;

        const auto request = makeIdentityRequest();

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

            for (std::size_t index = 0;
                index < bytesRead;
                ++index)
            {
                const char receivedCharacter =
                    static_cast<char>(
                        std::to_integer<unsigned char>(
                            readBuffer[index]));

                /*
                 * Discard anything received before a packet header.
                 */
                if (receiveBuffer.empty())
                {
                    if (receivedCharacter == packetHeader)
                    {
                        receiveBuffer.push_back(
                            receivedCharacter);
                    }

                    continue;
                }

                /*
                 * A new header inside an unfinished frame means the
                 * previous frame was incomplete. Start over using the
                 * new header.
                 */
                if (receivedCharacter == packetHeader)
                {
                    receiveBuffer.clear();
                    receiveBuffer.push_back(receivedCharacter);
                    continue;
                }

                receiveBuffer.push_back(receivedCharacter);

                /*
                 * An overlength packet is considered corrupt or
                 * incomplete. Discard it and wait for another header.
                 */
                if (receiveBuffer.size() > maximumFrameSize)
                {
                    receiveBuffer.clear();
                    continue;
                }

                if (receivedCharacter != packetTrailer)
                {
                    continue;
                }

                /*
                 * A minimum valid legacy frame contains:
                 *
                 *     [ command ]
                 */
                if (receiveBuffer.size() >= 3)
                {
                    const char command = receiveBuffer[1];

                    if (command == nameResponseCommand)
                    {
                        discoveredDevice.name =
                            extractQuotedString(receiveBuffer);
                    }
                    else if (command == versionResponseCommand)
                    {
                        discoveredDevice.version =
                            extractQuotedString(receiveBuffer);
                    }
                }

                receiveBuffer.clear();

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
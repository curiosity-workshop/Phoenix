#include <phoenix/protocol/legacy/LegacyFrameParser.h>

namespace phoenix::protocol::legacy
{
    std::vector<LegacyFrame> LegacyFrameParser::push(
        std::span<const std::byte> bytes)
    {
        std::vector<LegacyFrame> frames;

        for (const std::byte value : bytes)
        {
            const char character =
                static_cast<char>(
                    std::to_integer<unsigned char>(value));

            if (buffer_.empty())
            {
                if (character == packetHeader)
                {
                    buffer_.push_back(character);
                }

                continue;
            }

            if (character == packetHeader)
            {
                buffer_.clear();
                buffer_.push_back(character);
                continue;
            }

            buffer_.push_back(character);

            if (buffer_.size() > maximumFrameSize)
            {
                buffer_.clear();
                continue;
            }

            if (character != packetTrailer)
            {
                continue;
            }

            if (buffer_.size() >= 3)
            {
                frames.push_back({
                    buffer_[1],
                    buffer_.substr(2, buffer_.size() - 3),
                    buffer_
                });
            }

            buffer_.clear();
        }

        return frames;
    }

    void LegacyFrameParser::reset()
    {
        buffer_.clear();
    }

    bool LegacyFrameParser::hasPartialFrame() const
    {
        return !buffer_.empty();
    }
}

#include <cockpitlink/protocol/Frame.h>
#include <cockpitlink/protocol/FrameParser.h>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

namespace
{
    std::vector<std::byte> bytes(
        std::string_view text)
    {
        std::vector<std::byte> result;
        result.reserve(text.size());

        for (const char character : text)
        {
            result.push_back(
                static_cast<std::byte>(character));
        }

        return result;
    }

    bool expect(
        bool condition,
        std::string_view message)
    {
        if (!condition)
        {
            std::cerr << message << '\n';
            return false;
        }

        return true;
    }
}

int main()
{
    using cockpitlink::protocol::Frame;
    using cockpitlink::protocol::FrameParser;
    using cockpitlink::protocol::MessageType;
    using cockpitlink::protocol::encodeFrame;

    bool passed = true;

    const std::vector<std::byte> payload{
        std::byte{ 'A' },
        std::byte{ 0 },
        std::byte{ 'B' },
        std::byte{ 'C' }
    };

    const auto encoded =
        encodeFrame({
            MessageType::Hello,
            3,
            42,
            payload
        });

    {
        FrameParser parser;

        const auto first =
            parser.push(
                std::span<const std::byte>{
                    encoded.data(),
                    5
                });

        passed &= expect(
            first.empty(),
            "partial frame should not emit");
        passed &= expect(
            parser.hasPartialFrame(),
            "partial frame should be tracked");

        const auto second =
            parser.push(
                std::span<const std::byte>{
                    encoded.data() + 5,
                    encoded.size() - 5
                });

        passed &= expect(
            second.size() == 1,
            "completed frame should emit");
        passed &= expect(
            second[0].type == MessageType::Hello,
            "message type failed");
        passed &= expect(
            second[0].flags == 3,
            "flags failed");
        passed &= expect(
            second[0].sequence == 42,
            "sequence failed");
        passed &= expect(
            second[0].payload == payload,
            "payload failed");
        passed &= expect(
            !parser.hasPartialFrame(),
            "completed frame should clear parser state");
    }

    {
        FrameParser parser;

        auto corrupted =
            encoded;
        corrupted[10] =
            static_cast<std::byte>(
                std::to_integer<std::uint8_t>(corrupted[10]) ^ 0xff);

        const auto frames =
            parser.push(corrupted);

        passed &= expect(
            frames.empty(),
            "bad checksum should reject frame");
        passed &= expect(
            !parser.hasPartialFrame(),
            "bad checksum should clear parser state");
    }

    {
        FrameParser parser;

        const auto noise =
            bytes("noise");
        auto stream =
            noise;
        stream.insert(
            stream.end(),
            encoded.begin(),
            encoded.end());

        const auto frames =
            parser.push(stream);

        passed &= expect(
            frames.size() == 1,
            "parser should skip leading noise");
    }

    return passed ? 0 : 1;
}

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace cockpitlink::protocol
{
    constexpr std::byte magic0{ 'C' };
    constexpr std::byte magic1{ 'L' };
    constexpr std::uint8_t protocolVersion = 2;
    constexpr std::size_t headerSize = 9;
    constexpr std::size_t checksumSize = 4;
    constexpr std::size_t maximumPayloadSize = 4096;

    enum class MessageType : std::uint8_t
    {
        Hello = 1,
        HelloAck = 2,
        BehaviorRequest = 10,
        ProfileAccepted = 11,
        Subscribe = 20,
        ValueUpdate = 21,
        CommandAction = 30,
        FlowControl = 40,
        Diagnostic = 50
    };

    struct Frame
    {
        MessageType type = MessageType::Diagnostic;
        std::uint8_t flags = 0;
        std::uint16_t sequence = 0;
        std::vector<std::byte> payload;
    };

    std::vector<std::byte> encodeFrame(
        const Frame& frame);

    std::uint32_t crc32(
        std::span<const std::byte> bytes);
}

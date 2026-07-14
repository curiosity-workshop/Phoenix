#include <phoenix/protocol/legacy/LegacyFrame.h>
#include <phoenix/protocol/legacy/LegacyFrameParser.h>
#include <phoenix/protocol/legacy/LegacyMessage.h>

#include <cstddef>
#include <iostream>
#include <span>
#include <string_view>
#include <variant>
#include <vector>

namespace
{
    std::vector<std::byte> bytes(std::string_view text)
    {
        std::vector<std::byte> result;
        result.reserve(text.size());

        for (const char character : text)
        {
            result.push_back(static_cast<std::byte>(character));
        }

        return result;
    }

    bool expect(bool condition, std::string_view message)
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
    using phoenix::protocol::legacy::LegacyFrameParser;
    using phoenix::protocol::legacy::CommandAction;
    using phoenix::protocol::legacy::CommandEvent;
    using phoenix::protocol::legacy::DataRefUpdate;
    using phoenix::protocol::legacy::DataRefValueType;
    using phoenix::protocol::legacy::DeviceName;
    using phoenix::protocol::legacy::NoMoreRequests;
    using phoenix::protocol::legacy::RegisterCommand;
    using phoenix::protocol::legacy::StringDataRefUpdate;
    using phoenix::protocol::legacy::UpdatesRequest;
    using phoenix::protocol::legacy::decodeFrame;
    using phoenix::protocol::legacy::makeFrame;

    bool passed = true;

    {
        const auto frame = makeFrame('N');
        passed &= expect(frame == bytes("[N]"), "identity frame encoding failed");
    }

    {
        LegacyFrameParser parser;

        const auto first = parser.push(bytes("noise[n,\"XPL"));
        passed &= expect(first.empty(), "partial frame should not emit");
        passed &= expect(parser.hasPartialFrame(), "partial frame should be retained");

        const auto second = parser.push(bytes("Pro\"]tail"));
        passed &= expect(second.size() == 1, "completed frame should emit once");
        passed &= expect(second[0].command == 'n', "name command parse failed");
        passed &= expect(second[0].payload == ",\"XPLPro\"", "name payload parse failed");
        passed &= expect(second[0].raw == "[n,\"XPLPro\"]", "raw frame parse failed");
        passed &= expect(!parser.hasPartialFrame(), "completed frame should clear buffer");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[bad[N]"));
        passed &= expect(frames.size() == 1, "new header should restart frame");
        passed &= expect(frames[0].command == 'N', "restarted frame command failed");
    }

    {
        LegacyFrameParser parser;

        auto oversized = bytes("[");
        oversized.resize(
            phoenix::protocol::legacy::maximumFrameSize + 1,
            static_cast<std::byte>('x'));

        const auto frames = parser.push(oversized);
        passed &= expect(frames.empty(), "oversized frame should be discarded");
        passed &= expect(!parser.hasPartialFrame(), "oversized frame should clear buffer");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[n,\"Device\"] "));
        passed &= expect(frames.size() == 1, "trailing noise should be ignored");

        const auto message = decodeFrame(frames[0]);
        const auto* name = std::get_if<DeviceName>(&message);
        passed &= expect(name != nullptr, "name response should decode");
        passed &= expect(name && name->value == "Device", "name response value failed");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[m,\"sim/radios/com1_standy_flip\"]"));
        passed &= expect(frames.size() == 1, "command registration frame should parse");

        const auto message = decodeFrame(frames[0]);
        const auto* command = std::get_if<RegisterCommand>(&message);
        passed &= expect(command != nullptr, "command registration should decode");
        passed &= expect(
            command && command->name == "sim/radios/com1_standy_flip",
            "command registration name failed");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[4,7,123.45,1]"));
        passed &= expect(frames.size() == 1, "float array update frame should parse");

        const auto message = decodeFrame(frames[0]);
        const auto* update = std::get_if<DataRefUpdate>(&message);
        passed &= expect(update != nullptr, "float array update should decode");
        passed &= expect(
            update && update->type == DataRefValueType::FloatArray,
            "float array update type failed");
        passed &= expect(update && update->handle == 7, "float array update handle failed");
        passed &= expect(update && update->value == "123.45", "float array update value failed");
        passed &= expect(
            update && update->element && *update->element == 1,
            "float array update element failed");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[r,2,100,0.0100]"));
        passed &= expect(frames.size() == 1, "updates request frame should parse");

        const auto message = decodeFrame(frames[0]);
        const auto* request = std::get_if<UpdatesRequest>(&message);
        passed &= expect(request != nullptr, "updates request should decode");
        passed &= expect(request && request->handle == 2, "updates request handle failed");
        passed &= expect(request && request->rate == 100, "updates request rate failed");
        passed &= expect(
            request && request->precision == "0.0100",
            "updates request precision failed");
        passed &= expect(
            request && !request->element,
            "non-array updates request should not have element");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[k,4,3]"));
        passed &= expect(frames.size() == 1, "command trigger frame should parse");

        const auto message = decodeFrame(frames[0]);
        const auto* command = std::get_if<CommandEvent>(&message);
        passed &= expect(command != nullptr, "command trigger should decode");
        passed &= expect(
            command && command->action == CommandAction::Trigger,
            "command trigger action failed");
        passed &= expect(command && command->handle == 4, "command trigger handle failed");
        passed &= expect(
            command && command->triggerCount == 3,
            "command trigger count failed");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[9,12,10]raw-string"));
        passed &= expect(frames.size() == 1, "string update metadata frame should parse");

        const auto message = decodeFrame(frames[0]);
        const auto* update = std::get_if<StringDataRefUpdate>(&message);
        passed &= expect(update != nullptr, "string update metadata should decode");
        passed &= expect(update && update->handle == 12, "string update handle failed");
        passed &= expect(update && update->byteCount == 10, "string update byte count failed");
    }

    {
        LegacyFrameParser parser;

        const auto frames = parser.push(bytes("[c]"));
        passed &= expect(frames.size() == 1, "no-more-requests frame should parse");

        const auto message = decodeFrame(frames[0]);
        passed &= expect(
            std::holds_alternative<NoMoreRequests>(message),
            "no-more-requests should decode as inert legacy message");
    }

    return passed ? 0 : 1;
}

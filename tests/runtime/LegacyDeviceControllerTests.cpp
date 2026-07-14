#include <phoenix/runtime/LegacyDeviceController.h>

#include <algorithm>
#include <cstddef>
#include <deque>
#include <iostream>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{
    class FakeTransport final : public phoenix::transport::IByteTransport
    {
    public:
        bool open() override
        {
            open_ = true;
            return true;
        }

        void close() override
        {
            open_ = false;
        }

        bool isOpen() const override
        {
            return open_;
        }

        std::size_t read(std::span<std::byte> buffer) override
        {
            if (incoming_.empty())
            {
                return 0;
            }

            const auto chunk = incoming_.front();
            incoming_.pop_front();

            const std::size_t bytesToRead =
                std::min(buffer.size(), chunk.size());

            std::copy_n(
                chunk.begin(),
                bytesToRead,
                buffer.begin());

            if (bytesToRead < chunk.size())
            {
                incoming_.push_front({
                    chunk.begin() + bytesToRead,
                    chunk.end()
                });
            }

            return bytesToRead;
        }

        std::size_t write(std::span<const std::byte> data) override
        {
            written_.insert(
                written_.end(),
                data.begin(),
                data.end());

            return data.size();
        }

        void pushIncoming(std::string_view text)
        {
            std::vector<std::byte> bytes;
            bytes.reserve(text.size());

            for (const char character : text)
            {
                bytes.push_back(static_cast<std::byte>(character));
            }

            incoming_.push_back(std::move(bytes));
        }

        std::string writtenText() const
        {
            std::string result;
            result.reserve(written_.size());

            for (const std::byte value : written_)
            {
                result.push_back(
                    static_cast<char>(
                        std::to_integer<unsigned char>(value)));
            }

            return result;
        }

    private:
        bool open_ = true;
        std::deque<std::vector<std::byte>> incoming_;
        std::vector<std::byte> written_;
    };

    class FakeXPlaneBridge final : public phoenix::xplane::IXPlaneBridge
    {
    public:
        phoenix::xplane::DataRefLookupResult findDataRef(
            std::string_view name) override
        {
            const auto found =
                dataRefs.find(std::string{ name });

            if (found == dataRefs.end())
            {
                return {};
            }

            return {
                true,
                found->second
            };
        }

        phoenix::xplane::CommandLookupResult findCommand(
            std::string_view name) override
        {
            return {
                std::find(
                    commands.begin(),
                    commands.end(),
                    std::string{ name }) != commands.end()
            };
        }

        void writeDataRef(
            const phoenix::xplane::DataRefWrite& write) override
        {
            dataRefWrites.push_back(write);
        }

        void touchDataRef(
            std::string_view name,
            int handle) override
        {
            touchedDataRefs.push_back({
                std::string{ name },
                handle
            });
        }

        void commandBegin(
            std::string_view name,
            int handle) override
        {
            commandEvents.push_back(
                "begin:" + std::string{ name } + ":" + std::to_string(handle));
        }

        void commandEnd(
            std::string_view name,
            int handle) override
        {
            commandEvents.push_back(
                "end:" + std::string{ name } + ":" + std::to_string(handle));
        }

        void commandTrigger(
            std::string_view name,
            int handle,
            int triggerCount) override
        {
            commandEvents.push_back(
                "trigger:" + std::string{ name } + ":" +
                std::to_string(handle) + ":" +
                std::to_string(triggerCount));
        }

        void debugMessage(std::string_view message) override
        {
            debugMessages.push_back(std::string{ message });
        }

        void speak(std::string_view message) override
        {
            spokenMessages.push_back(std::string{ message });
        }

        void resetRequested() override
        {
            resetCount++;
        }

        std::map<std::string, int> dataRefs;
        std::vector<std::string> commands;
        std::vector<phoenix::xplane::DataRefWrite> dataRefWrites;
        std::vector<std::pair<std::string, int>> touchedDataRefs;
        std::vector<std::string> commandEvents;
        std::vector<std::string> debugMessages;
        std::vector<std::string> spokenMessages;
        int resetCount = 0;
    };

    class FakeLegacyDeviceObserver final
        : public phoenix::runtime::ILegacyDeviceObserver
    {
    public:
        void microcontrollerRequestedDataRef(
            std::string_view name) override
        {
            requests.push_back(
                "dataref:" + std::string{ name });
        }

        void microcontrollerRequestedCommand(
            std::string_view name) override
        {
            requests.push_back(
                "command:" + std::string{ name });
        }

        void microcontrollerRequestedUpdates(
            const phoenix::protocol::legacy::UpdatesRequest& request) override
        {
            requests.push_back(
                "updates:" +
                std::to_string(request.handle) + ":" +
                std::to_string(request.rate) + ":" +
                request.precision);
        }

        void microcontrollerRequestedScaling(
            const phoenix::protocol::legacy::ScalingRequest& request) override
        {
            requests.push_back(
                "scaling:" +
                std::to_string(request.handle) + ":" +
                std::to_string(request.fromLow) + ":" +
                std::to_string(request.fromHigh) + ":" +
                std::to_string(request.toLow) + ":" +
                std::to_string(request.toHigh));
        }

        std::vector<std::string> requests;
    };

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
    using phoenix::runtime::LegacyDeviceController;
    using phoenix::runtime::LegacyDeviceSession;
    using phoenix::runtime::LegacyDeviceSessionOptions;
    using phoenix::xplane::DataRefTypeFloat;
    using phoenix::xplane::DataRefTypeInt;

    bool passed = true;

    FakeTransport transport;
    FakeXPlaneBridge xplane;
    FakeLegacyDeviceObserver observer;

    xplane.dataRefs["sim/test/int"] = DataRefTypeInt;
    xplane.dataRefs["sim/test/float"] = DataRefTypeFloat;
    xplane.commands.push_back("sim/test/command");

    LegacyDeviceSession session{
        transport,
        LegacyDeviceSessionOptions{
            .readBufferSize = 64,
            .maximumReadPasses = 8
        }
    };

    LegacyDeviceController controller{
        session,
        xplane,
        observer
    };

    controller.requestRegistrations();
    auto tick = controller.tick();

    passed &= expect(controller.registrationsRequested(), "registration phase should be tracked");
    passed &= expect(tick.session.bytesWritten == 3, "registration query should flush");
    passed &= expect(transport.writtenText() == "[Q]", "registration query frame failed");

    transport.pushIncoming("[b,\"sim/test/int\"]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "dataref registration should process");
    passed &= expect(transport.writtenText() == "[Q][D,0]", "dataref registration response failed");
    passed &= expect(controller.dataRefs().size() == 1, "dataref binding should be retained");
    passed &= expect(
        controller.dataRefs()[0].name == "sim/test/int",
        "dataref binding name failed");
    passed &= expect(
        observer.requests.size() == 1 &&
        observer.requests[0] == "dataref:sim/test/int",
        "dataref request should be observable");

    transport.pushIncoming("[m,\"sim/test/command\"]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "command registration should process");
    passed &= expect(
        transport.writtenText() == "[Q][D,0][C,0]",
        "command registration response failed");
    passed &= expect(controller.commands().size() == 1, "command binding should be retained");
    passed &= expect(
        observer.requests.size() == 2 &&
        observer.requests[1] == "command:sim/test/command",
        "command request should be observable");

    transport.pushIncoming("[1,0,42]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "dataref update should process");
    passed &= expect(xplane.dataRefWrites.size() == 1, "dataref update should reach xplane");
    passed &= expect(
        xplane.dataRefWrites[0].name == "sim/test/int",
        "dataref update name failed");
    passed &= expect(xplane.dataRefWrites[0].value == "42", "dataref update value failed");

    transport.pushIncoming("[r,0,100,0.0000]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "updates request should process");
    passed &= expect(
        controller.updateSubscriptions().size() == 1,
        "updates request should be retained");
    passed &= expect(
        controller.updateSubscriptions()[0].forceUpdate,
        "updates request should force initial update");
    passed &= expect(
        observer.requests.size() == 3 &&
        observer.requests[2] == "updates:0:100:0.0000",
        "updates request should be observable");

    transport.pushIncoming("[u,0,0,1024,0,1]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "scaling request should process");
    passed &= expect(controller.dataRefs()[0].scalingActive, "scaling should be active");
    passed &= expect(controller.dataRefs()[0].scaleFromHigh == 1024, "scaling range failed");
    passed &= expect(
        observer.requests.size() == 4 &&
        observer.requests[3] == "scaling:0:0:1024:0:1",
        "scaling request should be observable");

    transport.pushIncoming("[k,0,3]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "command trigger should process");
    passed &= expect(
        xplane.commandEvents.size() == 1 &&
        xplane.commandEvents[0] == "trigger:sim/test/command:0:3",
        "command trigger should reach xplane");

    transport.pushIncoming("[g,\"hello\"]");
    transport.pushIncoming("[s,\"speak\"]");
    transport.pushIncoming("[z,0]");
    transport.pushIncoming("[p]");
    transport.pushIncoming("[q]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 5, "control messages should process");
    passed &= expect(
        xplane.debugMessages.size() == 1 &&
        xplane.debugMessages[0] == "hello",
        "debug message should reach xplane");
    passed &= expect(
        xplane.spokenMessages.size() == 1 &&
        xplane.spokenMessages[0] == "speak",
        "speak message should reach xplane");
    passed &= expect(xplane.resetCount == 1, "reset should reach xplane");
    passed &= expect(!controller.dataFlowPaused(), "flow should be resumed");

    transport.pushIncoming("[b,\"sim/missing\"]");
    tick = controller.tick();

    passed &= expect(tick.messagesProcessed == 1, "missing dataref should process");
    passed &= expect(
        transport.writtenText().ends_with("[D,-02,\"sim/missing\"]"),
        "missing dataref response failed");

    return passed ? 0 : 1;
}

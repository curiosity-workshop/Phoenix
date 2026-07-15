#include <phoenix/runtime/LegacyDeviceController.h>
#include <phoenix/runtime/LegacyDeviceSession.h>
#include <phoenix/runtime/LegacyUpdateScheduler.h>

#include <algorithm>
#include <chrono>
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
            dataRefLookupNames.push_back(std::string{ name });

            return {
                true,
                dataRefType
            };
        }

        phoenix::xplane::CommandLookupResult findCommand(
            std::string_view) override
        {
            return { false };
        }

        void writeDataRef(
            const phoenix::xplane::DataRefWrite&) override
        {
        }

        phoenix::xplane::DataRefReadResult readDataRef(
            const phoenix::xplane::DataRefReadRequest& request) override
        {
            readRequests.push_back(request);

            return {
                true,
                request.preferredType.value_or(phoenix::xplane::DataRefTypeInt),
                currentValue,
                request.element
            };
        }

        void touchDataRef(std::string_view, int) override
        {
        }

        void commandBegin(std::string_view, int) override
        {
        }

        void commandEnd(std::string_view, int) override
        {
        }

        void commandTrigger(std::string_view, int, int) override
        {
        }

        void debugMessage(std::string_view) override
        {
        }

        void speak(std::string_view) override
        {
        }

        void resetRequested() override
        {
        }

        int dataRefType = phoenix::xplane::DataRefTypeInt;
        std::string currentValue = "1";
        std::vector<std::string> dataRefLookupNames;
        std::vector<phoenix::xplane::DataRefReadRequest> readRequests;
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
    using phoenix::runtime::LegacyUpdateScheduler;

    bool passed = true;

    FakeTransport transport;
    FakeXPlaneBridge xplane;

    LegacyDeviceSession session{
        transport,
        LegacyDeviceSessionOptions{
            .readBufferSize = 64,
            .maximumReadPasses = 8
        }
    };

    LegacyDeviceController controller{
        session,
        xplane
    };

    LegacyUpdateScheduler scheduler{
        session,
        controller,
        xplane
    };

    transport.pushIncoming("[b,\"sim/test/int\"]");
    auto controllerTick = controller.tick();

    passed &= expect(
        controllerTick.messagesProcessed == 1,
        "dataref registration should process");
    passed &= expect(
        controller.dataRefs().size() == 1,
        "dataref should be registered");

    transport.pushIncoming("[r,0,100,0.0000]");
    controllerTick = controller.tick();

    passed &= expect(
        controller.updateSubscriptions().size() == 1,
        "update subscription should be registered");

    const auto start =
        std::chrono::steady_clock::time_point{};

    auto schedulerTick =
        scheduler.tick(start);

    passed &= expect(
        schedulerTick.updatesQueued == 1,
        "first scheduler tick should force an update");
    passed &= expect(
        transport.writtenText() == "[D,0][1,0,1]",
        "first scheduler update frame failed");
    passed &= expect(
        xplane.readRequests.size() == 1 &&
        xplane.readRequests[0].name == "sim/test/int",
        "scheduler should read the bound dataref");

    schedulerTick =
        scheduler.tick(start + std::chrono::milliseconds{ 50 });

    passed &= expect(
        schedulerTick.updatesQueued == 0,
        "scheduler should honor subscription rate");

    schedulerTick =
        scheduler.tick(start + std::chrono::milliseconds{ 100 });

    passed &= expect(
        schedulerTick.updatesQueued == 0,
        "scheduler should suppress unchanged values");

    xplane.currentValue = "0";
    schedulerTick =
        scheduler.tick(start + std::chrono::milliseconds{ 200 });

    passed &= expect(
        schedulerTick.updatesQueued == 1,
        "scheduler should send changed values");
    passed &= expect(
        transport.writtenText().ends_with("[1,0,0]"),
        "changed scheduler update frame failed");

    transport.pushIncoming("[p]");
    controllerTick = controller.tick();

    xplane.currentValue = "1";
    schedulerTick =
        scheduler.tick(start + std::chrono::milliseconds{ 300 });

    passed &= expect(
        controller.dataFlowPaused(),
        "pause frame should pause data flow");
    passed &= expect(
        schedulerTick.updatesQueued == 0,
        "scheduler should not send while data flow is paused");

    {
        FakeTransport budgetTransport;
        FakeXPlaneBridge budgetXPlane;

        LegacyDeviceSession budgetSession{
            budgetTransport,
            LegacyDeviceSessionOptions{
                .readBufferSize = 128,
                .maximumReadPasses = 8
            }
        };

        LegacyDeviceController budgetController{
            budgetSession,
            budgetXPlane
        };

        LegacyUpdateScheduler budgetScheduler{
            budgetSession,
            budgetController,
            budgetXPlane,
            phoenix::runtime::LegacyUpdateSchedulerOptions{
                .maxFramesPerTick = 2
            }
        };

        budgetTransport.pushIncoming(
            "[b,\"sim/test/0\"][b,\"sim/test/1\"][b,\"sim/test/2\"]"
            "[r,0,100,0.0000][r,1,100,0.0000][r,2,100,0.0000]");

        const auto budgetControllerTick =
            budgetController.tick();

        passed &= expect(
            budgetControllerTick.messagesProcessed == 6,
            "budget setup messages should process");
        passed &= expect(
            budgetController.updateSubscriptions().size() == 3,
            "budget setup should retain three subscriptions");

        auto budgetTick =
            budgetScheduler.tick(start);

        passed &= expect(
            budgetTick.updatesQueued == 2,
            "frame budget should limit the first scheduler tick");
        passed &= expect(
            budgetTick.budgetExhausted,
            "frame budget should report exhaustion");
        passed &= expect(
            budgetTransport.writtenText().ends_with("[1,0,1][1,1,1]"),
            "frame budget first batch failed");

        budgetTick =
            budgetScheduler.tick(start);

        passed &= expect(
            budgetTick.updatesQueued == 1,
            "budgeted scheduler should send deferred update next tick");
        passed &= expect(
            !budgetTick.budgetExhausted,
            "deferred final update should fit the next tick budget");
        passed &= expect(
            budgetTransport.writtenText().ends_with("[1,2,1]"),
            "frame budget deferred update failed");
    }

    return passed ? 0 : 1;
}

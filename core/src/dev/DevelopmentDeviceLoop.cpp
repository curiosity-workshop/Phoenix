#include <phoenix/dev/DevelopmentDeviceLoop.h>

#include <phoenix/logging/Log.h>
#include <phoenix/protocol/legacy/LegacyFrame.h>
#include <phoenix/runtime/LegacyDeviceController.h>
#include <phoenix/runtime/LegacyDeviceSession.h>
#include <phoenix/xplane/IXPlaneBridge.h>

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <utility>

namespace phoenix::dev
{
    namespace
    {
        class DevelopmentXPlaneBridge final
            : public xplane::IXPlaneBridge
        {
        public:
            xplane::DataRefLookupResult findDataRef(
                std::string_view name) override
            {
                std::ostringstream message;

                message
                    << "    X-Plane emulator accepted dataref: "
                    << name
                    << '\n';

                logging::info(message.str());

                return {
                    true,
                    xplane::DataRefTypeInt |
                    xplane::DataRefTypeFloat |
                    xplane::DataRefTypeIntArray |
                    xplane::DataRefTypeFloatArray |
                    xplane::DataRefTypeData
                };
            }

            xplane::CommandLookupResult findCommand(
                std::string_view name) override
            {
                std::ostringstream message;

                message
                    << "    X-Plane emulator accepted command: "
                    << name
                    << '\n';

                logging::info(message.str());

                return { true };
            }

            void writeDataRef(
                const xplane::DataRefWrite& write) override
            {
                std::ostringstream message;

                message
                    << "    Device wrote dataref "
                    << write.handle
                    << " ("
                    << write.name
                    << ") = "
                    << write.value;

                if (write.element)
                {
                    message
                        << " element "
                        << *write.element;
                }

                message << '\n';

                logging::info(message.str());
            }

            void touchDataRef(
                std::string_view name,
                int handle) override
            {
                std::ostringstream message;

                message
                    << "    Device requested dataref refresh "
                    << handle
                    << " ("
                    << name
                    << ")\n";

                logging::info(message.str());
            }

            void commandBegin(
                std::string_view name,
                int handle) override
            {
                logCommandAction("begin", name, handle);
            }

            void commandEnd(
                std::string_view name,
                int handle) override
            {
                logCommandAction("end", name, handle);
            }

            void commandTrigger(
                std::string_view name,
                int handle,
                int triggerCount) override
            {
                std::ostringstream message;

                message
                    << "    Device triggered command "
                    << handle
                    << " ("
                    << name
                    << ") "
                    << triggerCount
                    << " time(s)\n";

                logging::info(message.str());
            }

            void debugMessage(
                std::string_view messageText) override
            {
                std::ostringstream message;

                message
                    << "    Device debug: "
                    << messageText
                    << '\n';

                logging::info(message.str());
            }

            void speak(
                std::string_view messageText) override
            {
                std::ostringstream message;

                message
                    << "    Device speak request: "
                    << messageText
                    << '\n';

                logging::info(message.str());
            }

            void resetRequested() override
            {
                logging::info(
                    "    Device requested reset.\n");
            }

        private:
            void logCommandAction(
                std::string_view action,
                std::string_view name,
                int handle)
            {
                std::ostringstream message;

                message
                    << "    Device command "
                    << action
                    << ' '
                    << handle
                    << " ("
                    << name
                    << ")\n";

                logging::info(message.str());
            }
        };

        class DevelopmentLegacyDeviceObserver final
            : public runtime::ILegacyDeviceObserver
        {
        public:
            void microcontrollerRequestedDataRef(
                std::string_view name) override
            {
                std::ostringstream message;

                message
                    << "    Microcontroller requested dataref: "
                    << name
                    << '\n';

                logging::info(message.str());
            }

            void microcontrollerRequestedCommand(
                std::string_view name) override
            {
                std::ostringstream message;

                message
                    << "    Microcontroller requested command: "
                    << name
                    << '\n';

                logging::info(message.str());
            }

            void microcontrollerRequestedUpdates(
                const protocol::legacy::UpdatesRequest& request) override
            {
                std::ostringstream message;

                message
                    << "    Microcontroller requested updates for handle "
                    << request.handle
                    << " every "
                    << request.rate
                    << " ms, precision "
                    << request.precision;

                if (request.requestedType)
                {
                    message
                        << ", type "
                        << *request.requestedType;
                }

                if (request.element)
                {
                    message
                        << ", element "
                        << *request.element;
                }

                message << '\n';

                logging::info(message.str());
            }

            void microcontrollerRequestedScaling(
                const protocol::legacy::ScalingRequest& request) override
            {
                std::ostringstream message;

                message
                    << "    Microcontroller requested scaling for handle "
                    << request.handle
                    << ": "
                    << request.fromLow
                    << "-"
                    << request.fromHigh
                    << " -> "
                    << request.toLow
                    << "-"
                    << request.toHigh
                    << '\n';

                logging::info(message.str());
            }
        };
    }

    struct DevelopmentDeviceManager::DeviceContext
    {
        DeviceContext(
            std::string portNameValue,
            std::string deviceNameValue,
            std::string deviceVersionValue,
            std::unique_ptr<transport::IByteTransport> transportValue,
            logging::ISerialTraceSink& serialTrace)
            : portName(std::move(portNameValue))
            , deviceName(std::move(deviceNameValue))
            , deviceVersion(std::move(deviceVersionValue))
            , transport(std::move(transportValue))
            , session(
                std::make_unique<runtime::LegacyDeviceSession>(
                    *transport,
                    runtime::LegacyDeviceSessionOptions{
                        .readBufferSize = 256,
                        .maximumReadPasses = 8,
                        .serialTrace = &serialTrace,
                        .tracePortName = portName
                    }))
            , controller(
                std::make_unique<runtime::LegacyDeviceController>(
                    *session,
                    xplane,
                    observer))
        {
        }

        ~DeviceContext()
        {
            if (transport && transport->isOpen())
            {
                transport->close();
            }
        }

        std::string portName;
        std::string deviceName;
        std::string deviceVersion;
        std::unique_ptr<transport::IByteTransport> transport;
        DevelopmentXPlaneBridge xplane;
        DevelopmentLegacyDeviceObserver observer;
        std::unique_ptr<runtime::LegacyDeviceSession> session;
        std::unique_ptr<runtime::LegacyDeviceController> controller;
        bool emulatedDataRefState = true;
        std::chrono::steady_clock::time_point nextToggleTime =
            std::chrono::steady_clock::now();
    };

    namespace
    {
        void logTickActivity(
            const DevelopmentDeviceManager::DeviceContext& device,
            const runtime::LegacyDeviceControllerTickResult& tick)
        {
            if (tick.session.bytesRead == 0 &&
                tick.session.bytesWritten == 0 &&
                tick.messagesProcessed == 0 &&
                tick.responseBytesWritten == 0)
            {
                return;
            }

            std::ostringstream message;

            message
                << "    "
                << device.portName
                << " tick: read "
                << tick.session.bytesRead
                << " byte(s), processed "
                << tick.messagesProcessed
                << " message(s), wrote "
                << tick.session.bytesWritten +
                    tick.responseBytesWritten
                << " byte(s).\n";

            logging::info(message.str());
        }

        void sendEmulatedDataRefUpdates(
            DevelopmentDeviceManager::DeviceContext& device,
            std::chrono::steady_clock::time_point now)
        {
            if (device.controller->updateSubscriptions().empty() ||
                now < device.nextToggleTime)
            {
                return;
            }

            for (const auto& subscription :
                device.controller->updateSubscriptions())
            {
                std::ostringstream payload;

                payload
                    << ','
                    << subscription.handle
                    << ','
                    << (device.emulatedDataRefState ? 1 : 0);

                if (subscription.element)
                {
                    payload
                        << ','
                        << *subscription.element;

                    device.session->queueFrame(
                        protocol::legacy::dataRefUpdateIntArrayCommand,
                        payload.str());
                }
                else
                {
                    device.session->queueFrame(
                        protocol::legacy::dataRefUpdateIntCommand,
                        payload.str());
                }

                std::ostringstream message;

                message
                    << "    "
                    << device.portName
                    << " X-Plane emulator sent dataref handle "
                    << subscription.handle
                    << " = "
                    << (device.emulatedDataRefState ? 1 : 0);

                if (subscription.element)
                {
                    message
                        << " element "
                        << *subscription.element;
                }

                message << '\n';

                logging::info(message.str());
            }

            const std::size_t bytesWritten =
                device.session->flushPendingOutput();

            if (bytesWritten > 0)
            {
                std::ostringstream message;

                message
                    << "    "
                    << device.portName
                    << " tick: emulator wrote "
                    << bytesWritten
                    << " data update byte(s).\n";

                logging::info(message.str());
            }

            device.emulatedDataRefState =
                !device.emulatedDataRefState;
            device.nextToggleTime =
                now + std::chrono::seconds{ 2 };
        }
    }

    DevelopmentDeviceManager::DevelopmentDeviceManager(
        logging::ISerialTraceSink& serialTrace)
        : serialTrace_(serialTrace)
    {
    }

    DevelopmentDeviceManager::~DevelopmentDeviceManager() = default;

    void DevelopmentDeviceManager::addDevice(
        std::string portName,
        std::string deviceName,
        std::string deviceVersion,
        std::unique_ptr<transport::IByteTransport> transport)
    {
        auto device = std::make_unique<DeviceContext>(
            std::move(portName),
            std::move(deviceName),
            std::move(deviceVersion),
            std::move(transport),
            serialTrace_);

        std::ostringstream message;

        message
            << "  Queued "
            << device->portName
            << " for the development runtime loop.\n";

        logging::info(message.str());

        device->controller->requestRegistrations();
        device->nextToggleTime =
            std::chrono::steady_clock::now();

        devices_.push_back(std::move(device));
    }

    std::size_t DevelopmentDeviceManager::deviceCount() const
    {
        return devices_.size();
    }

    void DevelopmentDeviceManager::runFor(
        std::chrono::steady_clock::duration duration)
    {
        if (devices_.empty())
        {
            return;
        }

        {
            std::ostringstream message;

            message
                << "\nRunning development runtime loop for "
                << devices_.size()
                << " device(s)...\n";

            logging::info(message.str());
        }

        const auto deadline =
            std::chrono::steady_clock::now() +
            duration;

        while (std::chrono::steady_clock::now() < deadline)
        {
            const auto now =
                std::chrono::steady_clock::now();

            for (auto& device : devices_)
            {
                const auto tick =
                    device->controller->tick();

                logTickActivity(*device, tick);
                sendEmulatedDataRefUpdates(*device, now);
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds{ 10 });
        }

        logging::info(
            "  Development runtime loop complete.\n");

        for (const auto& device : devices_)
        {
            std::ostringstream summary;

            summary
                << "  "
                << device->portName
                << " registered datarefs: "
                << device->controller->dataRefs().size()
                << '\n'
                << "  "
                << device->portName
                << " registered commands: "
                << device->controller->commands().size()
                << '\n'
                << "  "
                << device->portName
                << " update subscriptions: "
                << device->controller->updateSubscriptions().size()
                << '\n';

            logging::info(summary.str());
        }
    }
}

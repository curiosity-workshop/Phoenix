#include <phoenix/discovery/LegacyXplproProbe.h>
#include <phoenix/logging/Log.h>
#include <phoenix/runtime/LegacyDeviceController.h>
#include <phoenix/runtime/LegacyDeviceSession.h>
#include <phoenix/serial/SerialDeviceClassifier.h>
#include <phoenix/serial/SerialDeviceKind.h>
#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>
#include <phoenix/xplane/IXPlaneBridge.h>

#include <chrono>
#include <sstream>
#include <string_view>
#include <thread>

namespace
{
    std::string_view deviceKindName(
        phoenix::serial::SerialDeviceKind kind)
    {
        using phoenix::serial::SerialDeviceKind;

        switch (kind)
        {
        case SerialDeviceKind::ArduinoCompatible:
            return "Arduino-compatible";

        case SerialDeviceKind::UsbSerial:
            return "USB serial";

        case SerialDeviceKind::Bluetooth:
            return "Bluetooth";

        case SerialDeviceKind::BuiltInSerial:
            return "Built-in serial";

        case SerialDeviceKind::Unknown:
        default:
            return "Unknown";
        }
    }

    void printPortInformation(
        const phoenix::serial::SerialPortInfo& port,
        phoenix::serial::SerialDeviceKind kind)
    {
        std::ostringstream message;

        message
            << port.portName << '\n'
            << "  Classification: "
            << deviceKindName(kind) << '\n'
            << "  Name:           "
            << port.displayName << '\n'
            << "  Manufacturer:   "
            << port.manufacturer << '\n'
            << "  Hardware ID:    "
            << port.hardwareId << "\n\n";

        phoenix::logging::info(message.str());
    }

    class DevelopmentXPlaneBridge final
        : public phoenix::xplane::IXPlaneBridge
    {
    public:
        phoenix::xplane::DataRefLookupResult findDataRef(
            std::string_view name) override
        {
            std::ostringstream message;

            message
                << "    X-Plane emulator accepted dataref: "
                << name
                << '\n';

            phoenix::logging::info(message.str());

            return {
                true,
                phoenix::xplane::DataRefTypeInt |
                phoenix::xplane::DataRefTypeFloat |
                phoenix::xplane::DataRefTypeIntArray |
                phoenix::xplane::DataRefTypeFloatArray |
                phoenix::xplane::DataRefTypeData
            };
        }

        phoenix::xplane::CommandLookupResult findCommand(
            std::string_view name) override
        {
            std::ostringstream message;

            message
                << "    X-Plane emulator accepted command: "
                << name
                << '\n';

            phoenix::logging::info(message.str());

            return { true };
        }

        void writeDataRef(
            const phoenix::xplane::DataRefWrite& write) override
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

            phoenix::logging::info(message.str());
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

            phoenix::logging::info(message.str());
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

            phoenix::logging::info(message.str());
        }

        void debugMessage(
            std::string_view messageText) override
        {
            std::ostringstream message;

            message
                << "    Device debug: "
                << messageText
                << '\n';

            phoenix::logging::info(message.str());
        }

        void speak(
            std::string_view messageText) override
        {
            std::ostringstream message;

            message
                << "    Device speak request: "
                << messageText
                << '\n';

            phoenix::logging::info(message.str());
        }

        void resetRequested() override
        {
            phoenix::logging::info(
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

            phoenix::logging::info(message.str());
        }
    };

    class DevelopmentLegacyDeviceObserver final
        : public phoenix::runtime::ILegacyDeviceObserver
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

            phoenix::logging::info(message.str());
        }

        void microcontrollerRequestedCommand(
            std::string_view name) override
        {
            std::ostringstream message;

            message
                << "    Microcontroller requested command: "
                << name
                << '\n';

            phoenix::logging::info(message.str());
        }

        void microcontrollerRequestedUpdates(
            const phoenix::protocol::legacy::UpdatesRequest& request) override
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

            phoenix::logging::info(message.str());
        }

        void microcontrollerRequestedScaling(
            const phoenix::protocol::legacy::ScalingRequest& request) override
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

            phoenix::logging::info(message.str());
        }
    };

    void runDevelopmentDeviceLoop(
        phoenix::transport::IByteTransport& transport)
    {
        phoenix::logging::info(
            "  Requesting dataref and command registrations...\n");

        DevelopmentXPlaneBridge xplane;
        DevelopmentLegacyDeviceObserver observer;

        phoenix::runtime::LegacyDeviceSession session{
            transport,
            phoenix::runtime::LegacyDeviceSessionOptions{
                .readBufferSize = 256,
                .maximumReadPasses = 8
            }
        };

        phoenix::runtime::LegacyDeviceController controller{
            session,
            xplane,
            observer
        };

        controller.requestRegistrations();

        const auto deadline =
            std::chrono::steady_clock::now() +
            std::chrono::seconds{ 8 };

        auto nextToggleTime =
            std::chrono::steady_clock::now();

        bool emulatedDataRefState = true;

        while (std::chrono::steady_clock::now() < deadline)
        {
            const auto tick = controller.tick();
            const auto now =
                std::chrono::steady_clock::now();

            if (tick.session.bytesRead > 0 ||
                tick.session.bytesWritten > 0 ||
                tick.messagesProcessed > 0 ||
                tick.responseBytesWritten > 0)
            {
                std::ostringstream message;

                message
                    << "    Tick: read "
                    << tick.session.bytesRead
                    << " byte(s), processed "
                    << tick.messagesProcessed
                    << " message(s), wrote "
                    << tick.session.bytesWritten +
                        tick.responseBytesWritten
                    << " byte(s).\n";

                phoenix::logging::info(message.str());
            }

            if (!controller.updateSubscriptions().empty() &&
                now >= nextToggleTime)
            {
                for (const auto& subscription :
                    controller.updateSubscriptions())
                {
                    std::ostringstream payload;

                    payload
                        << ','
                        << subscription.handle
                        << ','
                        << (emulatedDataRefState ? 1 : 0);

                    if (subscription.element)
                    {
                        payload
                            << ','
                            << *subscription.element;

                        session.queueFrame(
                            phoenix::protocol::legacy::dataRefUpdateIntArrayCommand,
                            payload.str());
                    }
                    else
                    {
                        session.queueFrame(
                            phoenix::protocol::legacy::dataRefUpdateIntCommand,
                            payload.str());
                    }

                    std::ostringstream message;

                    message
                        << "    X-Plane emulator sent dataref handle "
                        << subscription.handle
                        << " = "
                        << (emulatedDataRefState ? 1 : 0);

                    if (subscription.element)
                    {
                        message
                            << " element "
                            << *subscription.element;
                    }

                    message << '\n';

                    phoenix::logging::info(message.str());
                }

                const std::size_t bytesWritten =
                    session.flushPendingOutput();

                if (bytesWritten > 0)
                {
                    std::ostringstream message;

                    message
                        << "    Tick: emulator wrote "
                        << bytesWritten
                        << " data update byte(s).\n";

                    phoenix::logging::info(message.str());
                }

                emulatedDataRefState = !emulatedDataRefState;
                nextToggleTime =
                    now + std::chrono::seconds{ 2 };
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds{ 10 });
        }

        std::ostringstream summary;

        summary
            << "  Registration loop complete.\n"
            << "  Registered datarefs: "
            << controller.dataRefs().size()
            << '\n'
            << "  Registered commands: "
            << controller.commands().size()
            << '\n'
            << "  Update subscriptions: "
            << controller.updateSubscriptions().size()
            << '\n';

        phoenix::logging::info(summary.str());
    }
}

int main()
{
    phoenix::logging::info(
        "Phoenix starting\n"
        "Enumerating serial ports...\n\n");

    phoenix::serial::WindowsSerialEnumerator enumerator;

    const auto ports = enumerator.enumerate();

    {
        std::ostringstream message;

        message
        << "Serial ports found: "
        << ports.size()
        << "\n\n";

        phoenix::logging::info(message.str());
    }

    if (ports.empty())
    {
        phoenix::logging::info(
            "No serial ports were detected.\n"
            "Phoenix stopped");

        return 0;
    }

    for (const auto& port : ports)
    {
        const auto kind =
            phoenix::serial::SerialDeviceClassifier::classify(port);

        printPortInformation(port, kind);
    }

    phoenix::logging::info(
        "Beginning XPLPro device discovery...\n\n");

    phoenix::discovery::LegacyXplproProbe probe;

    std::size_t devicesFound = 0;

    for (const auto& port : ports)
    {
        const auto kind =
            phoenix::serial::SerialDeviceClassifier::classify(port);

        if (kind == phoenix::serial::SerialDeviceKind::Bluetooth)
        {
            std::ostringstream message;

            message
                << "Skipping "
                << port.portName
                << " because it is classified as Bluetooth.\n\n";

            phoenix::logging::info(message.str());

            continue;
        }

        {
            std::ostringstream message;

            message
            << "Probing "
            << port.portName
            << "...\n"
            << "  Classification: "
            << deviceKindName(kind)
            << '\n';

            phoenix::logging::info(message.str());
        }

        phoenix::serial::WindowsSerialTransport transport(
            port.portName,
            115200);

        if (!transport.open())
        {
            std::ostringstream message;

            message
                << "  Unable to open "
                << port.portName
                << ".\n\n";

            phoenix::logging::warning(message.str());

            continue;
        }

        phoenix::logging::info(
            "  Port opened successfully.\n"
            "  Sending XPLPro identity requests...\n");

        const auto device = probe.probe(transport);

        if (device)
        {
            ++devicesFound;

            std::ostringstream message;

            message
                << "  XPLPro device discovered.\n"
                << "  Device name:    "
                << device->name << '\n'
                << "  Device version: "
                << device->version << '\n';

            phoenix::logging::info(message.str());

            runDevelopmentDeviceLoop(transport);
        }
        else
        {
            phoenix::logging::info(
                "  No valid XPLPro response received.\n");
        }

        transport.close();

        phoenix::logging::info("  Port closed.\n\n");
    }

    {
        std::ostringstream message;

        message
        << "Discovery complete.\n"
        << "XPLPro devices found: "
        << devicesFound
        << '\n'
        << "Phoenix stopped\n";

        phoenix::logging::info(message.str());
    }

    return 0;
}

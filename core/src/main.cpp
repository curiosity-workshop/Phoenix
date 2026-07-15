#include <phoenix/discovery/LegacyXplproProbe.h>
#include <phoenix/dev/DevelopmentDeviceLoop.h>
#include <phoenix/logging/Log.h>
#include <phoenix/logging/SerialTraceLogger.h>
#include <phoenix/serial/SerialDeviceClassifier.h>
#include <phoenix/serial/SerialDeviceKind.h>
#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string_view>

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

    std::filesystem::path sourceRoot()
    {
#ifdef PHOENIX_SOURCE_DIR
        return PHOENIX_SOURCE_DIR;
#else
        return std::filesystem::current_path();
#endif
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

    phoenix::discovery::LegacyXplproProbe probe{
        std::chrono::milliseconds{ 50 },
        std::chrono::seconds{ 2 }
    };
    const auto serialTracePath =
        sourceRoot() / "PhoenixSerial.log";

    phoenix::logging::SerialTraceLogger serialTrace{
        serialTracePath
    };

    if (serialTrace.isOpen())
    {
        std::ostringstream message;

        message
            << "Serial trace logging enabled: "
            << serialTracePath.string()
            << "\n\n";

        phoenix::logging::info(
            message.str());
    }
    else
    {
        std::ostringstream message;

        message
            << "Unable to open "
            << serialTracePath.string()
            << " for serial tracing.\n\n";

        phoenix::logging::warning(
            message.str());
    }

    std::size_t devicesFound = 0;
    phoenix::dev::DevelopmentDeviceManager deviceManager{
        serialTrace,
        sourceRoot() / "profiles"
    };

    for (const auto& port : ports)
    {
        const auto kind =
            phoenix::serial::SerialDeviceClassifier::classify(port);

        if (kind == phoenix::serial::SerialDeviceKind::Bluetooth ||
            kind == phoenix::serial::SerialDeviceKind::BuiltInSerial)
        {
            std::ostringstream message;

            message
                << "Skipping "
                << port.portName
                << " because it is classified as "
                << deviceKindName(kind)
                << ".\n\n";

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

        auto transport =
            std::make_unique<phoenix::serial::WindowsSerialTransport>(
                port.portName,
                115200);

        if (!transport->open())
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

        const auto device =
            probe.probe(
                *transport,
                phoenix::discovery::LegacyXplproProbeTrace{
                    .serialTrace = &serialTrace,
                    .portName = port.portName
                });

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

            deviceManager.addDevice(
                port.portName,
                device->name,
                device->version,
                std::move(transport));
        }
        else
        {
            phoenix::logging::info(
                "  No valid XPLPro response received.\n");

            transport->close();

            phoenix::logging::info("  Port closed.\n\n");
        }
    }

    deviceManager.runFor(std::chrono::seconds{ 8 });

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

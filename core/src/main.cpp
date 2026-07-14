#include <phoenix/discovery/LegacyXplproProbe.h>
#include <phoenix/serial/SerialDeviceClassifier.h>
#include <phoenix/serial/SerialDeviceKind.h>
#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>

#include <iostream>
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
        std::cout
            << port.portName << '\n'
            << "  Classification: "
            << deviceKindName(kind) << '\n'
            << "  Name:           "
            << port.displayName << '\n'
            << "  Manufacturer:   "
            << port.manufacturer << '\n'
            << "  Hardware ID:    "
            << port.hardwareId << "\n\n";
    }
}

int main()
{
    std::cout
        << "Phoenix starting\n"
        << "Enumerating serial ports...\n\n";

    phoenix::serial::WindowsSerialEnumerator enumerator;

    const auto ports = enumerator.enumerate();

    std::cout
        << "Serial ports found: "
        << ports.size()
        << "\n\n";

    if (ports.empty())
    {
        std::cout
            << "No serial ports were detected.\n"
            << "Phoenix stopped\n";

        return 0;
    }

    for (const auto& port : ports)
    {
        const auto kind =
            phoenix::serial::SerialDeviceClassifier::classify(port);

        printPortInformation(port, kind);
    }

    std::cout
        << "Beginning XPLPro device discovery...\n\n";

    phoenix::discovery::LegacyXplproProbe probe;

    std::size_t devicesFound = 0;

    for (const auto& port : ports)
    {
        const auto kind =
            phoenix::serial::SerialDeviceClassifier::classify(port);

        if (kind == phoenix::serial::SerialDeviceKind::Bluetooth)
        {
            std::cout
                << "Skipping "
                << port.portName
                << " because it is classified as Bluetooth.\n\n";

            continue;
        }

        std::cout
            << "Probing "
            << port.portName
            << "...\n"
            << "  Classification: "
            << deviceKindName(kind)
            << '\n';

        phoenix::serial::WindowsSerialTransport transport(
            port.portName,
            115200);

        if (!transport.open())
        {
            std::cout
                << "  Unable to open "
                << port.portName
                << ".\n\n";

            continue;
        }

        std::cout
            << "  Port opened successfully.\n"
            << "  Sending XPLPro identity requests...\n";

        const auto device = probe.probe(transport);

        if (device)
        {
            ++devicesFound;

            std::cout
                << "  XPLPro device discovered.\n"
                << "  Device name:    "
                << device->name << '\n'
                << "  Device version: "
                << device->version << '\n';
        }
        else
        {
            std::cout
                << "  No valid XPLPro response received.\n";
        }

        transport.close();

        std::cout
            << "  Port closed.\n\n";
    }

    std::cout
        << "Discovery complete.\n"
        << "XPLPro devices found: "
        << devicesFound
        << '\n'
        << "Phoenix stopped\n";

    return 0;
}
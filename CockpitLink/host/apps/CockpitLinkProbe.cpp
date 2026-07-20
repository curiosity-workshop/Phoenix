#include <cockpitlink/serial/SerialDeviceKind.h>
#include <cockpitlink/serial/WindowsSerialEnumerator.h>

#include <iostream>
#include <string_view>

namespace
{
    std::string_view kindName(
        cockpitlink::serial::SerialDeviceKind kind)
    {
        using cockpitlink::serial::SerialDeviceKind;

        switch (kind)
        {
        case SerialDeviceKind::ArduinoCompatible:
            return "arduino-compatible";
        case SerialDeviceKind::UsbSerial:
            return "usb-serial";
        case SerialDeviceKind::Bluetooth:
            return "bluetooth";
        case SerialDeviceKind::BuiltInSerial:
            return "built-in-serial";
        case SerialDeviceKind::Unknown:
        default:
            return "unknown";
        }
    }
}

int main()
{
    cockpitlink::serial::WindowsSerialEnumerator enumerator;

    const auto ports =
        enumerator.enumerate();

    std::cout
        << "CockpitLink probe\n"
        << "Ports detected: "
        << ports.size()
        << "\n";

    for (const auto& port : ports)
    {
        std::cout
            << port.portName
            << "  "
            << kindName(port.kind)
            << "  "
            << port.displayName
            << "\n";
    }

    return 0;
}

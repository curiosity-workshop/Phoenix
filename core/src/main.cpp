#include <phoenix/serial/WindowsSerialEnumerator.h>

#include <iostream>

int main()
{
    std::cout << "Phoenix starting\n\n";

    phoenix::serial::WindowsSerialEnumerator enumerator;

    const auto ports = enumerator.enumerate();

    std::cout << "Serial ports found: "
        << ports.size()
        << "\n\n";

    for (const auto& port : ports)
    {
        std::cout
            << port.portName << '\n'
            << "  Name:         " << port.displayName << '\n'
            << "  Manufacturer: " << port.manufacturer << '\n'
            << "  Hardware ID:  " << port.hardwareId << "\n\n";
    }

    std::cout << "Phoenix stopped\n";
    return 0;
}
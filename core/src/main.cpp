#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>

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

    phoenix::serial::WindowsSerialTransport transport(
        "COM5",
        115200);

    if (!transport.open())
    {
        std::cerr << "Unable to open COM5\n";
        return 1;
    }

    std::cout << "Opened COM5 successfully\n";

    transport.close();

    std::cout << "Closed COM5 successfully\n";
   

    std::cout << "Phoenix stopped\n";
    return 0;
}
#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>
#include <phoenix/discovery/LegacyXplproProbe.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <span>
#include <thread>

int main()
{
    phoenix::serial::WindowsSerialEnumerator enumerator;

    const auto ports = enumerator.enumerate();

    for (const auto& port : ports)
    {
        phoenix::serial::WindowsSerialTransport transport(port.portName, 115200);

        if (!transport.open())
        {
            continue;
        }

        phoenix::discovery::LegacyXplproProbe probe;

        const auto device = probe.probe(transport);

        if (device)
        {
            std::cout
                << "XPLPro device found\n"
                << "  Name:    " << device->name << '\n'
                << "  Version: " << device->version << '\n';
        }
        else
        {
            std::cout << "No XPLPro response received\n";
        }
    }
}


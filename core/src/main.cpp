#include <phoenix/serial/WindowsSerialEnumerator.h>
#include <phoenix/serial/WindowsSerialTransport.h>

#include <array>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <span>
#include <thread>

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
        "COM4",
        115200);

    if (!transport.open())
    {
        std::cerr << "Unable to open COM4\n";
        return 1;
    }

    std::cout << "Opened COM4 successfully\n";

    constexpr std::byte packetHeader{ '[' };
    constexpr std::byte sendNameCommand{ 'N' };
    constexpr std::byte packetTrailer{ ']' };

    const std::array<std::byte, 3> request
    {
       packetHeader,
       sendNameCommand,
       packetTrailer
    };
    
    std::cout << "Waiting 3 seconds for board reset\n";

    // Opening the serial port may reset Arduino-class boards.
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    
    std::cout << "Sending ";
    for (auto b : request)
    {
        std::cout << static_cast<char>(std::to_integer<unsigned char>(b));
    }


    std::cout << " to arduino\n";

    const std::size_t bytesWritten = transport.write(request);

    std::cout << "Sent " << bytesWritten << " bytes\n";

  

    std::array<std::byte, 256> buffer{};

    const auto deadline =
        std::chrono::steady_clock::now() +
        std::chrono::seconds(3);

    while (std::chrono::steady_clock::now() < deadline)
    {
        const std::size_t bytesRead = transport.read(buffer);

        if (bytesRead > 0)
        {
            std::cout << "RX: ";

            for (std::size_t i = 0; i < bytesRead; ++i)
            {
                const unsigned char value =
                    std::to_integer<unsigned char>(buffer[i]);

                if (value >= 32 && value <= 126)
                {
                    std::cout << static_cast<char>(value);
                }
                else
                {
                    std::cout << "[0x"
                        << std::hex
                        << static_cast<int>(value)
                        << std::dec
                        << "]";
                }
            }

            std::cout << '\n';
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(10));
    }

    transport.close();

    std::cout << "Closed COM4 successfully\n";
   

    std::cout << "Phoenix stopped\n";
    return 0;
}
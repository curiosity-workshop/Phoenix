#include <phoenix/serial/WindowsSerialTransportFactory.h>

#include <phoenix/serial/WindowsSerialTransport.h>

#include <string>

namespace phoenix::serial
{
    std::unique_ptr<transport::IByteTransport>
        WindowsSerialTransportFactory::create(
            std::string_view portName,
            std::uint32_t baudRate) const
    {
        return std::make_unique<WindowsSerialTransport>(
            std::string{ portName },
            baudRate);
    }
}

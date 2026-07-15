#pragma once

#include <phoenix/serial/ISerialTransportFactory.h>

namespace phoenix::serial
{
    class WindowsSerialTransportFactory final
        : public ISerialTransportFactory
    {
    public:
        std::unique_ptr<transport::IByteTransport> create(
            std::string_view portName,
            std::uint32_t baudRate) const override;
    };
}

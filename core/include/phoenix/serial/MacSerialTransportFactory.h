#pragma once

#include <phoenix/serial/ISerialTransportFactory.h>
#include <phoenix/serial/MacSerialTransport.h>

namespace phoenix::serial
{
    class MacSerialTransportFactory final
        : public ISerialTransportFactory
    {
    public:
        std::unique_ptr<transport::IByteTransport> create(
            std::string_view portName,
            std::uint32_t baudRate) const override;

        std::unique_ptr<transport::IByteTransport> create(
            std::string_view portName,
            std::uint32_t baudRate,
            MacSerialControlMode controlMode) const;
    };
}

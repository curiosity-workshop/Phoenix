#pragma once

#include <phoenix/serial/ISerialTransportFactory.h>
#include <phoenix/serial/LinuxSerialTransport.h>

namespace phoenix::serial
{
    class LinuxSerialTransportFactory final
        : public ISerialTransportFactory
    {
    public:
        std::unique_ptr<transport::IByteTransport> create(
            std::string_view portName,
            std::uint32_t baudRate) const override;

        std::unique_ptr<transport::IByteTransport> create(
            std::string_view portName,
            std::uint32_t baudRate,
            LinuxSerialControlMode controlMode) const;
    };
}

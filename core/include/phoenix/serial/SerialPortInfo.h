#pragma once

#include <string>
#include <phoenix/serial/SerialDeviceKind.h>

namespace phoenix::serial
{
    struct SerialPortInfo
    {
        std::string portName;
        std::string displayName;
        std::string manufacturer;
        std::string hardwareId;

        SerialDeviceKind kind = SerialDeviceKind::Unknown;
    };
}
#pragma once

#include <string>

namespace phoenix::serial
{
    struct SerialPortInfo
    {
        std::string portName;      // COM3
        std::string displayName;   // Arduino Mega 2560 (COM3)
        std::string hardwareId;    // USB\VID_2341&PID_0042...
        std::string manufacturer;  // Arduino LLC
    };
}
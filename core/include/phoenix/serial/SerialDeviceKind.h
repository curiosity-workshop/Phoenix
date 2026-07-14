#pragma once

namespace phoenix::serial
{
    enum class SerialDeviceKind
    {
        Unknown,
        ArduinoCompatible,
        UsbSerial,
        Bluetooth,
        BuiltInSerial
    };
}
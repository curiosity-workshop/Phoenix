#pragma once

#include <phoenix/serial/SerialDeviceKind.h>
#include <phoenix/serial/SerialPortInfo.h>

namespace phoenix::serial
{
    class SerialDeviceClassifier
    {
    public:
        static SerialDeviceKind classify(
            const SerialPortInfo& port);
    };
}
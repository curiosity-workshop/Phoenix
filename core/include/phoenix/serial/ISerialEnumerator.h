#pragma once

#include <vector>

#include <phoenix/serial/SerialPortInfo.h>

namespace phoenix::serial
{
    class ISerialEnumerator
    {
    public:
        virtual ~ISerialEnumerator() = default;

        virtual std::vector<SerialPortInfo> enumerate() const = 0;
    };
}
#pragma once

#include <phoenix/serial/ISerialEnumerator.h>

namespace phoenix::serial
{
    class WindowsSerialEnumerator final : public ISerialEnumerator
    {
    public:
        std::vector<SerialPortInfo> enumerate() const override;
    };
}
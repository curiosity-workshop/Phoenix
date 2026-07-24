#pragma once

#include <phoenix/serial/ISerialEnumerator.h>

namespace phoenix::serial
{
    class LinuxSerialEnumerator final : public ISerialEnumerator
    {
    public:
        std::vector<SerialPortInfo> enumerate() const override;
    };
}

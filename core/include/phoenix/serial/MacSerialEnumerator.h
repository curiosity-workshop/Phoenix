#pragma once

#include <phoenix/serial/ISerialEnumerator.h>

namespace phoenix::serial
{
    class MacSerialEnumerator final : public ISerialEnumerator
    {
    public:
        std::vector<SerialPortInfo> enumerate() const override;
    };
}

#include "PhoenixXPL.h"

namespace phoenix
{
    PhoenixXPL::PhoenixXPL(Stream& stream)
        : stream_(stream)
    {
    }

    void PhoenixXPL::begin(const char* deviceName)
    {
        deviceName_ = deviceName;
    }

    void PhoenixXPL::loop()
    {
    }
}

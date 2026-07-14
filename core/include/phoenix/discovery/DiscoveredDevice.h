#pragma once

#include <string>

namespace phoenix::discovery
{
    struct DiscoveredDevice
    {
        std::string name;
        std::string version;
    };
}
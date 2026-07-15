#pragma once

#include <Arduino.h>

namespace phoenix::platform
{
    inline unsigned long milliseconds()
    {
        return millis();
    }
}

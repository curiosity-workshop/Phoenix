#pragma once

#include <Arduino.h>

namespace phoenix
{
    class XPLLink
    {
    public:
        explicit XPLLink(Stream& stream);

        void begin(const char* deviceName);
        void loop();

    private:
        Stream& stream_;
        const char* deviceName_ = nullptr;
    };
}

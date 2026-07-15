#include "XPLLink.h"

namespace phoenix
{
    XPLLink::XPLLink(Stream& stream)
        : stream_(stream)
    {
    }

    void XPLLink::begin(const char* deviceName)
    {
        deviceName_ = deviceName;
    }

    void XPLLink::loop()
    {
    }
}

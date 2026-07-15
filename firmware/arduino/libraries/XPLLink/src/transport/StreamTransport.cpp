#include "StreamTransport.h"

namespace phoenix::transport
{
    StreamTransport::StreamTransport(Stream& stream)
        : stream_(stream)
    {
    }

    int StreamTransport::available()
    {
        return stream_.available();
    }

    int StreamTransport::read()
    {
        return stream_.read();
    }

    size_t StreamTransport::write(const uint8_t* data, size_t size)
    {
        return stream_.write(data, size);
    }
}

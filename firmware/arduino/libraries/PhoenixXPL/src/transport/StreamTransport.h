#pragma once

#include "PhoenixTransport.h"

namespace phoenix::transport
{
    class StreamTransport final : public PhoenixTransport
    {
    public:
        explicit StreamTransport(Stream& stream);

        int available() override;
        int read() override;
        size_t write(const uint8_t* data, size_t size) override;

    private:
        Stream& stream_;
    };
}

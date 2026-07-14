#pragma once

#include <phoenix/protocol/legacy/LegacyFrame.h>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace phoenix::protocol::legacy
{
    class LegacyFrameParser
    {
    public:
        std::vector<LegacyFrame> push(
            std::span<const std::byte> bytes);

        void reset();

        bool hasPartialFrame() const;

    private:
        std::string buffer_;
    };
}

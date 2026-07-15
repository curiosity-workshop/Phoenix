#pragma once

#include <phoenix/logging/SerialTraceLogger.h>
#include <phoenix/transport/IByteTransport.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace phoenix::dev
{
    class DevelopmentDeviceManager
    {
    public:
        struct DeviceContext;

        explicit DevelopmentDeviceManager(
            logging::ISerialTraceSink& serialTrace,
            std::filesystem::path profileDirectory);

        ~DevelopmentDeviceManager();

        DevelopmentDeviceManager(
            const DevelopmentDeviceManager&) = delete;

        DevelopmentDeviceManager& operator=(
            const DevelopmentDeviceManager&) = delete;

        void addDevice(
            std::string portName,
            std::string deviceName,
            std::string deviceVersion,
            std::unique_ptr<transport::IByteTransport> transport);

        std::size_t deviceCount() const;

        void runFor(
            std::chrono::steady_clock::duration duration);

    private:
        logging::ISerialTraceSink& serialTrace_;
        std::filesystem::path profileDirectory_;
        std::vector<std::unique_ptr<DeviceContext>> devices_;
    };
}

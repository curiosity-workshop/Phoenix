#pragma once

#include <phoenix/logging/SerialTraceLogger.h>
#include <phoenix/runtime/LegacyDeviceController.h>
#include <phoenix/runtime/LegacyUpdateScheduler.h>
#include <phoenix/transport/IByteTransport.h>
#include <phoenix/xplane/IXPlaneBridge.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace phoenix::runtime
{
    class IXPlaneBridgeFactory
    {
    public:
        virtual ~IXPlaneBridgeFactory() = default;

        virtual std::unique_ptr<xplane::IXPlaneBridge> createBridge(
            std::string_view portName,
            std::string_view deviceName,
            std::string_view deviceVersion) = 0;
    };

    class ILegacyDeviceObserverFactory
    {
    public:
        virtual ~ILegacyDeviceObserverFactory() = default;

        virtual std::unique_ptr<ILegacyDeviceObserver> createObserver(
            std::string_view portName,
            std::string_view deviceName,
            std::string_view deviceVersion) = 0;
    };

    struct DeviceRuntimeManagerOptions
    {
        std::filesystem::path profileDirectory;
        LegacyUpdateSchedulerOptions updateScheduler;
        std::size_t readBufferSize = 256;
        std::size_t maximumReadPasses = 8;
    };

    class DeviceRuntimeManager
    {
    public:
        struct DeviceContext;

        DeviceRuntimeManager(
            logging::ISerialTraceSink& serialTrace,
            IXPlaneBridgeFactory& bridgeFactory,
            DeviceRuntimeManagerOptions options,
            ILegacyDeviceObserverFactory* observerFactory = nullptr);

        ~DeviceRuntimeManager();

        DeviceRuntimeManager(
            const DeviceRuntimeManager&) = delete;

        DeviceRuntimeManager& operator=(
            const DeviceRuntimeManager&) = delete;

        void addDevice(
            std::string portName,
            std::string deviceName,
            std::string deviceVersion,
            std::unique_ptr<transport::IByteTransport> transport);

        void tick(
            std::chrono::steady_clock::time_point now);

        void runFor(
            std::chrono::steady_clock::duration duration);

        void saveProfiles() const;

        std::size_t disengageDevices();

        std::size_t deviceCount() const;
        std::size_t dataRefCount() const;
        std::size_t commandCount() const;
        std::size_t updateSubscriptionCount() const;

    private:
        logging::ISerialTraceSink& serialTrace_;
        IXPlaneBridgeFactory& bridgeFactory_;
        ILegacyDeviceObserverFactory* observerFactory_ = nullptr;
        DeviceRuntimeManagerOptions options_;
        std::vector<std::unique_ptr<DeviceContext>> devices_;
    };
}

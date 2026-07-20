#pragma once

#include <Arduino.h>

#ifndef COCKPITLINK_BAUDRATE
#define COCKPITLINK_BAUDRATE 115200
#endif

namespace cockpitlink
{
    class SwitchBinding
    {
    public:
        SwitchBinding(
            uint8_t pin,
            const char* behaviorId);

        uint8_t pin() const;
        const char* behaviorId() const;

    private:
        uint8_t pin_;
        const char* behaviorId_;
    };

    class OutputBinding
    {
    public:
        OutputBinding(
            uint8_t pin,
            const char* behaviorId);

        uint8_t pin() const;
        const char* behaviorId() const;

    private:
        uint8_t pin_;
        const char* behaviorId_;
    };

    class ButtonBinding
    {
    public:
        ButtonBinding(
            uint8_t pin,
            const char* behaviorId);

        uint8_t pin() const;
        const char* behaviorId() const;

    private:
        uint8_t pin_;
        const char* behaviorId_;
    };

    class SwitchBuilder
    {
    public:
        explicit SwitchBuilder(uint8_t pin);

        SwitchBinding controls(
            const char* behaviorId) const;

    private:
        uint8_t pin_;
    };

    class OutputBuilder
    {
    public:
        explicit OutputBuilder(uint8_t pin);

        OutputBinding follows(
            const char* behaviorId) const;

    private:
        uint8_t pin_;
    };

    class ButtonBuilder
    {
    public:
        explicit ButtonBuilder(uint8_t pin);

        ButtonBinding triggers(
            const char* behaviorId) const;

    private:
        uint8_t pin_;
    };

    class CockpitLinkDevice
    {
    public:
        void begin(
            const char* deviceName,
            const char* firmwareVersion);

        void loop();

        SwitchBuilder switchInput(
            uint8_t pin);

        OutputBuilder digitalOutput(
            uint8_t pin);

        ButtonBuilder button(
            uint8_t pin);

        const char* deviceName() const;
        const char* firmwareVersion() const;

    private:
        const char* deviceName_ = nullptr;
        const char* firmwareVersion_ = nullptr;
    };
}

extern cockpitlink::CockpitLinkDevice CockpitLink;

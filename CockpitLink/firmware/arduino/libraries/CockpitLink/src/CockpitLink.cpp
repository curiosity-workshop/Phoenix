#include "CockpitLink.h"

namespace cockpitlink
{
    SwitchBinding::SwitchBinding(
        uint8_t pin,
        const char* behaviorId)
        : pin_(pin),
        behaviorId_(behaviorId)
    {
        pinMode(pin_, INPUT_PULLUP);
    }

    uint8_t SwitchBinding::pin() const
    {
        return pin_;
    }

    const char* SwitchBinding::behaviorId() const
    {
        return behaviorId_;
    }

    OutputBinding::OutputBinding(
        uint8_t pin,
        const char* behaviorId)
        : pin_(pin),
        behaviorId_(behaviorId)
    {
        pinMode(pin_, OUTPUT);
    }

    uint8_t OutputBinding::pin() const
    {
        return pin_;
    }

    const char* OutputBinding::behaviorId() const
    {
        return behaviorId_;
    }

    ButtonBinding::ButtonBinding(
        uint8_t pin,
        const char* behaviorId)
        : pin_(pin),
        behaviorId_(behaviorId)
    {
        pinMode(pin_, INPUT_PULLUP);
    }

    uint8_t ButtonBinding::pin() const
    {
        return pin_;
    }

    const char* ButtonBinding::behaviorId() const
    {
        return behaviorId_;
    }

    SwitchBuilder::SwitchBuilder(uint8_t pin)
        : pin_(pin)
    {
    }

    SwitchBinding SwitchBuilder::controls(
        const char* behaviorId) const
    {
        return { pin_, behaviorId };
    }

    OutputBuilder::OutputBuilder(uint8_t pin)
        : pin_(pin)
    {
    }

    OutputBinding OutputBuilder::follows(
        const char* behaviorId) const
    {
        return { pin_, behaviorId };
    }

    ButtonBuilder::ButtonBuilder(uint8_t pin)
        : pin_(pin)
    {
    }

    ButtonBinding ButtonBuilder::triggers(
        const char* behaviorId) const
    {
        return { pin_, behaviorId };
    }

    void CockpitLinkDevice::begin(
        const char* deviceName,
        const char* firmwareVersion)
    {
        deviceName_ = deviceName;
        firmwareVersion_ = firmwareVersion;
        Serial.begin(COCKPITLINK_BAUDRATE);
    }

    void CockpitLinkDevice::loop()
    {
    }

    SwitchBuilder CockpitLinkDevice::switchInput(
        uint8_t pin)
    {
        return SwitchBuilder{ pin };
    }

    OutputBuilder CockpitLinkDevice::digitalOutput(
        uint8_t pin)
    {
        return OutputBuilder{ pin };
    }

    ButtonBuilder CockpitLinkDevice::button(
        uint8_t pin)
    {
        return ButtonBuilder{ pin };
    }

    const char* CockpitLinkDevice::deviceName() const
    {
        return deviceName_;
    }

    const char* CockpitLinkDevice::firmwareVersion() const
    {
        return firmwareVersion_;
    }
}

cockpitlink::CockpitLinkDevice CockpitLink;

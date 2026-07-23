#include "CockpitLink.h"

#include <string.h>

namespace cockpitlink
{
    namespace
    {
        void writeU16(
            uint8_t* output,
            uint16_t value)
        {
            output[0] =
                static_cast<uint8_t>((value >> 8) & 0xff);
            output[1] =
                static_cast<uint8_t>(value & 0xff);
        }

        void writeU32(
            uint8_t* output,
            uint32_t value)
        {
            output[0] =
                static_cast<uint8_t>((value >> 24) & 0xff);
            output[1] =
                static_cast<uint8_t>((value >> 16) & 0xff);
            output[2] =
                static_cast<uint8_t>((value >> 8) & 0xff);
            output[3] =
                static_cast<uint8_t>(value & 0xff);
        }

        void writeI32(
            uint8_t* output,
            int32_t value)
        {
            writeU32(
                output,
                static_cast<uint32_t>(value));
        }

        uint16_t readU16(
            uint8_t high,
            uint8_t low)
        {
            return static_cast<uint16_t>(
                (static_cast<uint16_t>(high) << 8) |
                static_cast<uint16_t>(low));
        }

        uint32_t readU32(
            const uint8_t* bytes)
        {
            uint32_t value = 0;

            for (uint8_t index = 0;
                index < COCKPITLINK_CHECKSUM_SIZE;
                ++index)
            {
                value =
                    (value << 8) |
                    static_cast<uint32_t>(bytes[index]);
            }

            return value;
        }
    }

    uint32_t cockpitLinkCrc32(
        const uint8_t* data,
        size_t length)
    {
        uint32_t crc = 0xffffffffUL;

        for (size_t index = 0;
            index < length;
            ++index)
        {
            crc ^= data[index];

            for (uint8_t bit = 0;
                bit < 8;
                ++bit)
            {
                const uint32_t mask =
                    0UL - (crc & 1UL);
                crc =
                    (crc >> 1) ^ (0xedb88320UL & mask);
            }
        }

        return ~crc;
    }

    size_t encodeFrame(
        uint8_t type,
        uint8_t flags,
        uint16_t sequence,
        const uint8_t* payload,
        uint16_t payloadLength,
        uint8_t* output,
        size_t outputSize)
    {
        if (payloadLength > COCKPITLINK_MAX_PAYLOAD)
        {
            payloadLength = COCKPITLINK_MAX_PAYLOAD;
        }

        const size_t totalSize =
            COCKPITLINK_HEADER_SIZE +
            payloadLength +
            COCKPITLINK_CHECKSUM_SIZE;

        if (outputSize < totalSize)
        {
            return 0;
        }

        output[0] = COCKPITLINK_MAGIC0;
        output[1] = COCKPITLINK_MAGIC1;
        output[2] = COCKPITLINK_PROTOCOL_VERSION;
        output[3] = type;
        output[4] = flags;
        writeU16(&output[5], sequence);
        writeU16(&output[7], payloadLength);

        if (payloadLength > 0 && payload != nullptr)
        {
            memcpy(
                &output[COCKPITLINK_HEADER_SIZE],
                payload,
                payloadLength);
        }

        const uint32_t checksum =
            cockpitLinkCrc32(
                output,
                COCKPITLINK_HEADER_SIZE + payloadLength);

        writeU32(
            &output[COCKPITLINK_HEADER_SIZE + payloadLength],
            checksum);

        return totalSize;
    }

    size_t encodeHelloPayload(
        const char* deviceName,
        const char* firmwareVersion,
        uint16_t maxPayload,
        uint16_t capabilityFlags,
        uint8_t* output,
        size_t outputSize)
    {
        const char* name =
            deviceName == nullptr ? "" : deviceName;
        const char* firmware =
            firmwareVersion == nullptr ? "" : firmwareVersion;

        const size_t nameLength =
            strlen(name);
        const size_t firmwareLength =
            strlen(firmware);

        const size_t totalPayload =
            nameLength + 1 + firmwareLength + 1 + 6;

        if (outputSize < totalPayload)
        {
            return 0;
        }

        memcpy(output, name, nameLength);
        output[nameLength] = 0;
        memcpy(
            output + nameLength + 1,
            firmware,
            firmwareLength);
        output[nameLength + 1 + firmwareLength] = 0;

        size_t offset =
            nameLength + 1 + firmwareLength + 1;
        writeU16(output + offset, maxPayload);
        offset += 2;
        output[offset++] = COCKPITLINK_MIN_PROTOCOL_VERSION;
        output[offset++] = COCKPITLINK_PROTOCOL_VERSION;
        writeU16(output + offset, capabilityFlags);

        return totalPayload;
    }

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

    PotentiometerBinding::PotentiometerBinding(
        uint8_t pin,
        const char* behaviorId)
        : pin_(pin),
        behaviorId_(behaviorId)
    {
        pinMode(pin_, INPUT);
    }

    uint8_t PotentiometerBinding::pin() const
    {
        return pin_;
    }

    const char* PotentiometerBinding::behaviorId() const
    {
        return behaviorId_;
    }

    SwitchBuilder::SwitchBuilder(
        CockpitLinkDevice* device,
        uint8_t pin)
        : device_(device),
        pin_(pin)
    {
    }

    SwitchBinding SwitchBuilder::controls(
        const char* behaviorId) const
    {
        if (device_ != nullptr)
        {
            device_->addBinding(
                CockpitLinkDevice::BindingRole::Controls,
                CockpitLinkDevice::BindingInput::Digital,
                pin_,
                behaviorId);
        }

        return { pin_, behaviorId };
    }

    OutputBuilder::OutputBuilder(
        CockpitLinkDevice* device,
        uint8_t pin)
        : device_(device),
        pin_(pin)
    {
    }

    OutputBinding OutputBuilder::follows(
        const char* behaviorId) const
    {
        if (device_ != nullptr)
        {
            device_->addBinding(
                CockpitLinkDevice::BindingRole::Follows,
                CockpitLinkDevice::BindingInput::Digital,
                pin_,
                behaviorId);
        }

        return { pin_, behaviorId };
    }

    ButtonBuilder::ButtonBuilder(
        CockpitLinkDevice* device,
        uint8_t pin)
        : device_(device),
        pin_(pin)
    {
    }

    ButtonBinding ButtonBuilder::triggers(
        const char* behaviorId) const
    {
        if (device_ != nullptr)
        {
            device_->addBinding(
                CockpitLinkDevice::BindingRole::Triggers,
                CockpitLinkDevice::BindingInput::Digital,
                pin_,
                behaviorId);
        }

        return { pin_, behaviorId };
    }

    PotentiometerBuilder::PotentiometerBuilder(
        CockpitLinkDevice* device,
        uint8_t pin)
        : device_(device),
        pin_(pin)
    {
    }

    PotentiometerBuilder::PotentiometerBuilder(
        CockpitLinkDevice* device,
        uint8_t pin,
        int rawMin,
        int rawMax,
        uint8_t deadbandPercent,
        uint8_t bucketPercent,
        uint16_t sampleIntervalMs)
        : device_(device),
        pin_(pin),
        rawMin_(rawMin),
        rawMax_(rawMax),
        deadbandPercent_(deadbandPercent),
        bucketPercent_(bucketPercent),
        sampleIntervalMs_(sampleIntervalMs)
    {
    }

    PotentiometerBuilder PotentiometerBuilder::calibrated(
        int rawMin,
        int rawMax) const
    {
        return PotentiometerBuilder{
            device_,
            pin_,
            rawMin,
            rawMax,
            deadbandPercent_,
            bucketPercent_,
            sampleIntervalMs_
        };
    }

    PotentiometerBuilder PotentiometerBuilder::deadband(
        uint8_t percent) const
    {
        return PotentiometerBuilder{
            device_,
            pin_,
            rawMin_,
            rawMax_,
            percent,
            bucketPercent_,
            sampleIntervalMs_
        };
    }

    PotentiometerBuilder PotentiometerBuilder::bucket(
        uint8_t percent) const
    {
        return PotentiometerBuilder{
            device_,
            pin_,
            rawMin_,
            rawMax_,
            deadbandPercent_,
            percent == 0 ? 1 : percent,
            sampleIntervalMs_
        };
    }

    PotentiometerBuilder PotentiometerBuilder::sampleEvery(
        uint16_t milliseconds) const
    {
        return PotentiometerBuilder{
            device_,
            pin_,
            rawMin_,
            rawMax_,
            deadbandPercent_,
            bucketPercent_,
            milliseconds
        };
    }

    PotentiometerBinding PotentiometerBuilder::controls(
        const char* behaviorId) const
    {
        if (device_ != nullptr)
        {
            device_->addBinding(
                CockpitLinkDevice::BindingRole::Controls,
                CockpitLinkDevice::BindingInput::AnalogPercent,
                pin_,
                behaviorId,
                rawMin_,
                rawMax_,
                deadbandPercent_,
                bucketPercent_,
                sampleIntervalMs_);
        }

        return { pin_, behaviorId };
    }

    int PotentiometerBuilder::readRaw() const
    {
        return analogRead(pin_);
    }

    int PotentiometerBuilder::readMapped(
        int minValue,
        int maxValue) const
    {
        const long raw =
            readRaw();
        const long mapped =
            map(raw, 0, 1023, minValue, maxValue);

        if (minValue <= maxValue)
        {
            if (mapped < minValue)
            {
                return minValue;
            }

            if (mapped > maxValue)
            {
                return maxValue;
            }
        }
        else
        {
            if (mapped > minValue)
            {
                return minValue;
            }

            if (mapped < maxValue)
            {
                return maxValue;
            }
        }

        return static_cast<int>(mapped);
    }

    int PotentiometerBuilder::readPercent() const
    {
        if (rawMin_ == rawMax_)
        {
            return 0;
        }

        const long raw =
            readRaw();
        const long mapped =
            ((raw - rawMin_) * 100L) / (rawMax_ - rawMin_);

        if (mapped < 0)
        {
            return 0;
        }

        if (mapped > 100)
        {
            return 100;
        }

        int percent =
            static_cast<int>(mapped);

        if (bucketPercent_ > 1)
        {
            const int bucket =
                bucketPercent_;
            percent =
                ((percent + (bucket / 2)) / bucket) * bucket;

            if (percent > 100)
            {
                percent = 100;
            }
        }

        return percent;
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
        processSerial();
        processControls();
    }

    void CockpitLinkDevice::controlRefreshEvery(
        uint16_t intervalMs)
    {
        controlRefreshIntervalMs_ = intervalMs;
    }

    SwitchBuilder CockpitLinkDevice::switchInput(
        uint8_t pin)
    {
        return SwitchBuilder{ this, pin };
    }

    OutputBuilder CockpitLinkDevice::digitalOutput(
        uint8_t pin)
    {
        return OutputBuilder{ this, pin };
    }

    ButtonBuilder CockpitLinkDevice::button(
        uint8_t pin)
    {
        return ButtonBuilder{ this, pin };
    }

    PotentiometerBuilder CockpitLinkDevice::potentiometer(
        uint8_t pin)
    {
        return PotentiometerBuilder{ this, pin };
    }

    const char* CockpitLinkDevice::deviceName() const
    {
        return deviceName_;
    }

    const char* CockpitLinkDevice::firmwareVersion() const
    {
        return firmwareVersion_;
    }

    void CockpitLinkDevice::processSerial()
    {
        while (Serial.available() > 0)
        {
            const uint8_t byte =
                static_cast<uint8_t>(Serial.read());

            switch (parserState_)
            {
            case ParserState::SeekingMagic0:
                if (byte == COCKPITLINK_MAGIC0)
                {
                    resetParser();
                    header_[0] = byte;
                    headerIndex_ = 1;
                    parserState_ = ParserState::SeekingMagic1;
                }
                break;

            case ParserState::SeekingMagic1:
                if (byte == COCKPITLINK_MAGIC1)
                {
                    header_[1] = byte;
                    headerIndex_ = 2;
                    parserState_ = ParserState::ReadingHeader;
                }
                else
                {
                    resetParser();
                }
                break;

            case ParserState::ReadingHeader:
                header_[headerIndex_++] = byte;

                if (headerIndex_ < COCKPITLINK_HEADER_SIZE)
                {
                    break;
                }

                if (header_[2] != COCKPITLINK_PROTOCOL_VERSION)
                {
                    resetParser();
                    break;
                }

                payloadLength_ =
                    readU16(header_[7], header_[8]);

                if (payloadLength_ > COCKPITLINK_MAX_PAYLOAD)
                {
                    resetParser();
                    break;
                }

                payloadIndex_ = 0;
                parserState_ =
                    payloadLength_ == 0 ?
                        ParserState::ReadingChecksum :
                        ParserState::ReadingPayload;
                break;

            case ParserState::ReadingPayload:
                payload_[payloadIndex_++] = byte;

                if (payloadIndex_ == payloadLength_)
                {
                    checksumIndex_ = 0;
                    parserState_ = ParserState::ReadingChecksum;
                }
                break;

            case ParserState::ReadingChecksum:
                checksum_[checksumIndex_++] = byte;

                if (checksumIndex_ == COCKPITLINK_CHECKSUM_SIZE)
                {
                    if (finishFrame())
                    {
                        processFrame(inboundFrame_);
                    }

                    resetParser();
                }
                break;
            }
        }
    }

    void CockpitLinkDevice::processFrame(
        const ProtocolFrame& frame)
    {
        if (frame.type == COCKPITLINK_MSG_HELLO)
        {
            resetRegistration();
            sendHelloAck(frame.sequence);
            connected_ = true;
            sendBehaviorRequests();
        }
        else if (frame.type == COCKPITLINK_MSG_BEHAVIOR_ASSIGNMENT)
        {
            handleBehaviorAssignment(frame);
        }
        else if (frame.type == COCKPITLINK_MSG_VALUE_UPDATE)
        {
            handleValueUpdate(frame);
        }
    }

    void CockpitLinkDevice::processControls()
    {
        const unsigned long now =
            millis();

        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            Binding& binding =
                bindings_[index];

            if (!binding.assigned ||
                binding.role != BindingRole::Controls)
            {
                continue;
            }

            if (now - binding.lastSentAt < binding.sampleIntervalMs)
            {
                continue;
            }

            int value = 0;

            if (binding.input == BindingInput::Digital)
            {
                value =
                    digitalRead(binding.pin) == LOW ? 1 : 0;
            }
            else if (binding.input == BindingInput::AnalogPercent)
            {
                value =
                    PotentiometerBuilder{
                        this,
                        binding.pin
                    }
                        .calibrated(
                            binding.rawMin,
                            binding.rawMax)
                        .bucket(
                            binding.bucketPercent)
                        .readPercent();
            }
            else
            {
                continue;
            }

            const bool refreshDue =
                controlRefreshIntervalMs_ > 0 &&
                binding.lastSentValue >= 0 &&
                now - binding.lastSentAt >= controlRefreshIntervalMs_;

            if (!refreshDue &&
                binding.lastSentValue >= 0 &&
                abs(value - binding.lastSentValue) <
                    binding.deadbandPercent)
            {
                continue;
            }

            if (binding.input == BindingInput::Digital)
            {
                sendBoolValueUpdate(
                    binding.handle,
                    value != 0);
            }
            else
            {
                sendIntValueUpdate(
                    binding.handle,
                    value);
            }

            binding.lastSentValue = value;
            binding.lastSentAt = now;
        }
    }

    void CockpitLinkDevice::resetRegistration()
    {
        connected_ = false;

        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            Binding& binding =
                bindings_[index];

            binding.handle = 0;
            binding.assigned = false;
            binding.requested = false;
            binding.lastSentValue = -1;
            binding.lastSentAt = 0;
        }
    }

    void CockpitLinkDevice::sendHelloAck(
        uint16_t sequence)
    {
        uint8_t payload[COCKPITLINK_MAX_PAYLOAD]{};
        const size_t payloadLength =
            encodeHelloPayload(
                deviceName_,
                firmwareVersion_,
                COCKPITLINK_MAX_PAYLOAD,
                COCKPITLINK_CAP_SERIAL |
                    COCKPITLINK_CAP_BEHAVIOR_IDS |
                    COCKPITLINK_CAP_BINARY_VALUES |
                    COCKPITLINK_CAP_DECODED_DIAGNOSTICS,
                payload,
                sizeof(payload));

        if (payloadLength == 0)
        {
            return;
        }

        uint8_t output[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD +
            COCKPITLINK_CHECKSUM_SIZE]{};

        const size_t outputSize =
            encodeFrame(
                COCKPITLINK_MSG_HELLO_ACK,
                0,
                sequence,
                payload,
                static_cast<uint16_t>(payloadLength),
                output,
                sizeof(output));

        if (outputSize > 0)
        {
            Serial.write(output, outputSize);
        }
    }

    void CockpitLinkDevice::sendBehaviorRequests()
    {
        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            sendBehaviorRequest(bindings_[index]);
        }
    }

    void CockpitLinkDevice::sendBehaviorRequest(
        Binding& binding)
    {
        if (binding.behaviorId == nullptr ||
            binding.requested)
        {
            return;
        }

        uint8_t payload[COCKPITLINK_MAX_PAYLOAD]{};
        const size_t behaviorLength =
            strlen(binding.behaviorId);
        const size_t payloadLength =
            behaviorLength + 3;

        if (payloadLength > sizeof(payload))
        {
            return;
        }

        payload[0] = binding.requestId;
        payload[1] = static_cast<uint8_t>(binding.role);
        memcpy(
            payload + 2,
            binding.behaviorId,
            behaviorLength);
        payload[2 + behaviorLength] = 0;

        uint8_t output[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD +
            COCKPITLINK_CHECKSUM_SIZE]{};

        const size_t outputSize =
            encodeFrame(
                COCKPITLINK_MSG_BEHAVIOR_REQUEST,
                0,
                nextSequence_++,
                payload,
                static_cast<uint16_t>(payloadLength),
                output,
                sizeof(output));

        if (outputSize > 0)
        {
            Serial.write(output, outputSize);
            binding.requested = true;
        }
    }

    void CockpitLinkDevice::sendSubscribe(
        const Binding& binding)
    {
        if (!binding.assigned ||
            binding.role != BindingRole::Follows)
        {
            return;
        }

        uint8_t payload[5]{};
        writeU16(payload, binding.handle);
        payload[2] = COCKPITLINK_VALUE_BOOL;
        writeU16(payload + 3, 100);

        uint8_t output[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD +
            COCKPITLINK_CHECKSUM_SIZE]{};

        const size_t outputSize =
            encodeFrame(
                COCKPITLINK_MSG_SUBSCRIBE,
                0,
                nextSequence_++,
                payload,
                sizeof(payload),
                output,
                sizeof(output));

        if (outputSize > 0)
        {
            Serial.write(output, outputSize);
        }
    }

    void CockpitLinkDevice::handleBehaviorAssignment(
        const ProtocolFrame& frame)
    {
        if (frame.payloadLength < 6)
        {
            return;
        }

        Binding* binding =
            findBindingByRequest(frame.payload[0]);

        if (binding == nullptr)
        {
            return;
        }

        binding->handle =
            readPayloadU16(frame, 1);
        binding->assigned = true;

        sendSubscribe(*binding);
    }

    void CockpitLinkDevice::handleValueUpdate(
        const ProtocolFrame& frame)
    {
        if (frame.payloadLength < 6)
        {
            return;
        }

        const uint16_t handle =
            readPayloadU16(frame, 0);
        const uint8_t valueType =
            frame.payload[2];
        const uint16_t valueLength =
            readPayloadU16(frame, 3);

        if (valueType != COCKPITLINK_VALUE_BOOL ||
            valueLength != 1 ||
            frame.payloadLength < 6)
        {
            return;
        }

        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            Binding& binding =
                bindings_[index];

            if (!binding.assigned ||
                binding.handle != handle ||
                binding.role != BindingRole::Follows)
            {
                continue;
            }

            digitalWrite(
                binding.pin,
                frame.payload[5] != 0 ? HIGH : LOW);
        }
    }

    void CockpitLinkDevice::sendBoolValueUpdate(
        uint16_t handle,
        bool value)
    {
        uint8_t payload[6]{};
        writeU16(payload, handle);
        payload[2] = COCKPITLINK_VALUE_BOOL;
        writeU16(payload + 3, 1);
        payload[5] = value ? 1 : 0;

        uint8_t output[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD +
            COCKPITLINK_CHECKSUM_SIZE]{};

        const size_t outputSize =
            encodeFrame(
                COCKPITLINK_MSG_VALUE_UPDATE,
                0,
                nextSequence_++,
                payload,
                sizeof(payload),
                output,
                sizeof(output));

        if (outputSize > 0)
        {
            Serial.write(output, outputSize);
        }
    }

    void CockpitLinkDevice::sendIntValueUpdate(
        uint16_t handle,
        int32_t value)
    {
        uint8_t payload[9]{};
        writeU16(payload, handle);
        payload[2] = COCKPITLINK_VALUE_INT;
        writeU16(payload + 3, 4);
        writeI32(payload + 5, value);

        uint8_t output[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD +
            COCKPITLINK_CHECKSUM_SIZE]{};

        const size_t outputSize =
            encodeFrame(
                COCKPITLINK_MSG_VALUE_UPDATE,
                0,
                nextSequence_++,
                payload,
                sizeof(payload),
                output,
                sizeof(output));

        if (outputSize > 0)
        {
            Serial.write(output, outputSize);
        }
    }

    uint8_t CockpitLinkDevice::addBinding(
        BindingRole role,
        BindingInput input,
        uint8_t pin,
        const char* behaviorId,
        int rawMin,
        int rawMax,
        uint8_t deadbandPercent,
        uint8_t bucketPercent,
        uint16_t sampleIntervalMs)
    {
        if (bindingCount_ >= maxBindings_)
        {
            return 0xff;
        }

        const uint8_t requestId =
            bindingCount_;

        Binding& binding =
            bindings_[bindingCount_++];

        binding.requestId = requestId;
        binding.role = role;
        binding.input = input;
        binding.pin = pin;
        binding.behaviorId = behaviorId;
        binding.handle = 0;
        binding.assigned = false;
        binding.requested = false;
        binding.rawMin = rawMin;
        binding.rawMax = rawMax;
        binding.deadbandPercent = deadbandPercent;
        binding.bucketPercent = bucketPercent;
        binding.sampleIntervalMs = sampleIntervalMs;
        binding.lastSentValue = -1;
        binding.lastSentAt = 0;

        if (connected_)
        {
            sendBehaviorRequest(bindings_[requestId]);
        }

        return requestId;
    }

    CockpitLinkDevice::Binding*
        CockpitLinkDevice::findBindingByRequest(
            uint8_t requestId)
    {
        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            if (bindings_[index].requestId == requestId)
            {
                return &bindings_[index];
            }
        }

        return nullptr;
    }

    CockpitLinkDevice::Binding*
        CockpitLinkDevice::findBindingByHandle(
            uint16_t handle)
    {
        for (uint8_t index = 0;
            index < bindingCount_;
            ++index)
        {
            if (bindings_[index].assigned &&
                bindings_[index].handle == handle)
            {
                return &bindings_[index];
            }
        }

        return nullptr;
    }

    uint16_t CockpitLinkDevice::readPayloadU16(
        const ProtocolFrame& frame,
        uint16_t offset) const
    {
        if (offset + 1 >= frame.payloadLength)
        {
            return 0;
        }

        return readU16(
            frame.payload[offset],
            frame.payload[offset + 1]);
    }

    void CockpitLinkDevice::resetParser()
    {
        parserState_ = ParserState::SeekingMagic0;
        headerIndex_ = 0;
        payloadIndex_ = 0;
        payloadLength_ = 0;
        checksumIndex_ = 0;
    }

    bool CockpitLinkDevice::finishFrame()
    {
        uint8_t covered[
            COCKPITLINK_HEADER_SIZE +
            COCKPITLINK_MAX_PAYLOAD]{};

        memcpy(
            covered,
            header_,
            COCKPITLINK_HEADER_SIZE);

        if (payloadLength_ > 0)
        {
            memcpy(
                covered + COCKPITLINK_HEADER_SIZE,
                payload_,
                payloadLength_);
        }

        const uint32_t actual =
            cockpitLinkCrc32(
                covered,
                COCKPITLINK_HEADER_SIZE + payloadLength_);

        if (actual != readU32(checksum_))
        {
            return false;
        }

        inboundFrame_.type = header_[3];
        inboundFrame_.flags = header_[4];
        inboundFrame_.sequence = readU16(header_[5], header_[6]);
        inboundFrame_.payloadLength = payloadLength_;

        if (payloadLength_ > 0)
        {
            memcpy(
                inboundFrame_.payload,
                payload_,
                payloadLength_);
        }

        return true;
    }
}

cockpitlink::CockpitLinkDevice CockpitLink;

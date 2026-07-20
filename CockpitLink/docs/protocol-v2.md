# CockpitLink Protocol V2

The successor protocol should be binary-framed and transport-neutral. The serial
logger decodes frames into readable text.

## Frame Shape

```text
magic | version | type | flags | sequence | payloadLength | payload | checksum
```

Suggested field sizes:

```text
magic         2 bytes  "CL"
version       1 byte
type          1 byte
flags         1 byte
sequence      2 bytes
payloadLength 2 bytes
payload       N bytes
checksum      4 bytes CRC32
```

The first host implementation has an encoder/parser for this shape and tests
partial reads, CRC rejection, leading noise, and embedded `0x00` bytes in the
payload.

## Message Families

- Hello and capability negotiation.
- Profile acceptance and handle assignment.
- Behavior registration.
- Subscription requests.
- Typed value updates.
- Command actions.
- Flow control.
- Diagnostics.

## Typed Values

Every value update carries a type and explicit length.

```text
handle:uint16
valueType:uint8
element:int16
valueLength:uint16
valueBytes:N
```

This keeps `0x00` safe inside `string` and `data` payloads.

## Profile Shortcut

If the host has a matching behavior profile:

```text
Host -> Device: profile accepted, dataref/behavior handles are valid
Device -> Host: subscribe to known handles
```

If no matching profile exists:

```text
Host -> Device: send behavior requests
Device -> Host: behavior ID list
Host -> Device: assigned handles
```

## Flow Control

The host should advertise:

- Maximum payload size.
- Maximum bytes per tick.
- Maximum frames per tick.
- Whether acknowledgements are required.

The device should advertise:

- Receive buffer size.
- Transmit buffer size.
- Supported transports.
- Supported protocol versions.

## Decoded Log Examples

```text
RX hello device="737 MCP" firmware="2026.07.20" protocol=2 max-rx=512
TX profile accepted behaviors=42 commands=18
RX subscribe handle=12 type=bool rate=100 bucket=1
TX update handle=12 behavior=lights.beacon value=false
RX command handle=4 behavior=autopilot.ap_disconnect action=trigger count=1
```

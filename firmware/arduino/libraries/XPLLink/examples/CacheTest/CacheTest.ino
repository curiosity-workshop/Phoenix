#include <XPLLink.h>

XPLLink XP(Serial);

dref_handle drefNavLights = XPL_HANDLE_INVALID;

void xplRegister()
{
    drefNavLights =
        XP.registerDataRef(F("sim/cockpit2/switches/navigation_lights_on"));

    XP.requestUpdates(drefNavLights, 100, 0.0000);
}

void xplShutdown()
{
    drefNavLights = XPL_HANDLE_INVALID;
}

void xplInboundHandler(XPLLinkData* inData)
{
    if (inData->handle != drefNavLights)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, inData->inLong ? HIGH : LOW);
}

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(XPL_BAUDRATE);
    XP.begin(
        "XPLLink Cache Test",
        &xplRegister,
        &xplShutdown,
        &xplInboundHandler);
}

void loop()
{
    XP.xloop();
}

#include <CockpitLink.h>

void setup()
{
    CockpitLink.begin("Concept Button Box", "2026.07.20");

    CockpitLink.switchInput(2)
        .controls("lights.beacon");

    CockpitLink.digitalOutput(LED_BUILTIN)
        .follows("lights.beacon");

    CockpitLink.button(3)
        .triggers("autopilot.ap_disconnect");
}

void loop()
{
    CockpitLink.loop();
}

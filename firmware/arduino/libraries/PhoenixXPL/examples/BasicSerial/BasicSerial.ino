#include <PhoenixXPL.h>

phoenix::PhoenixXPL XP(Serial);

void setup()
{
    Serial.begin(115200);
    XP.begin("Phoenix Basic Serial");
}

void loop()
{
    XP.loop();
}

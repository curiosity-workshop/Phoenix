#include <XPLLink.h>

phoenix::XPLLink XP(Serial);

void setup()
{
    Serial.begin(115200);
    XP.begin("XPLLink Basic Serial");
}

void loop()
{
    XP.xloop();
}

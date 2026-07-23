#include <CockpitLink.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>

LiquidCrystal_I2C lcd27(0x27, 16, 2);

constexpr int axisRawMinimum = 0;
constexpr int axisRawMaximum = 1023;
constexpr uint8_t axisDeadbandPercent = 2;
constexpr uint8_t axisBucketPercent = 5;
constexpr uint16_t axisSampleMs = 100;
constexpr uint8_t beaconSwitchPin = 10;
constexpr uint8_t navSwitchPin = 11;
constexpr uint8_t strobeSwitchPin = 12;

void updatePotentiometerDisplay();
void createAxisBarCharacters();
uint8_t axisBarCharacter(int percent);
char switchStateChar(uint8_t pin);

void setup()
{
    CockpitLink.begin("Concept Button Box", __DATE__ " " __TIME__);

    CockpitLink.i2cLcd(lcd27)
        .begin()
        .line(0, "TT PP MM")
        .line(1, "SW:---");

    createAxisBarCharacters();

    CockpitLink.switchInput(beaconSwitchPin)
        .controls("lights.beacon");

    CockpitLink.switchInput(navSwitchPin)
        .controls("lights.nav");

    CockpitLink.switchInput(strobeSwitchPin)
        .controls("lights.strobe");

    CockpitLink.digitalOutput(LED_BUILTIN)
        .follows("lights.beacon");

    CockpitLink.button(3)
        .triggers("autopilot.ap_disconnect");

    CockpitLink.potentiometer(A0)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.1.throttle");

    CockpitLink.potentiometer(A1)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.2.throttle");

    CockpitLink.potentiometer(A2)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.1.prop_rpm");

    CockpitLink.potentiometer(A3)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.2.prop_rpm");

    CockpitLink.potentiometer(A4)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.1.mixture");

    CockpitLink.potentiometer(A5)
        .calibrated(axisRawMinimum, axisRawMaximum)
        .deadband(axisDeadbandPercent)
        .bucket(axisBucketPercent)
        .sampleEvery(axisSampleMs)
        .controls("engine.2.mixture");
}

void loop()
{
    CockpitLink.loop();
    updatePotentiometerDisplay();
}

void updatePotentiometerDisplay()
{
    static unsigned long lastSampleAt = 0;
    static int lastThrottle1Percent = -1;
    static int lastThrottle2Percent = -1;
    static int lastProp1Percent = -1;
    static int lastProp2Percent = -1;
    static int lastMixture1Percent = -1;
    static int lastMixture2Percent = -1;
    static char lastSwitch1 = 0;
    static char lastSwitch2 = 0;
    static char lastSwitch3 = 0;

    const unsigned long now =
        millis();

    if (now - lastSampleAt < 100)
    {
        return;
    }

    lastSampleAt = now;

    const int throttle1Percent =
        CockpitLink.potentiometer(A0)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();
    const int throttle2Percent =
        CockpitLink.potentiometer(A1)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();
    const int prop1Percent =
        CockpitLink.potentiometer(A2)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();
    const int prop2Percent =
        CockpitLink.potentiometer(A3)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();
    const int mixture1Percent =
        CockpitLink.potentiometer(A4)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();
    const int mixture2Percent =
        CockpitLink.potentiometer(A5)
            .calibrated(axisRawMinimum, axisRawMaximum)
            .bucket(axisBucketPercent)
            .readPercent();

    const char switch1 =
        switchStateChar(beaconSwitchPin);
    const char switch2 =
        switchStateChar(navSwitchPin);
    const char switch3 =
        switchStateChar(strobeSwitchPin);

    if (throttle1Percent == lastThrottle1Percent &&
        throttle2Percent == lastThrottle2Percent &&
        prop1Percent == lastProp1Percent &&
        prop2Percent == lastProp2Percent &&
        mixture1Percent == lastMixture1Percent &&
        mixture2Percent == lastMixture2Percent &&
        switch1 == lastSwitch1 &&
        switch2 == lastSwitch2 &&
        switch3 == lastSwitch3)
    {
        return;
    }

    lastThrottle1Percent = throttle1Percent;
    lastThrottle2Percent = throttle2Percent;
    lastProp1Percent = prop1Percent;
    lastProp2Percent = prop2Percent;
    lastMixture1Percent = mixture1Percent;
    lastMixture2Percent = mixture2Percent;
    lastSwitch1 = switch1;
    lastSwitch2 = switch2;
    lastSwitch3 = switch3;

    char bottomLine[17];

    snprintf(
        bottomLine,
        sizeof(bottomLine),
        "SW:%c%c%c---",
        switch1,
        switch2,
        switch3);

    lcd27.setCursor(0, 0);
    lcd27.print('T');
    lcd27.write(axisBarCharacter(throttle1Percent));
    lcd27.print(' ');
    lcd27.print('T');
    lcd27.write(axisBarCharacter(throttle2Percent));
    lcd27.print(' ');
    lcd27.print('P');
    lcd27.write(axisBarCharacter(prop1Percent));
    lcd27.print(' ');
    lcd27.print('P');
    lcd27.write(axisBarCharacter(prop2Percent));
    lcd27.print(' ');
    lcd27.print('M');
    lcd27.write(axisBarCharacter(mixture1Percent));
    lcd27.print(' ');
    lcd27.print('M');
    lcd27.write(axisBarCharacter(mixture2Percent));

    CockpitLink.i2cLcd(lcd27)
        .line(1, bottomLine);
}

void createAxisBarCharacters()
{
    byte emptyBar[8] = {
        0b11111,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11111
    };
    byte oneThirdBar[8] = {
        0b11111,
        0b10001,
        0b10001,
        0b10001,
        0b10001,
        0b11111,
        0b11111,
        0b11111
    };
    byte twoThirdsBar[8] = {
        0b11111,
        0b10001,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111
    };
    byte fullBar[8] = {
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111,
        0b11111
    };

    lcd27.createChar(0, emptyBar);
    lcd27.createChar(1, oneThirdBar);
    lcd27.createChar(2, twoThirdsBar);
    lcd27.createChar(3, fullBar);
}

uint8_t axisBarCharacter(int percent)
{
    if (percent < 25)
    {
        return 0;
    }

    if (percent < 55)
    {
        return 1;
    }

    if (percent < 85)
    {
        return 2;
    }

    return 3;
}

char switchStateChar(uint8_t pin)
{
    return digitalRead(pin) == LOW ? '*' : '-';
}

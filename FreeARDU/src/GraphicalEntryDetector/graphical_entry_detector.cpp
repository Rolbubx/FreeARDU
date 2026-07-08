#include "graphical_entry_detector.h"

// Stubs for bare-metal if Arduino is missing
#ifdef FREEARDU_BARE_METAL
extern "C" {
    void Wire_begin();
    void Wire_beginTransmission(uint8_t address);
    uint8_t Wire_endTransmission();
    bool isPinConnected(int pin);
}
#define Wire_begin() Wire_begin()
#define Wire_beginTransmission(addr) Wire_beginTransmission(addr)
#define Wire_endTransmission() Wire_endTransmission()
#else
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#define Wire_begin() Wire.begin()
#define Wire_beginTransmission(addr) Wire.beginTransmission(addr)
#define Wire_endTransmission() Wire.endTransmission()
#define isPinConnected(p) false // Placeholder
#endif

static ScreenDetectionResult makeEmptyResult() {
    ScreenDetectionResult result;

    result.type = SCREEN_NONE;
    result.i2cAddress = 0;
    result.detectedPin = -1;

    result.pinCount = 0;

    for (uint8_t i = 0; i < MAX_SCREEN_PINS; i++) {
        result.pins[i] = -1;
    }

    result.sdaPin = -1;
    result.sclPin = -1;

    result.mosiPin = -1;
    result.misoPin = -1;
    result.sckPin = -1;
    result.csPin = -1;
    result.dcPin = -1;
    result.resetPin = -1;

    result.parallelDataPinCount = 0;

    for (uint8_t i = 0; i < MAX_PARALLEL_DATA_PINS; i++) {
        result.dataPins[i] = -1;
    }

    result.canDraw = false;
    result.width = 0;
    result.height = 0;

    return result;
}

static void addPin(ScreenDetectionResult &result, int pin) {
    if (pin < 0) {
        return;
    }

    if (result.pinCount >= MAX_SCREEN_PINS) {
        return;
    }

    for (uint8_t i = 0; i < result.pinCount; i++) {
        if (result.pins[i] == pin) {
            return;
        }
    }

    result.pins[result.pinCount] = pin;
    result.pinCount++;
}

static bool probeI2CAddress(uint8_t address) {
    Wire_beginTransmission(address);
    return Wire_endTransmission() == 0;
}

static ScreenDetectionResult detectI2CScreen() {
    const uint8_t oledAddresses[] = {
        0x3C,
        0x3D
    };

    const uint8_t lcdAddresses[] = {
        0x20,
        0x21,
        0x22,
        0x23,
        0x24,
        0x25,
        0x26,
        0x27,
        0x38,
        0x39,
        0x3A,
        0x3B,
        0x3E,
        0x3F
    };

    for (uint8_t i = 0; i < sizeof(oledAddresses); i++) {
        if (probeI2CAddress(oledAddresses[i])) {
            ScreenDetectionResult res = makeEmptyResult();
            res.type = SCREEN_I2C_OLED;
            res.i2cAddress = oledAddresses[i];
            res.width = 128;
            res.height = 64;
            res.canDraw = true;
            return res;
        }
    }

    for (uint8_t i = 0; i < sizeof(lcdAddresses); i++) {
        if (probeI2CAddress(lcdAddresses[i])) {
            ScreenDetectionResult res = makeEmptyResult();
            res.type = SCREEN_I2C_LCD;
            res.i2cAddress = lcdAddresses[i];
            res.width = 16;
            res.height = 2;
            res.canDraw = true;
            return res;
        }
    }

    return makeEmptyResult();
}

static ScreenDetectionResult detectSPIScreen() {
    const int possiblePins[] = {5, 6, 7, 8, 9, 10, 11, 12, 13};

    int connectedCount = 0;
    int firstConnectedPin = -1;

    for (uint8_t i = 0; i < sizeof(possiblePins) / sizeof(possiblePins[0]); i++) {
        if (isPinConnected(possiblePins[i])) {
            connectedCount++;
            if (firstConnectedPin == -1) firstConnectedPin = possiblePins[i];
        }
    }

    if (connectedCount >= 3) {
        ScreenDetectionResult res = makeEmptyResult();
        res.type = SCREEN_SPI_DISPLAY;
        res.detectedPin = firstConnectedPin;
        res.width = 320;
        res.height = 240;
        res.canDraw = true;
        return res;
    }

    return makeEmptyResult();
}

static ScreenDetectionResult detectParallelScreen() {
    const int parallelPins[] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 23};

    int connectedCount = 0;
    int firstConnectedPin = -1;

    for (uint8_t i = 0; i < sizeof(parallelPins) / sizeof(parallelPins[0]); i++) {
        if (isPinConnected(parallelPins[i])) {
            connectedCount++;
            if (firstConnectedPin == -1) firstConnectedPin = parallelPins[i];
        }
    }

    if (connectedCount >= 6) {
        ScreenDetectionResult res = makeEmptyResult();
        res.type = SCREEN_PARALLEL_DISPLAY;
        res.detectedPin = firstConnectedPin;
        res.width = 480;
        res.height = 320;
        res.canDraw = true;
        return res;
    }

    return makeEmptyResult();
}

extern "C" ScreenDetectionResult DETECT_SCRN() {
    Wire_begin();

    ScreenDetectionResult i2cResult = detectI2CScreen();
    if (i2cResult.type != SCREEN_NONE) return i2cResult;

    ScreenDetectionResult spiResult = detectSPIScreen();
    if (spiResult.type != SCREEN_NONE) return spiResult;

    ScreenDetectionResult parallelResult = detectParallelScreen();
    if (parallelResult.type != SCREEN_NONE) return parallelResult;

    return makeEmptyResult();
}
#include "graphical_entry_detector.h"

// Stubs for bare-metal if Arduino is missing
#ifdef FREEARDU_BARE_METAL
extern "C" {
    bool isPinConnected(int pin);
}
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
#if defined(FREEARDU_FORCE_SCREEN_TYPE) && FREEARDU_FORCE_SCREEN_TYPE == 1
    ScreenDetectionResult res = makeEmptyResult();
    res.type = SCREEN_SPI_DISPLAY;
    res.width = 320;
    res.height = 240;
    res.canDraw = true;
    return res;
#elif defined(FREEARDU_FORCE_SCREEN_TYPE) && FREEARDU_FORCE_SCREEN_TYPE == 2
    ScreenDetectionResult res = makeEmptyResult();
    res.type = SCREEN_PARALLEL_DISPLAY;
    res.width = 320;
    res.height = 240;
    res.canDraw = true;
    return res;
#endif

    ScreenDetectionResult spiResult = detectSPIScreen();
    if (spiResult.type != SCREEN_NONE) return spiResult;

    ScreenDetectionResult parallelResult = detectParallelScreen();
    if (parallelResult.type != SCREEN_NONE) return parallelResult;

    return makeEmptyResult();
}
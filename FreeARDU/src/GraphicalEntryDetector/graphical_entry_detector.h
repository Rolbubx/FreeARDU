#ifndef GRAPHICAL_ENTRY_DETECTOR_H
#define GRAPHICAL_ENTRY_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum ScreenType {
    SCREEN_NONE = 0,
    SCREEN_I2C_OLED,
    SCREEN_I2C_LCD,
    SCREEN_SPI_DISPLAY,
    SCREEN_PARALLEL_DISPLAY,
    SCREEN_UNKNOWN
};

#define MAX_SCREEN_PINS 16
#define MAX_PARALLEL_DATA_PINS 8

struct ScreenDetectionResult {
    enum ScreenType type;
    uint8_t i2cAddress;
    int detectedPin;

    uint8_t pinCount;
    int pins[MAX_SCREEN_PINS];

    int sdaPin;
    int sclPin;

    int mosiPin;
    int misoPin;
    int sckPin;
    int csPin;
    int dcPin;
    int resetPin;

    uint8_t parallelDataPinCount;
    int dataPins[MAX_PARALLEL_DATA_PINS];

    bool canDraw;
    
    int width;
    int height;
};

struct ScreenDetectionResult DETECT_SCRN();

#ifdef __cplusplus
}
#endif

#endif // GRAPHICAL_ENTRY_DETECTOR_H

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include "../GraphicalEntryDetector/graphical_entry_detector.h"

struct Vector2 {
    unsigned int x;
    unsigned int y;
};

struct Color {
    float r;
    float g;
    float b;
};

struct FramebufferPixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Framebuffer {
public:
    Framebuffer();
    int INIT();
    int WIDTH() const;
    int HEIGHT() const;
    int PUSH_PIXEL(Vector2 position, Color color, bool ProductionMode);
    int CLEAR(Color color);
    int FLUSH();
    int EMPTY_BUFFER(int x, int y, Color FRMBUFFER_CONTENT[], bool ProductionMode);

private:
    void detectFramebufferSize();
    void detectI2COLEDSize();
    void detectI2CLCDSize();
    void detectSPIDisplaySize();
    void detectParallelDisplaySize();

    uint8_t colorToByte(float value);

    int flushI2COLED();
    int flushI2CLCD();
    int flushSPIDisplay();
    int flushParallelDisplay();

    ScreenDetectionResult screenInfo;
    bool initialized;
    unsigned int width;
    unsigned int height;
    // We avoid dynamic allocation. For 1MB RAM, we can afford some static buffer if needed,
    // but for now we keep it as a wrapper to PUSH_PIXEL or similar.
};

#endif // FRAMEBUFFER_H

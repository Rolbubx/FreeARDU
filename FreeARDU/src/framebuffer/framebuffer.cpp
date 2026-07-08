#include "framebuffer.h"

#ifdef FREEARDU_BARE_METAL
#include <stdint.h>
#else
#include <Arduino.h>
#endif

#define MAX_FRAMEBUFFER_WIDTH 320
#define MAX_FRAMEBUFFER_HEIGHT 240

static FramebufferPixel pixels[MAX_FRAMEBUFFER_HEIGHT][MAX_FRAMEBUFFER_WIDTH];

Framebuffer::Framebuffer() : initialized(false), width(0), height(0) {
}

int Framebuffer::INIT() {
    screenInfo = DETECT_SCRN();

    if (!screenInfo.canDraw) {
        initialized = false;
        width = 0;
        height = 0;
        return -1;
    }

    detectFramebufferSize();

    if (width <= 0 || height <= 0) {
        initialized = false;
        return -2;
    }

    if (width > MAX_FRAMEBUFFER_WIDTH || height > MAX_FRAMEBUFFER_HEIGHT) {
        initialized = false;
        return -3;
    }

    initialized = true;
    Color black = {0.0f, 0.0f, 0.0f};
    CLEAR(black);

    return 0;
}

int Framebuffer::WIDTH() const {
    return width;
}

int Framebuffer::HEIGHT() const {
    return height;
}

int Framebuffer::PUSH_PIXEL(Vector2 position, Color color) {
    if (!initialized) {
        return -1;
    }

    int x = position.x;
    int y = position.y;

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return -2;
    }

    pixels[y][x] = {
        colorToByte(color.r),
        colorToByte(color.g),
        colorToByte(color.b)
    };

    return 0;
}

int Framebuffer::CLEAR(Color color) {
    if (width <= 0 || height <= 0) {
        return -1;
    }

    uint8_t r = colorToByte(color.r);
    uint8_t g = colorToByte(color.g);
    uint8_t b = colorToByte(color.b);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y][x] = { r, g, b };
        }
    }

    return 0;
}

int Framebuffer::FLUSH() {
    if (!initialized) {
        return -1;
    }

    switch (screenInfo.type) {
        case SCREEN_I2C_OLED:
            return flushI2COLED();

        case SCREEN_I2C_LCD:
            return flushI2CLCD();

        case SCREEN_SPI_DISPLAY:
            return flushSPIDisplay();

        case SCREEN_PARALLEL_DISPLAY:
            return flushParallelDisplay();

        default:
            return -2;
    }
}

void Framebuffer::detectFramebufferSize() {
    if (screenInfo.width > 0 && screenInfo.height > 0) {
        width = screenInfo.width;
        height = screenInfo.height;
        return;
    }

    switch (screenInfo.type) {
        case SCREEN_I2C_OLED:
            detectI2COLEDSize();
            break;
        case SCREEN_I2C_LCD:
            detectI2CLCDSize();
            break;
        case SCREEN_SPI_DISPLAY:
            detectSPIDisplaySize();
            break;
        case SCREEN_PARALLEL_DISPLAY:
            detectParallelDisplaySize();
            break;
        default:
            width = 0;
            height = 0;
            break;
    }
}

void Framebuffer::detectI2COLEDSize() {
    width = 128;
    height = 64;
}

void Framebuffer::detectI2CLCDSize() {
    width = 16;
    height = 2;
}

void Framebuffer::detectSPIDisplaySize() {
    width = 320;
    height = 240;
}

void Framebuffer::detectParallelDisplaySize() {
    width = 320;
    height = 240;
}

uint8_t Framebuffer::colorToByte(float value) {
    if (value <= 0.0f) return 0;
    if (value >= 1.0f) return 255;
    return (uint8_t)(value * 255.0f);
}

int Framebuffer::flushI2COLED() { return 0; }
int Framebuffer::flushI2CLCD() { return -3; }
int Framebuffer::flushSPIDisplay() { return 0; }
int Framebuffer::flushParallelDisplay() { return 0; }

extern "C" {
    void Wire_begin() {}
    void Wire_beginTransmission(uint8_t address) { (void)address; }
    uint8_t Wire_endTransmission() { return 1; } // No screen by default
    bool isPinConnected(int pin) { (void)pin; return false; }
}

Framebuffer framebuffer;

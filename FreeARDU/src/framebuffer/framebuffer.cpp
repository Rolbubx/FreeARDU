#include "Framebuffer.h"
#include "../Infos/Infos.h"
#include "../Hardware/GpioBareMetal.h"
#include <stdint.h>
#include <stddef.h>

#define MAX_FRAMEBUFFER_WIDTH 320
#define MAX_FRAMEBUFFER_HEIGHT 240

struct Pixel {
    uint8_t r, g, b;
};

static Pixel pixels[MAX_FRAMEBUFFER_HEIGHT][MAX_FRAMEBUFFER_WIDTH];

DisplayDriver::DisplayDriver() : initialized(false), width(0), height(0) {}

void DisplayDriver::drawPixelCallback(UG_S16 x, UG_S16 y, UG_COLOR c) {
    if (x >= 0 && y >= 0 && (unsigned int)x < display.getWidth() &&
        (unsigned int)y < display.getHeight()) {
        pixels[(unsigned int)y][(unsigned int)x] = {
            (uint8_t)((c >> 16) & 0xFF),
            (uint8_t)((c >> 8) & 0xFF),
            (uint8_t)(c & 0xFF)
        };
    }
}

int DisplayDriver::init() {
    screenInfo = DETECT_SCRN();

    if (!screenInfo.canDraw && screenInfo.type != SCREEN_NONE) {
        return -1;
    }

    detectSize();

    if (width <= 0 || height <= 0 || width > MAX_FRAMEBUFFER_WIDTH || height > MAX_FRAMEBUFFER_HEIGHT) {
        return -2;
    }

    if (screenInfo.type == SCREEN_SPI_DISPLAY) {
        gpio_init();
        gpio_configure_output(screenInfo.csPin, true);
        gpio_configure_output(screenInfo.dcPin, true);
        gpio_configure_output(screenInfo.resetPin, true);
        gpio_configure_output(screenInfo.sckPin, false);
        gpio_configure_output(screenInfo.mosiPin, false);
        gpio_write(screenInfo.resetPin, false);
        delay_cycles(200000);
        gpio_write(screenInfo.resetPin, true);
        delay_cycles(1200000);
        initIli9341();
    }

    UG_Init(&gui, drawPixelCallback, (UG_S16)width, (UG_S16)height);
    UG_FillScreen(C_BLACK);
    initialized = true;

    return 0;
}

int DisplayDriver::flush() {
    if (!initialized) return -1;
    if (screenInfo.type == SCREEN_SPI_DISPLAY) return flushSPI();
    if (screenInfo.type == SCREEN_PARALLEL_DISPLAY) return flushParallel();
    return 0;
}

void DisplayDriver::detectSize() {
    if (screenInfo.width > 0 && screenInfo.height > 0) {
        width = screenInfo.width;
        height = screenInfo.height;
    } else {
        width = 320;
        height = 240;
    }
}

int DisplayDriver::flushSPI() {
    writeCommand(0x2A);
    writeData(0x00); writeData(0x00);
    writeData((uint8_t)((width - 1) >> 8)); writeData((uint8_t)(width - 1));

    writeCommand(0x2B);
    writeData(0x00); writeData(0x00);
    writeData((uint8_t)((height - 1) >> 8)); writeData((uint8_t)(height - 1));

    gpio_write(screenInfo.csPin, false);
    gpio_write(screenInfo.dcPin, true);
    writeSpiByte(0x2C);

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            Pixel p = pixels[y][x];
            uint16_t color565 = ((p.r & 0xF8) << 8) | ((p.g & 0xFC) << 3) | (p.b >> 3);
            writeSpiByte((uint8_t)(color565 >> 8));
            writeSpiByte((uint8_t)color565);
        }
    }
    gpio_write(screenInfo.csPin, true);
    return 0;
}

int DisplayDriver::flushParallel() {
    return -1;
}

void DisplayDriver::delay_cycles(uint32_t cycles) {
    while (cycles-- != 0) {
        __asm__ volatile ("nop");
    }
}

void DisplayDriver::writeSpiByte(uint8_t value) {
    for (uint8_t bit = 0x80; bit != 0; bit >>= 1) {
        gpio_write(screenInfo.mosiPin, (value & bit) != 0);
        gpio_write(screenInfo.sckPin, true);
        gpio_write(screenInfo.sckPin, false);
    }
}

void DisplayDriver::writeCommand(uint8_t command) {
    gpio_write(screenInfo.csPin, false);
    gpio_write(screenInfo.dcPin, false);
    writeSpiByte(command);
    gpio_write(screenInfo.csPin, true);
}

void DisplayDriver::writeData(uint8_t data) {
    gpio_write(screenInfo.csPin, false);
    gpio_write(screenInfo.dcPin, true);
    writeSpiByte(data);
    gpio_write(screenInfo.csPin, true);
}

void DisplayDriver::initIli9341() {
    writeCommand(0x01);
    delay_cycles(1200000);
    writeCommand(0x11);
    delay_cycles(1200000);
    writeCommand(0x3A); writeData(0x55);
    writeCommand(0x36); writeData(0x48);
    writeCommand(0xB1); writeData(0x00); writeData(0x1B);
    writeCommand(0xC0); writeData(0x23);
    writeCommand(0xC1); writeData(0x10);
    writeCommand(0xC5); writeData(0x3E); writeData(0x28);
    writeCommand(0xC7); writeData(0x86);
    writeCommand(0xE0);
    const uint8_t positive[] = {0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00};
    for (uint8_t i = 0; i < sizeof(positive); i++) writeData(positive[i]);
    writeCommand(0xE1);
    const uint8_t negative[] = {0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F};
    for (uint8_t i = 0; i < sizeof(negative); i++) writeData(negative[i]);
    writeCommand(0x29);
    delay_cycles(1200000);
}

DisplayDriver display;

extern "C" int display_init(void) {
    return display.init();
}

extern "C" int display_flush(void) {
    return display.flush();
}

extern "C" int display_get_width(void) {
    return display.getWidth();
}

extern "C" int display_get_height(void) {
    return display.getHeight();
}

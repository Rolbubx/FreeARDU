#include "framebuffer.h"
#include "../infos/INFOS.h"
#include "../uart_putc/UART_PUTCHAR.h"
#include <stdint.h>
#include <stddef.h>

// i.MX RT1060 LPSPI4 Registers
#define LPSPI4_BASE 0x403AC000
#define LPSPI4_CR    (*(volatile uint32_t*)(LPSPI4_BASE + 0x10))
#define LPSPI4_SR    (*(volatile uint32_t*)(LPSPI4_BASE + 0x14))
#define LPSPI4_CFGR1 (*(volatile uint32_t*)(LPSPI4_BASE + 0x24))
#define LPSPI4_FCR   (*(volatile uint32_t*)(LPSPI4_BASE + 0x58))
#define LPSPI4_TDR   (*(volatile uint32_t*)(LPSPI4_BASE + 0x64))

// GPIO Registers
#define GPIO1_BASE 0x401B8000
#define GPIO1_DR   (*(volatile uint32_t*)(GPIO1_BASE + 0x00))
#define GPIO1_GDIR (*(volatile uint32_t*)(GPIO1_BASE + 0x04))

#define MAX_FRAMEBUFFER_WIDTH 320
#define MAX_FRAMEBUFFER_HEIGHT 240

struct Pixel {
    uint8_t r, g, b;
};

static Pixel pixels[MAX_FRAMEBUFFER_HEIGHT][MAX_FRAMEBUFFER_WIDTH];

DisplayDriver::DisplayDriver() : initialized(false), width(0), height(0) {}

void DisplayDriver::drawPixelCallback(UG_S16 x, UG_S16 y, UG_COLOR c) {
    if (x >= 0 && (unsigned int)x < display.getWidth() && y >= 0 && (unsigned int)y < display.getHeight()) {
        pixels[y][x] = {
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

    if (screenInfo.type == SCREEN_NONE) {
        width = 200;
        height = 100;
    } else {
        detectSize();
    }

    if (width <= 0 || height <= 0 || width > MAX_FRAMEBUFFER_WIDTH || height > MAX_FRAMEBUFFER_HEIGHT) {
        return -2;
    }

    // Initialize Hardware
    if (screenInfo.type == SCREEN_SPI_DISPLAY) {
        LPSPI4_CR = 1; // Reset
        while (LPSPI4_CR & 1);
        LPSPI4_CFGR1 = 1; // Master
        LPSPI4_FCR = 0;
        LPSPI4_CR = (1 << 3) | (1 << 0);
        
        GPIO1_GDIR |= (1 << LPSPI4_DC_PIN) | (1 << LPSPI4_RST_PIN);
        GPIO1_DR &= ~(1 << LPSPI4_RST_PIN);
        for (volatile int i = 0; i < 1000000; i++);
        GPIO1_DR |= (1 << LPSPI4_RST_PIN);
        for (volatile int i = 0; i < 1000000; i++);
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
    auto sendCmd = [](uint8_t cmd) {
        GPIO1_DR &= ~(1 << LPSPI4_DC_PIN);
        while (!(LPSPI4_SR & (1 << 0)));
        LPSPI4_TDR = cmd;
    };

    auto sendData = [](uint8_t data) {
        GPIO1_DR |= (1 << LPSPI4_DC_PIN);
        while (!(LPSPI4_SR & (1 << 0)));
        LPSPI4_TDR = data;
    };

    sendCmd(0x2A); // CASET
    sendData(0x00); sendData(0x00);
    sendData((uint8_t)((width - 1) >> 8)); sendData((uint8_t)((width - 1) & 0xFF));

    sendCmd(0x2B); // PASET
    sendData(0x00); sendData(0x00);
    sendData((uint8_t)((height - 1) >> 8)); sendData((uint8_t)((height - 1) & 0xFF));

    sendCmd(0x2C); // RAMWR
    GPIO1_DR |= (1 << LPSPI4_DC_PIN);

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            Pixel p = pixels[y][x];
            uint16_t color565 = ((p.r & 0xF8) << 8) | ((p.g & 0xFC) << 3) | (p.b >> 3);
            while (!(LPSPI4_SR & (1 << 0)));
            LPSPI4_TDR = (uint8_t)(color565 >> 8);
            while (!(LPSPI4_SR & (1 << 0)));
            LPSPI4_TDR = (uint8_t)(color565 & 0xFF);
        }
    }
    return 0;
}

int DisplayDriver::flushParallel() {
    return 0;
}

DisplayDriver display;

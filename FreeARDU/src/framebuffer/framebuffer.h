#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <stdint.h>
#include "../DisplayDetector/DisplayDetector.h"
#include "../ThirdParty/Ugui/ugui.h"

// Hardware display driver for FreeARDU
// Handles low-level display initialization and buffer flushing
class DisplayDriver {
public:
    DisplayDriver();
    
    // Initializes hardware and uGUI
    int init();
    
    // Flushes the pixel buffer to the actual hardware
    int flush();

    // Accessors
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    bool isInitialized() const { return initialized; }

    // uGUI pixel callback
    static void drawPixelCallback(UG_S16 x, UG_S16 y, UG_COLOR c);

private:
    void detectSize();
    int flushSPI();
    int flushParallel();
    void initIli9341();
    void writeCommand(uint8_t command);
    void writeData(uint8_t data);
    void writeSpiByte(uint8_t value);
    static void delay_cycles(uint32_t cycles);

    ScreenDetectionResult screenInfo;
    bool initialized;
    unsigned int width;
    unsigned int height;
    
    UG_GUI gui;
};

extern DisplayDriver display;

#endif // DISPLAY_DRIVER_H

#ifdef FREEARDU_BARE_METAL
#include "../../framebuffer/framebuffer.h"
#include "../../GraphicalEntryDetector/graphical_entry_detector.h"

extern Framebuffer framebuffer;

extern "C" int main() {
    // Initialize OS/Runtime components
    framebuffer.INIT();
    
    // Clear screen
    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    while (1) {
        // Main OS Loop
    }
    return 0;
}
#else
#include <Arduino.h>

void setup() {
}

void loop() {
}
#endif
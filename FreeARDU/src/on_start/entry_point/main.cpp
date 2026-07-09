#ifdef FREEARDU_BARE_METAL
#include "../../framebuffer/framebuffer.h"
#include "../../GraphicalEntryDetector/graphical_entry_detector.h"
#define CPU_FREQ 600000000
extern Framebuffer framebuffer;


class WaiterScheduler {
public:
    float transform_cycles_to_seconds(volatile unsigned int cycles) {
        return (float)cycles / CPU_FREQ;
    }

    void wait_cycles(volatile unsigned int count) {
        while (count > 0) {
            count--;
        }
    }
};

extern "C" int main() {
    // init the framebuffer
    framebuffer.INIT();
    
    // clear screen
    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    while (1) {
        // mains os loop


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
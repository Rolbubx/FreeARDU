#ifdef FREEARDU_BARE_METAL
#include "../../framebuffer/framebuffer.h"
#include "../../GraphicalEntryDetector/graphical_entry_detector.h"
#include "uart_putc/UART_PUTCHAR.h"

#define CPU_FREQ 600000000
#define IsMainOsDebugged true

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


void ON_LOAD_success() {
    uart_puts("FreeARDU successfully loaded, now awaiting instructions\r\n");

}

extern "C" int main() {
    uart_init();
    ON_LOAD_success();

    // init the framebuffer
    framebuffer.INIT();

    uart_puts("Framebuffer initiated \r\n");

    // clear screen
    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    uart_puts("Iniating waiter scheduler \r\n");

    WaiterScheduler s;


    uart_puts("Going to main os loop \r\n");
    while (1) {
        // mains os loop
        if (IsMainOsDebugged == true) {
            s.wait_cycles(100000000);
            uart_puts("Main OS loop \r\n");
        }


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
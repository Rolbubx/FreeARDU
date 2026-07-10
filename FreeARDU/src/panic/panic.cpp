#include "panic/panic.h"
#include "uart_putc/UART_PUTCHAR.h"

#define PANIC_AUTO_RESTART false // set to true to soft-reboot after a panic instead of halting

extern "C" void reset_handler(); // defined in startup.S

static void print_panic_banner() {
    uart_puts("\r\n");
    uart_puts("############################################\r\n");
    uart_puts("#            KERNEL PANIC                  #\r\n");
    uart_puts("############################################\r\n");
}

[[noreturn]] void kernel_panic(const char* reason) {
    print_panic_banner();
    uart_puts("Reason: ");
    uart_puts(reason);
    uart_puts("\r\n");
    uart_puts("System halted.\r\n");

    #if PANIC_AUTO_RESTART
        uart_puts("Auto-restart is enabled. Rebooting in 3 seconds...\r\n");
        for (volatile unsigned int i = 0; i < 200000000; i++) {
            // crude delay
        }
        reset_handler(); // soft reset: jumps back to the very start of boot
    #endif

    while (1) {
        // halt forever
    }
}

// ------------------------------------------------------------
// Cortex-M fault handlers.
// These override the weak defaults declared in startup.S,
// because a global C function with the exact same name takes
// priority over a .weak assembly symbol at link time.
// ------------------------------------------------------------
extern "C" void HardFault_Handler() {
    kernel_panic("Hard Fault (unhandled exception, invalid memory access, or bad instruction)");
}

extern "C" void MemManage_Handler() {
    kernel_panic("Memory Management Fault (invalid memory access, MPU violation)");
}

extern "C" void BusFault_Handler() {
    kernel_panic("Bus Fault (invalid bus access, e.g. unmapped memory)");
}

extern "C" void UsageFault_Handler() {
    kernel_panic("Usage Fault (invalid instruction or illegal state)");
}

void panic_init() {
    // No-op for now: handlers are installed automatically at link time
    // by overriding the weak symbols from startup.S.
    // This function exists as a hook point for future watchdog setup, etc.
}
#include "panic/panic.h"

#include <cstdint>

#include "uart_putc/UART_PUTCHAR.h"

#define PANIC_AUTO_RESTART false // set to true to soft-reboot after a panic instead of halting

extern "C" void reset_handler(); // defined in startup.S

extern "C" void hard_fault_handler_c(uint32_t* stack_frame) {
    uint32_t pc = stack_frame[6]; // The instruction address that caused the crash

    // Write the error code first
    uart_puts("\r\nERROR: S/0x01\r\n");
    
    // Red color for the address info
    uart_puts("\033[1;31m");
    uart_puts("FAULT AT ADDRESS: ");
    uart_print_hex(pc);
    uart_puts("\r\n");

    kernel_panic("S/0x01", "Hard Fault (unhandled exception or invalid memory access)");
}

extern "C" {
    extern unsigned int _sdata, _edata;
    extern unsigned int _sbss, _ebss;
    extern unsigned int _sheap, _eheap;
    extern unsigned int _estack;
}

static void dump_memory_panic() {
    uart_puts("\r\n--- Memory Dump ---\r\n");
    uart_puts(".data: "); uart_print_hex((unsigned int)&_sdata); uart_puts(" - "); uart_print_hex((unsigned int)&_edata); uart_puts("\r\n");
    uart_puts(".bss : "); uart_print_hex((unsigned int)&_sbss); uart_puts(" - "); uart_print_hex((unsigned int)&_ebss); uart_puts("\r\n");
    uart_puts("heap : "); uart_print_hex((unsigned int)&_sheap); uart_puts(" - "); uart_print_hex((unsigned int)&_eheap); uart_puts("\r\n");
    uart_puts("stack: top at "); uart_print_hex((unsigned int)&_estack); uart_puts("\r\n");
    uart_puts("-------------------\r\n");
}


static void print_panic_banner() {
    // Red color using ANSI escape codes: \033[1;31m
    uart_puts("\033[1;31m");
    uart_puts("\r\n");
    uart_puts("############################################\r\n");
    uart_puts("#            KERNEL PANIC                  #\r\n");
    uart_puts("############################################\r\n");
}

[[noreturn]] void kernel_panic(const char* errorCode, const char* reason) {
    // Write the error code first as requested: "before the kernel panic, it should just wirte an error (add error codes) like S (software)/code"
    uart_puts("\r\nERROR: ");
    uart_puts(errorCode);
    uart_puts("\r\n");

    print_panic_banner();
    uart_puts("Reason: ");
    uart_puts(reason);
    uart_puts("\r\n");
    
    dump_memory_panic();
    
    uart_puts("\r\nSystem halted.\r\n");
    uart_puts("\033[0m"); // Reset color

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


extern "C" void MemManage_Handler() {
    kernel_panic("S/0x02", "Memory Management Fault (invalid memory access, MPU violation)");
}

extern "C" void BusFault_Handler() {
    kernel_panic("S/0x03", "Bus Fault (invalid bus access, e.g. unmapped memory)");
}

extern "C" void UsageFault_Handler() {
    kernel_panic("S/0x04", "Usage Fault (invalid instruction or illegal state)");
}

void panic_init() {
    // No-op for now: handlers are installed automatically at link time
    // by overriding the weak symbols from startup.S.
    // This function exists as a hook point for future watchdog setup, etc.
}
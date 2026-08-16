#include "panic/panic.h"

#include <cstdint>

#include "uart_putc/UART_PUTCHAR.h"

// Enable auto-restart after panic (set to true to auto-reboot)
#define PANIC_AUTO_RESTART true

// Number of funny phrases for kernel panic
#define PANIC_PHRASE_COUNT 8

extern "C" void reset_handler(); // defined in startup.S

// Funny phrases to display during kernel panic
static const char* panic_phrases[PANIC_PHRASE_COUNT] = {
    "bro i think your board is in depression",
    "your microcontroller is having a mental breakdown",
    "the chip is too tired, needs a coffee",
    "404: board not found (in emotional support)",
    "your code made the processor cry",
    "this board has officially given up on life",
    "recycling old chips... just like your bugs",
    "the machine is judging your life choices"
};

// Simple pseudo-random number generator (based on address and error code)
static uint32_t simple_rand(uint32_t seed) {
    return ((seed * 1103515245 + 12345) & 0x7fffffff);
}

// Get a random phrase based on error code
static const char* get_random_panic_phrase(const char* errorCode) {
    // Use address of errorCode pointer and length as seed for randomness
    uint32_t seed = (uint32_t)(uintptr_t)errorCode;
    for (int i = 0; errorCode[i] != '\0'; i++) {
        seed += errorCode[i];
    }
    uint32_t index = simple_rand(seed) % PANIC_PHRASE_COUNT;
    return panic_phrases[index];
}

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
    uart_puts("#               KERNEL PANIC                #\r\n");
    uart_puts("############################################\r\n");
    uart_puts("\033[0m"); // Reset color
}

[[noreturn]] void kernel_panic(const char* errorCode, const char* reason) {
    // Write the error code first as requested
    uart_puts("\r\nERROR: ");
    uart_puts(errorCode);
    uart_puts("\r\n");

    // Print the funny random phrase before the banner
    uart_puts("\r\n\033[1;33m"); // Yellow color for the funny message
    uart_puts(get_random_panic_phrase(errorCode));
    uart_puts("\r\n\033[0m"); // Reset color

    print_panic_banner();
    
    // Print reason in cyan for visibility
    uart_puts("\033[1;36m"); // Cyan color
    uart_puts("Reason: ");
    uart_puts(reason);
    uart_puts("\r\n\033[0m"); // Reset color
    
    dump_memory_panic();
    
    uart_puts("\r\n\033[1;31m"); // Red color
    uart_puts("System halting... but fear not, auto-restarting in 2 seconds!\r\n");
    uart_puts("\033[0m"); // Reset color
    
    // Wait for a short delay before restart
    for (volatile unsigned int i = 0; i < 100000000; i++) {
        // crude delay
    }
    
    // Restart the system
    reset_handler();
    
    // Should never reach here, but keep the compiler happy
    while (1) {
        // halt forever (should never get here with auto-restart enabled)
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

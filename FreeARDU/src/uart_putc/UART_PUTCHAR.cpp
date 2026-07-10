#include "uart_putc/UART_PUTCHAR.h"

#define LPUART1_BASE 0x40184000
#define LPUART1_STAT (*(volatile unsigned int*)(LPUART1_BASE + 0x14))
#define LPUART1_CTRL (*(volatile unsigned int*)(LPUART1_BASE + 0x18))
#define LPUART1_DATA (*(volatile unsigned int*)(LPUART1_BASE + 0x1C))

void uart_init() {
    LPUART1_CTRL |= (1 << 19); // TE (Transmit Enable) = bit 19
}

void uart_putc(char c) {
    while (!(LPUART1_STAT & (1 << 22))) {
        // wait until TDRE ready
    }
    LPUART1_DATA = c;
}

void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str);
        str++;
    }
}
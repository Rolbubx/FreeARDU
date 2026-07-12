#include "uart_putc/UART_PUTCHAR.h"

#define LPUART1_BASE 0x40184000
#define LPUART1_STAT (*(volatile unsigned int*)(LPUART1_BASE + 0x14))
#define LPUART1_CTRL (*(volatile unsigned int*)(LPUART1_BASE + 0x18))
#define LPUART1_DATA (*(volatile unsigned int*)(LPUART1_BASE + 0x1C))

void uart_init() {
    LPUART1_CTRL |= (1 << 19); // TE (Transmit Enable)
    LPUART1_CTRL |= (1 << 18); // RE (Receive Enable)
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

void uart_print_uint(unsigned int value) {
    char buffer[11];
    int i = 10;
    buffer[i] = '\0';

    if (value == 0) {
        uart_putc('0');
        return;
    }

    while (value > 0 && i > 0) {
        i--;
        buffer[i] = '0' + (value % 10);
        value /= 10;
    }

    uart_puts(&buffer[i]);
}

void uart_print_hex(unsigned int value) {
    const char hexDigits[] = "0123456789ABCDEF";
    char buffer[9];
    buffer[8] = '\0';

    for (int i = 7; i >= 0; i--) {
        buffer[i] = hexDigits[value & 0xF];
        value >>= 4;
    }

    uart_puts("0x");
    uart_puts(buffer);
}

char uart_getc() {
    while (!(LPUART1_STAT & (1 << 21))) {
        // wait until RDRF (data available)
    }
    return (char)(LPUART1_DATA & 0xFF);
}

bool uart_data_available() {
    return (LPUART1_STAT & (1 << 21)) != 0;
}

// Reads a line (blocking) until \r or \n, echoes each character back.
// Returns the number of characters read (excluding the terminator).
int uart_read_line(char* buffer, int max_len) {
    int index = 0;

    while (index < max_len - 1) {
        char c = uart_getc();

        if (c == '\r' || c == '\n') {
            uart_puts("\r\n");
            break;
        }

        // basic backspace support
        if (c == 0x08 || c == 0x7F) {
            if (index > 0) {
                index--;
                uart_puts("\b \b"); // erase char visually
            }
            continue;
        }

        uart_putc(c); // echo
        buffer[index] = c;
        index++;
    }

    buffer[index] = '\0';
    return index;
}
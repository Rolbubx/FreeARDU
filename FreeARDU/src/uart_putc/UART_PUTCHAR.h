#ifndef FREEARDUREP_UART_PUTCHAR_H
#define FREEARDUREP_UART_PUTCHAR_H

void uart_init();
void uart_putc(char c);
void uart_puts(const char* str);

#endif //FREEARDUREP_UART_PUTCHAR_H
#ifndef FREEARDUREP_UART_PUTCHAR_H
#define FREEARDUREP_UART_PUTCHAR_H

void uart_init();
void uart_putc(char c);
void uart_puts(const char* str);
void uart_print_uint(unsigned int value);
void uart_print_hex(unsigned int value);

char uart_getc();
bool uart_data_available();
int  uart_read_line(char* buffer, int max_len); // blocking, returns length read

#endif //FREEARDUREP_UART_PUTCHAR_H
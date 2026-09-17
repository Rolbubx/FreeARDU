#ifndef FREEARDU_STRING_H
#define FREEARDU_STRING_H

#ifdef __cplusplus
#include <type_traits>
#include "../Uart/UartPutchar.h"

namespace freeardu {

static inline void uart_print_ull(unsigned long long value) {
    char buffer[32];
    int index = sizeof(buffer) - 1;
    buffer[index] = '\0';

    if (value == 0ULL) {
        uart_putc('0');
        return;
    }

    while (value > 0ULL && index > 0) {
        --index;
        buffer[index] = static_cast<char>('0' + (value % 10ULL));
        value /= 10ULL;
    }

    uart_puts(&buffer[index]);
}

static inline void print_arg(const char* value) {
    if (value != nullptr) {
        uart_puts(value);
    }
}

static inline void print_arg(char* value) {
    if (value != nullptr) {
        uart_puts(value);
    }
}

static inline void print_arg(char value) {
    uart_putc(value);
}

static inline void print_arg(bool value) {
    uart_puts(value ? "true" : "false");
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, void>::type
print_arg(T value) {
    if (value < 0) {
        uart_putc('-');
        uart_print_ull((unsigned long long)(-(value + 1)) + 1ULL);
    } else {
        uart_print_ull((unsigned long long)value);
    }
}

template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, void>::type
print_arg(T value) {
    uart_print_ull((unsigned long long)value);
}

template <typename... Args>
static inline void print_impl(Args&&... args) {
    int dummy[] = { 0, (print_arg(args), 0)... };
    (void)dummy;
}

} // namespace freeardu

#define print(...) do { ::freeardu::print_impl(__VA_ARGS__); } while (0)
#else
#include <stdio.h>
#define print(...) do { printf(__VA_ARGS__); } while (0)
#endif

#define string (const char*)(auto)
#define to_string(constchar, container) (string)container

#endif // FREEARDU_STRING_H

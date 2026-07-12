#include "api.h"
#include "../uart_putc/UART_PUTCHAR.h"

// Hardware specific addresses for i.MX RT1060 (Example)
// GPIO1 base: 0x401B8000
// ADC1 base: 0x400C4000
// WDOG1 base: 0x400B8000

// Mock implementation for bare metal demo
static void _pinMode(int pin, PinMode mode) {
    uart_puts("API: Pin ");
    uart_print_uint(pin);
    uart_puts(" set to mode ");
    uart_print_uint((unsigned int)mode);
    uart_puts("\r\n");
}

static void _digitalWrite(int pin, PinStatus status) {
    uart_puts("API: Pin ");
    uart_print_uint(pin);
    uart_puts(" set to ");
    uart_puts(status == PIN_HIGH ? "HIGH" : "LOW");
    uart_puts("\r\n");
}

static PinStatus _digitalRead(int pin) {
    (void)pin;
    return PIN_LOW;
}

static void _togglePin(int pin) {
    uart_puts("API: Toggling pin ");
    uart_print_uint(pin);
    uart_puts("\r\n");
}

const PinAPI PIN_API = {
    .pinMode = _pinMode,
    .digitalWrite = _digitalWrite,
    .digitalRead = _digitalRead,
    .togglePin = _togglePin
};

// ADC Mock
static void _adcInit(int channel) {
    uart_puts("API: ADC Channel ");
    uart_print_uint(channel);
    uart_puts(" initialized\r\n");
}

static uint32_t _adcRead(int channel) {
    (void)channel;
    return 512; // Return dummy value
}

const AdcAPI ADC_API = {
    .init = _adcInit,
    .read = _adcRead
};

// Watchdog Mock
static void _wdogInit(uint32_t timeout_ms) {
    uart_puts("API: Watchdog initialized with ");
    uart_print_uint(timeout_ms);
    uart_puts(" ms timeout\r\n");
}

static void _wdogFeed() {
    // uart_puts("API: Watchdog fed\r\n");
}

static void _wdogStop() {
    uart_puts("API: Watchdog stopped\r\n");
}

const WatchdogAPI WATCHDOG_API = {
    .init = _wdogInit,
    .feed = _wdogFeed,
    .stop = _wdogStop
};

// Global API object
const FreeArduAPI API = {
    .pin = &PIN_API,
    .adc = &ADC_API,
    .watchdog = &WATCHDOG_API
};

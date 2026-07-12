#ifndef PIN_API_H
#define PIN_API_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PIN_MODE_INPUT = 0,
    PIN_MODE_OUTPUT,
    PIN_MODE_INPUT_PULLUP,
    PIN_MODE_INPUT_PULLDOWN,
    PIN_MODE_ANALOG
} PinMode;

typedef enum {
    PIN_LOW = 0,
    PIN_HIGH = 1
} PinStatus;

typedef struct {
    void (*pinMode)(int pin, PinMode mode);
    void (*digitalWrite)(int pin, PinStatus status);
    PinStatus (*digitalRead)(int pin);
    void (*togglePin)(int pin);
} PinAPI;

extern const PinAPI PIN_API;

// ADC API
typedef struct {
    void (*init)(int channel);
    uint32_t (*read)(int channel);
} AdcAPI;

extern const AdcAPI ADC_API;

// Watchdog API
typedef struct {
    void (*init)(uint32_t timeout_ms);
    void (*feed)();
    void (*stop)();
} WatchdogAPI;

extern const WatchdogAPI WATCHDOG_API;

// Unified API structure
typedef struct {
    const PinAPI* pin;
    const AdcAPI* adc;
    const WatchdogAPI* watchdog;
} FreeArduAPI;

extern const FreeArduAPI API;

#ifdef __cplusplus
}
#endif

#endif // PIN_API_H

#ifndef GPIO_BARE_METAL_H
#define GPIO_BARE_METAL_H

#ifdef FREEARDU_BARE_METAL

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize GPIO system for pin detection
void gpio_init(void);

// Check if a pin is connected (has external pull-up or connected signal)
// Uses internal pull-down resistor - if pin reads high, something is pulling it up
bool isPinConnected(int pin);

// Helper: detect common display types via I2C address probing
// Returns true if an I2C device responds at the given address
bool i2c_probe_address(uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif // FREEARDU_BARE_METAL

#endif // GPIO_BARE_METAL_h
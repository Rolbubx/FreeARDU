#ifndef BIOS_H
#define BIOS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// BIOS configuration structure
typedef struct {
    uint32_t cpu_mhz;
    uint32_t watchdog_timeout_ms;
    bool auto_restart_enabled;
    bool debug_commands_enabled;
    uint8_t verbosity_level;
    char bios_version[16];
} BIOSConfig;

// BIOS functions
void bios_init();
void bios_load();  // Load BIOS from UART (simulated - loads "first" as requested)
void bios_set_param(const char* param, const char* value);
const char* bios_get_param(const char* param);
void bios_print_config();
void bios_print_banner();

// Get BIOS config
const BIOSConfig* bios_get_config();
void bios_set_config(const BIOSConfig* config);

#ifdef __cplusplus
}
#endif

#endif // BIOS_H

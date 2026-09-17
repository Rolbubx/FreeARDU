#include "Bios.h"
#include "../Uart/UartPutchar.h"
#include <string.h>
#include <stdlib.h>

// Default BIOS configuration
static BIOSConfig config = {
    .cpu_mhz = 600000000,
    .watchdog_timeout_ms = 5000,
    .auto_restart_enabled = false,
    .debug_commands_enabled = true,
    .verbosity_level = 2,
    .bios_version = "1.0.0"
};

// Simulated BIOS storage (acts as if loaded from UART first)
static char bios_param_buffer[256];
static bool bios_loaded = false;

void bios_init() {
    // Initialize with default values, marking BIOS as "loaded"
    bios_loaded = true;
    config.cpu_mhz = 600000000;  // Default 600MHz
    config.watchdog_timeout_ms = 5000;
    config.auto_restart_enabled = false;
    config.debug_commands_enabled = true;
    config.verbosity_level = 2;
    strncpy(config.bios_version, "1.0.0", sizeof(config.bios_version) - 1);
    config.bios_version[sizeof(config.bios_version) - 1] = '\0';
}

void bios_load() {
    uart_puts("Loading BIOS simulation from UART...\r\n");
    
    // Simulated BIOS loading from UART port
    // In a real system, this would read binary data from UART
    // For simulation, we load default configuration
    
    uart_puts("BIOS Version: ");
    uart_puts(config.bios_version);
    uart_puts("\r\n");
    uart_puts("BIOS Load: SUCCESS\r\n");
    uart_puts("BIOS Parameters applied from UART simulation.\r\n");
    
    bios_loaded = true;
}

void bios_set_param(const char* param, const char* value) {
    if (!param) return;
    
    if (strcmp(param, "cpu_mhz") == 0) {
        config.cpu_mhz = atoi(value);
        uart_puts("CPU MHz set to: ");
        uart_print_uint(config.cpu_mhz);
        uart_puts("\r\n");
    }
    else if (strcmp(param, "watchdog_timeout") == 0) {
        config.watchdog_timeout_ms = atoi(value);
        uart_puts("Watchdog timeout set to: ");
        uart_print_uint(config.watchdog_timeout_ms);
        uart_puts(" ms\r\n");
    }
    else if (strcmp(param, "auto_restart") == 0) {
        config.auto_restart_enabled = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        uart_puts("Auto restart: ");
        uart_puts(config.auto_restart_enabled ? "ENABLED" : "DISABLED");
        uart_puts("\r\n");
    }
    else if (strcmp(param, "debug_commands") == 0) {
        config.debug_commands_enabled = (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
        uart_puts("Debug commands: ");
        uart_puts(config.debug_commands_enabled ? "ENABLED" : "DISABLED");
        uart_puts("\r\n");
    }
    else if (strcmp(param, "verbosity") == 0) {
        config.verbosity_level = atoi(value);
        uart_puts("Verbosity level set to: ");
        uart_print_uint(config.verbosity_level);
        uart_puts("\r\n");
    }
    else if (strcmp(param, "version") == 0) {
        strncpy(config.bios_version, value, sizeof(config.bios_version) - 1);
        config.bios_version[sizeof(config.bios_version) - 1] = '\0';
        uart_puts("BIOS version set to: ");
        uart_puts(config.bios_version);
        uart_puts("\r\n");
    }
    else {
        uart_puts("Unknown parameter: ");
        uart_puts(param);
        uart_puts("\r\n");
        uart_puts("Available parameters: cpu_mhz, watchdog_timeout, auto_restart, debug_commands, verbosity, version\r\n");
    }
}

const char* bios_get_param(const char* param) {
    static char result[32];
    
    if (strcmp(param, "cpu_mhz") == 0) {
        itoa(config.cpu_mhz, result, 10);
        return result;
    }
    else if (strcmp(param, "watchdog_timeout") == 0) {
        itoa(config.watchdog_timeout_ms, result, 10);
        return result;
    }
    else if (strcmp(param, "auto_restart") == 0) {
        return config.auto_restart_enabled ? "true" : "false";
    }
    else if (strcmp(param, "debug_commands") == 0) {
        return config.debug_commands_enabled ? "true" : "false";
    }
    else if (strcmp(param, "verbosity") == 0) {
        itoa(config.verbosity_level, result, 10);
        return result;
    }
    else if (strcmp(param, "version") == 0) {
        return config.bios_version;
    }
    
    return "unknown";
}

void bios_print_config() {
    uart_puts("=== BIOS Configuration ===\r\n");
    uart_puts("Version: ");
    uart_puts(config.bios_version);
    uart_puts("\r\n");
    uart_puts("CPU MHz: ");
    uart_print_uint(config.cpu_mhz);
    uart_puts("\r\n");
    uart_puts("Watchdog Timeout: ");
    uart_print_uint(config.watchdog_timeout_ms);
    uart_puts(" ms\r\n");
    uart_puts("Auto Restart: ");
    uart_puts(config.auto_restart_enabled ? "YES" : "NO");
    uart_puts("\r\n");
    uart_puts("Debug Commands: ");
    uart_puts(config.debug_commands_enabled ? "YES" : "NO");
    uart_puts("\r\n");
    uart_puts("Verbosity: ");
    uart_print_uint(config.verbosity_level);
    uart_puts("\r\n");
    uart_puts("==========================\r\n");
}

void bios_print_banner() {
    uart_puts("\r\n");
    uart_puts("    ___      __  ________\r\n");
    uart_puts("   /   |    /  |/  _______\r\n");
    uart_puts("  / /| |   / /|  /  __/   \r\n");
    uart_puts(" / ___ |  / ___ |_/  /__    \r\n");
    uart_puts("/_/  |_| /_/  |_/______/    \r\n");
    uart_puts("   BIOS v");
    uart_puts(config.bios_version);
    uart_puts(" Simulation -- First to load program\r\n");
    uart_puts("    Loading from UART...\r\n");
}

const BIOSConfig* bios_get_config() {
    return &config;
}

void bios_set_config(const BIOSConfig* new_config) {
    if (new_config) {
        config = *new_config;
    }
}

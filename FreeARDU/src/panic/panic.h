#ifndef FREEARDUREP_PANIC_H
#define FREEARDUREP_PANIC_H

// Triggers a kernel panic: logs the reason and halts (or resets, depending on config).
[[noreturn]] void kernel_panic(const char* errorCode, const char* reason);

// Installs fault handlers (called once at boot, though override via linker is automatic).
void panic_init();

#endif //FREEARDUREP_PANIC_H
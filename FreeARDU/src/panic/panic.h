#ifndef FREEARDUREP_PANIC_H
#define FREEARDUREP_PANIC_H

#ifdef __cplusplus
extern "C" {
#endif

// Reports a fatal condition and never returns.
#ifdef __cplusplus
[[noreturn]]
#else
__attribute__((noreturn))
#endif
void kernel_panic(const char* errorCode, const char* reason);

// Stops the kernel after notifying the stopping hook.
#ifdef __cplusplus
[[noreturn]]
#else
__attribute__((noreturn))
#endif
void kernel_stop();

// Enables the Cortex-M configurable fault handlers.
void panic_init();

#ifdef __cplusplus
}
#endif

#endif //FREEARDUREP_PANIC_H
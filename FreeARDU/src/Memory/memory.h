#ifndef FREEARDU_MEMORY_H
#define FREEARDU_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#include "../Infos/Infos.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Callback)(void);
typedef void (*KernelStoppingHook)(void);

void kernel_register_stopping_hook(KernelStoppingHook hook);
void kernel_mark_stopping(void);

void *memalloc(size_t size);
void memfree(void *ptr, size_t size);
/*
 * Copies size bytes and supports overlapping source/destination buffers.
 * The caller must ensure both buffers contain at least size bytes.
 */
void memcopy(size_t size, void *dst, const void *src);

/*
 * Runs callback with a private 64 KiB MPU-protected stack/data region.
 * The callback must not access globals, peripherals, or memory outside the
 * sandbox. Returns 0 when the callback returns and -1 for invalid input.
 * MPU violations are handled by the configured fault handler.
 */
int safexecution(Callback callback);

/* Called by fault handlers to abort a faulting sandbox callback. */
int sandbox_fault_recover(uint32_t *stack_frame);

#ifdef __cplusplus
}
#endif

#endif

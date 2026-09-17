#include "memory.h"

#include <stdint.h>

#define SANDBOX_SIZE       (64u * 1024u)
#define SANDBOX_ALIGNMENT  SANDBOX_SIZE
#define SANDBOX_STACK_SIZE (16u * 1024u)

#define NULL_PTR (void*)0

/*
 * A 64 KiB aligned object is required because ARMv7-M MPU regions are
 * naturally aligned and the region size is encoded as log2(size) - 1.
 */
static uint8_t sandbox_storage[SANDBOX_SIZE]
    __attribute__((aligned(SANDBOX_ALIGNMENT), section(".sandbox")));

static uint8_t sandbox_stack[SANDBOX_STACK_SIZE]
    __attribute__((aligned(SANDBOX_STACK_SIZE), section(".sandbox_stack")));

#define SANDBOX_METADATA_SIZE (sizeof(uintptr_t))

static volatile uint32_t sandbox_active;
volatile bool isKernelStopping = false;
static KernelStoppingHook stopping_hook;

void kernel_register_stopping_hook(KernelStoppingHook hook)
{
    stopping_hook = hook;
}

void kernel_mark_stopping(void)
{
    if (isKernelStopping) {
        return;
    }

    isKernelStopping = true;
    if (stopping_hook != NULL) {
        stopping_hook();
    }
}

extern void sandbox_fault_resume(void);

static uintptr_t *sandbox_heap_cursor(void)
{
    return (uintptr_t *)(void *)sandbox_storage;
}

static uintptr_t align_up(uintptr_t value, uintptr_t alignment)
{
    return (value + alignment - 1u) & ~(alignment - 1u);
}

void *memalloc(size_t size)
{
    uintptr_t *cursor_slot;
    uintptr_t heap_start;
    uintptr_t heap_end;
    uintptr_t heap_ptr;
    uintptr_t start;
    uintptr_t end;

    if (size == 0u) {
        return NULL;
    }

    cursor_slot = sandbox_heap_cursor();
    heap_start = align_up((uintptr_t)sandbox_storage + SANDBOX_METADATA_SIZE, 8u);
    heap_end = (uintptr_t)sandbox_storage + sizeof(sandbox_storage);
    heap_ptr = *cursor_slot;

    if (heap_ptr < heap_start || heap_ptr > heap_end) {
        heap_ptr = heap_start;
    }

    start = align_up(heap_ptr, 8u);
    end = start + size;
    if (end < start || end > heap_end) {
        return NULL;
    }

    *cursor_slot = end;
    return (void *)start;
}

void memfree(void *ptr, size_t size)
{
    uintptr_t *cursor_slot;
    uintptr_t heap_start;
    uintptr_t heap_end;
    uintptr_t address;
    uintptr_t end;

    if (ptr == NULL || size == 0u) {
        return;
    }

    cursor_slot = sandbox_heap_cursor();
    heap_start = align_up((uintptr_t)sandbox_storage + SANDBOX_METADATA_SIZE, 8u);
    heap_end = (uintptr_t)sandbox_storage + sizeof(sandbox_storage);
    address = (uintptr_t)ptr;
    end = address + size;
    if (address < heap_start || end < address || end > heap_end) {
        return;
    }

    if (end == *cursor_slot) {
        *cursor_slot = address;
    }
}

static void mpu_enable(void)
{
    volatile uint32_t *control = (volatile uint32_t *)0xE000ED94u;
    *control = 1u; /* enable MPU with no background/default address map */
    __asm__ volatile ("dsb; isb" ::: "memory");
}

static void mpu_configure_sandbox(void)
{
    volatile uint32_t *rbar = (volatile uint32_t *)0xE000ED9Cu;
    volatile uint32_t *rasr = (volatile uint32_t *)0xE000EDA0u;
    uintptr_t data_base = (uintptr_t)sandbox_storage;
    uintptr_t stack_base = (uintptr_t)sandbox_stack;

    /* Region 0: sandbox RAM, 64 KiB, read/write, execute-never. */
    *rbar = (uint32_t)data_base | 0u;
    *rasr = (15u << 1) | (3u << 24) | (1u << 28) | 1u;

    /* Region 1: sandbox stack, 16 KiB, read/write, execute-never. */
    *rbar = (uint32_t)stack_base | 1u;
    *rasr = (13u << 1) | (3u << 24) | (1u << 28) | 1u;

    /*
     * Region 2: firmware flash, 8 MiB, read-only and executable. The
     * callback can execute its own code and read constants, but cannot use
     * normal RAM, peripherals, or memory-mapped I/O.
     */
    *rbar = 0x60000000u | 2u;
    *rasr = (22u << 1) | (5u << 24) | 1u;
    __asm__ volatile ("dsb; isb" ::: "memory");
}

/*
 * Enter the callback in unprivileged Thread mode using PSP. The callback is
 * allowed to return normally; fault handlers handle MPU violations.
 */
__attribute__((naked, noinline)) static void switch_stack_and_call(
    Callback callback, uintptr_t stack_top)
{
    __asm__ volatile (
        "push {r4-r7, lr}\n"
        "sub sp, sp, #4\n"
        "mov r4, r0\n"

        "msr psp, r1\n"
        "isb\n"

        "mrs r2, control\n"
        "orr r2, r2, #3\n" /* use PSP and unprivileged Thread mode */
        "msr control, r2\n"
        "isb\n"

        "blx r4\n"

        ".global sandbox_fault_resume\n"
        "sandbox_fault_resume:\n"
        "ldr r2, =sandbox_active\n"
        "movs r3, #0\n"
        "str r3, [r2, #0]\n"
        "movs r2, #0\n" /* return to privileged Thread mode using MSP */
        "msr control, r2\n"
        "isb\n"

        "ldr r2, =0xE000ED94\n"
        "movs r3, #0\n"
        "str r3, [r2, #0]\n"
        "dsb\n"
        "isb\n"

        "add sp, sp, #4\n"
        "pop {r4-r7, pc}\n"
        ::: "memory");
}

int safexecution(Callback callback)
{
    uintptr_t stack_top;

    if (callback == NULL) {
        return -1;
    }

    /* NOLOAD sandbox memory is not cleared by startup code. */
    *sandbox_heap_cursor() =
        align_up((uintptr_t)sandbox_storage + SANDBOX_METADATA_SIZE, 8u);
    sandbox_active = 1u;
    stack_top = ((uintptr_t)sandbox_stack + SANDBOX_STACK_SIZE) &
                ~((uintptr_t)7u);
    mpu_configure_sandbox();
    mpu_enable();
    switch_stack_and_call(callback, stack_top);

    return 0;
}

int sandbox_fault_recover(uint32_t *stack_frame)
{
    volatile uint32_t *mpu_control = (volatile uint32_t *)0xE000ED94u;
    uint32_t control;

    if (stack_frame == NULL || sandbox_active == 0u) {
        return 0;
    }

    sandbox_active = 0u;
    *mpu_control = 0u;
    __asm__ volatile ("dsb; isb" ::: "memory");

    /* Return to privileged Thread mode before executing cleanup code. */
    control = 0u;
    __asm__ volatile ("msr control, %0\nisb" : : "r"(control) : "memory");

    stack_frame[6] = ((uint32_t)(uintptr_t)&sandbox_fault_resume) | 1u;
    return 1;
}

void memcopy(size_t size, void *dst, const void *src)
{
    uintptr_t destination;
    uintptr_t source;
    size_t index;

    if (size == 0u || dst == NULL || src == NULL) {
        return;
    }

    destination = (uintptr_t)dst;
    source = (uintptr_t)src;

    if (destination > source && destination - source < size) {
        for (index = size; index != 0u; --index) {
            ((uint8_t *)dst)[index - 1u] =
                ((const uint8_t *)src)[index - 1u];
        }
        return;
    }

    for (index = 0u; index < size; ++index) {
        ((uint8_t *)dst)[index] = ((const uint8_t *)src)[index];
    }
}


auto smart_ptr_list = {

}; // smart_ptr_list

void cleanup(void *ptr) {
    for (int i = 0; i < smart_ptr_list.size(); i++) {
        if (smart_ptr_list[i] == ptr) {
            memfree(ptr, sizeof(ptr));
        }
    }
    smart_ptr_list.remove(ptr);
}
void smart_ptr(void *ptr) {
    if (ptr == NULL || NULL_PTR == ptr) {
        return;
    }

    // detects when the kernel is stopped
    smart_ptr_list.push_back(ptr);

}

kernel_register_stopping_hook(cleanup);
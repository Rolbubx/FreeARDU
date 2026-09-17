#include <stdint.h>
#include <stdbool.h>

#define CPU_FREQ 600000000u
#define THREAD_MAX_COUNT 8u
#define THREAD_STACK_SIZE 256u

#define DWT_CTRL   (*(volatile uint32_t *)0xE0001000u)
#define DWT_CYCCNT (*(volatile uint32_t *)0xE0001004u)
#define DEMCR      (*(volatile uint32_t *)0xE000EDFCu)

#define SYSTICK_BASE 0xE000E010u
#define SYSTICK_CTRL (*(volatile uint32_t *)(SYSTICK_BASE + 0x00u))
#define SYSTICK_LOAD (*(volatile uint32_t *)(SYSTICK_BASE + 0x04u))
#define SYSTICK_VAL  (*(volatile uint32_t *)(SYSTICK_BASE + 0x08u))

#define SYSTICK_ENABLE  ((uint32_t)1u << 0u)
#define SYSTICK_TICKINT ((uint32_t)1u << 1u)
#define SYSTICK_CLKSRC  ((uint32_t)1u << 2u)

typedef void (*ThreadEntry)(void);

typedef struct {
    const char *name;
    ThreadEntry entry;
    uint32_t stack[THREAD_STACK_SIZE];
    uint32_t *sp;
    bool started;
    bool runnable;
} Thread;

static Thread g_threads[THREAD_MAX_COUNT];
static unsigned int g_thread_count = 0u;
static unsigned int g_current_thread = 0u;
static volatile bool g_scheduler_running = false;
static volatile uint32_t g_tick_count = 0u;

static inline uint32_t ms_to_cycles(uint32_t milliseconds) {
    return (CPU_FREQ / 1000u) * milliseconds;
}

static inline void dwt_start_cycles(void) {
    DEMCR |= (1u << 24u);
    DWT_CYCCNT = 0u;
    DWT_CTRL |= 1u;
}

void wait_ms(uint32_t milliseconds) {
    uint32_t start;
    uint32_t target;

    if (milliseconds == 0u) {
        return;
    }

    dwt_start_cycles();
    start = DWT_CYCCNT;
    target = ms_to_cycles(milliseconds);

    while ((DWT_CYCCNT - start) < target) {
        /* busy wait */
    }
}

void wait(unsigned int milliseconds) {
    wait_ms(milliseconds);
}

static void thread_init_stack(Thread *thread, ThreadEntry entry) {
    uint32_t *stack_top;

    if (thread == (Thread *)0 || entry == (ThreadEntry)0) {
        return;
    }

    stack_top = &thread->stack[THREAD_STACK_SIZE - 1u];
    stack_top = (uint32_t *)((uintptr_t)stack_top & ~((uintptr_t)7u));

    *(--stack_top) = 0x01000000u; /* xPSR, Thumb bit */
    *(--stack_top) = (uint32_t)entry; /* PC */
    *(--stack_top) = 0u; /* LR */
    *(--stack_top) = 0u; /* R12 */
    *(--stack_top) = 0u; /* R3 */
    *(--stack_top) = 0u; /* R2 */
    *(--stack_top) = 0u; /* R1 */
    *(--stack_top) = 0u; /* R0 */

    thread->sp = stack_top;
    thread->entry = entry;
    thread->started = false;
    thread->runnable = true;
}

int thread_create(const char *name, ThreadEntry entry) {
    Thread *thread;

    if (name == (const char *)0 || entry == (ThreadEntry)0 || g_thread_count >= THREAD_MAX_COUNT) {
        return -1;
    }

    thread = &g_threads[g_thread_count++];
    thread->name = name;
    thread->entry = entry;
    thread->stack[0] = 0u;
    thread->sp = (uint32_t *)0;
    thread->started = false;
    thread->runnable = true;
    thread_init_stack(thread, entry);
    return (int)(g_thread_count - 1u);
}

static void scheduler_run_next(void) {
    Thread *thread;
    unsigned int index;

    if (g_thread_count == 0u) {
        return;
    }

    index = g_current_thread;
    for (unsigned int i = 0u; i < g_thread_count; i++) {
        index = (index + 1u) % g_thread_count;
        thread = &g_threads[index];
        if (thread->runnable && thread->entry != (ThreadEntry)0) {
            g_current_thread = index;
            thread->started = true;
            thread->entry();
            return;
        }
    }
}

void thread_start(Thread *thread) {
    if (thread == (Thread *)0 || thread->entry == (ThreadEntry)0) {
        return;
    }

    g_current_thread = 0u;
    g_scheduler_running = true;
    thread->started = true;
    thread->entry();
}

void thread_run_all(void) {
    if (g_thread_count == 0u) {
        return;
    }

    g_current_thread = 0u;
    g_scheduler_running = true;
    g_threads[g_current_thread].entry();
}

void SysTick_Handler(void) {
    g_tick_count++;

    if (g_scheduler_running && g_thread_count > 0u) {
        scheduler_run_next();
    }
}

void systick_init(uint32_t reload_value) {
    SYSTICK_VAL = 0u;
    SYSTICK_LOAD = reload_value;
    SYSTICK_CTRL = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSRC;
}

void systick_init_ms(uint32_t milliseconds_period) {
    uint32_t reload = ((CPU_FREQ / 1000u) * milliseconds_period) - 1u;
    systick_init(reload);
}

#include "Panic.h"

#include <cstdint>

#include "../Memory/memory.h"
#include "../Uart/UartPutchar.h"

#ifndef PANIC_AUTO_RESTART
#define PANIC_AUTO_RESTART false
#endif

namespace {

constexpr uint32_t SCB_SHCSR = 0xE000ED24u;
constexpr uint32_t SCB_CFSR  = 0xE000ED28u;
constexpr uint32_t SCB_HFSR  = 0xE000ED2Cu;
constexpr uint32_t SCB_DFSR  = 0xE000ED30u;
constexpr uint32_t SCB_MMFAR = 0xE000ED34u;
constexpr uint32_t SCB_BFAR  = 0xE000ED38u;
constexpr uint32_t SCB_AFSR  = 0xE000ED3Cu;
constexpr uint32_t SCB_AIRCR = 0xE000ED0Cu;

constexpr uint32_t SHCSR_MEMFAULTENA = 1u << 16;
constexpr uint32_t SHCSR_BUSFAULTENA = 1u << 17;
constexpr uint32_t SHCSR_USGFAULTENA = 1u << 18;
constexpr uint32_t AIRCR_VECTKEY     = 0x5FAu << 16;
constexpr uint32_t AIRCR_SYSRESETREQ = 1u << 2;

volatile uint32_t* reg32(uint32_t address)
{
    return reinterpret_cast<volatile uint32_t*>(address);
}

volatile bool panic_active = false;

void print_label_hex(const char* label, uint32_t value)
{
    uart_puts(label);
    uart_print_hex(value);
    uart_puts("\r\n");
}

void print_fault_frame(const uint32_t* frame)
{
    if (frame == nullptr) {
        uart_puts("Stacked frame: unavailable\r\n");
        return;
    }

    uart_puts("Stacked registers:\r\n");
    print_label_hex("  R0  = ", frame[0]);
    print_label_hex("  R1  = ", frame[1]);
    print_label_hex("  R2  = ", frame[2]);
    print_label_hex("  R3  = ", frame[3]);
    print_label_hex("  R12 = ", frame[4]);
    print_label_hex("  LR  = ", frame[5]);
    print_label_hex("  PC  = ", frame[6]);
    print_label_hex("  xPSR= ", frame[7]);
}

void disable_mpu_for_fault()
{
    *reg32(0xE000ED94u) = 0u;
    __asm__ volatile ("dsb; isb" ::: "memory");
}

[[noreturn]] void reset_or_halt()
{
    kernel_mark_stopping();

#if PANIC_AUTO_RESTART
    *reg32(SCB_AIRCR) = AIRCR_VECTKEY | AIRCR_SYSRESETREQ;
    __asm__ volatile ("dsb" ::: "memory");
#endif

    for (;;) {
        __asm__ volatile ("wfi");
    }
}

void report_fault(const char* code,
                  const char* reason,
                  uint32_t* frame)
{
    if (panic_active) {
        reset_or_halt();
    }
    panic_active = true;

    uart_puts("\r\n=== FREEARDU FAULT ===\r\n");
    uart_puts("Code: ");
    uart_puts(code != nullptr ? code : "UNKNOWN");
    uart_puts("\r\nReason: ");
    uart_puts(reason != nullptr ? reason : "unspecified");
    uart_puts("\r\n");

    print_fault_frame(frame);
    print_label_hex("CFSR : ", *reg32(SCB_CFSR));
    print_label_hex("HFSR : ", *reg32(SCB_HFSR));
    print_label_hex("DFSR : ", *reg32(SCB_DFSR));
    print_label_hex("MMFAR: ", *reg32(SCB_MMFAR));
    print_label_hex("BFAR : ", *reg32(SCB_BFAR));
    print_label_hex("AFSR : ", *reg32(SCB_AFSR));
    uart_puts("=======================\r\n");

    if (sandbox_fault_recover(frame) != 0) {
        uart_puts("Sandbox execution aborted; firmware continuing.\r\n");
        return;
    }

    reset_or_halt();
}

} // namespace

extern "C" void hard_fault_handler_c(uint32_t* stack_frame)
{
    disable_mpu_for_fault();
    report_fault("HARDFAULT", "Unhandled exception or escalated configurable fault",
                 stack_frame);
}

extern "C" void hard_fault_handler_segfault(uint32_t* stack_frame)
{
    disable_mpu_for_fault();
    report_fault("MEMORY", "Illegal memory access", stack_frame);
}

extern "C" void memmanage_handler_c(uint32_t* stack_frame)
{
    disable_mpu_for_fault();
    report_fault("MEMMANAGE", "MPU memory protection violation", stack_frame);
}

extern "C" void bus_fault_handler_c(uint32_t* stack_frame)
{
    disable_mpu_for_fault();
    report_fault("BUSFAULT", "Bus access violation", stack_frame);
}

extern "C" void usage_fault_handler_c(uint32_t* stack_frame)
{
    disable_mpu_for_fault();
    report_fault("USAGEFAULT", "Invalid instruction or processor state", stack_frame);
}

[[noreturn]] void kernel_panic(const char* errorCode, const char* reason)
{
    report_fault(errorCode, reason, nullptr);
    reset_or_halt();
}

[[noreturn]] void kernel_stop()
{
    reset_or_halt();
}

void panic_init()
{
    /*
     * Route configurable faults to their dedicated handlers instead of
     * escalating them to HardFault. HardFault remains the final fallback.
     */
    *reg32(SCB_SHCSR) |= SHCSR_MEMFAULTENA |
                         SHCSR_BUSFAULTENA |
                         SHCSR_USGFAULTENA;
}

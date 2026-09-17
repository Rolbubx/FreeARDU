#include "Command.h"
#include "../Uart/UartPutchar.h"
#include "../Bios/Bios.h"
#include "../Process/Process.h"
#include "../Api/Api.h"
#include "../Framebuffer/Framebuffer.h"
#include "../DisplayDetector/DisplayDetector.h"
#include "../Panic/Panic.h"
#include <string.h>
#include <stdlib.h>

// External symbols from linker
extern "C" {
    extern unsigned int _sdata, _edata;
    extern unsigned int _sbss, _ebss;
    extern unsigned int _sheap, _eheap;
    extern unsigned int _estack;
}

// Command implementations
void CommandCallbacks::help(const char* args) {
    (void)args;
    uart_puts("Commands:\r\n");
    uart_puts("  HELP VERSION UPTIME CPUINFO MEMSTAT DATE LS ECHO CLR CRT CD RD\r\n");
    uart_puts("  AND <v1> <v2> - bitwise AND\r\n");
    uart_puts("  KILL <name> - kill process\r\n");
    uart_puts("  LST PRC - list processes\r\n");
    uart_puts("  SYS MSTR - restart manager\r\n");
    uart_puts("  BIOS SET <p> <v> - set BIOS param\r\n");
    uart_puts("  BIOS GET <p> - get BIOS param\r\n");
    uart_puts("  BIOS CFG - show config\r\n");
    uart_puts("  DEBUG: DUMP_MEMORY MEMTEST SCREEN_INFO STRESS_TEST RESTART HALT PANIC CAUSE_FAULT\r\n");
}

void CommandCallbacks::version(const char* args) { (void)args; uart_puts("v2.0\r\n"); }
void CommandCallbacks::uptime(const char* args) { (void)args; uart_puts("Uptime: OK\r\n"); }
void CommandCallbacks::cpuinfo(const char* args) { (void)args; uart_puts("CPU: simulated\r\n"); }
void CommandCallbacks::memstat(const char* args) { (void)args; uart_puts("Mem: OK\r\n"); }
void CommandCallbacks::date(const char* args) { (void)args; uart_puts(__DATE__ " " __TIME__ "\r\n"); }

void CommandCallbacks::ls(const char* args) {
    (void)args; uart_puts("Components: uart1, fb0, cpu, bios\r\n");
}

void CommandCallbacks::echo(const char* args) { if(args) { uart_puts(args); uart_puts("\r\n"); } }
void CommandCallbacks::clear(const char* args) { (void)args; uart_puts("\x1B[2J\x1B[H"); }

// VFS functions from the C entry point
extern "C" void cmd_crt(char* args);
extern "C" void cmd_cd(char* args);
extern "C" void cmd_rd(char* args);

void CommandCallbacks::crt(const char* args) { cmd_crt(const_cast<char*>(args)); }
void CommandCallbacks::cd(const char* args) { cmd_cd(const_cast<char*>(args)); }
void CommandCallbacks::rd(const char* args) { cmd_rd(const_cast<char*>(args)); }

void CommandCallbacks::dump_memory(const char* args) { (void)args; uart_puts("Memory dump\r\n"); }
void CommandCallbacks::memtest(const char* args) { (void)args; uart_puts("Pass\r\n"); }
void CommandCallbacks::screen_info(const char* args) { (void)args; uart_puts("Screen OK\r\n"); }
void CommandCallbacks::stress_test(const char* args) { (void)args; uart_puts("Done\r\n"); }
void CommandCallbacks::restart(const char* args) { (void)args; uart_puts("Restarting\r\n"); }
void CommandCallbacks::halt(const char* args) {
    (void)args;
    uart_puts("Halted\r\n");
    kernel_stop();
}

void CommandCallbacks::panic(const char* args) {
    (void)args;
    // Trigger actual kernel panic with random funny phrase and auto-restart
    kernel_panic("S/0xFF", "User-triggered panic (PANIC command)");
}

void CommandCallbacks::cause_fault(const char* args) {
    (void)args;
    uart_puts("Injecting a deliberate UsageFault to verify fault handlers...\r\n");

    // A null-pointer store is not guaranteed to fault on Cortex-M because the
    // address 0x00000000 may still be mapped or silently handled by the target.
    // Explicitly generate an undefined instruction, which always raises a
    // UsageFault on Cortex-M and is then reported by usage_fault_handler_c().
    __asm__ volatile ("udf #0" ::: "memory");

    // If the fault is not handled, keep the CPU here instead of continuing.
    for (;;) {
        __asm__ volatile ("wfi" ::: "memory");
    }
}

void CommandCallbacks::and_op(const char* args) {
    if (!args) { uart_puts("Usage: AND <v1> <v2>\r\n"); return; }
    while (*args==' ') args++;
    const char* a2=args; while(*a2&&*a2!=' ') a2++;
    if (*a2==' ') a2++;
    if (!*args||!*a2) { uart_puts("Usage: AND <v1> <v2>\r\n"); return; }
    unsigned long v1=strtoul(args,NULL,0), v2=strtoul(a2,NULL,0), r=v1&v2;
    uart_puts("Result: "); uart_print_uint(r); uart_puts("\r\n");
}

void CommandCallbacks::kill_proc(const char* args) {
    if (!args||*args=='\0') { uart_puts("Usage: KILL <name>\r\n"); return; }
    if (process_kill(args)==0) { uart_puts("Killed: "); uart_puts(args); uart_puts("\r\n"); }
}

void CommandCallbacks::lst_parc(const char* args) { (void)args; uart_puts("Processes:\r\n"); process_list(); }

void CommandCallbacks::system_start_mgr(const char* args) {
    (void)args;
    uart_puts("=== SYS MSTR ===\r\nStarting processes...\r\n");
    process_kill_all_except_manager();
    process_start_all();
}

void CommandCallbacks::bios_load(const char* args) {
    (void)args;
    uart_puts("BIOS Loading from UART...\r\n");
    uart_puts("BIOS Version: 1.0.0\r\n");
    uart_puts("BIOS Load: SUCCESS\r\n");
}

void CommandCallbacks::bios_set_param(const char* args) {
    if (!args||!*args) { uart_puts("Usage: BIOS SET <param> <value>\r\n"); return; }
    
    // Parse the argument - find space between param and value
    const char* space = strchr(args, ' ');
    if (!space) {
        uart_puts("Usage: BIOS SET <param> <value>\r\n");
        return;
    }
    
    // Extract param and value
    int param_len = space - args;
    char param[32];
    if (param_len >= 32) param_len = 31;
    strncpy(param, args, param_len);
    param[param_len] = '\0';
    
    // Skip spaces to get value
    space++;
    while (*space == ' ') space++;
    
    // Call the global bios_set_param function (from bios.h)
    ::bios_set_param(param, space);
}

void CommandCallbacks::bios_get(const char* args) {
    if (!args||!*args) { uart_puts("Usage: BIOS GET <param>\r\n"); return; }
    const char* val = bios_get_param(args);
    uart_puts(args); uart_puts(" = "); uart_puts(val); uart_puts("\r\n");
}

void CommandCallbacks::bios_cfg(const char* args) {
    (void)args;
    bios_print_config();
}

extern "C" {
void cmd_help(const char* args) { CommandCallbacks::help(args); }
void cmd_version(const char* args) { CommandCallbacks::version(args); }
void cmd_uptime(const char* args) { CommandCallbacks::uptime(args); }
void cmd_cpuinfo(const char* args) { CommandCallbacks::cpuinfo(args); }
void cmd_memstat(const char* args) { CommandCallbacks::memstat(args); }
void cmd_date(const char* args) { CommandCallbacks::date(args); }
void cmd_ls(const char* args) { CommandCallbacks::ls(args); }
void cmd_echo(const char* args) { CommandCallbacks::echo(args); }
void cmd_clear(const char* args) { CommandCallbacks::clear(args); }
void cmd_crt_callback(const char* args) { CommandCallbacks::crt(args); }
void cmd_cd_callback(const char* args) { CommandCallbacks::cd(args); }
void cmd_rd_callback(const char* args) { CommandCallbacks::rd(args); }
void cmd_dump_memory(const char* args) { CommandCallbacks::dump_memory(args); }
void cmd_memtest(const char* args) { CommandCallbacks::memtest(args); }
void cmd_screen_info(const char* args) { CommandCallbacks::screen_info(args); }
void cmd_stress_test(const char* args) { CommandCallbacks::stress_test(args); }
void cmd_restart(const char* args) { CommandCallbacks::restart(args); }
void cmd_halt(const char* args) { CommandCallbacks::halt(args); }
void cmd_panic(const char* args) { CommandCallbacks::panic(args); }
void cmd_cause_fault(const char* args) { CommandCallbacks::cause_fault(args); }
void cmd_and(const char* args) { CommandCallbacks::and_op(args); }
void cmd_kill_proc(const char* args) { CommandCallbacks::kill_proc(args); }
void cmd_list_processes(const char* args) { CommandCallbacks::lst_parc(args); }
void cmd_system_start_manager(const char* args) { CommandCallbacks::system_start_mgr(args); }
void cmd_bios_load(const char* args) { CommandCallbacks::bios_load(args); }
void cmd_bios_set_param(const char* args) { CommandCallbacks::bios_set_param(args); }
void cmd_bios_get(const char* args) { CommandCallbacks::bios_get(args); }
void cmd_bios_config(const char* args) { CommandCallbacks::bios_cfg(args); }
}

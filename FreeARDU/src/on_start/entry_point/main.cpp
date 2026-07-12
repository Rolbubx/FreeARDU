#ifdef FREEARDU_BARE_METAL
#include "../../framebuffer/framebuffer.h"
#include "../../GraphicalEntryDetector/graphical_entry_detector.h"
#include "../../api/api.h"
#include "uart_putc/UART_PUTCHAR.h"
#include "panic/panic.h"


// ============================================================
// Configuration
// ============================================================
#define CPU_FREQ 600000000

extern "C" uint32_t upd_mode_entry(uint32_t mhz);

static uint32_t current_cpu_mhz = CPU_FREQ;

void simulate_cycle_delay() {
    if (current_cpu_mhz < CPU_FREQ) {
        // Simple approximation: if we want to simulate 100MHz on 600MHz hardware,
        // each "cycle" should take 6 times longer.
        // However, we'll implement it as a global slowdown in wait_cycles.
    }
}

#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_BASIC   1
#define DEBUG_LEVEL_VERBOSE 2

#define CURRENT_DEBUG_LEVEL DEBUG_LEVEL_VERBOSE


#define IsProductionMode true

// Master switch for all debug/diagnostic commands.
// Set to false to lock the shell down to a minimal safe command set
// (useful for a "production" build where you don't want DUMP_MEMORY,
// PANIC, MEMTEST etc. exposed over UART).
#define DEBUG_COMMANDS_ENABLED true

#define STRESS_TEST_ITERATIONS 10000000
#define CMD_BUFFER_SIZE 64

#define BUILD_VERSION "Build 1.0.1"
#define BUILD_DATE    __DATE__
#define BUILD_TIME    __TIME__


// ============================================================
// Debug macros
// ============================================================
#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_BASIC
    #define LOG_BASIC(msg) uart_puts(msg)
#else
    #define LOG_BASIC(msg)
#endif

#if CURRENT_DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
    #define LOG_VERBOSE(msg) uart_puts(msg)
#else
    #define LOG_VERBOSE(msg)
#endif

// ============================================================
// Utilities
// ============================================================
bool string_equals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }
    return *a == *b;
}

bool string_starts_with(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return false;
        str++;
        prefix++;
    }
    return true;
}

// ============================================================
// Virtual File System (VFS)
// ============================================================
#define MAX_FILES 16
#define MAX_FILENAME 32
#define MAX_CONTENT 128

struct VFSNode {
    char name[MAX_FILENAME];
    char content[MAX_CONTENT];
    bool is_folder;
    bool used;
    int parent_idx;
};

VFSNode vfs[MAX_FILES];
int current_dir_idx = -1; // Root

void vfs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        vfs[i].used = false;
    }
}

int find_node(const char* name, int parent) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (vfs[i].used && vfs[i].parent_idx == parent && string_equals(vfs[i].name, name)) {
            return i;
        }
    }
    return -1;
}

void cmd_crt(char* args) {
    // Basic parser for crt "Name" -content "Content"
    // Expecting: "Name" -content "Content"
    if (*args != '"') {
        uart_puts("Usage: crt \"FolderName\" [-content \"here is the content\"]\r\n");
        return;
    }

    args++; // skip first "
    char name[MAX_FILENAME];
    int i = 0;
    while (*args && *args != '"' && i < MAX_FILENAME - 1) {
        name[i++] = *args++;
    }
    name[i] = '\0';
    if (*args == '"') args++;

    char content[MAX_CONTENT] = "";
    while (*args && *args == ' ') args++;
    if (string_starts_with(args, "-content \"")) {
        args += 10;
        i = 0;
        while (*args && *args != '"' && i < MAX_CONTENT - 1) {
            content[i++] = *args++;
        }
        content[i] = '\0';
    }

    // Find free slot
    int slot = -1;
    for (int j = 0; j < MAX_FILES; j++) {
        if (!vfs[j].used) {
            slot = j;
            break;
        }
    }

    if (slot == -1) {
        kernel_panic("S/0x06", "VFS Storage Exhausted (Critical system file limit reached)");
        return;
    }

    for (int k = 0; k < i; k++) vfs[slot].content[k] = content[k];
    vfs[slot].content[i] = '\0';
    
    i = 0;
    while(name[i]) { vfs[slot].name[i] = name[i]; i++; }
    vfs[slot].name[i] = '\0';

    vfs[slot].used = true;
    vfs[slot].parent_idx = current_dir_idx;
    vfs[slot].is_folder = (content[0] == '\0'); // If no content, assume folder for simplicity or as requested

    uart_puts("Created: ");
    uart_puts(name);
    uart_puts("\r\n");
}

void cmd_cd(char* args) {
    if (string_equals(args, "..")) {
        if (current_dir_idx != -1) {
            current_dir_idx = vfs[current_dir_idx].parent_idx;
        }
        return;
    }
    
    if (*args == '"') args++;
    char name[MAX_FILENAME];
    int i = 0;
    while (*args && *args != '"' && i < MAX_FILENAME - 1) {
        name[i++] = *args++;
    }
    name[i] = '\0';

    int idx = find_node(name, current_dir_idx);
    if (idx != -1) {
        current_dir_idx = idx;
        if (vfs[idx].is_folder == false && !string_equals(name, "..")) {
             // If someone tries to CD into a file, we could warn, but let's stick to the user's "panic can really happen"
             // maybe not here though as it's a user error.
        }
    } else {
        uart_puts("Directory not found\r\n");
    }
}

void cmd_rd(char* args) {
    // Simplistic path handling
    if (*args == '"') args++;
    char name[MAX_FILENAME];
    int i = 0;
    while (*args && *args != '"' && i < MAX_FILENAME - 1) {
        name[i++] = *args++;
    }
    name[i] = '\0';

    int idx = find_node(name, current_dir_idx);
    if (idx != -1) {
        uart_puts("Content: ");
        uart_puts(vfs[idx].content);
        uart_puts("\r\n");
    } else {
        uart_puts("File/Folder not found\r\n");
    }
}

// ============================================================
// Linker-provided symbols
// ============================================================
extern "C" {
    extern unsigned int _sdata, _edata;
    extern unsigned int _sbss, _ebss;
    extern unsigned int _sheap, _eheap;
    extern unsigned int _estack;
}

extern "C" void reset_handler();

extern DisplayDriver display;

// ============================================================
// DWT cycle counter (hardware uptime, Cortex-M7)
// ============================================================
#define DWT_CTRL   (*(volatile unsigned int*)0xE0001000)
#define DWT_CYCCNT (*(volatile unsigned int*)0xE0001004)
#define DEMCR      (*(volatile unsigned int*)0xE000EDFC)

void uptime_init() {
    DEMCR |= (1 << 24);      // TRCENA: enable trace/debug block
    DWT_CYCCNT = 0;
    DWT_CTRL |= 1;           // CYCCNTENA: enable the cycle counter
}

unsigned int get_cycle_count() {
    return DWT_CYCCNT;
}

// ============================================================
// Waiter / Scheduler
// ============================================================
class WaiterScheduler {
public:
    float transform_cycles_to_seconds(volatile unsigned int cycles) {
        return (float)cycles / CPU_FREQ;
    }

    void wait_cycles(volatile unsigned int count) {
        if (current_cpu_mhz < CPU_FREQ) {
            // Factor to slow down. e.g. 600M / 100M = 6.
            uint32_t factor = CPU_FREQ / current_cpu_mhz;
            count *= factor;
        }
        while (count > 0) {
            count--;
        }
    }
};

// ============================================================
// Diagnostics
// ============================================================
void print_memstat() {
    unsigned int data_size = (unsigned int)&_edata - (unsigned int)&_sdata;
    unsigned int bss_size  = (unsigned int)&_ebss  - (unsigned int)&_sbss;
    unsigned int heap_size = (unsigned int)&_eheap - (unsigned int)&_sheap;
    unsigned int total_used = data_size + bss_size + heap_size;

    uart_puts("--- Memory Statistics ---\r\n");
    uart_puts("Total SRAM Used: ");
    uart_print_uint(total_used);
    uart_puts(" bytes\r\n");
    uart_puts("  .data: "); uart_print_uint(data_size); uart_puts("\r\n");
    uart_puts("  .bss : "); uart_print_uint(bss_size); uart_puts("\r\n");
    uart_puts("  heap : "); uart_print_uint(heap_size); uart_puts("\r\n");
    uart_puts("Stack Top: ");
    uart_print_hex((unsigned int)&_estack);
    uart_puts("\r\n-------------------------\r\n");
}

void print_cpuinfo() {
    uart_puts("--- CPU Information ---\r\n");
    uart_puts("Model: NXP i.MX RT1060 (MIMXRT1062)\r\n");
    uart_puts("Core: ARM Cortex-M7\r\n");
    uart_puts("Clock Speed: ");
    uart_print_uint(current_cpu_mhz / 1000000);
    uart_puts(" MHz");
    if (current_cpu_mhz < CPU_FREQ) {
        uart_puts(" (Simulated from ");
        uart_print_uint(CPU_FREQ / 1000000);
        uart_puts(" MHz)");
    }
    uart_puts("\r\n");
    uart_puts("FPU: FPv5-D16 (Hard)\r\n");
    uart_puts("-----------------------\r\n");
}

void dump_memory() {
    unsigned int data_size = (unsigned int)&_edata - (unsigned int)&_sdata;
    unsigned int bss_size  = (unsigned int)&_ebss  - (unsigned int)&_sbss;
    unsigned int heap_size = (unsigned int)&_eheap - (unsigned int)&_sheap;

    uart_puts("=== Memory Dump ===\r\n");

    uart_puts(".data : start=");
    uart_print_hex((unsigned int)&_sdata);
    uart_puts(" end=");
    uart_print_hex((unsigned int)&_edata);
    uart_puts(" size=");
    uart_print_uint(data_size);
    uart_puts(" bytes\r\n");

    uart_puts(".bss  : start=");
    uart_print_hex((unsigned int)&_sbss);
    uart_puts(" end=");
    uart_print_hex((unsigned int)&_ebss);
    uart_puts(" size=");
    uart_print_uint(bss_size);
    uart_puts(" bytes\r\n");

    uart_puts("heap  : start=");
    uart_print_hex((unsigned int)&_sheap);
    uart_puts(" end=");
    uart_print_hex((unsigned int)&_eheap);
    uart_puts(" size=");
    uart_print_uint(heap_size);
    uart_puts(" bytes\r\n");

    uart_puts("stack : top=");
    uart_print_hex((unsigned int)&_estack);
    uart_puts("\r\n");

    uart_puts("====================\r\n");
}

void stress_test_cpu(unsigned int iterations) {
    uart_puts("Starting CPU stress test (");
    uart_print_uint(iterations);
    uart_puts(" iterations)...\r\n");

    unsigned int start = get_cycle_count();
    volatile unsigned long dummy = 0;
    for (unsigned int i = 0; i < iterations; i++) {
        dummy += i * i;
    }
    unsigned int elapsed = get_cycle_count() - start;

    uart_puts("Stress test done. Result checksum: ");
    uart_print_hex((unsigned int)dummy);
    uart_puts("\r\n");
    uart_puts("Elapsed cycles: ");
    uart_print_uint(elapsed);
    uart_puts("\r\n");
}

// Writes then reads back a test pattern in the heap region to catch
// stuck-at-bit or wiring faults in RAM. Non-destructive: restores
// nothing, since the heap isn't actively used elsewhere yet.
void run_memtest() {
    unsigned int* start = &_sheap;
    unsigned int* end = &_eheap;
    unsigned int pattern = 0xA5A5A5A5;
    unsigned int errors = 0;

    uart_puts("Running RAM test on heap region...\r\n");

    for (unsigned int* p = start; p < end; p++) {
        *p = pattern;
    }

    for (unsigned int* p = start; p < end; p++) {
        if (*p != pattern) {
            errors++;
        }
    }

    if (errors == 0) {
        uart_puts("MEMTEST PASSED. ");
        uart_print_uint((unsigned int)(end - start) * 4);
        uart_puts(" bytes verified.\r\n");
    } else {
        uart_puts("MEMTEST FAILED. ");
        uart_print_uint(errors);
        uart_puts(" word(s) mismatched!\r\n");
    }
}

void print_screen_info() {
    ScreenDetectionResult info = DETECT_SCRN();

    uart_puts("=== Screen Detection Info ===\r\n");
    uart_puts("Type: ");

    switch (info.type) {
        case SCREEN_NONE:             uart_puts("NONE"); break;
        case SCREEN_SPI_DISPLAY:      uart_puts("SPI Display"); break;
        case SCREEN_PARALLEL_DISPLAY: uart_puts("Parallel Display"); break;
        default:                      uart_puts("UNKNOWN"); break;
    }
    uart_puts("\r\n");

    uart_puts("Resolution: ");
    uart_print_uint((unsigned int)info.width);
    uart_puts("x");
    uart_print_uint((unsigned int)info.height);
    uart_puts("\r\n");

    uart_puts("Can draw: ");
    uart_puts(info.canDraw ? "YES" : "NO");
    uart_puts("\r\n");

    uart_puts("==============================\r\n");
}

void print_uptime() {
    unsigned int cycles = get_cycle_count();
    float seconds = (float)cycles / current_cpu_mhz;

    uart_puts("Uptime: ");
    uart_print_uint(cycles);
    uart_puts(" cycles (~");
    uart_print_uint((unsigned int)seconds);
    uart_puts(" seconds)\r\n");
}

void print_version() {
    uart_puts(BUILD_VERSION);
    uart_puts(" (Built on ");
    uart_puts(BUILD_DATE);
    uart_puts(" at ");
    uart_puts(BUILD_TIME);
    uart_puts(")\r\n");
}

// ============================================================
// Command handling
// ============================================================
void print_help() {
    uart_puts("Available commands:\r\n");
    uart_puts("  HELP          - show this message\r\n");
    uart_puts("  VERSION       - show firmware version info\r\n");
    uart_puts("  UPTIME        - show elapsed cycles since boot\r\n");
    uart_puts("  CPUINFO       - show detailed CPU information\r\n");
    uart_puts("  MEMSTAT       - show memory usage statistics\r\n");
    uart_puts("  DATE          - show build date and time\r\n");
    uart_puts("  LS            - list system components\r\n");
    uart_puts("  ECHO <text>   - echo back the given text\r\n");
    uart_puts("  CLEAR         - clear the terminal screen\r\n");
    uart_puts("  CRT \"name\"    - create a file/folder\r\n");
    uart_puts("  CD \"name\"     - change directory\r\n");
    uart_puts("  RD \"name\"     - read file/folder content\r\n");

    #if DEBUG_COMMANDS_ENABLED
        uart_puts("  --- debug commands ---\r\n");
        uart_puts("  DUMP_MEMORY   - show memory layout and usage\r\n");
        uart_puts("  MEMTEST       - test heap RAM integrity\r\n");
        uart_puts("  SCREEN_INFO   - show display detection results\r\n");
        uart_puts("  STRESS_TEST   - run a CPU stress test\r\n");
        uart_puts("  RESTART       - soft-reset the system\r\n");
        uart_puts("  HALT          - stop the CPU\r\n");
        uart_puts("  PANIC         - trigger a test kernel panic\r\n");
        uart_puts("  CAUSE_FAULT   - trigger a real hardware HardFault\r\n");
    #endif
}

void handle_command(char* cmd) {

    int len = 0;
    while (cmd[len]) len++;
    while (len > 0 && cmd[len - 1] == ' ') {
        cmd[--len] = '\0';
    }

    if (string_equals(cmd, "")) {
        return;
    }
    else if (string_equals(cmd, "HELP")) {
        print_help();
    }
    else if (string_equals(cmd, "VERSION")) {
        print_version();
    }
    else if (string_equals(cmd, "UPTIME")) {
        print_uptime();
    }
    else if (string_equals(cmd, "CPUINFO")) {
        print_cpuinfo();
    }
    else if (string_equals(cmd, "MEMSTAT")) {
        print_memstat();
    }
    else if (string_equals(cmd, "DATE")) {
        uart_puts("Build Date: " __DATE__ "\r\n");
        uart_puts("Build Time: " __TIME__ "\r\n");
    }
    else if (string_equals(cmd, "LS")) {
        uart_puts("Contents of current directory:\r\n");
        for (int i = 0; i < MAX_FILES; i++) {
            if (vfs[i].used && vfs[i].parent_idx == current_dir_idx) {
                uart_puts(vfs[i].is_folder ? "[DIR] " : "[FILE] ");
                uart_puts(vfs[i].name);
                uart_puts("\r\n");
            }
        }
        uart_puts("\r\nSystem Components:\r\n");
        uart_puts("  /dev/uart1 (LPUART1)\r\n");
        uart_puts("  /dev/fb0   (Framebuffer)\r\n");
        uart_puts("  /sys/cpu   (Cortex-M7)\r\n");
    }
    else if (string_starts_with(cmd, "ECHO ")) {
        uart_puts(cmd + 5);
        uart_puts("\r\n");
    }
    else if (string_starts_with(cmd, "CRT ")) {
        cmd_crt(cmd + 4);
    }
    else if (string_starts_with(cmd, "CD ")) {
        cmd_cd(cmd + 3);
    }
    else if (string_starts_with(cmd, "RD ")) {
        cmd_rd(cmd + 3);
    }
    else if (string_equals(cmd, "CLEAR")) {
        uart_puts("\x1B[2J\x1B[H"); // ANSI clear screen + cursor home
    }
    #if DEBUG_COMMANDS_ENABLED
    else if (string_equals(cmd, "DUMP_MEMORY")) {
        dump_memory();
    }
    else if (string_equals(cmd, "MEMTEST")) {
        run_memtest();
    }
    else if (string_equals(cmd, "SCREEN_INFO")) {
        print_screen_info();
    }
    else if (string_equals(cmd, "STRESS_TEST")) {
        stress_test_cpu(STRESS_TEST_ITERATIONS);
    }
    else if (string_equals(cmd, "RESTART")) {
        uart_puts("Restarting system...\r\n");
        reset_handler();
    }
    else if (string_equals(cmd, "HALT")) {
        uart_puts("System halted.\r\n");
        while(1) { __asm("wfi"); }
    }
    else if (string_equals(cmd, "PANIC")) {
        kernel_panic("S/0xFF", "User-triggered test panic (PANIC command)");
    }
    else if (string_equals(cmd, "CAUSE_FAULT")) {
        uart_puts("Triggering HardFault via illegal access...\r\n");
        volatile uint32_t* bad_ptr = (volatile uint32_t*)0xFFFFFFF0;
        uint32_t val = *bad_ptr;
        (void)val;
    }
    #endif
    else {
        uart_puts("Unknown command: '");
        uart_puts(cmd);
        uart_puts("'. Type HELP for a list of commands.\r\n");
    }
}

// ============================================================
// Boot sequence
// ============================================================
void init_display() {
    display.init();
    UG_FillScreen(C_BLACK);
    display.flush();
}

void display_splash() {
    uart_puts("\x1B[2J\x1B[H"); // Clear screen
    uart_puts("  ______               ___    ____  ____  __  __ \r\n");
    uart_puts(" / ____/________  ___ /   |  / __ \\/ __ \\/ / / / \r\n");
    uart_puts("/ /_  / ___/ _ \\/ _ \\/ /| | / /_/ / / / / / / /  \r\n");
    uart_puts("/ __/ / /  /  __/  __/ ___ |/ _, _/ /_/ / /_/ /   \r\n");
    uart_puts("/_/   /_/   \\___/\\___/_/  |_/_/ |_/_____/\\____/    \r\n");
    uart_puts("\r\n");
    uart_puts("FreeARDU Kernel version 1.0.2-baremetal\r\n");
    uart_puts("Initializing system components...\r\n\r\n");

    const char* steps[] = {
        "Initializing Uptime Counter",
        "Checking Memory Layout",
        "Detecting Display Hardware",
        "Initializing Framebuffer",
        "Loading Virtual File System",
        "Starting Shell"
    };
    const int num_steps = 6;

    for (int i = 0; i < num_steps; i++) {
        uart_puts("[ ");
        int progress = (i * 100) / num_steps;
        uart_print_uint(progress);
        uart_puts("% ] ");
        uart_puts(steps[i]);
        uart_puts("... ");


        switch(i) {
            case 0: uptime_init(); break;
            case 1: 
                if (&_sdata < &_edata && &_sbss < &_ebss) {
                    uart_puts("OK"); 
                } else {
                    kernel_panic("S/0x05", "Memory Layout Sanity Check Failed (Linker symbols invalid)");
                }
                break;
            case 2: {
                ScreenDetectionResult scr = DETECT_SCRN();
                if (scr.type != SCREEN_NONE) uart_puts("FOUND");
                else uart_puts("NONE"); // Not necessarily critical, but can be
                break;
            }
            case 3: 
                if (display.init() == 0) {
                    uart_puts("OK");
    // Demo uGUI
                    UG_FillScreen(C_BLUE);
                    UG_DrawFrame(10, 10, 100, 100, C_WHITE);
                    UG_PutString(15, 15, (char*)"FreeARDU uGUI");
                    display.flush();
                    
                    // Added: Verification line to really show something is drawing
                    UG_DrawLine(0, 0, (UG_S16)display.getWidth(), (UG_S16)display.getHeight(), C_RED);
                    UG_DrawLine(0, (UG_S16)display.getHeight(), (UG_S16)display.getWidth(), 0, C_RED);
                    display.flush();
                } else {
                    // Optional: Panic if FB init is required
                    // kernel_panic("H/0x01", "Framebuffer Initialization Failed");
                    uart_puts("FAILED");
                }
                break;
            case 4: vfs_init(); break;
            case 5: break;
        }

        // Apply "slow down" if MHz was changed in UpdMode
        // Factor to slow down. e.g. 600M / 100M = 6.
        uint32_t delay_limit = 2000000;
        if (current_cpu_mhz < CPU_FREQ) {
            uint32_t factor = CPU_FREQ / current_cpu_mhz;
            delay_limit *= factor;
        }

        // Fake loading bar for effect as requested "real loading (like with a loading bar ect)"
        uart_puts("\r\n[");
        for (int j = 0; j < 20; j++) {
            if (j < (i + 1) * 20 / num_steps) uart_putc('=');
            else uart_putc(' ');
        }
        uart_puts("]\r\n");

        for (volatile uint32_t delay = 0; delay < delay_limit; delay++); // Delay to see the bar
    }

    uart_puts("\r\n[ 100% ] Initialization Complete. READY.\r\n");
    uart_puts("\r\nWelcome to FreeARDU DOSUart mode. Type HELP for commands.\r\n");
}



uint32_t parse_uint(const char* str) {
    uint32_t res = 0;
    while (*str >= '0' && *str <= '9') {
        res = res * 10 + (*str - '0');
        str++;
    }
    return res;
}

void enter_upd_mode() {
    uart_puts("\r\nEntering UpdMode.S...\r\n");
    uart_puts("Format: -load_kernel_at_lower_artifical_mhz:600000\r\n");
    uart_puts("UpdMode> ");
    
    char buf[64];
    uart_read_line(buf, 64);
    
    if (string_starts_with(buf, "-load_kernel_at_lower_artifical_mhz:")) {
        char* val_str = buf + 36;
        uint32_t mhz = parse_uint(val_str);
        if (mhz > 0 && mhz <= CPU_FREQ) {
            current_cpu_mhz = upd_mode_entry(mhz);
            uart_puts("CPU Speed set to ");
            uart_print_uint(current_cpu_mhz);
            uart_puts(" Hz\r\n");
        } else {
            uart_puts("Invalid MHz value. Using default.\r\n");
        }
    } else {
        uart_puts("Unknown command in UpdMode.\r\n");
    }
}

// ============================================================
// Main OS loop
// ============================================================
extern "C" int main() {
    uart_init();
    
    uart_puts("Press 'F' to enter UpdMode...\r\n");
    // Wait a bit for 'F'
    for (volatile int i = 0; i < 2000000; i++) {
        if (uart_data_available()) {
            char c = uart_getc();
            if (c == 'F' || c == 'f') {
                enter_upd_mode();
                break;
            }
        }
    }

    display_splash();
    panic_init();

    char cmd_buffer[CMD_BUFFER_SIZE];

    while (1) {
        if (IsProductionMode == false) {
            uart_puts("> ");
            uart_read_line(cmd_buffer, CMD_BUFFER_SIZE);
            handle_command(cmd_buffer);
        } else if (IsProductionMode == true) {
            uart_puts("Free ARDU Production mode");

            // Example API usage
            if (true == false) {
                API.pin->pinMode(13, PIN_MODE_OUTPUT);
                API.pin->digitalWrite(13, PIN_HIGH);
                API.adc->init(0);
                API.watchdog->init(5000);

                // waits 5 seconds
                for (volatile int i = 0; i < 5000000; i++) {
                    if (uart_data_available()) {
                        char c = uart_getc();
                        if (c == 'F' || c == 'f') {
                            enter_upd_mode();
                            break;
                        }
                    }
                    API.watchdog->feed();
                }
                uart_puts("\x1B[2J\x1B[H");
                uart_puts("Entering os mode....");
                // draws a pixel
                UG_DrawPixel(5, 5, C_WHITE);
                display.flush();
            }
        }
    }

    return 0;
}

#else
#include <Arduino.h>

void setup() {
}

void loop() {
}
#endif
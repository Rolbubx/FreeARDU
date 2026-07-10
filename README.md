# FreeARDU

**Bare-Metal Cortex-M7 Firmware for i.MX RT1060 with Framebuffer, Automatic Display Detection, UART Debug Output & Renode Emulation Support**

FreeARDU is an actively developed bare-metal firmware project for the NXP i.MX RT1060 microcontroller, featuring a custom startup/runtime environment, a hand-written linker script, a graphics framebuffer, automatic display detection, a UART debug logging driver, a basic cooperative waiter/scheduler, and a fully working Renode emulation setup — meaning the firmware can be built, booted, and debugged entirely without physical hardware.

> ⚠️ **Board Compatibility**: This project targets the NXP i.MX RT1060 EVK (`mimxrt1060_evk` PlatformIO board ID). The core architecture is portable to other Cortex-M7 devices, but hardware-specific configuration (linker script, memory layout, peripheral addresses) is tailored to this chip. Adapting to another board requires editing `imxrt1060.ld` and `platformio.ini`.

## ⚠️ Status

This project is in **active development**. Core boot, memory init, and UART logging are confirmed working (both via `build.bat` compilation and Renode emulation). Display output over real I2C/SPI hardware has not yet been validated on physical silicon. Contributions and issue reports are welcome.

---

## Features

### Custom Bare-Metal Startup (`startup.S`)
- Full Cortex-M7 reset sequence, hand-written in assembly
- Copies `.data` from Flash to DTCM, zeroes `.bss`
- Runs C++ global constructors (`__init_array`) — required for global objects like `framebuffer`
- Supports both a standard `main()` entry point and Arduino-style `setup()`/`loop()`
- Weak default exception handlers for NMI, HardFault, MemManage, BusFault, UsageFault, SVC, PendSV, SysTick

### Optimized Linker Script (`imxrt1060.ld`)
- Targets the real i.MX RT1060 memory map (ITCM/DTCM/OCRAM 512KB each, 8MB external FLASH via FlexSPI)
- `KEEP()`-protected `.isr_vector` section so `--gc-sections` never strips the vector table
- Explicit `.data`/`.bss`/`.heap`/`._stack` layout in DTCM
- Dead-code elimination via `--gc-sections`, no standard library startup overhead

### Framebuffer Graphics Engine
- Pixel-level drawing API (`PUSH_PIXEL`, `CLEAR`, `FLUSH`)
- Automatic display type detection at `INIT()` time
- Supports multiple display interfaces:
  - I2C OLED (128×64)
  - I2C LCD (16×2)
  - SPI displays (320×240)
  - Parallel displays (480×320)
- Static pixel buffer — no dynamic allocation
- Hardware-agnostic `Color` (normalized float RGB) and `Vector2` types

### Automatic Screen Detection (`GraphicalEntryDetector`)
- I2C address probing for common OLED/LCD addresses
- GPIO-based heuristic detection for SPI and parallel displays
- Returns a full `ScreenDetectionResult` (type, address, pin config, resolution, `canDraw` flag)
- Zero runtime cost when no display is connected

### UART Debug Driver (`uart_putc`)
- Minimal, register-level LPUART1 driver (`uart_init`, `uart_putc`, `uart_puts`)
- No dependency on any HAL — direct register access (`STAT`, `CTRL`, `DATA`)
- Used throughout `main.cpp` to trace the boot sequence and main loop in real time

### Cooperative Waiter/Scheduler (`WaiterScheduler`)
- Simple cycle-based busy-wait timing primitive
- `wait_cycles(count)` — blocking cycle-accurate delay
- `transform_cycles_to_seconds(cycles)` — converts a cycle count to wall-clock seconds based on `CPU_FREQ`
- Foundation for a future cooperative task scheduler

### Bare-Metal HAL Stubs
- Placeholder `Wire_*` / `isPinConnected` implementations for I2C and GPIO
- Easily replaceable with real driver code once running on physical hardware
- Supports both bare-metal (`FREEARDU_BARE_METAL`) and Arduino-compatible build modes

### Renode Emulation Support
- Verified working custom Renode platform description for the RT1060 (adapted from the built-in RT1064 board, since no native RT1060 `.repl` ships with Renode)
- Boots the real compiled `firmware.elf`, executes from the correct vector table, and streams live UART output to Renode's analyzer window
- Lets you test and debug the entire boot sequence, scheduler, and logging — with **no physical EVK board required**

---

## Project Structure

```
FreeARDU/
├── README.md                            # Project documentation
├── platformio.ini                       # PlatformIO build configuration
├── imxrt1060.ld                         # Linker script for i.MX RT1060
├── build.bat                            # One-command build script
├── include/
│   └── README                           # Header file guidelines
├── src/
│   ├── on_start/
│   │   ├── boot/
│   │   │   └── startup.S                # Cortex-M7 startup code (ASM)
│   │   └── entry_point/
│   │       └── main.cpp                 # Application entry point
│   ├── framebuffer/
│   │   ├── framebuffer.h                # Graphics API header
│   │   └── framebuffer.cpp              # Framebuffer implementation
│   ├── GraphicalEntryDetector/
│   │   ├── graphical_entry_detector.h   # Display detection header
│   │   └── graphical_entry_detector.cpp # Display detection logic
│   ├── uart_putc/
│   │   ├── UART_PUTCHAR.h               # UART debug driver header
│   │   └── UART_PUTCHAR.cpp             # UART debug driver implementation
│   └── infos/
│       └── INFOS.h                      # Configuration stub (user-editable)
└── test/                                # Test directory (placeholder)
```

---

## Key Components

### `startup.S` — Cortex-M7 Bare-Metal Bootstrap
- Initializes the stack pointer from `_estack`
- Copies initialized data from Flash to RAM (`.data`)
- Zeros the `.bss` segment
- Runs C++ global constructors (`__init_array_start` → `__init_array_end`)
- Calls an optional weak `SystemInit()` hook for chip-specific setup
- Falls back to Arduino-style `setup()`/`loop()` if `main()` is not defined
- Default handlers provided for all core Cortex-M exceptions

### `imxrt1060.ld` — Memory Layout

| Region | Address Range         | Size  |
|--------|------------------------|-------|
| ITCM (Instruction TCM) | `0x00000000`–`0x00080000` | 512 KB |
| DTCM (Data TCM)        | `0x20000000`–`0x20080000` | 512 KB |
| OCRAM (On-Chip RAM)    | `0x20200000`–`0x20280000` | 512 KB |
| FLASH (external, FlexSPI) | `0x60000000`–`0x60800000` | 8 MB |

- Code and read-only data live in FLASH
- `.data` is stored in FLASH but loaded into DTCM at runtime
- Heap and stack live in DTCM

### `framebuffer.{h,cpp}` — Graphics API

```cpp
class Framebuffer {
public:
    int INIT();                        // Initialize and detect display
    int WIDTH() const;                 // Get framebuffer width
    int HEIGHT() const;                // Get framebuffer height
    int PUSH_PIXEL(Vector2, Color);    // Draw single pixel
    int CLEAR(Color);                  // Clear entire framebuffer
    int FLUSH();                       // Send buffer to display
};

// Color format: normalized floats (0.0–1.0)
struct Color   { float r, g, b; };
struct Vector2 { int x, y; };
```

### `graphical_entry_detector.{h,cpp}` — Display Detection

```cpp
// Automatically detects connected display type and returns metadata
ScreenDetectionResult DETECT_SCRN();

enum ScreenType {
    SCREEN_NONE,
    SCREEN_I2C_OLED,
    SCREEN_I2C_LCD,
    SCREEN_SPI_DISPLAY,
    SCREEN_PARALLEL_DISPLAY
};

struct ScreenDetectionResult {
    enum ScreenType type;
    uint8_t i2cAddress;
    int width, height;
    bool canDraw;
    // ... pin configuration ...
};
```

### `uart_putc/UART_PUTCHAR.{h,cpp}` — UART Debug Driver

```cpp
void uart_init();               // Enable the LPUART1 transmitter
void uart_putc(char c);         // Blocking single-character send
void uart_puts(const char* str);// Blocking null-terminated string send
```

Direct register access on LPUART1 (`0x40184000`): waits on the `TDRE` status bit before writing to `DATA`, and enables the transmitter (`TE`) via `CTRL` on init.

### `main.cpp` — `WaiterScheduler`

```cpp
class WaiterScheduler {
public:
    float transform_cycles_to_seconds(volatile unsigned int cycles);
    void wait_cycles(volatile unsigned int count);
};
```

A minimal cycle-counting primitive, used to pace the main loop and its debug output.

---

## Getting Started

### Prerequisites
- PlatformIO Core (or VS Code / CLion with the PlatformIO plugin)
- Arm GNU Toolchain (auto-installed by PlatformIO)
- J-Link debugger (for flashing/debugging a real NXP EVK board)
- *(Optional, no hardware needed)* [Renode](https://renode.io/) for full emulation

### Building

```bash
# Using the provided build script
./build.bat

# Or directly via PlatformIO
platformio run -e mimxrt1060_evk

# Verbose output
platformio run -e mimxrt1060_evk -v

# Clean build
platformio run -e mimxrt1060_evk --target clean
```

### Flashing (real hardware)

```bash
# Upload to device via J-Link
platformio run -e mimxrt1060_evk --target upload

# Debug with GDB
platformio debug -e mimxrt1060_evk
```

### Running in Renode (no hardware required)

FreeARDU ships with a verified custom Renode platform description, since Renode does not natively include an RT1060 board file. Setup summary:

1. Generate a custom CPU description from the built-in RT1064 file, removing the conflicting FlexSPI memory-mapped region.
2. Map `0x60000000` as flat FLASH memory (bypassing the FlexSPI controller model, which isn't needed for plain code execution).
3. Load the compiled `firmware.elf`, set the CPU's `VectorTableOffset` and `PC` to the `reset_handler` symbol, and start emulation.

```
mach create
machine LoadPlatformDescription @platforms/boards/mimxrt1060_evk.repl
sysbus LoadELF @path/to/firmware.elf
cpu PC `sysbus GetSymbolAddress "reset_handler"`
showAnalyzer sysbus.lpuart1
start
```

You should see live UART output confirming the boot sequence:

```
FreeARDU successfully loaded, now awaiting instructions
Framebuffer initiated
Iniating waiter scheduler
Going to main os loop
Main OS loop
Main OS loop
...
```

---

## Current `main.cpp` Behavior

```cpp
extern "C" int main() {
    uart_init();
    uart_puts("FreeARDU successfully loaded, now awaiting instructions\n");

    framebuffer.INIT();
    uart_puts("Framebuffer initiated \n");

    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    uart_puts("Iniating waiter scheduler \n");
    WaiterScheduler s;

    uart_puts("Going to main os loop \n");
    while (1) {
        s.wait_cycles(100000000);
        uart_puts("Main OS loop \n");
    }
    return 0;
}
```

---

## Build Configuration

### `platformio.ini`

```ini
[env:mimxrt1060_evk]
platform = platformio/nxpimxrt
board = mimxrt1060_evk
framework =                    # Bare-metal (no RTOS/framework)

build_flags =
    -Wl,-T,FreeARDU/imxrt1060.ld  # Use our custom linker script
    -Wl,--nostdlib                # Don't pull in a default startup/link setup
    -mcpu=cortex-m7                # Cortex-M7 CPU
    -mthumb                        # Thumb instruction set
    -mfpu=fpv5-d16                 # FPUv5 with 16 double-precision registers
    -mfloat-abi=soft                # Software floating-point ABI
    -DCPU_MIMXRT1062DVL6A          # Chip identifier
    -DFREEARDU_BARE_METAL          # Enable bare-metal mode
    -O3                             # Optimize for speed
    -ffunction-sections
    -fdata-sections
    -fno-exceptions                 # Disable C++ exceptions
    -fno-rtti                       # Disable RTTI
    -nostartfiles                   # No default crt0
    -Wl,--gc-sections                # Dead code elimination
    -Wl,--relax

upload_protocol = jlink
debug_tool = jlink
```

> **Note:** `-mfloat-abi=soft` is used (not `hard`) to keep floating-point calling conventions software-based, matching the current lack of hardware FPU initialization in `startup.S`.

### Compile Flags Reference

| Flag | Purpose |
|------|---------|
| `-DFREEARDU_BARE_METAL` | Enables bare-metal mode; disables Arduino headers |
| `-Wl,-T,FreeARDU/imxrt1060.ld` | Forces the linker to use our custom memory layout instead of the platform default |
| `-fno-exceptions` | Reduces code size by removing C++ exception handling |
| `-fno-rtti` | Disables runtime type information |
| `-Wl,--gc-sections` | Links only used sections (smaller binaries); relies on `KEEP()` in the linker script to protect the vector table |

---

## Architecture & Data Flow

### Startup Sequence

```
1. reset_handler (startup.S)
   ├─ Set stack pointer
   ├─ Copy .data from Flash → DTCM
   ├─ Zero .bss segment
   ├─ Call SystemInit() [optional chip init]
   ├─ Run C++ global constructors
   ├─ Enable interrupts
   └─ Jump to main() or setup()/loop()

2. main() (entry_point/main.cpp)
   ├─ uart_init() + boot log
   ├─ framebuffer.INIT()
   │  └─ Detect display type via DETECT_SCRN()
   ├─ Clear and flush display
   ├─ Construct WaiterScheduler
   └─ Enter main loop (paced by wait_cycles + UART logging)
```

### Display Detection Priority

`graphical_entry_detector.cpp` probes displays in this order:

1. I2C OLED (addresses `0x3C`, `0x3D`) → 128×64
2. I2C LCD (addresses `0x20`–`0x27`, `0x38`–`0x3F`) → 16×2
3. SPI Display (GPIO pins 5–13, 3+ pins detected) → 320×240
4. Parallel Display (GPIO pins 14–23, 6+ pins detected) → 480×320
5. No Display (returns `SCREEN_NONE`)

### Memory Layout at Runtime

```
DTCM (0x20000000)
├─ .data     (initialized globals)
├─ .bss      (uninitialized globals)
├─ .heap     (512 bytes minimum)
└─ ._stack   (1KB minimum, grows downward)

FLASH (0x60000000)
├─ .isr_vector    (exception table, KEEP()-protected)
├─ .text          (code)
├─ .rodata        (constants, look-up tables)
├─ .init_array    (global constructor pointers)
└─ [.data initialization image]
```

---

## Extending the Project

### Adding a New Display Driver

Create `FreeARDU/src/framebuffer/drivers/my_display.cpp`:

```cpp
// Detect display
static ScreenDetectionResult detectMyDisplay() {
    // Your detection logic
    // Return screen info
}

// Implement flush for your display
int Framebuffer::flushMyDisplay() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            sendPixel(pixels[y][x]);
        }
    }
    return 0;
}
```

Then update `framebuffer.cpp` and `graphical_entry_detector.cpp` to register and call your driver.

### Implementing `SystemInit()`

Override the weak `SystemInit()` in your code:

```cpp
extern "C" void SystemInit() {
    // Initialize PLL, clock tree, GPIO, etc.
    // Configure IOMUX for display pins
    // Set up I2C/SPI peripherals
}
```

### Using Arduino-Style Code

Compile with `FREEARDU_BARE_METAL` disabled to use:

```cpp
void setup() {
    // One-time initialization
}

void loop() {
    // Runs repeatedly
}
```

---

## Memory & Performance

### Current Binary Size (from latest build)
- FLASH used: **2,076 bytes** of 8 MB available
- RAM used: **612 bytes** of 32 MB available (DTCM)

*(Numbers reflect the current minimal `main.cpp`; will grow as more features — display drivers, rendering, scheduling — are added.)*

### Performance
- Clock speed target: 600 MHz Cortex-M7
- `FLUSH()` latency: depends on display interface (I2C ~50ms, SPI ~10ms — not yet benchmarked on real hardware)

---

## Supported Displays

| Interface | Type | Resolution | Bus Speed | Typical Use |
|-----------|------|------------|-----------|-------------|
| I2C | OLED | 128×64 | 100–400 kHz | Status displays, small UI |
| I2C | LCD | 16×2 | 100–400 kHz | Text-only, low power |
| SPI | ILI9341, ST7789 | 320×240 | 10–40 MHz | Graphics, fast updates |
| Parallel | 8/16-bit | 480×320+ | 5–20 MHz | High-res graphics |

---

## Troubleshooting

### Build Failures

**Problem:** `undefined reference to 'main'`
**Solution:** Make sure your linker script (`-Wl,-T,...`) is actually being passed, and that PlatformIO isn't silently falling back to its default CRT0/linker script. Verify with `arm-none-eabi-nm firmware.elf | grep vector_table` — it should resolve to an address in FLASH (`0x60000000` range), not somewhere near `0x8000`.

**Problem:** `collect2.exe: error: ld returned 1 exit status` with `Access refusé` / `Access denied` on `firmware.elf`
**Solution:** Usually caused by another process (Renode, an open debugger session, antivirus scanning) holding a lock on the file. Close Renode and any debugger, then rebuild.

### Vector Table / Boot Issues (Renode)

**Problem:** `cpu VectorTableOffset` reports "symbol not found"
**Solution:** Confirm your linker script wraps `.isr_vector` in `KEEP(*(.isr_vector))` — without it, `--gc-sections` will silently strip the vector table and its symbol from the final binary.

**Problem:** CPU stays halted / `PC` stuck at the same address after `start`
**Solution:** In Cortex-M, `PC` must have its Thumb bit set (LSB = 1) when jumping to a handler. Also verify you're not writing directly into a FlexSPI-controller-mapped region (like the default RT1064 `.repl`'s `flex_spi` at `0x60000000`) — for plain code execution, remap that address as flat `Memory.MappedMemory` instead.

### Display Not Detected

- Check I2C/SPI pin connections
- Verify display address matches the expected table in `graphical_entry_detector.cpp`
- Add debug output via `uart_puts()` inside `DETECT_SCRN()`
- Use an I2C scanner to verify the address

---

## Contributing

Contributions are welcome! Please:

- Test on real hardware (i.MX RT1060 EVK) when possible, and note if you've only tested in Renode
- Follow the existing code style (K&R for C, Google C++ style for C++)
- Add documentation for new features
- Report issues with full reproduction steps (build logs, Renode monitor output, etc.)

---

## License

MIT License — See `LICENSE` for details.

Copyright © 2026 — FreeARDU Contributors

---

## Acknowledgments

- **Jibril** — Project author
- **NXP** — i.MX RT1060 technical reference & examples
- **ARM** — Cortex-M7 architecture documentation
- **PlatformIO** — Build and deployment infrastructure
- **Renode / Antmicro** — Open-source hardware emulation making hardware-free development and debugging possible

---

## References

- [NXP i.MX RT1060 Reference Manual](https://www.nxp.com/)
- [ARM Cortex-M7 Processor Reference](https://developer.arm.com/)
- [GNU Arm Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [Renode Documentation](https://renode.readthedocs.io/)

---

## Quick Reference

```bash
# Clone and enter project
git clone https://github.com/Rolbubx/FreeARDU.git
cd FreeARDU/FreeARDU

# Build
./build.bat
# or: platformio run -e mimxrt1060_evk

# Upload (real hardware)
platformio run -e mimxrt1060_evk --target upload

# Monitor serial (after upload, real hardware)
platformio device monitor -b 115200

# Run in Renode (no hardware needed)
cd path/to/renode
./bin/Renode.exe freeardu.resc
```

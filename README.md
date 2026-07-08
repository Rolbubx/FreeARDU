# FreeARDU

**Bare-Metal Cortex-M7 Firmware for i.MX RT1060 with Framebuffer & Automatic Display Detection**

FreeARDU is a production-ready bare-metal firmware project for the NXP i.MX RT1060 microcontroller, featuring a custom startup/runtime environment, optimized linker script, graphics framebuffer support, and automatic hardware detection for multiple display interfaces. This project provides developers with a lightweight foundation for embedded graphics applications on Cortex-M7 systems.

## ⚠️ Status

**This project is in active development** — compilation may fail on some configurations. Contributors and users are encouraged to report issues.

## Features

- **Custom Bare-Metal Startup** (`startup.S`)
  - Full Cortex-M7 initialization with C++ global constructor support
  - Hardware-agnostic exception vector table
  - Support for both `main()` and Arduino-style `setup()`/`loop()` entry points
  - Proper memory initialization (copy .data, zero .bss, run constructors)

- **Optimized Linker Script** (`imxrt1060.ld`)
  - Targets i.MX RT1060 memory layout (512KB ITCM, 512KB DTCM, 512KB OCRAM)
  - Sections for Flash (8MB), RAM, stack, and heap
  - Garbage collection enabled for minimal binary size
  - No standard library overhead

- **Framebuffer Graphics Engine**
  - Pixel-level drawing API
  - Automatic display type detection
  - Support for multiple display interfaces:
    - I2C OLED (128×64)
    - I2C LCD (16×2)
    - SPI displays (320×240)
    - Parallel displays (480×320)
  - Hardware-agnostic color management
  - Efficient buffer management without dynamic allocation

- **Automatic Screen Detection** (`GraphicalEntryDetector`)
  - I2C address probing for OLED and LCD displays
  - GPIO and SPI pin detection for external displays
  - Runtime screen capability reporting
  - Zero-runtime-cost when no display is connected

- **Bare-Metal HAL Stubs**
  - Placeholder implementations for I2C, SPI, and GPIO operations
  - Easily replaceable with actual driver code
  - Supports both bare-metal and Arduino-compatible build modes

---

## Project Structure

```
FreeARDU/
├── README.md                           # Project documentation
├── platformio.ini                      # PlatformIO build configuration
├── imxrt1060.ld                        # Linker script for i.MX RT1060
├── include/
│   └── README                          # Header file guidelines
├── src/
│   ├── on_start/
│   │   ├── boot/
│   │   │   └── startup.S               # Cortex-M7 startup code (ASM)
│   │   └── entry_point/
│   │       └── main.cpp                # Application entry point
│   ├── framebuffer/
│   │   ├── framebuffer.h               # Graphics API header
│   │   └── framebuffer.cpp             # Framebuffer implementation
│   ├── GraphicalEntryDetector/
│   │   ├── graphical_entry_detector.h  # Display detection header
│   │   └── graphical_entry_detector.cpp# Display detection logic
│   └── infos/
│       └── INFOS.h                     # Configuration stub (user-editable)
└── test/                               # Test directory (placeholder)
```

### Key Components

#### **startup.S** — Cortex-M7 Bare-Metal Bootstrap
- Initializes stack pointer
- Copies initialized data from Flash to RAM
- Zeros BSS segment
- Runs C++ global constructors (`__init_array`)
- Supports optional `SystemInit()` hook for chip-specific initialization
- Falls back to `setup()`/`loop()` if `main()` is not defined
- Default exception handlers provided

#### **imxrt1060.ld** — Memory Layout
- **ITCM (Instruction TCM):** 0x00000000–0x00080000 (512KB)
- **DTCM (Data TCM):** 0x20000000–0x20080000 (512KB)
- **OCRAM (On-Chip RAM):** 0x20200000–0x20280000 (512KB)
- **FLASH (External):** 0x60000000–0x60800000 (8MB)
- Code and read-only data go to FLASH
- Initialized data loaded from FLASH but executed from DTCM
- Heap and stack in DTCM

#### **framebuffer.{h,cpp}** — Graphics API
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
struct Color { float r, g, b; };
struct Vector2 { int x, y; };
```

#### **graphical_entry_detector.{h,cpp}** — Display Detection
```c
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

---

## Getting Started

### Prerequisites

- **PlatformIO Core** (or VS Code with PlatformIO extension)
- **Arm GNU Toolchain** (auto-installed by PlatformIO)
- **J-Link debugger** (for flashing/debugging NXP EVK boards)
- i.MX RT1060 EVK board (or compatible Cortex-M7 dev board)

### Building

```bash
# Build for bare-metal target
platformio run -e nxp_imxrt1060_evk

# Build with verbose output
platformio run -e nxp_imxrt1060_evk -v

# Clean build
platformio run -e nxp_imxrt1060_evk --target clean
```

### Flashing

```bash
# Upload to device via J-Link
platformio run -e nxp_imxrt1060_evk --target upload

# Debug with GDB
platformio debug -e nxp_imxrt1060_evk
```

### Hello World Example

Edit `FreeARDU/src/on_start/entry_point/main.cpp`:

```cpp
#include "../../framebuffer/framebuffer.h"

extern Framebuffer framebuffer;

extern "C" int main() {
    // Initialize framebuffer and auto-detect display
    if (framebuffer.INIT() != 0) {
        // No display detected, halt
        while (1);
    }

    // Clear screen to black
    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    // Draw a red pixel at (10, 10)
    Color red = {1.0f, 0.0f, 0.0f};
    framebuffer.PUSH_PIXEL({10, 10}, red);
    framebuffer.FLUSH();

    while (1) {
        // Main loop
    }
    return 0;
}
```

---

## Build Configuration

### platformio.ini

```ini
[env:nxp_imxrt1060_evk]
platform = platformio/nxpimxrt
board = nxp_imxrt1060_evk
framework =  # Bare-metal (no RTOS/framework)

build_flags =
    -mcpu=cortex-m7           # Cortex-M7 CPU
    -mthumb                   # Thumb instruction set
    -mfpu=fpv5-d16            # FPUv5 with 16 double-precision registers
    -mfloat-abi=hard          # Hardware floating-point ABI
    -DCPU_MIMXRT1062DVL6A     # Chip identifier
    -DFREEARDU_BARE_METAL     # Enable bare-metal mode
    -O3                       # Optimize for speed
    -fno-exceptions           # Disable C++ exceptions
    -fno-rtti                 # Disable RTTI
    -nostartfiles             # No default crt0
    -Wl,--gc-sections         # Dead code elimination
    -Wl,--entry=reset_handler # Set entry point

upload_protocol = jlink       # J-Link debugger
debug_tool = jlink
```

### Compile Flags Reference

| Flag | Purpose |
|------|---------|
| `-DFREEARDU_BARE_METAL` | Enables bare-metal mode; disables Arduino headers |
| `-fno-exceptions` | Reduces code size by removing C++ exception handling |
| `-fno-rtti` | Disables runtime type information |
| `-Wl,--gc-sections` | Links only used sections (smaller binaries) |

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

2. main() or setup() (entry_point/main.cpp)
   ├─ Call framebuffer.INIT()
   │  └─ Detect display type via DETECT_SCRN()
   ├─ Clear and initialize display
   └─ Enter application loop
```

### Display Detection Priority

The `graphical_entry_detector.cpp` probes displays in this order:

1. **I2C OLED** (addresses 0x3C, 0x3D) → 128×64
2. **I2C LCD** (addresses 0x20–0x27, 0x38–0x3F) → 16×2
3. **SPI Display** (GPIO pins 5–13, 3+ pins detected) → 320×240
4. **Parallel Display** (GPIO pins 14–23, 6+ pins detected) → 480×320
5. **No Display** (returns `SCREEN_NONE`)

### Memory Layout at Runtime

```
DTCM (0x20000000)
├─ .data (initialized globals)
├─ .bss (uninitialized globals)
├─ .heap (256 bytes minimum)
└─ ._stack (1KB minimum, grows downward)

FLASH (0x60000000)
├─ .isr_vector (exception table)
├─ .text (code)
├─ .rodata (constants, look-up tables)
├─ .init_array (global constructor pointers)
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
    // Copy pixels array to your display
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            sendPixel(pixels[y][x]);
        }
    }
    return 0;
}
```

Then update `framebuffer.cpp` and `graphical_entry_detector.cpp` to register and call your driver.

### Implementing SystemInit()

Override the weak `SystemInit()` in your code:

```cpp
extern "C" void SystemInit() {
    // Initialize PLL, clock tree, GPIO, etc.
    // Configure IOMUX for display pins
    // Set up I2C/SPI peripherals
}
```

### Using Arduino-Style Code

Compile with `FREEARDU_BARE_METAL` disabled:

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

### Binary Size Estimates

- Minimal (no graphics): **~8 KB**
- With framebuffer & detection: **~25 KB**
- With full HAL: **~40–60 KB**

### RAM Usage

- Static framebuffer (320×240, RGB): **~230 KB**
- BSS & data: **~2 KB**
- Stack: **≥1 KB** (configurable in linker script)
- **Total: ~235 KB** (plenty of headroom in 1 MB)

### Performance

- **Clock speed:** 600 MHz Cortex-M7 with FPU
- **Pixel fill rate:** ~100M pixels/sec (theoretical)
- **FLUSH() latency:** Depends on display interface (I2C: ~50ms, SPI: ~10ms)

---

## Supported Displays

| Interface | Type | Resolution | Bus Speed | Typical Use |
|-----------|------|-----------|-----------|-------------|
| I2C | OLED | 128×64 | 100–400 kHz | Status displays, small UI |
| I2C | LCD | 16×2 | 100–400 kHz | Text-only, low power |
| SPI | ILI9341, ST7789 | 320×240 | 10–40 MHz | Graphics, fast updates |
| Parallel | (8/16-bit) | 480×320+ | 5–20 MHz | High-res graphics |

---

## Troubleshooting

### Build Failures

**Problem:** `startup.S: error: unknown directive`
- **Solution:** Check that the `arm-none-eabi-as` assembler version matches your toolchain. Ensure `-march=cortex-m7` is passed during assembly.

**Problem:** `undefined reference to 'reset_handler'`
- **Solution:** Verify `startup.S` is included in `build_src_filter` in `platformio.ini`.

### Display Not Detected

1. Check I2C/SPI pin connections
2. Verify display address matches expected table in `graphical_entry_detector.cpp`
3. Add debug output to `DETECT_SCRN()` function
4. Use an I2C scanner to verify address

### Linker Script Errors

**Problem:** `section `.bss' not found`
- **Solution:** Ensure GCC's linker (not an old ld.bfd) is in use. Update toolchain.

---

## Contributing

Contributions are welcome! Please:

1. Test on real hardware (i.MX RT1060 EVK or compatible)
2. Follow the existing code style (K&R for C, Google C++ for C++)
3. Add documentation for new features
4. Report issues with full reproduction steps

---

## License

MIT License — See [LICENSE](LICENSE) for details.

**Copyright © 2026** — FreeARDU Contributors

---

## Acknowledgments

- **Jibril** — Project author
- **NXP** — i.MX RT1060 technical reference & examples
- **ARM** — Cortex-M7 architecture documentation
- **PlatformIO** — Build and deployment infrastructure

---

## References

- [NXP i.MX RT1060 Reference Manual](https://www.nxp.com/webapp/Download?colCode=IMXRT1060RM)
- [ARM Cortex-M7 Processor Reference](https://developer.arm.com/products/architecture/m-profile)
- [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/gnu-toolchain)
- [PlatformIO Documentation](https://docs.platformio.org/)

---

## Quick Reference

```bash
# Clone and enter project
git clone https://github.com/Rolbubx/FreeARDU.git
cd FreeARDU/FreeARDU

# Build
platformio run -e nxp_imxrt1060_evk

# Upload
platformio run -e nxp_imxrt1060_evk --target upload

# Monitor serial (after upload)
platformio device monitor -b 115200
```


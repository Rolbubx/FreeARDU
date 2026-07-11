# FreeARDU

A bare-metal learning project for Cortex-M7 (NXP i.MX RT1060), with a custom startup sequence, linker script, and a small framebuffer layer to drive a display.

This is **not** a generic framework or a finished product: it's a personal project to understand how a microcontroller works "from scratch," with no OS or Arduino core underneath. Feedback and contributions are welcome, but expect the code to still be experimental.

## Status

🚧 **Actively in development.** Compilation may fail depending on your setup. Only tested on the NXP i.MX RT1060 EVK board.

## What this project actually does

- **Bare-metal startup** (`startup.S`): Cortex-M7 initialization in assembly — stack setup, copying `.data`, zeroing `.bss`, calling C++ global constructors. Supports either a `main()` entry point or an Arduino-style `setup()`/`loop()`.
- **Linker script** (`imxrt1060.ld`): memory layout for the i.MX RT1060 (ITCM, DTCM, OCRAM, external Flash). Specific to this chip — you'll need to adapt it for any other board.
- **Minimal framebuffer**: a small API to write pixels into a buffer and push it to a display (`INIT`, `CLEAR`, `PUSH_PIXEL`, `FLUSH`).
- **Screen detection** (`GraphicalEntryDetector`): tries to guess what type of display is connected by scanning known I2C addresses (OLED, LCD) or GPIO/SPI pins.

## What this project is NOT (yet)

To be upfront about the current limitations:

- **The I2C/SPI/GPIO drivers are stubs.** There's no real low-level peripheral implementation yet — just placeholders meant to be replaced with actual driver code.
- **Only one board is supported** (i.MX RT1060 EVK). Nothing is tested or guaranteed on other Cortex-M7 targets, despite the architecture being designed to be portable in theory.
- **Screen detection is basic**: fixed I2C address scanning, no real negotiation or robust error handling.
- **This is not comparable to CMSIS or a vendor HAL** (like ST's HAL): those layers are validated across dozens of chip families, with years of field feedback and real error handling. FreeARDU is a learning starting point, not an alternative to those tools.

## Project structure

```
FreeARDU/
├── README.md
├── platformio.ini
├── imxrt1060.ld
├── include/
├── src/
│   ├── on_start/
│   │   ├── boot/startup.S
│   │   └── entry_point/main.cpp
│   ├── framebuffer/
│   │   ├── framebuffer.h
│   │   └── framebuffer.cpp
│   ├── GraphicalEntryDetector/
│   │   ├── graphical_entry_detector.h
│   │   └── graphical_entry_detector.cpp
│   └── infos/INFOS.h
└── test/
```

## Prerequisites

- PlatformIO Core (or VS Code + PlatformIO extension)
- Arm GNU Toolchain (auto-installed by PlatformIO)
- A J-Link debugger
- An NXP i.MX RT1060 EVK board

## Build & flash

```bash
# Build
platformio run -e nxp_imxrt1060_evk

# Flash via J-Link
platformio run -e nxp_imxrt1060_evk --target upload

# Debug
platformio debug -e nxp_imxrt1060_evk
```

## Minimal example

```cpp
#include "../../framebuffer/framebuffer.h"

extern Framebuffer framebuffer;

extern "C" int main() {
    if (framebuffer.INIT() != 0) {
        while (1); // no display detected
    }

    Color black = {0.0f, 0.0f, 0.0f};
    framebuffer.CLEAR(black);
    framebuffer.FLUSH();

    Color red = {1.0f, 0.0f, 0.0f};
    framebuffer.PUSH_PIXEL({10, 10}, red);
    framebuffer.FLUSH();

    while (1) {}
}
```

## What's left to do 

- [ ] Replace the I2C/SPI/GPIO stubs with real low-level implementations
- [ ] Add real error handling (timeouts, retries) to screen detection
- [ ] Test on at least one second board to validate the claimed portability
- [ ] Add tests (the `test/` folder is currently empty)
- [ ] Document the registers and timings used in `startup.S` so it's genuinely educational, not just functional

## Why this project exists

This is mainly an exercise to understand, step by step, what happens between powering up a Cortex-M7 board and drawing the first pixel — without relying on an existing framework. If you're looking for something production-ready, look at CMSIS, ST's HAL, or a mature graphics framework like LVGL instead.

## License

MIT — see [LICENSE](LICENSE).

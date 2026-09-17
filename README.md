# FreeARDU

A bare-metal learning project for Cortex-M7 (NXP i.MX RT1060), with a custom startup sequence, linker script, and a small framebuffer layer to drive a display.

This is **not** a generic framework or a finished product: it's a personal project to understand how a microcontroller works "from scratch," with no OS or Arduino core underneath. Feedback and contributions are welcome, but expect the code to still be experimental.

> **What this project is :** a personal, in-progress learning exercise. It is **not** a real framework, not something meant to be adopted by other teams, and not something you should compare to mature platform tooling. If that's what you're looking for, see the "What this project is NOT (yet)" section below for direct pointers to the real thing.

## Status

🚧 **Actively in development.** Compilation may fail depending on your setup. Only tested on the NXP i.MX RT1060 EVK board.

## What this project actually does

- **Bare-metal startup** (`OnStart/Boot/Startup.S`): Cortex-M7 initialization in assembly — stack setup, copying `.data`, zeroing `.bss`, calling C++ global constructors. Supports either a `main()` entry point or an Arduino-style `setup()`/`loop()`.
- **Linker script** (`imxrt1060.ld`): memory layout for the i.MX RT1060 (ITCM, DTCM, OCRAM, external Flash). Specific to this chip — you'll need to adapt it for any other board.
- **Minimal framebuffer**: a small API to write pixels into a buffer and push it to a display (`INIT`, `CLEAR`, `PUSH_PIXEL`, `FLUSH`).
- **Display configuration** (`DisplayDetector`): selects the explicitly configured controller and wiring. Passive pin scanning is not used because it cannot identify a TFT reliably.
- **µGUI integration** (`src/ThirdParty/Ugui`): the project bundles [µGUI](http://www.embeddedlightning.com/) (Achim Döbler, 2015), a small third-party embedded GUI library, wired up to the framebuffer as a pixel-drawing backend. This is used for basic on-screen rendering (filling the screen, drawing frames/lines, putting text) — it is **someone else's library integrated into the project**, not something FreeARDU itself implements from scratch. Its original license is preserved and included separately (see `THIRD_PARTY_LICENSES.md` / `src/ThirdParty/Ugui/LICENSE.txt`).

## What this project is NOT (yet)

To be upfront about the current limitations:

- **The supported real display path is an ILI9341-compatible 320x240 SPI TFT.** It uses GPIO1 bit-banged SPI with explicit CS/DC/RST control and RGB565 frame transfers. The wiring is configured in `FreeARDU/src/Infos/Infos.h`; other controllers and parallel panels still need their own drivers.
- **Only one board is supported** (i.MX RT1060 EVK). Nothing is tested or guaranteed on other Cortex-M7 targets, despite the architecture being designed to be portable in theory.
- **Physical validation is still required** on the target panel and wiring; the firmware build alone cannot validate electrical connections.
- **This is not comparable to CMSIS or a vendor HAL** (like ST's HAL): those layers are validated across dozens of chip families, with years of field feedback and real error handling. FreeARDU is a learning starting point, not an alternative to those tools.
- **This is not a framework, and it is not trying to become one.** A framework implies a stable, general-purpose API meant for other teams and projects to build on top of — that requires years of iteration, real-world usage, and broad hardware validation. FreeARDU doesn't have that, doesn't claim to have it, and isn't attempting to shortcut its way there. If you came here expecting something you could drop into a production project, this isn't it — and it isn't trying to pretend otherwise.

## Project structure

Think of the repo in three layers: **build/tools at the root**, **firmware under `FreeARDU/`**, and **source modules under `FreeARDU/src/`**.

```
FreeARDU_OT_ACCNT/
│
├── platformio.ini              # PlatformIO build / flash / debug
├── platformio_post.py          # Post-build hook
├── build.sh / build.bat        # Convenience build scripts
├── Renode.* / run_*.resc       # Emulator (Renode) scripts
├── LICENSE
├── README.md                   # You are here
│
└── FreeARDU/                   # Firmware package
    ├── imxrt1060.ld            # Memory map (linker script)
    ├── include/                # Extra public headers (if any)
    ├── test/                   # Unit tests (empty for now)
    └── src/                    # All firmware source
        │
        │ ── Boot ──────────────────────────────────────────
        ├── OnStart/
        │   ├── Boot/           # Startup.S, UpdMode.S (CPU bring-up)
        │   └── EntryPoint/     # Main.c (application entry)
        │
        │ ── Core runtime ──────────────────────────────────
        ├── Bios/               # Early board / BIOS-style init
        ├── Api/                # High-level FreeARDU API
        ├── Process/            # Process / task helpers
        ├── Command/            # Shell-style command handling
        ├── Panic/              # Panic / fault reporting (C++)
        ├── HardFault/          # HardFaultHandler.S (fault entry)
        │
        │ ── Hardware I/O ──────────────────────────────────
        ├── Hardware/           # GPIO (bare-metal stubs)
        ├── Uart/               # UART putchar output
        ├── Infos/              # Board / build info (Infos.h)
        │
        │ ── Graphics ──────────────────────────────────────
        ├── Framebuffer/        # Pixel buffer + flush API
        ├── DisplayDetector/    # Guess connected display type
        └── ThirdParty/
            └── Ugui/           # µGUI (external library)
```

| Folder | Role in one line |
|--------|------------------|
| `OnStart/` | Power-on → `main()` |
| `Bios/` + `Api/` | Init and public API surface |
| `Process/` + `Command/` | Runtime / command layer |
| `Panic/` + `HardFault/` | Crash / HardFault path |
| `Hardware/` + `Uart/` | Low-level I/O |
| `Framebuffer/` + `DisplayDetector/` | Drawing + display probe |
| `ThirdParty/Ugui/` | Bundled GUI library (not FreeARDU code) |

## Prerequisites

- PlatformIO Core (or VS Code + PlatformIO extension)
- Arm GNU Toolchain (auto-installed by PlatformIO)
- A J-Link debugger
- An NXP i.MX RT1060 EVK board

## Build & flash

```bash
# Interactive Windows CLI
FreeARDU.bat

# Compile
FreeARDU.bat build debug
FreeARDU.bat build release

# Compile and run in Renode
FreeARDU.bat build-run release

# Flash via J-Link
FreeARDU.bat upload release

# Debug
FreeARDU.bat debug debug
```

The CLI also provides clean, run-existing-firmware, and serial-monitor commands:

```bash
FreeARDU.bat clean debug
FreeARDU.bat run release
FreeARDU.bat monitor

```

## Minimal example

```cpp
#include "../../Framebuffer/Framebuffer.h"

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

- [x] Add a real GPIO/SPI TFT initialization and framebuffer flush path
- [ ] Validate the configured pin wiring and µGUI rendering on the target physical display
- [ ] Add real error handling (timeouts, retries) to screen detection
- [ ] Test on at least one second board to validate the claimed portability
- [ ] Add tests (the `test/` folder is currently empty)
- [ ] Document the registers and timings used in `Startup.S` so it's genuinely educational, not just functional

## Why this project exists

This is mainly an exercise to understand, step by step, what happens between powering up a Cortex-M7 board and drawing the first pixel — without relying on an existing framework. If you're looking for something production-ready, look at CMSIS, ST's HAL, or a mature graphics framework like LVGL instead.

## License

MIT for FreeARDU's own code — see [LICENSE](LICENSE).

This project bundles third-party software (µGUI by Achim Döbler) under its own separate license terms — see  `src/ThirdParty/Ugui/LICENSE.txt`.

#include "gpio_bare_metal.h"
#include <stdint.h>

#ifdef FREEARDU_BARE_METAL

// ============================================================
// i.MX RT1060 Register Definitions
// ============================================================

// CCM (Clock Control Module) - for enabling GPIO clocks
#define CCM_BASE_ADDR       0x401F4000
#define CCM_CCGR1           (*(volatile uint32_t*)(CCM_BASE_ADDR + 0x0848))  // GPIO1-2 clock gate
#define CCM_CCGR2           (*(volatile uint32_t*)(CCM_BASE_ADDR + 0x084C))  // GPIO3-4 clock gate
#define CCM_CCGR3           (*(volatile uint32_t*)(CCM_BASE_ADDR + 0x0850))  // GPIO5 clock gate

// IOMUXC - for pin muxing
#define IOMUXC_BASE_ADDR    0x401F8000
#define IOMUXC_GPR1         (*(volatile uint32_t*)(IOMUXC_BASE_ADDR + 0x0040))
#define IOMUXC_GPR10        (*(volatile uint32_t*)(IOMUXC_BASE_ADDR + 0x0078))

// GPIO register bases
#define GPIO1_BASE_ADDR     0x401B8000
#define GPIO2_BASE_ADDR     0x401BC000

// GPIO register offsets
#define GPIO_GDIR_OFFSET    0x04
#define GPIO_PDIR_OFFSET    0x00
#define GPIO_IOx_DR_OFFSET  0x00  // Data register
#define GPIO_IOx_GDR_OFFSET 0x00  // GDIR (same offset)

// Pad Control register base (per GPIO bank)
#define IOMUXC_SW_PAD_CTL_GPIO1_BASE  0x0380
#define IOMUXC_SW_MUX_CTL_GPIO1_BASE  0x0240

// ============================================================
// GPIO Register Access Macros
// ============================================================
#define GPIO1_GDIR (*(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_GDIR_OFFSET))
#define GPIO1_PDIR (*(volatile uint32_t*)(GPIO1_BASE_ADDR + GPIO_PDIR_OFFSET))

#define GPIO2_GDIR (*(volatile uint32_t*)(GPIO2_BASE_ADDR + GPIO_GDIR_OFFSET))
#define GPIO2_PDIR (*(volatile uint32_t*)(GPIO2_BASE_ADDR + GPIO_PDIR_OFFSET))

// ============================================================
// Pin Mapping for MIMXRT1060 EVK Arduino Header
// Based on EVK schematic: Arduino pins -> AdB pins -> GPIO
// ============================================================
// Arduino Digital Pins on MIMXRT1060 EVK:
// Pin 0-7:  GPIO1_IO00-GPIO1_IO07 (J19/Arduino header)
// Pin 8-11: GPIO1_IO08-GPIO1_IO11
// Pin 12-13: GPIO1_IO12-GPIO1_IO13
// Pin 14-15: GPIO2_IO00-GPIO2_IO01 (different bank)
// Pin 16-17: GPIO1_IO14-GPIO1_IO15
// Pin 18-19: GPIO1_IO16-GPIO1_IO17

// Combined mapping: Arduino pin -> GPIO bank and pin
typedef struct {
    volatile uint32_t* gdir_reg;
    volatile uint32_t* pdir_reg;
    uint32_t mask;
} PinInfo;

// Static pin info array
static const PinInfo pinInfo[] = {
    // Arduino pins 0-17 (common SPI/Parallel display pins used in your code)
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  0) },  // Pin 0
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  1) },  // Pin 1
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  2) },  // Pin 2
    { &GPIO1_GDIR, &GPIO1_GDIR, (1U <<  3) },  // Pin 3
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  4) },  // Pin 4
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  5) },  // Pin 5
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  6) },  // Pin 6
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  7) },  // Pin 7
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  8) },  // Pin 8
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U <<  9) },  // Pin 9
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 10) },  // Pin 10
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 11) },  // Pin 11
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 12) },  // Pin 12
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 13) },  // Pin 13
    { &GPIO2_GDIR, &GPIO2_PDIR, (1U <<  0) },  // Pin 14 (GPIO2_IO00)
    { &GPIO2_GDIR, &GPIO2_PDIR, (1U <<  1) },  // Pin 15 (GPIO2_IO01)
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 14) },  // Pin 16
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 15) },  // Pin 17
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 16) },  // Pin 18
    { &GPIO1_GDIR, &GPIO1_PDIR, (1U << 17) },  // Pin 19
};

// ============================================================
// IOMUXC Configuration
// ============================================================

// Configure a pin for GPIO input with pull-down
static void configure_gpio_input_pull_down(int arduino_pin) {
    if (arduino_pin < 0 || arduino_pin >= (int)(sizeof(pinInfo)/sizeof(pinInfo[0]))) {
        return;
    }
    
    const PinInfo* info = &pinInfo[arduino_pin];
    
    // 1. Configure as GPIO (ALT5 = GPIO) via IOMUXC mux control
    // For GPIO1: SW_MUX_CTL_GPIO1_IO_XX registers at offset 0x0240 + (pin * 4)
    // For GPIO2: SW_MUX_CTL_GPIO2_IO_XX registers at offset 0x0280 + (pin * 4)
    
    volatile uint32_t* mux_ctrl;
    uint32_t gpio_pin;
    
    if (info->pdir_reg == &GPIO1_PDIR) {
        // GPIO1: pins 0-31
        gpio_pin = __builtin_ctz(info->mask);
        mux_ctrl = (volatile uint32_t*)(IOMUXC_BASE_ADDR + IOMUXC_SW_MUX_CTL_GPIO1_BASE + (gpio_pin * 4));
    } else {
        // GPIO2: pins 0-31
        gpio_pin = __builtin_ctz(info->mask);
        mux_ctrl = (volatile uint32_t*)(IOMUXC_BASE_ADDR + 0x0280 + (gpio_pin * 4));
    }
    
    // Set ALT mode to 5 (GPIO)
    *mux_ctrl = 5;  // IOMUXC_SW_MUX_CTL_PAD_GPIO1_IO00_ALT5 = GPIO1_IO00
    
    // 2. Configure pad control for pull-down
    // SW_PAD_CTL_GPIO1_IO_XX registers at offset 0x0380 + (pin * 4)
    volatile uint32_t* pad_ctrl;
    if (info->pdir_reg == &GPIO1_PDIR) {
        pad_ctrl = (volatile uint32_t*)(IOMUXC_BASE_ADDR + IOMUXC_SW_PAD_CTL_GPIO1_BASE + (gpio_pin * 4));
    } else {
        pad_ctrl = (volatile uint32_t*)(IOMUXC_BASE_ADDR + 0x03C0 + (gpio_pin * 4));
    }
    
    // Set pull-down (0x0A = 100K pull-down, pull/keeper enabled)
    *pad_ctrl = 0x00A0;  // PUS(2:0) = 011 -> 100K pull-down
}

// ============================================================
// GPIO Functions
// ============================================================

void gpio_init(void) {
    // Enable GPIO1 and GPIO2 clocks via CCM
    // CCGR1: CG13 = GPIO1, CG14 = GPIO2, CG15 = GPIO3
    // Set to 0x3 (always on) for both GPIO1 and GPIO2
    CCM_CCGR1 |= (3 << 12) | (3 << 14);  // Enable GPIO1 & GPIO2
    
    // Configure all pins we'll be checking as inputs with pull-down
    // This prevents floating inputs from giving false readings
    for (int i = 0; i < (int)(sizeof(pinInfo)/sizeof(pinInfo[0])); i++) {
        configure_gpio_input_pull_down(i);
    }
}

bool isPinConnected(int pin) {
    if (pin < 0 || pin >= (int)(sizeof(pinInfo)/sizeof(pinInfo[0]))) {
        return false;
    }
    
    const PinInfo* info = &pinInfo[pin];
    
    // Set pin as input (clear GDIR bit)
    *info->gdir_reg &= ~info->mask;
    
    // Small delay for signal stabilization
    for(volatile int i = 0; i < 1000; i++);
    
    // Read pin state
    // If the pin reads HIGH with pull-down, something is pulling it up
    // This indicates a connected device (SPI display with pull-up, etc.)
    return (*info->pdir_reg & info->mask) != 0;
}

bool i2c_probe_address(uint8_t addr) {
    // For I2C probing, we'd need to configure I2C pins and send a probe
    // This is a placeholder - real implementation would use:
    // - LPI2C peripheral registers
    // - Or bit-banged GPIO on SCL/SDA pins
    
    // Common display I2C addresses for detection:
    // 0x3C, 0x3D - SSD1306 OLED
    // 0x38 - Some TFT displays
    // 0x44 - Some color displays
    // 0x50, 0x51 - Some LCDs
    
    (void)addr;  // Suppress unused warning
    return false;
}

#endif // FREEARDU_BARE_METAL
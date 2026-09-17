//
// In this script, you need to define everything that you need (your pins ect)
//

#ifndef FREEARDU_INFOS_H
#define FREEARDU_INFOS_H

#include <stdbool.h>

/*
 * Set to true before the kernel enters its terminal stopping path.
 * A stopping hook may be registered through memory.h.
 */
#ifdef __cplusplus
extern "C" {
#endif
extern volatile bool isKernelStopping;
#ifdef __cplusplus
}
#endif

// Enable or disable printing pixels to UART when no screen is detected
#define FREEARDU_PIXEL_DEBUG_UART true

// A passive GPIO scan cannot reliably identify a display. Configure the
// controller and wiring explicitly instead.
// 0: no display, 1: SPI TFT, 2: parallel TFT
#define FREEARDU_FORCE_SCREEN_TYPE 0

// The SPI implementation targets ILI9341-compatible 320x240 controllers.
#define FREEARDU_DISPLAY_WIDTH  320
#define FREEARDU_DISPLAY_HEIGHT 240

// Hardware Pin Definitions (i.MX RT1060 LPSPI4)
#define LPSPI4_SCK_PIN  13
#define LPSPI4_MOSI_PIN 11
#define LPSPI4_MISO_PIN 12
#define LPSPI4_CS_PIN   10
#define LPSPI4_DC_PIN   9
#define LPSPI4_RST_PIN  8

// version



#define VERSION         "1.0.2"
#define ChangeLog       " Updated Infos.h to include version and build date information."
#define BUILD_DATE      __DATE__


#define TO_STRING(x)    #x
#define STRINGIFY(x)    TO_STRING(x)
#define VERSION_COMPILE STRINGIFY(__GNUC__)


#define END VERSION ChangeLog BUILD_DATE " (GCC " VERSION_COMPILE ")"


#endif //FREEARDU_INFOS_H

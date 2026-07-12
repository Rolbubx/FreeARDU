//
// In this script, you need to define everything that you need (your pins ect)
//

#ifndef FREEARDU_INFOS_H
#define FREEARDU_INFOS_H

// Enable or disable printing pixels to UART when no screen is detected
#define FREEARDU_PIXEL_DEBUG_UART false

// Force a specific screen type for bare-metal testing if detection fails
// 0: NONE, 1: SPI, 2: PARALLEL
#define FREEARDU_FORCE_SCREEN_TYPE 0

// Hardware Pin Definitions (i.MX RT1060 LPSPI4)
#define LPSPI4_SCK_PIN  13
#define LPSPI4_MOSI_PIN 11
#define LPSPI4_MISO_PIN 12
#define LPSPI4_CS_PIN   10
#define LPSPI4_DC_PIN   9
#define LPSPI4_RST_PIN  8

#endif //FREEARDU_INFOS_H

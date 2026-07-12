#ifndef __UGUI_CONFIG_H
#define __UGUI_CONFIG_H

/* -------------------------------------------------------------------------------- */
/* -- µGUI - Configuration --                                                    -- */
/* -------------------------------------------------------------------------------- */

/* Color schema */
/* Choose the color schema for your display */
#define USE_COLOR_RGB888   /* 24-bit RGB */
/* #define USE_COLOR_RGB565 */

/* Feature settings */
#define USE_PRERENDER_EVENT
#define USE_POSTRENDER_EVENT

/* Screen size */
/* These will be overridden by the driver initialization in FreeARDU */
#define USE_CONTROLS

#endif

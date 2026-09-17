#ifndef FREEARDU_FRAMEBUFFER_API_H
#define FREEARDU_FRAMEBUFFER_API_H

#ifdef __cplusplus
extern "C" {
#endif

int display_init(void);
int display_flush(void);
int display_get_width(void);
int display_get_height(void);

#ifdef __cplusplus
}
#endif

#endif

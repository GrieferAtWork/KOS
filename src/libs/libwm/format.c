/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_LIBS_LIBWM_SURFACE_C
#define GUARD_LIBS_LIBWM_SURFACE_C 1
#define _EXCEPT_SOURCE 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/atomic.h>
#include <hybrid/align.h>
#include <wm/api.h>
#include <wm/server.h>
#include <wm/window.h>
#include <assert.h>
#include <unistd.h>
#include <except.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <malloc.h>

#include "libwm.h"

DECL_BEGIN

INTERN struct PACKED {
    ATOMIC_DATA __ref_t          p_refcnt;    /* Palette reference counter. */
    __uint8_t                    p_bpp;       /* [>= 1 && <= 16] Bits per pixel (power-of-2). */
    __uint8_t                    p_mask;      /* Per-color channel mask of effective color bits.
                                               * Bits not masked by this must be set to ZERO. */
    __uint16_t                   p_count;     /* [2...2^16][== 1 << p_bpp] Number of colors in the palette. */
    struct wm_color_triple       p_colors[256]; /* [p_count] Colors associated with individual palette indices. */
} libwm_palette_256_ ASMNAME("libwm_palette_256") = {
    .p_refcnt = 0x3fffffff,
    .p_bpp    = 8,
    .p_mask   = 0x3f,
    .p_count  = 256,
    .p_colors = {
        {0x00,0x00,0x00},{0x00,0x00,0x2a},{0x00,0x2a,0x00},{0x00,0x2a,0x2a},{0x2a,0x00,0x00},{0x2a,0x00,0x2a},{0x2a,0x15,0x00},{0x2a,0x2a,0x2a},
        {0x15,0x15,0x15},{0x15,0x15,0x3f},{0x15,0x3f,0x15},{0x15,0x3f,0x3f},{0x3f,0x15,0x15},{0x3f,0x15,0x3f},{0x3f,0x3f,0x15},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x00},{0x05,0x05,0x05},{0x08,0x08,0x08},{0x0b,0x0b,0x0b},{0x0e,0x0e,0x0e},{0x11,0x11,0x11},{0x14,0x14,0x14},{0x18,0x18,0x18},
        {0x1c,0x1c,0x1c},{0x20,0x20,0x20},{0x24,0x24,0x24},{0x28,0x28,0x28},{0x2d,0x2d,0x2d},{0x32,0x32,0x32},{0x38,0x38,0x38},{0x3f,0x3f,0x3f},
        {0x00,0x00,0x3f},{0x10,0x00,0x3f},{0x1f,0x00,0x3f},{0x2f,0x00,0x3f},{0x3f,0x00,0x3f},{0x3f,0x00,0x2f},{0x3f,0x00,0x1f},{0x3f,0x00,0x10},
        {0x3f,0x00,0x00},{0x3f,0x10,0x00},{0x3f,0x1f,0x00},{0x3f,0x2f,0x00},{0x3f,0x3f,0x00},{0x2f,0x3f,0x00},{0x1f,0x3f,0x00},{0x10,0x3f,0x00},
        {0x00,0x3f,0x00},{0x00,0x3f,0x10},{0x00,0x3f,0x1f},{0x00,0x3f,0x2f},{0x00,0x3f,0x3f},{0x00,0x2f,0x3f},{0x00,0x1f,0x3f},{0x00,0x10,0x3f},
        {0x1f,0x1f,0x3f},{0x27,0x1f,0x3f},{0x2f,0x1f,0x3f},{0x37,0x1f,0x3f},{0x3f,0x1f,0x3f},{0x3f,0x1f,0x37},{0x3f,0x1f,0x2f},{0x3f,0x1f,0x27},
        {0x3f,0x1f,0x1f},{0x3f,0x27,0x1f},{0x3f,0x2f,0x1f},{0x3f,0x37,0x1f},{0x3f,0x3f,0x1f},{0x37,0x3f,0x1f},{0x2f,0x3f,0x1f},{0x27,0x3f,0x1f},
        {0x1f,0x3f,0x1f},{0x1f,0x3f,0x27},{0x1f,0x3f,0x2f},{0x1f,0x3f,0x37},{0x1f,0x3f,0x3f},{0x1f,0x37,0x3f},{0x1f,0x2f,0x3f},{0x1f,0x27,0x3f},
        {0x2d,0x2d,0x3f},{0x31,0x2d,0x3f},{0x36,0x2d,0x3f},{0x3a,0x2d,0x3f},{0x3f,0x2d,0x3f},{0x3f,0x2d,0x3a},{0x3f,0x2d,0x36},{0x3f,0x2d,0x31},
        {0x3f,0x2d,0x2d},{0x3f,0x31,0x2d},{0x3f,0x36,0x2d},{0x3f,0x3a,0x2d},{0x3f,0x3f,0x2d},{0x3a,0x3f,0x2d},{0x36,0x3f,0x2d},{0x31,0x3f,0x2d},
        {0x2d,0x3f,0x2d},{0x2d,0x3f,0x31},{0x2d,0x3f,0x36},{0x2d,0x3f,0x3a},{0x2d,0x3f,0x3f},{0x2d,0x3a,0x3f},{0x2d,0x36,0x3f},{0x2d,0x31,0x3f},
        {0x00,0x00,0x1c},{0x07,0x00,0x1c},{0x0e,0x00,0x1c},{0x15,0x00,0x1c},{0x1c,0x00,0x1c},{0x1c,0x00,0x15},{0x1c,0x00,0x0e},{0x1c,0x00,0x07},
        {0x1c,0x00,0x00},{0x1c,0x07,0x00},{0x1c,0x0e,0x00},{0x1c,0x15,0x00},{0x1c,0x1c,0x00},{0x15,0x1c,0x00},{0x0e,0x1c,0x00},{0x07,0x1c,0x00},
        {0x00,0x1c,0x00},{0x00,0x1c,0x07},{0x00,0x1c,0x0e},{0x00,0x1c,0x15},{0x00,0x1c,0x1c},{0x00,0x15,0x1c},{0x00,0x0e,0x1c},{0x00,0x07,0x1c},
        {0x0e,0x0e,0x1c},{0x11,0x0e,0x1c},{0x15,0x0e,0x1c},{0x18,0x0e,0x1c},{0x1c,0x0e,0x1c},{0x1c,0x0e,0x18},{0x1c,0x0e,0x15},{0x1c,0x0e,0x11},
        {0x1c,0x0e,0x0e},{0x1c,0x11,0x0e},{0x1c,0x15,0x0e},{0x1c,0x18,0x0e},{0x1c,0x1c,0x0e},{0x18,0x1c,0x0e},{0x15,0x1c,0x0e},{0x11,0x1c,0x0e},
        {0x0e,0x1c,0x0e},{0x0e,0x1c,0x11},{0x0e,0x1c,0x15},{0x0e,0x1c,0x18},{0x0e,0x1c,0x1c},{0x0e,0x18,0x1c},{0x0e,0x15,0x1c},{0x0e,0x11,0x1c},
        {0x14,0x14,0x1c},{0x16,0x14,0x1c},{0x18,0x14,0x1c},{0x1a,0x14,0x1c},{0x1c,0x14,0x1c},{0x1c,0x14,0x1a},{0x1c,0x14,0x18},{0x1c,0x14,0x16},
        {0x1c,0x14,0x14},{0x1c,0x16,0x14},{0x1c,0x18,0x14},{0x1c,0x1a,0x14},{0x1c,0x1c,0x14},{0x1a,0x1c,0x14},{0x18,0x1c,0x14},{0x16,0x1c,0x14},
        {0x14,0x1c,0x14},{0x14,0x1c,0x16},{0x14,0x1c,0x18},{0x14,0x1c,0x1a},{0x14,0x1c,0x1c},{0x14,0x1a,0x1c},{0x14,0x18,0x1c},{0x14,0x16,0x1c},
        {0x00,0x00,0x10},{0x04,0x00,0x10},{0x08,0x00,0x10},{0x0c,0x00,0x10},{0x10,0x00,0x10},{0x10,0x00,0x0c},{0x10,0x00,0x08},{0x10,0x00,0x04},
        {0x10,0x00,0x00},{0x10,0x04,0x00},{0x10,0x08,0x00},{0x10,0x0c,0x00},{0x10,0x10,0x00},{0x0c,0x10,0x00},{0x08,0x10,0x00},{0x04,0x10,0x00},
        {0x00,0x10,0x00},{0x00,0x10,0x04},{0x00,0x10,0x08},{0x00,0x10,0x0c},{0x00,0x10,0x10},{0x00,0x0c,0x10},{0x00,0x08,0x10},{0x00,0x04,0x10},
        {0x08,0x08,0x10},{0x0a,0x08,0x10},{0x0c,0x08,0x10},{0x0e,0x08,0x10},{0x10,0x08,0x10},{0x10,0x08,0x0e},{0x10,0x08,0x0c},{0x10,0x08,0x0a},
        {0x10,0x08,0x08},{0x10,0x0a,0x08},{0x10,0x0c,0x08},{0x10,0x0e,0x08},{0x10,0x10,0x08},{0x0e,0x10,0x08},{0x0c,0x10,0x08},{0x0a,0x10,0x08},
        {0x08,0x10,0x08},{0x08,0x10,0x0a},{0x08,0x10,0x0c},{0x08,0x10,0x0e},{0x08,0x10,0x10},{0x08,0x0e,0x10},{0x08,0x0c,0x10},{0x08,0x0a,0x10},
        {0x0b,0x0b,0x10},{0x0c,0x0b,0x10},{0x0d,0x0b,0x10},{0x0f,0x0b,0x10},{0x10,0x0b,0x10},{0x10,0x0b,0x0f},{0x10,0x0b,0x0d},{0x10,0x0b,0x0c},
        {0x10,0x0b,0x0b},{0x10,0x0c,0x0b},{0x10,0x0d,0x0b},{0x10,0x0f,0x0b},{0x10,0x10,0x0b},{0x0f,0x10,0x0b},{0x0d,0x10,0x0b},{0x0c,0x10,0x0b},
        {0x0b,0x10,0x0b},{0x0b,0x10,0x0c},{0x0b,0x10,0x0d},{0x0b,0x10,0x0f},{0x0b,0x10,0x10},{0x0b,0x0f,0x10},{0x0b,0x0d,0x10},{0x0b,0x0c,0x10},
        {0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00},{0x00,0x00,0x00}
    }
};



PRIVATE wm_pixel_t WMCALL default_surface_getpixel(struct wm_surface const *__restrict self, int x, int y);
PRIVATE void WMCALL default_surface_setpixel(struct wm_surface *__restrict self, int x, int y, wm_pixel_t pixel);
PRIVATE void WMCALL default_surface_hline(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, wm_pixel_t pixel);
PRIVATE void WMCALL default_surface_vline(struct wm_surface *__restrict self, int x, int y, unsigned int size_y, wm_pixel_t pixel);
PRIVATE void WMCALL default_surface_rect(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);
PRIVATE void WMCALL default_surface_fill(struct wm_surface *__restrict self, int x, int y, unsigned int size_x, unsigned int size_y, wm_pixel_t pixel);
PRIVATE void WMCALL
default_surface_bblit(struct wm_surface *__restrict self, int x, int y,
                      struct wm_surface const *__restrict source,
                      unsigned int source_x, unsigned int source_y,
                      unsigned int size_x, unsigned int size_y);
PRIVATE void WMCALL
default_surface_cblit(struct wm_surface *__restrict self, int x, int y,
                      struct wm_surface const *__restrict source,
                      unsigned int source_x, unsigned int source_y,
                      unsigned int size_x, unsigned int size_y,
                      wm_pixel_t color_key);



PRIVATE struct wm_surface_ops default_surface_ops = {
    .so_getpixel = &default_surface_getpixel,
    .so_setpixel = &default_surface_setpixel,
    .so_hline    = &default_surface_hline,
    .so_vline    = &default_surface_vline,
    .so_rect     = &default_surface_rect,
    .so_fill     = &default_surface_fill,
    .so_bblit    = &default_surface_bblit,
    .so_cblit    = &default_surface_cblit
};

INTERN ATTR_NOTHROW ATTR_RETNONNULL struct wm_surface_ops const *
WMCALL libwm_lookup_surface_ops(unsigned int bpp) {
 /* TODO: Optimizations for bpp=8,16 and 32 */
 return &default_surface_ops;
}




PRIVATE wm_pixel_t WMCALL
default_surface_getpixel(struct wm_surface const *__restrict self,
                         int x, int y) {
 /* TODO */
 return 0;
}

PRIVATE void WMCALL
default_surface_setpixel(struct wm_surface *__restrict self,
                         int x, int y, wm_pixel_t pixel) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_hline(struct wm_surface *__restrict self,
                      int x, int y, unsigned int size_x,
                      wm_pixel_t pixel) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_vline(struct wm_surface *__restrict self,
                      int x, int y, unsigned int size_y,
                      wm_pixel_t pixel) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_rect(struct wm_surface *__restrict self,
                     int x, int y, unsigned int size_x,
                     unsigned int size_y, wm_pixel_t pixel) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_fill(struct wm_surface *__restrict self,
                     int x, int y, unsigned int size_x,
                     unsigned int size_y, wm_pixel_t pixel) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_bblit(struct wm_surface *__restrict self, int x, int y,
                      struct wm_surface const *__restrict source,
                      unsigned int source_x, unsigned int source_y,
                      unsigned int size_x, unsigned int size_y) {
 /* TODO */
}

PRIVATE void WMCALL
default_surface_cblit(struct wm_surface *__restrict self, int x, int y,
                      struct wm_surface const *__restrict source,
                      unsigned int source_x, unsigned int source_y,
                      unsigned int size_x, unsigned int size_y,
                      wm_pixel_t color_key) {
 /* TODO */
}


DECL_END

#endif /* !GUARD_LIBS_LIBWM_SURFACE_C */

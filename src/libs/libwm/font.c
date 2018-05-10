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
#ifndef GUARD_LIBS_LIBWM_FONT_C
#define GUARD_LIBS_LIBWM_FONT_C 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <wm/api.h>
#include <wm/font.h>
#include <wm/surface.h>

#include "libwm.h"

DECL_BEGIN

DEFINE_PUBLIC_ALIAS(wm_font_draw,libwm_font_draw);
INTERN void WMCALL
libwm_font_draw(struct wm_font const *__restrict self,
                char const *__restrict text, size_t textlen,
                struct wm_surface *__restrict dst, int x, int y,
                struct wm_text_state *state, unsigned int flags) {
 struct wm_text_state st;
 int start_x = x;
 if (state) {
  st = *state;
 } else {
  st = self->f_default;
  assert(st.f_color < WM_COLOR_COUNT);
  assert(st.f_shadow < WM_COLOR_COUNT);
  st.f_color  = dst->s_format->f_color[st.f_color];
  st.f_shadow = dst->s_format->f_color[st.f_shadow];
 }
 for (; textlen; --textlen) {
  unsigned char chr = (unsigned char)*text++;
  switch (chr) {

  case '\r':
#if 0 /* We should be able to reasonably allow this... */
   if (flags & WM_FONT_DRAW_FSINGLE)
       goto draw_generic;
#endif
   x = start_x;
   break;

  case '\n':
   if (flags & WM_FONT_DRAW_FSINGLE)
       goto draw_generic;
   x  = start_x;
   y += self->f_lnsiz;
   break;

  case '\t':
   assert(self->f_tabsize != 0);
   x = start_x + (((x - start_x) + (self->f_tabsize - 1)) / self->f_tabsize) * self->f_tabsize;
   break;

   /* TODO: All the other special characters. */

  {
   unsigned int char_x;
   unsigned int char_y;
  default:
draw_generic:
   if (chr > 127) chr = '?';
   /* Draw the character. */
   char_x = (chr % 16) << self->f_log2_chsizx;
   char_y = (chr / 16) << self->f_log2_chsizy;
   if (st.f_shadow_x || st.f_shadow_y) {
    wm_surface_ccblit(dst,
                      x+st.f_shadow_x,
                      y+st.f_shadow_y,
                      self,char_x,char_y,
                      self->f_ascii[chr],
                      self->f_chsizy,
                      0,st.f_shadow);
   }
   wm_surface_ccblit(dst,x,y,
                     self,char_x,char_y,
                     self->f_ascii[chr],
                     self->f_chsizy,
                     0,st.f_color);
   x += self->f_ascii[chr];
  } break;
  }
 }
 if (state) *state = st;
}




/*[[[deemon
#include <gfx>
#include <file>
#include <util>
#include <sys>
#include <fs>

fs.chdir(fs.path.head(__FILE__));
local sfx = gfx.surface_rgba8888("font/histd_8x8.png");
print "#define DEFAULT_FONT_PIXEL_DATA \\";
print "    ",;
for (local chy: util.range(64)) {
    for (local chx: util.range(128/8)) {
        local byte = 0x00;
        for (local x: util.range(8)) {
            local px = sfx[chx*8+x:chy];
            if (px.r || px.g || px.b)
                byte = byte | (1 << x);
        }
        print "0x%.2I8x," % byte,;
    }
    print " \\",;
    if (chy != 63) print "\n    ",;
}
print;
print "/" "**" "/";
print "#define DEFAULT_FONT_SPRITE_DATA \\";
print "    ",;
for (local chy: util.range(8)) {
    for (local chx: util.range(16)) {
        local char_sizex = 8;
        while (char_sizex) {
            for (local y: util.range(8)) {
                local px = sfx[chx*8+char_sizex-1:chy*8+y];
                if (px.r || px.g || px.b)
                    goto got_char_size_x;
            }
            char_sizex = char_sizex - 1;
        }
got_char_size_x:
        if (!char_sizex)
             char_sizex = 3; // Space characters
        print "%u," % char_sizex,;
    }
    print " \\",;
    if (chy != 7) print "\n    ",;
}
print;
print "/" "**" "/";

]]]*/
#define DEFAULT_FONT_PIXEL_DATA \
    0x00,0x7e,0x7e,0x36,0x10,0x38,0x08,0x00,0xff,0x00,0xff,0xf0,0x3c,0xfc,0xfe,0x18, \
    0x00,0x81,0xff,0x7f,0x38,0x38,0x08,0x00,0xff,0x3c,0xc3,0xe0,0x66,0xcc,0xc6,0xdb, \
    0x00,0xa5,0xdb,0x7f,0x7c,0x38,0x1c,0x18,0xe7,0x66,0x99,0xf0,0x66,0xfc,0xfe,0x3c, \
    0x00,0x81,0xff,0x7f,0xfe,0xfe,0x3e,0x3c,0xc3,0x42,0xbd,0xbe,0x66,0x0c,0xc6,0xe7, \
    0x00,0xbd,0xc3,0x3e,0x7c,0xfe,0x7f,0x3c,0xc3,0x42,0xbd,0x33,0x3c,0x0c,0xc6,0xe7, \
    0x00,0x99,0xe7,0x1c,0x38,0xd6,0x3e,0x18,0xe7,0x66,0x99,0x33,0x18,0x0e,0xe6,0x3c, \
    0x00,0x81,0xff,0x08,0x10,0x10,0x08,0x00,0xff,0x3c,0xc3,0x33,0x7e,0x0f,0x67,0xdb, \
    0x00,0x7e,0x7e,0x00,0x00,0x38,0x1c,0x00,0xff,0x00,0xff,0x1e,0x18,0x07,0x03,0x18, \
    0x01,0x40,0x18,0x66,0xfe,0x7c,0x00,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00, \
    0x07,0x70,0x3c,0x66,0xdb,0xc6,0x00,0x3c,0x3c,0x18,0x18,0x0c,0x00,0x24,0x18,0xff, \
    0x1f,0x7c,0x7e,0x66,0xdb,0x1c,0x00,0x7e,0x7e,0x18,0x30,0x06,0x03,0x66,0x3c,0xff, \
    0x7f,0x7f,0x18,0x66,0xde,0x36,0x00,0x18,0x18,0x18,0x7f,0x7f,0x03,0xff,0x7e,0x7e, \
    0x1f,0x7c,0x18,0x66,0xd8,0x36,0x7e,0x7e,0x18,0x7e,0x30,0x06,0x03,0x66,0xff,0x3c, \
    0x07,0x70,0x7e,0x00,0xd8,0x1c,0x7e,0x3c,0x18,0x3c,0x18,0x0c,0x7f,0x24,0xff,0x18, \
    0x01,0x40,0x3c,0x66,0xd8,0x33,0x7e,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00, \
    0x00,0x00,0x18,0x00,0x00,0x1e,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
    0x00,0x0c,0x36,0x36,0x18,0x00,0x38,0x0c,0x30,0x0c,0x00,0x00,0x00,0x00,0x00,0xc0, \
    0x00,0x1e,0x36,0x36,0x7c,0xc6,0x6c,0x0c,0x18,0x18,0x66,0x0c,0x00,0x00,0x00,0x60, \
    0x00,0x1e,0x36,0x7f,0x06,0x66,0x38,0x06,0x0c,0x30,0x3c,0x0c,0x00,0x00,0x00,0x30, \
    0x00,0x0c,0x00,0x36,0x3c,0x30,0xdc,0x00,0x0c,0x30,0xff,0x3f,0x00,0x7e,0x00,0x18, \
    0x00,0x0c,0x00,0x7f,0x60,0x18,0x76,0x00,0x0c,0x30,0x3c,0x0c,0x00,0x00,0x00,0x0c, \
    0x00,0x00,0x00,0x36,0x3e,0xcc,0x66,0x00,0x18,0x18,0x66,0x0c,0x18,0x00,0x18,0x06, \
    0x00,0x0c,0x00,0x36,0x18,0xc6,0xdc,0x00,0x30,0x0c,0x00,0x00,0x18,0x00,0x18,0x02, \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x00, \
    0x7c,0x18,0x3c,0x3c,0x70,0x7e,0x38,0x7e,0x3c,0x3c,0x00,0x00,0x30,0x00,0x0c,0x3c, \
    0xc6,0x1c,0x66,0x66,0x78,0x06,0x0c,0x66,0x66,0x66,0x18,0x18,0x18,0x00,0x18,0x66, \
    0xe6,0x1a,0x60,0x60,0x6c,0x3e,0x06,0x60,0x66,0x66,0x18,0x18,0x0c,0x7e,0x30,0x60, \
    0xf6,0x18,0x38,0x38,0x66,0x60,0x3e,0x30,0x3c,0x7c,0x00,0x00,0x06,0x00,0x60,0x30, \
    0xde,0x18,0x0c,0x60,0xfe,0x60,0x66,0x18,0x66,0x60,0x00,0x00,0x0c,0x00,0x30,0x18, \
    0xce,0x18,0x66,0x66,0x60,0x66,0x66,0x18,0x66,0x30,0x18,0x18,0x18,0x7e,0x18,0x00, \
    0x7c,0x7e,0x7e,0x3c,0xf0,0x3c,0x3c,0x18,0x3c,0x1c,0x18,0x18,0x30,0x00,0x0c,0x18, \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,0x00,0x00,0x00,0x00, \
    0x7c,0x18,0x7e,0x78,0x3e,0xfe,0xfe,0x78,0x66,0x3c,0xf0,0xce,0x1e,0xc6,0xc6,0x7c, \
    0xc6,0x3c,0xcc,0xcc,0x6c,0x8c,0x8c,0xcc,0x66,0x18,0x60,0xcc,0x0c,0xee,0xce,0xc6, \
    0xf6,0x66,0xcc,0x06,0xcc,0x2c,0x2c,0x06,0x66,0x18,0x60,0x6c,0x0c,0xfe,0xde,0xc6, \
    0x96,0x66,0x7c,0x06,0xcc,0x3c,0x3c,0x06,0x7e,0x18,0x60,0x3c,0x0c,0xfe,0xf6,0xc6, \
    0xf6,0x7e,0xcc,0x06,0xcc,0x2c,0x2c,0xe6,0x66,0x18,0x66,0x6c,0x8c,0xd6,0xe6,0xc6, \
    0x06,0x66,0xcc,0xcc,0x6c,0x8c,0x0c,0xcc,0x66,0x18,0x66,0xcc,0xcc,0xc6,0xc6,0xc6, \
    0x7c,0x66,0x7e,0x78,0x3e,0xfe,0x1e,0xf8,0x66,0x3c,0x3c,0xce,0xfe,0xc6,0xc6,0x7c, \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
    0x7e,0x3c,0x7e,0x3c,0x7e,0x66,0x66,0xc6,0xc6,0x66,0xfe,0x3c,0x06,0x3c,0x10,0x00, \
    0xcc,0x66,0xcc,0x66,0x5a,0x66,0x66,0xc6,0xc6,0x66,0xc6,0x0c,0x0c,0x30,0x38,0x00, \
    0xcc,0x66,0xcc,0x0c,0x18,0x66,0x66,0xc6,0x6c,0x66,0x62,0x0c,0x18,0x30,0x6c,0x00, \
    0x7c,0x66,0x7c,0x18,0x18,0x66,0x66,0xd6,0x38,0x3c,0x30,0x0c,0x30,0x30,0xc6,0x00, \
    0x0c,0x76,0x6c,0x30,0x18,0x66,0x66,0xfe,0x38,0x18,0x98,0x0c,0x60,0x30,0x00,0x00, \
    0x0c,0x3c,0xcc,0x66,0x18,0x66,0x3c,0xee,0x6c,0x18,0xcc,0x0c,0xc0,0x30,0x00,0x00, \
    0x1e,0x70,0xce,0x3c,0x3c,0x7e,0x18,0xc6,0xc6,0x3c,0xfe,0x3c,0x80,0x3c,0x00,0x00, \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff, \
    0x18,0x00,0x0e,0x00,0x70,0x00,0x38,0x00,0x0e,0x18,0x60,0x0e,0x1c,0x00,0x00,0x00, \
    0x18,0x00,0x0c,0x00,0x60,0x00,0x6c,0x00,0x0c,0x00,0x00,0x0c,0x18,0x00,0x00,0x00, \
    0x30,0x3c,0x0c,0x3c,0x60,0x3c,0x0c,0xdc,0x6c,0x1c,0x60,0xcc,0x18,0x66,0x3e,0x3c, \
    0x00,0x60,0x7c,0x66,0x7c,0x66,0x1e,0x66,0xdc,0x18,0x60,0x6c,0x18,0xfe,0x66,0x66, \
    0x00,0x7c,0xcc,0x06,0x66,0x7e,0x0c,0x66,0xcc,0x18,0x60,0x3c,0x18,0xfe,0x66,0x66, \
    0x00,0x66,0xcc,0x66,0x66,0x06,0x0c,0x7c,0xcc,0x18,0x66,0x6c,0x18,0xd6,0x66,0x66, \
    0x00,0xdc,0x76,0x3c,0xdc,0x3c,0x1e,0x60,0xce,0x3c,0x66,0xce,0x3c,0xc6,0x66,0x3c, \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x3c,0x00,0x00,0x00,0x00,0x00, \
    0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x70,0x30,0x0e,0xdc,0x00, \
    0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x30,0x18,0x76,0x10, \
    0x76,0xdc,0x76,0x7c,0x7c,0x66,0x66,0xc6,0xc6,0x66,0x7e,0x18,0x30,0x18,0x00,0x38, \
    0xcc,0x66,0xdc,0x06,0x18,0x66,0x66,0xd6,0x6c,0x66,0x32,0x0e,0x00,0x70,0x00,0x6c, \
    0xcc,0x66,0xcc,0x3c,0x18,0x66,0x66,0xfe,0x38,0x66,0x18,0x18,0x30,0x18,0x00,0xc6, \
    0x7c,0x7c,0x0c,0x60,0x58,0x66,0x3c,0xfe,0x6c,0x7c,0x4c,0x18,0x30,0x18,0x00,0xc6, \
    0x0c,0x60,0x1e,0x3e,0x30,0xdc,0x18,0x6c,0xc6,0x60,0x7e,0x70,0x30,0x0e,0x00,0xfe, \
    0x1e,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x00,0x00,0x00,0x00,0x00,0x00, \
/**/
#define DEFAULT_FONT_SPRITE_DATA \
    3,8,8,7,8,8,7,6,8,7,8,8,7,8,8,8, \
    7,7,7,7,8,8,7,8,7,7,7,7,7,8,8,8, \
    3,5,6,7,7,8,8,4,6,6,8,6,5,7,5,8, \
    8,7,7,7,8,7,7,7,7,7,5,5,6,7,7,7, \
    8,7,8,8,8,8,8,8,7,6,8,8,8,8,8,8, \
    8,7,8,7,7,7,7,8,8,7,8,6,8,6,8,8, \
    6,8,8,7,8,7,7,8,8,6,7,8,6,8,7,7, \
    8,8,8,7,7,8,7,8,8,7,7,7,6,7,8,8, \
/**/
//[[[end]]]


PRIVATE byte_t const default_font_data[(128 / 8)*64] = {
    DEFAULT_FONT_PIXEL_DATA
};


INTDEF struct wm_format libwm_format_monochrome;
INTDEF struct wm_surface_ops surface1_ops;
PRIVATE struct wm_font wm_default_font = {
    .s_refcnt      = 0x3fffffff,
    .s_ops         = &surface1_ops,
    .s_format      = &libwm_format_monochrome,
    .s_flags       = WM_SURFACE_FISFONT,
    .s_lock        = ATOMIC_RWLOCK_INIT,
    .s_sizex       = 128,
    .s_sizey       = 64,
    .s_stride      = 128 / 8,
    .s_buffer      = (byte_t *)default_font_data,
    .f_lnsiz       = 9,
    .f_log2_chsizx = 3,
    .f_log2_chsizy = 3,
    .f_chsizy      = 8,
    .f_tabsize     = 8*4,
    .f_default = {
        .f_color    = WM_COLOR_WHITE,
        .f_shadow   = WM_COLOR_BLACK,
        .f_options  = WM_TEXT_OPTION_FNORMAL,
        .f_shadow_x = 1,
        .f_shadow_y = 1,
    },
    .f_ascii = {
        DEFAULT_FONT_SPRITE_DATA
    }
};


DEFINE_PUBLIC_ALIAS(wm_font_system,libwm_font_system);
INTERN ATTR_RETNONNULL struct wm_font const *
WMCALL libwm_font_system(unsigned int name) {
 struct wm_font const *result;
 switch (name) {

 case WM_FONT_SYSTEM_DEFAULT:
  result = &wm_default_font;
  break;

 default:
  error_throw(E_INVALID_ARGUMENT);
  break;
 }
 return result;
}



DECL_END

#endif /* !GUARD_LIBS_LIBWM_FONT_C */

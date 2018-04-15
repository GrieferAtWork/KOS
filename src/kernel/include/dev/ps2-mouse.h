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
#ifndef GUARD_KERNEL_INCLUDE_DEV_PS2_MOUSE_H
#define GUARD_KERNEL_INCLUDE_DEV_PS2_MOUSE_H 1
#define _KOS_SOURCE 1

#include <hybrid/compiler.h>
#include <dev/ps2.h>

#ifdef CONFIG_HAVE_DEV_PS2
DECL_BEGIN

/* PS/2 mouse command codes (For use with `ps2_send'; s.a. `ps2_runprogram'). */
#define PS2_MOUSE_FSET_RESOLUTION       0xe8
#define PS2_MOUSE_FSTATUS_REQUEST       0xe9
#define PS2_MOUSE_FSET_STREAM_MODE      0xea
#define PS2_MOUSE_FREAD_DATA            0xeb
#define PS2_MOUSE_FRESET_WRAP_MODE      0xec
#define PS2_MOUSE_FSET_WRAP_MODE        0xee
#define PS2_MOUSE_FSET_REMOTE_MODE      0xf0
#define PS2_MOUSE_FSET_SAMPLE_RATE      0xf3
#define PS2_MOUSE_FENABLE_REPORTING     0xf4
#define PS2_MOUSE_FDISABLE_REPORTING    0xf5
#define PS2_MOUSE_FSETDEFAULT           0xf6
#define PS2_MOUSE_FRESEND               0xfe
#define PS2_MOUSE_FRESET                0xff


#define PS2_MOUSE_TYPE_FNORMAL   0x00 /* Normal mouse */
#define PS2_MOUSE_TYPE_FWHEEL    0x03 /* Mouse with scroll-wheel */
#define PS2_MOUSE_TYPE_F5BUTTON  0x04 /* 5-button mouse */

DECL_END
#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_INCLUDE_DEV_PS2_MOUSE_H */
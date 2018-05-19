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
#ifndef GUARD_LIBS_LIBC_I386_KOS_TLS_C
#define GUARD_LIBS_LIBC_I386_KOS_TLS_C 1

#include "../libc.h"
#include "../rtl.h"
#include <kos/thread.h>
#include <hybrid/compiler.h>
#include <syslog.h>

DECL_BEGIN

typedef struct {
    uintptr_t ti_moduleid;
    uintptr_t ti_tlsoffset;
} TLS_index;

INTERN void *FCALL x86_dynamic_tls_addr(TLS_index *__restrict index) {
 libc_syslog(LOG_DEBUG,"TODO: x86_dynamic_tls_addr(%p,%p) -- Lazy allocation\n",
             index->ti_moduleid,
             index->ti_tlsoffset);
 return (void *)index;
}


DECL_END

#endif /* !GUARD_LIBS_LIBC_I386_KOS_TLS_C */

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
#ifndef GUARD_KERNEL_SRC_UNWIND_LINKER_C
#define GUARD_KERNEL_SRC_UNWIND_LINKER_C 1
#define _NOSERVE_SOURCE 1

#include <hybrid/compiler.h>
#include <kos/types.h>
#include <unwind/eh_frame.h>
#include <unwind/linker.h>
#include <stdbool.h>
#include <kernel/debug.h>
#include <kernel/vm.h>
#include <except.h>
#include <fs/driver.h>
#include <fs/linker.h>
#include <fs/path.h>
#include <fs/node.h>

DECL_BEGIN

PUBLIC bool KCALL
linker_findfde_consafe(uintptr_t ip, struct fde_info *__restrict result) {
 return TASK_EVAL_CONSAFE(linker_findfde(ip,result));
}

PUBLIC bool KCALL
linker_findexcept_consafe(uintptr_t ip, u16 exception_code,
                          struct exception_handler_info *__restrict result) {
 return TASK_EVAL_CONSAFE(linker_findexcept(ip,exception_code,result));
}


/* Lookup the application at `ip' and load the FDE entry associated with `ip'.
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located the FDE entry.
 * @return: false: Could not find an FDE entry for `ip' */
PUBLIC bool KCALL
linker_findfde(uintptr_t ip, struct fde_info *__restrict result) {
 REF struct application *EXCEPT_VAR app = NULL;
 bool COMPILER_IGNORE_UNINITIALIZED(fde_ok);
 REF struct vm_region *EXCEPT_VAR region; uintptr_t reloffset;
 struct vm *EXCEPT_VAR effective_vm; struct vm_node *node;
 effective_vm = ip >= KERNEL_BASE ? &vm_kernel : THIS_VM;
#ifdef NDEBUG
 if (!vm_tryacquire_read(effective_vm))
#endif
 {
  if (PREEMPTION_ENABLED()) {
   u16 EXCEPT_VAR old_state;
   old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
   TRY {
    vm_acquire_read(effective_vm);
   } FINALLY {
    if (!(old_state & TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
   }
  } else {
   /* Can only search the kernel itself. */
   if (effective_vm != &vm_kernel)
       return false;
   return kernel_eh_findfde(ip,result);
  }
 }
 TRY {
  node = vm_getnode(VM_ADDR2PAGE(ip));
  if (node) {
   if (node->vn_notify == &application_notify) {
    app = (REF struct application *)node->vn_closure;
    application_incref(app);
   } else if ((region = node->vn_region)->vr_ctl != NULL) {
    /* Make `ip' become relative to the region. */
    reloffset  = VM_NODE_MINADDR(node);
    reloffset += node->vn_start * PAGESIZE;
    vm_region_incref(region);
    vm_release_read(effective_vm);
    goto invoke_region; /* XXX: Skip over FINALLY is intentional */
   }
  }
 } FINALLY {
  vm_release_read(effective_vm);
 }
 if (!app) return false;
 TRY {
  if (!module_loadexcept(app) &&
      !app->a_module->m_sect.m_eh_frame.ds_size)
      fde_ok = false;
  else {
   struct module *mod = app->a_module;
   /* Lookup FDE information. */
   fde_ok = fde_cache_lookup(&mod->m_fde_cache,result,ip - app->a_loadaddr);
   if (fde_ok) {
    /* Convert into absolute FDE information. */
    fde_info_mkabs(result,app->a_loadaddr);
   } else {
    fde_ok = eh_findfde((byte_t *)
                       ((uintptr_t)mod->m_sect.m_eh_frame.ds_base + app->a_loadaddr),
                                   mod->m_sect.m_eh_frame.ds_size,
                         ip,result);
    if (fde_ok) {
     /* We managed to find something! now to cache it. */
     fde_info_mkrel(result,app->a_loadaddr);
     fde_cache_insert(mod,result);
     fde_info_mkabs(result,app->a_loadaddr);
    }
   }
  }
 } FINALLY {
  application_decref(app);
 }
 return fde_ok;
invoke_region:
 TRY {
  /* Perform a region control command to find the FDE. */
  fde_ok = (*region->vr_ctl)(region,REGION_CTL_FFIND_FDE,
                             ip-reloffset,result) != 0;
  if (fde_ok)
      result->fi_pcbegin += reloffset,
      result->fi_pcend   += reloffset;
 } FINALLY {
  vm_region_decref(region);
 }
 return fde_ok;
}

/* Lookup the effective exception handler for the given `ip' and fill in `result'
 * NOTE: The caller is responsible to ensure, or deal with problems
 *       caused by the associated application suddenly being unmapped.
 * @return: true:  Successfully located an exception handler.
 * @return: false: Could not find an exception handler for `ip' */
FUNDEF bool KCALL
linker_findexcept(uintptr_t ip, u16 exception_code,
                  struct exception_handler_info *__restrict result) {
 REF struct application *EXCEPT_VAR app = NULL;
 bool COMPILER_IGNORE_UNINITIALIZED(except_ok);
 struct vm *EXCEPT_VAR effective_vm; struct vm_node *node;
 effective_vm = ip >= KERNEL_BASE ? &vm_kernel : THIS_VM;
#ifdef NDEBUG
 if (!vm_tryacquire_read(effective_vm))
#endif
 {
  if (PREEMPTION_ENABLED()) {
   u16 EXCEPT_VAR old_state;
   old_state = ATOMIC_FETCHOR(THIS_TASK->t_state,TASK_STATE_FDONTSERVE);
   TRY {
    vm_acquire_read(effective_vm);
   } FINALLY {
    if (!(old_state & TASK_STATE_FDONTSERVE))
          ATOMIC_FETCHAND(THIS_TASK->t_state,~TASK_STATE_FDONTSERVE);
   }
  } else {
   /* Can only search the kernel itself. */
   if (effective_vm != &vm_kernel)
       return false;
   return kernel_findexcept(ip,exception_code,result);
  }
 }
 TRY {
  node = vm_getnode(VM_ADDR2PAGE(ip));
  if (node && node->vn_notify == &application_notify) {
   app = (REF struct application *)node->vn_closure;
   application_incref(app);
  }
 } FINALLY {
  vm_release_read(effective_vm);
 }
 if (!app) return false;
 TRY {
  except_ok = false;
  if (module_loadexcept(app) ||
      app->a_module->m_sect.m_except.ds_size) {
   struct module *mod = app->a_module;
   struct except_handler *iter;
   iter = (struct except_handler *)((uintptr_t)mod->m_sect.m_except.ds_base+
                                               app->a_loadaddr);
   except_ok = except_cache_lookup(iter,
                                  (struct except_handler *)((uintptr_t)iter+
                                                             mod->m_sect.m_except.ds_size),
                                   ip - app->a_loadaddr,exception_code,
                                   app,result);
  }
 } FINALLY {
  application_decref(app);
 }
 return except_ok;
}


DECL_END

#endif /* !GUARD_KERNEL_SRC_UNWIND_LINKER_C */

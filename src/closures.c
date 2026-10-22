/* -----------------------------------------------------------------------
   closures.c - Copyright (c) 2019, 2022, 2026 Anthony Green
                Copyright (c) 2007, 2009, 2010 Red Hat, Inc.
                Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc
                Copyright (c) 2011 Plausible Labs Cooperative, Inc.

   Code to allocate and deallocate memory for closures.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ਜੇ (defined __linux__ || defined __CYGWIN__) && !defined GNU_ਸਰੋਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ GNU_ਸਰੋਤ 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __EMSCRIPTEN__

#ਅਨ੍ਤਰ੍ਭਾਵ <fficonfig.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NetBSD__
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਜੇ_ਅੰਤ

#ਜੇ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ  HAVE_SYS_MEMFD_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/memfd.h>
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅਚਲ size_t overhead =
  (ਆਕਾਰ(max_align_t) > ਆਕਾਰ(ਖਾਲੀ *) + ਆਕਾਰ(size_t)) ?
    ਆਕਾਰ(max_align_t)
    : ਆਕਾਰ(ਖਾਲੀ *) + ਆਕਾਰ(size_t);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(p, d) ((ਖਾਲੀ *)((uintptr_t)(p) + (d)))

ਖਾਲੀ *
ffi_ਘੇਰਾ_alloc (size_t ਮਾਪ, ਖਾਲੀ **ਕੋਡ)
{
  ਸਥਿਰ size_t ਪੰਨਾ_ਮਾਪ;
  size_t rounded_ਮਾਪ;
  ਖਾਲੀ *codeseg, *dataseg;
  ਪੂਰਨ prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  ਜੇ (!ਕੋਡ)
    ਵਾਪਸ ਨਿਰਾ;

  /* Obtain system page size. */
  ਜੇ (!ਪੰਨਾ_ਮਾਪ)
    ਪੰਨਾ_ਮਾਪ = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_ਮਾਪ = (ਮਾਪ + overhead + ਪੰਨਾ_ਮਾਪ - 1) & ~(ਪੰਨਾ_ਮਾਪ - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ਨਿਰਾ, rounded_ਮਾਪ, prot, MAP_ANON | MAP_ਨਿੱਜੀ, -1, 0);
  ਜੇ (dataseg == MAP_FAILED)
    ਵਾਪਸ ਨਿਰਾ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_ਮਾਪ, ਨਿਰਾ, rounded_ਮਾਪ, MAP_REMAPDUP);
  ਜੇ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_ਮਾਪ);
    ਵਾਪਸ ਨਿਰਾ;
  }
  ਜੇ (mprotect(codeseg, rounded_ਮਾਪ, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_ਮਾਪ);
    munmap(dataseg, rounded_ਮਾਪ);
    ਵਾਪਸ ਨਿਰਾ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(dataseg, &rounded_ਮਾਪ, ਆਕਾਰ(rounded_ਮਾਪ));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(dataseg, ਆਕਾਰ(size_t)), &codeseg, ਆਕਾਰ(ਖਾਲੀ *));
  *ਕੋਡ = ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(codeseg, overhead);
  ਵਾਪਸ ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(dataseg, overhead);
}

ਖਾਲੀ
ffi_ਘੇਰਾ_free (ਖਾਲੀ *ptr)
{
  ਖਾਲੀ *codeseg, *dataseg;
  size_t rounded_ਮਾਪ;

  dataseg = ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(ptr, -overhead);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&rounded_ਮਾਪ, dataseg, ਆਕਾਰ(rounded_ਮਾਪ));
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(&codeseg, ਜੋੜੋ_ਮੰਜ਼ਿਲ_ਸੂਚਕ(dataseg, ਆਕਾਰ(size_t)), ਆਕਾਰ(ਖਾਲੀ *));
  munmap(dataseg, rounded_ਮਾਪ);
  munmap(codeseg, rounded_ਮਾਪ);
}

ਪੂਰਨ
ffi_tramp_is_present (__attribute__((ਅਣਵਰਤਿਆ)) ਖਾਲੀ *ptr)
{
  ਵਾਪਸ 0;
}
#ਨਹੀਂਤਾਂ /* !NetBSD with PROT_MPROTECT */

#ਜੇ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ
# ਜੇ __linux__ && !defined(__ANDROID__)
/* This macro indicates it may be forbidden to map anonymous memory
   with both write and execute permission.  Code compiled when this
   option is defined will attempt to map such pages once, but if it
   fails, it falls back to creating a temporary file in a writable and
   executable filesystem and mapping pages from it into separate
   locations in the virtual memory space, one location writable and
   another executable.  */
#  define FFI_MMAP_EXEC_WRIT 1
#  define HAVE_MNTENT 1
# endif
# ਜੇ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#ਜੇ_ਅੰਤ

#ਜੇ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# ਜੇ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# ਜੇ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#ਜੇ_ਅੰਤ /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#ਜੇ FFI_CLOSURES

#ਜੇ FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MACH__

#ਅਨ੍ਤਰ੍ਭਾਵ <mach/mach.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ARM64E_PTRAUTH
#ਅਨ੍ਤਰ੍ਭਾਵ <ptrauth.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਬਾਹਰੀ ਖਾਲੀ *ffi_ਘੇਰਾ_trampoline_ਸਾਰਣੀ_ਪੰਨਾ;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ffi_trampoline_ਸਾਰਣੀ ffi_trampoline_ਸਾਰਣੀ;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ;

ਬਣਤਰ ffi_trampoline_ਸਾਰਣੀ
{
  /* contiguous writable and executable pages */
  vm_ਪਤਾ_t config_ਪੰਨਾ;

  /* free list tracking */
  uint16_t free_ਗਿਣਤੀ;
  ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *free_list;
  ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *free_list_pool;

  ffi_trampoline_ਸਾਰਣੀ *prev;
  ffi_trampoline_ਸਾਰਣੀ *ਅਗਲਾ;
};

ਬਣਤਰ ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ
{
  ਖਾਲੀ *(*trampoline) (ਖਾਲੀ);
  ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *ਅਗਲਾ;
};

/* Total number of trampolines that fit in one trampoline table */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਗਿਣਤੀ (ਪੰਨਾ_MAX_ਮਾਪ / FFI_TRAMPOLINE_ਮਾਪ)

ਸਥਿਰ pthread_mutex_t ffi_trampoline_ਤਾਲਾ = PTHREAD_MUTEX_INITIALIZER;
ਸਥਿਰ ffi_trampoline_ਸਾਰਣੀ *ffi_trampoline_tables = ਨਿਰਾ;

ਸਥਿਰ ffi_trampoline_ਸਾਰਣੀ *
ffi_trampoline_ਸਾਰਣੀ_alloc (ਖਾਲੀ)
{
  ffi_trampoline_ਸਾਰਣੀ *ਸਾਰਣੀ;
  vm_ਪਤਾ_t config_ਪੰਨਾ;
  vm_ਪਤਾ_t trampoline_ਪੰਨਾ;
  vm_ਪਤਾ_t trampoline_ਪੰਨਾ_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_ਪੰਨਾ = 0x0;
  kt = vm_allocate (mach_task_self (), &config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2,
		    VM_FLAGS_ANYWHERE);
  ਜੇ (kt != KERN_SUCCESS)
    ਵਾਪਸ ਨਿਰਾ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_ਪੰਨਾ = config_ਪੰਨਾ + ਪੰਨਾ_MAX_ਮਾਪ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ARM64E_PTRAUTH
  trampoline_ਪੰਨਾ_template = (vm_ਪਤਾ_t)(uintptr_t)ptrauth_auth_ਅੰਕੜੇ((ਖਾਲੀ *)&ffi_ਘੇਰਾ_trampoline_ਸਾਰਣੀ_ਪੰਨਾ, ptrauth_ਕੁੰਜੀ_function_ਸੂਚਕ, 0);
#ਨਹੀਂਤਾਂ
  trampoline_ਪੰਨਾ_template = (vm_ਪਤਾ_t)&ffi_ਘੇਰਾ_trampoline_ਸਾਰਣੀ_ਪੰਨਾ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_ਪੰਨਾ_template &= ~1UL;
#ਜੇ_ਅੰਤ
  kt = vm_remap (mach_task_self (), &trampoline_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_ਪੰਨਾ_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  ਜੇ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2);
      ਵਾਪਸ ਨਿਰਾ;
    }

  ਜੇ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      ਜੇ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2);
          ਵਾਪਸ ਨਿਰਾ;
        }
    }

  /* We have valid trampoline and config pages */
  ਸਾਰਣੀ = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ (1, ਆਕਾਰ (ffi_trampoline_ਸਾਰਣੀ));
  ਜੇ (ਸਾਰਣੀ == ਨਿਰਾ)
    {
      vm_deallocate (mach_task_self (), config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2);
      ਵਾਪਸ ਨਿਰਾ;
    }
  ਸਾਰਣੀ->free_ਗਿਣਤੀ = FFI_TRAMPOLINE_ਗਿਣਤੀ;
  ਸਾਰਣੀ->config_ਪੰਨਾ = config_ਪੰਨਾ;

  /* Create and initialize the free list */
  ਸਾਰਣੀ->free_list_pool =
    ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ (FFI_TRAMPOLINE_ਗਿਣਤੀ, ਆਕਾਰ (ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ));
  ਜੇ (ਸਾਰਣੀ->free_list_pool == ਨਿਰਾ)
    {
      vm_deallocate (mach_task_self (), config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2);
      ਮੁਕਤ (ਸਾਰਣੀ);
      ਵਾਪਸ ਨਿਰਾ;
    }

  ਲਈ (i = 0; i < ਸਾਰਣੀ->free_ਗਿਣਤੀ; i++)
    {
      ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = &ਸਾਰਣੀ->free_list_pool[i];
      ਇੰਦਰਾਜ->trampoline =
	(ਖਾਲੀ *) (trampoline_ਪੰਨਾ + (i * FFI_TRAMPOLINE_ਮਾਪ));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ARM64E_PTRAUTH
      ਇੰਦਰਾਜ->trampoline = ptrauth_sign_unauthenticated(ਇੰਦਰਾਜ->trampoline, ptrauth_ਕੁੰਜੀ_function_ਸੂਚਕ, 0);
#ਜੇ_ਅੰਤ

      ਜੇ (i < ਸਾਰਣੀ->free_ਗਿਣਤੀ - 1)
	ਇੰਦਰਾਜ->ਅਗਲਾ = &ਸਾਰਣੀ->free_list_pool[i + 1];
    }

  ਸਾਰਣੀ->free_list = ਸਾਰਣੀ->free_list_pool;

  ਵਾਪਸ ਸਾਰਣੀ;
}

ਸਥਿਰ ਖਾਲੀ
ffi_trampoline_ਸਾਰਣੀ_free (ffi_trampoline_ਸਾਰਣੀ *ਸਾਰਣੀ)
{
  /* Remove from the list */
  ਜੇ (ਸਾਰਣੀ->prev != ਨਿਰਾ)
    ਸਾਰਣੀ->prev->ਅਗਲਾ = ਸਾਰਣੀ->ਅਗਲਾ;

  ਜੇ (ਸਾਰਣੀ->ਅਗਲਾ != ਨਿਰਾ)
    ਸਾਰਣੀ->ਅਗਲਾ->prev = ਸਾਰਣੀ->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), ਸਾਰਣੀ->config_ਪੰਨਾ, ਪੰਨਾ_MAX_ਮਾਪ * 2);

  /* Deallocate free list */
  ਮੁਕਤ (ਸਾਰਣੀ->free_list_pool);
  ਮੁਕਤ (ਸਾਰਣੀ);
}

ਖਾਲੀ *
ffi_ਘੇਰਾ_alloc (size_t ਮਾਪ, ਖਾਲੀ **ਕੋਡ)
{
  /* Create the closure */
  ffi_ਘੇਰਾ *ਘੇਰਾ = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਮਾਪ);
  ਜੇ (ਘੇਰਾ == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;

  pthread_mutex_ਤਾਲਾ (&ffi_trampoline_ਤਾਲਾ);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_ਸਾਰਣੀ *ਸਾਰਣੀ = ffi_trampoline_tables;
  ਜੇ (ਸਾਰਣੀ == ਨਿਰਾ || ਸਾਰਣੀ->free_list == ਨਿਰਾ)
    {
      ਸਾਰਣੀ = ffi_trampoline_ਸਾਰਣੀ_alloc ();
      ਜੇ (ਸਾਰਣੀ == ਨਿਰਾ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_ਤਾਲਾ);
	  ਮੁਕਤ (ਘੇਰਾ);
	  ਵਾਪਸ ਨਿਰਾ;
	}

      /* Insert the new table at the top of the list */
      ਸਾਰਣੀ->ਅਗਲਾ = ffi_trampoline_tables;
      ਜੇ (ਸਾਰਣੀ->ਅਗਲਾ != ਨਿਰਾ)
	ਸਾਰਣੀ->ਅਗਲਾ->prev = ਸਾਰਣੀ;

      ffi_trampoline_tables = ਸਾਰਣੀ;
    }

  /* Claim the free entry */
  ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = ਇੰਦਰਾਜ->ਅਗਲਾ;
  ffi_trampoline_tables->free_ਗਿਣਤੀ--;
  ਇੰਦਰਾਜ->ਅਗਲਾ = ਨਿਰਾ;

  pthread_mutex_unlock (&ffi_trampoline_ਤਾਲਾ);

  /* Initialize the return values */
  *ਕੋਡ = ਇੰਦਰਾਜ->trampoline;
  ਘੇਰਾ->trampoline_ਸਾਰਣੀ = ਸਾਰਣੀ;
  ਘੇਰਾ->trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ = ਇੰਦਰਾਜ;

  ਵਾਪਸ ਘੇਰਾ;
}

ਖਾਲੀ
ffi_ਘੇਰਾ_free (ਖਾਲੀ *ptr)
{
  ffi_ਘੇਰਾ *ਘੇਰਾ = ptr;

  pthread_mutex_ਤਾਲਾ (&ffi_trampoline_ਤਾਲਾ);

  /* Fetch the table and entry references */
  ffi_trampoline_ਸਾਰਣੀ *ਸਾਰਣੀ = ਘੇਰਾ->trampoline_ਸਾਰਣੀ;
  ffi_trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ *ਇੰਦਰਾਜ = ਘੇਰਾ->trampoline_ਸਾਰਣੀ_ਇੰਦਰਾਜ;

  /* Return the entry to the free list */
  ਇੰਦਰਾਜ->ਅਗਲਾ = ਸਾਰਣੀ->free_list;
  ਸਾਰਣੀ->free_list = ਇੰਦਰਾਜ;
  ਸਾਰਣੀ->free_ਗਿਣਤੀ++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  ਜੇ (ਸਾਰਣੀ->free_ਗਿਣਤੀ == FFI_TRAMPOLINE_ਗਿਣਤੀ
      && ffi_trampoline_tables != ਸਾਰਣੀ)
    {
      ffi_trampoline_ਸਾਰਣੀ_free (ਸਾਰਣੀ);
    }
  ਨਹੀਂਤਾਂ ਜੇ (ffi_trampoline_tables != ਸਾਰਣੀ)
    {
      /* Otherwise, bump this table to the top of the list */
      ਸਾਰਣੀ->prev = ਨਿਰਾ;
      ਸਾਰਣੀ->ਅਗਲਾ = ffi_trampoline_tables;
      ਜੇ (ffi_trampoline_tables != ਨਿਰਾ)
	ffi_trampoline_tables->prev = ਸਾਰਣੀ;

      ffi_trampoline_tables = ਸਾਰਣੀ;
    }

  pthread_mutex_unlock (&ffi_trampoline_ਤਾਲਾ);

  /* Free the closure */
  ਮੁਕਤ (ਘੇਰਾ);
}

#ਜੇ_ਅੰਤ

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#ਜਾਂਜੇ FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_LOCKS 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_DL_ਅਗੇਤਰ 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_MMAP_ਹੱਦ MAX_ਮਾਪ_T

/* Don't allocate more than a page unless needed.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਜੇ !defined(_WIN32)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT
#ਅਨ੍ਤਰ੍ਭਾਵ <mntent.h>
#ਜੇ_ਅੰਤ /* HAVE_MNTENT */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/param.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LACKS_SYS_MMAN_H 1

#ਜੇ FFI_MMAP_EXEC_SELINUX
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/statfs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਸਥਿਰ ਪੂਰਨ selinux_enabled = -1;

ਸਥਿਰ ਪੂਰਨ
selinux_enabled_check (ਖਾਲੀ)
{
  ਬਣਤਰ statfs sfs;
  FILE *f;
  ਅੱਖਰ *buf = ਨਿਰਾ;
  size_t len = 0;

  ਜੇ (statfs ("/selinux", &sfs) >= 0
      && (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) sfs.f_type == 0xf97cff8cU)
    ਵਾਪਸ 1;
  f = fopen ("/proc/mounts", "r");
  ਜੇ (f == ਨਿਰਾ)
    ਵਾਪਸ 0;
  ਜਦੋਂ (getline (&buf, &len, f) >= 0)
    {
      ਅੱਖਰ *p = strchr (buf, ' ');
      ਜੇ (p == ਨਿਰਾ)
        ਰੋਕੋ;
      p = strchr (p + 1, ' ');
      ਜੇ (p == ਨਿਰਾ)
        ਰੋਕੋ;
      ਜੇ (ਲੜੀ_ਤੁਲਨਾ (p + 1, "selinuxfs ", 10) == 0)
        {
          ਮੁਕਤ (buf);
          fclose (f);
          ਵਾਪਸ 1;
        }
    }
  ਮੁਕਤ (buf);
  fclose (f);
  ਵਾਪਸ 0;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#ਨਹੀਂਤਾਂ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_selinux_enabled() 0

#ਜੇ_ਅੰਤ /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#ਜੇ defined FFI_MMAP_PAX
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਗਿਣਤੀ {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ਸਥਿਰ ਪੂਰਨ cached_pax_flags = -1;

ਸਥਿਰ ਪੂਰਨ
pax_flags_check (ਖਾਲੀ)
{
  ਅੱਖਰ *buf = ਨਿਰਾ;
  size_t len = 0;
  FILE *f;
  ਪੂਰਨ ret;
  f = fopen ("/proc/self/status", "r");
  ਜੇ (f == ਨਿਰਾ)
    ਵਾਪਸ 0;
  ret = 0;

  ਜਦੋਂ (getline (&buf, &len, f) != -1)
    ਜੇ (!ਲੜੀ_ਤੁਲਨਾ (buf, "PaX:", 4))
      {
        ਜੇ (ਨਿਰਾ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        ਜੇ (ਨਿਰਾ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ਰੋਕੋ;
      }
  ਮੁਕਤ (buf);
  fclose (f);
  ਵਾਪਸ ret;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#ਜੇ_ਅੰਤ /* defined FFI_MMAP_PAX */

#ਜਾਂਜੇ defined (__CYGWIN__) || defined(__INTERIX)

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_selinux_enabled() 0

#ਜੇ_ਅੰਤ /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#ਜੇ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#ਜੇ_ਅੰਤ /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ਸਥਿਰ ਖਾਲੀ *dlmalloc(size_t);
ਸਥਿਰ ਖਾਲੀ dlfree(ਖਾਲੀ*);
ਸਥਿਰ ਖਾਲੀ *dlcalloc(size_t, size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ *dlrealloc(ਖਾਲੀ *, size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ *dlmemalign(size_t, size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ *dlvalloc(size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਪੂਰਨ dlmallopt(ਪੂਰਨ, ਪੂਰਨ) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ size_t dlmalloc_footprint(ਖਾਲੀ) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ size_t dlmalloc_max_footprint(ਖਾਲੀ) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ** dlindependent_calloc(size_t, size_t, ਖਾਲੀ**) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ** dlindependent_comalloc(size_t, size_t*, ਖਾਲੀ**) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ *dlpvalloc(size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਪੂਰਨ dlmalloc_trim(size_t) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ size_t dlmalloc_usable_ਮਾਪ(ਖਾਲੀ*) MAYBE_ਅਣਵਰਤਿਆ;
ਸਥਿਰ ਖਾਲੀ dlmalloc_stats(ਖਾਲੀ) MAYBE_ਅਣਵਰਤਿਆ;

#ਜੇ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ਸਥਿਰ ਖਾਲੀ *dlmmap(ਖਾਲੀ *, size_t, ਪੂਰਨ, ਪੂਰਨ, ਪੂਰਨ, off_t);
ਸਥਿਰ ਪੂਰਨ dlmunmap(ਖਾਲੀ *, size_t);
#ਜੇ_ਅੰਤ /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mmap dlmmap
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ munmap dlmunmap

#ਅਨ੍ਤਰ੍ਭਾਵ "dlmalloc.c"

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ mmap
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ munmap

#ਜੇ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ਸਥਿਰ pthread_mutex_t open_ਆਰਜ਼ੀ_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ਸਥਿਰ ਪੂਰਨ execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ਸਥਿਰ size_t execsize = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_memfd (ਅਚਲ ਅੱਖਰ *name)
{
  ਪੂਰਨ fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ਵਾਪਸ fd;
}
#ਜੇ_ਅੰਤ

/* Open a temporary file name, and immediately unlink it.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_name (ਅੱਖਰ *name, ਪੂਰਨ flags MAYBE_ਅਣਵਰਤਿਆ)
{
  ਪੂਰਨ fd;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#ਨਹੀਂਤਾਂ
  fd = mkstemp (name);
#ਜੇ_ਅੰਤ

  ਜੇ (fd != -1)
    unlink (name);

  ਵਾਪਸ fd;
}

/* Open a temporary file in the named directory.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_dir (ਅਚਲ ਅੱਖਰ *dir)
{
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ suffix[] = "/ffiXXXXXX";
  ਪੂਰਨ lendir, flags;
  ਅੱਖਰ *tempname;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_TMPFILE
  ਪੂਰਨ fd;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_CLOEXEC
  flags = O_CLOEXEC;
#ਨਹੀਂਤਾਂ
  flags = 0;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  ਜੇ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ਵਾਪਸ fd;
  } ਨਹੀਂਤਾਂ {
    errno = 0;
  }
#ਜੇ_ਅੰਤ

  lendir = (ਪੂਰਨ) ਲੜੀ_ਲੰਬਾਈ (dir);
  tempname = __builtin_alloca (lendir + ਆਕਾਰ (suffix));

  ਜੇ (!tempname)
    ਵਾਪਸ -1;

  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tempname, dir, lendir);
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tempname + lendir, suffix, ਆਕਾਰ (suffix));

  ਵਾਪਸ open_ਆਰਜ਼ੀ_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_env (ਅਚਲ ਅੱਖਰ *envvar)
{
  ਅਚਲ ਅੱਖਰ *ਮੁੱਲ = getenv (envvar);

  ਜੇ (!ਮੁੱਲ)
    ਵਾਪਸ -1;

  ਵਾਪਸ open_ਆਰਜ਼ੀ_exec_file_dir (ਮੁੱਲ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_mnt (ਅਚਲ ਅੱਖਰ *mounts)
{
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਆਖਰੀ_mounts;
  ਸਥਿਰ FILE *ਆਖਰੀ_mntent;

  ਜੇ (mounts != ਆਖਰੀ_mounts)
    {
      ਜੇ (ਆਖਰੀ_mntent)
	endmntent (ਆਖਰੀ_mntent);

      ਆਖਰੀ_mounts = mounts;

      ਜੇ (mounts)
	ਆਖਰੀ_mntent = setmntent (mounts, "r");
      ਨਹੀਂਤਾਂ
	ਆਖਰੀ_mntent = ਨਿਰਾ;
    }

  ਜੇ (!ਆਖਰੀ_mntent)
    ਵਾਪਸ -1;

  ਲਈ (;;)
    {
      ਪੂਰਨ fd;
      ਬਣਤਰ mntent mnt;
      ਅੱਖਰ buf[MAXPATHLEN * 3];

      ਜੇ (getmntent_r (ਆਖਰੀ_mntent, &mnt, buf, ਆਕਾਰ (buf)) == ਨਿਰਾ)
	ਵਾਪਸ -1;

      ਜੇ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ਜਾਰੀ;

      fd = open_ਆਰਜ਼ੀ_exec_file_dir (mnt.mnt_dir);

      ਜੇ (fd != -1)
	ਵਾਪਸ fd;
    }
}
#ਜੇ_ਅੰਤ /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ਸਥਿਰ ਬਣਤਰ
{
  ਪੂਰਨ (*func)(ਅਚਲ ਅੱਖਰ *);
  ਅਚਲ ਅੱਖਰ *arg;
  ਪੂਰਨ repeat;
} open_ਆਰਜ਼ੀ_exec_file_opts[] = {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MEMFD_CREATE
  { open_ਆਰਜ਼ੀ_exec_file_memfd, "libffi", 0 },
#ਜੇ_ਅੰਤ
  { open_ਆਰਜ਼ੀ_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_ਆਰਜ਼ੀ_exec_file_env, "TMPDIR", 0 },
  { open_ਆਰਜ਼ੀ_exec_file_dir, "/tmp", 0 },
  { open_ਆਰਜ਼ੀ_exec_file_dir, "/var/tmp", 0 },
  { open_ਆਰਜ਼ੀ_exec_file_dir, "/dev/shm", 0 },
  { open_ਆਰਜ਼ੀ_exec_file_env, "HOME", 0 },
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_MNTENT
  { open_ਆਰਜ਼ੀ_exec_file_mnt, "/etc/mtab", 1 },
  { open_ਆਰਜ਼ੀ_exec_file_mnt, "/proc/mounts", 1 },
#ਜੇ_ਅੰਤ /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ਸਥਿਰ ਪੂਰਨ open_ਆਰਜ਼ੀ_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ਸਥਿਰ ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file_opts_ਅਗਲਾ (ਖਾਲੀ)
{
  ਜੇ (open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].repeat)
    open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].func (ਨਿਰਾ);

  open_ਆਰਜ਼ੀ_exec_file_opts_idx++;
  ਜੇ (open_ਆਰਜ਼ੀ_exec_file_opts_idx
      == (ਆਕਾਰ (open_ਆਰਜ਼ੀ_exec_file_opts)
	  / ਆਕਾਰ (*open_ਆਰਜ਼ੀ_exec_file_opts)))
    {
      open_ਆਰਜ਼ੀ_exec_file_opts_idx = 0;
      ਵਾਪਸ 1;
    }

  ਵਾਪਸ 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
ਪੂਰਨ
open_ਆਰਜ਼ੀ_exec_file (ਖਾਲੀ)
{
  ਪੂਰਨ fd;

  ਕਰੋ
    {
      fd = open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].func
	(open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].arg);

      ਜੇ (!open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  ਜੇ (open_ਆਰਜ਼ੀ_exec_file_opts_ਅਗਲਾ ())
	    ਰੋਕੋ;
	}
    }
  ਜਦੋਂ (fd == -1);

  ਵਾਪਸ fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ਸਥਿਰ ਪੂਰਨ
allocate_space (ਪੂਰਨ fd, off_t len)
{
  ਸਥਿਰ ਲੰਮਾ ਪੰਨਾ_ਮਾਪ;

  /* Obtain system page size. */
  ਜੇ (!ਪੰਨਾ_ਮਾਪ)
    ਪੰਨਾ_ਮਾਪ = sysconf(_SC_PAGESIZE);
  ਜੇ (ਪੰਨਾ_ਮਾਪ <= 0)
    ਵਾਪਸ -1;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ buf[ਪੰਨਾ_ਮਾਪ];
  memset (buf, 0, ਪੰਨਾ_ਮਾਪ);

  ਜਦੋਂ (len > 0)
    {
      off_t ਮੰਜ਼ਿਲ_write = (len < ਪੰਨਾ_ਮਾਪ) ? len : ਪੰਨਾ_ਮਾਪ;
      ਜੇ (ਲਿਖ (fd, buf, ਮੰਜ਼ਿਲ_write) < ਮੰਜ਼ਿਲ_write)
        ਵਾਪਸ -1;
      len -= ਮੰਜ਼ਿਲ_write;
    }

  ਵਾਪਸ 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ਸਥਿਰ ਖਾਲੀ *
dlmmap_locked (ਖਾਲੀ *ਸ਼ੁਰੂ, size_t ਲੰਮਾਈ, ਪੂਰਨ prot, ਪੂਰਨ flags, off_t offset)
{
  ਖਾਲੀ *ptr;

  ਜੇ (execfd == -1)
    {
      open_ਆਰਜ਼ੀ_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_ਆਰਜ਼ੀ_exec_file ();
      ਜੇ (execfd == -1)
	ਵਾਪਸ MFAIL;
    }

  offset = execsize;

  ਜੇ (allocate_space (execfd, ਲੰਮਾਈ))
    ਵਾਪਸ MFAIL;

  flags &= ~(MAP_ਨਿੱਜੀ | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ਨਿਰਾ, ਲੰਮਾਈ, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  ਜੇ (ptr == MFAIL)
    {
      ਜੇ (!offset)
	{
	  close (execfd);
	  ਜਾਓ retry_open;
	}
      ਜੇ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ਵਾਪਸ MFAIL;
    }
  ਨਹੀਂਤਾਂ ਜੇ (!offset
	   && open_ਆਰਜ਼ੀ_exec_file_opts[open_ਆਰਜ਼ੀ_exec_file_opts_idx].repeat)
    open_ਆਰਜ਼ੀ_exec_file_opts_ਅਗਲਾ ();

  ਸ਼ੁਰੂ = mmap (ਸ਼ੁਰੂ, ਲੰਮਾਈ, prot, flags, execfd, offset);

  ਜੇ (ਸ਼ੁਰੂ == MFAIL)
    {
      munmap (ptr, ਲੰਮਾਈ);
      ਜੇ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ਵਾਪਸ ਸ਼ੁਰੂ;
    }

  mmap_exec_ਵਿੱਥ ((ਅੱਖਰ *)ਸ਼ੁਰੂ, ਲੰਮਾਈ) = (ਅੱਖਰ*)ptr - (ਅੱਖਰ*)ਸ਼ੁਰੂ;

  execsize += ਲੰਮਾਈ;

  ਵਾਪਸ ਸ਼ੁਰੂ;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ਸਥਿਰ ਖਾਲੀ *
dlmmap (ਖਾਲੀ *ਸ਼ੁਰੂ, size_t ਲੰਮਾਈ, ਪੂਰਨ prot,
	ਪੂਰਨ flags, ਪੂਰਨ fd, off_t offset)
{
  ਖਾਲੀ *ptr;

  ਪ੍ਰਤਿਜ੍ਞਾ (ਸ਼ੁਰੂ == ਨਿਰਾ && ਲੰਮਾਈ % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_ਨਿੱਜੀ | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  ਜੇ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (ਸ਼ੁਰੂ, ਲੰਮਾਈ, prot & ~PROT_EXEC, flags, fd, offset);
      ਵਾਪਸ ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  ਜੇ (execfd == -1 && is_mprotect_enabled ())
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MMAP_EXEC_EMUTRAMP_PAX
      ਜੇ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (ਸ਼ੁਰੂ, ਲੰਮਾਈ, prot & ~PROT_EXEC, flags, fd, offset);
          ਵਾਪਸ ptr;
        }
#ਜੇ_ਅੰਤ
      /* fallback to dlmmap_locked.  */
    }
  ਨਹੀਂਤਾਂ ਜੇ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (ਸ਼ੁਰੂ, ਲੰਮਾਈ, prot | PROT_EXEC, flags, fd, offset);

      ਜੇ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ਵਾਪਸ ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_ਤਾਲਾ (&open_ਆਰਜ਼ੀ_exec_file_mutex);
  ptr = dlmmap_locked (ਸ਼ੁਰੂ, ਲੰਮਾਈ, prot, flags, offset);
  pthread_mutex_unlock (&open_ਆਰਜ਼ੀ_exec_file_mutex);

  ਵਾਪਸ ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ਸਥਿਰ ਪੂਰਨ
dlmunmap (ਖਾਲੀ *ਸ਼ੁਰੂ, size_t ਲੰਮਾਈ)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, ਸ਼ੁਰੂ);
  ਖਾਲੀ *ਕੋਡ;

  ਜੇ (seg && (ਕੋਡ = ਜੋੜੋ_segment_exec_ਵਿੱਥ (ਸ਼ੁਰੂ, seg)) != ਸ਼ੁਰੂ)
    {
      ਪੂਰਨ ret = munmap (ਕੋਡ, ਲੰਮਾਈ);
      ਜੇ (ret)
	ਵਾਪਸ ret;
    }

  ਵਾਪਸ munmap (ਸ਼ੁਰੂ, ਲੰਮਾਈ);
}

#ਜੇ FFI_ਘੇਰਾ_FREE_ਕੋਡ
/* Return segment holding given code address.  */
ਸਥਿਰ msegmentptr
segment_holding_ਕੋਡ (mstate m, ਅੱਖਰ* addr)
{
  msegmentptr sp = &m->seg;
  ਲਈ (;;) {
    ਜੇ (addr >= ਜੋੜੋ_segment_exec_ਵਿੱਥ (sp->ਅਧਾਰ, sp)
	&& addr < ਜੋੜੋ_segment_exec_ਵਿੱਥ (sp->ਅਧਾਰ, sp) + sp->ਮਾਪ)
      ਵਾਪਸ sp;
    ਜੇ ((sp = sp->ਅਗਲਾ) == 0)
      ਵਾਪਸ 0;
  }
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
ਖਾਲੀ *
ffi_ਘੇਰਾ_alloc (size_t ਮਾਪ, ਖਾਲੀ **ਕੋਡ)
{
  ਖਾਲੀ *ptr, *ftramp;

  ਜੇ (!ਕੋਡ)
    ਵਾਪਸ ਨਿਰਾ;

  ptr = dlmalloc (ਮਾਪ);

  ਜੇ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *ਕੋਡ = FFI_FN (ਜੋੜੋ_segment_exec_ਵਿੱਥ (ptr, seg));
      ਜੇ (!ffi_tramp_is_supported ())
        ਵਾਪਸ ptr;

      ftramp = ffi_tramp_alloc (0);
      ਜੇ (ftramp == ਨਿਰਾ)
      {
        dlfree (ptr);
        ਵਾਪਸ ਨਿਰਾ;
      }
      *ਕੋਡ = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_ਘੇਰਾ *) ptr)->ftramp = ftramp;
    }

  ਵਾਪਸ ptr;
}

ਖਾਲੀ *
ffi_ਅੰਕੜੇ_ਮੰਜ਼ਿਲ_ਕੋਡ_ਸੂਚਕ (ਖਾਲੀ *ਅੰਕੜੇ)
{
  msegmentptr seg = segment_holding (gm, ਅੰਕੜੇ);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  ਜੇ (seg)
    {
      ਜੇ (!ffi_tramp_is_supported ())
        ਵਾਪਸ ਜੋੜੋ_segment_exec_ਵਿੱਥ (ਅੰਕੜੇ, seg);
      ਵਾਪਸ ffi_tramp_get_addr (((ffi_ਘੇਰਾ *) ਅੰਕੜੇ)->ftramp);
    }
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ ਅੰਕੜੇ;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
ਖਾਲੀ
ffi_ਘੇਰਾ_free (ਖਾਲੀ *ptr)
{
#ਜੇ FFI_ਘੇਰਾ_FREE_ਕੋਡ
  msegmentptr seg = segment_holding_ਕੋਡ (gm, ptr);

  ਜੇ (seg)
    ptr = sub_segment_exec_ਵਿੱਥ (ptr, seg);
#ਜੇ_ਅੰਤ
  ਜੇ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_ਘੇਰਾ *) ptr)->ftramp);

  dlfree (ptr);
}

ਪੂਰਨ
ffi_tramp_is_present (ਖਾਲੀ *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ਵਾਪਸ seg != ਨਿਰਾ && ffi_tramp_is_supported();
}

# ਨਹੀਂਤਾਂ /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

ਖਾਲੀ *
ffi_ਘੇਰਾ_alloc (size_t ਮਾਪ, ਖਾਲੀ **ਕੋਡ)
{
  ਖਾਲੀ *c;

  ਜੇ (!ਕੋਡ)
    ਵਾਪਸ ਨਿਰਾ;

  c = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਮਾਪ);
  *ਕੋਡ = FFI_FN (c);
  ਵਾਪਸ c;
}

ਖਾਲੀ
ffi_ਘੇਰਾ_free (ਖਾਲੀ *ptr)
{
  ਮੁਕਤ (ptr);
}

ਖਾਲੀ *
ffi_ਅੰਕੜੇ_ਮੰਜ਼ਿਲ_ਕੋਡ_ਸੂਚਕ (ਖਾਲੀ *ਅੰਕੜੇ)
{
  ਵਾਪਸ ਅੰਕੜੇ;
}

ਪੂਰਨ
ffi_tramp_is_present (__attribute__((ਅਣਵਰਤਿਆ)) ਖਾਲੀ *ptr)
{
  ਵਾਪਸ 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#ਜੇ_ਅੰਤ /* FFI_CLOSURES */

#ਜੇ_ਅੰਤ /* NetBSD with PROT_MPROTECT */
#ਜੇ_ਅੰਤ /* __EMSCRIPTEN__ */

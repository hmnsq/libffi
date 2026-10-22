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

#ಆದರೆ (defined __linux__ || defined __CYGWIN__) && !defined GNU_ಮೂಲ
#ವ್ಯಾಖ್ಯಾನಿಸು GNU_ಮೂಲ 1
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __EMSCRIPTEN__

#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NetBSD__
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ಅನ್ತರ್ಭಾವ <sys/mman.h>

#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ  HAVE_SYS_MEMFD_H
#ಅನ್ತರ್ಭಾವ <sys/memfd.h>
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ size_t overhead =
  (ಗಾತ್ರ(max_align_t) > ಗಾತ್ರ(ಶೂನ್ಯ *) + ಗಾತ್ರ(size_t)) ?
    ಗಾತ್ರ(max_align_t)
    : ಗಾತ್ರ(ಶೂನ್ಯ *) + ಗಾತ್ರ(size_t);

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(p, d) ((ಶೂನ್ಯ *)((uintptr_t)(p) + (d)))

ಶೂನ್ಯ *
ffi_ಆವರಣ_alloc (size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2)
{
  ಸ್ಥಿರ size_t ಪುಟ_ಗಾತ್ರ;
  size_t rounded_ಗಾತ್ರ;
  ಶೂನ್ಯ *codeseg, *dataseg;
  ಪೂರ್ಣಾಂಕ prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  ಆದರೆ (!ಸಂಕೇತ_2)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  /* Obtain system page size. */
  ಆದರೆ (!ಪುಟ_ಗಾತ್ರ)
    ಪುಟ_ಗಾತ್ರ = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_ಗಾತ್ರ = (ಗಾತ್ರ_2 + overhead + ಪುಟ_ಗಾತ್ರ - 1) & ~(ಪುಟ_ಗಾತ್ರ - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ಖಾಲಿ, rounded_ಗಾತ್ರ, prot, MAP_ANON | MAP_ಖಾಸಗಿ, -1, 0);
  ಆದರೆ (dataseg == MAP_FAILED)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_ಗಾತ್ರ, ಖಾಲಿ, rounded_ಗಾತ್ರ, MAP_REMAPDUP);
  ಆದರೆ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_ಗಾತ್ರ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }
  ಆದರೆ (mprotect(codeseg, rounded_ಗಾತ್ರ, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_ಗಾತ್ರ);
    munmap(dataseg, rounded_ಗಾತ್ರ);
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  ಸ್ಮರಣೆ_ನಕಲು(dataseg, &rounded_ಗಾತ್ರ, ಗಾತ್ರ(rounded_ಗಾತ್ರ));
  ಸ್ಮರಣೆ_ನಕಲು(ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(dataseg, ಗಾತ್ರ(size_t)), &codeseg, ಗಾತ್ರ(ಶೂನ್ಯ *));
  *ಸಂಕೇತ_2 = ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(codeseg, overhead);
  ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(dataseg, overhead);
}

ಶೂನ್ಯ
ffi_ಆವರಣ_free (ಶೂನ್ಯ *ptr)
{
  ಶೂನ್ಯ *codeseg, *dataseg;
  size_t rounded_ಗಾತ್ರ;

  dataseg = ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(ptr, -overhead);
  ಸ್ಮರಣೆ_ನಕಲು(&rounded_ಗಾತ್ರ, dataseg, ಗಾತ್ರ(rounded_ಗಾತ್ರ));
  ಸ್ಮರಣೆ_ನಕಲು(&codeseg, ಕೂಡಿಸು_ಗಮ್ಯ_ಸೂಚಕ(dataseg, ಗಾತ್ರ(size_t)), ಗಾತ್ರ(ಶೂನ್ಯ *));
  munmap(dataseg, rounded_ಗಾತ್ರ);
  munmap(codeseg, rounded_ಗಾತ್ರ);
}

ಪೂರ್ಣಾಂಕ
ffi_tramp_is_present (__attribute__((ಬಳಸದ)) ಶೂನ್ಯ *ptr)
{
  ಹಿಂತಿರುಗಿ 0;
}
#ಇಲ್ಲದಿದ್ದರೆ /* !NetBSD with PROT_MPROTECT */

#ಆದರೆ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ
# ಆದರೆ __linux__ && !defined(__ANDROID__)
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
# ಆದರೆ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# ಆದರೆ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# ಆದರೆ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#ಆದರೆ_ಅಂತ್ಯ /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#ಆದರೆ FFI_CLOSURES

#ಆದರೆ FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MACH__

#ಅನ್ತರ್ಭಾವ <mach/mach.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ARM64E_PTRAUTH
#ಅನ್ತರ್ಭಾವ <ptrauth.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಬಾಹ್ಯ ಶೂನ್ಯ *ffi_ಆವರಣ_trampoline_ಕೋಷ್ಟಕ_ಪುಟ;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ffi_trampoline_ಕೋಷ್ಟಕ ffi_trampoline_ಕೋಷ್ಟಕ;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು;

ರಚನೆ ffi_trampoline_ಕೋಷ್ಟಕ
{
  /* contiguous writable and executable pages */
  vm_ವಿಳಾಸ_t config_ಪುಟ;

  /* free list tracking */
  uint16_t free_ಎಣಿಕೆ;
  ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *free_list;
  ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *free_list_pool;

  ffi_trampoline_ಕೋಷ್ಟಕ *prev;
  ffi_trampoline_ಕೋಷ್ಟಕ *ಮುಂದಿನ;
};

ರಚನೆ ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು
{
  ಶೂನ್ಯ *(*trampoline) (ಶೂನ್ಯ);
  ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *ಮುಂದಿನ;
};

/* Total number of trampolines that fit in one trampoline table */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಎಣಿಕೆ (ಪುಟ_MAX_ಗಾತ್ರ / FFI_TRAMPOLINE_ಗಾತ್ರ)

ಸ್ಥಿರ pthread_mutex_t ffi_trampoline_ಬೀಗ = PTHREAD_MUTEX_INITIALIZER;
ಸ್ಥಿರ ffi_trampoline_ಕೋಷ್ಟಕ *ffi_trampoline_tables = ಖಾಲಿ;

ಸ್ಥಿರ ffi_trampoline_ಕೋಷ್ಟಕ *
ffi_trampoline_ಕೋಷ್ಟಕ_alloc (ಶೂನ್ಯ)
{
  ffi_trampoline_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ;
  vm_ವಿಳಾಸ_t config_ಪುಟ;
  vm_ವಿಳಾಸ_t trampoline_ಪುಟ;
  vm_ವಿಳಾಸ_t trampoline_ಪುಟ_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_ಪುಟ = 0x0;
  kt = vm_allocate (mach_task_self (), &config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2,
		    VM_FLAGS_ANYWHERE);
  ಆದರೆ (kt != KERN_SUCCESS)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_ಪುಟ = config_ಪುಟ + ಪುಟ_MAX_ಗಾತ್ರ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ARM64E_PTRAUTH
  trampoline_ಪುಟ_template = (vm_ವಿಳಾಸ_t)(uintptr_t)ptrauth_auth_ದತ್ತಾಂಶ((ಶೂನ್ಯ *)&ffi_ಆವರಣ_trampoline_ಕೋಷ್ಟಕ_ಪುಟ, ptrauth_ಕೀಲಿ_function_ಸೂಚಕ, 0);
#ಇಲ್ಲದಿದ್ದರೆ
  trampoline_ಪುಟ_template = (vm_ವಿಳಾಸ_t)&ffi_ಆವರಣ_trampoline_ಕೋಷ್ಟಕ_ಪುಟ;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_ಪುಟ_template &= ~1UL;
#ಆದರೆ_ಅಂತ್ಯ
  kt = vm_remap (mach_task_self (), &trampoline_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_ಪುಟ_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  ಆದರೆ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2);
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

  ಆದರೆ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      ಆದರೆ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2);
          ಹಿಂತಿರುಗಿ ಖಾಲಿ;
        }
    }

  /* We have valid trampoline and config pages */
  ಕೋಷ್ಟಕ = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು (1, ಗಾತ್ರ (ffi_trampoline_ಕೋಷ್ಟಕ));
  ಆದರೆ (ಕೋಷ್ಟಕ == ಖಾಲಿ)
    {
      vm_deallocate (mach_task_self (), config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2);
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }
  ಕೋಷ್ಟಕ->free_ಎಣಿಕೆ = FFI_TRAMPOLINE_ಎಣಿಕೆ;
  ಕೋಷ್ಟಕ->config_ಪುಟ = config_ಪುಟ;

  /* Create and initialize the free list */
  ಕೋಷ್ಟಕ->free_list_pool =
    ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು (FFI_TRAMPOLINE_ಎಣಿಕೆ, ಗಾತ್ರ (ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು));
  ಆದರೆ (ಕೋಷ್ಟಕ->free_list_pool == ಖಾಲಿ)
    {
      vm_deallocate (mach_task_self (), config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2);
      ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ);
      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
    }

  ಗಾಗಿ (i = 0; i < ಕೋಷ್ಟಕ->free_ಎಣಿಕೆ; i++)
    {
      ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *ನಮೂದು = &ಕೋಷ್ಟಕ->free_list_pool[i];
      ನಮೂದು->trampoline =
	(ಶೂನ್ಯ *) (trampoline_ಪುಟ + (i * FFI_TRAMPOLINE_ಗಾತ್ರ));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ARM64E_PTRAUTH
      ನಮೂದು->trampoline = ptrauth_sign_unauthenticated(ನಮೂದು->trampoline, ptrauth_ಕೀಲಿ_function_ಸೂಚಕ, 0);
#ಆದರೆ_ಅಂತ್ಯ

      ಆದರೆ (i < ಕೋಷ್ಟಕ->free_ಎಣಿಕೆ - 1)
	ನಮೂದು->ಮುಂದಿನ = &ಕೋಷ್ಟಕ->free_list_pool[i + 1];
    }

  ಕೋಷ್ಟಕ->free_list = ಕೋಷ್ಟಕ->free_list_pool;

  ಹಿಂತಿರುಗಿ ಕೋಷ್ಟಕ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_trampoline_ಕೋಷ್ಟಕ_free (ffi_trampoline_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ)
{
  /* Remove from the list */
  ಆದರೆ (ಕೋಷ್ಟಕ->prev != ಖಾಲಿ)
    ಕೋಷ್ಟಕ->prev->ಮುಂದಿನ = ಕೋಷ್ಟಕ->ಮುಂದಿನ;

  ಆದರೆ (ಕೋಷ್ಟಕ->ಮುಂದಿನ != ಖಾಲಿ)
    ಕೋಷ್ಟಕ->ಮುಂದಿನ->prev = ಕೋಷ್ಟಕ->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), ಕೋಷ್ಟಕ->config_ಪುಟ, ಪುಟ_MAX_ಗಾತ್ರ * 2);

  /* Deallocate free list */
  ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ->free_list_pool);
  ಬಿಡುಗಡೆ (ಕೋಷ್ಟಕ);
}

ಶೂನ್ಯ *
ffi_ಆವರಣ_alloc (size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2)
{
  /* Create the closure */
  ffi_ಆವರಣ *ಆವರಣ = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ_2);
  ಆದರೆ (ಆವರಣ == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  pthread_mutex_ಬೀಗ (&ffi_trampoline_ಬೀಗ);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ = ffi_trampoline_tables;
  ಆದರೆ (ಕೋಷ್ಟಕ == ಖಾಲಿ || ಕೋಷ್ಟಕ->free_list == ಖಾಲಿ)
    {
      ಕೋಷ್ಟಕ = ffi_trampoline_ಕೋಷ್ಟಕ_alloc ();
      ಆದರೆ (ಕೋಷ್ಟಕ == ಖಾಲಿ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_ಬೀಗ);
	  ಬಿಡುಗಡೆ (ಆವರಣ);
	  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

      /* Insert the new table at the top of the list */
      ಕೋಷ್ಟಕ->ಮುಂದಿನ = ffi_trampoline_tables;
      ಆದರೆ (ಕೋಷ್ಟಕ->ಮುಂದಿನ != ಖಾಲಿ)
	ಕೋಷ್ಟಕ->ಮುಂದಿನ->prev = ಕೋಷ್ಟಕ;

      ffi_trampoline_tables = ಕೋಷ್ಟಕ;
    }

  /* Claim the free entry */
  ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *ನಮೂದು = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = ನಮೂದು->ಮುಂದಿನ;
  ffi_trampoline_tables->free_ಎಣಿಕೆ--;
  ನಮೂದು->ಮುಂದಿನ = ಖಾಲಿ;

  pthread_mutex_unlock (&ffi_trampoline_ಬೀಗ);

  /* Initialize the return values */
  *ಸಂಕೇತ_2 = ನಮೂದು->trampoline;
  ಆವರಣ->trampoline_ಕೋಷ್ಟಕ = ಕೋಷ್ಟಕ;
  ಆವರಣ->trampoline_ಕೋಷ್ಟಕ_ನಮೂದು = ನಮೂದು;

  ಹಿಂತಿರುಗಿ ಆವರಣ;
}

ಶೂನ್ಯ
ffi_ಆವರಣ_free (ಶೂನ್ಯ *ptr)
{
  ffi_ಆವರಣ *ಆವರಣ = ptr;

  pthread_mutex_ಬೀಗ (&ffi_trampoline_ಬೀಗ);

  /* Fetch the table and entry references */
  ffi_trampoline_ಕೋಷ್ಟಕ *ಕೋಷ್ಟಕ = ಆವರಣ->trampoline_ಕೋಷ್ಟಕ;
  ffi_trampoline_ಕೋಷ್ಟಕ_ನಮೂದು *ನಮೂದು = ಆವರಣ->trampoline_ಕೋಷ್ಟಕ_ನಮೂದು;

  /* Return the entry to the free list */
  ನಮೂದು->ಮುಂದಿನ = ಕೋಷ್ಟಕ->free_list;
  ಕೋಷ್ಟಕ->free_list = ನಮೂದು;
  ಕೋಷ್ಟಕ->free_ಎಣಿಕೆ++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  ಆದರೆ (ಕೋಷ್ಟಕ->free_ಎಣಿಕೆ == FFI_TRAMPOLINE_ಎಣಿಕೆ
      && ffi_trampoline_tables != ಕೋಷ್ಟಕ)
    {
      ffi_trampoline_ಕೋಷ್ಟಕ_free (ಕೋಷ್ಟಕ);
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ffi_trampoline_tables != ಕೋಷ್ಟಕ)
    {
      /* Otherwise, bump this table to the top of the list */
      ಕೋಷ್ಟಕ->prev = ಖಾಲಿ;
      ಕೋಷ್ಟಕ->ಮುಂದಿನ = ffi_trampoline_tables;
      ಆದರೆ (ffi_trampoline_tables != ಖಾಲಿ)
	ffi_trampoline_tables->prev = ಕೋಷ್ಟಕ;

      ffi_trampoline_tables = ಕೋಷ್ಟಕ;
    }

  pthread_mutex_unlock (&ffi_trampoline_ಬೀಗ);

  /* Free the closure */
  ಬಿಡುಗಡೆ (ಆವರಣ);
}

#ಆದರೆ_ಅಂತ್ಯ

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#ಅಥವಾಆದರೆ FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#ವ್ಯಾಖ್ಯಾನಿಸು USE_LOCKS 1
#ವ್ಯಾಖ್ಯಾನಿಸು USE_DL_ಪೂರ್ವಪ್ರತ್ಯಯ 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#ವ್ಯಾಖ್ಯಾನಿಸು HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#ವ್ಯಾಖ್ಯಾನಿಸು NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_MMAP_ಮಿತಿ MAX_ಗಾತ್ರ_T

/* Don't allocate more than a page unless needed.  */
#ವ್ಯಾಖ್ಯಾನಿಸು DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _MSC_VER
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಆದರೆ !defined(_WIN32)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT
#ಅನ್ತರ್ಭಾವ <mntent.h>
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MNTENT */
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ವ್ಯಾಖ್ಯಾನಿಸು LACKS_SYS_MMAN_H 1

#ಆದರೆ FFI_MMAP_EXEC_SELINUX
#ಅನ್ತರ್ಭಾವ <sys/statfs.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selinux_enabled = -1;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
selinux_enabled_check (ಶೂನ್ಯ)
{
  ರಚನೆ statfs sfs;
  FILE *f;
  ಅಕ್ಷರ *buf = ಖಾಲಿ;
  size_t len = 0;

  ಆದರೆ (statfs ("/selinux", &sfs) >= 0
      && (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) sfs.f_type == 0xf97cff8cU)
    ಹಿಂತಿರುಗಿ 1;
  f = fopen ("/proc/mounts", "r");
  ಆದರೆ (f == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;
  ಆಗ (getline (&buf, &len, f) >= 0)
    {
      ಅಕ್ಷರ *p = strchr (buf, ' ');
      ಆದರೆ (p == ಖಾಲಿ)
        ನಿಲ್ಲಿಸು;
      p = strchr (p + 1, ' ');
      ಆದರೆ (p == ಖಾಲಿ)
        ನಿಲ್ಲಿಸು;
      ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ (p + 1, "selinuxfs ", 10) == 0)
        {
          ಬಿಡುಗಡೆ (buf);
          fclose (f);
          ಹಿಂತಿರುಗಿ 1;
        }
    }
  ಬಿಡುಗಡೆ (buf);
  fclose (f);
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿಸು is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#ಇಲ್ಲದಿದ್ದರೆ

#ವ್ಯಾಖ್ಯಾನಿಸು is_selinux_enabled() 0

#ಆದರೆ_ಅಂತ್ಯ /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#ಆದರೆ defined FFI_MMAP_PAX
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಎಣಿಕೆ {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cached_pax_flags = -1;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
pax_flags_check (ಶೂನ್ಯ)
{
  ಅಕ್ಷರ *buf = ಖಾಲಿ;
  size_t len = 0;
  FILE *f;
  ಪೂರ್ಣಾಂಕ ret;
  f = fopen ("/proc/self/status", "r");
  ಆದರೆ (f == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;
  ret = 0;

  ಆಗ (getline (&buf, &len, f) != -1)
    ಆದರೆ (!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ (buf, "PaX:", 4))
      {
        ಆದರೆ (ಖಾಲಿ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        ಆದರೆ (ಖಾಲಿ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ನಿಲ್ಲಿಸು;
      }
  ಬಿಡುಗಡೆ (buf);
  fclose (f);
  ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿಸು get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#ವ್ಯಾಖ್ಯಾನಿಸು has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#ವ್ಯಾಖ್ಯಾನಿಸು is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#ವ್ಯಾಖ್ಯಾನಿಸು is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#ಆದರೆ_ಅಂತ್ಯ /* defined FFI_MMAP_PAX */

#ಅಥವಾಆದರೆ defined (__CYGWIN__) || defined(__INTERIX)

#ಅನ್ತರ್ಭಾವ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#ವ್ಯಾಖ್ಯಾನಿಸು is_selinux_enabled() 0

#ಆದರೆ_ಅಂತ್ಯ /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#ಆದರೆ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#ಆದರೆ_ಅಂತ್ಯ /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ಸ್ಥಿರ ಶೂನ್ಯ *dlmalloc(size_t);
ಸ್ಥಿರ ಶೂನ್ಯ dlfree(ಶೂನ್ಯ*);
ಸ್ಥಿರ ಶೂನ್ಯ *dlcalloc(size_t, size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ *dlrealloc(ಶೂನ್ಯ *, size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ *dlmemalign(size_t, size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ *dlvalloc(size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dlmallopt(ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ size_t dlmalloc_footprint(ಶೂನ್ಯ) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ size_t dlmalloc_max_footprint(ಶೂನ್ಯ) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ** dlindependent_calloc(size_t, size_t, ಶೂನ್ಯ**) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ** dlindependent_comalloc(size_t, size_t*, ಶೂನ್ಯ**) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ *dlpvalloc(size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dlmalloc_trim(size_t) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ size_t dlmalloc_usable_ಗಾತ್ರ(ಶೂನ್ಯ*) MAYBE_ಬಳಸದ;
ಸ್ಥಿರ ಶೂನ್ಯ dlmalloc_stats(ಶೂನ್ಯ) MAYBE_ಬಳಸದ;

#ಆದರೆ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ಸ್ಥಿರ ಶೂನ್ಯ *dlmmap(ಶೂನ್ಯ *, size_t, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ, off_t);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dlmunmap(ಶೂನ್ಯ *, size_t);
#ಆದರೆ_ಅಂತ್ಯ /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#ವ್ಯಾಖ್ಯಾನಿಸು mmap dlmmap
#ವ್ಯಾಖ್ಯಾನಿಸು munmap dlmunmap

#ಅನ್ತರ್ಭಾವ "dlmalloc.c"

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು mmap
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು munmap

#ಆದರೆ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ಸ್ಥಿರ pthread_mutex_t open_ತಾತ್ಕಾಲಿಕ_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ಸ್ಥಿರ size_t execsize = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_memfd (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
  ಪೂರ್ಣಾಂಕ fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ಹಿಂತಿರುಗಿ fd;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Open a temporary file name, and immediately unlink it.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_name (ಅಕ್ಷರ *name, ಪೂರ್ಣಾಂಕ flags MAYBE_ಬಳಸದ)
{
  ಪೂರ್ಣಾಂಕ fd;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#ಇಲ್ಲದಿದ್ದರೆ
  fd = mkstemp (name);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (fd != -1)
    unlink (name);

  ಹಿಂತಿರುಗಿ fd;
}

/* Open a temporary file in the named directory.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_dir (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *dir)
{
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ suffix[] = "/ffiXXXXXX";
  ಪೂರ್ಣಾಂಕ lendir, flags;
  ಅಕ್ಷರ *tempname;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_TMPFILE
  ಪೂರ್ಣಾಂಕ fd;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_CLOEXEC
  flags = O_CLOEXEC;
#ಇಲ್ಲದಿದ್ದರೆ
  flags = 0;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  ಆದರೆ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ಹಿಂತಿರುಗಿ fd;
  } ಇಲ್ಲದಿದ್ದರೆ {
    errno = 0;
  }
#ಆದರೆ_ಅಂತ್ಯ

  lendir = (ಪೂರ್ಣಾಂಕ) ಅಕ್ಷರಸರಣಿ_ಉದ್ದ (dir);
  tempname = __builtin_alloca (lendir + ಗಾತ್ರ (suffix));

  ಆದರೆ (!tempname)
    ಹಿಂತಿರುಗಿ -1;

  ಸ್ಮರಣೆ_ನಕಲು (tempname, dir, lendir);
  ಸ್ಮರಣೆ_ನಕಲು (tempname + lendir, suffix, ಗಾತ್ರ (suffix));

  ಹಿಂತಿರುಗಿ open_ತಾತ್ಕಾಲಿಕ_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_env (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *envvar)
{
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ = getenv (envvar);

  ಆದರೆ (!ಮೌಲ್ಯ)
    ಹಿಂತಿರುಗಿ -1;

  ಹಿಂತಿರುಗಿ open_ತಾತ್ಕಾಲಿಕ_exec_file_dir (ಮೌಲ್ಯ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_mnt (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *mounts)
{
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆಯ_mounts;
  ಸ್ಥಿರ FILE *ಕೊನೆಯ_mntent;

  ಆದರೆ (mounts != ಕೊನೆಯ_mounts)
    {
      ಆದರೆ (ಕೊನೆಯ_mntent)
	endmntent (ಕೊನೆಯ_mntent);

      ಕೊನೆಯ_mounts = mounts;

      ಆದರೆ (mounts)
	ಕೊನೆಯ_mntent = setmntent (mounts, "r");
      ಇಲ್ಲದಿದ್ದರೆ
	ಕೊನೆಯ_mntent = ಖಾಲಿ;
    }

  ಆದರೆ (!ಕೊನೆಯ_mntent)
    ಹಿಂತಿರುಗಿ -1;

  ಗಾಗಿ (;;)
    {
      ಪೂರ್ಣಾಂಕ fd;
      ರಚನೆ mntent mnt;
      ಅಕ್ಷರ buf[MAXPATHLEN * 3];

      ಆದರೆ (getmntent_r (ಕೊನೆಯ_mntent, &mnt, buf, ಗಾತ್ರ (buf)) == ಖಾಲಿ)
	ಹಿಂತಿರುಗಿ -1;

      ಆದರೆ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ಮುಂದುವರಿ;

      fd = open_ತಾತ್ಕಾಲಿಕ_exec_file_dir (mnt.mnt_dir);

      ಆದರೆ (fd != -1)
	ಹಿಂತಿರುಗಿ fd;
    }
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ಸ್ಥಿರ ರಚನೆ
{
  ಪೂರ್ಣಾಂಕ (*func)(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *);
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg;
  ಪೂರ್ಣಾಂಕ repeat;
} open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[] = {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MEMFD_CREATE
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_memfd, "libffi", 0 },
#ಆದರೆ_ಅಂತ್ಯ
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_env, "TMPDIR", 0 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_dir, "/tmp", 0 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_dir, "/var/tmp", 0 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_dir, "/dev/shm", 0 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_env, "HOME", 0 },
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_MNTENT
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_mnt, "/etc/mtab", 1 },
  { open_ತಾತ್ಕಾಲಿಕ_exec_file_mnt, "/proc/mounts", 1 },
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_ಮುಂದಿನ (ಶೂನ್ಯ)
{
  ಆದರೆ (open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].repeat)
    open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].func (ಖಾಲಿ);

  open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx++;
  ಆದರೆ (open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx
      == (ಗಾತ್ರ (open_ತಾತ್ಕಾಲಿಕ_exec_file_opts)
	  / ಗಾತ್ರ (*open_ತಾತ್ಕಾಲಿಕ_exec_file_opts)))
    {
      open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx = 0;
      ಹಿಂತಿರುಗಿ 1;
    }

  ಹಿಂತಿರುಗಿ 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
ಪೂರ್ಣಾಂಕ
open_ತಾತ್ಕಾಲಿಕ_exec_file (ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಂಕ fd;

  ಮಾಡು
    {
      fd = open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].func
	(open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].arg);

      ಆದರೆ (!open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  ಆದರೆ (open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_ಮುಂದಿನ ())
	    ನಿಲ್ಲಿಸು;
	}
    }
  ಆಗ (fd == -1);

  ಹಿಂತಿರುಗಿ fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
allocate_space (ಪೂರ್ಣಾಂಕ fd, off_t len)
{
  ಸ್ಥಿರ ದೀರ್ಘ ಪುಟ_ಗಾತ್ರ;

  /* Obtain system page size. */
  ಆದರೆ (!ಪುಟ_ಗಾತ್ರ)
    ಪುಟ_ಗಾತ್ರ = sysconf(_SC_PAGESIZE);
  ಆದರೆ (ಪುಟ_ಗಾತ್ರ <= 0)
    ಹಿಂತಿರುಗಿ -1;

  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ buf[ಪುಟ_ಗಾತ್ರ];
  memset (buf, 0, ಪುಟ_ಗಾತ್ರ);

  ಆಗ (len > 0)
    {
      off_t ಗಮ್ಯ_write = (len < ಪುಟ_ಗಾತ್ರ) ? len : ಪುಟ_ಗಾತ್ರ;
      ಆದರೆ (ಲಿಖ (fd, buf, ಗಮ್ಯ_write) < ಗಮ್ಯ_write)
        ಹಿಂತಿರುಗಿ -1;
      len -= ಗಮ್ಯ_write;
    }

  ಹಿಂತಿರುಗಿ 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
dlmmap_locked (ಶೂನ್ಯ *ಆರಂಭ, size_t ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ prot, ಪೂರ್ಣಾಂಕ flags, off_t offset)
{
  ಶೂನ್ಯ *ptr;

  ಆದರೆ (execfd == -1)
    {
      open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_ತಾತ್ಕಾಲಿಕ_exec_file ();
      ಆದರೆ (execfd == -1)
	ಹಿಂತಿರುಗಿ MFAIL;
    }

  offset = execsize;

  ಆದರೆ (allocate_space (execfd, ಉದ್ದಳತೆ))
    ಹಿಂತಿರುಗಿ MFAIL;

  flags &= ~(MAP_ಖಾಸಗಿ | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ಖಾಲಿ, ಉದ್ದಳತೆ, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  ಆದರೆ (ptr == MFAIL)
    {
      ಆದರೆ (!offset)
	{
	  close (execfd);
	  ಹೋಗು retry_open;
	}
      ಆದರೆ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ಹಿಂತಿರುಗಿ MFAIL;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!offset
	   && open_ತಾತ್ಕಾಲಿಕ_exec_file_opts[open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_idx].repeat)
    open_ತಾತ್ಕಾಲಿಕ_exec_file_opts_ಮುಂದಿನ ();

  ಆರಂಭ = mmap (ಆರಂಭ, ಉದ್ದಳತೆ, prot, flags, execfd, offset);

  ಆದರೆ (ಆರಂಭ == MFAIL)
    {
      munmap (ptr, ಉದ್ದಳತೆ);
      ಆದರೆ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ಹಿಂತಿರುಗಿ ಆರಂಭ;
    }

  mmap_exec_ಸ್ಥಾನಾಂತರ ((ಅಕ್ಷರ *)ಆರಂಭ, ಉದ್ದಳತೆ) = (ಅಕ್ಷರ*)ptr - (ಅಕ್ಷರ*)ಆರಂಭ;

  execsize += ಉದ್ದಳತೆ;

  ಹಿಂತಿರುಗಿ ಆರಂಭ;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
dlmmap (ಶೂನ್ಯ *ಆರಂಭ, size_t ಉದ್ದಳತೆ, ಪೂರ್ಣಾಂಕ prot,
	ಪೂರ್ಣಾಂಕ flags, ಪೂರ್ಣಾಂಕ fd, off_t offset)
{
  ಶೂನ್ಯ *ptr;

  ಪ್ರತಿಜ್ಞಾ (ಆರಂಭ == ಖಾಲಿ && ಉದ್ದಳತೆ % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_ಖಾಸಗಿ | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  ಆದರೆ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (ಆರಂಭ, ಉದ್ದಳತೆ, prot & ~PROT_EXEC, flags, fd, offset);
      ಹಿಂತಿರುಗಿ ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  ಆದರೆ (execfd == -1 && is_mprotect_enabled ())
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MMAP_EXEC_EMUTRAMP_PAX
      ಆದರೆ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (ಆರಂಭ, ಉದ್ದಳತೆ, prot & ~PROT_EXEC, flags, fd, offset);
          ಹಿಂತಿರುಗಿ ptr;
        }
#ಆದರೆ_ಅಂತ್ಯ
      /* fallback to dlmmap_locked.  */
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (ಆರಂಭ, ಉದ್ದಳತೆ, prot | PROT_EXEC, flags, fd, offset);

      ಆದರೆ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ಹಿಂತಿರುಗಿ ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_ಬೀಗ (&open_ತಾತ್ಕಾಲಿಕ_exec_file_mutex);
  ptr = dlmmap_locked (ಆರಂಭ, ಉದ್ದಳತೆ, prot, flags, offset);
  pthread_mutex_unlock (&open_ತಾತ್ಕಾಲಿಕ_exec_file_mutex);

  ಹಿಂತಿರುಗಿ ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
dlmunmap (ಶೂನ್ಯ *ಆರಂಭ, size_t ಉದ್ದಳತೆ)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, ಆರಂಭ);
  ಶೂನ್ಯ *ಸಂಕೇತ_2;

  ಆದರೆ (seg && (ಸಂಕೇತ_2 = ಕೂಡಿಸು_segment_exec_ಸ್ಥಾನಾಂತರ (ಆರಂಭ, seg)) != ಆರಂಭ)
    {
      ಪೂರ್ಣಾಂಕ ret = munmap (ಸಂಕೇತ_2, ಉದ್ದಳತೆ);
      ಆದರೆ (ret)
	ಹಿಂತಿರುಗಿ ret;
    }

  ಹಿಂತಿರುಗಿ munmap (ಆರಂಭ, ಉದ್ದಳತೆ);
}

#ಆದರೆ FFI_ಆವರಣ_FREE_ಸಂಕೇತ
/* Return segment holding given code address.  */
ಸ್ಥಿರ msegmentptr
segment_holding_ಸಂಕೇತ (mstate m, ಅಕ್ಷರ* addr)
{
  msegmentptr sp = &m->seg;
  ಗಾಗಿ (;;) {
    ಆದರೆ (addr >= ಕೂಡಿಸು_segment_exec_ಸ್ಥಾನಾಂತರ (sp->ತಳ, sp)
	&& addr < ಕೂಡಿಸು_segment_exec_ಸ್ಥಾನಾಂತರ (sp->ತಳ, sp) + sp->ಗಾತ್ರ_2)
      ಹಿಂತಿರುಗಿ sp;
    ಆದರೆ ((sp = sp->ಮುಂದಿನ) == 0)
      ಹಿಂತಿರುಗಿ 0;
  }
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
ಶೂನ್ಯ *
ffi_ಆವರಣ_alloc (size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2)
{
  ಶೂನ್ಯ *ptr, *ftramp;

  ಆದರೆ (!ಸಂಕೇತ_2)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  ptr = dlmalloc (ಗಾತ್ರ_2);

  ಆದರೆ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *ಸಂಕೇತ_2 = FFI_FN (ಕೂಡಿಸು_segment_exec_ಸ್ಥಾನಾಂತರ (ptr, seg));
      ಆದರೆ (!ffi_tramp_is_supported ())
        ಹಿಂತಿರುಗಿ ptr;

      ftramp = ffi_tramp_alloc (0);
      ಆದರೆ (ftramp == ಖಾಲಿ)
      {
        dlfree (ptr);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
      }
      *ಸಂಕೇತ_2 = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_ಆವರಣ *) ptr)->ftramp = ftramp;
    }

  ಹಿಂತಿರುಗಿ ptr;
}

ಶೂನ್ಯ *
ffi_ದತ್ತಾಂಶ_ಗಮ್ಯ_ಸಂಕೇತ_ಸೂಚಕ (ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  msegmentptr seg = segment_holding (gm, ದತ್ತಾಂಶ);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  ಆದರೆ (seg)
    {
      ಆದರೆ (!ffi_tramp_is_supported ())
        ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_segment_exec_ಸ್ಥಾನಾಂತರ (ದತ್ತಾಂಶ, seg);
      ಹಿಂತಿರುಗಿ ffi_tramp_get_addr (((ffi_ಆವರಣ *) ದತ್ತಾಂಶ)->ftramp);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
ಶೂನ್ಯ
ffi_ಆವರಣ_free (ಶೂನ್ಯ *ptr)
{
#ಆದರೆ FFI_ಆವರಣ_FREE_ಸಂಕೇತ
  msegmentptr seg = segment_holding_ಸಂಕೇತ (gm, ptr);

  ಆದರೆ (seg)
    ptr = sub_segment_exec_ಸ್ಥಾನಾಂತರ (ptr, seg);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_ಆವರಣ *) ptr)->ftramp);

  dlfree (ptr);
}

ಪೂರ್ಣಾಂಕ
ffi_tramp_is_present (ಶೂನ್ಯ *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ಹಿಂತಿರುಗಿ seg != ಖಾಲಿ && ffi_tramp_is_supported();
}

# ಇಲ್ಲದಿದ್ದರೆ /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಶೂನ್ಯ *
ffi_ಆವರಣ_alloc (size_t ಗಾತ್ರ_2, ಶೂನ್ಯ **ಸಂಕೇತ_2)
{
  ಶೂನ್ಯ *c;

  ಆದರೆ (!ಸಂಕೇತ_2)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  c = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ_2);
  *ಸಂಕೇತ_2 = FFI_FN (c);
  ಹಿಂತಿರುಗಿ c;
}

ಶೂನ್ಯ
ffi_ಆವರಣ_free (ಶೂನ್ಯ *ptr)
{
  ಬಿಡುಗಡೆ (ptr);
}

ಶೂನ್ಯ *
ffi_ದತ್ತಾಂಶ_ಗಮ್ಯ_ಸಂಕೇತ_ಸೂಚಕ (ಶೂನ್ಯ *ದತ್ತಾಂಶ)
{
  ಹಿಂತಿರುಗಿ ದತ್ತಾಂಶ;
}

ಪೂರ್ಣಾಂಕ
ffi_tramp_is_present (__attribute__((ಬಳಸದ)) ಶೂನ್ಯ *ptr)
{
  ಹಿಂತಿರುಗಿ 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */

#ಆದರೆ_ಅಂತ್ಯ /* NetBSD with PROT_MPROTECT */
#ಆದರೆ_ಅಂತ್ಯ /* __EMSCRIPTEN__ */

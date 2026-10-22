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

#ಯದಿ (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ifdef __NetBSD__
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#endif

#ಯದಿ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ಅನ್ತರ್ಭಾವ <sys/mman.h>

#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#ಅನ್ತರ್ಭಾವ <sys/memfd.h>
#endif

ಸ್ಥಿರ ಧ್ರುವ size_t overhead =
  (ಪರಿಮಾಣ(max_align_t) > ಪರಿಮಾಣ(ಶೂನ್ಯ *) + ಪರಿಮಾಣ(size_t)) ?
    ಪರಿಮಾಣ(max_align_t)
    : ಪರಿಮಾಣ(ಶೂನ್ಯ *) + ಪರಿಮಾಣ(size_t);

#define ADD_TO_POINTER(p, d) ((ಶೂನ್ಯ *)((uintptr_t)(p) + (d)))

ಶೂನ್ಯ *
ffi_closure_alloc (size_t size, ಶೂನ್ಯ **code)
{
  ಸ್ಥಿರ size_t page_size;
  size_t rounded_size;
  ಶೂನ್ಯ *codeseg, *dataseg;
  ಪೂರ್ಣಾಙ್ಕ prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  ಯದಿ (!code)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  /* Obtain system page size. */
  ಯದಿ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ರಿಕ್ತ, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  ಯದಿ (dataseg == MAP_FAILED)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, ರಿಕ್ತ, rounded_size, MAP_REMAPDUP);
  ಯದಿ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }
  ಯದಿ (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, ಪರಿಮಾಣ(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, ಪರಿಮಾಣ(size_t)), &codeseg, ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  ಪ್ರತ್ಯಾಗಮ ADD_TO_POINTER(dataseg, overhead);
}

ಶೂನ್ಯ
ffi_closure_free (ಶೂನ್ಯ *ptr)
{
  ಶೂನ್ಯ *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, ಪರಿಮಾಣ(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, ಪರಿಮಾಣ(size_t)), ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_is_present (__attribute__((unused)) ಶೂನ್ಯ *ptr)
{
  ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ /* !NetBSD with PROT_MPROTECT */

#ಯದಿ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# ಯದಿ __linux__ && !defined(__ANDROID__)
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
# ಯದಿ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#ಯದಿ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# ಯದಿ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# ಯದಿ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#ಯದಿ FFI_CLOSURES

#ಯದಿ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#ಅನ್ತರ್ಭಾವ <mach/mach.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#ಅನ್ತರ್ಭಾವ <ptrauth.h>
#endif
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಬಾಹ್ಯ ಶೂನ್ಯ *ffi_closure_trampoline_table_page;

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ffi_trampoline_table ffi_trampoline_table;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ ffi_trampoline_table_entry ffi_trampoline_table_entry;

ಸಂರಚನಾ ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_ಎಣಿಕೆ;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

ಸಂರಚನಾ ffi_trampoline_table_entry
{
  ಶೂನ್ಯ *(*trampoline) (ಶೂನ್ಯ);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_ಎಣಿಕೆ (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

ಸ್ಥಿರ pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
ಸ್ಥಿರ ffi_trampoline_table *ffi_trampoline_tables = ರಿಕ್ತ;

ಸ್ಥಿರ ffi_trampoline_table *
ffi_trampoline_table_alloc (ಶೂನ್ಯ)
{
  ffi_trampoline_table *table;
  vm_address_t config_page;
  vm_address_t trampoline_page;
  vm_address_t trampoline_page_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_page = 0x0;
  kt = vm_allocate (mach_task_self (), &config_page, PAGE_MAX_SIZE * 2,
		    VM_FLAGS_ANYWHERE);
  ಯದಿ (kt != KERN_SUCCESS)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((ಶೂನ್ಯ *)&ffi_closure_trampoline_table_page, ptrauth_ಕೀಲಿ_function_pointer, 0);
#ಅನ್ಯಥಾ
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  ಯದಿ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

  ಯದಿ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      ಯದಿ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, ಪರಿಮಾಣ (ffi_trampoline_table));
  ಯದಿ (table == ರಿಕ್ತ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }
  table->free_ಎಣಿಕೆ = FFI_TRAMPOLINE_ಎಣಿಕೆ;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_ಎಣಿಕೆ, ಪರಿಮಾಣ (ffi_trampoline_table_entry));
  ಯದಿ (table->free_list_pool == ರಿಕ್ತ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ಮುಕ್ತ (table);
      ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
    }

  ಪ್ರತಿ (i = 0; i < table->free_ಎಣಿಕೆ; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(ಶೂನ್ಯ *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_ಕೀಲಿ_function_pointer, 0);
#endif

      ಯದಿ (i < table->free_ಎಣಿಕೆ - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  ಪ್ರತ್ಯಾಗಮ table;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  ಯದಿ (table->prev != ರಿಕ್ತ)
    table->prev->next = table->next;

  ಯದಿ (table->next != ರಿಕ್ತ)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  ಮುಕ್ತ (table->free_list_pool);
  ಮುಕ್ತ (table);
}

ಶೂನ್ಯ *
ffi_closure_alloc (size_t size, ಶೂನ್ಯ **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  ಯದಿ (closure == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  ಯದಿ (table == ರಿಕ್ತ || table->free_list == ರಿಕ್ತ)
    {
      table = ffi_trampoline_table_alloc ();
      ಯದಿ (table == ರಿಕ್ತ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  ಮುಕ್ತ (closure);
	  ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      ಯದಿ (table->next != ರಿಕ್ತ)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_ಎಣಿಕೆ--;
  entry->next = ರಿಕ್ತ;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  ಪ್ರತ್ಯಾಗಮ closure;
}

ಶೂನ್ಯ
ffi_closure_free (ಶೂನ್ಯ *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_ಎಣಿಕೆ++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  ಯದಿ (table->free_ಎಣಿಕೆ == FFI_TRAMPOLINE_ಎಣಿಕೆ
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  ಅನ್ಯಥಾ ಯದಿ (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = ರಿಕ್ತ;
      table->next = ffi_trampoline_tables;
      ಯದಿ (ffi_trampoline_tables != ರಿಕ್ತ)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  ಮುಕ್ತ (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#ನೋಚೇದ್ಯದಿ FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#define USE_LOCKS 1
#define USE_DL_PREFIX 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#define HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#define HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#define NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#define DEFAULT_MMAP_THRESHOLD MAX_SIZE_T

/* Don't allocate more than a page unless needed.  */
#define DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ifndef _MSC_VER
#ಅನ್ತರ್ಭಾವ <unistd.h>
#endif
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಯದಿ !defined(_WIN32)
#ifdef HAVE_MNTENT
#ಅನ್ತರ್ಭಾವ <mntent.h>
#endif /* HAVE_MNTENT */
#ಅನ್ತರ್ಭಾವ <sys/param.h>
#ಅನ್ತರ್ಭಾವ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#ಯದಿ FFI_MMAP_EXEC_SELINUX
#ಅನ್ತರ್ಭಾವ <sys/statfs.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ selinux_enabled = -1;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
selinux_enabled_check (ಶೂನ್ಯ)
{
  ಸಂರಚನಾ statfs sfs;
  FILE *f;
  ವರ್ಣ *buf = ರಿಕ್ತ;
  size_t len = 0;

  ಯದಿ (statfs ("/selinux", &sfs) >= 0
      && (unsigned ಪೂರ್ಣಾಙ್ಕ) sfs.f_type == 0xf97cff8cU)
    ಪ್ರತ್ಯಾಗಮ 1;
  f = fopen ("/proc/mounts", "r");
  ಯದಿ (f == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯಾವತ್ (getline (&buf, &len, f) >= 0)
    {
      ವರ್ಣ *p = strchr (buf, ' ');
      ಯದಿ (p == ರಿಕ್ತ)
        ಭಙ್ಗ;
      p = strchr (p + 1, ' ');
      ಯದಿ (p == ರಿಕ್ತ)
        ಭಙ್ಗ;
      ಯದಿ (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          ಮುಕ್ತ (buf);
          fclose (f);
          ಪ್ರತ್ಯಾಗಮ 1;
        }
    }
  ಮುಕ್ತ (buf);
  fclose (f);
  ಪ್ರತ್ಯಾಗಮ 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#ಅನ್ಯಥಾ

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#ಯದಿ defined FFI_MMAP_PAX
#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಗಣನಾ {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cached_pax_flags = -1;

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
pax_flags_check (ಶೂನ್ಯ)
{
  ವರ್ಣ *buf = ರಿಕ್ತ;
  size_t len = 0;
  FILE *f;
  ಪೂರ್ಣಾಙ್ಕ ret;
  f = fopen ("/proc/self/status", "r");
  ಯದಿ (f == ರಿಕ್ತ)
    ಪ್ರತ್ಯಾಗಮ 0;
  ret = 0;

  ಯಾವತ್ (getline (&buf, &len, f) != -1)
    ಯದಿ (!strncmp (buf, "PaX:", 4))
      {
        ಯದಿ (ರಿಕ್ತ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        ಯದಿ (ರಿಕ್ತ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ಭಙ್ಗ;
      }
  ಮುಕ್ತ (buf);
  fclose (f);
  ಪ್ರತ್ಯಾಗಮ ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#ನೋಚೇದ್ಯದಿ defined (__CYGWIN__) || defined(__INTERIX)

#ಅನ್ತರ್ಭಾವ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#ಯದಿ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ಸ್ಥಿರ ಶೂನ್ಯ *dlmalloc(size_t);
ಸ್ಥಿರ ಶೂನ್ಯ dlfree(ಶೂನ್ಯ*);
ಸ್ಥಿರ ಶೂನ್ಯ *dlcalloc(size_t, size_t) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ *dlrealloc(ಶೂನ್ಯ *, size_t) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ *dlmemalign(size_t, size_t) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ *dlvalloc(size_t) MAYBE_UNUSED;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dlmallopt(ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ) MAYBE_UNUSED;
ಸ್ಥಿರ size_t dlmalloc_footprint(ಶೂನ್ಯ) MAYBE_UNUSED;
ಸ್ಥಿರ size_t dlmalloc_max_footprint(ಶೂನ್ಯ) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ** dlindependent_calloc(size_t, size_t, ಶೂನ್ಯ**) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ** dlindependent_comalloc(size_t, size_t*, ಶೂನ್ಯ**) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ *dlpvalloc(size_t) MAYBE_UNUSED;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dlmalloc_trim(size_t) MAYBE_UNUSED;
ಸ್ಥಿರ size_t dlmalloc_usable_size(ಶೂನ್ಯ*) MAYBE_UNUSED;
ಸ್ಥಿರ ಶೂನ್ಯ dlmalloc_stats(ಶೂನ್ಯ) MAYBE_UNUSED;

#ಯದಿ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ಸ್ಥಿರ ಶೂನ್ಯ *dlmmap(ಶೂನ್ಯ *, size_t, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ, off_t);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dlmunmap(ಶೂನ್ಯ *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#ಅನ್ತರ್ಭಾವ "dlmalloc.c"

#undef mmap
#undef munmap

#ಯದಿ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ಸ್ಥಿರ pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ಸ್ಥಿರ size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_memfd (ಧ್ರುವ ವರ್ಣ *name)
{
  ಪೂರ್ಣಾಙ್ಕ fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ಪ್ರತ್ಯಾಗಮ fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_name (ವರ್ಣ *name, ಪೂರ್ಣಾಙ್ಕ flags MAYBE_UNUSED)
{
  ಪೂರ್ಣಾಙ್ಕ fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#ಅನ್ಯಥಾ
  fd = mkstemp (name);
#endif

  ಯದಿ (fd != -1)
    unlink (name);

  ಪ್ರತ್ಯಾಗಮ fd;
}

/* Open a temporary file in the named directory.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_dir (ಧ್ರುವ ವರ್ಣ *dir)
{
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ suffix[] = "/ffiXXXXXX";
  ಪೂರ್ಣಾಙ್ಕ lendir, flags;
  ವರ್ಣ *tempname;
#ifdef O_TMPFILE
  ಪೂರ್ಣಾಙ್ಕ fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#ಅನ್ಯಥಾ
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  ಯದಿ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ಪ್ರತ್ಯಾಗಮ fd;
  } ಅನ್ಯಥಾ {
    errno = 0;
  }
#endif

  lendir = (ಪೂರ್ಣಾಙ್ಕ) strlen (dir);
  tempname = __builtin_alloca (lendir + ಪರಿಮಾಣ (suffix));

  ಯದಿ (!tempname)
    ಪ್ರತ್ಯಾಗಮ -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, ಪರಿಮಾಣ (suffix));

  ಪ್ರತ್ಯಾಗಮ open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_env (ಧ್ರುವ ವರ್ಣ *envvar)
{
  ಧ್ರುವ ವರ್ಣ *ಮೌಲ್ಯ = getenv (envvar);

  ಯದಿ (!ಮೌಲ್ಯ)
    ಪ್ರತ್ಯಾಗಮ -1;

  ಪ್ರತ್ಯಾಗಮ open_temp_exec_file_dir (ಮೌಲ್ಯ);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_mnt (ಧ್ರುವ ವರ್ಣ *mounts)
{
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *last_mounts;
  ಸ್ಥಿರ FILE *last_mntent;

  ಯದಿ (mounts != last_mounts)
    {
      ಯದಿ (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      ಯದಿ (mounts)
	last_mntent = setmntent (mounts, "r");
      ಅನ್ಯಥಾ
	last_mntent = ರಿಕ್ತ;
    }

  ಯದಿ (!last_mntent)
    ಪ್ರತ್ಯಾಗಮ -1;

  ಪ್ರತಿ (;;)
    {
      ಪೂರ್ಣಾಙ್ಕ fd;
      ಸಂರಚನಾ mntent mnt;
      ವರ್ಣ buf[MAXPATHLEN * 3];

      ಯದಿ (getmntent_r (last_mntent, &mnt, buf, ಪರಿಮಾಣ (buf)) == ರಿಕ್ತ)
	ಪ್ರತ್ಯಾಗಮ -1;

      ಯದಿ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ಅನುವರ್ತ;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      ಯದಿ (fd != -1)
	ಪ್ರತ್ಯಾಗಮ fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ಸ್ಥಿರ ಸಂರಚನಾ
{
  ಪೂರ್ಣಾಙ್ಕ (*func)(ಧ್ರುವ ವರ್ಣ *);
  ಧ್ರುವ ವರ್ಣ *arg;
  ಪೂರ್ಣಾಙ್ಕ repeat;
} open_temp_exec_file_opts[] = {
#ifdef HAVE_MEMFD_CREATE
  { open_temp_exec_file_memfd, "libffi", 0 },
#endif
  { open_temp_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_temp_exec_file_env, "TMPDIR", 0 },
  { open_temp_exec_file_dir, "/tmp", 0 },
  { open_temp_exec_file_dir, "/var/tmp", 0 },
  { open_temp_exec_file_dir, "/dev/shm", 0 },
  { open_temp_exec_file_env, "HOME", 0 },
#ifdef HAVE_MNTENT
  { open_temp_exec_file_mnt, "/etc/mtab", 1 },
  { open_temp_exec_file_mnt, "/proc/mounts", 1 },
#endif /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file_opts_next (ಶೂನ್ಯ)
{
  ಯದಿ (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (ರಿಕ್ತ);

  open_temp_exec_file_opts_idx++;
  ಯದಿ (open_temp_exec_file_opts_idx
      == (ಪರಿಮಾಣ (open_temp_exec_file_opts)
	  / ಪರಿಮಾಣ (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      ಪ್ರತ್ಯಾಗಮ 1;
    }

  ಪ್ರತ್ಯಾಗಮ 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
ಪೂರ್ಣಾಙ್ಕ
open_temp_exec_file (ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಙ್ಕ fd;

  ಕುರು
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      ಯದಿ (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  ಯದಿ (open_temp_exec_file_opts_next ())
	    ಭಙ್ಗ;
	}
    }
  ಯಾವತ್ (fd == -1);

  ಪ್ರತ್ಯಾಗಮ fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
allocate_space (ಪೂರ್ಣಾಙ್ಕ fd, off_t len)
{
  ಸ್ಥಿರ long page_size;

  /* Obtain system page size. */
  ಯದಿ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  ಯದಿ (page_size <= 0)
    ಪ್ರತ್ಯಾಗಮ -1;

  unsigned ವರ್ಣ buf[page_size];
  memset (buf, 0, page_size);

  ಯಾವತ್ (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      ಯದಿ (ಲಿಖ (fd, buf, to_write) < to_write)
        ಪ್ರತ್ಯಾಗಮ -1;
      len -= to_write;
    }

  ಪ್ರತ್ಯಾಗಮ 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
dlmmap_locked (ಶೂನ್ಯ *start, size_t length, ಪೂರ್ಣಾಙ್ಕ prot, ಪೂರ್ಣಾಙ್ಕ flags, off_t offset)
{
  ಶೂನ್ಯ *ptr;

  ಯದಿ (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      ಯದಿ (execfd == -1)
	ಪ್ರತ್ಯಾಗಮ MFAIL;
    }

  offset = execsize;

  ಯದಿ (allocate_space (execfd, length))
    ಪ್ರತ್ಯಾಗಮ MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ರಿಕ್ತ, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  ಯದಿ (ptr == MFAIL)
    {
      ಯದಿ (!offset)
	{
	  close (execfd);
	  ಗಚ್ಛ retry_open;
	}
      ಯದಿ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ಪ್ರತ್ಯಾಗಮ MFAIL;
    }
  ಅನ್ಯಥಾ ಯದಿ (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  ಯದಿ (start == MFAIL)
    {
      munmap (ptr, length);
      ಯದಿ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ಪ್ರತ್ಯಾಗಮ start;
    }

  mmap_exec_offset ((ವರ್ಣ *)start, length) = (ವರ್ಣ*)ptr - (ವರ್ಣ*)start;

  execsize += length;

  ಪ್ರತ್ಯಾಗಮ start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
dlmmap (ಶೂನ್ಯ *start, size_t length, ಪೂರ್ಣಾಙ್ಕ prot,
	ಪೂರ್ಣಾಙ್ಕ flags, ಪೂರ್ಣಾಙ್ಕ fd, off_t offset)
{
  ಶೂನ್ಯ *ptr;

  ಪ್ರತಿಜ್ಞಾ (start == ರಿಕ್ತ && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  ಯದಿ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      ಪ್ರತ್ಯಾಗಮ ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  ಯದಿ (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      ಯದಿ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          ಪ್ರತ್ಯಾಗಮ ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  ಅನ್ಯಥಾ ಯದಿ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      ಯದಿ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ಪ್ರತ್ಯಾಗಮ ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  ಪ್ರತ್ಯಾಗಮ ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
dlmunmap (ಶೂನ್ಯ *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  ಶೂನ್ಯ *code;

  ಯದಿ (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      ಪೂರ್ಣಾಙ್ಕ ret = munmap (code, length);
      ಯದಿ (ret)
	ಪ್ರತ್ಯಾಗಮ ret;
    }

  ಪ್ರತ್ಯಾಗಮ munmap (start, length);
}

#ಯದಿ FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
ಸ್ಥಿರ msegmentptr
segment_holding_code (mstate m, ವರ್ಣ* addr)
{
  msegmentptr sp = &m->seg;
  ಪ್ರತಿ (;;) {
    ಯದಿ (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      ಪ್ರತ್ಯಾಗಮ sp;
    ಯದಿ ((sp = sp->next) == 0)
      ಪ್ರತ್ಯಾಗಮ 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
ಶೂನ್ಯ *
ffi_closure_alloc (size_t size, ಶೂನ್ಯ **code)
{
  ಶೂನ್ಯ *ptr, *ftramp;

  ಯದಿ (!code)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  ptr = dlmalloc (size);

  ಯದಿ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      ಯದಿ (!ffi_tramp_is_supported ())
        ಪ್ರತ್ಯಾಗಮ ptr;

      ftramp = ffi_tramp_alloc (0);
      ಯದಿ (ftramp == ರಿಕ್ತ)
      {
        dlfree (ptr);
        ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  ಪ್ರತ್ಯಾಗಮ ptr;
}

ಶೂನ್ಯ *
ffi_data_to_code_pointer (ಶೂನ್ಯ *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  ಯದಿ (seg)
    {
      ಯದಿ (!ffi_tramp_is_supported ())
        ಪ್ರತ್ಯಾಗಮ add_segment_exec_offset (data, seg);
      ಪ್ರತ್ಯಾಗಮ ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
ಶೂನ್ಯ
ffi_closure_free (ಶೂನ್ಯ *ptr)
{
#ಯದಿ FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  ಯದಿ (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  ಯದಿ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_is_present (ಶೂನ್ಯ *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ಪ್ರತ್ಯಾಗಮ seg != ರಿಕ್ತ && ffi_tramp_is_supported();
}

# ಅನ್ಯಥಾ /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#ಅನ್ತರ್ಭಾವ <stdlib.h>

ಶೂನ್ಯ *
ffi_closure_alloc (size_t size, ಶೂನ್ಯ **code)
{
  ಶೂನ್ಯ *c;

  ಯದಿ (!code)
    ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

  c = malloc (size);
  *code = FFI_FN (c);
  ಪ್ರತ್ಯಾಗಮ c;
}

ಶೂನ್ಯ
ffi_closure_free (ಶೂನ್ಯ *ptr)
{
  ಮುಕ್ತ (ptr);
}

ಶೂನ್ಯ *
ffi_data_to_code_pointer (ಶೂನ್ಯ *data)
{
  ಪ್ರತ್ಯಾಗಮ data;
}

ಪೂರ್ಣಾಙ್ಕ
ffi_tramp_is_present (__attribute__((unused)) ಶೂನ್ಯ *ptr)
{
  ಪ್ರತ್ಯಾಗಮ 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

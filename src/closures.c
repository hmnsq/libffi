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

#എങ്കിൽ (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#അന്തര്ഭാവ <fficonfig.h>
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <tramp.h>

#ifdef __NetBSD__
#അന്തര്ഭാവ <sys/param.h>
#endif

#എങ്കിൽ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#അന്തര്ഭാവ <sys/mman.h>

#അന്തര്ഭാവ <stddef.h>
#അന്തര്ഭാവ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#അന്തര്ഭാവ <sys/memfd.h>
#endif

സ്ഥിതം സ്ഥിരം size_t overhead =
  (വലുപ്പം(max_align_t) > വലുപ്പം(ശൂന്യം *) + വലുപ്പം(size_t)) ?
    വലുപ്പം(max_align_t)
    : വലുപ്പം(ശൂന്യം *) + വലുപ്പം(size_t);

#define ADD_TO_POINTER(p, d) ((ശൂന്യം *)((uintptr_t)(p) + (d)))

ശൂന്യം *
ffi_closure_alloc (size_t size, ശൂന്യം **code)
{
  സ്ഥിതം size_t page_size;
  size_t rounded_size;
  ശൂന്യം *codeseg, *dataseg;
  പൂർണ്ണസംഖ്യ prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  എങ്കിൽ (!code)
    തിരികെ ശൂന്യ;

  /* Obtain system page size. */
  എങ്കിൽ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ശൂന്യ, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  എങ്കിൽ (dataseg == MAP_FAILED)
    തിരികെ ശൂന്യ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, ശൂന്യ, rounded_size, MAP_REMAPDUP);
  എങ്കിൽ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    തിരികെ ശൂന്യ;
  }
  എങ്കിൽ (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    തിരികെ ശൂന്യ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, വലുപ്പം(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, വലുപ്പം(size_t)), &codeseg, വലുപ്പം(ശൂന്യം *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  തിരികെ ADD_TO_POINTER(dataseg, overhead);
}

ശൂന്യം
ffi_closure_free (ശൂന്യം *ptr)
{
  ശൂന്യം *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, വലുപ്പം(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, വലുപ്പം(size_t)), വലുപ്പം(ശൂന്യം *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

പൂർണ്ണസംഖ്യ
ffi_tramp_is_present (__attribute__((unused)) ശൂന്യം *ptr)
{
  തിരികെ 0;
}
#അല്ലെങ്കിൽ /* !NetBSD with PROT_MPROTECT */

#എങ്കിൽ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# എങ്കിൽ __linux__ && !defined(__ANDROID__)
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
# എങ്കിൽ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#എങ്കിൽ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# എങ്കിൽ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# എങ്കിൽ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#എങ്കിൽ FFI_CLOSURES

#എങ്കിൽ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#അന്തര്ഭാവ <mach/mach.h>
#അന്തര്ഭാവ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#അന്തര്ഭാവ <ptrauth.h>
#endif
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>

ബാഹ്യ ശൂന്യം *ffi_closure_trampoline_table_page;

തരനിർവചനം ഘടന ffi_trampoline_table ffi_trampoline_table;
തരനിർവചനം ഘടന ffi_trampoline_table_entry ffi_trampoline_table_entry;

ഘടന ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_എണ്ണം;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

ഘടന ffi_trampoline_table_entry
{
  ശൂന്യം *(*trampoline) (ശൂന്യം);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_എണ്ണം (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

സ്ഥിതം pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
സ്ഥിതം ffi_trampoline_table *ffi_trampoline_tables = ശൂന്യ;

സ്ഥിതം ffi_trampoline_table *
ffi_trampoline_table_alloc (ശൂന്യം)
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
  എങ്കിൽ (kt != KERN_SUCCESS)
    തിരികെ ശൂന്യ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((ശൂന്യം *)&ffi_closure_trampoline_table_page, ptrauth_താക്കോൽ_function_pointer, 0);
#അല്ലെങ്കിൽ
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  എങ്കിൽ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      തിരികെ ശൂന്യ;
    }

  എങ്കിൽ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      എങ്കിൽ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          തിരികെ ശൂന്യ;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, വലുപ്പം (ffi_trampoline_table));
  എങ്കിൽ (table == ശൂന്യ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      തിരികെ ശൂന്യ;
    }
  table->free_എണ്ണം = FFI_TRAMPOLINE_എണ്ണം;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_എണ്ണം, വലുപ്പം (ffi_trampoline_table_entry));
  എങ്കിൽ (table->free_list_pool == ശൂന്യ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      സ്വതന്ത്രം (table);
      തിരികെ ശൂന്യ;
    }

  വേണ്ടി (i = 0; i < table->free_എണ്ണം; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(ശൂന്യം *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_താക്കോൽ_function_pointer, 0);
#endif

      എങ്കിൽ (i < table->free_എണ്ണം - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  തിരികെ table;
}

സ്ഥിതം ശൂന്യം
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  എങ്കിൽ (table->prev != ശൂന്യ)
    table->prev->next = table->next;

  എങ്കിൽ (table->next != ശൂന്യ)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  സ്വതന്ത്രം (table->free_list_pool);
  സ്വതന്ത്രം (table);
}

ശൂന്യം *
ffi_closure_alloc (size_t size, ശൂന്യം **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  എങ്കിൽ (closure == ശൂന്യ)
    തിരികെ ശൂന്യ;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  എങ്കിൽ (table == ശൂന്യ || table->free_list == ശൂന്യ)
    {
      table = ffi_trampoline_table_alloc ();
      എങ്കിൽ (table == ശൂന്യ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  സ്വതന്ത്രം (closure);
	  തിരികെ ശൂന്യ;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      എങ്കിൽ (table->next != ശൂന്യ)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_എണ്ണം--;
  entry->next = ശൂന്യ;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  തിരികെ closure;
}

ശൂന്യം
ffi_closure_free (ശൂന്യം *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_എണ്ണം++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  എങ്കിൽ (table->free_എണ്ണം == FFI_TRAMPOLINE_എണ്ണം
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  അല്ലെങ്കിൽ എങ്കിൽ (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = ശൂന്യ;
      table->next = ffi_trampoline_tables;
      എങ്കിൽ (ffi_trampoline_tables != ശൂന്യ)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  സ്വതന്ത്രം (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#അല്ലെങ്കിലെങ്കിൽ FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <errno.h>
#ifndef _MSC_VER
#അന്തര്ഭാവ <unistd.h>
#endif
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <stdio.h>
#എങ്കിൽ !defined(_WIN32)
#ifdef HAVE_MNTENT
#അന്തര്ഭാവ <mntent.h>
#endif /* HAVE_MNTENT */
#അന്തര്ഭാവ <sys/param.h>
#അന്തര്ഭാവ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#അന്തര്ഭാവ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#എങ്കിൽ FFI_MMAP_EXEC_SELINUX
#അന്തര്ഭാവ <sys/statfs.h>
#അന്തര്ഭാവ <stdlib.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ selinux_enabled = -1;

സ്ഥിതം പൂർണ്ണസംഖ്യ
selinux_enabled_check (ശൂന്യം)
{
  ഘടന statfs sfs;
  FILE *f;
  അക്ഷരം *buf = ശൂന്യ;
  size_t len = 0;

  എങ്കിൽ (statfs ("/selinux", &sfs) >= 0
      && (unsigned പൂർണ്ണസംഖ്യ) sfs.f_type == 0xf97cff8cU)
    തിരികെ 1;
  f = fopen ("/proc/mounts", "r");
  എങ്കിൽ (f == ശൂന്യ)
    തിരികെ 0;
  ആകുമ്പോൾ (getline (&buf, &len, f) >= 0)
    {
      അക്ഷരം *p = strchr (buf, ' ');
      എങ്കിൽ (p == ശൂന്യ)
        നിർത്തുക;
      p = strchr (p + 1, ' ');
      എങ്കിൽ (p == ശൂന്യ)
        നിർത്തുക;
      എങ്കിൽ (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          സ്വതന്ത്രം (buf);
          fclose (f);
          തിരികെ 1;
        }
    }
  സ്വതന്ത്രം (buf);
  fclose (f);
  തിരികെ 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#അല്ലെങ്കിൽ

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#എങ്കിൽ defined FFI_MMAP_PAX
#അന്തര്ഭാവ <stdlib.h>

എണ്ണൽ {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
സ്ഥിതം പൂർണ്ണസംഖ്യ cached_pax_flags = -1;

സ്ഥിതം പൂർണ്ണസംഖ്യ
pax_flags_check (ശൂന്യം)
{
  അക്ഷരം *buf = ശൂന്യ;
  size_t len = 0;
  FILE *f;
  പൂർണ്ണസംഖ്യ ret;
  f = fopen ("/proc/self/status", "r");
  എങ്കിൽ (f == ശൂന്യ)
    തിരികെ 0;
  ret = 0;

  ആകുമ്പോൾ (getline (&buf, &len, f) != -1)
    എങ്കിൽ (!strncmp (buf, "PaX:", 4))
      {
        എങ്കിൽ (ശൂന്യ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        എങ്കിൽ (ശൂന്യ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        നിർത്തുക;
      }
  സ്വതന്ത്രം (buf);
  fclose (f);
  തിരികെ ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#അല്ലെങ്കിലെങ്കിൽ defined (__CYGWIN__) || defined(__INTERIX)

#അന്തര്ഭാവ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#എങ്കിൽ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
സ്ഥിതം ശൂന്യം *dlmalloc(size_t);
സ്ഥിതം ശൂന്യം dlfree(ശൂന്യം*);
സ്ഥിതം ശൂന്യം *dlcalloc(size_t, size_t) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം *dlrealloc(ശൂന്യം *, size_t) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം *dlmemalign(size_t, size_t) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം *dlvalloc(size_t) MAYBE_UNUSED;
സ്ഥിതം പൂർണ്ണസംഖ്യ dlmallopt(പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ) MAYBE_UNUSED;
സ്ഥിതം size_t dlmalloc_footprint(ശൂന്യം) MAYBE_UNUSED;
സ്ഥിതം size_t dlmalloc_max_footprint(ശൂന്യം) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം** dlindependent_calloc(size_t, size_t, ശൂന്യം**) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം** dlindependent_comalloc(size_t, size_t*, ശൂന്യം**) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം *dlpvalloc(size_t) MAYBE_UNUSED;
സ്ഥിതം പൂർണ്ണസംഖ്യ dlmalloc_trim(size_t) MAYBE_UNUSED;
സ്ഥിതം size_t dlmalloc_usable_size(ശൂന്യം*) MAYBE_UNUSED;
സ്ഥിതം ശൂന്യം dlmalloc_stats(ശൂന്യം) MAYBE_UNUSED;

#എങ്കിൽ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
സ്ഥിതം ശൂന്യം *dlmmap(ശൂന്യം *, size_t, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ, off_t);
സ്ഥിതം പൂർണ്ണസംഖ്യ dlmunmap(ശൂന്യം *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#അന്തര്ഭാവ "dlmalloc.c"

#undef mmap
#undef munmap

#എങ്കിൽ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
സ്ഥിതം pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ execfd = -1;

/* The amount of space already allocated from the temporary file.  */
സ്ഥിതം size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_memfd (സ്ഥിരം അക്ഷരം *name)
{
  പൂർണ്ണസംഖ്യ fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  തിരികെ fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_name (അക്ഷരം *name, പൂർണ്ണസംഖ്യ flags MAYBE_UNUSED)
{
  പൂർണ്ണസംഖ്യ fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#അല്ലെങ്കിൽ
  fd = mkstemp (name);
#endif

  എങ്കിൽ (fd != -1)
    unlink (name);

  തിരികെ fd;
}

/* Open a temporary file in the named directory.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_dir (സ്ഥിരം അക്ഷരം *dir)
{
  സ്ഥിതം സ്ഥിരം അക്ഷരം suffix[] = "/ffiXXXXXX";
  പൂർണ്ണസംഖ്യ lendir, flags;
  അക്ഷരം *tempname;
#ifdef O_TMPFILE
  പൂർണ്ണസംഖ്യ fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#അല്ലെങ്കിൽ
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  എങ്കിൽ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    തിരികെ fd;
  } അല്ലെങ്കിൽ {
    errno = 0;
  }
#endif

  lendir = (പൂർണ്ണസംഖ്യ) strlen (dir);
  tempname = __builtin_alloca (lendir + വലുപ്പം (suffix));

  എങ്കിൽ (!tempname)
    തിരികെ -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, വലുപ്പം (suffix));

  തിരികെ open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_env (സ്ഥിരം അക്ഷരം *envvar)
{
  സ്ഥിരം അക്ഷരം *മൂല്യം = getenv (envvar);

  എങ്കിൽ (!മൂല്യം)
    തിരികെ -1;

  തിരികെ open_temp_exec_file_dir (മൂല്യം);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_mnt (സ്ഥിരം അക്ഷരം *mounts)
{
  സ്ഥിതം സ്ഥിരം അക്ഷരം *last_mounts;
  സ്ഥിതം FILE *last_mntent;

  എങ്കിൽ (mounts != last_mounts)
    {
      എങ്കിൽ (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      എങ്കിൽ (mounts)
	last_mntent = setmntent (mounts, "r");
      അല്ലെങ്കിൽ
	last_mntent = ശൂന്യ;
    }

  എങ്കിൽ (!last_mntent)
    തിരികെ -1;

  വേണ്ടി (;;)
    {
      പൂർണ്ണസംഖ്യ fd;
      ഘടന mntent mnt;
      അക്ഷരം buf[MAXPATHLEN * 3];

      എങ്കിൽ (getmntent_r (last_mntent, &mnt, buf, വലുപ്പം (buf)) == ശൂന്യ)
	തിരികെ -1;

      എങ്കിൽ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	തുടരുക;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      എങ്കിൽ (fd != -1)
	തിരികെ fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
സ്ഥിതം ഘടന
{
  പൂർണ്ണസംഖ്യ (*func)(സ്ഥിരം അക്ഷരം *);
  സ്ഥിരം അക്ഷരം *arg;
  പൂർണ്ണസംഖ്യ repeat;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
open_temp_exec_file_opts_next (ശൂന്യം)
{
  എങ്കിൽ (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (ശൂന്യ);

  open_temp_exec_file_opts_idx++;
  എങ്കിൽ (open_temp_exec_file_opts_idx
      == (വലുപ്പം (open_temp_exec_file_opts)
	  / വലുപ്പം (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      തിരികെ 1;
    }

  തിരികെ 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
പൂർണ്ണസംഖ്യ
open_temp_exec_file (ശൂന്യം)
{
  പൂർണ്ണസംഖ്യ fd;

  ചെയ്യുക
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      എങ്കിൽ (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  എങ്കിൽ (open_temp_exec_file_opts_next ())
	    നിർത്തുക;
	}
    }
  ആകുമ്പോൾ (fd == -1);

  തിരികെ fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
allocate_space (പൂർണ്ണസംഖ്യ fd, off_t len)
{
  സ്ഥിതം long page_size;

  /* Obtain system page size. */
  എങ്കിൽ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  എങ്കിൽ (page_size <= 0)
    തിരികെ -1;

  unsigned അക്ഷരം buf[page_size];
  memset (buf, 0, page_size);

  ആകുമ്പോൾ (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      എങ്കിൽ (ലിഖ (fd, buf, to_write) < to_write)
        തിരികെ -1;
      len -= to_write;
    }

  തിരികെ 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
സ്ഥിതം ശൂന്യം *
dlmmap_locked (ശൂന്യം *start, size_t length, പൂർണ്ണസംഖ്യ prot, പൂർണ്ണസംഖ്യ flags, off_t offset)
{
  ശൂന്യം *ptr;

  എങ്കിൽ (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      എങ്കിൽ (execfd == -1)
	തിരികെ MFAIL;
    }

  offset = execsize;

  എങ്കിൽ (allocate_space (execfd, length))
    തിരികെ MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ശൂന്യ, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  എങ്കിൽ (ptr == MFAIL)
    {
      എങ്കിൽ (!offset)
	{
	  close (execfd);
	  പോകുക retry_open;
	}
      എങ്കിൽ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      തിരികെ MFAIL;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  എങ്കിൽ (start == MFAIL)
    {
      munmap (ptr, length);
      എങ്കിൽ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      തിരികെ start;
    }

  mmap_exec_offset ((അക്ഷരം *)start, length) = (അക്ഷരം*)ptr - (അക്ഷരം*)start;

  execsize += length;

  തിരികെ start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
സ്ഥിതം ശൂന്യം *
dlmmap (ശൂന്യം *start, size_t length, പൂർണ്ണസംഖ്യ prot,
	പൂർണ്ണസംഖ്യ flags, പൂർണ്ണസംഖ്യ fd, off_t offset)
{
  ശൂന്യം *ptr;

  പ്രതിജ്ഞാ (start == ശൂന്യ && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  എങ്കിൽ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      തിരികെ ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  എങ്കിൽ (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      എങ്കിൽ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          തിരികെ ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  അല്ലെങ്കിൽ എങ്കിൽ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      എങ്കിൽ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	തിരികെ ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  തിരികെ ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
dlmunmap (ശൂന്യം *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  ശൂന്യം *code;

  എങ്കിൽ (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      പൂർണ്ണസംഖ്യ ret = munmap (code, length);
      എങ്കിൽ (ret)
	തിരികെ ret;
    }

  തിരികെ munmap (start, length);
}

#എങ്കിൽ FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
സ്ഥിതം msegmentptr
segment_holding_code (mstate m, അക്ഷരം* addr)
{
  msegmentptr sp = &m->seg;
  വേണ്ടി (;;) {
    എങ്കിൽ (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      തിരികെ sp;
    എങ്കിൽ ((sp = sp->next) == 0)
      തിരികെ 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
ശൂന്യം *
ffi_closure_alloc (size_t size, ശൂന്യം **code)
{
  ശൂന്യം *ptr, *ftramp;

  എങ്കിൽ (!code)
    തിരികെ ശൂന്യ;

  ptr = dlmalloc (size);

  എങ്കിൽ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      എങ്കിൽ (!ffi_tramp_is_supported ())
        തിരികെ ptr;

      ftramp = ffi_tramp_alloc (0);
      എങ്കിൽ (ftramp == ശൂന്യ)
      {
        dlfree (ptr);
        തിരികെ ശൂന്യ;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  തിരികെ ptr;
}

ശൂന്യം *
ffi_data_to_code_pointer (ശൂന്യം *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  എങ്കിൽ (seg)
    {
      എങ്കിൽ (!ffi_tramp_is_supported ())
        തിരികെ add_segment_exec_offset (data, seg);
      തിരികെ ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  അല്ലെങ്കിൽ
    തിരികെ data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
ശൂന്യം
ffi_closure_free (ശൂന്യം *ptr)
{
#എങ്കിൽ FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  എങ്കിൽ (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  എങ്കിൽ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

പൂർണ്ണസംഖ്യ
ffi_tramp_is_present (ശൂന്യം *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  തിരികെ seg != ശൂന്യ && ffi_tramp_is_supported();
}

# അല്ലെങ്കിൽ /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#അന്തര്ഭാവ <stdlib.h>

ശൂന്യം *
ffi_closure_alloc (size_t size, ശൂന്യം **code)
{
  ശൂന്യം *c;

  എങ്കിൽ (!code)
    തിരികെ ശൂന്യ;

  c = malloc (size);
  *code = FFI_FN (c);
  തിരികെ c;
}

ശൂന്യം
ffi_closure_free (ശൂന്യം *ptr)
{
  സ്വതന്ത്രം (ptr);
}

ശൂന്യം *
ffi_data_to_code_pointer (ശൂന്യം *data)
{
  തിരികെ data;
}

പൂർണ്ണസംഖ്യ
ffi_tramp_is_present (__attribute__((unused)) ശൂന്യം *ptr)
{
  തിരികെ 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

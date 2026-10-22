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

#যদি (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#অন্তর্ভাৱ <fficonfig.h>
#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>
#অন্তর্ভাৱ <tramp.h>

#ifdef __NetBSD__
#অন্তর্ভাৱ <sys/param.h>
#endif

#যদি __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#অন্তর্ভাৱ <sys/mman.h>

#অন্তর্ভাৱ <stddef.h>
#অন্তর্ভাৱ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#অন্তর্ভাৱ <sys/memfd.h>
#endif

স্থির ধ্রুৱ size_t overhead =
  (পরিমাণ(max_align_t) > পরিমাণ(শূন্য *) + পরিমাণ(size_t)) ?
    পরিমাণ(max_align_t)
    : পরিমাণ(শূন্য *) + পরিমাণ(size_t);

#define ADD_TO_POINTER(p, d) ((শূন্য *)((uintptr_t)(p) + (d)))

শূন্য *
ffi_closure_alloc (size_t size, শূন্য **code)
{
  স্থির size_t page_size;
  size_t rounded_size;
  শূন্য *codeseg, *dataseg;
  পূর্ণাঙ্ক prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  যদি (!code)
    ঘূৰাই রিক্ত;

  /* Obtain system page size. */
  যদি (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(রিক্ত, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  যদি (dataseg == MAP_FAILED)
    ঘূৰাই রিক্ত;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, রিক্ত, rounded_size, MAP_REMAPDUP);
  যদি (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    ঘূৰাই রিক্ত;
  }
  যদি (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    ঘূৰাই রিক্ত;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, পরিমাণ(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, পরিমাণ(size_t)), &codeseg, পরিমাণ(শূন্য *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  ঘূৰাই ADD_TO_POINTER(dataseg, overhead);
}

শূন্য
ffi_closure_free (শূন্য *ptr)
{
  শূন্য *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, পরিমাণ(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, পরিমাণ(size_t)), পরিমাণ(শূন্য *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

পূর্ণাঙ্ক
ffi_tramp_is_present (__attribute__((unused)) শূন্য *ptr)
{
  ঘূৰাই 0;
}
#অন্যথা /* !NetBSD with PROT_MPROTECT */

#যদি !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# যদি __linux__ && !defined(__ANDROID__)
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
# যদি defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#যদি FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# যদি !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# যদি !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#যদি FFI_CLOSURES

#যদি FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#অন্তর্ভাৱ <mach/mach.h>
#অন্তর্ভাৱ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#অন্তর্ভাৱ <ptrauth.h>
#endif
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>

বাহ্য শূন্য *ffi_closure_trampoline_table_page;

প্রকারনির্দেশ সংরচনা ffi_trampoline_table ffi_trampoline_table;
প্রকারনির্দেশ সংরচনা ffi_trampoline_table_entry ffi_trampoline_table_entry;

সংরচনা ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_গুনতি;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

সংরচনা ffi_trampoline_table_entry
{
  শূন্য *(*trampoline) (শূন্য);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_গুনতি (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

স্থির pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
স্থির ffi_trampoline_table *ffi_trampoline_tables = রিক্ত;

স্থির ffi_trampoline_table *
ffi_trampoline_table_alloc (শূন্য)
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
  যদি (kt != KERN_SUCCESS)
    ঘূৰাই রিক্ত;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((শূন্য *)&ffi_closure_trampoline_table_page, ptrauth_চাবি_function_pointer, 0);
#অন্যথা
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  যদি (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ঘূৰাই রিক্ত;
    }

  যদি (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      যদি (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          ঘূৰাই রিক্ত;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, পরিমাণ (ffi_trampoline_table));
  যদি (table == রিক্ত)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ঘূৰাই রিক্ত;
    }
  table->free_গুনতি = FFI_TRAMPOLINE_গুনতি;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_গুনতি, পরিমাণ (ffi_trampoline_table_entry));
  যদি (table->free_list_pool == রিক্ত)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      মুক্ত (table);
      ঘূৰাই রিক্ত;
    }

  বাবে (i = 0; i < table->free_গুনতি; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(শূন্য *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_চাবি_function_pointer, 0);
#endif

      যদি (i < table->free_গুনতি - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  ঘূৰাই table;
}

স্থির শূন্য
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  যদি (table->prev != রিক্ত)
    table->prev->next = table->next;

  যদি (table->next != রিক্ত)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  মুক্ত (table->free_list_pool);
  মুক্ত (table);
}

শূন্য *
ffi_closure_alloc (size_t size, শূন্য **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  যদি (closure == রিক্ত)
    ঘূৰাই রিক্ত;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  যদি (table == রিক্ত || table->free_list == রিক্ত)
    {
      table = ffi_trampoline_table_alloc ();
      যদি (table == রিক্ত)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  মুক্ত (closure);
	  ঘূৰাই রিক্ত;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      যদি (table->next != রিক্ত)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_গুনতি--;
  entry->next = রিক্ত;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  ঘূৰাই closure;
}

শূন্য
ffi_closure_free (শূন্য *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_গুনতি++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  যদি (table->free_গুনতি == FFI_TRAMPOLINE_গুনতি
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  অন্যথা যদি (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = রিক্ত;
      table->next = ffi_trampoline_tables;
      যদি (ffi_trampoline_tables != রিক্ত)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  মুক্ত (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#নোচেদ্যদি FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/stat.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <errno.h>
#ifndef _MSC_VER
#অন্তর্ভাৱ <unistd.h>
#endif
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <stdio.h>
#যদি !defined(_WIN32)
#ifdef HAVE_MNTENT
#অন্তর্ভাৱ <mntent.h>
#endif /* HAVE_MNTENT */
#অন্তর্ভাৱ <sys/param.h>
#অন্তর্ভাৱ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#অন্তর্ভাৱ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#যদি FFI_MMAP_EXEC_SELINUX
#অন্তর্ভাৱ <sys/statfs.h>
#অন্তর্ভাৱ <stdlib.h>

স্থির পূর্ণাঙ্ক selinux_enabled = -1;

স্থির পূর্ণাঙ্ক
selinux_enabled_check (শূন্য)
{
  সংরচনা statfs sfs;
  FILE *f;
  ৱর্ণ *buf = রিক্ত;
  size_t len = 0;

  যদি (statfs ("/selinux", &sfs) >= 0
      && (unsigned পূর্ণাঙ্ক) sfs.f_type == 0xf97cff8cU)
    ঘূৰাই 1;
  f = fopen ("/proc/mounts", "r");
  যদি (f == রিক্ত)
    ঘূৰাই 0;
  যেতিয়ালৈ (getline (&buf, &len, f) >= 0)
    {
      ৱর্ণ *p = strchr (buf, ' ');
      যদি (p == রিক্ত)
        ভঙ্গ;
      p = strchr (p + 1, ' ');
      যদি (p == রিক্ত)
        ভঙ্গ;
      যদি (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          মুক্ত (buf);
          fclose (f);
          ঘূৰাই 1;
        }
    }
  মুক্ত (buf);
  fclose (f);
  ঘূৰাই 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#অন্যথা

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#যদি defined FFI_MMAP_PAX
#অন্তর্ভাৱ <stdlib.h>

গণনা {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
স্থির পূর্ণাঙ্ক cached_pax_flags = -1;

স্থির পূর্ণাঙ্ক
pax_flags_check (শূন্য)
{
  ৱর্ণ *buf = রিক্ত;
  size_t len = 0;
  FILE *f;
  পূর্ণাঙ্ক ret;
  f = fopen ("/proc/self/status", "r");
  যদি (f == রিক্ত)
    ঘূৰাই 0;
  ret = 0;

  যেতিয়ালৈ (getline (&buf, &len, f) != -1)
    যদি (!strncmp (buf, "PaX:", 4))
      {
        যদি (রিক্ত != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        যদি (রিক্ত != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ভঙ্গ;
      }
  মুক্ত (buf);
  fclose (f);
  ঘূৰাই ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#নোচেদ্যদি defined (__CYGWIN__) || defined(__INTERIX)

#অন্তর্ভাৱ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#যদি !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
স্থির শূন্য *dlmalloc(size_t);
স্থির শূন্য dlfree(শূন্য*);
স্থির শূন্য *dlcalloc(size_t, size_t) MAYBE_UNUSED;
স্থির শূন্য *dlrealloc(শূন্য *, size_t) MAYBE_UNUSED;
স্থির শূন্য *dlmemalign(size_t, size_t) MAYBE_UNUSED;
স্থির শূন্য *dlvalloc(size_t) MAYBE_UNUSED;
স্থির পূর্ণাঙ্ক dlmallopt(পূর্ণাঙ্ক, পূর্ণাঙ্ক) MAYBE_UNUSED;
স্থির size_t dlmalloc_footprint(শূন্য) MAYBE_UNUSED;
স্থির size_t dlmalloc_max_footprint(শূন্য) MAYBE_UNUSED;
স্থির শূন্য** dlindependent_calloc(size_t, size_t, শূন্য**) MAYBE_UNUSED;
স্থির শূন্য** dlindependent_comalloc(size_t, size_t*, শূন্য**) MAYBE_UNUSED;
স্থির শূন্য *dlpvalloc(size_t) MAYBE_UNUSED;
স্থির পূর্ণাঙ্ক dlmalloc_trim(size_t) MAYBE_UNUSED;
স্থির size_t dlmalloc_usable_size(শূন্য*) MAYBE_UNUSED;
স্থির শূন্য dlmalloc_stats(শূন্য) MAYBE_UNUSED;

#যদি !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
স্থির শূন্য *dlmmap(শূন্য *, size_t, পূর্ণাঙ্ক, পূর্ণাঙ্ক, পূর্ণাঙ্ক, off_t);
স্থির পূর্ণাঙ্ক dlmunmap(শূন্য *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#অন্তর্ভাৱ "dlmalloc.c"

#undef mmap
#undef munmap

#যদি !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
স্থির pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
স্থির পূর্ণাঙ্ক execfd = -1;

/* The amount of space already allocated from the temporary file.  */
স্থির size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_memfd (ধ্রুৱ ৱর্ণ *name)
{
  পূর্ণাঙ্ক fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ঘূৰাই fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_name (ৱর্ণ *name, পূর্ণাঙ্ক flags MAYBE_UNUSED)
{
  পূর্ণাঙ্ক fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#অন্যথা
  fd = mkstemp (name);
#endif

  যদি (fd != -1)
    unlink (name);

  ঘূৰাই fd;
}

/* Open a temporary file in the named directory.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_dir (ধ্রুৱ ৱর্ণ *dir)
{
  স্থির ধ্রুৱ ৱর্ণ suffix[] = "/ffiXXXXXX";
  পূর্ণাঙ্ক lendir, flags;
  ৱর্ণ *tempname;
#ifdef O_TMPFILE
  পূর্ণাঙ্ক fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#অন্যথা
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  যদি (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ঘূৰাই fd;
  } অন্যথা {
    errno = 0;
  }
#endif

  lendir = (পূর্ণাঙ্ক) strlen (dir);
  tempname = __builtin_alloca (lendir + পরিমাণ (suffix));

  যদি (!tempname)
    ঘূৰাই -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, পরিমাণ (suffix));

  ঘূৰাই open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_env (ধ্রুৱ ৱর্ণ *envvar)
{
  ধ্রুৱ ৱর্ণ *মান = getenv (envvar);

  যদি (!মান)
    ঘূৰাই -1;

  ঘূৰাই open_temp_exec_file_dir (মান);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_mnt (ধ্রুৱ ৱর্ণ *mounts)
{
  স্থির ধ্রুৱ ৱর্ণ *last_mounts;
  স্থির FILE *last_mntent;

  যদি (mounts != last_mounts)
    {
      যদি (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      যদি (mounts)
	last_mntent = setmntent (mounts, "r");
      অন্যথা
	last_mntent = রিক্ত;
    }

  যদি (!last_mntent)
    ঘূৰাই -1;

  বাবে (;;)
    {
      পূর্ণাঙ্ক fd;
      সংরচনা mntent mnt;
      ৱর্ণ buf[MAXPATHLEN * 3];

      যদি (getmntent_r (last_mntent, &mnt, buf, পরিমাণ (buf)) == রিক্ত)
	ঘূৰাই -1;

      যদি (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	অনুৱর্ত;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      যদি (fd != -1)
	ঘূৰাই fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
স্থির সংরচনা
{
  পূর্ণাঙ্ক (*func)(ধ্রুৱ ৱর্ণ *);
  ধ্রুৱ ৱর্ণ *arg;
  পূর্ণাঙ্ক repeat;
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
স্থির পূর্ণাঙ্ক open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
স্থির পূর্ণাঙ্ক
open_temp_exec_file_opts_next (শূন্য)
{
  যদি (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (রিক্ত);

  open_temp_exec_file_opts_idx++;
  যদি (open_temp_exec_file_opts_idx
      == (পরিমাণ (open_temp_exec_file_opts)
	  / পরিমাণ (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      ঘূৰাই 1;
    }

  ঘূৰাই 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
পূর্ণাঙ্ক
open_temp_exec_file (শূন্য)
{
  পূর্ণাঙ্ক fd;

  কৰা
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      যদি (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  যদি (open_temp_exec_file_opts_next ())
	    ভঙ্গ;
	}
    }
  যেতিয়ালৈ (fd == -1);

  ঘূৰাই fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
স্থির পূর্ণাঙ্ক
allocate_space (পূর্ণাঙ্ক fd, off_t len)
{
  স্থির long page_size;

  /* Obtain system page size. */
  যদি (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  যদি (page_size <= 0)
    ঘূৰাই -1;

  unsigned ৱর্ণ buf[page_size];
  memset (buf, 0, page_size);

  যেতিয়ালৈ (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      যদি (লিখ (fd, buf, to_write) < to_write)
        ঘূৰাই -1;
      len -= to_write;
    }

  ঘূৰাই 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
স্থির শূন্য *
dlmmap_locked (শূন্য *start, size_t length, পূর্ণাঙ্ক prot, পূর্ণাঙ্ক flags, off_t offset)
{
  শূন্য *ptr;

  যদি (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      যদি (execfd == -1)
	ঘূৰাই MFAIL;
    }

  offset = execsize;

  যদি (allocate_space (execfd, length))
    ঘূৰাই MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (রিক্ত, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  যদি (ptr == MFAIL)
    {
      যদি (!offset)
	{
	  close (execfd);
	  গচ্ছ retry_open;
	}
      যদি (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ঘূৰাই MFAIL;
    }
  অন্যথা যদি (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  যদি (start == MFAIL)
    {
      munmap (ptr, length);
      যদি (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ঘূৰাই start;
    }

  mmap_exec_offset ((ৱর্ণ *)start, length) = (ৱর্ণ*)ptr - (ৱর্ণ*)start;

  execsize += length;

  ঘূৰাই start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
স্থির শূন্য *
dlmmap (শূন্য *start, size_t length, পূর্ণাঙ্ক prot,
	পূর্ণাঙ্ক flags, পূর্ণাঙ্ক fd, off_t offset)
{
  শূন্য *ptr;

  প্রতিজ্ঞা (start == রিক্ত && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  যদি (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      ঘূৰাই ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  যদি (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      যদি (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          ঘূৰাই ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  অন্যথা যদি (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      যদি (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ঘূৰাই ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  ঘূৰাই ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
স্থির পূর্ণাঙ্ক
dlmunmap (শূন্য *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  শূন্য *code;

  যদি (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      পূর্ণাঙ্ক ret = munmap (code, length);
      যদি (ret)
	ঘূৰাই ret;
    }

  ঘূৰাই munmap (start, length);
}

#যদি FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
স্থির msegmentptr
segment_holding_code (mstate m, ৱর্ণ* addr)
{
  msegmentptr sp = &m->seg;
  বাবে (;;) {
    যদি (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      ঘূৰাই sp;
    যদি ((sp = sp->next) == 0)
      ঘূৰাই 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
শূন্য *
ffi_closure_alloc (size_t size, শূন্য **code)
{
  শূন্য *ptr, *ftramp;

  যদি (!code)
    ঘূৰাই রিক্ত;

  ptr = dlmalloc (size);

  যদি (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      যদি (!ffi_tramp_is_supported ())
        ঘূৰাই ptr;

      ftramp = ffi_tramp_alloc (0);
      যদি (ftramp == রিক্ত)
      {
        dlfree (ptr);
        ঘূৰাই রিক্ত;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  ঘূৰাই ptr;
}

শূন্য *
ffi_data_to_code_pointer (শূন্য *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  যদি (seg)
    {
      যদি (!ffi_tramp_is_supported ())
        ঘূৰাই add_segment_exec_offset (data, seg);
      ঘূৰাই ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  অন্যথা
    ঘূৰাই data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
শূন্য
ffi_closure_free (শূন্য *ptr)
{
#যদি FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  যদি (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  যদি (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

পূর্ণাঙ্ক
ffi_tramp_is_present (শূন্য *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ঘূৰাই seg != রিক্ত && ffi_tramp_is_supported();
}

# অন্যথা /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#অন্তর্ভাৱ <stdlib.h>

শূন্য *
ffi_closure_alloc (size_t size, শূন্য **code)
{
  শূন্য *c;

  যদি (!code)
    ঘূৰাই রিক্ত;

  c = malloc (size);
  *code = FFI_FN (c);
  ঘূৰাই c;
}

শূন্য
ffi_closure_free (শূন্য *ptr)
{
  মুক্ত (ptr);
}

শূন্য *
ffi_data_to_code_pointer (শূন্য *data)
{
  ঘূৰাই data;
}

পূর্ণাঙ্ক
ffi_tramp_is_present (__attribute__((unused)) শূন্য *ptr)
{
  ঘূৰাই 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

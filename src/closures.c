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

#অন্তর্ভাব <fficonfig.h>
#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <tramp.h>

#ifdef __NetBSD__
#অন্তর্ভাব <sys/param.h>
#endif

#যদি __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#অন্তর্ভাব <sys/mman.h>

#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#অন্তর্ভাব <sys/memfd.h>
#endif

স্থির ধ্রুবক size_t overhead =
  (আকার(max_align_t) > আকার(শূন্য *) + আকার(size_t)) ?
    আকার(max_align_t)
    : আকার(শূন্য *) + আকার(size_t);

#define ADD_TO_POINTER(p, d) ((শূন্য *)((uintptr_t)(p) + (d)))

শূন্য *
ffi_closure_alloc (size_t আকার_2, শূন্য **code)
{
  স্থির size_t page_আকার;
  size_t rounded_আকার;
  শূন্য *codeseg, *dataseg;
  পূর্ণসংখ্যা prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  যদি (!code)
    ফেরত ফাঁকা;

  /* Obtain system page size. */
  যদি (!page_আকার)
    page_আকার = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_আকার = (আকার_2 + overhead + page_আকার - 1) & ~(page_আকার - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ফাঁকা, rounded_আকার, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  যদি (dataseg == MAP_FAILED)
    ফেরত ফাঁকা;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_আকার, ফাঁকা, rounded_আকার, MAP_REMAPDUP);
  যদি (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_আকার);
    ফেরত ফাঁকা;
  }
  যদি (mprotect(codeseg, rounded_আকার, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_আকার);
    munmap(dataseg, rounded_আকার);
    ফেরত ফাঁকা;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_আকার, আকার(rounded_আকার));
  memcpy(ADD_TO_POINTER(dataseg, আকার(size_t)), &codeseg, আকার(শূন্য *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  ফেরত ADD_TO_POINTER(dataseg, overhead);
}

শূন্য
ffi_closure_free (শূন্য *ptr)
{
  শূন্য *codeseg, *dataseg;
  size_t rounded_আকার;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_আকার, dataseg, আকার(rounded_আকার));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, আকার(size_t)), আকার(শূন্য *));
  munmap(dataseg, rounded_আকার);
  munmap(codeseg, rounded_আকার);
}

পূর্ণসংখ্যা
ffi_tramp_is_present (__attribute__((unused)) শূন্য *ptr)
{
  ফেরত 0;
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

#অন্তর্ভাব <mach/mach.h>
#অন্তর্ভাব <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#অন্তর্ভাব <ptrauth.h>
#endif
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>

বাহ্যিক শূন্য *ffi_closure_trampoline_table_page;

প্রকারলেখ কাঠামো ffi_trampoline_table ffi_trampoline_table;
প্রকারলেখ কাঠামো ffi_trampoline_table_entry ffi_trampoline_table_entry;

কাঠামো ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_গুনতি;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *পূর্ব;
  ffi_trampoline_table *পরবর্তী;
};

কাঠামো ffi_trampoline_table_entry
{
  শূন্য *(*trampoline) (শূন্য);
  ffi_trampoline_table_entry *পরবর্তী;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_গুনতি (PAGE_MAX_আকার / FFI_TRAMPOLINE_আকার)

স্থির pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
স্থির ffi_trampoline_table *ffi_trampoline_tables = ফাঁকা;

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
  kt = vm_allocate (mach_task_self (), &config_page, PAGE_MAX_আকার * 2,
		    VM_FLAGS_ANYWHERE);
  যদি (kt != KERN_SUCCESS)
    ফেরত ফাঁকা;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_আকার;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_তথ্য((শূন্য *)&ffi_closure_trampoline_table_page, ptrauth_চাবি_function_pointer, 0);
#অন্যথা
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_আকার, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  যদি (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_আকার * 2);
      ফেরত ফাঁকা;
    }

  যদি (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_আকার,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      যদি (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_আকার * 2);
          ফেরত ফাঁকা;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, আকার (ffi_trampoline_table));
  যদি (table == ফাঁকা)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_আকার * 2);
      ফেরত ফাঁকা;
    }
  table->free_গুনতি = FFI_TRAMPOLINE_গুনতি;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_গুনতি, আকার (ffi_trampoline_table_entry));
  যদি (table->free_list_pool == ফাঁকা)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_আকার * 2);
      মুক্ত (table);
      ফেরত ফাঁকা;
    }

  জন্য (i = 0; i < table->free_গুনতি; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(শূন্য *) (trampoline_page + (i * FFI_TRAMPOLINE_আকার));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_চাবি_function_pointer, 0);
#endif

      যদি (i < table->free_গুনতি - 1)
	entry->পরবর্তী = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  ফেরত table;
}

স্থির শূন্য
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  যদি (table->পূর্ব != ফাঁকা)
    table->পূর্ব->পরবর্তী = table->পরবর্তী;

  যদি (table->পরবর্তী != ফাঁকা)
    table->পরবর্তী->পূর্ব = table->পূর্ব;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_আকার * 2);

  /* Deallocate free list */
  মুক্ত (table->free_list_pool);
  মুক্ত (table);
}

শূন্য *
ffi_closure_alloc (size_t আকার_2, শূন্য **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (আকার_2);
  যদি (closure == ফাঁকা)
    ফেরত ফাঁকা;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  যদি (table == ফাঁকা || table->free_list == ফাঁকা)
    {
      table = ffi_trampoline_table_alloc ();
      যদি (table == ফাঁকা)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  মুক্ত (closure);
	  ফেরত ফাঁকা;
	}

      /* Insert the new table at the top of the list */
      table->পরবর্তী = ffi_trampoline_tables;
      যদি (table->পরবর্তী != ফাঁকা)
	table->পরবর্তী->পূর্ব = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->পরবর্তী;
  ffi_trampoline_tables->free_গুনতি--;
  entry->পরবর্তী = ফাঁকা;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  ফেরত closure;
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
  entry->পরবর্তী = table->free_list;
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
      table->পূর্ব = ফাঁকা;
      table->পরবর্তী = ffi_trampoline_tables;
      যদি (ffi_trampoline_tables != ফাঁকা)
	ffi_trampoline_tables->পূর্ব = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  মুক্ত (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#নতুবা_যদি FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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
#define DEFAULT_MMAP_THRESHOLD MAX_আকার_T

/* Don't allocate more than a page unless needed.  */
#define DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <fcntl.h>
#অন্তর্ভাব <errno.h>
#ifndef _MSC_VER
#অন্তর্ভাব <unistd.h>
#endif
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <stdio.h>
#যদি !defined(_WIN32)
#ifdef HAVE_MNTENT
#অন্তর্ভাব <mntent.h>
#endif /* HAVE_MNTENT */
#অন্তর্ভাব <sys/param.h>
#অন্তর্ভাব <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#অন্তর্ভাব <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#যদি FFI_MMAP_EXEC_SELINUX
#অন্তর্ভাব <sys/statfs.h>
#অন্তর্ভাব <stdlib.h>

স্থির পূর্ণসংখ্যা selinux_enabled = -1;

স্থির পূর্ণসংখ্যা
selinux_enabled_check (শূন্য)
{
  কাঠামো statfs sfs;
  FILE *f;
  অক্ষর *buf = ফাঁকা;
  size_t len = 0;

  যদি (statfs ("/selinux", &sfs) >= 0
      && (unsigned পূর্ণসংখ্যা) sfs.f_type == 0xf97cff8cU)
    ফেরত 1;
  f = fopen ("/proc/mounts", "r");
  যদি (f == ফাঁকা)
    ফেরত 0;
  যখন (getline (&buf, &len, f) >= 0)
    {
      অক্ষর *p = strchr (buf, ' ');
      যদি (p == ফাঁকা)
        বিরতি;
      p = strchr (p + 1, ' ');
      যদি (p == ফাঁকা)
        বিরতি;
      যদি (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          মুক্ত (buf);
          fclose (f);
          ফেরত 1;
        }
    }
  মুক্ত (buf);
  fclose (f);
  ফেরত 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#অন্যথা

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#যদি defined FFI_MMAP_PAX
#অন্তর্ভাব <stdlib.h>

গণনা {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
স্থির পূর্ণসংখ্যা cached_pax_flags = -1;

স্থির পূর্ণসংখ্যা
pax_flags_check (শূন্য)
{
  অক্ষর *buf = ফাঁকা;
  size_t len = 0;
  FILE *f;
  পূর্ণসংখ্যা ret;
  f = fopen ("/proc/self/status", "r");
  যদি (f == ফাঁকা)
    ফেরত 0;
  ret = 0;

  যখন (getline (&buf, &len, f) != -1)
    যদি (!strncmp (buf, "PaX:", 4))
      {
        যদি (ফাঁকা != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        যদি (ফাঁকা != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        বিরতি;
      }
  মুক্ত (buf);
  fclose (f);
  ফেরত ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#নতুবা_যদি defined (__CYGWIN__) || defined(__INTERIX)

#অন্তর্ভাব <sys/mman.h>

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
স্থির পূর্ণসংখ্যা dlmallopt(পূর্ণসংখ্যা, পূর্ণসংখ্যা) MAYBE_UNUSED;
স্থির size_t dlmalloc_footprint(শূন্য) MAYBE_UNUSED;
স্থির size_t dlmalloc_max_footprint(শূন্য) MAYBE_UNUSED;
স্থির শূন্য** dlindependent_calloc(size_t, size_t, শূন্য**) MAYBE_UNUSED;
স্থির শূন্য** dlindependent_comalloc(size_t, size_t*, শূন্য**) MAYBE_UNUSED;
স্থির শূন্য *dlpvalloc(size_t) MAYBE_UNUSED;
স্থির পূর্ণসংখ্যা dlmalloc_trim(size_t) MAYBE_UNUSED;
স্থির size_t dlmalloc_usable_আকার(শূন্য*) MAYBE_UNUSED;
স্থির শূন্য dlmalloc_stats(শূন্য) MAYBE_UNUSED;

#যদি !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
স্থির শূন্য *dlmmap(শূন্য *, size_t, পূর্ণসংখ্যা, পূর্ণসংখ্যা, পূর্ণসংখ্যা, off_t);
স্থির পূর্ণসংখ্যা dlmunmap(শূন্য *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#অন্তর্ভাব "dlmalloc.c"

#undef mmap
#undef munmap

#যদি !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
স্থির pthread_mutex_t open_অস্থায়ী_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
স্থির পূর্ণসংখ্যা execfd = -1;

/* The amount of space already allocated from the temporary file.  */
স্থির size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_memfd (ধ্রুবক অক্ষর *name)
{
  পূর্ণসংখ্যা fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ফেরত fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_name (অক্ষর *name, পূর্ণসংখ্যা flags MAYBE_UNUSED)
{
  পূর্ণসংখ্যা fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#অন্যথা
  fd = mkstemp (name);
#endif

  যদি (fd != -1)
    unlink (name);

  ফেরত fd;
}

/* Open a temporary file in the named directory.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_dir (ধ্রুবক অক্ষর *dir)
{
  স্থির ধ্রুবক অক্ষর suffix[] = "/ffiXXXXXX";
  পূর্ণসংখ্যা lendir, flags;
  অক্ষর *tempname;
#ifdef O_TMPFILE
  পূর্ণসংখ্যা fd;
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
    ফেরত fd;
  } অন্যথা {
    errno = 0;
  }
#endif

  lendir = (পূর্ণসংখ্যা) strlen (dir);
  tempname = __builtin_alloca (lendir + আকার (suffix));

  যদি (!tempname)
    ফেরত -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, আকার (suffix));

  ফেরত open_অস্থায়ী_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_env (ধ্রুবক অক্ষর *envvar)
{
  ধ্রুবক অক্ষর *মান = getenv (envvar);

  যদি (!মান)
    ফেরত -1;

  ফেরত open_অস্থায়ী_exec_file_dir (মান);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_mnt (ধ্রুবক অক্ষর *mounts)
{
  স্থির ধ্রুবক অক্ষর *last_mounts;
  স্থির FILE *last_mntent;

  যদি (mounts != last_mounts)
    {
      যদি (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      যদি (mounts)
	last_mntent = setmntent (mounts, "r");
      অন্যথা
	last_mntent = ফাঁকা;
    }

  যদি (!last_mntent)
    ফেরত -1;

  জন্য (;;)
    {
      পূর্ণসংখ্যা fd;
      কাঠামো mntent mnt;
      অক্ষর buf[MAXPATHLEN * 3];

      যদি (getmntent_r (last_mntent, &mnt, buf, আকার (buf)) == ফাঁকা)
	ফেরত -1;

      যদি (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	চালু;

      fd = open_অস্থায়ী_exec_file_dir (mnt.mnt_dir);

      যদি (fd != -1)
	ফেরত fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
স্থির কাঠামো
{
  পূর্ণসংখ্যা (*func)(ধ্রুবক অক্ষর *);
  ধ্রুবক অক্ষর *arg;
  পূর্ণসংখ্যা repeat;
} open_অস্থায়ী_exec_file_opts[] = {
#ifdef HAVE_MEMFD_CREATE
  { open_অস্থায়ী_exec_file_memfd, "libffi", 0 },
#endif
  { open_অস্থায়ী_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_অস্থায়ী_exec_file_env, "TMPDIR", 0 },
  { open_অস্থায়ী_exec_file_dir, "/tmp", 0 },
  { open_অস্থায়ী_exec_file_dir, "/var/tmp", 0 },
  { open_অস্থায়ী_exec_file_dir, "/dev/shm", 0 },
  { open_অস্থায়ী_exec_file_env, "HOME", 0 },
#ifdef HAVE_MNTENT
  { open_অস্থায়ী_exec_file_mnt, "/etc/mtab", 1 },
  { open_অস্থায়ী_exec_file_mnt, "/proc/mounts", 1 },
#endif /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
স্থির পূর্ণসংখ্যা open_অস্থায়ী_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
স্থির পূর্ণসংখ্যা
open_অস্থায়ী_exec_file_opts_পরবর্তী (শূন্য)
{
  যদি (open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].repeat)
    open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].func (ফাঁকা);

  open_অস্থায়ী_exec_file_opts_idx++;
  যদি (open_অস্থায়ী_exec_file_opts_idx
      == (আকার (open_অস্থায়ী_exec_file_opts)
	  / আকার (*open_অস্থায়ী_exec_file_opts)))
    {
      open_অস্থায়ী_exec_file_opts_idx = 0;
      ফেরত 1;
    }

  ফেরত 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
পূর্ণসংখ্যা
open_অস্থায়ী_exec_file (শূন্য)
{
  পূর্ণসংখ্যা fd;

  করো
    {
      fd = open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].func
	(open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].arg);

      যদি (!open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  যদি (open_অস্থায়ী_exec_file_opts_পরবর্তী ())
	    বিরতি;
	}
    }
  যখন (fd == -1);

  ফেরত fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
স্থির পূর্ণসংখ্যা
allocate_space (পূর্ণসংখ্যা fd, off_t len)
{
  স্থির long page_আকার;

  /* Obtain system page size. */
  যদি (!page_আকার)
    page_আকার = sysconf(_SC_PAGESIZE);
  যদি (page_আকার <= 0)
    ফেরত -1;

  unsigned অক্ষর buf[page_আকার];
  memset (buf, 0, page_আকার);

  যখন (len > 0)
    {
      off_t to_write = (len < page_আকার) ? len : page_আকার;
      যদি (লিখ (fd, buf, to_write) < to_write)
        ফেরত -1;
      len -= to_write;
    }

  ফেরত 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
স্থির শূন্য *
dlmmap_locked (শূন্য *start, size_t দৈর্ঘ্য, পূর্ণসংখ্যা prot, পূর্ণসংখ্যা flags, off_t offset)
{
  শূন্য *ptr;

  যদি (execfd == -1)
    {
      open_অস্থায়ী_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_অস্থায়ী_exec_file ();
      যদি (execfd == -1)
	ফেরত MFAIL;
    }

  offset = execsize;

  যদি (allocate_space (execfd, দৈর্ঘ্য))
    ফেরত MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ফাঁকা, দৈর্ঘ্য, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  যদি (ptr == MFAIL)
    {
      যদি (!offset)
	{
	  close (execfd);
	  যাও retry_open;
	}
      যদি (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ফেরত MFAIL;
    }
  অন্যথা যদি (!offset
	   && open_অস্থায়ী_exec_file_opts[open_অস্থায়ী_exec_file_opts_idx].repeat)
    open_অস্থায়ী_exec_file_opts_পরবর্তী ();

  start = mmap (start, দৈর্ঘ্য, prot, flags, execfd, offset);

  যদি (start == MFAIL)
    {
      munmap (ptr, দৈর্ঘ্য);
      যদি (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ফেরত start;
    }

  mmap_exec_offset ((অক্ষর *)start, দৈর্ঘ্য) = (অক্ষর*)ptr - (অক্ষর*)start;

  execsize += দৈর্ঘ্য;

  ফেরত start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
স্থির শূন্য *
dlmmap (শূন্য *start, size_t দৈর্ঘ্য, পূর্ণসংখ্যা prot,
	পূর্ণসংখ্যা flags, পূর্ণসংখ্যা fd, off_t offset)
{
  শূন্য *ptr;

  প্রতিজ্ঞা (start == ফাঁকা && দৈর্ঘ্য % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  যদি (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, দৈর্ঘ্য, prot & ~PROT_EXEC, flags, fd, offset);
      ফেরত ptr;
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
          ptr = mmap (start, দৈর্ঘ্য, prot & ~PROT_EXEC, flags, fd, offset);
          ফেরত ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  অন্যথা যদি (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, দৈর্ঘ্য, prot | PROT_EXEC, flags, fd, offset);

      যদি (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ফেরত ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_অস্থায়ী_exec_file_mutex);
  ptr = dlmmap_locked (start, দৈর্ঘ্য, prot, flags, offset);
  pthread_mutex_unlock (&open_অস্থায়ী_exec_file_mutex);

  ফেরত ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
স্থির পূর্ণসংখ্যা
dlmunmap (শূন্য *start, size_t দৈর্ঘ্য)
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
      পূর্ণসংখ্যা ret = munmap (code, দৈর্ঘ্য);
      যদি (ret)
	ফেরত ret;
    }

  ফেরত munmap (start, দৈর্ঘ্য);
}

#যদি FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
স্থির msegmentptr
segment_holding_code (mstate m, অক্ষর* addr)
{
  msegmentptr sp = &m->seg;
  জন্য (;;) {
    যদি (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->আকার_2)
      ফেরত sp;
    যদি ((sp = sp->পরবর্তী) == 0)
      ফেরত 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
শূন্য *
ffi_closure_alloc (size_t আকার_2, শূন্য **code)
{
  শূন্য *ptr, *ftramp;

  যদি (!code)
    ফেরত ফাঁকা;

  ptr = dlmalloc (আকার_2);

  যদি (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      যদি (!ffi_tramp_is_supported ())
        ফেরত ptr;

      ftramp = ffi_tramp_alloc (0);
      যদি (ftramp == ফাঁকা)
      {
        dlfree (ptr);
        ফেরত ফাঁকা;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  ফেরত ptr;
}

শূন্য *
ffi_তথ্য_to_code_pointer (শূন্য *তথ্য)
{
  msegmentptr seg = segment_holding (gm, তথ্য);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  যদি (seg)
    {
      যদি (!ffi_tramp_is_supported ())
        ফেরত add_segment_exec_offset (তথ্য, seg);
      ফেরত ffi_tramp_get_addr (((ffi_closure *) তথ্য)->ftramp);
    }
  অন্যথা
    ফেরত তথ্য;
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

পূর্ণসংখ্যা
ffi_tramp_is_present (শূন্য *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ফেরত seg != ফাঁকা && ffi_tramp_is_supported();
}

# অন্যথা /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#অন্তর্ভাব <stdlib.h>

শূন্য *
ffi_closure_alloc (size_t আকার_2, শূন্য **code)
{
  শূন্য *c;

  যদি (!code)
    ফেরত ফাঁকা;

  c = malloc (আকার_2);
  *code = FFI_FN (c);
  ফেরত c;
}

শূন্য
ffi_closure_free (শূন্য *ptr)
{
  মুক্ত (ptr);
}

শূন্য *
ffi_তথ্য_to_code_pointer (শূন্য *তথ্য)
{
  ফেরত তথ্য;
}

পূর্ণসংখ্যা
ffi_tramp_is_present (__attribute__((unused)) শূন্য *ptr)
{
  ফেরত 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

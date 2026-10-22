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

#எனில் (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#அந்தர்பாவ <fficonfig.h>
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <tramp.h>

#ifdef __NetBSD__
#அந்தர்பாவ <sys/param.h>
#endif

#எனில் __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#அந்தர்பாவ <sys/mman.h>

#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#அந்தர்பாவ <sys/memfd.h>
#endif

நிலையான மாறிலி size_t overhead =
  (அளவு(max_align_t) > அளவு(வெறுமை *) + அளவு(size_t)) ?
    அளவு(max_align_t)
    : அளவு(வெறுமை *) + அளவு(size_t);

#define ADD_TO_POINTER(p, d) ((வெறுமை *)((uintptr_t)(p) + (d)))

வெறுமை *
ffi_closure_alloc (size_t size, வெறுமை **code)
{
  நிலையான size_t page_size;
  size_t rounded_size;
  வெறுமை *codeseg, *dataseg;
  முழு prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  எனில் (!code)
    திரும்பு வெற்று;

  /* Obtain system page size. */
  எனில் (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(வெற்று, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  எனில் (dataseg == MAP_FAILED)
    திரும்பு வெற்று;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, வெற்று, rounded_size, MAP_REMAPDUP);
  எனில் (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    திரும்பு வெற்று;
  }
  எனில் (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    திரும்பு வெற்று;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, அளவு(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, அளவு(size_t)), &codeseg, அளவு(வெறுமை *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  திரும்பு ADD_TO_POINTER(dataseg, overhead);
}

வெறுமை
ffi_closure_free (வெறுமை *ptr)
{
  வெறுமை *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, அளவு(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, அளவு(size_t)), அளவு(வெறுமை *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

முழு
ffi_tramp_is_present (__attribute__((unused)) வெறுமை *ptr)
{
  திரும்பு 0;
}
#இல்லையெனில் /* !NetBSD with PROT_MPROTECT */

#எனில் !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# எனில் __linux__ && !defined(__ANDROID__)
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
# எனில் defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#எனில் FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# எனில் !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# எனில் !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#எனில் FFI_CLOSURES

#எனில் FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#அந்தர்பாவ <mach/mach.h>
#அந்தர்பாவ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#அந்தர்பாவ <ptrauth.h>
#endif
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>

வெளி வெறுமை *ffi_closure_trampoline_table_page;

வகைவரை கட்டமைப்பு ffi_trampoline_table ffi_trampoline_table;
வகைவரை கட்டமைப்பு ffi_trampoline_table_entry ffi_trampoline_table_entry;

கட்டமைப்பு ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_மொத்தம்;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

கட்டமைப்பு ffi_trampoline_table_entry
{
  வெறுமை *(*trampoline) (வெறுமை);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_மொத்தம் (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

நிலையான pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
நிலையான ffi_trampoline_table *ffi_trampoline_tables = வெற்று;

நிலையான ffi_trampoline_table *
ffi_trampoline_table_alloc (வெறுமை)
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
  எனில் (kt != KERN_SUCCESS)
    திரும்பு வெற்று;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((வெறுமை *)&ffi_closure_trampoline_table_page, ptrauth_சாவி_function_pointer, 0);
#இல்லையெனில்
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  எனில் (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      திரும்பு வெற்று;
    }

  எனில் (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      எனில் (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          திரும்பு வெற்று;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, அளவு (ffi_trampoline_table));
  எனில் (table == வெற்று)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      திரும்பு வெற்று;
    }
  table->free_மொத்தம் = FFI_TRAMPOLINE_மொத்தம்;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_மொத்தம், அளவு (ffi_trampoline_table_entry));
  எனில் (table->free_list_pool == வெற்று)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      விடுவி (table);
      திரும்பு வெற்று;
    }

  க்கு (i = 0; i < table->free_மொத்தம்; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(வெறுமை *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_சாவி_function_pointer, 0);
#endif

      எனில் (i < table->free_மொத்தம் - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  திரும்பு table;
}

நிலையான வெறுமை
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  எனில் (table->prev != வெற்று)
    table->prev->next = table->next;

  எனில் (table->next != வெற்று)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  விடுவி (table->free_list_pool);
  விடுவி (table);
}

வெறுமை *
ffi_closure_alloc (size_t size, வெறுமை **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  எனில் (closure == வெற்று)
    திரும்பு வெற்று;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  எனில் (table == வெற்று || table->free_list == வெற்று)
    {
      table = ffi_trampoline_table_alloc ();
      எனில் (table == வெற்று)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  விடுவி (closure);
	  திரும்பு வெற்று;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      எனில் (table->next != வெற்று)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_மொத்தம்--;
  entry->next = வெற்று;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  திரும்பு closure;
}

வெறுமை
ffi_closure_free (வெறுமை *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_மொத்தம்++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  எனில் (table->free_மொத்தம் == FFI_TRAMPOLINE_மொத்தம்
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  இல்லையெனில் எனில் (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = வெற்று;
      table->next = ffi_trampoline_tables;
      எனில் (ffi_trampoline_tables != வெற்று)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  விடுவி (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#அல்லதெனில் FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <errno.h>
#ifndef _MSC_VER
#அந்தர்பாவ <unistd.h>
#endif
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <stdio.h>
#எனில் !defined(_WIN32)
#ifdef HAVE_MNTENT
#அந்தர்பாவ <mntent.h>
#endif /* HAVE_MNTENT */
#அந்தர்பாவ <sys/param.h>
#அந்தர்பாவ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#அந்தர்பாவ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#எனில் FFI_MMAP_EXEC_SELINUX
#அந்தர்பாவ <sys/statfs.h>
#அந்தர்பாவ <stdlib.h>

நிலையான முழு selinux_enabled = -1;

நிலையான முழு
selinux_enabled_check (வெறுமை)
{
  கட்டமைப்பு statfs sfs;
  FILE *f;
  எழுத்து *buf = வெற்று;
  size_t len = 0;

  எனில் (statfs ("/selinux", &sfs) >= 0
      && (unsigned முழு) sfs.f_type == 0xf97cff8cU)
    திரும்பு 1;
  f = fopen ("/proc/mounts", "r");
  எனில் (f == வெற்று)
    திரும்பு 0;
  போது (getline (&buf, &len, f) >= 0)
    {
      எழுத்து *p = strchr (buf, ' ');
      எனில் (p == வெற்று)
        நிறுத்து;
      p = strchr (p + 1, ' ');
      எனில் (p == வெற்று)
        நிறுத்து;
      எனில் (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          விடுவி (buf);
          fclose (f);
          திரும்பு 1;
        }
    }
  விடுவி (buf);
  fclose (f);
  திரும்பு 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#இல்லையெனில்

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#எனில் defined FFI_MMAP_PAX
#அந்தர்பாவ <stdlib.h>

எண்ணிக்கை {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
நிலையான முழு cached_pax_flags = -1;

நிலையான முழு
pax_flags_check (வெறுமை)
{
  எழுத்து *buf = வெற்று;
  size_t len = 0;
  FILE *f;
  முழு ret;
  f = fopen ("/proc/self/status", "r");
  எனில் (f == வெற்று)
    திரும்பு 0;
  ret = 0;

  போது (getline (&buf, &len, f) != -1)
    எனில் (!strncmp (buf, "PaX:", 4))
      {
        எனில் (வெற்று != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        எனில் (வெற்று != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        நிறுத்து;
      }
  விடுவி (buf);
  fclose (f);
  திரும்பு ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#அல்லதெனில் defined (__CYGWIN__) || defined(__INTERIX)

#அந்தர்பாவ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#எனில் !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
நிலையான வெறுமை *dlmalloc(size_t);
நிலையான வெறுமை dlfree(வெறுமை*);
நிலையான வெறுமை *dlcalloc(size_t, size_t) MAYBE_UNUSED;
நிலையான வெறுமை *dlrealloc(வெறுமை *, size_t) MAYBE_UNUSED;
நிலையான வெறுமை *dlmemalign(size_t, size_t) MAYBE_UNUSED;
நிலையான வெறுமை *dlvalloc(size_t) MAYBE_UNUSED;
நிலையான முழு dlmallopt(முழு, முழு) MAYBE_UNUSED;
நிலையான size_t dlmalloc_footprint(வெறுமை) MAYBE_UNUSED;
நிலையான size_t dlmalloc_max_footprint(வெறுமை) MAYBE_UNUSED;
நிலையான வெறுமை** dlindependent_calloc(size_t, size_t, வெறுமை**) MAYBE_UNUSED;
நிலையான வெறுமை** dlindependent_comalloc(size_t, size_t*, வெறுமை**) MAYBE_UNUSED;
நிலையான வெறுமை *dlpvalloc(size_t) MAYBE_UNUSED;
நிலையான முழு dlmalloc_trim(size_t) MAYBE_UNUSED;
நிலையான size_t dlmalloc_usable_size(வெறுமை*) MAYBE_UNUSED;
நிலையான வெறுமை dlmalloc_stats(வெறுமை) MAYBE_UNUSED;

#எனில் !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
நிலையான வெறுமை *dlmmap(வெறுமை *, size_t, முழு, முழு, முழு, off_t);
நிலையான முழு dlmunmap(வெறுமை *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#அந்தர்பாவ "dlmalloc.c"

#undef mmap
#undef munmap

#எனில் !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
நிலையான pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
நிலையான முழு execfd = -1;

/* The amount of space already allocated from the temporary file.  */
நிலையான size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
நிலையான முழு
open_temp_exec_file_memfd (மாறிலி எழுத்து *name)
{
  முழு fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  திரும்பு fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
நிலையான முழு
open_temp_exec_file_name (எழுத்து *name, முழு flags MAYBE_UNUSED)
{
  முழு fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#இல்லையெனில்
  fd = mkstemp (name);
#endif

  எனில் (fd != -1)
    unlink (name);

  திரும்பு fd;
}

/* Open a temporary file in the named directory.  */
நிலையான முழு
open_temp_exec_file_dir (மாறிலி எழுத்து *dir)
{
  நிலையான மாறிலி எழுத்து suffix[] = "/ffiXXXXXX";
  முழு lendir, flags;
  எழுத்து *tempname;
#ifdef O_TMPFILE
  முழு fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#இல்லையெனில்
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  எனில் (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    திரும்பு fd;
  } இல்லையெனில் {
    errno = 0;
  }
#endif

  lendir = (முழு) strlen (dir);
  tempname = __builtin_alloca (lendir + அளவு (suffix));

  எனில் (!tempname)
    திரும்பு -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, அளவு (suffix));

  திரும்பு open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
நிலையான முழு
open_temp_exec_file_env (மாறிலி எழுத்து *envvar)
{
  மாறிலி எழுத்து *மதிப்பு = getenv (envvar);

  எனில் (!மதிப்பு)
    திரும்பு -1;

  திரும்பு open_temp_exec_file_dir (மதிப்பு);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
நிலையான முழு
open_temp_exec_file_mnt (மாறிலி எழுத்து *mounts)
{
  நிலையான மாறிலி எழுத்து *last_mounts;
  நிலையான FILE *last_mntent;

  எனில் (mounts != last_mounts)
    {
      எனில் (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      எனில் (mounts)
	last_mntent = setmntent (mounts, "r");
      இல்லையெனில்
	last_mntent = வெற்று;
    }

  எனில் (!last_mntent)
    திரும்பு -1;

  க்கு (;;)
    {
      முழு fd;
      கட்டமைப்பு mntent mnt;
      எழுத்து buf[MAXPATHLEN * 3];

      எனில் (getmntent_r (last_mntent, &mnt, buf, அளவு (buf)) == வெற்று)
	திரும்பு -1;

      எனில் (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	தொடர்;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      எனில் (fd != -1)
	திரும்பு fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
நிலையான கட்டமைப்பு
{
  முழு (*func)(மாறிலி எழுத்து *);
  மாறிலி எழுத்து *arg;
  முழு repeat;
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
நிலையான முழு open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
நிலையான முழு
open_temp_exec_file_opts_next (வெறுமை)
{
  எனில் (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (வெற்று);

  open_temp_exec_file_opts_idx++;
  எனில் (open_temp_exec_file_opts_idx
      == (அளவு (open_temp_exec_file_opts)
	  / அளவு (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      திரும்பு 1;
    }

  திரும்பு 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
முழு
open_temp_exec_file (வெறுமை)
{
  முழு fd;

  செய்
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      எனில் (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  எனில் (open_temp_exec_file_opts_next ())
	    நிறுத்து;
	}
    }
  போது (fd == -1);

  திரும்பு fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
நிலையான முழு
allocate_space (முழு fd, off_t len)
{
  நிலையான long page_size;

  /* Obtain system page size. */
  எனில் (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  எனில் (page_size <= 0)
    திரும்பு -1;

  unsigned எழுத்து buf[page_size];
  memset (buf, 0, page_size);

  போது (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      எனில் (லிக (fd, buf, to_write) < to_write)
        திரும்பு -1;
      len -= to_write;
    }

  திரும்பு 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
நிலையான வெறுமை *
dlmmap_locked (வெறுமை *start, size_t length, முழு prot, முழு flags, off_t offset)
{
  வெறுமை *ptr;

  எனில் (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      எனில் (execfd == -1)
	திரும்பு MFAIL;
    }

  offset = execsize;

  எனில் (allocate_space (execfd, length))
    திரும்பு MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (வெற்று, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  எனில் (ptr == MFAIL)
    {
      எனில் (!offset)
	{
	  close (execfd);
	  செல் retry_open;
	}
      எனில் (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      திரும்பு MFAIL;
    }
  இல்லையெனில் எனில் (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  எனில் (start == MFAIL)
    {
      munmap (ptr, length);
      எனில் (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      திரும்பு start;
    }

  mmap_exec_offset ((எழுத்து *)start, length) = (எழுத்து*)ptr - (எழுத்து*)start;

  execsize += length;

  திரும்பு start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
நிலையான வெறுமை *
dlmmap (வெறுமை *start, size_t length, முழு prot,
	முழு flags, முழு fd, off_t offset)
{
  வெறுமை *ptr;

  ப்ரதிஜ்ஞா (start == வெற்று && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  எனில் (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      திரும்பு ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  எனில் (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      எனில் (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          திரும்பு ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  இல்லையெனில் எனில் (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      எனில் (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	திரும்பு ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  திரும்பு ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
நிலையான முழு
dlmunmap (வெறுமை *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  வெறுமை *code;

  எனில் (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      முழு ret = munmap (code, length);
      எனில் (ret)
	திரும்பு ret;
    }

  திரும்பு munmap (start, length);
}

#எனில் FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
நிலையான msegmentptr
segment_holding_code (mstate m, எழுத்து* addr)
{
  msegmentptr sp = &m->seg;
  க்கு (;;) {
    எனில் (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      திரும்பு sp;
    எனில் ((sp = sp->next) == 0)
      திரும்பு 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
வெறுமை *
ffi_closure_alloc (size_t size, வெறுமை **code)
{
  வெறுமை *ptr, *ftramp;

  எனில் (!code)
    திரும்பு வெற்று;

  ptr = dlmalloc (size);

  எனில் (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      எனில் (!ffi_tramp_is_supported ())
        திரும்பு ptr;

      ftramp = ffi_tramp_alloc (0);
      எனில் (ftramp == வெற்று)
      {
        dlfree (ptr);
        திரும்பு வெற்று;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  திரும்பு ptr;
}

வெறுமை *
ffi_data_to_code_pointer (வெறுமை *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  எனில் (seg)
    {
      எனில் (!ffi_tramp_is_supported ())
        திரும்பு add_segment_exec_offset (data, seg);
      திரும்பு ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  இல்லையெனில்
    திரும்பு data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
வெறுமை
ffi_closure_free (வெறுமை *ptr)
{
#எனில் FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  எனில் (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  எனில் (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

முழு
ffi_tramp_is_present (வெறுமை *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  திரும்பு seg != வெற்று && ffi_tramp_is_supported();
}

# இல்லையெனில் /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#அந்தர்பாவ <stdlib.h>

வெறுமை *
ffi_closure_alloc (size_t size, வெறுமை **code)
{
  வெறுமை *c;

  எனில் (!code)
    திரும்பு வெற்று;

  c = malloc (size);
  *code = FFI_FN (c);
  திரும்பு c;
}

வெறுமை
ffi_closure_free (வெறுமை *ptr)
{
  விடுவி (ptr);
}

வெறுமை *
ffi_data_to_code_pointer (வெறுமை *data)
{
  திரும்பு data;
}

முழு
ffi_tramp_is_present (__attribute__((unused)) வெறுமை *ptr)
{
  திரும்பு 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

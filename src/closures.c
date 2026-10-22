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

#𐡄𐡍 (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <tramp.h>

#ifdef __NetBSD__
#ضمن <sys/param.h>
#endif

#𐡄𐡍 __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ضمن <sys/mman.h>

#ضمن <stddef.h>
#ضمن <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#ضمن <sys/memfd.h>
#endif

ساكن ثابت size_t overhead =
  (حجم(max_align_t) > حجم(فراغ *) + حجم(size_t)) ?
    حجم(max_align_t)
    : حجم(فراغ *) + حجم(size_t);

#define ADD_TO_POINTER(p, d) ((فراغ *)((uintptr_t)(p) + (d)))

فراغ *
ffi_closure_alloc (size_t size, فراغ **code)
{
  ساكن size_t page_size;
  size_t rounded_size;
  فراغ *codeseg, *dataseg;
  صحيح prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  𐡄𐡍 (!code)
    أرجع عدم;

  /* Obtain system page size. */
  𐡄𐡍 (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(عدم, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  𐡄𐡍 (dataseg == MAP_FAILED)
    أرجع عدم;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, عدم, rounded_size, MAP_REMAPDUP);
  𐡄𐡍 (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    أرجع عدم;
  }
  𐡄𐡍 (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    أرجع عدم;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, حجم(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, حجم(size_t)), &codeseg, حجم(فراغ *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  أرجع ADD_TO_POINTER(dataseg, overhead);
}

فراغ
ffi_closure_free (فراغ *ptr)
{
  فراغ *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, حجم(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, حجم(size_t)), حجم(فراغ *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

صحيح
ffi_tramp_is_present (__attribute__((unused)) فراغ *ptr)
{
  أرجع 0;
}
#وإلا /* !NetBSD with PROT_MPROTECT */

#𐡄𐡍 !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# 𐡄𐡍 __linux__ && !defined(__ANDROID__)
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
# 𐡄𐡍 defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#𐡄𐡍 FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# 𐡄𐡍 !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# 𐡄𐡍 !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#𐡄𐡍 FFI_CLOSURES

#𐡄𐡍 FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#ضمن <mach/mach.h>
#ضمن <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#ضمن <ptrauth.h>
#endif
#ضمن <stdio.h>
#ضمن <stdlib.h>

خارجي فراغ *ffi_closure_trampoline_table_page;

تعريفنوع بنية ffi_trampoline_table ffi_trampoline_table;
تعريفنوع بنية ffi_trampoline_table_entry ffi_trampoline_table_entry;

بنية ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_count;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

بنية ffi_trampoline_table_entry
{
  فراغ *(*trampoline) (فراغ);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_COUNT (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

ساكن pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
ساكن ffi_trampoline_table *ffi_trampoline_tables = عدم;

ساكن ffi_trampoline_table *
ffi_trampoline_table_alloc (فراغ)
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
  𐡄𐡍 (kt != KERN_SUCCESS)
    أرجع عدم;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((فراغ *)&ffi_closure_trampoline_table_page, ptrauth_key_function_pointer, 0);
#وإلا
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  𐡄𐡍 (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      أرجع عدم;
    }

  𐡄𐡍 (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      𐡄𐡍 (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          أرجع عدم;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, حجم (ffi_trampoline_table));
  𐡄𐡍 (table == عدم)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      أرجع عدم;
    }
  table->free_count = FFI_TRAMPOLINE_COUNT;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_COUNT, حجم (ffi_trampoline_table_entry));
  𐡄𐡍 (table->free_list_pool == عدم)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      حرر (table);
      أرجع عدم;
    }

  لكل (i = 0; i < table->free_count; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(فراغ *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_key_function_pointer, 0);
#endif

      𐡄𐡍 (i < table->free_count - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  أرجع table;
}

ساكن فراغ
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  𐡄𐡍 (table->prev != عدم)
    table->prev->next = table->next;

  𐡄𐡍 (table->next != عدم)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  حرر (table->free_list_pool);
  حرر (table);
}

فراغ *
ffi_closure_alloc (size_t size, فراغ **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  𐡄𐡍 (closure == عدم)
    أرجع عدم;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  𐡄𐡍 (table == عدم || table->free_list == عدم)
    {
      table = ffi_trampoline_table_alloc ();
      𐡄𐡍 (table == عدم)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  حرر (closure);
	  أرجع عدم;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      𐡄𐡍 (table->next != عدم)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_count--;
  entry->next = عدم;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  أرجع closure;
}

فراغ
ffi_closure_free (فراغ *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_count++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  𐡄𐡍 (table->free_count == FFI_TRAMPOLINE_COUNT
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  وإلا 𐡄𐡍 (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = عدم;
      table->next = ffi_trampoline_tables;
      𐡄𐡍 (ffi_trampoline_tables != عدم)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  حرر (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#وإلاإذا FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <fcntl.h>
#ضمن <errno.h>
#ifndef _MSC_VER
#ضمن <unistd.h>
#endif
#ضمن <string.h>
#ضمن <stdio.h>
#𐡄𐡍 !defined(_WIN32)
#ifdef HAVE_MNTENT
#ضمن <mntent.h>
#endif /* HAVE_MNTENT */
#ضمن <sys/param.h>
#ضمن <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ضمن <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#𐡄𐡍 FFI_MMAP_EXEC_SELINUX
#ضمن <sys/statfs.h>
#ضمن <stdlib.h>

ساكن صحيح selinux_enabled = -1;

ساكن صحيح
selinux_enabled_check (فراغ)
{
  بنية statfs sfs;
  FILE *f;
  حرف *buf = عدم;
  size_t len = 0;

  𐡄𐡍 (statfs ("/selinux", &sfs) >= 0
      && (unsigned صحيح) sfs.f_type == 0xf97cff8cU)
    أرجع 1;
  f = fopen ("/proc/mounts", "r");
  𐡄𐡍 (f == عدم)
    أرجع 0;
  طالما (getline (&buf, &len, f) >= 0)
    {
      حرف *p = strchr (buf, ' ');
      𐡄𐡍 (p == عدم)
        اكسر;
      p = strchr (p + 1, ' ');
      𐡄𐡍 (p == عدم)
        اكسر;
      𐡄𐡍 (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          حرر (buf);
          fclose (f);
          أرجع 1;
        }
    }
  حرر (buf);
  fclose (f);
  أرجع 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#وإلا

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#𐡄𐡍 defined FFI_MMAP_PAX
#ضمن <stdlib.h>

تعداد {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ساكن صحيح cached_pax_flags = -1;

ساكن صحيح
pax_flags_check (فراغ)
{
  حرف *buf = عدم;
  size_t len = 0;
  FILE *f;
  صحيح ret;
  f = fopen ("/proc/self/status", "r");
  𐡄𐡍 (f == عدم)
    أرجع 0;
  ret = 0;

  طالما (getline (&buf, &len, f) != -1)
    𐡄𐡍 (!strncmp (buf, "PaX:", 4))
      {
        𐡄𐡍 (عدم != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        𐡄𐡍 (عدم != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        اكسر;
      }
  حرر (buf);
  fclose (f);
  أرجع ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#وإلاإذا defined (__CYGWIN__) || defined(__INTERIX)

#ضمن <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#𐡄𐡍 !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ساكن فراغ *dlmalloc(size_t);
ساكن فراغ dlfree(فراغ*);
ساكن فراغ *dlcalloc(size_t, size_t) MAYBE_UNUSED;
ساكن فراغ *dlrealloc(فراغ *, size_t) MAYBE_UNUSED;
ساكن فراغ *dlmemalign(size_t, size_t) MAYBE_UNUSED;
ساكن فراغ *dlvalloc(size_t) MAYBE_UNUSED;
ساكن صحيح dlmallopt(صحيح, صحيح) MAYBE_UNUSED;
ساكن size_t dlmalloc_footprint(فراغ) MAYBE_UNUSED;
ساكن size_t dlmalloc_max_footprint(فراغ) MAYBE_UNUSED;
ساكن فراغ** dlindependent_calloc(size_t, size_t, فراغ**) MAYBE_UNUSED;
ساكن فراغ** dlindependent_comalloc(size_t, size_t*, فراغ**) MAYBE_UNUSED;
ساكن فراغ *dlpvalloc(size_t) MAYBE_UNUSED;
ساكن صحيح dlmalloc_trim(size_t) MAYBE_UNUSED;
ساكن size_t dlmalloc_usable_size(فراغ*) MAYBE_UNUSED;
ساكن فراغ dlmalloc_stats(فراغ) MAYBE_UNUSED;

#𐡄𐡍 !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ساكن فراغ *dlmmap(فراغ *, size_t, صحيح, صحيح, صحيح, off_t);
ساكن صحيح dlmunmap(فراغ *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#ضمن "dlmalloc.c"

#undef mmap
#undef munmap

#𐡄𐡍 !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ساكن pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ساكن صحيح execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ساكن size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_temp_exec_file_memfd (ثابت حرف *name)
{
  صحيح fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  أرجع fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_temp_exec_file_name (حرف *name, صحيح flags MAYBE_UNUSED)
{
  صحيح fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#وإلا
  fd = mkstemp (name);
#endif

  𐡄𐡍 (fd != -1)
    unlink (name);

  أرجع fd;
}

/* Open a temporary file in the named directory.  */
ساكن صحيح
open_temp_exec_file_dir (ثابت حرف *dir)
{
  ساكن ثابت حرف suffix[] = "/ffiXXXXXX";
  صحيح lendir, flags;
  حرف *tempname;
#ifdef O_TMPFILE
  صحيح fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#وإلا
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  𐡄𐡍 (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    أرجع fd;
  } وإلا {
    errno = 0;
  }
#endif

  lendir = (صحيح) strlen (dir);
  tempname = __builtin_alloca (lendir + حجم (suffix));

  𐡄𐡍 (!tempname)
    أرجع -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, حجم (suffix));

  أرجع open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ساكن صحيح
open_temp_exec_file_env (ثابت حرف *envvar)
{
  ثابت حرف *value = getenv (envvar);

  𐡄𐡍 (!value)
    أرجع -1;

  أرجع open_temp_exec_file_dir (value);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ساكن صحيح
open_temp_exec_file_mnt (ثابت حرف *mounts)
{
  ساكن ثابت حرف *last_mounts;
  ساكن FILE *last_mntent;

  𐡄𐡍 (mounts != last_mounts)
    {
      𐡄𐡍 (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      𐡄𐡍 (mounts)
	last_mntent = setmntent (mounts, "r");
      وإلا
	last_mntent = عدم;
    }

  𐡄𐡍 (!last_mntent)
    أرجع -1;

  لكل (;;)
    {
      صحيح fd;
      بنية mntent mnt;
      حرف buf[MAXPATHLEN * 3];

      𐡄𐡍 (getmntent_r (last_mntent, &mnt, buf, حجم (buf)) == عدم)
	أرجع -1;

      𐡄𐡍 (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	استمر;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      𐡄𐡍 (fd != -1)
	أرجع fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ساكن بنية
{
  صحيح (*func)(ثابت حرف *);
  ثابت حرف *arg;
  صحيح repeat;
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
ساكن صحيح open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ساكن صحيح
open_temp_exec_file_opts_next (فراغ)
{
  𐡄𐡍 (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (عدم);

  open_temp_exec_file_opts_idx++;
  𐡄𐡍 (open_temp_exec_file_opts_idx
      == (حجم (open_temp_exec_file_opts)
	  / حجم (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      أرجع 1;
    }

  أرجع 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
صحيح
open_temp_exec_file (فراغ)
{
  صحيح fd;

  𐡏𐡁𐡃
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      𐡄𐡍 (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  𐡄𐡍 (open_temp_exec_file_opts_next ())
	    اكسر;
	}
    }
  طالما (fd == -1);

  أرجع fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ساكن صحيح
allocate_space (صحيح fd, off_t len)
{
  ساكن long page_size;

  /* Obtain system page size. */
  𐡄𐡍 (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  𐡄𐡍 (page_size <= 0)
    أرجع -1;

  unsigned حرف buf[page_size];
  memset (buf, 0, page_size);

  طالما (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      𐡄𐡍 (اكتب (fd, buf, to_write) < to_write)
        أرجع -1;
      len -= to_write;
    }

  أرجع 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ساكن فراغ *
dlmmap_locked (فراغ *start, size_t length, صحيح prot, صحيح flags, off_t offset)
{
  فراغ *ptr;

  𐡄𐡍 (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      𐡄𐡍 (execfd == -1)
	أرجع MFAIL;
    }

  offset = execsize;

  𐡄𐡍 (allocate_space (execfd, length))
    أرجع MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (عدم, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  𐡄𐡍 (ptr == MFAIL)
    {
      𐡄𐡍 (!offset)
	{
	  close (execfd);
	  اذهب retry_open;
	}
      𐡄𐡍 (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      أرجع MFAIL;
    }
  وإلا 𐡄𐡍 (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  𐡄𐡍 (start == MFAIL)
    {
      munmap (ptr, length);
      𐡄𐡍 (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      أرجع start;
    }

  mmap_exec_offset ((حرف *)start, length) = (حرف*)ptr - (حرف*)start;

  execsize += length;

  أرجع start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ساكن فراغ *
dlmmap (فراغ *start, size_t length, صحيح prot,
	صحيح flags, صحيح fd, off_t offset)
{
  فراغ *ptr;

  أكد (start == عدم && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  𐡄𐡍 (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      أرجع ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  𐡄𐡍 (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      𐡄𐡍 (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          أرجع ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  وإلا 𐡄𐡍 (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      𐡄𐡍 (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	أرجع ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  أرجع ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ساكن صحيح
dlmunmap (فراغ *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  فراغ *code;

  𐡄𐡍 (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      صحيح ret = munmap (code, length);
      𐡄𐡍 (ret)
	أرجع ret;
    }

  أرجع munmap (start, length);
}

#𐡄𐡍 FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
ساكن msegmentptr
segment_holding_code (mstate m, حرف* addr)
{
  msegmentptr sp = &m->seg;
  لكل (;;) {
    𐡄𐡍 (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      أرجع sp;
    𐡄𐡍 ((sp = sp->next) == 0)
      أرجع 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
فراغ *
ffi_closure_alloc (size_t size, فراغ **code)
{
  فراغ *ptr, *ftramp;

  𐡄𐡍 (!code)
    أرجع عدم;

  ptr = dlmalloc (size);

  𐡄𐡍 (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      𐡄𐡍 (!ffi_tramp_is_supported ())
        أرجع ptr;

      ftramp = ffi_tramp_alloc (0);
      𐡄𐡍 (ftramp == عدم)
      {
        dlfree (ptr);
        أرجع عدم;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  أرجع ptr;
}

فراغ *
ffi_data_to_code_pointer (فراغ *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  𐡄𐡍 (seg)
    {
      𐡄𐡍 (!ffi_tramp_is_supported ())
        أرجع add_segment_exec_offset (data, seg);
      أرجع ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  وإلا
    أرجع data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
فراغ
ffi_closure_free (فراغ *ptr)
{
#𐡄𐡍 FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  𐡄𐡍 (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  𐡄𐡍 (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

صحيح
ffi_tramp_is_present (فراغ *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  أرجع seg != عدم && ffi_tramp_is_supported();
}

# وإلا /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#ضمن <stdlib.h>

فراغ *
ffi_closure_alloc (size_t size, فراغ **code)
{
  فراغ *c;

  𐡄𐡍 (!code)
    أرجع عدم;

  c = malloc (size);
  *code = FFI_FN (c);
  أرجع c;
}

فراغ
ffi_closure_free (فراغ *ptr)
{
  حرر (ptr);
}

فراغ *
ffi_data_to_code_pointer (فراغ *data)
{
  أرجع data;
}

صحيح
ffi_tramp_is_present (__attribute__((unused)) فراغ *ptr)
{
  أرجع 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

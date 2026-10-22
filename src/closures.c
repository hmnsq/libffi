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

#اگر (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <tramp.h>

#ifdef __NetBSD__
#شامل <sys/param.h>
#endif

#اگر __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#شامل <sys/mman.h>

#شامل <stddef.h>
#شامل <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#شامل <sys/memfd.h>
#endif

ایستا ثابت size_t overhead =
  (اندازه(max_align_t) > اندازه(تهی *) + اندازه(size_t)) ?
    اندازه(max_align_t)
    : اندازه(تهی *) + اندازه(size_t);

#define ADD_TO_POINTER(p, d) ((تهی *)((uintptr_t)(p) + (d)))

تهی *
ffi_closure_alloc (size_t size, تهی **code)
{
  ایستا size_t page_size;
  size_t rounded_size;
  تهی *codeseg, *dataseg;
  صحیح prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  اگر (!code)
    بازگشت پوچ;

  /* Obtain system page size. */
  اگر (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(پوچ, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  اگر (dataseg == MAP_FAILED)
    بازگشت پوچ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, پوچ, rounded_size, MAP_REMAPDUP);
  اگر (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    بازگشت پوچ;
  }
  اگر (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    بازگشت پوچ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, اندازه(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, اندازه(size_t)), &codeseg, اندازه(تهی *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  بازگشت ADD_TO_POINTER(dataseg, overhead);
}

تهی
ffi_closure_free (تهی *ptr)
{
  تهی *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, اندازه(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, اندازه(size_t)), اندازه(تهی *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

صحیح
ffi_tramp_is_present (__attribute__((unused)) تهی *ptr)
{
  بازگشت 0;
}
#وگرنه /* !NetBSD with PROT_MPROTECT */

#اگر !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# اگر __linux__ && !defined(__ANDROID__)
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
# اگر defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#اگر FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# اگر !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# اگر !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#اگر FFI_CLOSURES

#اگر FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#شامل <mach/mach.h>
#شامل <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#شامل <ptrauth.h>
#endif
#شامل <stdio.h>
#شامل <stdlib.h>

بیرونی تهی *ffi_closure_trampoline_table_page;

تعریفنوع ساختار ffi_trampoline_table ffi_trampoline_table;
تعریفنوع ساختار ffi_trampoline_table_entry ffi_trampoline_table_entry;

ساختار ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_شمار;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

ساختار ffi_trampoline_table_entry
{
  تهی *(*trampoline) (تهی);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_شمار (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

ایستا pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
ایستا ffi_trampoline_table *ffi_trampoline_tables = پوچ;

ایستا ffi_trampoline_table *
ffi_trampoline_table_alloc (تهی)
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
  اگر (kt != KERN_SUCCESS)
    بازگشت پوچ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((تهی *)&ffi_closure_trampoline_table_page, ptrauth_کلید_function_pointer, 0);
#وگرنه
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  اگر (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      بازگشت پوچ;
    }

  اگر (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      اگر (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          بازگشت پوچ;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, اندازه (ffi_trampoline_table));
  اگر (table == پوچ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      بازگشت پوچ;
    }
  table->free_شمار = FFI_TRAMPOLINE_شمار;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_شمار, اندازه (ffi_trampoline_table_entry));
  اگر (table->free_list_pool == پوچ)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      آزاد (table);
      بازگشت پوچ;
    }

  برای (i = 0; i < table->free_شمار; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(تهی *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_کلید_function_pointer, 0);
#endif

      اگر (i < table->free_شمار - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  بازگشت table;
}

ایستا تهی
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  اگر (table->prev != پوچ)
    table->prev->next = table->next;

  اگر (table->next != پوچ)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  آزاد (table->free_list_pool);
  آزاد (table);
}

تهی *
ffi_closure_alloc (size_t size, تهی **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  اگر (closure == پوچ)
    بازگشت پوچ;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  اگر (table == پوچ || table->free_list == پوچ)
    {
      table = ffi_trampoline_table_alloc ();
      اگر (table == پوچ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  آزاد (closure);
	  بازگشت پوچ;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      اگر (table->next != پوچ)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_شمار--;
  entry->next = پوچ;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  بازگشت closure;
}

تهی
ffi_closure_free (تهی *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_شمار++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  اگر (table->free_شمار == FFI_TRAMPOLINE_شمار
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  وگرنه اگر (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = پوچ;
      table->next = ffi_trampoline_tables;
      اگر (ffi_trampoline_tables != پوچ)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  آزاد (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#وگرنهاگر FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <fcntl.h>
#شامل <errno.h>
#ifndef _MSC_VER
#شامل <unistd.h>
#endif
#شامل <string.h>
#شامل <stdio.h>
#اگر !defined(_WIN32)
#ifdef HAVE_MNTENT
#شامل <mntent.h>
#endif /* HAVE_MNTENT */
#شامل <sys/param.h>
#شامل <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#شامل <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#اگر FFI_MMAP_EXEC_SELINUX
#شامل <sys/statfs.h>
#شامل <stdlib.h>

ایستا صحیح selinux_enabled = -1;

ایستا صحیح
selinux_enabled_check (تهی)
{
  ساختار statfs sfs;
  FILE *f;
  نویسه *buf = پوچ;
  size_t len = 0;

  اگر (statfs ("/selinux", &sfs) >= 0
      && (unsigned صحیح) sfs.f_type == 0xf97cff8cU)
    بازگشت 1;
  f = fopen ("/proc/mounts", "r");
  اگر (f == پوچ)
    بازگشت 0;
  وقتی (getline (&buf, &len, f) >= 0)
    {
      نویسه *p = strchr (buf, ' ');
      اگر (p == پوچ)
        بشکن;
      p = strchr (p + 1, ' ');
      اگر (p == پوچ)
        بشکن;
      اگر (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          آزاد (buf);
          fclose (f);
          بازگشت 1;
        }
    }
  آزاد (buf);
  fclose (f);
  بازگشت 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#وگرنه

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#اگر defined FFI_MMAP_PAX
#شامل <stdlib.h>

شمارش {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ایستا صحیح cached_pax_flags = -1;

ایستا صحیح
pax_flags_check (تهی)
{
  نویسه *buf = پوچ;
  size_t len = 0;
  FILE *f;
  صحیح ret;
  f = fopen ("/proc/self/status", "r");
  اگر (f == پوچ)
    بازگشت 0;
  ret = 0;

  وقتی (getline (&buf, &len, f) != -1)
    اگر (!strncmp (buf, "PaX:", 4))
      {
        اگر (پوچ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        اگر (پوچ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        بشکن;
      }
  آزاد (buf);
  fclose (f);
  بازگشت ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#وگرنهاگر defined (__CYGWIN__) || defined(__INTERIX)

#شامل <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#اگر !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ایستا تهی *dlmalloc(size_t);
ایستا تهی dlfree(تهی*);
ایستا تهی *dlcalloc(size_t, size_t) MAYBE_UNUSED;
ایستا تهی *dlrealloc(تهی *, size_t) MAYBE_UNUSED;
ایستا تهی *dlmemalign(size_t, size_t) MAYBE_UNUSED;
ایستا تهی *dlvalloc(size_t) MAYBE_UNUSED;
ایستا صحیح dlmallopt(صحیح, صحیح) MAYBE_UNUSED;
ایستا size_t dlmalloc_footprint(تهی) MAYBE_UNUSED;
ایستا size_t dlmalloc_max_footprint(تهی) MAYBE_UNUSED;
ایستا تهی** dlindependent_calloc(size_t, size_t, تهی**) MAYBE_UNUSED;
ایستا تهی** dlindependent_comalloc(size_t, size_t*, تهی**) MAYBE_UNUSED;
ایستا تهی *dlpvalloc(size_t) MAYBE_UNUSED;
ایستا صحیح dlmalloc_trim(size_t) MAYBE_UNUSED;
ایستا size_t dlmalloc_usable_size(تهی*) MAYBE_UNUSED;
ایستا تهی dlmalloc_stats(تهی) MAYBE_UNUSED;

#اگر !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ایستا تهی *dlmmap(تهی *, size_t, صحیح, صحیح, صحیح, off_t);
ایستا صحیح dlmunmap(تهی *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#شامل "dlmalloc.c"

#undef mmap
#undef munmap

#اگر !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ایستا pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ایستا صحیح execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ایستا size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ایستا صحیح
open_temp_exec_file_memfd (ثابت نویسه *name)
{
  صحیح fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  بازگشت fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
ایستا صحیح
open_temp_exec_file_name (نویسه *name, صحیح flags MAYBE_UNUSED)
{
  صحیح fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#وگرنه
  fd = mkstemp (name);
#endif

  اگر (fd != -1)
    unlink (name);

  بازگشت fd;
}

/* Open a temporary file in the named directory.  */
ایستا صحیح
open_temp_exec_file_dir (ثابت نویسه *dir)
{
  ایستا ثابت نویسه suffix[] = "/ffiXXXXXX";
  صحیح lendir, flags;
  نویسه *tempname;
#ifdef O_TMPFILE
  صحیح fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#وگرنه
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  اگر (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    بازگشت fd;
  } وگرنه {
    errno = 0;
  }
#endif

  lendir = (صحیح) strlen (dir);
  tempname = __builtin_alloca (lendir + اندازه (suffix));

  اگر (!tempname)
    بازگشت -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, اندازه (suffix));

  بازگشت open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ایستا صحیح
open_temp_exec_file_env (ثابت نویسه *envvar)
{
  ثابت نویسه *مقدار = getenv (envvar);

  اگر (!مقدار)
    بازگشت -1;

  بازگشت open_temp_exec_file_dir (مقدار);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ایستا صحیح
open_temp_exec_file_mnt (ثابت نویسه *mounts)
{
  ایستا ثابت نویسه *last_mounts;
  ایستا FILE *last_mntent;

  اگر (mounts != last_mounts)
    {
      اگر (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      اگر (mounts)
	last_mntent = setmntent (mounts, "r");
      وگرنه
	last_mntent = پوچ;
    }

  اگر (!last_mntent)
    بازگشت -1;

  برای (;;)
    {
      صحیح fd;
      ساختار mntent mnt;
      نویسه buf[MAXPATHLEN * 3];

      اگر (getmntent_r (last_mntent, &mnt, buf, اندازه (buf)) == پوچ)
	بازگشت -1;

      اگر (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ادامه;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      اگر (fd != -1)
	بازگشت fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ایستا ساختار
{
  صحیح (*func)(ثابت نویسه *);
  ثابت نویسه *arg;
  صحیح repeat;
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
ایستا صحیح open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ایستا صحیح
open_temp_exec_file_opts_next (تهی)
{
  اگر (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (پوچ);

  open_temp_exec_file_opts_idx++;
  اگر (open_temp_exec_file_opts_idx
      == (اندازه (open_temp_exec_file_opts)
	  / اندازه (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      بازگشت 1;
    }

  بازگشت 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
صحیح
open_temp_exec_file (تهی)
{
  صحیح fd;

  انجامبده
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      اگر (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  اگر (open_temp_exec_file_opts_next ())
	    بشکن;
	}
    }
  وقتی (fd == -1);

  بازگشت fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ایستا صحیح
allocate_space (صحیح fd, off_t len)
{
  ایستا long page_size;

  /* Obtain system page size. */
  اگر (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  اگر (page_size <= 0)
    بازگشت -1;

  unsigned نویسه buf[page_size];
  memset (buf, 0, page_size);

  وقتی (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      اگر (اكتب (fd, buf, to_write) < to_write)
        بازگشت -1;
      len -= to_write;
    }

  بازگشت 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ایستا تهی *
dlmmap_locked (تهی *start, size_t length, صحیح prot, صحیح flags, off_t offset)
{
  تهی *ptr;

  اگر (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      اگر (execfd == -1)
	بازگشت MFAIL;
    }

  offset = execsize;

  اگر (allocate_space (execfd, length))
    بازگشت MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (پوچ, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  اگر (ptr == MFAIL)
    {
      اگر (!offset)
	{
	  close (execfd);
	  برو retry_open;
	}
      اگر (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      بازگشت MFAIL;
    }
  وگرنه اگر (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  اگر (start == MFAIL)
    {
      munmap (ptr, length);
      اگر (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      بازگشت start;
    }

  mmap_exec_offset ((نویسه *)start, length) = (نویسه*)ptr - (نویسه*)start;

  execsize += length;

  بازگشت start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ایستا تهی *
dlmmap (تهی *start, size_t length, صحیح prot,
	صحیح flags, صحیح fd, off_t offset)
{
  تهی *ptr;

  أكد (start == پوچ && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  اگر (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      بازگشت ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  اگر (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      اگر (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          بازگشت ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  وگرنه اگر (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      اگر (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	بازگشت ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  بازگشت ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ایستا صحیح
dlmunmap (تهی *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  تهی *code;

  اگر (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      صحیح ret = munmap (code, length);
      اگر (ret)
	بازگشت ret;
    }

  بازگشت munmap (start, length);
}

#اگر FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
ایستا msegmentptr
segment_holding_code (mstate m, نویسه* addr)
{
  msegmentptr sp = &m->seg;
  برای (;;) {
    اگر (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      بازگشت sp;
    اگر ((sp = sp->next) == 0)
      بازگشت 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
تهی *
ffi_closure_alloc (size_t size, تهی **code)
{
  تهی *ptr, *ftramp;

  اگر (!code)
    بازگشت پوچ;

  ptr = dlmalloc (size);

  اگر (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      اگر (!ffi_tramp_is_supported ())
        بازگشت ptr;

      ftramp = ffi_tramp_alloc (0);
      اگر (ftramp == پوچ)
      {
        dlfree (ptr);
        بازگشت پوچ;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  بازگشت ptr;
}

تهی *
ffi_data_to_code_pointer (تهی *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  اگر (seg)
    {
      اگر (!ffi_tramp_is_supported ())
        بازگشت add_segment_exec_offset (data, seg);
      بازگشت ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  وگرنه
    بازگشت data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
تهی
ffi_closure_free (تهی *ptr)
{
#اگر FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  اگر (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  اگر (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

صحیح
ffi_tramp_is_present (تهی *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  بازگشت seg != پوچ && ffi_tramp_is_supported();
}

# وگرنه /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#شامل <stdlib.h>

تهی *
ffi_closure_alloc (size_t size, تهی **code)
{
  تهی *c;

  اگر (!code)
    بازگشت پوچ;

  c = malloc (size);
  *code = FFI_FN (c);
  بازگشت c;
}

تهی
ffi_closure_free (تهی *ptr)
{
  آزاد (ptr);
}

تهی *
ffi_data_to_code_pointer (تهی *data)
{
  بازگشت data;
}

صحیح
ffi_tramp_is_present (__attribute__((unused)) تهی *ptr)
{
  بازگشت 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

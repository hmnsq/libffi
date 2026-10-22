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

#အကယ်၍ (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#include <fficonfig.h>
#include <ffi.h>
#include <ffi_common.h>
#include <tramp.h>

#ifdef __NetBSD__
#include <sys/param.h>
#endif

#အကယ်၍ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#include <sys/mman.h>

#include <stddef.h>
#include <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#include <sys/memfd.h>
#endif

static const size_t overhead =
  (parimaa_na(max_align_t) > parimaa_na(void *) + parimaa_na(size_t)) ?
    parimaa_na(max_align_t)
    : parimaa_na(void *) + parimaa_na(size_t);

#define ADD_TO_POINTER(p, d) ((void *)((uintptr_t)(p) + (d)))

void *
ffi_closure_alloc (size_t size, void **code)
{
  static size_t page_size;
  size_t rounded_size;
  void *codeseg, *dataseg;
  ကိန်းပြည့် prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  အကယ်၍ (!code)
    ပြန်ပေး NULL;

  /* Obtain system page size. */
  အကယ်၍ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(NULL, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  အကယ်၍ (dataseg == MAP_FAILED)
    ပြန်ပေး NULL;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, NULL, rounded_size, MAP_REMAPDUP);
  အကယ်၍ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    ပြန်ပေး NULL;
  }
  အကယ်၍ (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    ပြန်ပေး NULL;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, parimaa_na(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, parimaa_na(size_t)), &codeseg, parimaa_na(void *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  ပြန်ပေး ADD_TO_POINTER(dataseg, overhead);
}

void
ffi_closure_free (void *ptr)
{
  void *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, parimaa_na(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, parimaa_na(size_t)), parimaa_na(void *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

ကိန်းပြည့်
ffi_tramp_is_present (__attribute__((unused)) void *ptr)
{
  ပြန်ပေး 0;
}
#မဟုတ်ပါက /* !NetBSD with PROT_MPROTECT */

#အကယ်၍ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# အကယ်၍ __linux__ && !defined(__ANDROID__)
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
# အကယ်၍ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#အကယ်၍ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# အကယ်၍ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# အကယ်၍ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#အကယ်၍ FFI_CLOSURES

#အကယ်၍ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#include <mach/mach.h>
#include <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#include <ptrauth.h>
#endif
#include <stdio.h>
#include <stdlib.h>

extern void *ffi_closure_trampoline_table_page;

typedef sa_nracanaa ffi_trampoline_table ffi_trampoline_table;
typedef sa_nracanaa ffi_trampoline_table_entry ffi_trampoline_table_entry;

sa_nracanaa ffi_trampoline_table
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

sa_nracanaa ffi_trampoline_table_entry
{
  void *(*trampoline) (void);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_COUNT (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

static pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
static ffi_trampoline_table *ffi_trampoline_tables = NULL;

static ffi_trampoline_table *
ffi_trampoline_table_alloc (void)
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
  အကယ်၍ (kt != KERN_SUCCESS)
    ပြန်ပေး NULL;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((void *)&ffi_closure_trampoline_table_page, ptrauth_key_function_pointer, 0);
#မဟုတ်ပါက
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  အကယ်၍ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ပြန်ပေး NULL;
    }

  အကယ်၍ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      အကယ်၍ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          ပြန်ပေး NULL;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, parimaa_na (ffi_trampoline_table));
  အကယ်၍ (table == NULL)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ပြန်ပေး NULL;
    }
  table->free_count = FFI_TRAMPOLINE_COUNT;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_COUNT, parimaa_na (ffi_trampoline_table_entry));
  အကယ်၍ (table->free_list_pool == NULL)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      free (table);
      ပြန်ပေး NULL;
    }

  အတွက် (i = 0; i < table->free_count; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(void *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_key_function_pointer, 0);
#endif

      အကယ်၍ (i < table->free_count - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  ပြန်ပေး table;
}

static void
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  အကယ်၍ (table->prev != NULL)
    table->prev->next = table->next;

  အကယ်၍ (table->next != NULL)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  free (table->free_list_pool);
  free (table);
}

void *
ffi_closure_alloc (size_t size, void **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  အကယ်၍ (closure == NULL)
    ပြန်ပေး NULL;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  အကယ်၍ (table == NULL || table->free_list == NULL)
    {
      table = ffi_trampoline_table_alloc ();
      အကယ်၍ (table == NULL)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  free (closure);
	  ပြန်ပေး NULL;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      အကယ်၍ (table->next != NULL)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_count--;
  entry->next = NULL;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  ပြန်ပေး closure;
}

void
ffi_closure_free (void *ptr)
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
  အကယ်၍ (table->free_count == FFI_TRAMPOLINE_COUNT
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  မဟုတ်ပါက အကယ်၍ (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = NULL;
      table->next = ffi_trampoline_tables;
      အကယ်၍ (ffi_trampoline_tables != NULL)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  free (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#elif FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#အကယ်၍ !defined(_WIN32)
#ifdef HAVE_MNTENT
#include <mntent.h>
#endif /* HAVE_MNTENT */
#include <sys/param.h>
#include <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#include <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#အကယ်၍ FFI_MMAP_EXEC_SELINUX
#include <sys/statfs.h>
#include <stdlib.h>

static ကိန်းပြည့် selinux_enabled = -1;

static ကိန်းပြည့်
selinux_enabled_check (void)
{
  sa_nracanaa statfs sfs;
  FILE *f;
  var_na *buf = NULL;
  size_t len = 0;

  အကယ်၍ (statfs ("/selinux", &sfs) >= 0
      && (unsigned ကိန်းပြည့်) sfs.f_type == 0xf97cff8cU)
    ပြန်ပေး 1;
  f = fopen ("/proc/mounts", "r");
  အကယ်၍ (f == NULL)
    ပြန်ပေး 0;
  နေစဉ် (getline (&buf, &len, f) >= 0)
    {
      var_na *p = strchr (buf, ' ');
      အကယ်၍ (p == NULL)
        bha_nga;
      p = strchr (p + 1, ' ');
      အကယ်၍ (p == NULL)
        bha_nga;
      အကယ်၍ (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          free (buf);
          fclose (f);
          ပြန်ပေး 1;
        }
    }
  free (buf);
  fclose (f);
  ပြန်ပေး 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#မဟုတ်ပါက

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#အကယ်၍ defined FFI_MMAP_PAX
#include <stdlib.h>

ga_nanaa {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
static ကိန်းပြည့် cached_pax_flags = -1;

static ကိန်းပြည့်
pax_flags_check (void)
{
  var_na *buf = NULL;
  size_t len = 0;
  FILE *f;
  ကိန်းပြည့် ret;
  f = fopen ("/proc/self/status", "r");
  အကယ်၍ (f == NULL)
    ပြန်ပေး 0;
  ret = 0;

  နေစဉ် (getline (&buf, &len, f) != -1)
    အကယ်၍ (!strncmp (buf, "PaX:", 4))
      {
        အကယ်၍ (NULL != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        အကယ်၍ (NULL != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        bha_nga;
      }
  free (buf);
  fclose (f);
  ပြန်ပေး ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#elif defined (__CYGWIN__) || defined(__INTERIX)

#include <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#အကယ်၍ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
static void *dlmalloc(size_t);
static void dlfree(void*);
static void *dlcalloc(size_t, size_t) MAYBE_UNUSED;
static void *dlrealloc(void *, size_t) MAYBE_UNUSED;
static void *dlmemalign(size_t, size_t) MAYBE_UNUSED;
static void *dlvalloc(size_t) MAYBE_UNUSED;
static ကိန်းပြည့် dlmallopt(ကိန်းပြည့်, ကိန်းပြည့်) MAYBE_UNUSED;
static size_t dlmalloc_footprint(void) MAYBE_UNUSED;
static size_t dlmalloc_max_footprint(void) MAYBE_UNUSED;
static void** dlindependent_calloc(size_t, size_t, void**) MAYBE_UNUSED;
static void** dlindependent_comalloc(size_t, size_t*, void**) MAYBE_UNUSED;
static void *dlpvalloc(size_t) MAYBE_UNUSED;
static ကိန်းပြည့် dlmalloc_trim(size_t) MAYBE_UNUSED;
static size_t dlmalloc_usable_size(void*) MAYBE_UNUSED;
static void dlmalloc_stats(void) MAYBE_UNUSED;

#အကယ်၍ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
static void *dlmmap(void *, size_t, ကိန်းပြည့်, ကိန်းပြည့်, ကိန်းပြည့်, off_t);
static ကိန်းပြည့် dlmunmap(void *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#include "dlmalloc.c"

#undef mmap
#undef munmap

#အကယ်၍ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
static pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
static ကိန်းပြည့် execfd = -1;

/* The amount of space already allocated from the temporary file.  */
static size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
static ကိန်းပြည့်
open_temp_exec_file_memfd (const var_na *name)
{
  ကိန်းပြည့် fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ပြန်ပေး fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
static ကိန်းပြည့်
open_temp_exec_file_name (var_na *name, ကိန်းပြည့် flags MAYBE_UNUSED)
{
  ကိန်းပြည့် fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#မဟုတ်ပါက
  fd = mkstemp (name);
#endif

  အကယ်၍ (fd != -1)
    unlink (name);

  ပြန်ပေး fd;
}

/* Open a temporary file in the named directory.  */
static ကိန်းပြည့်
open_temp_exec_file_dir (const var_na *dir)
{
  static const var_na suffix[] = "/ffiXXXXXX";
  ကိန်းပြည့် lendir, flags;
  var_na *tempname;
#ifdef O_TMPFILE
  ကိန်းပြည့် fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#မဟုတ်ပါက
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  အကယ်၍ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ပြန်ပေး fd;
  } မဟုတ်ပါက {
    errno = 0;
  }
#endif

  lendir = (ကိန်းပြည့်) strlen (dir);
  tempname = __builtin_alloca (lendir + parimaa_na (suffix));

  အကယ်၍ (!tempname)
    ပြန်ပေး -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, parimaa_na (suffix));

  ပြန်ပေး open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
static ကိန်းပြည့်
open_temp_exec_file_env (const var_na *envvar)
{
  const var_na *value = getenv (envvar);

  အကယ်၍ (!value)
    ပြန်ပေး -1;

  ပြန်ပေး open_temp_exec_file_dir (value);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
static ကိန်းပြည့်
open_temp_exec_file_mnt (const var_na *mounts)
{
  static const var_na *last_mounts;
  static FILE *last_mntent;

  အကယ်၍ (mounts != last_mounts)
    {
      အကယ်၍ (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      အကယ်၍ (mounts)
	last_mntent = setmntent (mounts, "r");
      မဟုတ်ပါက
	last_mntent = NULL;
    }

  အကယ်၍ (!last_mntent)
    ပြန်ပေး -1;

  အတွက် (;;)
    {
      ကိန်းပြည့် fd;
      sa_nracanaa mntent mnt;
      var_na buf[MAXPATHLEN * 3];

      အကယ်၍ (getmntent_r (last_mntent, &mnt, buf, parimaa_na (buf)) == NULL)
	ပြန်ပေး -1;

      အကယ်၍ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	continue;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      အကယ်၍ (fd != -1)
	ပြန်ပေး fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
static sa_nracanaa
{
  ကိန်းပြည့် (*func)(const var_na *);
  const var_na *arg;
  ကိန်းပြည့် repeat;
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
static ကိန်းပြည့် open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
static ကိန်းပြည့်
open_temp_exec_file_opts_next (void)
{
  အကယ်၍ (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (NULL);

  open_temp_exec_file_opts_idx++;
  အကယ်၍ (open_temp_exec_file_opts_idx
      == (parimaa_na (open_temp_exec_file_opts)
	  / parimaa_na (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      ပြန်ပေး 1;
    }

  ပြန်ပေး 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
ကိန်းပြည့်
open_temp_exec_file (void)
{
  ကိန်းပြည့် fd;

  do
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      အကယ်၍ (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  အကယ်၍ (open_temp_exec_file_opts_next ())
	    bha_nga;
	}
    }
  နေစဉ် (fd == -1);

  ပြန်ပေး fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
static ကိန်းပြည့်
allocate_space (ကိန်းပြည့် fd, off_t len)
{
  static long page_size;

  /* Obtain system page size. */
  အကယ်၍ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  အကယ်၍ (page_size <= 0)
    ပြန်ပေး -1;

  unsigned var_na buf[page_size];
  memset (buf, 0, page_size);

  နေစဉ် (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      အကယ်၍ (write (fd, buf, to_write) < to_write)
        ပြန်ပေး -1;
      len -= to_write;
    }

  ပြန်ပေး 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
static void *
dlmmap_locked (void *start, size_t length, ကိန်းပြည့် prot, ကိန်းပြည့် flags, off_t offset)
{
  void *ptr;

  အကယ်၍ (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      အကယ်၍ (execfd == -1)
	ပြန်ပေး MFAIL;
    }

  offset = execsize;

  အကယ်၍ (allocate_space (execfd, length))
    ပြန်ပေး MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (NULL, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  အကယ်၍ (ptr == MFAIL)
    {
      အကယ်၍ (!offset)
	{
	  close (execfd);
	  goto retry_open;
	}
      အကယ်၍ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ပြန်ပေး MFAIL;
    }
  မဟုတ်ပါက အကယ်၍ (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  အကယ်၍ (start == MFAIL)
    {
      munmap (ptr, length);
      အကယ်၍ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ပြန်ပေး start;
    }

  mmap_exec_offset ((var_na *)start, length) = (var_na*)ptr - (var_na*)start;

  execsize += length;

  ပြန်ပေး start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
static void *
dlmmap (void *start, size_t length, ကိန်းပြည့် prot,
	ကိန်းပြည့် flags, ကိန်းပြည့် fd, off_t offset)
{
  void *ptr;

  assert (start == NULL && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  အကယ်၍ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      ပြန်ပေး ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  အကယ်၍ (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      အကယ်၍ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          ပြန်ပေး ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  မဟုတ်ပါက အကယ်၍ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      အကယ်၍ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ပြန်ပေး ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  ပြန်ပေး ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
static ကိန်းပြည့်
dlmunmap (void *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  void *code;

  အကယ်၍ (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      ကိန်းပြည့် ret = munmap (code, length);
      အကယ်၍ (ret)
	ပြန်ပေး ret;
    }

  ပြန်ပေး munmap (start, length);
}

#အကယ်၍ FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
static msegmentptr
segment_holding_code (mstate m, var_na* addr)
{
  msegmentptr sp = &m->seg;
  အတွက် (;;) {
    အကယ်၍ (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      ပြန်ပေး sp;
    အကယ်၍ ((sp = sp->next) == 0)
      ပြန်ပေး 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
void *
ffi_closure_alloc (size_t size, void **code)
{
  void *ptr, *ftramp;

  အကယ်၍ (!code)
    ပြန်ပေး NULL;

  ptr = dlmalloc (size);

  အကယ်၍ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      အကယ်၍ (!ffi_tramp_is_supported ())
        ပြန်ပေး ptr;

      ftramp = ffi_tramp_alloc (0);
      အကယ်၍ (ftramp == NULL)
      {
        dlfree (ptr);
        ပြန်ပေး NULL;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  ပြန်ပေး ptr;
}

void *
ffi_data_to_code_pointer (void *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  အကယ်၍ (seg)
    {
      အကယ်၍ (!ffi_tramp_is_supported ())
        ပြန်ပေး add_segment_exec_offset (data, seg);
      ပြန်ပေး ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  မဟုတ်ပါက
    ပြန်ပေး data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
void
ffi_closure_free (void *ptr)
{
#အကယ်၍ FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  အကယ်၍ (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  အကယ်၍ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

ကိန်းပြည့်
ffi_tramp_is_present (void *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ပြန်ပေး seg != NULL && ffi_tramp_is_supported();
}

# မဟုတ်ပါက /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#include <stdlib.h>

void *
ffi_closure_alloc (size_t size, void **code)
{
  void *c;

  အကယ်၍ (!code)
    ပြန်ပေး NULL;

  c = malloc (size);
  *code = FFI_FN (c);
  ပြန်ပေး c;
}

void
ffi_closure_free (void *ptr)
{
  free (ptr);
}

void *
ffi_data_to_code_pointer (void *data)
{
  ပြန်ပေး data;
}

ကိန်းပြည့်
ffi_tramp_is_present (__attribute__((unused)) void *ptr)
{
  ပြန်ပေး 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

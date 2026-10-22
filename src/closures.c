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

#जइ (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <tramp.h>

#ifdef __NetBSD__
#अन्तर्भाव <sys/param.h>
#endif

#जइ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#अन्तर्भाव <sys/mman.h>

#अन्तर्भाव <stddef.h>
#अन्तर्भाव <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#अन्तर्भाव <sys/memfd.h>
#endif

स्थिर ध्रुव size_t overhead =
  (परिमाण(max_align_t) > परिमाण(शून्य *) + परिमाण(size_t)) ?
    परिमाण(max_align_t)
    : परिमाण(शून्य *) + परिमाण(size_t);

#define ADD_TO_POINTER(p, d) ((शून्य *)((uintptr_t)(p) + (d)))

शून्य *
ffi_closure_alloc (size_t size, शून्य **code)
{
  स्थिर size_t page_size;
  size_t rounded_size;
  शून्य *codeseg, *dataseg;
  पूर्णाङ्क prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  जइ (!code)
    प्रत्यागम रिक्त;

  /* Obtain system page size. */
  जइ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(रिक्त, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  जइ (dataseg == MAP_FAILED)
    प्रत्यागम रिक्त;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, रिक्त, rounded_size, MAP_REMAPDUP);
  जइ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    प्रत्यागम रिक्त;
  }
  जइ (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    प्रत्यागम रिक्त;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, परिमाण(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, परिमाण(size_t)), &codeseg, परिमाण(शून्य *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  प्रत्यागम ADD_TO_POINTER(dataseg, overhead);
}

शून्य
ffi_closure_free (शून्य *ptr)
{
  शून्य *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, परिमाण(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, परिमाण(size_t)), परिमाण(शून्य *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

पूर्णाङ्क
ffi_tramp_is_present (__attribute__((unused)) शून्य *ptr)
{
  प्रत्यागम 0;
}
#अण्णहा /* !NetBSD with PROT_MPROTECT */

#जइ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# जइ __linux__ && !defined(__ANDROID__)
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
# जइ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#जइ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# जइ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# जइ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#जइ FFI_CLOSURES

#जइ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#अन्तर्भाव <mach/mach.h>
#अन्तर्भाव <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#अन्तर्भाव <ptrauth.h>
#endif
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

बाह्य शून्य *ffi_closure_trampoline_table_page;

प्रकारनिर्देश संरचना ffi_trampoline_table ffi_trampoline_table;
प्रकारनिर्देश संरचना ffi_trampoline_table_entry ffi_trampoline_table_entry;

संरचना ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_गणनम्;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

संरचना ffi_trampoline_table_entry
{
  शून्य *(*trampoline) (शून्य);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_गणनम् (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

स्थिर pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
स्थिर ffi_trampoline_table *ffi_trampoline_tables = रिक्त;

स्थिर ffi_trampoline_table *
ffi_trampoline_table_alloc (शून्य)
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
  जइ (kt != KERN_SUCCESS)
    प्रत्यागम रिक्त;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((शून्य *)&ffi_closure_trampoline_table_page, ptrauth_कुञ्चिका_function_pointer, 0);
#अण्णहा
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  जइ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      प्रत्यागम रिक्त;
    }

  जइ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      जइ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          प्रत्यागम रिक्त;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, परिमाण (ffi_trampoline_table));
  जइ (table == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      प्रत्यागम रिक्त;
    }
  table->free_गणनम् = FFI_TRAMPOLINE_गणनम्;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_गणनम्, परिमाण (ffi_trampoline_table_entry));
  जइ (table->free_list_pool == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      मुक्त (table);
      प्रत्यागम रिक्त;
    }

  प्रति (i = 0; i < table->free_गणनम्; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(शून्य *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_कुञ्चिका_function_pointer, 0);
#endif

      जइ (i < table->free_गणनम् - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  प्रत्यागम table;
}

स्थिर शून्य
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  जइ (table->prev != रिक्त)
    table->prev->next = table->next;

  जइ (table->next != रिक्त)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  मुक्त (table->free_list_pool);
  मुक्त (table);
}

शून्य *
ffi_closure_alloc (size_t size, शून्य **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  जइ (closure == रिक्त)
    प्रत्यागम रिक्त;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  जइ (table == रिक्त || table->free_list == रिक्त)
    {
      table = ffi_trampoline_table_alloc ();
      जइ (table == रिक्त)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  मुक्त (closure);
	  प्रत्यागम रिक्त;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      जइ (table->next != रिक्त)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_गणनम्--;
  entry->next = रिक्त;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  प्रत्यागम closure;
}

शून्य
ffi_closure_free (शून्य *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_गणनम्++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  जइ (table->free_गणनम् == FFI_TRAMPOLINE_गणनम्
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  अण्णहा जइ (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = रिक्त;
      table->next = ffi_trampoline_tables;
      जइ (ffi_trampoline_tables != रिक्त)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  मुक्त (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#नोचेद्यदि FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>
#ifndef _MSC_VER
#अन्तर्भाव <unistd.h>
#endif
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#जइ !defined(_WIN32)
#ifdef HAVE_MNTENT
#अन्तर्भाव <mntent.h>
#endif /* HAVE_MNTENT */
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#अन्तर्भाव <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#जइ FFI_MMAP_EXEC_SELINUX
#अन्तर्भाव <sys/statfs.h>
#अन्तर्भाव <stdlib.h>

स्थिर पूर्णाङ्क selinux_enabled = -1;

स्थिर पूर्णाङ्क
selinux_enabled_check (शून्य)
{
  संरचना statfs sfs;
  FILE *f;
  वर्ण *buf = रिक्त;
  size_t len = 0;

  जइ (statfs ("/selinux", &sfs) >= 0
      && (unsigned पूर्णाङ्क) sfs.f_type == 0xf97cff8cU)
    प्रत्यागम 1;
  f = fopen ("/proc/mounts", "r");
  जइ (f == रिक्त)
    प्रत्यागम 0;
  जाव (getline (&buf, &len, f) >= 0)
    {
      वर्ण *p = strchr (buf, ' ');
      जइ (p == रिक्त)
        भङ्ग;
      p = strchr (p + 1, ' ');
      जइ (p == रिक्त)
        भङ्ग;
      जइ (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          मुक्त (buf);
          fclose (f);
          प्रत्यागम 1;
        }
    }
  मुक्त (buf);
  fclose (f);
  प्रत्यागम 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#अण्णहा

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#जइ defined FFI_MMAP_PAX
#अन्तर्भाव <stdlib.h>

गणना {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
स्थिर पूर्णाङ्क cached_pax_flags = -1;

स्थिर पूर्णाङ्क
pax_flags_check (शून्य)
{
  वर्ण *buf = रिक्त;
  size_t len = 0;
  FILE *f;
  पूर्णाङ्क ret;
  f = fopen ("/proc/self/status", "r");
  जइ (f == रिक्त)
    प्रत्यागम 0;
  ret = 0;

  जाव (getline (&buf, &len, f) != -1)
    जइ (!strncmp (buf, "PaX:", 4))
      {
        जइ (रिक्त != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        जइ (रिक्त != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        भङ्ग;
      }
  मुक्त (buf);
  fclose (f);
  प्रत्यागम ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#नोचेद्यदि defined (__CYGWIN__) || defined(__INTERIX)

#अन्तर्भाव <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#जइ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
स्थिर शून्य *dlmalloc(size_t);
स्थिर शून्य dlfree(शून्य*);
स्थिर शून्य *dlcalloc(size_t, size_t) MAYBE_UNUSED;
स्थिर शून्य *dlrealloc(शून्य *, size_t) MAYBE_UNUSED;
स्थिर शून्य *dlmemalign(size_t, size_t) MAYBE_UNUSED;
स्थिर शून्य *dlvalloc(size_t) MAYBE_UNUSED;
स्थिर पूर्णाङ्क dlmallopt(पूर्णाङ्क, पूर्णाङ्क) MAYBE_UNUSED;
स्थिर size_t dlmalloc_footprint(शून्य) MAYBE_UNUSED;
स्थिर size_t dlmalloc_max_footprint(शून्य) MAYBE_UNUSED;
स्थिर शून्य** dlindependent_calloc(size_t, size_t, शून्य**) MAYBE_UNUSED;
स्थिर शून्य** dlindependent_comalloc(size_t, size_t*, शून्य**) MAYBE_UNUSED;
स्थिर शून्य *dlpvalloc(size_t) MAYBE_UNUSED;
स्थिर पूर्णाङ्क dlmalloc_trim(size_t) MAYBE_UNUSED;
स्थिर size_t dlmalloc_usable_size(शून्य*) MAYBE_UNUSED;
स्थिर शून्य dlmalloc_stats(शून्य) MAYBE_UNUSED;

#जइ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
स्थिर शून्य *dlmmap(शून्य *, size_t, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क, off_t);
स्थिर पूर्णाङ्क dlmunmap(शून्य *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#अन्तर्भाव "dlmalloc.c"

#undef mmap
#undef munmap

#जइ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
स्थिर pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
स्थिर पूर्णाङ्क execfd = -1;

/* The amount of space already allocated from the temporary file.  */
स्थिर size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_memfd (ध्रुव वर्ण *name)
{
  पूर्णाङ्क fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  प्रत्यागम fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_name (वर्ण *name, पूर्णाङ्क flags MAYBE_UNUSED)
{
  पूर्णाङ्क fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#अण्णहा
  fd = mkstemp (name);
#endif

  जइ (fd != -1)
    unlink (name);

  प्रत्यागम fd;
}

/* Open a temporary file in the named directory.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_dir (ध्रुव वर्ण *dir)
{
  स्थिर ध्रुव वर्ण suffix[] = "/ffiXXXXXX";
  पूर्णाङ्क lendir, flags;
  वर्ण *tempname;
#ifdef O_TMPFILE
  पूर्णाङ्क fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#अण्णहा
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  जइ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    प्रत्यागम fd;
  } अण्णहा {
    errno = 0;
  }
#endif

  lendir = (पूर्णाङ्क) strlen (dir);
  tempname = __builtin_alloca (lendir + परिमाण (suffix));

  जइ (!tempname)
    प्रत्यागम -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, परिमाण (suffix));

  प्रत्यागम open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_env (ध्रुव वर्ण *envvar)
{
  ध्रुव वर्ण *मानम् = getenv (envvar);

  जइ (!मानम्)
    प्रत्यागम -1;

  प्रत्यागम open_temp_exec_file_dir (मानम्);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_mnt (ध्रुव वर्ण *mounts)
{
  स्थिर ध्रुव वर्ण *last_mounts;
  स्थिर FILE *last_mntent;

  जइ (mounts != last_mounts)
    {
      जइ (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      जइ (mounts)
	last_mntent = setmntent (mounts, "r");
      अण्णहा
	last_mntent = रिक्त;
    }

  जइ (!last_mntent)
    प्रत्यागम -1;

  प्रति (;;)
    {
      पूर्णाङ्क fd;
      संरचना mntent mnt;
      वर्ण buf[MAXPATHLEN * 3];

      जइ (getmntent_r (last_mntent, &mnt, buf, परिमाण (buf)) == रिक्त)
	प्रत्यागम -1;

      जइ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	अनुवर्त;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      जइ (fd != -1)
	प्रत्यागम fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
स्थिर संरचना
{
  पूर्णाङ्क (*func)(ध्रुव वर्ण *);
  ध्रुव वर्ण *arg;
  पूर्णाङ्क repeat;
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
स्थिर पूर्णाङ्क open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
स्थिर पूर्णाङ्क
open_temp_exec_file_opts_next (शून्य)
{
  जइ (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (रिक्त);

  open_temp_exec_file_opts_idx++;
  जइ (open_temp_exec_file_opts_idx
      == (परिमाण (open_temp_exec_file_opts)
	  / परिमाण (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      प्रत्यागम 1;
    }

  प्रत्यागम 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
पूर्णाङ्क
open_temp_exec_file (शून्य)
{
  पूर्णाङ्क fd;

  करेहि
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      जइ (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  जइ (open_temp_exec_file_opts_next ())
	    भङ्ग;
	}
    }
  जाव (fd == -1);

  प्रत्यागम fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
स्थिर पूर्णाङ्क
allocate_space (पूर्णाङ्क fd, off_t len)
{
  स्थिर long page_size;

  /* Obtain system page size. */
  जइ (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  जइ (page_size <= 0)
    प्रत्यागम -1;

  unsigned वर्ण buf[page_size];
  memset (buf, 0, page_size);

  जाव (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      जइ (लिख (fd, buf, to_write) < to_write)
        प्रत्यागम -1;
      len -= to_write;
    }

  प्रत्यागम 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
स्थिर शून्य *
dlmmap_locked (शून्य *start, size_t length, पूर्णाङ्क prot, पूर्णाङ्क flags, off_t offset)
{
  शून्य *ptr;

  जइ (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      जइ (execfd == -1)
	प्रत्यागम MFAIL;
    }

  offset = execsize;

  जइ (allocate_space (execfd, length))
    प्रत्यागम MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (रिक्त, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  जइ (ptr == MFAIL)
    {
      जइ (!offset)
	{
	  close (execfd);
	  गच्छ retry_open;
	}
      जइ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      प्रत्यागम MFAIL;
    }
  अण्णहा जइ (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  जइ (start == MFAIL)
    {
      munmap (ptr, length);
      जइ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      प्रत्यागम start;
    }

  mmap_exec_offset ((वर्ण *)start, length) = (वर्ण*)ptr - (वर्ण*)start;

  execsize += length;

  प्रत्यागम start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
स्थिर शून्य *
dlmmap (शून्य *start, size_t length, पूर्णाङ्क prot,
	पूर्णाङ्क flags, पूर्णाङ्क fd, off_t offset)
{
  शून्य *ptr;

  प्रतिज्ञा (start == रिक्त && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  जइ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      प्रत्यागम ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  जइ (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      जइ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          प्रत्यागम ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  अण्णहा जइ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      जइ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	प्रत्यागम ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  प्रत्यागम ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
स्थिर पूर्णाङ्क
dlmunmap (शून्य *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  शून्य *code;

  जइ (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      पूर्णाङ्क ret = munmap (code, length);
      जइ (ret)
	प्रत्यागम ret;
    }

  प्रत्यागम munmap (start, length);
}

#जइ FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
स्थिर msegmentptr
segment_holding_code (mstate m, वर्ण* addr)
{
  msegmentptr sp = &m->seg;
  प्रति (;;) {
    जइ (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      प्रत्यागम sp;
    जइ ((sp = sp->next) == 0)
      प्रत्यागम 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
शून्य *
ffi_closure_alloc (size_t size, शून्य **code)
{
  शून्य *ptr, *ftramp;

  जइ (!code)
    प्रत्यागम रिक्त;

  ptr = dlmalloc (size);

  जइ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      जइ (!ffi_tramp_is_supported ())
        प्रत्यागम ptr;

      ftramp = ffi_tramp_alloc (0);
      जइ (ftramp == रिक्त)
      {
        dlfree (ptr);
        प्रत्यागम रिक्त;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  प्रत्यागम ptr;
}

शून्य *
ffi_data_to_code_pointer (शून्य *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  जइ (seg)
    {
      जइ (!ffi_tramp_is_supported ())
        प्रत्यागम add_segment_exec_offset (data, seg);
      प्रत्यागम ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  अण्णहा
    प्रत्यागम data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
शून्य
ffi_closure_free (शून्य *ptr)
{
#जइ FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  जइ (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  जइ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

पूर्णाङ्क
ffi_tramp_is_present (शून्य *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  प्रत्यागम seg != रिक्त && ffi_tramp_is_supported();
}

# अण्णहा /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#अन्तर्भाव <stdlib.h>

शून्य *
ffi_closure_alloc (size_t size, शून्य **code)
{
  शून्य *c;

  जइ (!code)
    प्रत्यागम रिक्त;

  c = malloc (size);
  *code = FFI_FN (c);
  प्रत्यागम c;
}

शून्य
ffi_closure_free (शून्य *ptr)
{
  मुक्त (ptr);
}

शून्य *
ffi_data_to_code_pointer (शून्य *data)
{
  प्रत्यागम data;
}

पूर्णाङ्क
ffi_tramp_is_present (__attribute__((unused)) शून्य *ptr)
{
  प्रत्यागम 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

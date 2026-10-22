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

#जर (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
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

#जर __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#अन्तर्भाव <sys/mman.h>

#अन्तर्भाव <stddef.h>
#अन्तर्भाव <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#अन्तर्भाव <sys/memfd.h>
#endif

स्थिर स्थिरांक माप_प्रकार overhead =
  (आकार(max_align_t) > आकार(शून्य *) + आकार(माप_प्रकार)) ?
    आकार(max_align_t)
    : आकार(शून्य *) + आकार(माप_प्रकार);

#define ADD_TO_POINTER(p, d) ((शून्य *)((uintptr_t)(p) + (d)))

शून्य *
ffi_closure_alloc (माप_प्रकार size, शून्य **code)
{
  स्थिर माप_प्रकार page_size;
  माप_प्रकार rounded_size;
  शून्य *codeseg, *dataseg;
  पूर्णांक prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  जर (!code)
    परत रिक्त;

  /* Obtain system page size. */
  जर (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(रिक्त, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  जर (dataseg == MAP_FAILED)
    परत रिक्त;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, रिक्त, rounded_size, MAP_REMAPDUP);
  जर (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    परत रिक्त;
  }
  जर (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    परत रिक्त;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  स_नकल(dataseg, &rounded_size, आकार(rounded_size));
  स_नकल(ADD_TO_POINTER(dataseg, आकार(माप_प्रकार)), &codeseg, आकार(शून्य *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  परत ADD_TO_POINTER(dataseg, overhead);
}

शून्य
ffi_closure_free (शून्य *ptr)
{
  शून्य *codeseg, *dataseg;
  माप_प्रकार rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  स_नकल(&rounded_size, dataseg, आकार(rounded_size));
  स_नकल(&codeseg, ADD_TO_POINTER(dataseg, आकार(माप_प्रकार)), आकार(शून्य *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

पूर्णांक
ffi_tramp_is_present (__attribute__((unused)) शून्य *ptr)
{
  परत 0;
}
#अन्यथा /* !NetBSD with PROT_MPROTECT */

#जर !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# जर __linux__ && !defined(__ANDROID__)
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
# जर defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#जर FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# जर !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# जर !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#जर FFI_CLOSURES

#जर FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#अन्तर्भाव <mach/mach.h>
#अन्तर्भाव <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#अन्तर्भाव <ptrauth.h>
#endif
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

बाह्य शून्य *ffi_closure_trampoline_table_page;

प्रकारव्याख्या संरचना ffi_trampoline_table ffi_trampoline_table;
प्रकारव्याख्या संरचना ffi_trampoline_table_entry ffi_trampoline_table_entry;

संरचना ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_मोजणी;
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
#define FFI_TRAMPOLINE_मोजणी (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

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
  जर (kt != KERN_SUCCESS)
    परत रिक्त;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((शून्य *)&ffi_closure_trampoline_table_page, ptrauth_किल्ली_function_pointer, 0);
#अन्यथा
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  जर (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      परत रिक्त;
    }

  जर (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      जर (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          परत रिक्त;
        }
    }

  /* We have valid trampoline and config pages */
  table = सुस्मृति (1, आकार (ffi_trampoline_table));
  जर (table == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      परत रिक्त;
    }
  table->free_मोजणी = FFI_TRAMPOLINE_मोजणी;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    सुस्मृति (FFI_TRAMPOLINE_मोजणी, आकार (ffi_trampoline_table_entry));
  जर (table->free_list_pool == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      मुक्त (table);
      परत रिक्त;
    }

  साठी (i = 0; i < table->free_मोजणी; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(शून्य *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_किल्ली_function_pointer, 0);
#endif

      जर (i < table->free_मोजणी - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  परत table;
}

स्थिर शून्य
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  जर (table->prev != रिक्त)
    table->prev->next = table->next;

  जर (table->next != रिक्त)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  मुक्त (table->free_list_pool);
  मुक्त (table);
}

शून्य *
ffi_closure_alloc (माप_प्रकार size, शून्य **code)
{
  /* Create the closure */
  ffi_closure *closure = दो_स्मृति (size);
  जर (closure == रिक्त)
    परत रिक्त;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  जर (table == रिक्त || table->free_list == रिक्त)
    {
      table = ffi_trampoline_table_alloc ();
      जर (table == रिक्त)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  मुक्त (closure);
	  परत रिक्त;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      जर (table->next != रिक्त)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_मोजणी--;
  entry->next = रिक्त;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  परत closure;
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
  table->free_मोजणी++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  जर (table->free_मोजणी == FFI_TRAMPOLINE_मोजणी
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  अन्यथा जर (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = रिक्त;
      table->next = ffi_trampoline_tables;
      जर (ffi_trampoline_tables != रिक्त)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  मुक्त (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#किंवाजर FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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
#define DEFAULT_GRANULARITY ((माप_प्रकार)malloc_getpagesize)

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>
#ifndef _MSC_VER
#अन्तर्भाव <unistd.h>
#endif
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#जर !defined(_WIN32)
#ifdef HAVE_MNTENT
#अन्तर्भाव <mntent.h>
#endif /* HAVE_MNTENT */
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#अन्तर्भाव <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#जर FFI_MMAP_EXEC_SELINUX
#अन्तर्भाव <sys/statfs.h>
#अन्तर्भाव <stdlib.h>

स्थिर पूर्णांक selinux_enabled = -1;

स्थिर पूर्णांक
selinux_enabled_check (शून्य)
{
  संरचना statfs sfs;
  खाता *f;
  अक्षर *buf = रिक्त;
  माप_प्रकार len = 0;

  जर (statfs ("/selinux", &sfs) >= 0
      && (अचिन्हित पूर्णांक) sfs.f_type == 0xf97cff8cU)
    परत 1;
  f = ख_खोलो ("/proc/mounts", "r");
  जर (f == रिक्त)
    परत 0;
  पर्यंत (getline (&buf, &len, f) >= 0)
    {
      अक्षर *p = म_अक्षर (buf, ' ');
      जर (p == रिक्त)
        थांबा;
      p = म_अक्षर (p + 1, ' ');
      जर (p == रिक्त)
        थांबा;
      जर (म_भेदन (p + 1, "selinuxfs ", 10) == 0)
        {
          मुक्त (buf);
          ख_बंद (f);
          परत 1;
        }
    }
  मुक्त (buf);
  ख_बंद (f);
  परत 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#अन्यथा

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#जर defined FFI_MMAP_PAX
#अन्तर्भाव <stdlib.h>

गणना {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
स्थिर पूर्णांक cached_pax_flags = -1;

स्थिर पूर्णांक
pax_flags_check (शून्य)
{
  अक्षर *buf = रिक्त;
  माप_प्रकार len = 0;
  खाता *f;
  पूर्णांक ret;
  f = ख_खोलो ("/proc/self/status", "r");
  जर (f == रिक्त)
    परत 0;
  ret = 0;

  पर्यंत (getline (&buf, &len, f) != -1)
    जर (!म_भेदन (buf, "PaX:", 4))
      {
        जर (रिक्त != म_अक्षर (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        जर (रिक्त != म_अक्षर (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        थांबा;
      }
  मुक्त (buf);
  ख_बंद (f);
  परत ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#किंवाजर defined (__CYGWIN__) || defined(__INTERIX)

#अन्तर्भाव <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#जर !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
स्थिर शून्य *dlmalloc(माप_प्रकार);
स्थिर शून्य dlfree(शून्य*);
स्थिर शून्य *dlcalloc(माप_प्रकार, माप_प्रकार) MAYBE_UNUSED;
स्थिर शून्य *dlrealloc(शून्य *, माप_प्रकार) MAYBE_UNUSED;
स्थिर शून्य *dlmemalign(माप_प्रकार, माप_प्रकार) MAYBE_UNUSED;
स्थिर शून्य *dlvalloc(माप_प्रकार) MAYBE_UNUSED;
स्थिर पूर्णांक dlmallopt(पूर्णांक, पूर्णांक) MAYBE_UNUSED;
स्थिर माप_प्रकार dlmalloc_footprint(शून्य) MAYBE_UNUSED;
स्थिर माप_प्रकार dlmalloc_max_footprint(शून्य) MAYBE_UNUSED;
स्थिर शून्य** dlindependent_calloc(माप_प्रकार, माप_प्रकार, शून्य**) MAYBE_UNUSED;
स्थिर शून्य** dlindependent_comalloc(माप_प्रकार, माप_प्रकार*, शून्य**) MAYBE_UNUSED;
स्थिर शून्य *dlpvalloc(माप_प्रकार) MAYBE_UNUSED;
स्थिर पूर्णांक dlmalloc_trim(माप_प्रकार) MAYBE_UNUSED;
स्थिर माप_प्रकार dlmalloc_usable_size(शून्य*) MAYBE_UNUSED;
स्थिर शून्य dlmalloc_stats(शून्य) MAYBE_UNUSED;

#जर !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
स्थिर शून्य *dlmmap(शून्य *, माप_प्रकार, पूर्णांक, पूर्णांक, पूर्णांक, off_t);
स्थिर पूर्णांक dlmunmap(शून्य *, माप_प्रकार);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#अन्तर्भाव "dlmalloc.c"

#undef mmap
#undef munmap

#जर !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
स्थिर pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
स्थिर पूर्णांक execfd = -1;

/* The amount of space already allocated from the temporary file.  */
स्थिर माप_प्रकार execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णांक
open_temp_exec_file_memfd (स्थिरांक अक्षर *name)
{
  पूर्णांक fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  परत fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णांक
open_temp_exec_file_name (अक्षर *name, पूर्णांक flags MAYBE_UNUSED)
{
  पूर्णांक fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#अन्यथा
  fd = mkstemp (name);
#endif

  जर (fd != -1)
    unlink (name);

  परत fd;
}

/* Open a temporary file in the named directory.  */
स्थिर पूर्णांक
open_temp_exec_file_dir (स्थिरांक अक्षर *dir)
{
  स्थिर स्थिरांक अक्षर suffix[] = "/ffiXXXXXX";
  पूर्णांक lendir, flags;
  अक्षर *tempname;
#ifdef O_TMPFILE
  पूर्णांक fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#अन्यथा
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = खोलो (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  जर (fd != -1 || (त्रुटि_सं != EINVAL && त्रुटि_सं != EISDIR && त्रुटि_सं != EOPNOTSUPP)) {
    परत fd;
  } अन्यथा {
    त्रुटि_सं = 0;
  }
#endif

  lendir = (पूर्णांक) म_माप (dir);
  tempname = __builtin_alloca (lendir + आकार (suffix));

  जर (!tempname)
    परत -1;

  स_नकल (tempname, dir, lendir);
  स_नकल (tempname + lendir, suffix, आकार (suffix));

  परत open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
स्थिर पूर्णांक
open_temp_exec_file_env (स्थिरांक अक्षर *envvar)
{
  स्थिरांक अक्षर *मूल्य = दो_पर्या (envvar);

  जर (!मूल्य)
    परत -1;

  परत open_temp_exec_file_dir (मूल्य);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
स्थिर पूर्णांक
open_temp_exec_file_mnt (स्थिरांक अक्षर *mounts)
{
  स्थिर स्थिरांक अक्षर *last_mounts;
  स्थिर खाता *last_mntent;

  जर (mounts != last_mounts)
    {
      जर (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      जर (mounts)
	last_mntent = setmntent (mounts, "r");
      अन्यथा
	last_mntent = रिक्त;
    }

  जर (!last_mntent)
    परत -1;

  साठी (;;)
    {
      पूर्णांक fd;
      संरचना mntent mnt;
      अक्षर buf[MAXPATHLEN * 3];

      जर (getmntent_r (last_mntent, &mnt, buf, आकार (buf)) == रिक्त)
	परत -1;

      जर (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	सुरूठेवा;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      जर (fd != -1)
	परत fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
स्थिर संरचना
{
  पूर्णांक (*func)(स्थिरांक अक्षर *);
  स्थिरांक अक्षर *arg;
  पूर्णांक repeat;
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
स्थिर पूर्णांक open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
स्थिर पूर्णांक
open_temp_exec_file_opts_next (शून्य)
{
  जर (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (रिक्त);

  open_temp_exec_file_opts_idx++;
  जर (open_temp_exec_file_opts_idx
      == (आकार (open_temp_exec_file_opts)
	  / आकार (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      परत 1;
    }

  परत 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
पूर्णांक
open_temp_exec_file (शून्य)
{
  पूर्णांक fd;

  करा
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      जर (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  जर (open_temp_exec_file_opts_next ())
	    थांबा;
	}
    }
  पर्यंत (fd == -1);

  परत fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
स्थिर पूर्णांक
allocate_space (पूर्णांक fd, off_t len)
{
  स्थिर दीर्घ page_size;

  /* Obtain system page size. */
  जर (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  जर (page_size <= 0)
    परत -1;

  अचिन्हित अक्षर buf[page_size];
  स_रखो (buf, 0, page_size);

  पर्यंत (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      जर (लिख (fd, buf, to_write) < to_write)
        परत -1;
      len -= to_write;
    }

  परत 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
स्थिर शून्य *
dlmmap_locked (शून्य *start, माप_प्रकार length, पूर्णांक prot, पूर्णांक flags, off_t offset)
{
  शून्य *ptr;

  जर (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      जर (execfd == -1)
	परत MFAIL;
    }

  offset = execsize;

  जर (allocate_space (execfd, length))
    परत MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (रिक्त, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  जर (ptr == MFAIL)
    {
      जर (!offset)
	{
	  बंद (execfd);
	  जा retry_open;
	}
      जर (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      परत MFAIL;
    }
  अन्यथा जर (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  जर (start == MFAIL)
    {
      munmap (ptr, length);
      जर (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      परत start;
    }

  mmap_exec_offset ((अक्षर *)start, length) = (अक्षर*)ptr - (अक्षर*)start;

  execsize += length;

  परत start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
स्थिर शून्य *
dlmmap (शून्य *start, माप_प्रकार length, पूर्णांक prot,
	पूर्णांक flags, पूर्णांक fd, off_t offset)
{
  शून्य *ptr;

  प्रतिज्ञा (start == रिक्त && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  जर (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      परत ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  जर (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      जर (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          परत ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  अन्यथा जर (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      जर (ptr != MFAIL || (त्रुटि_सं != EPERM && त्रुटि_सं != EACCES))
	/* Cool, no need to mess with separate segments.  */
	परत ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  परत ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
स्थिर पूर्णांक
dlmunmap (शून्य *start, माप_प्रकार length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  शून्य *code;

  जर (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      पूर्णांक ret = munmap (code, length);
      जर (ret)
	परत ret;
    }

  परत munmap (start, length);
}

#जर FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
स्थिर msegmentptr
segment_holding_code (mstate m, अक्षर* addr)
{
  msegmentptr sp = &m->seg;
  साठी (;;) {
    जर (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      परत sp;
    जर ((sp = sp->next) == 0)
      परत 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
शून्य *
ffi_closure_alloc (माप_प्रकार size, शून्य **code)
{
  शून्य *ptr, *ftramp;

  जर (!code)
    परत रिक्त;

  ptr = dlmalloc (size);

  जर (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      जर (!ffi_tramp_is_supported ())
        परत ptr;

      ftramp = ffi_tramp_alloc (0);
      जर (ftramp == रिक्त)
      {
        dlfree (ptr);
        परत रिक्त;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  परत ptr;
}

शून्य *
ffi_data_to_code_pointer (शून्य *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  जर (seg)
    {
      जर (!ffi_tramp_is_supported ())
        परत add_segment_exec_offset (data, seg);
      परत ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  अन्यथा
    परत data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
शून्य
ffi_closure_free (शून्य *ptr)
{
#जर FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  जर (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  जर (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

पूर्णांक
ffi_tramp_is_present (शून्य *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  परत seg != रिक्त && ffi_tramp_is_supported();
}

# अन्यथा /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#अन्तर्भाव <stdlib.h>

शून्य *
ffi_closure_alloc (माप_प्रकार size, शून्य **code)
{
  शून्य *c;

  जर (!code)
    परत रिक्त;

  c = दो_स्मृति (size);
  *code = FFI_FN (c);
  परत c;
}

शून्य
ffi_closure_free (शून्य *ptr)
{
  मुक्त (ptr);
}

शून्य *
ffi_data_to_code_pointer (शून्य *data)
{
  परत data;
}

पूर्णांक
ffi_tramp_is_present (__attribute__((unused)) शून्य *ptr)
{
  परत 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

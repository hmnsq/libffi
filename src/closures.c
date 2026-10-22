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

#જો (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#અન્તર્ભાવ <fficonfig.h>
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <tramp.h>

#ifdef __NetBSD__
#અન્તર્ભાવ <sys/param.h>
#endif

#જો __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#અન્તર્ભાવ <sys/mman.h>

#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#અન્તર્ભાવ <sys/memfd.h>
#endif

સ્થિર અચલ size_t overhead =
  (કદ(max_align_t) > કદ(શૂન્ય *) + કદ(size_t)) ?
    કદ(max_align_t)
    : કદ(શૂન્ય *) + કદ(size_t);

#define ADD_TO_POINTER(p, d) ((શૂન્ય *)((uintptr_t)(p) + (d)))

શૂન્ય *
ffi_closure_alloc (size_t size, શૂન્ય **code)
{
  સ્થિર size_t page_size;
  size_t rounded_size;
  શૂન્ય *codeseg, *dataseg;
  પૂર્ણાંક prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  જો (!code)
    પાછા ખાલી;

  /* Obtain system page size. */
  જો (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ખાલી, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  જો (dataseg == MAP_FAILED)
    પાછા ખાલી;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, ખાલી, rounded_size, MAP_REMAPDUP);
  જો (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    પાછા ખાલી;
  }
  જો (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    પાછા ખાલી;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, કદ(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, કદ(size_t)), &codeseg, કદ(શૂન્ય *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  પાછા ADD_TO_POINTER(dataseg, overhead);
}

શૂન્ય
ffi_closure_free (શૂન્ય *ptr)
{
  શૂન્ય *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, કદ(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, કદ(size_t)), કદ(શૂન્ય *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

પૂર્ણાંક
ffi_tramp_is_present (__attribute__((unused)) શૂન્ય *ptr)
{
  પાછા 0;
}
#નહીંતર /* !NetBSD with PROT_MPROTECT */

#જો !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# જો __linux__ && !defined(__ANDROID__)
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
# જો defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#જો FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# જો !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# જો !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#જો FFI_CLOSURES

#જો FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#અન્તર્ભાવ <mach/mach.h>
#અન્તર્ભાવ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#અન્તર્ભાવ <ptrauth.h>
#endif
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>

બાહ્ય શૂન્ય *ffi_closure_trampoline_table_page;

પ્રકારવ્યાખ્યા સંરચના ffi_trampoline_table ffi_trampoline_table;
પ્રકારવ્યાખ્યા સંરચના ffi_trampoline_table_entry ffi_trampoline_table_entry;

સંરચના ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_ગણતરી;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

સંરચના ffi_trampoline_table_entry
{
  શૂન્ય *(*trampoline) (શૂન્ય);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_ગણતરી (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

સ્થિર pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
સ્થિર ffi_trampoline_table *ffi_trampoline_tables = ખાલી;

સ્થિર ffi_trampoline_table *
ffi_trampoline_table_alloc (શૂન્ય)
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
  જો (kt != KERN_SUCCESS)
    પાછા ખાલી;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((શૂન્ય *)&ffi_closure_trampoline_table_page, ptrauth_ચાવી_function_pointer, 0);
#નહીંતર
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  જો (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      પાછા ખાલી;
    }

  જો (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      જો (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          પાછા ખાલી;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, કદ (ffi_trampoline_table));
  જો (table == ખાલી)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      પાછા ખાલી;
    }
  table->free_ગણતરી = FFI_TRAMPOLINE_ગણતરી;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_ગણતરી, કદ (ffi_trampoline_table_entry));
  જો (table->free_list_pool == ખાલી)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      મુક્ત (table);
      પાછા ખાલી;
    }

  માટે (i = 0; i < table->free_ગણતરી; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(શૂન્ય *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_ચાવી_function_pointer, 0);
#endif

      જો (i < table->free_ગણતરી - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  પાછા table;
}

સ્થિર શૂન્ય
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  જો (table->prev != ખાલી)
    table->prev->next = table->next;

  જો (table->next != ખાલી)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  મુક્ત (table->free_list_pool);
  મુક્ત (table);
}

શૂન્ય *
ffi_closure_alloc (size_t size, શૂન્ય **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  જો (closure == ખાલી)
    પાછા ખાલી;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  જો (table == ખાલી || table->free_list == ખાલી)
    {
      table = ffi_trampoline_table_alloc ();
      જો (table == ખાલી)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  મુક્ત (closure);
	  પાછા ખાલી;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      જો (table->next != ખાલી)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_ગણતરી--;
  entry->next = ખાલી;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  પાછા closure;
}

શૂન્ય
ffi_closure_free (શૂન્ય *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_ગણતરી++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  જો (table->free_ગણતરી == FFI_TRAMPOLINE_ગણતરી
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  નહીંતર જો (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = ખાલી;
      table->next = ffi_trampoline_tables;
      જો (ffi_trampoline_tables != ખાલી)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  મુક્ત (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#અથવાજો FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <errno.h>
#ifndef _MSC_VER
#અન્તર્ભાવ <unistd.h>
#endif
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <stdio.h>
#જો !defined(_WIN32)
#ifdef HAVE_MNTENT
#અન્તર્ભાવ <mntent.h>
#endif /* HAVE_MNTENT */
#અન્તર્ભાવ <sys/param.h>
#અન્તર્ભાવ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#અન્તર્ભાવ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#જો FFI_MMAP_EXEC_SELINUX
#અન્તર્ભાવ <sys/statfs.h>
#અન્તર્ભાવ <stdlib.h>

સ્થિર પૂર્ણાંક selinux_enabled = -1;

સ્થિર પૂર્ણાંક
selinux_enabled_check (શૂન્ય)
{
  સંરચના statfs sfs;
  FILE *f;
  અક્ષર *buf = ખાલી;
  size_t len = 0;

  જો (statfs ("/selinux", &sfs) >= 0
      && (unsigned પૂર્ણાંક) sfs.f_type == 0xf97cff8cU)
    પાછા 1;
  f = fopen ("/proc/mounts", "r");
  જો (f == ખાલી)
    પાછા 0;
  જ્યારે (getline (&buf, &len, f) >= 0)
    {
      અક્ષર *p = strchr (buf, ' ');
      જો (p == ખાલી)
        તોડો;
      p = strchr (p + 1, ' ');
      જો (p == ખાલી)
        તોડો;
      જો (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          મુક્ત (buf);
          fclose (f);
          પાછા 1;
        }
    }
  મુક્ત (buf);
  fclose (f);
  પાછા 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#નહીંતર

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#જો defined FFI_MMAP_PAX
#અન્તર્ભાવ <stdlib.h>

ગણના {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
સ્થિર પૂર્ણાંક cached_pax_flags = -1;

સ્થિર પૂર્ણાંક
pax_flags_check (શૂન્ય)
{
  અક્ષર *buf = ખાલી;
  size_t len = 0;
  FILE *f;
  પૂર્ણાંક ret;
  f = fopen ("/proc/self/status", "r");
  જો (f == ખાલી)
    પાછા 0;
  ret = 0;

  જ્યારે (getline (&buf, &len, f) != -1)
    જો (!strncmp (buf, "PaX:", 4))
      {
        જો (ખાલી != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        જો (ખાલી != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        તોડો;
      }
  મુક્ત (buf);
  fclose (f);
  પાછા ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#અથવાજો defined (__CYGWIN__) || defined(__INTERIX)

#અન્તર્ભાવ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#જો !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
સ્થિર શૂન્ય *dlmalloc(size_t);
સ્થિર શૂન્ય dlfree(શૂન્ય*);
સ્થિર શૂન્ય *dlcalloc(size_t, size_t) MAYBE_UNUSED;
સ્થિર શૂન્ય *dlrealloc(શૂન્ય *, size_t) MAYBE_UNUSED;
સ્થિર શૂન્ય *dlmemalign(size_t, size_t) MAYBE_UNUSED;
સ્થિર શૂન્ય *dlvalloc(size_t) MAYBE_UNUSED;
સ્થિર પૂર્ણાંક dlmallopt(પૂર્ણાંક, પૂર્ણાંક) MAYBE_UNUSED;
સ્થિર size_t dlmalloc_footprint(શૂન્ય) MAYBE_UNUSED;
સ્થિર size_t dlmalloc_max_footprint(શૂન્ય) MAYBE_UNUSED;
સ્થિર શૂન્ય** dlindependent_calloc(size_t, size_t, શૂન્ય**) MAYBE_UNUSED;
સ્થિર શૂન્ય** dlindependent_comalloc(size_t, size_t*, શૂન્ય**) MAYBE_UNUSED;
સ્થિર શૂન્ય *dlpvalloc(size_t) MAYBE_UNUSED;
સ્થિર પૂર્ણાંક dlmalloc_trim(size_t) MAYBE_UNUSED;
સ્થિર size_t dlmalloc_usable_size(શૂન્ય*) MAYBE_UNUSED;
સ્થિર શૂન્ય dlmalloc_stats(શૂન્ય) MAYBE_UNUSED;

#જો !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
સ્થિર શૂન્ય *dlmmap(શૂન્ય *, size_t, પૂર્ણાંક, પૂર્ણાંક, પૂર્ણાંક, off_t);
સ્થિર પૂર્ણાંક dlmunmap(શૂન્ય *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#અન્તર્ભાવ "dlmalloc.c"

#undef mmap
#undef munmap

#જો !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
સ્થિર pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
સ્થિર પૂર્ણાંક execfd = -1;

/* The amount of space already allocated from the temporary file.  */
સ્થિર size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_memfd (અચલ અક્ષર *name)
{
  પૂર્ણાંક fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  પાછા fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_name (અક્ષર *name, પૂર્ણાંક flags MAYBE_UNUSED)
{
  પૂર્ણાંક fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#નહીંતર
  fd = mkstemp (name);
#endif

  જો (fd != -1)
    unlink (name);

  પાછા fd;
}

/* Open a temporary file in the named directory.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_dir (અચલ અક્ષર *dir)
{
  સ્થિર અચલ અક્ષર suffix[] = "/ffiXXXXXX";
  પૂર્ણાંક lendir, flags;
  અક્ષર *tempname;
#ifdef O_TMPFILE
  પૂર્ણાંક fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#નહીંતર
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  જો (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    પાછા fd;
  } નહીંતર {
    errno = 0;
  }
#endif

  lendir = (પૂર્ણાંક) strlen (dir);
  tempname = __builtin_alloca (lendir + કદ (suffix));

  જો (!tempname)
    પાછા -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, કદ (suffix));

  પાછા open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_env (અચલ અક્ષર *envvar)
{
  અચલ અક્ષર *કિંમત = getenv (envvar);

  જો (!કિંમત)
    પાછા -1;

  પાછા open_temp_exec_file_dir (કિંમત);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_mnt (અચલ અક્ષર *mounts)
{
  સ્થિર અચલ અક્ષર *last_mounts;
  સ્થિર FILE *last_mntent;

  જો (mounts != last_mounts)
    {
      જો (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      જો (mounts)
	last_mntent = setmntent (mounts, "r");
      નહીંતર
	last_mntent = ખાલી;
    }

  જો (!last_mntent)
    પાછા -1;

  માટે (;;)
    {
      પૂર્ણાંક fd;
      સંરચના mntent mnt;
      અક્ષર buf[MAXPATHLEN * 3];

      જો (getmntent_r (last_mntent, &mnt, buf, કદ (buf)) == ખાલી)
	પાછા -1;

      જો (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ચાલુ;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      જો (fd != -1)
	પાછા fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
સ્થિર સંરચના
{
  પૂર્ણાંક (*func)(અચલ અક્ષર *);
  અચલ અક્ષર *arg;
  પૂર્ણાંક repeat;
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
સ્થિર પૂર્ણાંક open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
સ્થિર પૂર્ણાંક
open_temp_exec_file_opts_next (શૂન્ય)
{
  જો (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (ખાલી);

  open_temp_exec_file_opts_idx++;
  જો (open_temp_exec_file_opts_idx
      == (કદ (open_temp_exec_file_opts)
	  / કદ (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      પાછા 1;
    }

  પાછા 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
પૂર્ણાંક
open_temp_exec_file (શૂન્ય)
{
  પૂર્ણાંક fd;

  કરો
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      જો (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  જો (open_temp_exec_file_opts_next ())
	    તોડો;
	}
    }
  જ્યારે (fd == -1);

  પાછા fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
સ્થિર પૂર્ણાંક
allocate_space (પૂર્ણાંક fd, off_t len)
{
  સ્થિર long page_size;

  /* Obtain system page size. */
  જો (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  જો (page_size <= 0)
    પાછા -1;

  unsigned અક્ષર buf[page_size];
  memset (buf, 0, page_size);

  જ્યારે (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      જો (લિખ (fd, buf, to_write) < to_write)
        પાછા -1;
      len -= to_write;
    }

  પાછા 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
સ્થિર શૂન્ય *
dlmmap_locked (શૂન્ય *start, size_t length, પૂર્ણાંક prot, પૂર્ણાંક flags, off_t offset)
{
  શૂન્ય *ptr;

  જો (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      જો (execfd == -1)
	પાછા MFAIL;
    }

  offset = execsize;

  જો (allocate_space (execfd, length))
    પાછા MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ખાલી, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  જો (ptr == MFAIL)
    {
      જો (!offset)
	{
	  close (execfd);
	  જાઓ retry_open;
	}
      જો (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      પાછા MFAIL;
    }
  નહીંતર જો (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  જો (start == MFAIL)
    {
      munmap (ptr, length);
      જો (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      પાછા start;
    }

  mmap_exec_offset ((અક્ષર *)start, length) = (અક્ષર*)ptr - (અક્ષર*)start;

  execsize += length;

  પાછા start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
સ્થિર શૂન્ય *
dlmmap (શૂન્ય *start, size_t length, પૂર્ણાંક prot,
	પૂર્ણાંક flags, પૂર્ણાંક fd, off_t offset)
{
  શૂન્ય *ptr;

  પ્રતિજ્ઞા (start == ખાલી && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  જો (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      પાછા ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  જો (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      જો (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          પાછા ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  નહીંતર જો (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      જો (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	પાછા ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  પાછા ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
સ્થિર પૂર્ણાંક
dlmunmap (શૂન્ય *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  શૂન્ય *code;

  જો (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      પૂર્ણાંક ret = munmap (code, length);
      જો (ret)
	પાછા ret;
    }

  પાછા munmap (start, length);
}

#જો FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
સ્થિર msegmentptr
segment_holding_code (mstate m, અક્ષર* addr)
{
  msegmentptr sp = &m->seg;
  માટે (;;) {
    જો (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      પાછા sp;
    જો ((sp = sp->next) == 0)
      પાછા 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
શૂન્ય *
ffi_closure_alloc (size_t size, શૂન્ય **code)
{
  શૂન્ય *ptr, *ftramp;

  જો (!code)
    પાછા ખાલી;

  ptr = dlmalloc (size);

  જો (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      જો (!ffi_tramp_is_supported ())
        પાછા ptr;

      ftramp = ffi_tramp_alloc (0);
      જો (ftramp == ખાલી)
      {
        dlfree (ptr);
        પાછા ખાલી;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  પાછા ptr;
}

શૂન્ય *
ffi_data_to_code_pointer (શૂન્ય *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  જો (seg)
    {
      જો (!ffi_tramp_is_supported ())
        પાછા add_segment_exec_offset (data, seg);
      પાછા ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  નહીંતર
    પાછા data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
શૂન્ય
ffi_closure_free (શૂન્ય *ptr)
{
#જો FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  જો (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  જો (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

પૂર્ણાંક
ffi_tramp_is_present (શૂન્ય *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  પાછા seg != ખાલી && ffi_tramp_is_supported();
}

# નહીંતર /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#અન્તર્ભાવ <stdlib.h>

શૂન્ય *
ffi_closure_alloc (size_t size, શૂન્ય **code)
{
  શૂન્ય *c;

  જો (!code)
    પાછા ખાલી;

  c = malloc (size);
  *code = FFI_FN (c);
  પાછા c;
}

શૂન્ય
ffi_closure_free (શૂન્ય *ptr)
{
  મુક્ત (ptr);
}

શૂન્ય *
ffi_data_to_code_pointer (શૂન્ય *data)
{
  પાછા data;
}

પૂર્ણાંક
ffi_tramp_is_present (__attribute__((unused)) શૂન્ય *ptr)
{
  પાછા 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

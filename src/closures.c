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

#ଯଦି (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#ifndef __EMSCRIPTEN__

#ଅନ୍ତର୍ଭାଵ <fficonfig.h>
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>

#ifdef __NetBSD__
#ଅନ୍ତର୍ଭାଵ <sys/param.h>
#endif

#ଯଦି __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>

#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ifdef  HAVE_SYS_MEMFD_H
#ଅନ୍ତର୍ଭାଵ <sys/memfd.h>
#endif

ସ୍ଥିର ଅଚଳ size_t overhead =
  (ଆକାର(max_align_t) > ଆକାର(ଶୂନ୍ୟ *) + ଆକାର(size_t)) ?
    ଆକାର(max_align_t)
    : ଆକାର(ଶୂନ୍ୟ *) + ଆକାର(size_t);

#define ADD_TO_POINTER(p, d) ((ଶୂନ୍ୟ *)((uintptr_t)(p) + (d)))

ଶୂନ୍ୟ *
ffi_closure_alloc (size_t size, ଶୂନ୍ୟ **code)
{
  ସ୍ଥିର size_t page_size;
  size_t rounded_size;
  ଶୂନ୍ୟ *codeseg, *dataseg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  ଯଦି (!code)
    ଫେରନ୍ତୁ ଖାଲି;

  /* Obtain system page size. */
  ଯଦି (!page_size)
    page_size = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_size = (size + overhead + page_size - 1) & ~(page_size - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ଖାଲି, rounded_size, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  ଯଦି (dataseg == MAP_FAILED)
    ଫେରନ୍ତୁ ଖାଲି;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_size, ଖାଲି, rounded_size, MAP_REMAPDUP);
  ଯଦି (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_size);
    ଫେରନ୍ତୁ ଖାଲି;
  }
  ଯଦି (mprotect(codeseg, rounded_size, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_size);
    munmap(dataseg, rounded_size);
    ଫେରନ୍ତୁ ଖାଲି;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  memcpy(dataseg, &rounded_size, ଆକାର(rounded_size));
  memcpy(ADD_TO_POINTER(dataseg, ଆକାର(size_t)), &codeseg, ଆକାର(ଶୂନ୍ୟ *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  ଫେରନ୍ତୁ ADD_TO_POINTER(dataseg, overhead);
}

ଶୂନ୍ୟ
ffi_closure_free (ଶୂନ୍ୟ *ptr)
{
  ଶୂନ୍ୟ *codeseg, *dataseg;
  size_t rounded_size;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  memcpy(&rounded_size, dataseg, ଆକାର(rounded_size));
  memcpy(&codeseg, ADD_TO_POINTER(dataseg, ଆକାର(size_t)), ଆକାର(ଶୂନ୍ୟ *));
  munmap(dataseg, rounded_size);
  munmap(codeseg, rounded_size);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_is_present (__attribute__((unused)) ଶୂନ୍ୟ *ptr)
{
  ଫେରନ୍ତୁ 0;
}
#ଅନ୍ୟଥା /* !NetBSD with PROT_MPROTECT */

#ଯଦି !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# ଯଦି __linux__ && !defined(__ANDROID__)
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
# ଯଦି defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#endif

#ଯଦି FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# ଯଦି !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# ଯଦି !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#endif /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#ଯଦି FFI_CLOSURES

#ଯଦି FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__

#ଅନ୍ତର୍ଭାଵ <mach/mach.h>
#ଅନ୍ତର୍ଭାଵ <pthread.h>
#ifdef HAVE_ARM64E_PTRAUTH
#ଅନ୍ତର୍ଭାଵ <ptrauth.h>
#endif
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

ବାହ୍ୟ ଶୂନ୍ୟ *ffi_closure_trampoline_table_page;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ffi_trampoline_table ffi_trampoline_table;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା ffi_trampoline_table_entry ffi_trampoline_table_entry;

ସଂରଚନା ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_ଗଣନା;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *prev;
  ffi_trampoline_table *next;
};

ସଂରଚନା ffi_trampoline_table_entry
{
  ଶୂନ୍ୟ *(*trampoline) (ଶୂନ୍ୟ);
  ffi_trampoline_table_entry *next;
};

/* Total number of trampolines that fit in one trampoline table */
#define FFI_TRAMPOLINE_ଗଣନା (PAGE_MAX_SIZE / FFI_TRAMPOLINE_SIZE)

ସ୍ଥିର pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
ସ୍ଥିର ffi_trampoline_table *ffi_trampoline_tables = ଖାଲି;

ସ୍ଥିର ffi_trampoline_table *
ffi_trampoline_table_alloc (ଶୂନ୍ୟ)
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
  ଯଦି (kt != KERN_SUCCESS)
    ଫେରନ୍ତୁ ଖାଲି;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_SIZE;

#ifdef HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_data((ଶୂନ୍ୟ *)&ffi_closure_trampoline_table_page, ptrauth_ଚାବି_function_pointer, 0);
#ଅନ୍ୟଥା
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#endif

#ifdef __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#endif
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_SIZE, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  ଯଦି (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ଫେରନ୍ତୁ ଖାଲି;
    }

  ଯଦି (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_SIZE,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      ଯଦି (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
          ଫେରନ୍ତୁ ଖାଲି;
        }
    }

  /* We have valid trampoline and config pages */
  table = calloc (1, ଆକାର (ffi_trampoline_table));
  ଯଦି (table == ଖାଲି)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ଫେରନ୍ତୁ ଖାଲି;
    }
  table->free_ଗଣନା = FFI_TRAMPOLINE_ଗଣନା;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    calloc (FFI_TRAMPOLINE_ଗଣନା, ଆକାର (ffi_trampoline_table_entry));
  ଯଦି (table->free_list_pool == ଖାଲି)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_SIZE * 2);
      ମୁକ୍ତ (table);
      ଫେରନ୍ତୁ ଖାଲି;
    }

  ପାଇଁ (i = 0; i < table->free_ଗଣନା; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(ଶୂନ୍ୟ *) (trampoline_page + (i * FFI_TRAMPOLINE_SIZE));
#ifdef HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_ଚାବି_function_pointer, 0);
#endif

      ଯଦି (i < table->free_ଗଣନା - 1)
	entry->next = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  ଫେରନ୍ତୁ table;
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  ଯଦି (table->prev != ଖାଲି)
    table->prev->next = table->next;

  ଯଦି (table->next != ଖାଲି)
    table->next->prev = table->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_SIZE * 2);

  /* Deallocate free list */
  ମୁକ୍ତ (table->free_list_pool);
  ମୁକ୍ତ (table);
}

ଶୂନ୍ୟ *
ffi_closure_alloc (size_t size, ଶୂନ୍ୟ **code)
{
  /* Create the closure */
  ffi_closure *closure = malloc (size);
  ଯଦି (closure == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  ଯଦି (table == ଖାଲି || table->free_list == ଖାଲି)
    {
      table = ffi_trampoline_table_alloc ();
      ଯଦି (table == ଖାଲି)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  ମୁକ୍ତ (closure);
	  ଫେରନ୍ତୁ ଖାଲି;
	}

      /* Insert the new table at the top of the list */
      table->next = ffi_trampoline_tables;
      ଯଦି (table->next != ଖାଲି)
	table->next->prev = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->next;
  ffi_trampoline_tables->free_ଗଣନା--;
  entry->next = ଖାଲି;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  ଫେରନ୍ତୁ closure;
}

ଶୂନ୍ୟ
ffi_closure_free (ଶୂନ୍ୟ *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->next = table->free_list;
  table->free_list = entry;
  table->free_ଗଣନା++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  ଯଦି (table->free_ଗଣନା == FFI_TRAMPOLINE_ଗଣନା
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  ଅନ୍ୟଥା ଯଦି (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->prev = ଖାଲି;
      table->next = ffi_trampoline_tables;
      ଯଦି (ffi_trampoline_tables != ଖାଲି)
	ffi_trampoline_tables->prev = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  ମୁକ୍ତ (closure);
}

#endif

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#ନଚେତଯଦି FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

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

#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ifndef _MSC_VER
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#endif
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଯଦି !defined(_WIN32)
#ifdef HAVE_MNTENT
#ଅନ୍ତର୍ଭାଵ <mntent.h>
#endif /* HAVE_MNTENT */
#ଅନ୍ତର୍ଭାଵ <sys/param.h>
#ଅନ୍ତର୍ଭାଵ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#define LACKS_SYS_MMAN_H 1

#ଯଦି FFI_MMAP_EXEC_SELINUX
#ଅନ୍ତର୍ଭାଵ <sys/statfs.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା selinux_enabled = -1;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
selinux_enabled_check (ଶୂନ୍ୟ)
{
  ସଂରଚନା statfs sfs;
  FILE *f;
  ଅକ୍ଷର *buf = ଖାଲି;
  size_t len = 0;

  ଯଦି (statfs ("/selinux", &sfs) >= 0
      && (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) sfs.f_type == 0xf97cff8cU)
    ଫେରନ୍ତୁ 1;
  f = fopen ("/proc/mounts", "r");
  ଯଦି (f == ଖାଲି)
    ଫେରନ୍ତୁ 0;
  ଯେତେବେଳେ (getline (&buf, &len, f) >= 0)
    {
      ଅକ୍ଷର *p = strchr (buf, ' ');
      ଯଦି (p == ଖାଲି)
        ଭାଙ୍ଗ;
      p = strchr (p + 1, ' ');
      ଯଦି (p == ଖାଲି)
        ଭାଙ୍ଗ;
      ଯଦି (strncmp (p + 1, "selinuxfs ", 10) == 0)
        {
          ମୁକ୍ତ (buf);
          fclose (f);
          ଫେରନ୍ତୁ 1;
        }
    }
  ମୁକ୍ତ (buf);
  fclose (f);
  ଫେରନ୍ତୁ 0;
}

#define is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#ଅନ୍ୟଥା

#define is_selinux_enabled() 0

#endif /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#ଯଦି defined FFI_MMAP_PAX
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

ଗଣନା {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cached_pax_flags = -1;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
pax_flags_check (ଶୂନ୍ୟ)
{
  ଅକ୍ଷର *buf = ଖାଲି;
  size_t len = 0;
  FILE *f;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
  f = fopen ("/proc/self/status", "r");
  ଯଦି (f == ଖାଲି)
    ଫେରନ୍ତୁ 0;
  ret = 0;

  ଯେତେବେଳେ (getline (&buf, &len, f) != -1)
    ଯଦି (!strncmp (buf, "PaX:", 4))
      {
        ଯଦି (ଖାଲି != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        ଯଦି (ଖାଲି != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ଭାଙ୍ଗ;
      }
  ମୁକ୍ତ (buf);
  fclose (f);
  ଫେରନ୍ତୁ ret;
}

#define get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#define has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#define is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#define is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#endif /* defined FFI_MMAP_PAX */

#ନଚେତଯଦି defined (__CYGWIN__) || defined(__INTERIX)

#ଅନ୍ତର୍ଭାଵ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#define is_selinux_enabled() 0

#endif /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#ଯଦି !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#endif /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ସ୍ଥିର ଶୂନ୍ୟ *dlmalloc(size_t);
ସ୍ଥିର ଶୂନ୍ୟ dlfree(ଶୂନ୍ୟ*);
ସ୍ଥିର ଶୂନ୍ୟ *dlcalloc(size_t, size_t) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ *dlrealloc(ଶୂନ୍ୟ *, size_t) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ *dlmemalign(size_t, size_t) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ *dlvalloc(size_t) MAYBE_UNUSED;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dlmallopt(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା) MAYBE_UNUSED;
ସ୍ଥିର size_t dlmalloc_footprint(ଶୂନ୍ୟ) MAYBE_UNUSED;
ସ୍ଥିର size_t dlmalloc_max_footprint(ଶୂନ୍ୟ) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ** dlindependent_calloc(size_t, size_t, ଶୂନ୍ୟ**) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ** dlindependent_comalloc(size_t, size_t*, ଶୂନ୍ୟ**) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ *dlpvalloc(size_t) MAYBE_UNUSED;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dlmalloc_trim(size_t) MAYBE_UNUSED;
ସ୍ଥିର size_t dlmalloc_usable_size(ଶୂନ୍ୟ*) MAYBE_UNUSED;
ସ୍ଥିର ଶୂନ୍ୟ dlmalloc_stats(ଶୂନ୍ୟ) MAYBE_UNUSED;

#ଯଦି !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ସ୍ଥିର ଶୂନ୍ୟ *dlmmap(ଶୂନ୍ୟ *, size_t, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, off_t);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dlmunmap(ଶୂନ୍ୟ *, size_t);
#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#define mmap dlmmap
#define munmap dlmunmap

#ଅନ୍ତର୍ଭାଵ "dlmalloc.c"

#undef mmap
#undef munmap

#ଯଦି !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ସ୍ଥିର pthread_mutex_t open_temp_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ସ୍ଥିର size_t execsize = 0;

#ifdef HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_memfd (ଅଚଳ ଅକ୍ଷର *name)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  ଫେରନ୍ତୁ fd;
}
#endif

/* Open a temporary file name, and immediately unlink it.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_name (ଅକ୍ଷର *name, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags MAYBE_UNUSED)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

#ifdef HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#ଅନ୍ୟଥା
  fd = mkstemp (name);
#endif

  ଯଦି (fd != -1)
    unlink (name);

  ଫେରନ୍ତୁ fd;
}

/* Open a temporary file in the named directory.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_dir (ଅଚଳ ଅକ୍ଷର *dir)
{
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର suffix[] = "/ffiXXXXXX";
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା lendir, flags;
  ଅକ୍ଷର *tempname;
#ifdef O_TMPFILE
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
#endif

#ifdef O_CLOEXEC
  flags = O_CLOEXEC;
#ଅନ୍ୟଥା
  flags = 0;
#endif

#ifdef O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  ଯଦି (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    ଫେରନ୍ତୁ fd;
  } ଅନ୍ୟଥା {
    errno = 0;
  }
#endif

  lendir = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) strlen (dir);
  tempname = __builtin_alloca (lendir + ଆକାର (suffix));

  ଯଦି (!tempname)
    ଫେରନ୍ତୁ -1;

  memcpy (tempname, dir, lendir);
  memcpy (tempname + lendir, suffix, ଆକାର (suffix));

  ଫେରନ୍ତୁ open_temp_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_env (ଅଚଳ ଅକ୍ଷର *envvar)
{
  ଅଚଳ ଅକ୍ଷର *ମୂଲ୍ୟ = getenv (envvar);

  ଯଦି (!ମୂଲ୍ୟ)
    ଫେରନ୍ତୁ -1;

  ଫେରନ୍ତୁ open_temp_exec_file_dir (ମୂଲ୍ୟ);
}

#ifdef HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_mnt (ଅଚଳ ଅକ୍ଷର *mounts)
{
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *last_mounts;
  ସ୍ଥିର FILE *last_mntent;

  ଯଦି (mounts != last_mounts)
    {
      ଯଦି (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      ଯଦି (mounts)
	last_mntent = setmntent (mounts, "r");
      ଅନ୍ୟଥା
	last_mntent = ଖାଲି;
    }

  ଯଦି (!last_mntent)
    ଫେରନ୍ତୁ -1;

  ପାଇଁ (;;)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
      ସଂରଚନା mntent mnt;
      ଅକ୍ଷର buf[MAXPATHLEN * 3];

      ଯଦି (getmntent_r (last_mntent, &mnt, buf, ଆକାର (buf)) == ଖାଲି)
	ଫେରନ୍ତୁ -1;

      ଯଦି (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	ଜାରି;

      fd = open_temp_exec_file_dir (mnt.mnt_dir);

      ଯଦି (fd != -1)
	ଫେରନ୍ତୁ fd;
    }
}
#endif /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ସ୍ଥିର ସଂରଚନା
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*func)(ଅଚଳ ଅକ୍ଷର *);
  ଅଚଳ ଅକ୍ଷର *arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା repeat;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_temp_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file_opts_next (ଶୂନ୍ୟ)
{
  ଯଦି (open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func (ଖାଲି);

  open_temp_exec_file_opts_idx++;
  ଯଦି (open_temp_exec_file_opts_idx
      == (ଆକାର (open_temp_exec_file_opts)
	  / ଆକାର (*open_temp_exec_file_opts)))
    {
      open_temp_exec_file_opts_idx = 0;
      ଫେରନ୍ତୁ 1;
    }

  ଫେରନ୍ତୁ 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
open_temp_exec_file (ଶୂନ୍ୟ)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

  କର
    {
      fd = open_temp_exec_file_opts[open_temp_exec_file_opts_idx].func
	(open_temp_exec_file_opts[open_temp_exec_file_opts_idx].arg);

      ଯଦି (!open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  ଯଦି (open_temp_exec_file_opts_next ())
	    ଭାଙ୍ଗ;
	}
    }
  ଯେତେବେଳେ (fd == -1);

  ଫେରନ୍ତୁ fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
allocate_space (ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t len)
{
  ସ୍ଥିର long page_size;

  /* Obtain system page size. */
  ଯଦି (!page_size)
    page_size = sysconf(_SC_PAGESIZE);
  ଯଦି (page_size <= 0)
    ଫେରନ୍ତୁ -1;

  unsigned ଅକ୍ଷର buf[page_size];
  memset (buf, 0, page_size);

  ଯେତେବେଳେ (len > 0)
    {
      off_t to_write = (len < page_size) ? len : page_size;
      ଯଦି (ଲିଖ (fd, buf, to_write) < to_write)
        ଫେରନ୍ତୁ -1;
      len -= to_write;
    }

  ଫେରନ୍ତୁ 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ସ୍ଥିର ଶୂନ୍ୟ *
dlmmap_locked (ଶୂନ୍ୟ *start, size_t length, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prot, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, off_t offset)
{
  ଶୂନ୍ୟ *ptr;

  ଯଦି (execfd == -1)
    {
      open_temp_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_temp_exec_file ();
      ଯଦି (execfd == -1)
	ଫେରନ୍ତୁ MFAIL;
    }

  offset = execsize;

  ଯଦି (allocate_space (execfd, length))
    ଫେରନ୍ତୁ MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ଖାଲି, length, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  ଯଦି (ptr == MFAIL)
    {
      ଯଦି (!offset)
	{
	  close (execfd);
	  ଯାଅ retry_open;
	}
      ଯଦି (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      ଫେରନ୍ତୁ MFAIL;
    }
  ଅନ୍ୟଥା ଯଦି (!offset
	   && open_temp_exec_file_opts[open_temp_exec_file_opts_idx].repeat)
    open_temp_exec_file_opts_next ();

  start = mmap (start, length, prot, flags, execfd, offset);

  ଯଦି (start == MFAIL)
    {
      munmap (ptr, length);
      ଯଦି (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      ଫେରନ୍ତୁ start;
    }

  mmap_exec_offset ((ଅକ୍ଷର *)start, length) = (ଅକ୍ଷର*)ptr - (ଅକ୍ଷର*)start;

  execsize += length;

  ଫେରନ୍ତୁ start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ସ୍ଥିର ଶୂନ୍ୟ *
dlmmap (ଶୂନ୍ୟ *start, size_t length, ପୂର୍ଣ୍ଣସଂଖ୍ୟା prot,
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, off_t offset)
{
  ଶୂନ୍ୟ *ptr;

  ପ୍ରତିଜ୍ଞା (start == ଖାଲି && length % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  ଯଦି (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
      ଫେରନ୍ତୁ ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  ଯଦି (execfd == -1 && is_mprotect_enabled ())
    {
#ifdef FFI_MMAP_EXEC_EMUTRAMP_PAX
      ଯଦି (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, length, prot & ~PROT_EXEC, flags, fd, offset);
          ଫେରନ୍ତୁ ptr;
        }
#endif
      /* fallback to dlmmap_locked.  */
    }
  ଅନ୍ୟଥା ଯଦି (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, length, prot | PROT_EXEC, flags, fd, offset);

      ଯଦି (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	ଫେରନ୍ତୁ ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_temp_exec_file_mutex);
  ptr = dlmmap_locked (start, length, prot, flags, offset);
  pthread_mutex_unlock (&open_temp_exec_file_mutex);

  ଫେରନ୍ତୁ ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
dlmunmap (ଶୂନ୍ୟ *start, size_t length)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  ଶୂନ୍ୟ *code;

  ଯଦି (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = munmap (code, length);
      ଯଦି (ret)
	ଫେରନ୍ତୁ ret;
    }

  ଫେରନ୍ତୁ munmap (start, length);
}

#ଯଦି FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
ସ୍ଥିର msegmentptr
segment_holding_code (mstate m, ଅକ୍ଷର* addr)
{
  msegmentptr sp = &m->seg;
  ପାଇଁ (;;) {
    ଯଦି (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->size)
      ଫେରନ୍ତୁ sp;
    ଯଦି ((sp = sp->next) == 0)
      ଫେରନ୍ତୁ 0;
  }
}
#endif

#endif /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
ଶୂନ୍ୟ *
ffi_closure_alloc (size_t size, ଶୂନ୍ୟ **code)
{
  ଶୂନ୍ୟ *ptr, *ftramp;

  ଯଦି (!code)
    ଫେରନ୍ତୁ ଖାଲି;

  ptr = dlmalloc (size);

  ଯଦି (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      ଯଦି (!ffi_tramp_is_supported ())
        ଫେରନ୍ତୁ ptr;

      ftramp = ffi_tramp_alloc (0);
      ଯଦି (ftramp == ଖାଲି)
      {
        dlfree (ptr);
        ଫେରନ୍ତୁ ଖାଲି;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  ଫେରନ୍ତୁ ptr;
}

ଶୂନ୍ୟ *
ffi_data_to_code_pointer (ଶୂନ୍ୟ *data)
{
  msegmentptr seg = segment_holding (gm, data);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  ଯଦି (seg)
    {
      ଯଦି (!ffi_tramp_is_supported ())
        ଫେରନ୍ତୁ add_segment_exec_offset (data, seg);
      ଫେରନ୍ତୁ ffi_tramp_get_addr (((ffi_closure *) data)->ftramp);
    }
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ data;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
ଶୂନ୍ୟ
ffi_closure_free (ଶୂନ୍ୟ *ptr)
{
#ଯଦି FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  ଯଦି (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#endif
  ଯଦି (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_is_present (ଶୂନ୍ୟ *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  ଫେରନ୍ତୁ seg != ଖାଲି && ffi_tramp_is_supported();
}

# ଅନ୍ୟଥା /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

ଶୂନ୍ୟ *
ffi_closure_alloc (size_t size, ଶୂନ୍ୟ **code)
{
  ଶୂନ୍ୟ *c;

  ଯଦି (!code)
    ଫେରନ୍ତୁ ଖାଲି;

  c = malloc (size);
  *code = FFI_FN (c);
  ଫେରନ୍ତୁ c;
}

ଶୂନ୍ୟ
ffi_closure_free (ଶୂନ୍ୟ *ptr)
{
  ମୁକ୍ତ (ptr);
}

ଶୂନ୍ୟ *
ffi_data_to_code_pointer (ଶୂନ୍ୟ *data)
{
  ଫେରନ୍ତୁ data;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_tramp_is_present (__attribute__((unused)) ଶୂନ୍ୟ *ptr)
{
  ଫେରନ୍ତୁ 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#endif /* FFI_CLOSURES */

#endif /* NetBSD with PROT_MPROTECT */
#endif /* __EMSCRIPTEN__ */

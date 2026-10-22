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

#अगर (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#घोषणा _GNU_SOURCE 1
#पूर्ण_अगर

#अगर_अघोषित __EMSCRIPTEN__

#समावेश <fficonfig.h>
#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <tramp.h>

#अगर_घोषित __NetBSD__
#समावेश <sys/param.h>
#पूर्ण_अगर

#अगर __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#समावेश <sys/mman.h>

#समावेश <stddef.h>
#समावेश <unistd.h>
#अगर_घोषित  HAVE_SYS_MEMFD_H
#समावेश <sys/memfd.h>
#पूर्ण_अगर

अटल स्थिर माप_प्रकार overhead =
  (माप(max_align_t) > माप(व्योम *) + माप(माप_प्रकार)) ?
    माप(max_align_t)
    : माप(व्योम *) + माप(माप_प्रकार);

#घोषणा ADD_TO_POINTER(p, d) ((व्योम *)((uintptr_t)(p) + (d)))

व्योम *
ffi_closure_alloc (माप_प्रकार आकार, व्योम **code)
{
  अटल माप_प्रकार page_आकार;
  माप_प्रकार rounded_आकार;
  व्योम *codeseg, *dataseg;
  पूर्णांक prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  अगर (!code)
    वापस शून्य;

  /* Obtain system page size. */
  अगर (!page_आकार)
    page_आकार = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_आकार = (आकार + overhead + page_आकार - 1) & ~(page_आकार - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(शून्य, rounded_आकार, prot, MAP_ANON | MAP_PRIVATE, -1, 0);
  अगर (dataseg == MAP_FAILED)
    वापस शून्य;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_आकार, शून्य, rounded_आकार, MAP_REMAPDUP);
  अगर (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_आकार);
    वापस शून्य;
  }
  अगर (mprotect(codeseg, rounded_आकार, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_आकार);
    munmap(dataseg, rounded_आकार);
    वापस शून्य;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  स_नकल(dataseg, &rounded_आकार, माप(rounded_आकार));
  स_नकल(ADD_TO_POINTER(dataseg, माप(माप_प्रकार)), &codeseg, माप(व्योम *));
  *code = ADD_TO_POINTER(codeseg, overhead);
  वापस ADD_TO_POINTER(dataseg, overhead);
}

व्योम
ffi_closure_free (व्योम *ptr)
{
  व्योम *codeseg, *dataseg;
  माप_प्रकार rounded_आकार;

  dataseg = ADD_TO_POINTER(ptr, -overhead);
  स_नकल(&rounded_आकार, dataseg, माप(rounded_आकार));
  स_नकल(&codeseg, ADD_TO_POINTER(dataseg, माप(माप_प्रकार)), माप(व्योम *));
  munmap(dataseg, rounded_आकार);
  munmap(codeseg, rounded_आकार);
}

पूर्णांक
ffi_tramp_is_present (__attribute__((unused)) व्योम *ptr)
{
  वापस 0;
}
#अन्यथा /* !NetBSD with PROT_MPROTECT */

#अगर !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_TABLE
# अगर __linux__ && !defined(__ANDROID__)
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
# अगर defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#पूर्ण_अगर

#अगर FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# अगर !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# अगर !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#पूर्ण_अगर /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#अगर FFI_CLOSURES

#अगर FFI_EXEC_TRAMPOLINE_TABLE

#अगर_घोषित __MACH__

#समावेश <mach/mach.h>
#समावेश <pthread.h>
#अगर_घोषित HAVE_ARM64E_PTRAUTH
#समावेश <ptrauth.h>
#पूर्ण_अगर
#समावेश <stdio.h>
#समावेश <stdlib.h>

बाह्य व्योम *ffi_closure_trampoline_table_page;

प्रकार काष्ठा ffi_trampoline_table ffi_trampoline_table;
प्रकार काष्ठा ffi_trampoline_table_entry ffi_trampoline_table_entry;

काष्ठा ffi_trampoline_table
{
  /* contiguous writable and executable pages */
  vm_address_t config_page;

  /* free list tracking */
  uint16_t free_गिनती;
  ffi_trampoline_table_entry *free_list;
  ffi_trampoline_table_entry *free_list_pool;

  ffi_trampoline_table *पूर्व;
  ffi_trampoline_table *अग्र;
};

काष्ठा ffi_trampoline_table_entry
{
  व्योम *(*trampoline) (व्योम);
  ffi_trampoline_table_entry *अग्र;
};

/* Total number of trampolines that fit in one trampoline table */
#घोषणा FFI_TRAMPOLINE_गिनती (PAGE_MAX_आकार / FFI_TRAMPOLINE_आकार)

अटल pthread_mutex_t ffi_trampoline_lock = PTHREAD_MUTEX_INITIALIZER;
अटल ffi_trampoline_table *ffi_trampoline_tables = शून्य;

अटल ffi_trampoline_table *
ffi_trampoline_table_alloc (व्योम)
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
  kt = vm_allocate (mach_task_self (), &config_page, PAGE_MAX_आकार * 2,
		    VM_FLAGS_ANYWHERE);
  अगर (kt != KERN_SUCCESS)
    वापस शून्य;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_आकार;

#अगर_घोषित HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_address_t)(uintptr_t)ptrauth_auth_तथ्य((व्योम *)&ffi_closure_trampoline_table_page, ptrauth_कुंजी_function_pointer, 0);
#अन्यथा
  trampoline_page_template = (vm_address_t)&ffi_closure_trampoline_table_page;
#पूर्ण_अगर

#अगर_घोषित __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#पूर्ण_अगर
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_आकार, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  अगर (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      वापस शून्य;
    }

  अगर (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_आकार,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      अगर (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
          वापस शून्य;
        }
    }

  /* We have valid trampoline and config pages */
  table = सुस्मृति (1, माप (ffi_trampoline_table));
  अगर (table == शून्य)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      वापस शून्य;
    }
  table->free_गिनती = FFI_TRAMPOLINE_गिनती;
  table->config_page = config_page;

  /* Create and initialize the free list */
  table->free_list_pool =
    सुस्मृति (FFI_TRAMPOLINE_गिनती, माप (ffi_trampoline_table_entry));
  अगर (table->free_list_pool == शून्य)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      मुक्त (table);
      वापस शून्य;
    }

  क्रम (i = 0; i < table->free_गिनती; i++)
    {
      ffi_trampoline_table_entry *entry = &table->free_list_pool[i];
      entry->trampoline =
	(व्योम *) (trampoline_page + (i * FFI_TRAMPOLINE_आकार));
#अगर_घोषित HAVE_ARM64E_PTRAUTH
      entry->trampoline = ptrauth_sign_unauthenticated(entry->trampoline, ptrauth_कुंजी_function_pointer, 0);
#पूर्ण_अगर

      अगर (i < table->free_गिनती - 1)
	entry->अग्र = &table->free_list_pool[i + 1];
    }

  table->free_list = table->free_list_pool;

  वापस table;
}

अटल व्योम
ffi_trampoline_table_free (ffi_trampoline_table *table)
{
  /* Remove from the list */
  अगर (table->पूर्व != शून्य)
    table->पूर्व->अग्र = table->अग्र;

  अगर (table->अग्र != शून्य)
    table->अग्र->पूर्व = table->पूर्व;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), table->config_page, PAGE_MAX_आकार * 2);

  /* Deallocate free list */
  मुक्त (table->free_list_pool);
  मुक्त (table);
}

व्योम *
ffi_closure_alloc (माप_प्रकार आकार, व्योम **code)
{
  /* Create the closure */
  ffi_closure *closure = दो_स्मृति (आकार);
  अगर (closure == शून्य)
    वापस शून्य;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_table *table = ffi_trampoline_tables;
  अगर (table == शून्य || table->free_list == शून्य)
    {
      table = ffi_trampoline_table_alloc ();
      अगर (table == शून्य)
	{
	  pthread_mutex_unlock (&ffi_trampoline_lock);
	  मुक्त (closure);
	  वापस शून्य;
	}

      /* Insert the new table at the top of the list */
      table->अग्र = ffi_trampoline_tables;
      अगर (table->अग्र != शून्य)
	table->अग्र->पूर्व = table;

      ffi_trampoline_tables = table;
    }

  /* Claim the free entry */
  ffi_trampoline_table_entry *entry = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = entry->अग्र;
  ffi_trampoline_tables->free_गिनती--;
  entry->अग्र = शून्य;

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Initialize the return values */
  *code = entry->trampoline;
  closure->trampoline_table = table;
  closure->trampoline_table_entry = entry;

  वापस closure;
}

व्योम
ffi_closure_free (व्योम *ptr)
{
  ffi_closure *closure = ptr;

  pthread_mutex_lock (&ffi_trampoline_lock);

  /* Fetch the table and entry references */
  ffi_trampoline_table *table = closure->trampoline_table;
  ffi_trampoline_table_entry *entry = closure->trampoline_table_entry;

  /* Return the entry to the free list */
  entry->अग्र = table->free_list;
  table->free_list = entry;
  table->free_गिनती++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  अगर (table->free_गिनती == FFI_TRAMPOLINE_गिनती
      && ffi_trampoline_tables != table)
    {
      ffi_trampoline_table_free (table);
    }
  अन्यथा अगर (ffi_trampoline_tables != table)
    {
      /* Otherwise, bump this table to the top of the list */
      table->पूर्व = शून्य;
      table->अग्र = ffi_trampoline_tables;
      अगर (ffi_trampoline_tables != शून्य)
	ffi_trampoline_tables->पूर्व = table;

      ffi_trampoline_tables = table;
    }

  pthread_mutex_unlock (&ffi_trampoline_lock);

  /* Free the closure */
  मुक्त (closure);
}

#पूर्ण_अगर

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#या_अगर FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#घोषणा USE_LOCKS 1
#घोषणा USE_DL_PREFIX 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#घोषणा HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#घोषणा HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#घोषणा NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#घोषणा DEFAULT_MMAP_THRESHOLD MAX_आकार_T

/* Don't allocate more than a page unless needed.  */
#घोषणा DEFAULT_GRANULARITY ((माप_प्रकार)malloc_getpagesize)

#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <fcntl.h>
#समावेश <errno.h>
#अगर_अघोषित _MSC_VER
#समावेश <unistd.h>
#पूर्ण_अगर
#समावेश <string.h>
#समावेश <stdio.h>
#अगर !defined(_WIN32)
#अगर_घोषित HAVE_MNTENT
#समावेश <mntent.h>
#पूर्ण_अगर /* HAVE_MNTENT */
#समावेश <sys/param.h>
#समावेश <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#समावेश <sys/mman.h>
#घोषणा LACKS_SYS_MMAN_H 1

#अगर FFI_MMAP_EXEC_SELINUX
#समावेश <sys/statfs.h>
#समावेश <stdlib.h>

अटल पूर्णांक selinux_enabled = -1;

अटल पूर्णांक
selinux_enabled_check (व्योम)
{
  काष्ठा statfs sfs;
  खाता *f;
  अक्षर *buf = शून्य;
  माप_प्रकार len = 0;

  अगर (statfs ("/selinux", &sfs) >= 0
      && (अचिन्हित पूर्णांक) sfs.f_type == 0xf97cff8cU)
    वापस 1;
  f = ख_खोलो ("/proc/mounts", "r");
  अगर (f == शून्य)
    वापस 0;
  जबतक (getline (&buf, &len, f) >= 0)
    {
      अक्षर *p = म_अक्षर (buf, ' ');
      अगर (p == शून्य)
        अवरोध;
      p = म_अक्षर (p + 1, ' ');
      अगर (p == शून्य)
        अवरोध;
      अगर (म_भेदन (p + 1, "selinuxfs ", 10) == 0)
        {
          मुक्त (buf);
          ख_बंद (f);
          वापस 1;
        }
    }
  मुक्त (buf);
  ख_बंद (f);
  वापस 0;
}

#घोषणा is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#अन्यथा

#घोषणा is_selinux_enabled() 0

#पूर्ण_अगर /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#अगर defined FFI_MMAP_PAX
#समावेश <stdlib.h>

क्रमागत {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
अटल पूर्णांक cached_pax_flags = -1;

अटल पूर्णांक
pax_flags_check (व्योम)
{
  अक्षर *buf = शून्य;
  माप_प्रकार len = 0;
  खाता *f;
  पूर्णांक ret;
  f = ख_खोलो ("/proc/self/status", "r");
  अगर (f == शून्य)
    वापस 0;
  ret = 0;

  जबतक (getline (&buf, &len, f) != -1)
    अगर (!म_भेदन (buf, "PaX:", 4))
      {
        अगर (शून्य != म_अक्षर (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        अगर (शून्य != म_अक्षर (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        अवरोध;
      }
  मुक्त (buf);
  ख_बंद (f);
  वापस ret;
}

#घोषणा get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#घोषणा has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#घोषणा is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#घोषणा is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#पूर्ण_अगर /* defined FFI_MMAP_PAX */

#या_अगर defined (__CYGWIN__) || defined(__INTERIX)

#समावेश <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#घोषणा is_selinux_enabled() 0

#पूर्ण_अगर /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#अगर !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#पूर्ण_अगर /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
अटल व्योम *dlmalloc(माप_प्रकार);
अटल व्योम dlfree(व्योम*);
अटल व्योम *dlcalloc(माप_प्रकार, माप_प्रकार) MAYBE_UNUSED;
अटल व्योम *dlrealloc(व्योम *, माप_प्रकार) MAYBE_UNUSED;
अटल व्योम *dlmemalign(माप_प्रकार, माप_प्रकार) MAYBE_UNUSED;
अटल व्योम *dlvalloc(माप_प्रकार) MAYBE_UNUSED;
अटल पूर्णांक dlmallopt(पूर्णांक, पूर्णांक) MAYBE_UNUSED;
अटल माप_प्रकार dlmalloc_footprint(व्योम) MAYBE_UNUSED;
अटल माप_प्रकार dlmalloc_max_footprint(व्योम) MAYBE_UNUSED;
अटल व्योम** dlindependent_calloc(माप_प्रकार, माप_प्रकार, व्योम**) MAYBE_UNUSED;
अटल व्योम** dlindependent_comalloc(माप_प्रकार, माप_प्रकार*, व्योम**) MAYBE_UNUSED;
अटल व्योम *dlpvalloc(माप_प्रकार) MAYBE_UNUSED;
अटल पूर्णांक dlmalloc_trim(माप_प्रकार) MAYBE_UNUSED;
अटल माप_प्रकार dlmalloc_usable_आकार(व्योम*) MAYBE_UNUSED;
अटल व्योम dlmalloc_stats(व्योम) MAYBE_UNUSED;

#अगर !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
अटल व्योम *dlmmap(व्योम *, माप_प्रकार, पूर्णांक, पूर्णांक, पूर्णांक, off_t);
अटल पूर्णांक dlmunmap(व्योम *, माप_प्रकार);
#पूर्ण_अगर /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#घोषणा mmap dlmmap
#घोषणा munmap dlmunmap

#समावेश "dlmalloc.c"

#अघोषित mmap
#अघोषित munmap

#अगर !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
अटल pthread_mutex_t open_अस्थायी_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
अटल पूर्णांक execfd = -1;

/* The amount of space already allocated from the temporary file.  */
अटल माप_प्रकार execsize = 0;

#अगर_घोषित HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
अटल पूर्णांक
open_अस्थायी_exec_file_memfd (स्थिर अक्षर *name)
{
  पूर्णांक fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  वापस fd;
}
#पूर्ण_अगर

/* Open a temporary file name, and immediately unlink it.  */
अटल पूर्णांक
open_अस्थायी_exec_file_name (अक्षर *name, पूर्णांक flags MAYBE_UNUSED)
{
  पूर्णांक fd;

#अगर_घोषित HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#अन्यथा
  fd = mkstemp (name);
#पूर्ण_अगर

  अगर (fd != -1)
    unlink (name);

  वापस fd;
}

/* Open a temporary file in the named directory.  */
अटल पूर्णांक
open_अस्थायी_exec_file_dir (स्थिर अक्षर *dir)
{
  अटल स्थिर अक्षर suffix[] = "/ffiXXXXXX";
  पूर्णांक lendir, flags;
  अक्षर *tempname;
#अगर_घोषित O_TMPFILE
  पूर्णांक fd;
#पूर्ण_अगर

#अगर_घोषित O_CLOEXEC
  flags = O_CLOEXEC;
#अन्यथा
  flags = 0;
#पूर्ण_अगर

#अगर_घोषित O_TMPFILE
  fd = खोलो (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  अगर (fd != -1 || (त्रुटि_सं != EINVAL && त्रुटि_सं != EISDIR && त्रुटि_सं != EOPNOTSUPP)) {
    वापस fd;
  } अन्यथा {
    त्रुटि_सं = 0;
  }
#पूर्ण_अगर

  lendir = (पूर्णांक) म_माप (dir);
  tempname = __builtin_alloca (lendir + माप (suffix));

  अगर (!tempname)
    वापस -1;

  स_नकल (tempname, dir, lendir);
  स_नकल (tempname + lendir, suffix, माप (suffix));

  वापस open_अस्थायी_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
अटल पूर्णांक
open_अस्थायी_exec_file_env (स्थिर अक्षर *envvar)
{
  स्थिर अक्षर *मूल्य = दो_पर्या (envvar);

  अगर (!मूल्य)
    वापस -1;

  वापस open_अस्थायी_exec_file_dir (मूल्य);
}

#अगर_घोषित HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
अटल पूर्णांक
open_अस्थायी_exec_file_mnt (स्थिर अक्षर *mounts)
{
  अटल स्थिर अक्षर *last_mounts;
  अटल खाता *last_mntent;

  अगर (mounts != last_mounts)
    {
      अगर (last_mntent)
	endmntent (last_mntent);

      last_mounts = mounts;

      अगर (mounts)
	last_mntent = setmntent (mounts, "r");
      अन्यथा
	last_mntent = शून्य;
    }

  अगर (!last_mntent)
    वापस -1;

  क्रम (;;)
    {
      पूर्णांक fd;
      काष्ठा mntent mnt;
      अक्षर buf[MAXPATHLEN * 3];

      अगर (getmntent_r (last_mntent, &mnt, buf, माप (buf)) == शून्य)
	वापस -1;

      अगर (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	जारी;

      fd = open_अस्थायी_exec_file_dir (mnt.mnt_dir);

      अगर (fd != -1)
	वापस fd;
    }
}
#पूर्ण_अगर /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
अटल काष्ठा
{
  पूर्णांक (*func)(स्थिर अक्षर *);
  स्थिर अक्षर *arg;
  पूर्णांक repeat;
} open_अस्थायी_exec_file_opts[] = {
#अगर_घोषित HAVE_MEMFD_CREATE
  { open_अस्थायी_exec_file_memfd, "libffi", 0 },
#पूर्ण_अगर
  { open_अस्थायी_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_अस्थायी_exec_file_env, "TMPDIR", 0 },
  { open_अस्थायी_exec_file_dir, "/tmp", 0 },
  { open_अस्थायी_exec_file_dir, "/var/tmp", 0 },
  { open_अस्थायी_exec_file_dir, "/dev/shm", 0 },
  { open_अस्थायी_exec_file_env, "HOME", 0 },
#अगर_घोषित HAVE_MNTENT
  { open_अस्थायी_exec_file_mnt, "/etc/mtab", 1 },
  { open_अस्थायी_exec_file_mnt, "/proc/mounts", 1 },
#पूर्ण_अगर /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
अटल पूर्णांक open_अस्थायी_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
अटल पूर्णांक
open_अस्थायी_exec_file_opts_अग्र (व्योम)
{
  अगर (open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat)
    open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].func (शून्य);

  open_अस्थायी_exec_file_opts_idx++;
  अगर (open_अस्थायी_exec_file_opts_idx
      == (माप (open_अस्थायी_exec_file_opts)
	  / माप (*open_अस्थायी_exec_file_opts)))
    {
      open_अस्थायी_exec_file_opts_idx = 0;
      वापस 1;
    }

  वापस 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
पूर्णांक
open_अस्थायी_exec_file (व्योम)
{
  पूर्णांक fd;

  करो
    {
      fd = open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].func
	(open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].arg);

      अगर (!open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  अगर (open_अस्थायी_exec_file_opts_अग्र ())
	    अवरोध;
	}
    }
  जबतक (fd == -1);

  वापस fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
अटल पूर्णांक
allocate_space (पूर्णांक fd, off_t len)
{
  अटल दीर्घ page_आकार;

  /* Obtain system page size. */
  अगर (!page_आकार)
    page_आकार = sysconf(_SC_PAGESIZE);
  अगर (page_आकार <= 0)
    वापस -1;

  अचिन्हित अक्षर buf[page_आकार];
  स_रखो (buf, 0, page_आकार);

  जबतक (len > 0)
    {
      off_t to_write = (len < page_आकार) ? len : page_आकार;
      अगर (ग_लिखो (fd, buf, to_write) < to_write)
        वापस -1;
      len -= to_write;
    }

  वापस 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
अटल व्योम *
dlmmap_locked (व्योम *start, माप_प्रकार लंबाई, पूर्णांक prot, पूर्णांक flags, off_t offset)
{
  व्योम *ptr;

  अगर (execfd == -1)
    {
      open_अस्थायी_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_अस्थायी_exec_file ();
      अगर (execfd == -1)
	वापस MFAIL;
    }

  offset = execsize;

  अगर (allocate_space (execfd, लंबाई))
    वापस MFAIL;

  flags &= ~(MAP_PRIVATE | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (शून्य, लंबाई, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  अगर (ptr == MFAIL)
    {
      अगर (!offset)
	{
	  बंद (execfd);
	  जाओ retry_open;
	}
      अगर (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      वापस MFAIL;
    }
  अन्यथा अगर (!offset
	   && open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat)
    open_अस्थायी_exec_file_opts_अग्र ();

  start = mmap (start, लंबाई, prot, flags, execfd, offset);

  अगर (start == MFAIL)
    {
      munmap (ptr, लंबाई);
      अगर (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      वापस start;
    }

  mmap_exec_offset ((अक्षर *)start, लंबाई) = (अक्षर*)ptr - (अक्षर*)start;

  execsize += लंबाई;

  वापस start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
अटल व्योम *
dlmmap (व्योम *start, माप_प्रकार लंबाई, पूर्णांक prot,
	पूर्णांक flags, पूर्णांक fd, off_t offset)
{
  व्योम *ptr;

  निश्चित (start == शून्य && लंबाई % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_PRIVATE | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  अगर (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, लंबाई, prot & ~PROT_EXEC, flags, fd, offset);
      वापस ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  अगर (execfd == -1 && is_mprotect_enabled ())
    {
#अगर_घोषित FFI_MMAP_EXEC_EMUTRAMP_PAX
      अगर (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, लंबाई, prot & ~PROT_EXEC, flags, fd, offset);
          वापस ptr;
        }
#पूर्ण_अगर
      /* fallback to dlmmap_locked.  */
    }
  अन्यथा अगर (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, लंबाई, prot | PROT_EXEC, flags, fd, offset);

      अगर (ptr != MFAIL || (त्रुटि_सं != EPERM && त्रुटि_सं != EACCES))
	/* Cool, no need to mess with separate segments.  */
	वापस ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_lock (&open_अस्थायी_exec_file_mutex);
  ptr = dlmmap_locked (start, लंबाई, prot, flags, offset);
  pthread_mutex_unlock (&open_अस्थायी_exec_file_mutex);

  वापस ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
अटल पूर्णांक
dlmunmap (व्योम *start, माप_प्रकार लंबाई)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  व्योम *code;

  अगर (seg && (code = add_segment_exec_offset (start, seg)) != start)
    {
      पूर्णांक ret = munmap (code, लंबाई);
      अगर (ret)
	वापस ret;
    }

  वापस munmap (start, लंबाई);
}

#अगर FFI_CLOSURE_FREE_CODE
/* Return segment holding given code address.  */
अटल msegmentptr
segment_holding_code (mstate m, अक्षर* addr)
{
  msegmentptr sp = &m->seg;
  क्रम (;;) {
    अगर (addr >= add_segment_exec_offset (sp->base, sp)
	&& addr < add_segment_exec_offset (sp->base, sp) + sp->आकार)
      वापस sp;
    अगर ((sp = sp->अग्र) == 0)
      वापस 0;
  }
}
#पूर्ण_अगर

#पूर्ण_अगर /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
व्योम *
ffi_closure_alloc (माप_प्रकार आकार, व्योम **code)
{
  व्योम *ptr, *ftramp;

  अगर (!code)
    वापस शून्य;

  ptr = dlmalloc (आकार);

  अगर (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *code = FFI_FN (add_segment_exec_offset (ptr, seg));
      अगर (!ffi_tramp_is_supported ())
        वापस ptr;

      ftramp = ffi_tramp_alloc (0);
      अगर (ftramp == शून्य)
      {
        dlfree (ptr);
        वापस शून्य;
      }
      *code = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_closure *) ptr)->ftramp = ftramp;
    }

  वापस ptr;
}

व्योम *
ffi_तथ्य_to_code_pointer (व्योम *तथ्य)
{
  msegmentptr seg = segment_holding (gm, तथ्य);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  अगर (seg)
    {
      अगर (!ffi_tramp_is_supported ())
        वापस add_segment_exec_offset (तथ्य, seg);
      वापस ffi_tramp_get_addr (((ffi_closure *) तथ्य)->ftramp);
    }
  अन्यथा
    वापस तथ्य;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
व्योम
ffi_closure_free (व्योम *ptr)
{
#अगर FFI_CLOSURE_FREE_CODE
  msegmentptr seg = segment_holding_code (gm, ptr);

  अगर (seg)
    ptr = sub_segment_exec_offset (ptr, seg);
#पूर्ण_अगर
  अगर (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_closure *) ptr)->ftramp);

  dlfree (ptr);
}

पूर्णांक
ffi_tramp_is_present (व्योम *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  वापस seg != शून्य && ffi_tramp_is_supported();
}

# अन्यथा /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#समावेश <stdlib.h>

व्योम *
ffi_closure_alloc (माप_प्रकार आकार, व्योम **code)
{
  व्योम *c;

  अगर (!code)
    वापस शून्य;

  c = दो_स्मृति (आकार);
  *code = FFI_FN (c);
  वापस c;
}

व्योम
ffi_closure_free (व्योम *ptr)
{
  मुक्त (ptr);
}

व्योम *
ffi_तथ्य_to_code_pointer (व्योम *तथ्य)
{
  वापस तथ्य;
}

पूर्णांक
ffi_tramp_is_present (__attribute__((unused)) व्योम *ptr)
{
  वापस 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#पूर्ण_अगर /* FFI_CLOSURES */

#पूर्ण_अगर /* NetBSD with PROT_MPROTECT */
#पूर्ण_अगर /* __EMSCRIPTEN__ */

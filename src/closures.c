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

#यदि (defined __linux__ || defined __CYGWIN__) && !defined _GNU_SOURCE
#परिभाषय _GNU_SOURCE 1
#यदि_अन्तः

#यदि_अपरिभाषितम् __EMSCRIPTEN__

#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <tramp.h>

#यदि_परिभाषितम् __NetBSD__
#अन्तर्भाव <sys/param.h>
#यदि_अन्तः

#यदि __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#अन्तर्भाव <sys/mman.h>

#अन्तर्भाव <stddef.h>
#अन्तर्भाव <unistd.h>
#यदि_परिभाषितम्  HAVE_SYS_MEMFD_H
#अन्तर्भाव <sys/memfd.h>
#यदि_अन्तः

स्थिर ध्रुव size_t overhead =
  (परिमाण(max_align_t) > परिमाण(शून्य *) + परिमाण(size_t)) ?
    परिमाण(max_align_t)
    : परिमाण(शून्य *) + परिमाण(size_t);

#परिभाषय सङ्कलय_गन्तव्यम्_POINTER(p, d) ((शून्य *)((uintptr_t)(p) + (d)))

शून्य *
ffi_संवृतिः_alloc (size_t आकार, शून्य **संहिता)
{
  स्थिर size_t page_आकार;
  size_t rounded_आकार;
  शून्य *codeseg, *dataseg;
  पूर्णाङ्क prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  यदि (!संहिता)
    प्रत्यागम रिक्त;

  /* Obtain system page size. */
  यदि (!page_आकार)
    page_आकार = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_आकार = (आकार + overhead + page_आकार - 1) & ~(page_आकार - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(रिक्त, rounded_आकार, prot, MAP_ANON | MAP_वैयक्तिकम्, -1, 0);
  यदि (dataseg == MAP_FAILED)
    प्रत्यागम रिक्त;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_आकार, रिक्त, rounded_आकार, MAP_REMAPDUP);
  यदि (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_आकार);
    प्रत्यागम रिक्त;
  }
  यदि (mprotect(codeseg, rounded_आकार, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_आकार);
    munmap(dataseg, rounded_आकार);
    प्रत्यागम रिक्त;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  स्मृतिं_प्रतिलिख(dataseg, &rounded_आकार, परिमाण(rounded_आकार));
  स्मृतिं_प्रतिलिख(सङ्कलय_गन्तव्यम्_POINTER(dataseg, परिमाण(size_t)), &codeseg, परिमाण(शून्य *));
  *संहिता = सङ्कलय_गन्तव्यम्_POINTER(codeseg, overhead);
  प्रत्यागम सङ्कलय_गन्तव्यम्_POINTER(dataseg, overhead);
}

शून्य
ffi_संवृतिः_free (शून्य *ptr)
{
  शून्य *codeseg, *dataseg;
  size_t rounded_आकार;

  dataseg = सङ्कलय_गन्तव्यम्_POINTER(ptr, -overhead);
  स्मृतिं_प्रतिलिख(&rounded_आकार, dataseg, परिमाण(rounded_आकार));
  स्मृतिं_प्रतिलिख(&codeseg, सङ्कलय_गन्तव्यम्_POINTER(dataseg, परिमाण(size_t)), परिमाण(शून्य *));
  munmap(dataseg, rounded_आकार);
  munmap(codeseg, rounded_आकार);
}

पूर्णाङ्क
ffi_tramp_is_present (__attribute__((अप्रयुक्तम्)) शून्य *ptr)
{
  प्रत्यागम 0;
}
#अन्यथा /* !NetBSD with PROT_MPROTECT */

#यदि !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_सारणी
# यदि __linux__ && !defined(__ANDROID__)
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
# यदि defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#यदि_अन्तः

#यदि FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# यदि !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# यदि !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#यदि_अन्तः /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#यदि FFI_CLOSURES

#यदि FFI_EXEC_TRAMPOLINE_सारणी

#यदि_परिभाषितम् __MACH__

#अन्तर्भाव <mach/mach.h>
#अन्तर्भाव <pthread.h>
#यदि_परिभाषितम् HAVE_ARM64E_PTRAUTH
#अन्तर्भाव <ptrauth.h>
#यदि_अन्तः
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>

बाह्य शून्य *ffi_संवृतिः_trampoline_सारणी_page;

प्रकारनिर्देश संरचना ffi_trampoline_सारणी ffi_trampoline_सारणी;
प्रकारनिर्देश संरचना ffi_trampoline_सारणी_प्रविष्टिः ffi_trampoline_सारणी_प्रविष्टिः;

संरचना ffi_trampoline_सारणी
{
  /* contiguous writable and executable pages */
  vm_स्थानाङ्कः_t config_page;

  /* free list tracking */
  uint16_t free_गणनम्;
  ffi_trampoline_सारणी_प्रविष्टिः *free_list;
  ffi_trampoline_सारणी_प्रविष्टिः *free_list_pool;

  ffi_trampoline_सारणी *पूर्व;
  ffi_trampoline_सारणी *अग्र;
};

संरचना ffi_trampoline_सारणी_प्रविष्टिः
{
  शून्य *(*trampoline) (शून्य);
  ffi_trampoline_सारणी_प्रविष्टिः *अग्र;
};

/* Total number of trampolines that fit in one trampoline table */
#परिभाषय FFI_TRAMPOLINE_गणनम् (PAGE_MAX_आकार / FFI_TRAMPOLINE_आकार)

स्थिर pthread_mutex_t ffi_trampoline_तालकम् = PTHREAD_MUTEX_INITIALIZER;
स्थिर ffi_trampoline_सारणी *ffi_trampoline_tables = रिक्त;

स्थिर ffi_trampoline_सारणी *
ffi_trampoline_सारणी_alloc (शून्य)
{
  ffi_trampoline_सारणी *सारणी;
  vm_स्थानाङ्कः_t config_page;
  vm_स्थानाङ्कः_t trampoline_page;
  vm_स्थानाङ्कः_t trampoline_page_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_page = 0x0;
  kt = vm_allocate (mach_task_self (), &config_page, PAGE_MAX_आकार * 2,
		    VM_FLAGS_ANYWHERE);
  यदि (kt != KERN_SUCCESS)
    प्रत्यागम रिक्त;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_page = config_page + PAGE_MAX_आकार;

#यदि_परिभाषितम् HAVE_ARM64E_PTRAUTH
  trampoline_page_template = (vm_स्थानाङ्कः_t)(uintptr_t)ptrauth_auth_दत्त((शून्य *)&ffi_संवृतिः_trampoline_सारणी_page, ptrauth_कुञ्चिका_function_pointer, 0);
#अन्यथा
  trampoline_page_template = (vm_स्थानाङ्कः_t)&ffi_संवृतिः_trampoline_सारणी_page;
#यदि_अन्तः

#यदि_परिभाषितम् __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_page_template &= ~1UL;
#यदि_अन्तः
  kt = vm_remap (mach_task_self (), &trampoline_page, PAGE_MAX_आकार, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_page_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  यदि (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      प्रत्यागम रिक्त;
    }

  यदि (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_page, PAGE_MAX_आकार,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      यदि (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
          प्रत्यागम रिक्त;
        }
    }

  /* We have valid trampoline and config pages */
  सारणी = शून्यस्मृतिं_देहि (1, परिमाण (ffi_trampoline_सारणी));
  यदि (सारणी == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      प्रत्यागम रिक्त;
    }
  सारणी->free_गणनम् = FFI_TRAMPOLINE_गणनम्;
  सारणी->config_page = config_page;

  /* Create and initialize the free list */
  सारणी->free_list_pool =
    शून्यस्मृतिं_देहि (FFI_TRAMPOLINE_गणनम्, परिमाण (ffi_trampoline_सारणी_प्रविष्टिः));
  यदि (सारणी->free_list_pool == रिक्त)
    {
      vm_deallocate (mach_task_self (), config_page, PAGE_MAX_आकार * 2);
      मुक्त (सारणी);
      प्रत्यागम रिक्त;
    }

  प्रति (i = 0; i < सारणी->free_गणनम्; i++)
    {
      ffi_trampoline_सारणी_प्रविष्टिः *प्रविष्टिः = &सारणी->free_list_pool[i];
      प्रविष्टिः->trampoline =
	(शून्य *) (trampoline_page + (i * FFI_TRAMPOLINE_आकार));
#यदि_परिभाषितम् HAVE_ARM64E_PTRAUTH
      प्रविष्टिः->trampoline = ptrauth_sign_unauthenticated(प्रविष्टिः->trampoline, ptrauth_कुञ्चिका_function_pointer, 0);
#यदि_अन्तः

      यदि (i < सारणी->free_गणनम् - 1)
	प्रविष्टिः->अग्र = &सारणी->free_list_pool[i + 1];
    }

  सारणी->free_list = सारणी->free_list_pool;

  प्रत्यागम सारणी;
}

स्थिर शून्य
ffi_trampoline_सारणी_free (ffi_trampoline_सारणी *सारणी)
{
  /* Remove from the list */
  यदि (सारणी->पूर्व != रिक्त)
    सारणी->पूर्व->अग्र = सारणी->अग्र;

  यदि (सारणी->अग्र != रिक्त)
    सारणी->अग्र->पूर्व = सारणी->पूर्व;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), सारणी->config_page, PAGE_MAX_आकार * 2);

  /* Deallocate free list */
  मुक्त (सारणी->free_list_pool);
  मुक्त (सारणी);
}

शून्य *
ffi_संवृतिः_alloc (size_t आकार, शून्य **संहिता)
{
  /* Create the closure */
  ffi_संवृतिः *संवृतिः = स्मृतिं_देहि (आकार);
  यदि (संवृतिः == रिक्त)
    प्रत्यागम रिक्त;

  pthread_mutex_तालकम् (&ffi_trampoline_तालकम्);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_सारणी *सारणी = ffi_trampoline_tables;
  यदि (सारणी == रिक्त || सारणी->free_list == रिक्त)
    {
      सारणी = ffi_trampoline_सारणी_alloc ();
      यदि (सारणी == रिक्त)
	{
	  pthread_mutex_unlock (&ffi_trampoline_तालकम्);
	  मुक्त (संवृतिः);
	  प्रत्यागम रिक्त;
	}

      /* Insert the new table at the top of the list */
      सारणी->अग्र = ffi_trampoline_tables;
      यदि (सारणी->अग्र != रिक्त)
	सारणी->अग्र->पूर्व = सारणी;

      ffi_trampoline_tables = सारणी;
    }

  /* Claim the free entry */
  ffi_trampoline_सारणी_प्रविष्टिः *प्रविष्टिः = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = प्रविष्टिः->अग्र;
  ffi_trampoline_tables->free_गणनम्--;
  प्रविष्टिः->अग्र = रिक्त;

  pthread_mutex_unlock (&ffi_trampoline_तालकम्);

  /* Initialize the return values */
  *संहिता = प्रविष्टिः->trampoline;
  संवृतिः->trampoline_सारणी = सारणी;
  संवृतिः->trampoline_सारणी_प्रविष्टिः = प्रविष्टिः;

  प्रत्यागम संवृतिः;
}

शून्य
ffi_संवृतिः_free (शून्य *ptr)
{
  ffi_संवृतिः *संवृतिः = ptr;

  pthread_mutex_तालकम् (&ffi_trampoline_तालकम्);

  /* Fetch the table and entry references */
  ffi_trampoline_सारणी *सारणी = संवृतिः->trampoline_सारणी;
  ffi_trampoline_सारणी_प्रविष्टिः *प्रविष्टिः = संवृतिः->trampoline_सारणी_प्रविष्टिः;

  /* Return the entry to the free list */
  प्रविष्टिः->अग्र = सारणी->free_list;
  सारणी->free_list = प्रविष्टिः;
  सारणी->free_गणनम्++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  यदि (सारणी->free_गणनम् == FFI_TRAMPOLINE_गणनम्
      && ffi_trampoline_tables != सारणी)
    {
      ffi_trampoline_सारणी_free (सारणी);
    }
  अन्यथा यदि (ffi_trampoline_tables != सारणी)
    {
      /* Otherwise, bump this table to the top of the list */
      सारणी->पूर्व = रिक्त;
      सारणी->अग्र = ffi_trampoline_tables;
      यदि (ffi_trampoline_tables != रिक्त)
	ffi_trampoline_tables->पूर्व = सारणी;

      ffi_trampoline_tables = सारणी;
    }

  pthread_mutex_unlock (&ffi_trampoline_तालकम्);

  /* Free the closure */
  मुक्त (संवृतिः);
}

#यदि_अन्तः

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#नोचेद्यदि FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#परिभाषय USE_LOCKS 1
#परिभाषय USE_DL_उपसर्गः 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#परिभाषय HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#परिभाषय HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#परिभाषय NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#परिभाषय DEFAULT_MMAP_देहली MAX_आकार_T

/* Don't allocate more than a page unless needed.  */
#परिभाषय DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <errno.h>
#यदि_अपरिभाषितम् _MSC_VER
#अन्तर्भाव <unistd.h>
#यदि_अन्तः
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#यदि !defined(_WIN32)
#यदि_परिभाषितम् HAVE_MNTENT
#अन्तर्भाव <mntent.h>
#यदि_अन्तः /* HAVE_MNTENT */
#अन्तर्भाव <sys/param.h>
#अन्तर्भाव <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#अन्तर्भाव <sys/mman.h>
#परिभाषय LACKS_SYS_MMAN_H 1

#यदि FFI_MMAP_EXEC_SELINUX
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

  यदि (statfs ("/selinux", &sfs) >= 0
      && (अचिह्नित पूर्णाङ्क) sfs.f_type == 0xf97cff8cU)
    प्रत्यागम 1;
  f = fopen ("/proc/mounts", "r");
  यदि (f == रिक्त)
    प्रत्यागम 0;
  यावत् (getline (&buf, &len, f) >= 0)
    {
      वर्ण *p = strchr (buf, ' ');
      यदि (p == रिक्त)
        भङ्ग;
      p = strchr (p + 1, ' ');
      यदि (p == रिक्त)
        भङ्ग;
      यदि (वाक्ये_तुलय (p + 1, "selinuxfs ", 10) == 0)
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

#परिभाषय is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#अन्यथा

#परिभाषय is_selinux_enabled() 0

#यदि_अन्तः /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#यदि defined FFI_MMAP_PAX
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
  यदि (f == रिक्त)
    प्रत्यागम 0;
  ret = 0;

  यावत् (getline (&buf, &len, f) != -1)
    यदि (!वाक्ये_तुलय (buf, "PaX:", 4))
      {
        यदि (रिक्त != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        यदि (रिक्त != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        भङ्ग;
      }
  मुक्त (buf);
  fclose (f);
  प्रत्यागम ret;
}

#परिभाषय get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#परिभाषय has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#परिभाषय is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#परिभाषय is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#यदि_अन्तः /* defined FFI_MMAP_PAX */

#नोचेद्यदि defined (__CYGWIN__) || defined(__INTERIX)

#अन्तर्भाव <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#परिभाषय is_selinux_enabled() 0

#यदि_अन्तः /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#यदि !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#यदि_अन्तः /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
स्थिर शून्य *dlmalloc(size_t);
स्थिर शून्य dlfree(शून्य*);
स्थिर शून्य *dlcalloc(size_t, size_t) MAYBE_अप्रयुक्तम्;
स्थिर शून्य *dlrealloc(शून्य *, size_t) MAYBE_अप्रयुक्तम्;
स्थिर शून्य *dlmemalign(size_t, size_t) MAYBE_अप्रयुक्तम्;
स्थिर शून्य *dlvalloc(size_t) MAYBE_अप्रयुक्तम्;
स्थिर पूर्णाङ्क dlmallopt(पूर्णाङ्क, पूर्णाङ्क) MAYBE_अप्रयुक्तम्;
स्थिर size_t dlmalloc_footprint(शून्य) MAYBE_अप्रयुक्तम्;
स्थिर size_t dlmalloc_max_footprint(शून्य) MAYBE_अप्रयुक्तम्;
स्थिर शून्य** dlindependent_calloc(size_t, size_t, शून्य**) MAYBE_अप्रयुक्तम्;
स्थिर शून्य** dlindependent_comalloc(size_t, size_t*, शून्य**) MAYBE_अप्रयुक्तम्;
स्थिर शून्य *dlpvalloc(size_t) MAYBE_अप्रयुक्तम्;
स्थिर पूर्णाङ्क dlmalloc_trim(size_t) MAYBE_अप्रयुक्तम्;
स्थिर size_t dlmalloc_usable_आकार(शून्य*) MAYBE_अप्रयुक्तम्;
स्थिर शून्य dlmalloc_stats(शून्य) MAYBE_अप्रयुक्तम्;

#यदि !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
स्थिर शून्य *dlmmap(शून्य *, size_t, पूर्णाङ्क, पूर्णाङ्क, पूर्णाङ्क, off_t);
स्थिर पूर्णाङ्क dlmunmap(शून्य *, size_t);
#यदि_अन्तः /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#परिभाषय mmap dlmmap
#परिभाषय munmap dlmunmap

#अन्तर्भाव "dlmalloc.c"

#अपरिभाषय mmap
#अपरिभाषय munmap

#यदि !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
स्थिर pthread_mutex_t open_अस्थायी_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
स्थिर पूर्णाङ्क execfd = -1;

/* The amount of space already allocated from the temporary file.  */
स्थिर size_t execsize = 0;

#यदि_परिभाषितम् HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_memfd (ध्रुव वर्ण *name)
{
  पूर्णाङ्क fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  प्रत्यागम fd;
}
#यदि_अन्तः

/* Open a temporary file name, and immediately unlink it.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_name (वर्ण *name, पूर्णाङ्क flags MAYBE_अप्रयुक्तम्)
{
  पूर्णाङ्क fd;

#यदि_परिभाषितम् HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#अन्यथा
  fd = mkstemp (name);
#यदि_अन्तः

  यदि (fd != -1)
    unlink (name);

  प्रत्यागम fd;
}

/* Open a temporary file in the named directory.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_dir (ध्रुव वर्ण *dir)
{
  स्थिर ध्रुव वर्ण suffix[] = "/ffiXXXXXX";
  पूर्णाङ्क lendir, flags;
  वर्ण *tempname;
#यदि_परिभाषितम् O_TMPFILE
  पूर्णाङ्क fd;
#यदि_अन्तः

#यदि_परिभाषितम् O_CLOEXEC
  flags = O_CLOEXEC;
#अन्यथा
  flags = 0;
#यदि_अन्तः

#यदि_परिभाषितम् O_TMPFILE
  fd = उद्घाटय (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  यदि (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    प्रत्यागम fd;
  } अन्यथा {
    errno = 0;
  }
#यदि_अन्तः

  lendir = (पूर्णाङ्क) वाक्यदैर्घ्यम् (dir);
  tempname = __builtin_alloca (lendir + परिमाण (suffix));

  यदि (!tempname)
    प्रत्यागम -1;

  स्मृतिं_प्रतिलिख (tempname, dir, lendir);
  स्मृतिं_प्रतिलिख (tempname + lendir, suffix, परिमाण (suffix));

  प्रत्यागम open_अस्थायी_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_env (ध्रुव वर्ण *envvar)
{
  ध्रुव वर्ण *मानम् = getenv (envvar);

  यदि (!मानम्)
    प्रत्यागम -1;

  प्रत्यागम open_अस्थायी_exec_file_dir (मानम्);
}

#यदि_परिभाषितम् HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_mnt (ध्रुव वर्ण *mounts)
{
  स्थिर ध्रुव वर्ण *चरमः_mounts;
  स्थिर FILE *चरमः_mntent;

  यदि (mounts != चरमः_mounts)
    {
      यदि (चरमः_mntent)
	endmntent (चरमः_mntent);

      चरमः_mounts = mounts;

      यदि (mounts)
	चरमः_mntent = setmntent (mounts, "r");
      अन्यथा
	चरमः_mntent = रिक्त;
    }

  यदि (!चरमः_mntent)
    प्रत्यागम -1;

  प्रति (;;)
    {
      पूर्णाङ्क fd;
      संरचना mntent mnt;
      वर्ण buf[MAXPATHLEN * 3];

      यदि (getmntent_r (चरमः_mntent, &mnt, buf, परिमाण (buf)) == रिक्त)
	प्रत्यागम -1;

      यदि (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	अनुवर्त;

      fd = open_अस्थायी_exec_file_dir (mnt.mnt_dir);

      यदि (fd != -1)
	प्रत्यागम fd;
    }
}
#यदि_अन्तः /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
स्थिर संरचना
{
  पूर्णाङ्क (*func)(ध्रुव वर्ण *);
  ध्रुव वर्ण *arg;
  पूर्णाङ्क repeat;
} open_अस्थायी_exec_file_opts[] = {
#यदि_परिभाषितम् HAVE_MEMFD_CREATE
  { open_अस्थायी_exec_file_memfd, "libffi", 0 },
#यदि_अन्तः
  { open_अस्थायी_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_अस्थायी_exec_file_env, "TMPDIR", 0 },
  { open_अस्थायी_exec_file_dir, "/tmp", 0 },
  { open_अस्थायी_exec_file_dir, "/var/tmp", 0 },
  { open_अस्थायी_exec_file_dir, "/dev/shm", 0 },
  { open_अस्थायी_exec_file_env, "HOME", 0 },
#यदि_परिभाषितम् HAVE_MNTENT
  { open_अस्थायी_exec_file_mnt, "/etc/mtab", 1 },
  { open_अस्थायी_exec_file_mnt, "/proc/mounts", 1 },
#यदि_अन्तः /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
स्थिर पूर्णाङ्क open_अस्थायी_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
स्थिर पूर्णाङ्क
open_अस्थायी_exec_file_opts_अग्र (शून्य)
{
  यदि (open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat)
    open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].func (रिक्त);

  open_अस्थायी_exec_file_opts_idx++;
  यदि (open_अस्थायी_exec_file_opts_idx
      == (परिमाण (open_अस्थायी_exec_file_opts)
	  / परिमाण (*open_अस्थायी_exec_file_opts)))
    {
      open_अस्थायी_exec_file_opts_idx = 0;
      प्रत्यागम 1;
    }

  प्रत्यागम 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
पूर्णाङ्क
open_अस्थायी_exec_file (शून्य)
{
  पूर्णाङ्क fd;

  कुरु
    {
      fd = open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].func
	(open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].arg);

      यदि (!open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  यदि (open_अस्थायी_exec_file_opts_अग्र ())
	    भङ्ग;
	}
    }
  यावत् (fd == -1);

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
  स्थिर दीर्घ page_आकार;

  /* Obtain system page size. */
  यदि (!page_आकार)
    page_आकार = sysconf(_SC_PAGESIZE);
  यदि (page_आकार <= 0)
    प्रत्यागम -1;

  अचिह्नित वर्ण buf[page_आकार];
  स्मृतिं_पूरय (buf, 0, page_आकार);

  यावत् (len > 0)
    {
      off_t गन्तव्यम्_write = (len < page_आकार) ? len : page_आकार;
      यदि (लिख (fd, buf, गन्तव्यम्_write) < गन्तव्यम्_write)
        प्रत्यागम -1;
      len -= गन्तव्यम्_write;
    }

  प्रत्यागम 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
स्थिर शून्य *
dlmmap_locked (शून्य *start, size_t दीर्घता, पूर्णाङ्क prot, पूर्णाङ्क flags, off_t offset)
{
  शून्य *ptr;

  यदि (execfd == -1)
    {
      open_अस्थायी_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_अस्थायी_exec_file ();
      यदि (execfd == -1)
	प्रत्यागम MFAIL;
    }

  offset = execsize;

  यदि (allocate_space (execfd, दीर्घता))
    प्रत्यागम MFAIL;

  flags &= ~(MAP_वैयक्तिकम् | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (रिक्त, दीर्घता, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  यदि (ptr == MFAIL)
    {
      यदि (!offset)
	{
	  close (execfd);
	  गच्छ retry_open;
	}
      यदि (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      प्रत्यागम MFAIL;
    }
  अन्यथा यदि (!offset
	   && open_अस्थायी_exec_file_opts[open_अस्थायी_exec_file_opts_idx].repeat)
    open_अस्थायी_exec_file_opts_अग्र ();

  start = mmap (start, दीर्घता, prot, flags, execfd, offset);

  यदि (start == MFAIL)
    {
      munmap (ptr, दीर्घता);
      यदि (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      प्रत्यागम start;
    }

  mmap_exec_विस्थापनम् ((वर्ण *)start, दीर्घता) = (वर्ण*)ptr - (वर्ण*)start;

  execsize += दीर्घता;

  प्रत्यागम start;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
स्थिर शून्य *
dlmmap (शून्य *start, size_t दीर्घता, पूर्णाङ्क prot,
	पूर्णाङ्क flags, पूर्णाङ्क fd, off_t offset)
{
  शून्य *ptr;

  प्रतिज्ञा (start == रिक्त && दीर्घता % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_वैयक्तिकम् | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  यदि (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (start, दीर्घता, prot & ~PROT_EXEC, flags, fd, offset);
      प्रत्यागम ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  यदि (execfd == -1 && is_mprotect_enabled ())
    {
#यदि_परिभाषितम् FFI_MMAP_EXEC_EMUTRAMP_PAX
      यदि (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (start, दीर्घता, prot & ~PROT_EXEC, flags, fd, offset);
          प्रत्यागम ptr;
        }
#यदि_अन्तः
      /* fallback to dlmmap_locked.  */
    }
  अन्यथा यदि (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (start, दीर्घता, prot | PROT_EXEC, flags, fd, offset);

      यदि (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	प्रत्यागम ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_तालकम् (&open_अस्थायी_exec_file_mutex);
  ptr = dlmmap_locked (start, दीर्घता, prot, flags, offset);
  pthread_mutex_unlock (&open_अस्थायी_exec_file_mutex);

  प्रत्यागम ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
स्थिर पूर्णाङ्क
dlmunmap (शून्य *start, size_t दीर्घता)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, start);
  शून्य *संहिता;

  यदि (seg && (संहिता = सङ्कलय_segment_exec_विस्थापनम् (start, seg)) != start)
    {
      पूर्णाङ्क ret = munmap (संहिता, दीर्घता);
      यदि (ret)
	प्रत्यागम ret;
    }

  प्रत्यागम munmap (start, दीर्घता);
}

#यदि FFI_संवृतिः_FREE_संहिता
/* Return segment holding given code address.  */
स्थिर msegmentptr
segment_holding_संहिता (mstate m, वर्ण* addr)
{
  msegmentptr sp = &m->seg;
  प्रति (;;) {
    यदि (addr >= सङ्कलय_segment_exec_विस्थापनम् (sp->आधारः, sp)
	&& addr < सङ्कलय_segment_exec_विस्थापनम् (sp->आधारः, sp) + sp->आकार)
      प्रत्यागम sp;
    यदि ((sp = sp->अग्र) == 0)
      प्रत्यागम 0;
  }
}
#यदि_अन्तः

#यदि_अन्तः /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
शून्य *
ffi_संवृतिः_alloc (size_t आकार, शून्य **संहिता)
{
  शून्य *ptr, *ftramp;

  यदि (!संहिता)
    प्रत्यागम रिक्त;

  ptr = dlmalloc (आकार);

  यदि (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *संहिता = FFI_FN (सङ्कलय_segment_exec_विस्थापनम् (ptr, seg));
      यदि (!ffi_tramp_is_supported ())
        प्रत्यागम ptr;

      ftramp = ffi_tramp_alloc (0);
      यदि (ftramp == रिक्त)
      {
        dlfree (ptr);
        प्रत्यागम रिक्त;
      }
      *संहिता = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_संवृतिः *) ptr)->ftramp = ftramp;
    }

  प्रत्यागम ptr;
}

शून्य *
ffi_दत्त_गन्तव्यम्_संहिता_pointer (शून्य *दत्त)
{
  msegmentptr seg = segment_holding (gm, दत्त);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  यदि (seg)
    {
      यदि (!ffi_tramp_is_supported ())
        प्रत्यागम सङ्कलय_segment_exec_विस्थापनम् (दत्त, seg);
      प्रत्यागम ffi_tramp_get_addr (((ffi_संवृतिः *) दत्त)->ftramp);
    }
  अन्यथा
    प्रत्यागम दत्त;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
शून्य
ffi_संवृतिः_free (शून्य *ptr)
{
#यदि FFI_संवृतिः_FREE_संहिता
  msegmentptr seg = segment_holding_संहिता (gm, ptr);

  यदि (seg)
    ptr = sub_segment_exec_विस्थापनम् (ptr, seg);
#यदि_अन्तः
  यदि (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_संवृतिः *) ptr)->ftramp);

  dlfree (ptr);
}

पूर्णाङ्क
ffi_tramp_is_present (शून्य *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  प्रत्यागम seg != रिक्त && ffi_tramp_is_supported();
}

# अन्यथा /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#अन्तर्भाव <stdlib.h>

शून्य *
ffi_संवृतिः_alloc (size_t आकार, शून्य **संहिता)
{
  शून्य *c;

  यदि (!संहिता)
    प्रत्यागम रिक्त;

  c = स्मृतिं_देहि (आकार);
  *संहिता = FFI_FN (c);
  प्रत्यागम c;
}

शून्य
ffi_संवृतिः_free (शून्य *ptr)
{
  मुक्त (ptr);
}

शून्य *
ffi_दत्त_गन्तव्यम्_संहिता_pointer (शून्य *दत्त)
{
  प्रत्यागम दत्त;
}

पूर्णाङ्क
ffi_tramp_is_present (__attribute__((अप्रयुक्तम्)) शून्य *ptr)
{
  प्रत्यागम 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#यदि_अन्तः /* FFI_CLOSURES */

#यदि_अन्तः /* NetBSD with PROT_MPROTECT */
#यदि_अन्तः /* __EMSCRIPTEN__ */

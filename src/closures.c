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

#ఒకవేళ (defined __linux__ || defined __CYGWIN__) && !defined GNU_ఆధారం
#నిర్వచించు GNU_ఆధారం 1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __EMSCRIPTEN__

#అన్తర్భావ <fficonfig.h>
#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <tramp.h>

#నిర్వచితమైతే __NetBSD__
#అన్తర్భావ <sys/param.h>
#అయితే_ముగింపు

#ఒకవేళ __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#అన్తర్భావ <sys/mman.h>

#అన్తర్భావ <stddef.h>
#అన్తర్భావ <unistd.h>
#నిర్వచితమైతే  HAVE_SYS_MEMFD_H
#అన్తర్భావ <sys/memfd.h>
#అయితే_ముగింపు

స్థిర స్థిరరాశి size_t overhead =
  (పరిమాణం(max_align_t) > పరిమాణం(శూన్య *) + పరిమాణం(size_t)) ?
    పరిమాణం(max_align_t)
    : పరిమాణం(శూన్య *) + పరిమాణం(size_t);

#నిర్వచించు కూడు_గమ్యం_సూచిక(p, d) ((శూన్య *)((uintptr_t)(p) + (d)))

శూన్య *
ffi_సంవృతం_alloc (size_t కొలత, శూన్య **సంకేతం_2)
{
  స్థిర size_t పుట_కొలత;
  size_t rounded_కొలత;
  శూన్య *codeseg, *dataseg;
  పూర్ణాంకం prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  ఒకవేళ (!సంకేతం_2)
    తిరిగి ఖాళీ;

  /* Obtain system page size. */
  ఒకవేళ (!పుట_కొలత)
    పుట_కొలత = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_కొలత = (కొలత + overhead + పుట_కొలత - 1) & ~(పుట_కొలత - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(ఖాళీ, rounded_కొలత, prot, MAP_ANON | MAP_వ్యక్తిగత, -1, 0);
  ఒకవేళ (dataseg == MAP_FAILED)
    తిరిగి ఖాళీ;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_కొలత, ఖాళీ, rounded_కొలత, MAP_REMAPDUP);
  ఒకవేళ (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_కొలత);
    తిరిగి ఖాళీ;
  }
  ఒకవేళ (mprotect(codeseg, rounded_కొలత, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_కొలత);
    munmap(dataseg, rounded_కొలత);
    తిరిగి ఖాళీ;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  జ్ఞాపకం_నకలు(dataseg, &rounded_కొలత, పరిమాణం(rounded_కొలత));
  జ్ఞాపకం_నకలు(కూడు_గమ్యం_సూచిక(dataseg, పరిమాణం(size_t)), &codeseg, పరిమాణం(శూన్య *));
  *సంకేతం_2 = కూడు_గమ్యం_సూచిక(codeseg, overhead);
  తిరిగి కూడు_గమ్యం_సూచిక(dataseg, overhead);
}

శూన్య
ffi_సంవృతం_free (శూన్య *ptr)
{
  శూన్య *codeseg, *dataseg;
  size_t rounded_కొలత;

  dataseg = కూడు_గమ్యం_సూచిక(ptr, -overhead);
  జ్ఞాపకం_నకలు(&rounded_కొలత, dataseg, పరిమాణం(rounded_కొలత));
  జ్ఞాపకం_నకలు(&codeseg, కూడు_గమ్యం_సూచిక(dataseg, పరిమాణం(size_t)), పరిమాణం(శూన్య *));
  munmap(dataseg, rounded_కొలత);
  munmap(codeseg, rounded_కొలత);
}

పూర్ణాంకం
ffi_tramp_is_present (__attribute__((వాడని)) శూన్య *ptr)
{
  తిరిగి 0;
}
#లేకపోతే /* !NetBSD with PROT_MPROTECT */

#ఒకవేళ !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_పట్టిక
# ఒకవేళ __linux__ && !defined(__ANDROID__)
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
# ఒకవేళ defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#అయితే_ముగింపు

#ఒకవేళ FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# ఒకవేళ !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# ఒకవేళ !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#అయితే_ముగింపు /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#ఒకవేళ FFI_CLOSURES

#ఒకవేళ FFI_EXEC_TRAMPOLINE_పట్టిక

#నిర్వచితమైతే __MACH__

#అన్తర్భావ <mach/mach.h>
#అన్తర్భావ <pthread.h>
#నిర్వచితమైతే HAVE_ARM64E_PTRAUTH
#అన్తర్భావ <ptrauth.h>
#అయితే_ముగింపు
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>

బాహ్య శూన్య *ffi_సంవృతం_trampoline_పట్టిక_పుట;

రకనిర్వచనం నిర్మాణం ffi_trampoline_పట్టిక ffi_trampoline_పట్టిక;
రకనిర్వచనం నిర్మాణం ffi_trampoline_పట్టిక_నమోదు ffi_trampoline_పట్టిక_నమోదు;

నిర్మాణం ffi_trampoline_పట్టిక
{
  /* contiguous writable and executable pages */
  vm_చిరునామా_t config_పుట;

  /* free list tracking */
  uint16_t free_లెక్క;
  ffi_trampoline_పట్టిక_నమోదు *free_list;
  ffi_trampoline_పట్టిక_నమోదు *free_list_pool;

  ffi_trampoline_పట్టిక *prev;
  ffi_trampoline_పట్టిక *తదుపరి;
};

నిర్మాణం ffi_trampoline_పట్టిక_నమోదు
{
  శూన్య *(*trampoline) (శూన్య);
  ffi_trampoline_పట్టిక_నమోదు *తదుపరి;
};

/* Total number of trampolines that fit in one trampoline table */
#నిర్వచించు FFI_TRAMPOLINE_లెక్క (పుట_MAX_కొలత / FFI_TRAMPOLINE_కొలత)

స్థిర pthread_mutex_t ffi_trampoline_తాళం = PTHREAD_MUTEX_INITIALIZER;
స్థిర ffi_trampoline_పట్టిక *ffi_trampoline_tables = ఖాళీ;

స్థిర ffi_trampoline_పట్టిక *
ffi_trampoline_పట్టిక_alloc (శూన్య)
{
  ffi_trampoline_పట్టిక *పట్టిక;
  vm_చిరునామా_t config_పుట;
  vm_చిరునామా_t trampoline_పుట;
  vm_చిరునామా_t trampoline_పుట_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_పుట = 0x0;
  kt = vm_allocate (mach_task_self (), &config_పుట, పుట_MAX_కొలత * 2,
		    VM_FLAGS_ANYWHERE);
  ఒకవేళ (kt != KERN_SUCCESS)
    తిరిగి ఖాళీ;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_పుట = config_పుట + పుట_MAX_కొలత;

#నిర్వచితమైతే HAVE_ARM64E_PTRAUTH
  trampoline_పుట_template = (vm_చిరునామా_t)(uintptr_t)ptrauth_auth_దత్తాంశం((శూన్య *)&ffi_సంవృతం_trampoline_పట్టిక_పుట, ptrauth_కీలకం_function_సూచిక, 0);
#లేకపోతే
  trampoline_పుట_template = (vm_చిరునామా_t)&ffi_సంవృతం_trampoline_పట్టిక_పుట;
#అయితే_ముగింపు

#నిర్వచితమైతే __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_పుట_template &= ~1UL;
#అయితే_ముగింపు
  kt = vm_remap (mach_task_self (), &trampoline_పుట, పుట_MAX_కొలత, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_పుట_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  ఒకవేళ (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_పుట, పుట_MAX_కొలత * 2);
      తిరిగి ఖాళీ;
    }

  ఒకవేళ (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_పుట, పుట_MAX_కొలత,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      ఒకవేళ (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_పుట, పుట_MAX_కొలత * 2);
          తిరిగి ఖాళీ;
        }
    }

  /* We have valid trampoline and config pages */
  పట్టిక = సున్నా_జ్ఞాపకం_ఇవ్వు (1, పరిమాణం (ffi_trampoline_పట్టిక));
  ఒకవేళ (పట్టిక == ఖాళీ)
    {
      vm_deallocate (mach_task_self (), config_పుట, పుట_MAX_కొలత * 2);
      తిరిగి ఖాళీ;
    }
  పట్టిక->free_లెక్క = FFI_TRAMPOLINE_లెక్క;
  పట్టిక->config_పుట = config_పుట;

  /* Create and initialize the free list */
  పట్టిక->free_list_pool =
    సున్నా_జ్ఞాపకం_ఇవ్వు (FFI_TRAMPOLINE_లెక్క, పరిమాణం (ffi_trampoline_పట్టిక_నమోదు));
  ఒకవేళ (పట్టిక->free_list_pool == ఖాళీ)
    {
      vm_deallocate (mach_task_self (), config_పుట, పుట_MAX_కొలత * 2);
      విడుదల (పట్టిక);
      తిరిగి ఖాళీ;
    }

  కొరకు (i = 0; i < పట్టిక->free_లెక్క; i++)
    {
      ffi_trampoline_పట్టిక_నమోదు *నమోదు_2 = &పట్టిక->free_list_pool[i];
      నమోదు_2->trampoline =
	(శూన్య *) (trampoline_పుట + (i * FFI_TRAMPOLINE_కొలత));
#నిర్వచితమైతే HAVE_ARM64E_PTRAUTH
      నమోదు_2->trampoline = ptrauth_sign_unauthenticated(నమోదు_2->trampoline, ptrauth_కీలకం_function_సూచిక, 0);
#అయితే_ముగింపు

      ఒకవేళ (i < పట్టిక->free_లెక్క - 1)
	నమోదు_2->తదుపరి = &పట్టిక->free_list_pool[i + 1];
    }

  పట్టిక->free_list = పట్టిక->free_list_pool;

  తిరిగి పట్టిక;
}

స్థిర శూన్య
ffi_trampoline_పట్టిక_free (ffi_trampoline_పట్టిక *పట్టిక)
{
  /* Remove from the list */
  ఒకవేళ (పట్టిక->prev != ఖాళీ)
    పట్టిక->prev->తదుపరి = పట్టిక->తదుపరి;

  ఒకవేళ (పట్టిక->తదుపరి != ఖాళీ)
    పట్టిక->తదుపరి->prev = పట్టిక->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), పట్టిక->config_పుట, పుట_MAX_కొలత * 2);

  /* Deallocate free list */
  విడుదల (పట్టిక->free_list_pool);
  విడుదల (పట్టిక);
}

శూన్య *
ffi_సంవృతం_alloc (size_t కొలత, శూన్య **సంకేతం_2)
{
  /* Create the closure */
  ffi_సంవృతం *సంవృతం = జ్ఞాపకం_ఇవ్వు (కొలత);
  ఒకవేళ (సంవృతం == ఖాళీ)
    తిరిగి ఖాళీ;

  pthread_mutex_తాళం (&ffi_trampoline_తాళం);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_పట్టిక *పట్టిక = ffi_trampoline_tables;
  ఒకవేళ (పట్టిక == ఖాళీ || పట్టిక->free_list == ఖాళీ)
    {
      పట్టిక = ffi_trampoline_పట్టిక_alloc ();
      ఒకవేళ (పట్టిక == ఖాళీ)
	{
	  pthread_mutex_unlock (&ffi_trampoline_తాళం);
	  విడుదల (సంవృతం);
	  తిరిగి ఖాళీ;
	}

      /* Insert the new table at the top of the list */
      పట్టిక->తదుపరి = ffi_trampoline_tables;
      ఒకవేళ (పట్టిక->తదుపరి != ఖాళీ)
	పట్టిక->తదుపరి->prev = పట్టిక;

      ffi_trampoline_tables = పట్టిక;
    }

  /* Claim the free entry */
  ffi_trampoline_పట్టిక_నమోదు *నమోదు_2 = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = నమోదు_2->తదుపరి;
  ffi_trampoline_tables->free_లెక్క--;
  నమోదు_2->తదుపరి = ఖాళీ;

  pthread_mutex_unlock (&ffi_trampoline_తాళం);

  /* Initialize the return values */
  *సంకేతం_2 = నమోదు_2->trampoline;
  సంవృతం->trampoline_పట్టిక = పట్టిక;
  సంవృతం->trampoline_పట్టిక_నమోదు = నమోదు_2;

  తిరిగి సంవృతం;
}

శూన్య
ffi_సంవృతం_free (శూన్య *ptr)
{
  ffi_సంవృతం *సంవృతం = ptr;

  pthread_mutex_తాళం (&ffi_trampoline_తాళం);

  /* Fetch the table and entry references */
  ffi_trampoline_పట్టిక *పట్టిక = సంవృతం->trampoline_పట్టిక;
  ffi_trampoline_పట్టిక_నమోదు *నమోదు_2 = సంవృతం->trampoline_పట్టిక_నమోదు;

  /* Return the entry to the free list */
  నమోదు_2->తదుపరి = పట్టిక->free_list;
  పట్టిక->free_list = నమోదు_2;
  పట్టిక->free_లెక్క++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  ఒకవేళ (పట్టిక->free_లెక్క == FFI_TRAMPOLINE_లెక్క
      && ffi_trampoline_tables != పట్టిక)
    {
      ffi_trampoline_పట్టిక_free (పట్టిక);
    }
  లేకపోతే ఒకవేళ (ffi_trampoline_tables != పట్టిక)
    {
      /* Otherwise, bump this table to the top of the list */
      పట్టిక->prev = ఖాళీ;
      పట్టిక->తదుపరి = ffi_trampoline_tables;
      ఒకవేళ (ffi_trampoline_tables != ఖాళీ)
	ffi_trampoline_tables->prev = పట్టిక;

      ffi_trampoline_tables = పట్టిక;
    }

  pthread_mutex_unlock (&ffi_trampoline_తాళం);

  /* Free the closure */
  విడుదల (సంవృతం);
}

#అయితే_ముగింపు

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#లేదావేళ FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#నిర్వచించు USE_LOCKS 1
#నిర్వచించు USE_DL_ఉపసర్గ 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#నిర్వచించు HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#నిర్వచించు HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#నిర్వచించు NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#నిర్వచించు DEFAULT_MMAP_పరిమితి MAX_కొలత_T

/* Don't allocate more than a page unless needed.  */
#నిర్వచించు DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <errno.h>
#నిర్వచితం_కాకపోతే _MSC_VER
#అన్తర్భావ <unistd.h>
#అయితే_ముగింపు
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdio.h>
#ఒకవేళ !defined(_WIN32)
#నిర్వచితమైతే HAVE_MNTENT
#అన్తర్భావ <mntent.h>
#అయితే_ముగింపు /* HAVE_MNTENT */
#అన్తర్భావ <sys/param.h>
#అన్తర్భావ <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#అన్తర్భావ <sys/mman.h>
#నిర్వచించు LACKS_SYS_MMAN_H 1

#ఒకవేళ FFI_MMAP_EXEC_SELINUX
#అన్తర్భావ <sys/statfs.h>
#అన్తర్భావ <stdlib.h>

స్థిర పూర్ణాంకం selinux_enabled = -1;

స్థిర పూర్ణాంకం
selinux_enabled_check (శూన్య)
{
  నిర్మాణం statfs sfs;
  FILE *f;
  అక్షరం *buf = ఖాళీ;
  size_t len = 0;

  ఒకవేళ (statfs ("/selinux", &sfs) >= 0
      && (గుర్తులేని పూర్ణాంకం) sfs.f_type == 0xf97cff8cU)
    తిరిగి 1;
  f = fopen ("/proc/mounts", "r");
  ఒకవేళ (f == ఖాళీ)
    తిరిగి 0;
  అయితే (getline (&buf, &len, f) >= 0)
    {
      అక్షరం *p = strchr (buf, ' ');
      ఒకవేళ (p == ఖాళీ)
        ఆపు;
      p = strchr (p + 1, ' ');
      ఒకవేళ (p == ఖాళీ)
        ఆపు;
      ఒకవేళ (అక్షరశ్రేణి_పోలిక (p + 1, "selinuxfs ", 10) == 0)
        {
          విడుదల (buf);
          fclose (f);
          తిరిగి 1;
        }
    }
  విడుదల (buf);
  fclose (f);
  తిరిగి 0;
}

#నిర్వచించు is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#లేకపోతే

#నిర్వచించు is_selinux_enabled() 0

#అయితే_ముగింపు /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#ఒకవేళ defined FFI_MMAP_PAX
#అన్తర్భావ <stdlib.h>

గణన {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
స్థిర పూర్ణాంకం cached_pax_flags = -1;

స్థిర పూర్ణాంకం
pax_flags_check (శూన్య)
{
  అక్షరం *buf = ఖాళీ;
  size_t len = 0;
  FILE *f;
  పూర్ణాంకం ret;
  f = fopen ("/proc/self/status", "r");
  ఒకవేళ (f == ఖాళీ)
    తిరిగి 0;
  ret = 0;

  అయితే (getline (&buf, &len, f) != -1)
    ఒకవేళ (!అక్షరశ్రేణి_పోలిక (buf, "PaX:", 4))
      {
        ఒకవేళ (ఖాళీ != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        ఒకవేళ (ఖాళీ != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        ఆపు;
      }
  విడుదల (buf);
  fclose (f);
  తిరిగి ret;
}

#నిర్వచించు get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#నిర్వచించు has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#నిర్వచించు is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#నిర్వచించు is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#అయితే_ముగింపు /* defined FFI_MMAP_PAX */

#లేదావేళ defined (__CYGWIN__) || defined(__INTERIX)

#అన్తర్భావ <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#నిర్వచించు is_selinux_enabled() 0

#అయితే_ముగింపు /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#ఒకవేళ !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#అయితే_ముగింపు /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
స్థిర శూన్య *dlmalloc(size_t);
స్థిర శూన్య dlfree(శూన్య*);
స్థిర శూన్య *dlcalloc(size_t, size_t) MAYBE_వాడని;
స్థిర శూన్య *dlrealloc(శూన్య *, size_t) MAYBE_వాడని;
స్థిర శూన్య *dlmemalign(size_t, size_t) MAYBE_వాడని;
స్థిర శూన్య *dlvalloc(size_t) MAYBE_వాడని;
స్థిర పూర్ణాంకం dlmallopt(పూర్ణాంకం, పూర్ణాంకం) MAYBE_వాడని;
స్థిర size_t dlmalloc_footprint(శూన్య) MAYBE_వాడని;
స్థిర size_t dlmalloc_max_footprint(శూన్య) MAYBE_వాడని;
స్థిర శూన్య** dlindependent_calloc(size_t, size_t, శూన్య**) MAYBE_వాడని;
స్థిర శూన్య** dlindependent_comalloc(size_t, size_t*, శూన్య**) MAYBE_వాడని;
స్థిర శూన్య *dlpvalloc(size_t) MAYBE_వాడని;
స్థిర పూర్ణాంకం dlmalloc_trim(size_t) MAYBE_వాడని;
స్థిర size_t dlmalloc_usable_కొలత(శూన్య*) MAYBE_వాడని;
స్థిర శూన్య dlmalloc_stats(శూన్య) MAYBE_వాడని;

#ఒకవేళ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
స్థిర శూన్య *dlmmap(శూన్య *, size_t, పూర్ణాంకం, పూర్ణాంకం, పూర్ణాంకం, off_t);
స్థిర పూర్ణాంకం dlmunmap(శూన్య *, size_t);
#అయితే_ముగింపు /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#నిర్వచించు mmap dlmmap
#నిర్వచించు munmap dlmunmap

#అన్తర్భావ "dlmalloc.c"

#నిర్వచనం_రద్దు mmap
#నిర్వచనం_రద్దు munmap

#ఒకవేళ !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
స్థిర pthread_mutex_t open_తాత్కాలిక_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
స్థిర పూర్ణాంకం execfd = -1;

/* The amount of space already allocated from the temporary file.  */
స్థిర size_t execsize = 0;

#నిర్వచితమైతే HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_memfd (స్థిరరాశి అక్షరం *name)
{
  పూర్ణాంకం fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  తిరిగి fd;
}
#అయితే_ముగింపు

/* Open a temporary file name, and immediately unlink it.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_name (అక్షరం *name, పూర్ణాంకం flags MAYBE_వాడని)
{
  పూర్ణాంకం fd;

#నిర్వచితమైతే HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#లేకపోతే
  fd = mkstemp (name);
#అయితే_ముగింపు

  ఒకవేళ (fd != -1)
    unlink (name);

  తిరిగి fd;
}

/* Open a temporary file in the named directory.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_dir (స్థిరరాశి అక్షరం *dir)
{
  స్థిర స్థిరరాశి అక్షరం suffix[] = "/ffiXXXXXX";
  పూర్ణాంకం lendir, flags;
  అక్షరం *tempname;
#నిర్వచితమైతే O_TMPFILE
  పూర్ణాంకం fd;
#అయితే_ముగింపు

#నిర్వచితమైతే O_CLOEXEC
  flags = O_CLOEXEC;
#లేకపోతే
  flags = 0;
#అయితే_ముగింపు

#నిర్వచితమైతే O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  ఒకవేళ (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    తిరిగి fd;
  } లేకపోతే {
    errno = 0;
  }
#అయితే_ముగింపు

  lendir = (పూర్ణాంకం) అక్షరశ్రేణి_పొడవు (dir);
  tempname = __builtin_alloca (lendir + పరిమాణం (suffix));

  ఒకవేళ (!tempname)
    తిరిగి -1;

  జ్ఞాపకం_నకలు (tempname, dir, lendir);
  జ్ఞాపకం_నకలు (tempname + lendir, suffix, పరిమాణం (suffix));

  తిరిగి open_తాత్కాలిక_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_env (స్థిరరాశి అక్షరం *envvar)
{
  స్థిరరాశి అక్షరం *విలువ = getenv (envvar);

  ఒకవేళ (!విలువ)
    తిరిగి -1;

  తిరిగి open_తాత్కాలిక_exec_file_dir (విలువ);
}

#నిర్వచితమైతే HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_mnt (స్థిరరాశి అక్షరం *mounts)
{
  స్థిర స్థిరరాశి అక్షరం *చివరి_mounts;
  స్థిర FILE *చివరి_mntent;

  ఒకవేళ (mounts != చివరి_mounts)
    {
      ఒకవేళ (చివరి_mntent)
	endmntent (చివరి_mntent);

      చివరి_mounts = mounts;

      ఒకవేళ (mounts)
	చివరి_mntent = setmntent (mounts, "r");
      లేకపోతే
	చివరి_mntent = ఖాళీ;
    }

  ఒకవేళ (!చివరి_mntent)
    తిరిగి -1;

  కొరకు (;;)
    {
      పూర్ణాంకం fd;
      నిర్మాణం mntent mnt;
      అక్షరం buf[MAXPATHLEN * 3];

      ఒకవేళ (getmntent_r (చివరి_mntent, &mnt, buf, పరిమాణం (buf)) == ఖాళీ)
	తిరిగి -1;

      ఒకవేళ (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	కొనసాగు;

      fd = open_తాత్కాలిక_exec_file_dir (mnt.mnt_dir);

      ఒకవేళ (fd != -1)
	తిరిగి fd;
    }
}
#అయితే_ముగింపు /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
స్థిర నిర్మాణం
{
  పూర్ణాంకం (*func)(స్థిరరాశి అక్షరం *);
  స్థిరరాశి అక్షరం *arg;
  పూర్ణాంకం repeat;
} open_తాత్కాలిక_exec_file_opts[] = {
#నిర్వచితమైతే HAVE_MEMFD_CREATE
  { open_తాత్కాలిక_exec_file_memfd, "libffi", 0 },
#అయితే_ముగింపు
  { open_తాత్కాలిక_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_తాత్కాలిక_exec_file_env, "TMPDIR", 0 },
  { open_తాత్కాలిక_exec_file_dir, "/tmp", 0 },
  { open_తాత్కాలిక_exec_file_dir, "/var/tmp", 0 },
  { open_తాత్కాలిక_exec_file_dir, "/dev/shm", 0 },
  { open_తాత్కాలిక_exec_file_env, "HOME", 0 },
#నిర్వచితమైతే HAVE_MNTENT
  { open_తాత్కాలిక_exec_file_mnt, "/etc/mtab", 1 },
  { open_తాత్కాలిక_exec_file_mnt, "/proc/mounts", 1 },
#అయితే_ముగింపు /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
స్థిర పూర్ణాంకం open_తాత్కాలిక_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
స్థిర పూర్ణాంకం
open_తాత్కాలిక_exec_file_opts_తదుపరి (శూన్య)
{
  ఒకవేళ (open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].repeat)
    open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].func (ఖాళీ);

  open_తాత్కాలిక_exec_file_opts_idx++;
  ఒకవేళ (open_తాత్కాలిక_exec_file_opts_idx
      == (పరిమాణం (open_తాత్కాలిక_exec_file_opts)
	  / పరిమాణం (*open_తాత్కాలిక_exec_file_opts)))
    {
      open_తాత్కాలిక_exec_file_opts_idx = 0;
      తిరిగి 1;
    }

  తిరిగి 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
పూర్ణాంకం
open_తాత్కాలిక_exec_file (శూన్య)
{
  పూర్ణాంకం fd;

  చేయి
    {
      fd = open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].func
	(open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].arg);

      ఒకవేళ (!open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  ఒకవేళ (open_తాత్కాలిక_exec_file_opts_తదుపరి ())
	    ఆపు;
	}
    }
  అయితే (fd == -1);

  తిరిగి fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
స్థిర పూర్ణాంకం
allocate_space (పూర్ణాంకం fd, off_t len)
{
  స్థిర దీర్ఘ పుట_కొలత;

  /* Obtain system page size. */
  ఒకవేళ (!పుట_కొలత)
    పుట_కొలత = sysconf(_SC_PAGESIZE);
  ఒకవేళ (పుట_కొలత <= 0)
    తిరిగి -1;

  గుర్తులేని అక్షరం buf[పుట_కొలత];
  memset (buf, 0, పుట_కొలత);

  అయితే (len > 0)
    {
      off_t గమ్యం_write = (len < పుట_కొలత) ? len : పుట_కొలత;
      ఒకవేళ (లిఖ (fd, buf, గమ్యం_write) < గమ్యం_write)
        తిరిగి -1;
      len -= గమ్యం_write;
    }

  తిరిగి 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
స్థిర శూన్య *
dlmmap_locked (శూన్య *ప్రారంభం, size_t నిడివి, పూర్ణాంకం prot, పూర్ణాంకం flags, off_t offset)
{
  శూన్య *ptr;

  ఒకవేళ (execfd == -1)
    {
      open_తాత్కాలిక_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_తాత్కాలిక_exec_file ();
      ఒకవేళ (execfd == -1)
	తిరిగి MFAIL;
    }

  offset = execsize;

  ఒకవేళ (allocate_space (execfd, నిడివి))
    తిరిగి MFAIL;

  flags &= ~(MAP_వ్యక్తిగత | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (ఖాళీ, నిడివి, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  ఒకవేళ (ptr == MFAIL)
    {
      ఒకవేళ (!offset)
	{
	  close (execfd);
	  వెళ్ళు retry_open;
	}
      ఒకవేళ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      తిరిగి MFAIL;
    }
  లేకపోతే ఒకవేళ (!offset
	   && open_తాత్కాలిక_exec_file_opts[open_తాత్కాలిక_exec_file_opts_idx].repeat)
    open_తాత్కాలిక_exec_file_opts_తదుపరి ();

  ప్రారంభం = mmap (ప్రారంభం, నిడివి, prot, flags, execfd, offset);

  ఒకవేళ (ప్రారంభం == MFAIL)
    {
      munmap (ptr, నిడివి);
      ఒకవేళ (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      తిరిగి ప్రారంభం;
    }

  mmap_exec_స్థానభ్రంశం ((అక్షరం *)ప్రారంభం, నిడివి) = (అక్షరం*)ptr - (అక్షరం*)ప్రారంభం;

  execsize += నిడివి;

  తిరిగి ప్రారంభం;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
స్థిర శూన్య *
dlmmap (శూన్య *ప్రారంభం, size_t నిడివి, పూర్ణాంకం prot,
	పూర్ణాంకం flags, పూర్ణాంకం fd, off_t offset)
{
  శూన్య *ptr;

  ప్రతిజ్ఞా (ప్రారంభం == ఖాళీ && నిడివి % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_వ్యక్తిగత | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  ఒకవేళ (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (ప్రారంభం, నిడివి, prot & ~PROT_EXEC, flags, fd, offset);
      తిరిగి ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  ఒకవేళ (execfd == -1 && is_mprotect_enabled ())
    {
#నిర్వచితమైతే FFI_MMAP_EXEC_EMUTRAMP_PAX
      ఒకవేళ (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (ప్రారంభం, నిడివి, prot & ~PROT_EXEC, flags, fd, offset);
          తిరిగి ptr;
        }
#అయితే_ముగింపు
      /* fallback to dlmmap_locked.  */
    }
  లేకపోతే ఒకవేళ (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (ప్రారంభం, నిడివి, prot | PROT_EXEC, flags, fd, offset);

      ఒకవేళ (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	తిరిగి ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_తాళం (&open_తాత్కాలిక_exec_file_mutex);
  ptr = dlmmap_locked (ప్రారంభం, నిడివి, prot, flags, offset);
  pthread_mutex_unlock (&open_తాత్కాలిక_exec_file_mutex);

  తిరిగి ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
స్థిర పూర్ణాంకం
dlmunmap (శూన్య *ప్రారంభం, size_t నిడివి)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, ప్రారంభం);
  శూన్య *సంకేతం_2;

  ఒకవేళ (seg && (సంకేతం_2 = కూడు_segment_exec_స్థానభ్రంశం (ప్రారంభం, seg)) != ప్రారంభం)
    {
      పూర్ణాంకం ret = munmap (సంకేతం_2, నిడివి);
      ఒకవేళ (ret)
	తిరిగి ret;
    }

  తిరిగి munmap (ప్రారంభం, నిడివి);
}

#ఒకవేళ FFI_సంవృతం_FREE_సంకేతం
/* Return segment holding given code address.  */
స్థిర msegmentptr
segment_holding_సంకేతం (mstate m, అక్షరం* addr)
{
  msegmentptr sp = &m->seg;
  కొరకు (;;) {
    ఒకవేళ (addr >= కూడు_segment_exec_స్థానభ్రంశం (sp->పునాది, sp)
	&& addr < కూడు_segment_exec_స్థానభ్రంశం (sp->పునాది, sp) + sp->కొలత)
      తిరిగి sp;
    ఒకవేళ ((sp = sp->తదుపరి) == 0)
      తిరిగి 0;
  }
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
శూన్య *
ffi_సంవృతం_alloc (size_t కొలత, శూన్య **సంకేతం_2)
{
  శూన్య *ptr, *ftramp;

  ఒకవేళ (!సంకేతం_2)
    తిరిగి ఖాళీ;

  ptr = dlmalloc (కొలత);

  ఒకవేళ (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *సంకేతం_2 = FFI_FN (కూడు_segment_exec_స్థానభ్రంశం (ptr, seg));
      ఒకవేళ (!ffi_tramp_is_supported ())
        తిరిగి ptr;

      ftramp = ffi_tramp_alloc (0);
      ఒకవేళ (ftramp == ఖాళీ)
      {
        dlfree (ptr);
        తిరిగి ఖాళీ;
      }
      *సంకేతం_2 = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_సంవృతం *) ptr)->ftramp = ftramp;
    }

  తిరిగి ptr;
}

శూన్య *
ffi_దత్తాంశం_గమ్యం_సంకేతం_సూచిక (శూన్య *దత్తాంశం)
{
  msegmentptr seg = segment_holding (gm, దత్తాంశం);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  ఒకవేళ (seg)
    {
      ఒకవేళ (!ffi_tramp_is_supported ())
        తిరిగి కూడు_segment_exec_స్థానభ్రంశం (దత్తాంశం, seg);
      తిరిగి ffi_tramp_get_addr (((ffi_సంవృతం *) దత్తాంశం)->ftramp);
    }
  లేకపోతే
    తిరిగి దత్తాంశం;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
శూన్య
ffi_సంవృతం_free (శూన్య *ptr)
{
#ఒకవేళ FFI_సంవృతం_FREE_సంకేతం
  msegmentptr seg = segment_holding_సంకేతం (gm, ptr);

  ఒకవేళ (seg)
    ptr = sub_segment_exec_స్థానభ్రంశం (ptr, seg);
#అయితే_ముగింపు
  ఒకవేళ (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_సంవృతం *) ptr)->ftramp);

  dlfree (ptr);
}

పూర్ణాంకం
ffi_tramp_is_present (శూన్య *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  తిరిగి seg != ఖాళీ && ffi_tramp_is_supported();
}

# లేకపోతే /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#అన్తర్భావ <stdlib.h>

శూన్య *
ffi_సంవృతం_alloc (size_t కొలత, శూన్య **సంకేతం_2)
{
  శూన్య *c;

  ఒకవేళ (!సంకేతం_2)
    తిరిగి ఖాళీ;

  c = జ్ఞాపకం_ఇవ్వు (కొలత);
  *సంకేతం_2 = FFI_FN (c);
  తిరిగి c;
}

శూన్య
ffi_సంవృతం_free (శూన్య *ptr)
{
  విడుదల (ptr);
}

శూన్య *
ffi_దత్తాంశం_గమ్యం_సంకేతం_సూచిక (శూన్య *దత్తాంశం)
{
  తిరిగి దత్తాంశం;
}

పూర్ణాంకం
ffi_tramp_is_present (__attribute__((వాడని)) శూన్య *ptr)
{
  తిరిగి 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#అయితే_ముగింపు /* FFI_CLOSURES */

#అయితే_ముగింపు /* NetBSD with PROT_MPROTECT */
#అయితే_ముగింపు /* __EMSCRIPTEN__ */

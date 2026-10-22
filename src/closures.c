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

#إذا (defined __linux__ || defined __CYGWIN__) && !defined GNU_سروت
#تعریف_کرو GNU_سروت 1
#جے_انت

#جے_بنا_تعریف __EMSCRIPTEN__

#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <tramp.h>

#جے_تعریف_شدہ __NetBSD__
#ضمن <sys/param.h>
#جے_انت

#إذا __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ضمن <sys/mman.h>

#ضمن <stddef.h>
#ضمن <unistd.h>
#جے_تعریف_شدہ  HAVE_SYS_MEMFD_H
#ضمن <sys/memfd.h>
#جے_انت

ساكن ثابت size_t overhead =
  (حجم(max_align_t) > حجم(فراغ *) + حجم(size_t)) ?
    حجم(max_align_t)
    : حجم(فراغ *) + حجم(size_t);

#تعریف_کرو جوڑو_منزل_اشارہ_گر(p, d) ((فراغ *)((uintptr_t)(p) + (d)))

فراغ *
ffi_گھیرا_alloc (size_t ناپ, فراغ **کوڈ)
{
  ساكن size_t ورقہ_ناپ;
  size_t rounded_ناپ;
  فراغ *codeseg, *dataseg;
  صحيح prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  إذا (!کوڈ)
    أرجع عدم;

  /* Obtain system page size. */
  إذا (!ورقہ_ناپ)
    ورقہ_ناپ = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_ناپ = (ناپ + overhead + ورقہ_ناپ - 1) & ~(ورقہ_ناپ - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(عدم, rounded_ناپ, prot, MAP_ANON | MAP_نجی, -1, 0);
  إذا (dataseg == MAP_FAILED)
    أرجع عدم;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_ناپ, عدم, rounded_ناپ, MAP_REMAPDUP);
  إذا (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_ناپ);
    أرجع عدم;
  }
  إذا (mprotect(codeseg, rounded_ناپ, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_ناپ);
    munmap(dataseg, rounded_ناپ);
    أرجع عدم;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  حافظہ_نقل(dataseg, &rounded_ناپ, حجم(rounded_ناپ));
  حافظہ_نقل(جوڑو_منزل_اشارہ_گر(dataseg, حجم(size_t)), &codeseg, حجم(فراغ *));
  *کوڈ = جوڑو_منزل_اشارہ_گر(codeseg, overhead);
  أرجع جوڑو_منزل_اشارہ_گر(dataseg, overhead);
}

فراغ
ffi_گھیرا_free (فراغ *ptr)
{
  فراغ *codeseg, *dataseg;
  size_t rounded_ناپ;

  dataseg = جوڑو_منزل_اشارہ_گر(ptr, -overhead);
  حافظہ_نقل(&rounded_ناپ, dataseg, حجم(rounded_ناپ));
  حافظہ_نقل(&codeseg, جوڑو_منزل_اشارہ_گر(dataseg, حجم(size_t)), حجم(فراغ *));
  munmap(dataseg, rounded_ناپ);
  munmap(codeseg, rounded_ناپ);
}

صحيح
ffi_tramp_is_present (__attribute__((بن_ورتیا)) فراغ *ptr)
{
  أرجع 0;
}
#وإلا /* !NetBSD with PROT_MPROTECT */

#إذا !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_جدول
# إذا __linux__ && !defined(__ANDROID__)
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
# إذا defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#جے_انت

#إذا FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# إذا !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# إذا !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#جے_انت /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#إذا FFI_CLOSURES

#إذا FFI_EXEC_TRAMPOLINE_جدول

#جے_تعریف_شدہ __MACH__

#ضمن <mach/mach.h>
#ضمن <pthread.h>
#جے_تعریف_شدہ HAVE_ARM64E_PTRAUTH
#ضمن <ptrauth.h>
#جے_انت
#ضمن <stdio.h>
#ضمن <stdlib.h>

خارجي فراغ *ffi_گھیرا_trampoline_جدول_ورقہ;

تعريفنوع بنية ffi_trampoline_جدول ffi_trampoline_جدول;
تعريفنوع بنية ffi_trampoline_جدول_اندراج ffi_trampoline_جدول_اندراج;

بنية ffi_trampoline_جدول
{
  /* contiguous writable and executable pages */
  vm_پتہ_t config_ورقہ;

  /* free list tracking */
  uint16_t free_گنتی;
  ffi_trampoline_جدول_اندراج *free_list;
  ffi_trampoline_جدول_اندراج *free_list_pool;

  ffi_trampoline_جدول *prev;
  ffi_trampoline_جدول *اگلا;
};

بنية ffi_trampoline_جدول_اندراج
{
  فراغ *(*trampoline) (فراغ);
  ffi_trampoline_جدول_اندراج *اگلا;
};

/* Total number of trampolines that fit in one trampoline table */
#تعریف_کرو FFI_TRAMPOLINE_گنتی (ورقہ_MAX_ناپ / FFI_TRAMPOLINE_ناپ)

ساكن pthread_mutex_t ffi_trampoline_جندرا = PTHREAD_MUTEX_INITIALIZER;
ساكن ffi_trampoline_جدول *ffi_trampoline_tables = عدم;

ساكن ffi_trampoline_جدول *
ffi_trampoline_جدول_alloc (فراغ)
{
  ffi_trampoline_جدول *جدول;
  vm_پتہ_t config_ورقہ;
  vm_پتہ_t trampoline_ورقہ;
  vm_پتہ_t trampoline_ورقہ_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_ورقہ = 0x0;
  kt = vm_allocate (mach_task_self (), &config_ورقہ, ورقہ_MAX_ناپ * 2,
		    VM_FLAGS_ANYWHERE);
  إذا (kt != KERN_SUCCESS)
    أرجع عدم;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_ورقہ = config_ورقہ + ورقہ_MAX_ناپ;

#جے_تعریف_شدہ HAVE_ARM64E_PTRAUTH
  trampoline_ورقہ_template = (vm_پتہ_t)(uintptr_t)ptrauth_auth_انکڑے((فراغ *)&ffi_گھیرا_trampoline_جدول_ورقہ, ptrauth_کلید_function_اشارہ_گر, 0);
#وإلا
  trampoline_ورقہ_template = (vm_پتہ_t)&ffi_گھیرا_trampoline_جدول_ورقہ;
#جے_انت

#جے_تعریف_شدہ __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_ورقہ_template &= ~1UL;
#جے_انت
  kt = vm_remap (mach_task_self (), &trampoline_ورقہ, ورقہ_MAX_ناپ, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_ورقہ_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  إذا (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_ورقہ, ورقہ_MAX_ناپ * 2);
      أرجع عدم;
    }

  إذا (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_ورقہ, ورقہ_MAX_ناپ,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      إذا (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_ورقہ, ورقہ_MAX_ناپ * 2);
          أرجع عدم;
        }
    }

  /* We have valid trampoline and config pages */
  جدول = صفر_حافظہ_دیو (1, حجم (ffi_trampoline_جدول));
  إذا (جدول == عدم)
    {
      vm_deallocate (mach_task_self (), config_ورقہ, ورقہ_MAX_ناپ * 2);
      أرجع عدم;
    }
  جدول->free_گنتی = FFI_TRAMPOLINE_گنتی;
  جدول->config_ورقہ = config_ورقہ;

  /* Create and initialize the free list */
  جدول->free_list_pool =
    صفر_حافظہ_دیو (FFI_TRAMPOLINE_گنتی, حجم (ffi_trampoline_جدول_اندراج));
  إذا (جدول->free_list_pool == عدم)
    {
      vm_deallocate (mach_task_self (), config_ورقہ, ورقہ_MAX_ناپ * 2);
      حرر (جدول);
      أرجع عدم;
    }

  لكل (i = 0; i < جدول->free_گنتی; i++)
    {
      ffi_trampoline_جدول_اندراج *اندراج = &جدول->free_list_pool[i];
      اندراج->trampoline =
	(فراغ *) (trampoline_ورقہ + (i * FFI_TRAMPOLINE_ناپ));
#جے_تعریف_شدہ HAVE_ARM64E_PTRAUTH
      اندراج->trampoline = ptrauth_sign_unauthenticated(اندراج->trampoline, ptrauth_کلید_function_اشارہ_گر, 0);
#جے_انت

      إذا (i < جدول->free_گنتی - 1)
	اندراج->اگلا = &جدول->free_list_pool[i + 1];
    }

  جدول->free_list = جدول->free_list_pool;

  أرجع جدول;
}

ساكن فراغ
ffi_trampoline_جدول_free (ffi_trampoline_جدول *جدول)
{
  /* Remove from the list */
  إذا (جدول->prev != عدم)
    جدول->prev->اگلا = جدول->اگلا;

  إذا (جدول->اگلا != عدم)
    جدول->اگلا->prev = جدول->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), جدول->config_ورقہ, ورقہ_MAX_ناپ * 2);

  /* Deallocate free list */
  حرر (جدول->free_list_pool);
  حرر (جدول);
}

فراغ *
ffi_گھیرا_alloc (size_t ناپ, فراغ **کوڈ)
{
  /* Create the closure */
  ffi_گھیرا *گھیرا = حافظہ_دیو (ناپ);
  إذا (گھیرا == عدم)
    أرجع عدم;

  pthread_mutex_جندرا (&ffi_trampoline_جندرا);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_جدول *جدول = ffi_trampoline_tables;
  إذا (جدول == عدم || جدول->free_list == عدم)
    {
      جدول = ffi_trampoline_جدول_alloc ();
      إذا (جدول == عدم)
	{
	  pthread_mutex_unlock (&ffi_trampoline_جندرا);
	  حرر (گھیرا);
	  أرجع عدم;
	}

      /* Insert the new table at the top of the list */
      جدول->اگلا = ffi_trampoline_tables;
      إذا (جدول->اگلا != عدم)
	جدول->اگلا->prev = جدول;

      ffi_trampoline_tables = جدول;
    }

  /* Claim the free entry */
  ffi_trampoline_جدول_اندراج *اندراج = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = اندراج->اگلا;
  ffi_trampoline_tables->free_گنتی--;
  اندراج->اگلا = عدم;

  pthread_mutex_unlock (&ffi_trampoline_جندرا);

  /* Initialize the return values */
  *کوڈ = اندراج->trampoline;
  گھیرا->trampoline_جدول = جدول;
  گھیرا->trampoline_جدول_اندراج = اندراج;

  أرجع گھیرا;
}

فراغ
ffi_گھیرا_free (فراغ *ptr)
{
  ffi_گھیرا *گھیرا = ptr;

  pthread_mutex_جندرا (&ffi_trampoline_جندرا);

  /* Fetch the table and entry references */
  ffi_trampoline_جدول *جدول = گھیرا->trampoline_جدول;
  ffi_trampoline_جدول_اندراج *اندراج = گھیرا->trampoline_جدول_اندراج;

  /* Return the entry to the free list */
  اندراج->اگلا = جدول->free_list;
  جدول->free_list = اندراج;
  جدول->free_گنتی++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  إذا (جدول->free_گنتی == FFI_TRAMPOLINE_گنتی
      && ffi_trampoline_tables != جدول)
    {
      ffi_trampoline_جدول_free (جدول);
    }
  وإلا إذا (ffi_trampoline_tables != جدول)
    {
      /* Otherwise, bump this table to the top of the list */
      جدول->prev = عدم;
      جدول->اگلا = ffi_trampoline_tables;
      إذا (ffi_trampoline_tables != عدم)
	ffi_trampoline_tables->prev = جدول;

      ffi_trampoline_tables = جدول;
    }

  pthread_mutex_unlock (&ffi_trampoline_جندرا);

  /* Free the closure */
  حرر (گھیرا);
}

#جے_انت

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#وإلاإذا FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#تعریف_کرو USE_LOCKS 1
#تعریف_کرو USE_DL_اگیتر 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#تعریف_کرو HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#تعریف_کرو HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#تعریف_کرو NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#تعریف_کرو DEFAULT_MMAP_حد MAX_ناپ_T

/* Don't allocate more than a page unless needed.  */
#تعریف_کرو DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <fcntl.h>
#ضمن <errno.h>
#جے_بنا_تعریف _MSC_VER
#ضمن <unistd.h>
#جے_انت
#ضمن <string.h>
#ضمن <stdio.h>
#إذا !defined(_WIN32)
#جے_تعریف_شدہ HAVE_MNTENT
#ضمن <mntent.h>
#جے_انت /* HAVE_MNTENT */
#ضمن <sys/param.h>
#ضمن <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ضمن <sys/mman.h>
#تعریف_کرو LACKS_SYS_MMAN_H 1

#إذا FFI_MMAP_EXEC_SELINUX
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

  إذا (statfs ("/selinux", &sfs) >= 0
      && (بنانشانی صحيح) sfs.f_type == 0xf97cff8cU)
    أرجع 1;
  f = fopen ("/proc/mounts", "r");
  إذا (f == عدم)
    أرجع 0;
  طالما (getline (&buf, &len, f) >= 0)
    {
      حرف *p = strchr (buf, ' ');
      إذا (p == عدم)
        اكسر;
      p = strchr (p + 1, ' ');
      إذا (p == عدم)
        اكسر;
      إذا (لڑی_مقابلہ (p + 1, "selinuxfs ", 10) == 0)
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

#تعریف_کرو is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#وإلا

#تعریف_کرو is_selinux_enabled() 0

#جے_انت /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#إذا defined FFI_MMAP_PAX
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
  إذا (f == عدم)
    أرجع 0;
  ret = 0;

  طالما (getline (&buf, &len, f) != -1)
    إذا (!لڑی_مقابلہ (buf, "PaX:", 4))
      {
        إذا (عدم != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        إذا (عدم != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        اكسر;
      }
  حرر (buf);
  fclose (f);
  أرجع ret;
}

#تعریف_کرو get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#تعریف_کرو has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#تعریف_کرو is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#تعریف_کرو is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#جے_انت /* defined FFI_MMAP_PAX */

#وإلاإذا defined (__CYGWIN__) || defined(__INTERIX)

#ضمن <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#تعریف_کرو is_selinux_enabled() 0

#جے_انت /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#إذا !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#جے_انت /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ساكن فراغ *dlmalloc(size_t);
ساكن فراغ dlfree(فراغ*);
ساكن فراغ *dlcalloc(size_t, size_t) MAYBE_بن_ورتیا;
ساكن فراغ *dlrealloc(فراغ *, size_t) MAYBE_بن_ورتیا;
ساكن فراغ *dlmemalign(size_t, size_t) MAYBE_بن_ورتیا;
ساكن فراغ *dlvalloc(size_t) MAYBE_بن_ورتیا;
ساكن صحيح dlmallopt(صحيح, صحيح) MAYBE_بن_ورتیا;
ساكن size_t dlmalloc_footprint(فراغ) MAYBE_بن_ورتیا;
ساكن size_t dlmalloc_max_footprint(فراغ) MAYBE_بن_ورتیا;
ساكن فراغ** dlindependent_calloc(size_t, size_t, فراغ**) MAYBE_بن_ورتیا;
ساكن فراغ** dlindependent_comalloc(size_t, size_t*, فراغ**) MAYBE_بن_ورتیا;
ساكن فراغ *dlpvalloc(size_t) MAYBE_بن_ورتیا;
ساكن صحيح dlmalloc_trim(size_t) MAYBE_بن_ورتیا;
ساكن size_t dlmalloc_usable_ناپ(فراغ*) MAYBE_بن_ورتیا;
ساكن فراغ dlmalloc_stats(فراغ) MAYBE_بن_ورتیا;

#إذا !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ساكن فراغ *dlmmap(فراغ *, size_t, صحيح, صحيح, صحيح, off_t);
ساكن صحيح dlmunmap(فراغ *, size_t);
#جے_انت /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#تعریف_کرو mmap dlmmap
#تعریف_کرو munmap dlmunmap

#ضمن "dlmalloc.c"

#تعریف_رد mmap
#تعریف_رد munmap

#إذا !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ساكن pthread_mutex_t open_عارضی_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ساكن صحيح execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ساكن size_t execsize = 0;

#جے_تعریف_شدہ HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_عارضی_exec_file_memfd (ثابت حرف *name)
{
  صحيح fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  أرجع fd;
}
#جے_انت

/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_عارضی_exec_file_name (حرف *name, صحيح flags MAYBE_بن_ورتیا)
{
  صحيح fd;

#جے_تعریف_شدہ HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#وإلا
  fd = mkstemp (name);
#جے_انت

  إذا (fd != -1)
    unlink (name);

  أرجع fd;
}

/* Open a temporary file in the named directory.  */
ساكن صحيح
open_عارضی_exec_file_dir (ثابت حرف *dir)
{
  ساكن ثابت حرف suffix[] = "/ffiXXXXXX";
  صحيح lendir, flags;
  حرف *tempname;
#جے_تعریف_شدہ O_TMPFILE
  صحيح fd;
#جے_انت

#جے_تعریف_شدہ O_CLOEXEC
  flags = O_CLOEXEC;
#وإلا
  flags = 0;
#جے_انت

#جے_تعریف_شدہ O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  إذا (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    أرجع fd;
  } وإلا {
    errno = 0;
  }
#جے_انت

  lendir = (صحيح) لڑی_لمبائی (dir);
  tempname = __builtin_alloca (lendir + حجم (suffix));

  إذا (!tempname)
    أرجع -1;

  حافظہ_نقل (tempname, dir, lendir);
  حافظہ_نقل (tempname + lendir, suffix, حجم (suffix));

  أرجع open_عارضی_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ساكن صحيح
open_عارضی_exec_file_env (ثابت حرف *envvar)
{
  ثابت حرف *قدر = getenv (envvar);

  إذا (!قدر)
    أرجع -1;

  أرجع open_عارضی_exec_file_dir (قدر);
}

#جے_تعریف_شدہ HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ساكن صحيح
open_عارضی_exec_file_mnt (ثابت حرف *mounts)
{
  ساكن ثابت حرف *آخری_mounts;
  ساكن FILE *آخری_mntent;

  إذا (mounts != آخری_mounts)
    {
      إذا (آخری_mntent)
	endmntent (آخری_mntent);

      آخری_mounts = mounts;

      إذا (mounts)
	آخری_mntent = setmntent (mounts, "r");
      وإلا
	آخری_mntent = عدم;
    }

  إذا (!آخری_mntent)
    أرجع -1;

  لكل (;;)
    {
      صحيح fd;
      بنية mntent mnt;
      حرف buf[MAXPATHLEN * 3];

      إذا (getmntent_r (آخری_mntent, &mnt, buf, حجم (buf)) == عدم)
	أرجع -1;

      إذا (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	استمر;

      fd = open_عارضی_exec_file_dir (mnt.mnt_dir);

      إذا (fd != -1)
	أرجع fd;
    }
}
#جے_انت /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ساكن بنية
{
  صحيح (*func)(ثابت حرف *);
  ثابت حرف *arg;
  صحيح repeat;
} open_عارضی_exec_file_opts[] = {
#جے_تعریف_شدہ HAVE_MEMFD_CREATE
  { open_عارضی_exec_file_memfd, "libffi", 0 },
#جے_انت
  { open_عارضی_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_عارضی_exec_file_env, "TMPDIR", 0 },
  { open_عارضی_exec_file_dir, "/tmp", 0 },
  { open_عارضی_exec_file_dir, "/var/tmp", 0 },
  { open_عارضی_exec_file_dir, "/dev/shm", 0 },
  { open_عارضی_exec_file_env, "HOME", 0 },
#جے_تعریف_شدہ HAVE_MNTENT
  { open_عارضی_exec_file_mnt, "/etc/mtab", 1 },
  { open_عارضی_exec_file_mnt, "/proc/mounts", 1 },
#جے_انت /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ساكن صحيح open_عارضی_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ساكن صحيح
open_عارضی_exec_file_opts_اگلا (فراغ)
{
  إذا (open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat)
    open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].func (عدم);

  open_عارضی_exec_file_opts_idx++;
  إذا (open_عارضی_exec_file_opts_idx
      == (حجم (open_عارضی_exec_file_opts)
	  / حجم (*open_عارضی_exec_file_opts)))
    {
      open_عارضی_exec_file_opts_idx = 0;
      أرجع 1;
    }

  أرجع 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
صحيح
open_عارضی_exec_file (فراغ)
{
  صحيح fd;

  افعل
    {
      fd = open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].func
	(open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].arg);

      إذا (!open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  إذا (open_عارضی_exec_file_opts_اگلا ())
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
  ساكن لما ورقہ_ناپ;

  /* Obtain system page size. */
  إذا (!ورقہ_ناپ)
    ورقہ_ناپ = sysconf(_SC_PAGESIZE);
  إذا (ورقہ_ناپ <= 0)
    أرجع -1;

  بنانشانی حرف buf[ورقہ_ناپ];
  memset (buf, 0, ورقہ_ناپ);

  طالما (len > 0)
    {
      off_t منزل_write = (len < ورقہ_ناپ) ? len : ورقہ_ناپ;
      إذا (اكتب (fd, buf, منزل_write) < منزل_write)
        أرجع -1;
      len -= منزل_write;
    }

  أرجع 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ساكن فراغ *
dlmmap_locked (فراغ *شروع, size_t لمائی, صحيح prot, صحيح flags, off_t offset)
{
  فراغ *ptr;

  إذا (execfd == -1)
    {
      open_عارضی_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_عارضی_exec_file ();
      إذا (execfd == -1)
	أرجع MFAIL;
    }

  offset = execsize;

  إذا (allocate_space (execfd, لمائی))
    أرجع MFAIL;

  flags &= ~(MAP_نجی | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (عدم, لمائی, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  إذا (ptr == MFAIL)
    {
      إذا (!offset)
	{
	  close (execfd);
	  اذهب retry_open;
	}
      إذا (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      أرجع MFAIL;
    }
  وإلا إذا (!offset
	   && open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat)
    open_عارضی_exec_file_opts_اگلا ();

  شروع = mmap (شروع, لمائی, prot, flags, execfd, offset);

  إذا (شروع == MFAIL)
    {
      munmap (ptr, لمائی);
      إذا (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      أرجع شروع;
    }

  mmap_exec_وتھ ((حرف *)شروع, لمائی) = (حرف*)ptr - (حرف*)شروع;

  execsize += لمائی;

  أرجع شروع;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ساكن فراغ *
dlmmap (فراغ *شروع, size_t لمائی, صحيح prot,
	صحيح flags, صحيح fd, off_t offset)
{
  فراغ *ptr;

  أكد (شروع == عدم && لمائی % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_نجی | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  إذا (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (شروع, لمائی, prot & ~PROT_EXEC, flags, fd, offset);
      أرجع ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  إذا (execfd == -1 && is_mprotect_enabled ())
    {
#جے_تعریف_شدہ FFI_MMAP_EXEC_EMUTRAMP_PAX
      إذا (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (شروع, لمائی, prot & ~PROT_EXEC, flags, fd, offset);
          أرجع ptr;
        }
#جے_انت
      /* fallback to dlmmap_locked.  */
    }
  وإلا إذا (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (شروع, لمائی, prot | PROT_EXEC, flags, fd, offset);

      إذا (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	أرجع ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_جندرا (&open_عارضی_exec_file_mutex);
  ptr = dlmmap_locked (شروع, لمائی, prot, flags, offset);
  pthread_mutex_unlock (&open_عارضی_exec_file_mutex);

  أرجع ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ساكن صحيح
dlmunmap (فراغ *شروع, size_t لمائی)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, شروع);
  فراغ *کوڈ;

  إذا (seg && (کوڈ = جوڑو_segment_exec_وتھ (شروع, seg)) != شروع)
    {
      صحيح ret = munmap (کوڈ, لمائی);
      إذا (ret)
	أرجع ret;
    }

  أرجع munmap (شروع, لمائی);
}

#إذا FFI_گھیرا_FREE_کوڈ
/* Return segment holding given code address.  */
ساكن msegmentptr
segment_holding_کوڈ (mstate m, حرف* addr)
{
  msegmentptr sp = &m->seg;
  لكل (;;) {
    إذا (addr >= جوڑو_segment_exec_وتھ (sp->ادھار, sp)
	&& addr < جوڑو_segment_exec_وتھ (sp->ادھار, sp) + sp->ناپ)
      أرجع sp;
    إذا ((sp = sp->اگلا) == 0)
      أرجع 0;
  }
}
#جے_انت

#جے_انت /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
فراغ *
ffi_گھیرا_alloc (size_t ناپ, فراغ **کوڈ)
{
  فراغ *ptr, *ftramp;

  إذا (!کوڈ)
    أرجع عدم;

  ptr = dlmalloc (ناپ);

  إذا (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *کوڈ = FFI_FN (جوڑو_segment_exec_وتھ (ptr, seg));
      إذا (!ffi_tramp_is_supported ())
        أرجع ptr;

      ftramp = ffi_tramp_alloc (0);
      إذا (ftramp == عدم)
      {
        dlfree (ptr);
        أرجع عدم;
      }
      *کوڈ = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_گھیرا *) ptr)->ftramp = ftramp;
    }

  أرجع ptr;
}

فراغ *
ffi_انکڑے_منزل_کوڈ_اشارہ_گر (فراغ *انکڑے)
{
  msegmentptr seg = segment_holding (gm, انکڑے);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  إذا (seg)
    {
      إذا (!ffi_tramp_is_supported ())
        أرجع جوڑو_segment_exec_وتھ (انکڑے, seg);
      أرجع ffi_tramp_get_addr (((ffi_گھیرا *) انکڑے)->ftramp);
    }
  وإلا
    أرجع انکڑے;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
فراغ
ffi_گھیرا_free (فراغ *ptr)
{
#إذا FFI_گھیرا_FREE_کوڈ
  msegmentptr seg = segment_holding_کوڈ (gm, ptr);

  إذا (seg)
    ptr = sub_segment_exec_وتھ (ptr, seg);
#جے_انت
  إذا (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_گھیرا *) ptr)->ftramp);

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
ffi_گھیرا_alloc (size_t ناپ, فراغ **کوڈ)
{
  فراغ *c;

  إذا (!کوڈ)
    أرجع عدم;

  c = حافظہ_دیو (ناپ);
  *کوڈ = FFI_FN (c);
  أرجع c;
}

فراغ
ffi_گھیرا_free (فراغ *ptr)
{
  حرر (ptr);
}

فراغ *
ffi_انکڑے_منزل_کوڈ_اشارہ_گر (فراغ *انکڑے)
{
  أرجع انکڑے;
}

صحيح
ffi_tramp_is_present (__attribute__((بن_ورتیا)) فراغ *ptr)
{
  أرجع 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#جے_انت /* FFI_CLOSURES */

#جے_انت /* NetBSD with PROT_MPROTECT */
#جے_انت /* __EMSCRIPTEN__ */

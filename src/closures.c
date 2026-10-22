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

#إذا (defined __linux__ || defined __CYGWIN__) && !defined GNU_منبع
#تعریف_کن GNU_منبع 1
#پایان_اگر

#اگر_تعریف_نشده __EMSCRIPTEN__

#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <tramp.h>

#اگر_تعریف_شده __NetBSD__
#ضمن <sys/param.h>
#پایان_اگر

#إذا __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#ضمن <sys/mman.h>

#ضمن <stddef.h>
#ضمن <unistd.h>
#اگر_تعریف_شده  HAVE_SYS_MEMFD_H
#ضمن <sys/memfd.h>
#پایان_اگر

ساكن ثابت size_t overhead =
  (حجم(max_align_t) > حجم(فراغ *) + حجم(size_t)) ?
    حجم(max_align_t)
    : حجم(فراغ *) + حجم(size_t);

#تعریف_کن جمع_کن_مقصد_اشاره_گر(p, d) ((فراغ *)((uintptr_t)(p) + (d)))

فراغ *
ffi_بستگی_alloc (size_t اندازه, فراغ **کد)
{
  ساكن size_t صفحه_اندازه;
  size_t rounded_اندازه;
  فراغ *codeseg, *dataseg;
  صحيح prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  إذا (!کد)
    أرجع عدم;

  /* Obtain system page size. */
  إذا (!صفحه_اندازه)
    صفحه_اندازه = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_اندازه = (اندازه + overhead + صفحه_اندازه - 1) & ~(صفحه_اندازه - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(عدم, rounded_اندازه, prot, MAP_ANON | MAP_خصوصی, -1, 0);
  إذا (dataseg == MAP_FAILED)
    أرجع عدم;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_اندازه, عدم, rounded_اندازه, MAP_REMAPDUP);
  إذا (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_اندازه);
    أرجع عدم;
  }
  إذا (mprotect(codeseg, rounded_اندازه, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_اندازه);
    munmap(dataseg, rounded_اندازه);
    أرجع عدم;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  کپی_حافظه(dataseg, &rounded_اندازه, حجم(rounded_اندازه));
  کپی_حافظه(جمع_کن_مقصد_اشاره_گر(dataseg, حجم(size_t)), &codeseg, حجم(فراغ *));
  *کد = جمع_کن_مقصد_اشاره_گر(codeseg, overhead);
  أرجع جمع_کن_مقصد_اشاره_گر(dataseg, overhead);
}

فراغ
ffi_بستگی_free (فراغ *ptr)
{
  فراغ *codeseg, *dataseg;
  size_t rounded_اندازه;

  dataseg = جمع_کن_مقصد_اشاره_گر(ptr, -overhead);
  کپی_حافظه(&rounded_اندازه, dataseg, حجم(rounded_اندازه));
  کپی_حافظه(&codeseg, جمع_کن_مقصد_اشاره_گر(dataseg, حجم(size_t)), حجم(فراغ *));
  munmap(dataseg, rounded_اندازه);
  munmap(codeseg, rounded_اندازه);
}

صحيح
ffi_tramp_is_present (__attribute__((استفادهنشده)) فراغ *ptr)
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
#پایان_اگر

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
#پایان_اگر /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#إذا FFI_CLOSURES

#إذا FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شده __MACH__

#ضمن <mach/mach.h>
#ضمن <pthread.h>
#اگر_تعریف_شده HAVE_ARM64E_PTRAUTH
#ضمن <ptrauth.h>
#پایان_اگر
#ضمن <stdio.h>
#ضمن <stdlib.h>

خارجي فراغ *ffi_بستگی_trampoline_جدول_صفحه;

تعريفنوع بنية ffi_trampoline_جدول ffi_trampoline_جدول;
تعريفنوع بنية ffi_trampoline_جدول_مدخل ffi_trampoline_جدول_مدخل;

بنية ffi_trampoline_جدول
{
  /* contiguous writable and executable pages */
  vm_آدرس_t config_صفحه;

  /* free list tracking */
  uint16_t free_شمار;
  ffi_trampoline_جدول_مدخل *free_list;
  ffi_trampoline_جدول_مدخل *free_list_pool;

  ffi_trampoline_جدول *prev;
  ffi_trampoline_جدول *بعدی;
};

بنية ffi_trampoline_جدول_مدخل
{
  فراغ *(*trampoline) (فراغ);
  ffi_trampoline_جدول_مدخل *بعدی;
};

/* Total number of trampolines that fit in one trampoline table */
#تعریف_کن FFI_TRAMPOLINE_شمار (صفحه_MAX_اندازه / FFI_TRAMPOLINE_اندازه)

ساكن pthread_mutex_t ffi_trampoline_قفل = PTHREAD_MUTEX_INITIALIZER;
ساكن ffi_trampoline_جدول *ffi_trampoline_tables = عدم;

ساكن ffi_trampoline_جدول *
ffi_trampoline_جدول_alloc (فراغ)
{
  ffi_trampoline_جدول *جدول;
  vm_آدرس_t config_صفحه;
  vm_آدرس_t trampoline_صفحه;
  vm_آدرس_t trampoline_صفحه_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_صفحه = 0x0;
  kt = vm_allocate (mach_task_self (), &config_صفحه, صفحه_MAX_اندازه * 2,
		    VM_FLAGS_ANYWHERE);
  إذا (kt != KERN_SUCCESS)
    أرجع عدم;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_صفحه = config_صفحه + صفحه_MAX_اندازه;

#اگر_تعریف_شده HAVE_ARM64E_PTRAUTH
  trampoline_صفحه_template = (vm_آدرس_t)(uintptr_t)ptrauth_auth_داده((فراغ *)&ffi_بستگی_trampoline_جدول_صفحه, ptrauth_کلید_function_اشاره_گر, 0);
#وإلا
  trampoline_صفحه_template = (vm_آدرس_t)&ffi_بستگی_trampoline_جدول_صفحه;
#پایان_اگر

#اگر_تعریف_شده __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_صفحه_template &= ~1UL;
#پایان_اگر
  kt = vm_remap (mach_task_self (), &trampoline_صفحه, صفحه_MAX_اندازه, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_صفحه_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  إذا (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_صفحه, صفحه_MAX_اندازه * 2);
      أرجع عدم;
    }

  إذا (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_صفحه, صفحه_MAX_اندازه,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      إذا (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_صفحه, صفحه_MAX_اندازه * 2);
          أرجع عدم;
        }
    }

  /* We have valid trampoline and config pages */
  جدول = حافظه_صفر_بده (1, حجم (ffi_trampoline_جدول));
  إذا (جدول == عدم)
    {
      vm_deallocate (mach_task_self (), config_صفحه, صفحه_MAX_اندازه * 2);
      أرجع عدم;
    }
  جدول->free_شمار = FFI_TRAMPOLINE_شمار;
  جدول->config_صفحه = config_صفحه;

  /* Create and initialize the free list */
  جدول->free_list_pool =
    حافظه_صفر_بده (FFI_TRAMPOLINE_شمار, حجم (ffi_trampoline_جدول_مدخل));
  إذا (جدول->free_list_pool == عدم)
    {
      vm_deallocate (mach_task_self (), config_صفحه, صفحه_MAX_اندازه * 2);
      حرر (جدول);
      أرجع عدم;
    }

  لكل (i = 0; i < جدول->free_شمار; i++)
    {
      ffi_trampoline_جدول_مدخل *مدخل = &جدول->free_list_pool[i];
      مدخل->trampoline =
	(فراغ *) (trampoline_صفحه + (i * FFI_TRAMPOLINE_اندازه));
#اگر_تعریف_شده HAVE_ARM64E_PTRAUTH
      مدخل->trampoline = ptrauth_sign_unauthenticated(مدخل->trampoline, ptrauth_کلید_function_اشاره_گر, 0);
#پایان_اگر

      إذا (i < جدول->free_شمار - 1)
	مدخل->بعدی = &جدول->free_list_pool[i + 1];
    }

  جدول->free_list = جدول->free_list_pool;

  أرجع جدول;
}

ساكن فراغ
ffi_trampoline_جدول_free (ffi_trampoline_جدول *جدول)
{
  /* Remove from the list */
  إذا (جدول->prev != عدم)
    جدول->prev->بعدی = جدول->بعدی;

  إذا (جدول->بعدی != عدم)
    جدول->بعدی->prev = جدول->prev;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), جدول->config_صفحه, صفحه_MAX_اندازه * 2);

  /* Deallocate free list */
  حرر (جدول->free_list_pool);
  حرر (جدول);
}

فراغ *
ffi_بستگی_alloc (size_t اندازه, فراغ **کد)
{
  /* Create the closure */
  ffi_بستگی *بستگی = حافظه_بده (اندازه);
  إذا (بستگی == عدم)
    أرجع عدم;

  pthread_mutex_قفل (&ffi_trampoline_قفل);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_جدول *جدول = ffi_trampoline_tables;
  إذا (جدول == عدم || جدول->free_list == عدم)
    {
      جدول = ffi_trampoline_جدول_alloc ();
      إذا (جدول == عدم)
	{
	  pthread_mutex_unlock (&ffi_trampoline_قفل);
	  حرر (بستگی);
	  أرجع عدم;
	}

      /* Insert the new table at the top of the list */
      جدول->بعدی = ffi_trampoline_tables;
      إذا (جدول->بعدی != عدم)
	جدول->بعدی->prev = جدول;

      ffi_trampoline_tables = جدول;
    }

  /* Claim the free entry */
  ffi_trampoline_جدول_مدخل *مدخل = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = مدخل->بعدی;
  ffi_trampoline_tables->free_شمار--;
  مدخل->بعدی = عدم;

  pthread_mutex_unlock (&ffi_trampoline_قفل);

  /* Initialize the return values */
  *کد = مدخل->trampoline;
  بستگی->trampoline_جدول = جدول;
  بستگی->trampoline_جدول_مدخل = مدخل;

  أرجع بستگی;
}

فراغ
ffi_بستگی_free (فراغ *ptr)
{
  ffi_بستگی *بستگی = ptr;

  pthread_mutex_قفل (&ffi_trampoline_قفل);

  /* Fetch the table and entry references */
  ffi_trampoline_جدول *جدول = بستگی->trampoline_جدول;
  ffi_trampoline_جدول_مدخل *مدخل = بستگی->trampoline_جدول_مدخل;

  /* Return the entry to the free list */
  مدخل->بعدی = جدول->free_list;
  جدول->free_list = مدخل;
  جدول->free_شمار++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  إذا (جدول->free_شمار == FFI_TRAMPOLINE_شمار
      && ffi_trampoline_tables != جدول)
    {
      ffi_trampoline_جدول_free (جدول);
    }
  وإلا إذا (ffi_trampoline_tables != جدول)
    {
      /* Otherwise, bump this table to the top of the list */
      جدول->prev = عدم;
      جدول->بعدی = ffi_trampoline_tables;
      إذا (ffi_trampoline_tables != عدم)
	ffi_trampoline_tables->prev = جدول;

      ffi_trampoline_tables = جدول;
    }

  pthread_mutex_unlock (&ffi_trampoline_قفل);

  /* Free the closure */
  حرر (بستگی);
}

#پایان_اگر

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#وإلاإذا FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#تعریف_کن USE_LOCKS 1
#تعریف_کن USE_DL_پیشوند 1
/* dlmalloc's bit-index macros use __builtin_clz/__builtin_ctz directly on any
   GNU-compatible compiler, so USE_BUILTIN_FFS (and pulling in <strings.h> for
   ffs()) is no longer needed (libffi #754).  */

/* We need to use mmap, not sbrk.  */
#تعریف_کن HAVE_MORECORE 0

/* We could, in theory, support mremap, but it wouldn't buy us anything.  */
#تعریف_کن HAVE_MREMAP 0

/* We have no use for this, so save some code and data.  */
#تعریف_کن NO_MALLINFO 1

/* We need all allocations to be in regular segments, otherwise we
   lose track of the corresponding code address.  */
#تعریف_کن DEFAULT_MMAP_آستانه MAX_اندازه_T

/* Don't allocate more than a page unless needed.  */
#تعریف_کن DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <fcntl.h>
#ضمن <errno.h>
#اگر_تعریف_نشده _MSC_VER
#ضمن <unistd.h>
#پایان_اگر
#ضمن <string.h>
#ضمن <stdio.h>
#إذا !defined(_WIN32)
#اگر_تعریف_شده HAVE_MNTENT
#ضمن <mntent.h>
#پایان_اگر /* HAVE_MNTENT */
#ضمن <sys/param.h>
#ضمن <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#ضمن <sys/mman.h>
#تعریف_کن LACKS_SYS_MMAN_H 1

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
      && (بیعلامت صحيح) sfs.f_type == 0xf97cff8cU)
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
      إذا (مقایسه_رشته (p + 1, "selinuxfs ", 10) == 0)
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

#تعریف_کن is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#وإلا

#تعریف_کن is_selinux_enabled() 0

#پایان_اگر /* !FFI_MMAP_EXEC_SELINUX */

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
    إذا (!مقایسه_رشته (buf, "PaX:", 4))
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

#تعریف_کن get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#تعریف_کن has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#تعریف_کن is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#تعریف_کن is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#پایان_اگر /* defined FFI_MMAP_PAX */

#وإلاإذا defined (__CYGWIN__) || defined(__INTERIX)

#ضمن <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#تعریف_کن is_selinux_enabled() 0

#پایان_اگر /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#إذا !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#پایان_اگر /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ساكن فراغ *dlmalloc(size_t);
ساكن فراغ dlfree(فراغ*);
ساكن فراغ *dlcalloc(size_t, size_t) MAYBE_استفادهنشده;
ساكن فراغ *dlrealloc(فراغ *, size_t) MAYBE_استفادهنشده;
ساكن فراغ *dlmemalign(size_t, size_t) MAYBE_استفادهنشده;
ساكن فراغ *dlvalloc(size_t) MAYBE_استفادهنشده;
ساكن صحيح dlmallopt(صحيح, صحيح) MAYBE_استفادهنشده;
ساكن size_t dlmalloc_footprint(فراغ) MAYBE_استفادهنشده;
ساكن size_t dlmalloc_max_footprint(فراغ) MAYBE_استفادهنشده;
ساكن فراغ** dlindependent_calloc(size_t, size_t, فراغ**) MAYBE_استفادهنشده;
ساكن فراغ** dlindependent_comalloc(size_t, size_t*, فراغ**) MAYBE_استفادهنشده;
ساكن فراغ *dlpvalloc(size_t) MAYBE_استفادهنشده;
ساكن صحيح dlmalloc_trim(size_t) MAYBE_استفادهنشده;
ساكن size_t dlmalloc_usable_اندازه(فراغ*) MAYBE_استفادهنشده;
ساكن فراغ dlmalloc_stats(فراغ) MAYBE_استفادهنشده;

#إذا !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ساكن فراغ *dlmmap(فراغ *, size_t, صحيح, صحيح, صحيح, off_t);
ساكن صحيح dlmunmap(فراغ *, size_t);
#پایان_اگر /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#تعریف_کن mmap dlmmap
#تعریف_کن munmap dlmunmap

#ضمن "dlmalloc.c"

#لغو_تعریف mmap
#لغو_تعریف munmap

#إذا !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ساكن pthread_mutex_t open_موقت_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ساكن صحيح execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ساكن size_t execsize = 0;

#اگر_تعریف_شده HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_موقت_exec_file_memfd (ثابت حرف *name)
{
  صحيح fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  أرجع fd;
}
#پایان_اگر

/* Open a temporary file name, and immediately unlink it.  */
ساكن صحيح
open_موقت_exec_file_name (حرف *name, صحيح flags MAYBE_استفادهنشده)
{
  صحيح fd;

#اگر_تعریف_شده HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#وإلا
  fd = mkstemp (name);
#پایان_اگر

  إذا (fd != -1)
    unlink (name);

  أرجع fd;
}

/* Open a temporary file in the named directory.  */
ساكن صحيح
open_موقت_exec_file_dir (ثابت حرف *dir)
{
  ساكن ثابت حرف suffix[] = "/ffiXXXXXX";
  صحيح lendir, flags;
  حرف *tempname;
#اگر_تعریف_شده O_TMPFILE
  صحيح fd;
#پایان_اگر

#اگر_تعریف_شده O_CLOEXEC
  flags = O_CLOEXEC;
#وإلا
  flags = 0;
#پایان_اگر

#اگر_تعریف_شده O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  إذا (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    أرجع fd;
  } وإلا {
    errno = 0;
  }
#پایان_اگر

  lendir = (صحيح) طول_رشته (dir);
  tempname = __builtin_alloca (lendir + حجم (suffix));

  إذا (!tempname)
    أرجع -1;

  کپی_حافظه (tempname, dir, lendir);
  کپی_حافظه (tempname + lendir, suffix, حجم (suffix));

  أرجع open_موقت_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ساكن صحيح
open_موقت_exec_file_env (ثابت حرف *envvar)
{
  ثابت حرف *مقدار = getenv (envvar);

  إذا (!مقدار)
    أرجع -1;

  أرجع open_موقت_exec_file_dir (مقدار);
}

#اگر_تعریف_شده HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ساكن صحيح
open_موقت_exec_file_mnt (ثابت حرف *mounts)
{
  ساكن ثابت حرف *آخرین_mounts;
  ساكن FILE *آخرین_mntent;

  إذا (mounts != آخرین_mounts)
    {
      إذا (آخرین_mntent)
	endmntent (آخرین_mntent);

      آخرین_mounts = mounts;

      إذا (mounts)
	آخرین_mntent = setmntent (mounts, "r");
      وإلا
	آخرین_mntent = عدم;
    }

  إذا (!آخرین_mntent)
    أرجع -1;

  لكل (;;)
    {
      صحيح fd;
      بنية mntent mnt;
      حرف buf[MAXPATHLEN * 3];

      إذا (getmntent_r (آخرین_mntent, &mnt, buf, حجم (buf)) == عدم)
	أرجع -1;

      إذا (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	استمر;

      fd = open_موقت_exec_file_dir (mnt.mnt_dir);

      إذا (fd != -1)
	أرجع fd;
    }
}
#پایان_اگر /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ساكن بنية
{
  صحيح (*func)(ثابت حرف *);
  ثابت حرف *arg;
  صحيح repeat;
} open_موقت_exec_file_opts[] = {
#اگر_تعریف_شده HAVE_MEMFD_CREATE
  { open_موقت_exec_file_memfd, "libffi", 0 },
#پایان_اگر
  { open_موقت_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_موقت_exec_file_env, "TMPDIR", 0 },
  { open_موقت_exec_file_dir, "/tmp", 0 },
  { open_موقت_exec_file_dir, "/var/tmp", 0 },
  { open_موقت_exec_file_dir, "/dev/shm", 0 },
  { open_موقت_exec_file_env, "HOME", 0 },
#اگر_تعریف_شده HAVE_MNTENT
  { open_موقت_exec_file_mnt, "/etc/mtab", 1 },
  { open_موقت_exec_file_mnt, "/proc/mounts", 1 },
#پایان_اگر /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ساكن صحيح open_موقت_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ساكن صحيح
open_موقت_exec_file_opts_بعدی (فراغ)
{
  إذا (open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].repeat)
    open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].func (عدم);

  open_موقت_exec_file_opts_idx++;
  إذا (open_موقت_exec_file_opts_idx
      == (حجم (open_موقت_exec_file_opts)
	  / حجم (*open_موقت_exec_file_opts)))
    {
      open_موقت_exec_file_opts_idx = 0;
      أرجع 1;
    }

  أرجع 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
صحيح
open_موقت_exec_file (فراغ)
{
  صحيح fd;

  افعل
    {
      fd = open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].func
	(open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].arg);

      إذا (!open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  إذا (open_موقت_exec_file_opts_بعدی ())
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
  ساكن دراز صفحه_اندازه;

  /* Obtain system page size. */
  إذا (!صفحه_اندازه)
    صفحه_اندازه = sysconf(_SC_PAGESIZE);
  إذا (صفحه_اندازه <= 0)
    أرجع -1;

  بیعلامت حرف buf[صفحه_اندازه];
  memset (buf, 0, صفحه_اندازه);

  طالما (len > 0)
    {
      off_t مقصد_write = (len < صفحه_اندازه) ? len : صفحه_اندازه;
      إذا (اكتب (fd, buf, مقصد_write) < مقصد_write)
        أرجع -1;
      len -= مقصد_write;
    }

  أرجع 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ساكن فراغ *
dlmmap_locked (فراغ *شروع, size_t درازی, صحيح prot, صحيح flags, off_t offset)
{
  فراغ *ptr;

  إذا (execfd == -1)
    {
      open_موقت_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_موقت_exec_file ();
      إذا (execfd == -1)
	أرجع MFAIL;
    }

  offset = execsize;

  إذا (allocate_space (execfd, درازی))
    أرجع MFAIL;

  flags &= ~(MAP_خصوصی | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (عدم, درازی, (prot & ~PROT_WRITE) | PROT_EXEC,
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
	   && open_موقت_exec_file_opts[open_موقت_exec_file_opts_idx].repeat)
    open_موقت_exec_file_opts_بعدی ();

  شروع = mmap (شروع, درازی, prot, flags, execfd, offset);

  إذا (شروع == MFAIL)
    {
      munmap (ptr, درازی);
      إذا (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      أرجع شروع;
    }

  mmap_exec_جابه_جایی ((حرف *)شروع, درازی) = (حرف*)ptr - (حرف*)شروع;

  execsize += درازی;

  أرجع شروع;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ساكن فراغ *
dlmmap (فراغ *شروع, size_t درازی, صحيح prot,
	صحيح flags, صحيح fd, off_t offset)
{
  فراغ *ptr;

  أكد (شروع == عدم && درازی % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_خصوصی | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  إذا (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (شروع, درازی, prot & ~PROT_EXEC, flags, fd, offset);
      أرجع ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  إذا (execfd == -1 && is_mprotect_enabled ())
    {
#اگر_تعریف_شده FFI_MMAP_EXEC_EMUTRAMP_PAX
      إذا (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (شروع, درازی, prot & ~PROT_EXEC, flags, fd, offset);
          أرجع ptr;
        }
#پایان_اگر
      /* fallback to dlmmap_locked.  */
    }
  وإلا إذا (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (شروع, درازی, prot | PROT_EXEC, flags, fd, offset);

      إذا (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	أرجع ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_قفل (&open_موقت_exec_file_mutex);
  ptr = dlmmap_locked (شروع, درازی, prot, flags, offset);
  pthread_mutex_unlock (&open_موقت_exec_file_mutex);

  أرجع ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ساكن صحيح
dlmunmap (فراغ *شروع, size_t درازی)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, شروع);
  فراغ *کد;

  إذا (seg && (کد = جمع_کن_segment_exec_جابه_جایی (شروع, seg)) != شروع)
    {
      صحيح ret = munmap (کد, درازی);
      إذا (ret)
	أرجع ret;
    }

  أرجع munmap (شروع, درازی);
}

#إذا FFI_بستگی_FREE_کد
/* Return segment holding given code address.  */
ساكن msegmentptr
segment_holding_کد (mstate m, حرف* addr)
{
  msegmentptr sp = &m->seg;
  لكل (;;) {
    إذا (addr >= جمع_کن_segment_exec_جابه_جایی (sp->پایه, sp)
	&& addr < جمع_کن_segment_exec_جابه_جایی (sp->پایه, sp) + sp->اندازه)
      أرجع sp;
    إذا ((sp = sp->بعدی) == 0)
      أرجع 0;
  }
}
#پایان_اگر

#پایان_اگر /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
فراغ *
ffi_بستگی_alloc (size_t اندازه, فراغ **کد)
{
  فراغ *ptr, *ftramp;

  إذا (!کد)
    أرجع عدم;

  ptr = dlmalloc (اندازه);

  إذا (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *کد = FFI_FN (جمع_کن_segment_exec_جابه_جایی (ptr, seg));
      إذا (!ffi_tramp_is_supported ())
        أرجع ptr;

      ftramp = ffi_tramp_alloc (0);
      إذا (ftramp == عدم)
      {
        dlfree (ptr);
        أرجع عدم;
      }
      *کد = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_بستگی *) ptr)->ftramp = ftramp;
    }

  أرجع ptr;
}

فراغ *
ffi_داده_مقصد_کد_اشاره_گر (فراغ *داده)
{
  msegmentptr seg = segment_holding (gm, داده);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  إذا (seg)
    {
      إذا (!ffi_tramp_is_supported ())
        أرجع جمع_کن_segment_exec_جابه_جایی (داده, seg);
      أرجع ffi_tramp_get_addr (((ffi_بستگی *) داده)->ftramp);
    }
  وإلا
    أرجع داده;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
فراغ
ffi_بستگی_free (فراغ *ptr)
{
#إذا FFI_بستگی_FREE_کد
  msegmentptr seg = segment_holding_کد (gm, ptr);

  إذا (seg)
    ptr = sub_segment_exec_جابه_جایی (ptr, seg);
#پایان_اگر
  إذا (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_بستگی *) ptr)->ftramp);

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
ffi_بستگی_alloc (size_t اندازه, فراغ **کد)
{
  فراغ *c;

  إذا (!کد)
    أرجع عدم;

  c = حافظه_بده (اندازه);
  *کد = FFI_FN (c);
  أرجع c;
}

فراغ
ffi_بستگی_free (فراغ *ptr)
{
  حرر (ptr);
}

فراغ *
ffi_داده_مقصد_کد_اشاره_گر (فراغ *داده)
{
  أرجع داده;
}

صحيح
ffi_tramp_is_present (__attribute__((استفادهنشده)) فراغ *ptr)
{
  أرجع 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#پایان_اگر /* FFI_CLOSURES */

#پایان_اگر /* NetBSD with PROT_MPROTECT */
#پایان_اگر /* __EMSCRIPTEN__ */

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

#اگر (defined __linux__ || defined __CYGWIN__) && !defined GNU_ماخذ
#تعریف_کرو GNU_ماخذ 1
#اگر_ختم

#اگر_غیر_تعریف_شدہ __EMSCRIPTEN__

#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <tramp.h>

#اگر_تعریف_شدہ __NetBSD__
#شامل <sys/param.h>
#اگر_ختم

#اگر __NetBSD_Version__ - 0 >= 799007200
/* NetBSD with PROT_MPROTECT */
#شامل <sys/mman.h>

#شامل <stddef.h>
#شامل <unistd.h>
#اگر_تعریف_شدہ  HAVE_SYS_MEMFD_H
#شامل <sys/memfd.h>
#اگر_ختم

ساکن ثابت size_t overhead =
  (حجم(max_align_t) > حجم(خالی *) + حجم(size_t)) ?
    حجم(max_align_t)
    : حجم(خالی *) + حجم(size_t);

#تعریف_کرو جوڑو_مقصد_اشارہ_گر(p, d) ((خالی *)((uintptr_t)(p) + (d)))

خالی *
ffi_احاطہ_alloc (size_t سائز, خالی **رمز)
{
  ساکن size_t صفحہ_سائز;
  size_t rounded_سائز;
  خالی *codeseg, *dataseg;
  صحیح prot;

  /* Expect that PAX mprotect is active and a separate code mapping is necessary. */
  اگر (!رمز)
    واپس تہی;

  /* Obtain system page size. */
  اگر (!صفحہ_سائز)
    صفحہ_سائز = sysconf(_SC_PAGESIZE);

  /* Round allocation size up to the next page, keeping in mind the size field and pointer to code map. */
  rounded_سائز = (سائز + overhead + صفحہ_سائز - 1) & ~(صفحہ_سائز - 1);

  /* Primary mapping is RW, but request permission to switch to PROT_EXEC later. */
  prot = PROT_READ | PROT_WRITE | PROT_MPROTECT(PROT_EXEC);
  dataseg = mmap(تہی, rounded_سائز, prot, MAP_ANON | MAP_نجی, -1, 0);
  اگر (dataseg == MAP_FAILED)
    واپس تہی;

  /* Create secondary mapping and switch it to RX. */
  codeseg = mremap(dataseg, rounded_سائز, تہی, rounded_سائز, MAP_REMAPDUP);
  اگر (codeseg == MAP_FAILED) {
    munmap(dataseg, rounded_سائز);
    واپس تہی;
  }
  اگر (mprotect(codeseg, rounded_سائز, PROT_READ | PROT_EXEC) == -1) {
    munmap(codeseg, rounded_سائز);
    munmap(dataseg, rounded_سائز);
    واپس تہی;
  }

  /* Remember allocation size and location of the secondary mapping for ffi_closure_free. */
  حافظہ_نقل(dataseg, &rounded_سائز, حجم(rounded_سائز));
  حافظہ_نقل(جوڑو_مقصد_اشارہ_گر(dataseg, حجم(size_t)), &codeseg, حجم(خالی *));
  *رمز = جوڑو_مقصد_اشارہ_گر(codeseg, overhead);
  واپس جوڑو_مقصد_اشارہ_گر(dataseg, overhead);
}

خالی
ffi_احاطہ_free (خالی *ptr)
{
  خالی *codeseg, *dataseg;
  size_t rounded_سائز;

  dataseg = جوڑو_مقصد_اشارہ_گر(ptr, -overhead);
  حافظہ_نقل(&rounded_سائز, dataseg, حجم(rounded_سائز));
  حافظہ_نقل(&codeseg, جوڑو_مقصد_اشارہ_گر(dataseg, حجم(size_t)), حجم(خالی *));
  munmap(dataseg, rounded_سائز);
  munmap(codeseg, rounded_سائز);
}

صحیح
ffi_tramp_is_present (__attribute__((غیر_مستعمل)) خالی *ptr)
{
  واپس 0;
}
#ورنہ /* !NetBSD with PROT_MPROTECT */

#اگر !FFI_MMAP_EXEC_WRIT && !FFI_EXEC_TRAMPOLINE_جدول
# اگر __linux__ && !defined(__ANDROID__)
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
# اگر defined(__CYGWIN__) || defined(_WIN32) || defined(__OS2__)
/* Windows systems may have Data Execution Protection (DEP) enabled,
   which requires the use of VirtualMalloc/VirtualFree to alloc/free
   executable memory. */
#  define FFI_MMAP_EXEC_WRIT 1
# endif
#اگر_ختم

#اگر FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__)
# اگر !defined FFI_MMAP_EXEC_SELINUX
/* When defined to 1 check for SELinux and if SELinux is active,
   don't attempt PROT_EXEC|PROT_WRITE mapping at all, as that
   might cause audit messages.  */
#  define FFI_MMAP_EXEC_SELINUX 1
# endif /* !defined FFI_MMAP_EXEC_SELINUX */
# اگر !defined FFI_MMAP_PAX
/* Also check for PaX MPROTECT */
#  define FFI_MMAP_PAX 1
# endif /* !defined FFI_MMAP_PAX */
#اگر_ختم /* FFI_MMAP_EXEC_WRIT && defined(__linux__) && !defined(__ANDROID__) */

#اگر FFI_CLOSURES

#اگر FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شدہ __MACH__

#شامل <mach/mach.h>
#شامل <pthread.h>
#اگر_تعریف_شدہ HAVE_ARM64E_PTRAUTH
#شامل <ptrauth.h>
#اگر_ختم
#شامل <stdio.h>
#شامل <stdlib.h>

خارجی خالی *ffi_احاطہ_trampoline_جدول_صفحہ;

قسمتعریف ساخت ffi_trampoline_جدول ffi_trampoline_جدول;
قسمتعریف ساخت ffi_trampoline_جدول_اندراج ffi_trampoline_جدول_اندراج;

ساخت ffi_trampoline_جدول
{
  /* contiguous writable and executable pages */
  vm_پتہ_t config_صفحہ;

  /* free list tracking */
  uint16_t free_گنتی;
  ffi_trampoline_جدول_اندراج *free_list;
  ffi_trampoline_جدول_اندراج *free_list_pool;

  ffi_trampoline_جدول *پچھلا;
  ffi_trampoline_جدول *اگلا;
};

ساخت ffi_trampoline_جدول_اندراج
{
  خالی *(*trampoline) (خالی);
  ffi_trampoline_جدول_اندراج *اگلا;
};

/* Total number of trampolines that fit in one trampoline table */
#تعریف_کرو FFI_TRAMPOLINE_گنتی (صفحہ_MAX_سائز / FFI_TRAMPOLINE_سائز)

ساکن pthread_mutex_t ffi_trampoline_تالا = PTHREAD_MUTEX_INITIALIZER;
ساکن ffi_trampoline_جدول *ffi_trampoline_tables = تہی;

ساکن ffi_trampoline_جدول *
ffi_trampoline_جدول_alloc (خالی)
{
  ffi_trampoline_جدول *جدول;
  vm_پتہ_t config_صفحہ;
  vm_پتہ_t trampoline_صفحہ;
  vm_پتہ_t trampoline_صفحہ_template;
  vm_prot_t cur_prot;
  vm_prot_t max_prot;
  kern_return_t kt;
  uint16_t i;

  /* Allocate two pages -- a config page and a placeholder page */
  config_صفحہ = 0x0;
  kt = vm_allocate (mach_task_self (), &config_صفحہ, صفحہ_MAX_سائز * 2,
		    VM_FLAGS_ANYWHERE);
  اگر (kt != KERN_SUCCESS)
    واپس تہی;

  /* Remap the trampoline table on top of the placeholder page */
  trampoline_صفحہ = config_صفحہ + صفحہ_MAX_سائز;

#اگر_تعریف_شدہ HAVE_ARM64E_PTRAUTH
  trampoline_صفحہ_template = (vm_پتہ_t)(uintptr_t)ptrauth_auth_ڈیٹا((خالی *)&ffi_احاطہ_trampoline_جدول_صفحہ, ptrauth_کلید_function_اشارہ_گر, 0);
#ورنہ
  trampoline_صفحہ_template = (vm_پتہ_t)&ffi_احاطہ_trampoline_جدول_صفحہ;
#اگر_ختم

#اگر_تعریف_شدہ __arm__
  /* ffi_closure_trampoline_table_page can be thumb-biased on some ARM archs */
  trampoline_صفحہ_template &= ~1UL;
#اگر_ختم
  kt = vm_remap (mach_task_self (), &trampoline_صفحہ, صفحہ_MAX_سائز, 0x0,
		 VM_FLAGS_OVERWRITE, mach_task_self (), trampoline_صفحہ_template,
		 FALSE, &cur_prot, &max_prot, VM_INHERIT_SHARE);
  اگر (kt != KERN_SUCCESS)
    {
      vm_deallocate (mach_task_self (), config_صفحہ, صفحہ_MAX_سائز * 2);
      واپس تہی;
    }

  اگر (!(cur_prot & VM_PROT_EXECUTE))
    {
      /* If VM_PROT_EXECUTE isn't set on the remapped trampoline page, set it */
      kt = vm_protect (mach_task_self (), trampoline_صفحہ, صفحہ_MAX_سائز,
         FALSE, cur_prot | VM_PROT_EXECUTE);
      اگر (kt != KERN_SUCCESS)
        {
          vm_deallocate (mach_task_self (), config_صفحہ, صفحہ_MAX_سائز * 2);
          واپس تہی;
        }
    }

  /* We have valid trampoline and config pages */
  جدول = صفر_حافظہ_دو (1, حجم (ffi_trampoline_جدول));
  اگر (جدول == تہی)
    {
      vm_deallocate (mach_task_self (), config_صفحہ, صفحہ_MAX_سائز * 2);
      واپس تہی;
    }
  جدول->free_گنتی = FFI_TRAMPOLINE_گنتی;
  جدول->config_صفحہ = config_صفحہ;

  /* Create and initialize the free list */
  جدول->free_list_pool =
    صفر_حافظہ_دو (FFI_TRAMPOLINE_گنتی, حجم (ffi_trampoline_جدول_اندراج));
  اگر (جدول->free_list_pool == تہی)
    {
      vm_deallocate (mach_task_self (), config_صفحہ, صفحہ_MAX_سائز * 2);
      آزاد (جدول);
      واپس تہی;
    }

  برائے (i = 0; i < جدول->free_گنتی; i++)
    {
      ffi_trampoline_جدول_اندراج *اندراج_2 = &جدول->free_list_pool[i];
      اندراج_2->trampoline =
	(خالی *) (trampoline_صفحہ + (i * FFI_TRAMPOLINE_سائز));
#اگر_تعریف_شدہ HAVE_ARM64E_PTRAUTH
      اندراج_2->trampoline = ptrauth_sign_unauthenticated(اندراج_2->trampoline, ptrauth_کلید_function_اشارہ_گر, 0);
#اگر_ختم

      اگر (i < جدول->free_گنتی - 1)
	اندراج_2->اگلا = &جدول->free_list_pool[i + 1];
    }

  جدول->free_list = جدول->free_list_pool;

  واپس جدول;
}

ساکن خالی
ffi_trampoline_جدول_free (ffi_trampoline_جدول *جدول)
{
  /* Remove from the list */
  اگر (جدول->پچھلا != تہی)
    جدول->پچھلا->اگلا = جدول->اگلا;

  اگر (جدول->اگلا != تہی)
    جدول->اگلا->پچھلا = جدول->پچھلا;

  /* Deallocate pages */
  vm_deallocate (mach_task_self (), جدول->config_صفحہ, صفحہ_MAX_سائز * 2);

  /* Deallocate free list */
  آزاد (جدول->free_list_pool);
  آزاد (جدول);
}

خالی *
ffi_احاطہ_alloc (size_t سائز, خالی **رمز)
{
  /* Create the closure */
  ffi_احاطہ *احاطہ = حافظہ_دو (سائز);
  اگر (احاطہ == تہی)
    واپس تہی;

  pthread_mutex_تالا (&ffi_trampoline_تالا);

  /* Check for an active trampoline table with available entries. */
  ffi_trampoline_جدول *جدول = ffi_trampoline_tables;
  اگر (جدول == تہی || جدول->free_list == تہی)
    {
      جدول = ffi_trampoline_جدول_alloc ();
      اگر (جدول == تہی)
	{
	  pthread_mutex_unlock (&ffi_trampoline_تالا);
	  آزاد (احاطہ);
	  واپس تہی;
	}

      /* Insert the new table at the top of the list */
      جدول->اگلا = ffi_trampoline_tables;
      اگر (جدول->اگلا != تہی)
	جدول->اگلا->پچھلا = جدول;

      ffi_trampoline_tables = جدول;
    }

  /* Claim the free entry */
  ffi_trampoline_جدول_اندراج *اندراج_2 = ffi_trampoline_tables->free_list;
  ffi_trampoline_tables->free_list = اندراج_2->اگلا;
  ffi_trampoline_tables->free_گنتی--;
  اندراج_2->اگلا = تہی;

  pthread_mutex_unlock (&ffi_trampoline_تالا);

  /* Initialize the return values */
  *رمز = اندراج_2->trampoline;
  احاطہ->trampoline_جدول = جدول;
  احاطہ->trampoline_جدول_اندراج = اندراج_2;

  واپس احاطہ;
}

خالی
ffi_احاطہ_free (خالی *ptr)
{
  ffi_احاطہ *احاطہ = ptr;

  pthread_mutex_تالا (&ffi_trampoline_تالا);

  /* Fetch the table and entry references */
  ffi_trampoline_جدول *جدول = احاطہ->trampoline_جدول;
  ffi_trampoline_جدول_اندراج *اندراج_2 = احاطہ->trampoline_جدول_اندراج;

  /* Return the entry to the free list */
  اندراج_2->اگلا = جدول->free_list;
  جدول->free_list = اندراج_2;
  جدول->free_گنتی++;

  /* If all trampolines within this table are free, and at least one other table exists, deallocate
   * the table */
  اگر (جدول->free_گنتی == FFI_TRAMPOLINE_گنتی
      && ffi_trampoline_tables != جدول)
    {
      ffi_trampoline_جدول_free (جدول);
    }
  ورنہ اگر (ffi_trampoline_tables != جدول)
    {
      /* Otherwise, bump this table to the top of the list */
      جدول->پچھلا = تہی;
      جدول->اگلا = ffi_trampoline_tables;
      اگر (ffi_trampoline_tables != تہی)
	ffi_trampoline_tables->پچھلا = جدول;

      ffi_trampoline_tables = جدول;
    }

  pthread_mutex_unlock (&ffi_trampoline_تالا);

  /* Free the closure */
  آزاد (احاطہ);
}

#اگر_ختم

// Per-target implementation; It's unclear what can reasonable be shared between two OS/architecture implementations.

#وگرنہ_اگر FFI_MMAP_EXEC_WRIT /* !FFI_EXEC_TRAMPOLINE_TABLE */

#تعریف_کرو USE_LOCKS 1
#تعریف_کرو USE_DL_سابقہ 1
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
#تعریف_کرو DEFAULT_MMAP_حد MAX_سائز_T

/* Don't allocate more than a page unless needed.  */
#تعریف_کرو DEFAULT_GRANULARITY ((size_t)malloc_getpagesize)

#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <fcntl.h>
#شامل <errno.h>
#اگر_غیر_تعریف_شدہ _MSC_VER
#شامل <unistd.h>
#اگر_ختم
#شامل <string.h>
#شامل <stdio.h>
#اگر !defined(_WIN32)
#اگر_تعریف_شدہ HAVE_MNTENT
#شامل <mntent.h>
#اگر_ختم /* HAVE_MNTENT */
#شامل <sys/param.h>
#شامل <pthread.h>

/* We don't want sys/mman.h to be included after we redefine mmap and
   dlmunmap.  */
#شامل <sys/mman.h>
#تعریف_کرو LACKS_SYS_MMAN_H 1

#اگر FFI_MMAP_EXEC_SELINUX
#شامل <sys/statfs.h>
#شامل <stdlib.h>

ساکن صحیح selinux_enabled = -1;

ساکن صحیح
selinux_enabled_check (خالی)
{
  ساخت statfs sfs;
  FILE *f;
  حرف *buf = تہی;
  size_t len = 0;

  اگر (statfs ("/selinux", &sfs) >= 0
      && (غیرعلامتی صحیح) sfs.f_type == 0xf97cff8cU)
    واپس 1;
  f = fopen ("/proc/mounts", "r");
  اگر (f == تہی)
    واپس 0;
  جب (getline (&buf, &len, f) >= 0)
    {
      حرف *p = strchr (buf, ' ');
      اگر (p == تہی)
        توڑو;
      p = strchr (p + 1, ' ');
      اگر (p == تہی)
        توڑو;
      اگر (لڑی_موازنہ (p + 1, "selinuxfs ", 10) == 0)
        {
          آزاد (buf);
          fclose (f);
          واپس 1;
        }
    }
  آزاد (buf);
  fclose (f);
  واپس 0;
}

#تعریف_کرو is_selinux_enabled() (selinux_enabled >= 0 ? selinux_enabled \
			      : (selinux_enabled = selinux_enabled_check ()))

#ورنہ

#تعریف_کرو is_selinux_enabled() 0

#اگر_ختم /* !FFI_MMAP_EXEC_SELINUX */

/* On PaX enable kernels that have MPROTECT enabled we can't use PROT_EXEC. */
#اگر defined FFI_MMAP_PAX
#شامل <stdlib.h>

شمار {
  PAX_MPROTECT = (1 << 0),
  PAX_EMUTRAMP = (1 << 1),
};
ساکن صحیح cached_pax_flags = -1;

ساکن صحیح
pax_flags_check (خالی)
{
  حرف *buf = تہی;
  size_t len = 0;
  FILE *f;
  صحیح ret;
  f = fopen ("/proc/self/status", "r");
  اگر (f == تہی)
    واپس 0;
  ret = 0;

  جب (getline (&buf, &len, f) != -1)
    اگر (!لڑی_موازنہ (buf, "PaX:", 4))
      {
        اگر (تہی != strchr (buf + 4, 'M'))
          ret |= PAX_MPROTECT;
        اگر (تہی != strchr (buf + 4, 'E'))
          ret |= PAX_EMUTRAMP;
        توڑو;
      }
  آزاد (buf);
  fclose (f);
  واپس ret;
}

#تعریف_کرو get_pax_flags() (cached_pax_flags >= 0 ? cached_pax_flags \
                               : (cached_pax_flags = pax_flags_check ()))
#تعریف_کرو has_pax_flags(flags) ((flags) == ((flags) & get_pax_flags ()))
#تعریف_کرو is_mprotect_enabled() (has_pax_flags (PAX_MPROTECT))
#تعریف_کرو is_emutramp_enabled() (has_pax_flags (PAX_EMUTRAMP))

#اگر_ختم /* defined FFI_MMAP_PAX */

#وگرنہ_اگر defined (__CYGWIN__) || defined(__INTERIX)

#شامل <sys/mman.h>

/* Cygwin is Linux-like, but not quite that Linux-like.  */
#تعریف_کرو is_selinux_enabled() 0

#اگر_ختم /* !defined(X86_WIN32) && !defined(X86_WIN64) */

#اگر !defined FFI_MMAP_PAX
# define is_mprotect_enabled() 0
# define is_emutramp_enabled() 0
#اگر_ختم /* !defined FFI_MMAP_PAX */

/* Declare all functions defined in dlmalloc.c as static.  */
ساکن خالی *dlmalloc(size_t);
ساکن خالی dlfree(خالی*);
ساکن خالی *dlcalloc(size_t, size_t) MAYBE_غیر_مستعمل;
ساکن خالی *dlrealloc(خالی *, size_t) MAYBE_غیر_مستعمل;
ساکن خالی *dlmemalign(size_t, size_t) MAYBE_غیر_مستعمل;
ساکن خالی *dlvalloc(size_t) MAYBE_غیر_مستعمل;
ساکن صحیح dlmallopt(صحیح, صحیح) MAYBE_غیر_مستعمل;
ساکن size_t dlmalloc_footprint(خالی) MAYBE_غیر_مستعمل;
ساکن size_t dlmalloc_max_footprint(خالی) MAYBE_غیر_مستعمل;
ساکن خالی** dlindependent_calloc(size_t, size_t, خالی**) MAYBE_غیر_مستعمل;
ساکن خالی** dlindependent_comalloc(size_t, size_t*, خالی**) MAYBE_غیر_مستعمل;
ساکن خالی *dlpvalloc(size_t) MAYBE_غیر_مستعمل;
ساکن صحیح dlmalloc_trim(size_t) MAYBE_غیر_مستعمل;
ساکن size_t dlmalloc_usable_سائز(خالی*) MAYBE_غیر_مستعمل;
ساکن خالی dlmalloc_stats(خالی) MAYBE_غیر_مستعمل;

#اگر !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)
/* Use these for mmap and munmap within dlmalloc.c.  */
ساکن خالی *dlmmap(خالی *, size_t, صحیح, صحیح, صحیح, off_t);
ساکن صحیح dlmunmap(خالی *, size_t);
#اگر_ختم /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

#تعریف_کرو mmap dlmmap
#تعریف_کرو munmap dlmunmap

#شامل "dlmalloc.c"

#تعریف_منسوخ mmap
#تعریف_منسوخ munmap

#اگر !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX)

/* A mutex used to synchronize access to *exec* variables in this file.  */
ساکن pthread_mutex_t open_عارضی_exec_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* A file descriptor of a temporary file from which we'll map
   executable pages.  */
ساکن صحیح execfd = -1;

/* The amount of space already allocated from the temporary file.  */
ساکن size_t execsize = 0;

#اگر_تعریف_شدہ HAVE_MEMFD_CREATE
/* Open a temporary file name, and immediately unlink it.  */
ساکن صحیح
open_عارضی_exec_file_memfd (ثابت حرف *name)
{
  صحیح fd;
  fd = memfd_create (name, MFD_CLOEXEC);
  واپس fd;
}
#اگر_ختم

/* Open a temporary file name, and immediately unlink it.  */
ساکن صحیح
open_عارضی_exec_file_name (حرف *name, صحیح flags MAYBE_غیر_مستعمل)
{
  صحیح fd;

#اگر_تعریف_شدہ HAVE_MKOSTEMP
  fd = mkostemp (name, flags);
#ورنہ
  fd = mkstemp (name);
#اگر_ختم

  اگر (fd != -1)
    unlink (name);

  واپس fd;
}

/* Open a temporary file in the named directory.  */
ساکن صحیح
open_عارضی_exec_file_dir (ثابت حرف *dir)
{
  ساکن ثابت حرف suffix[] = "/ffiXXXXXX";
  صحیح lendir, flags;
  حرف *tempname;
#اگر_تعریف_شدہ O_TMPFILE
  صحیح fd;
#اگر_ختم

#اگر_تعریف_شدہ O_CLOEXEC
  flags = O_CLOEXEC;
#ورنہ
  flags = 0;
#اگر_ختم

#اگر_تعریف_شدہ O_TMPFILE
  fd = open (dir, flags | O_RDWR | O_EXCL | O_TMPFILE, 0700);
  /* If the running system does not support the O_TMPFILE flag then retry without it. */
  اگر (fd != -1 || (errno != EINVAL && errno != EISDIR && errno != EOPNOTSUPP)) {
    واپس fd;
  } ورنہ {
    errno = 0;
  }
#اگر_ختم

  lendir = (صحیح) لڑی_لمبائی (dir);
  tempname = __builtin_alloca (lendir + حجم (suffix));

  اگر (!tempname)
    واپس -1;

  حافظہ_نقل (tempname, dir, lendir);
  حافظہ_نقل (tempname + lendir, suffix, حجم (suffix));

  واپس open_عارضی_exec_file_name (tempname, flags);
}

/* Open a temporary file in the directory in the named environment
   variable.  */
ساکن صحیح
open_عارضی_exec_file_env (ثابت حرف *envvar)
{
  ثابت حرف *قدر = getenv (envvar);

  اگر (!قدر)
    واپس -1;

  واپس open_عارضی_exec_file_dir (قدر);
}

#اگر_تعریف_شدہ HAVE_MNTENT
/* Open a temporary file in an executable and writable mount point
   listed in the mounts file.  Subsequent calls with the same mounts
   keep searching for mount points in the same file.  Providing NULL
   as the mounts file closes the file.  */
ساکن صحیح
open_عارضی_exec_file_mnt (ثابت حرف *mounts)
{
  ساکن ثابت حرف *آخری_mounts;
  ساکن FILE *آخری_mntent;

  اگر (mounts != آخری_mounts)
    {
      اگر (آخری_mntent)
	endmntent (آخری_mntent);

      آخری_mounts = mounts;

      اگر (mounts)
	آخری_mntent = setmntent (mounts, "r");
      ورنہ
	آخری_mntent = تہی;
    }

  اگر (!آخری_mntent)
    واپس -1;

  برائے (;;)
    {
      صحیح fd;
      ساخت mntent mnt;
      حرف buf[MAXPATHLEN * 3];

      اگر (getmntent_r (آخری_mntent, &mnt, buf, حجم (buf)) == تہی)
	واپس -1;

      اگر (hasmntopt (&mnt, "ro")
	  || hasmntopt (&mnt, "noexec")
	  || access (mnt.mnt_dir, W_OK))
	جاری;

      fd = open_عارضی_exec_file_dir (mnt.mnt_dir);

      اگر (fd != -1)
	واپس fd;
    }
}
#اگر_ختم /* HAVE_MNTENT */

/* Instructions to look for a location to hold a temporary file that
   can be mapped in for execution.  */
ساکن ساخت
{
  صحیح (*func)(ثابت حرف *);
  ثابت حرف *arg;
  صحیح repeat;
} open_عارضی_exec_file_opts[] = {
#اگر_تعریف_شدہ HAVE_MEMFD_CREATE
  { open_عارضی_exec_file_memfd, "libffi", 0 },
#اگر_ختم
  { open_عارضی_exec_file_env, "LIBFFI_TMPDIR", 0 },
  { open_عارضی_exec_file_env, "TMPDIR", 0 },
  { open_عارضی_exec_file_dir, "/tmp", 0 },
  { open_عارضی_exec_file_dir, "/var/tmp", 0 },
  { open_عارضی_exec_file_dir, "/dev/shm", 0 },
  { open_عارضی_exec_file_env, "HOME", 0 },
#اگر_تعریف_شدہ HAVE_MNTENT
  { open_عارضی_exec_file_mnt, "/etc/mtab", 1 },
  { open_عارضی_exec_file_mnt, "/proc/mounts", 1 },
#اگر_ختم /* HAVE_MNTENT */
};

/* Current index into open_temp_exec_file_opts.  */
ساکن صحیح open_عارضی_exec_file_opts_idx = 0;

/* Reset a current multi-call func, then advances to the next entry.
   If we're at the last, go back to the first and return nonzero,
   otherwise return zero.  */
ساکن صحیح
open_عارضی_exec_file_opts_اگلا (خالی)
{
  اگر (open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat)
    open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].func (تہی);

  open_عارضی_exec_file_opts_idx++;
  اگر (open_عارضی_exec_file_opts_idx
      == (حجم (open_عارضی_exec_file_opts)
	  / حجم (*open_عارضی_exec_file_opts)))
    {
      open_عارضی_exec_file_opts_idx = 0;
      واپس 1;
    }

  واپس 0;
}

/* Return a file descriptor of a temporary zero-sized file in a
   writable and executable filesystem.  */
صحیح
open_عارضی_exec_file (خالی)
{
  صحیح fd;

  کرو
    {
      fd = open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].func
	(open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].arg);

      اگر (!open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat
	  || fd == -1)
	{
	  اگر (open_عارضی_exec_file_opts_اگلا ())
	    توڑو;
	}
    }
  جب (fd == -1);

  واپس fd;
}

/* We need to allocate space in a file that will be backing a writable
   mapping.  Several problems exist with the usual approaches:
   - fallocate() is Linux-only
   - posix_fallocate() is not available on all platforms
   - ftruncate() does not allocate space on filesystems with sparse files
   Failure to allocate the space will cause SIGBUS to be thrown when
   the mapping is subsequently written to.  */
ساکن صحیح
allocate_space (صحیح fd, off_t len)
{
  ساکن طویل صفحہ_سائز;

  /* Obtain system page size. */
  اگر (!صفحہ_سائز)
    صفحہ_سائز = sysconf(_SC_PAGESIZE);
  اگر (صفحہ_سائز <= 0)
    واپس -1;

  غیرعلامتی حرف buf[صفحہ_سائز];
  memset (buf, 0, صفحہ_سائز);

  جب (len > 0)
    {
      off_t مقصد_write = (len < صفحہ_سائز) ? len : صفحہ_سائز;
      اگر (اكتب (fd, buf, مقصد_write) < مقصد_write)
        واپس -1;
      len -= مقصد_write;
    }

  واپس 0;
}

/* Map in a chunk of memory from the temporary exec file into separate
   locations in the virtual memory address space, one writable and one
   executable.  Returns the address of the writable portion, after
   storing an offset to the corresponding executable portion at the
   last word of the requested chunk.  */
ساکن خالی *
dlmmap_locked (خالی *آغاز, size_t لمبائی, صحیح prot, صحیح flags, off_t offset)
{
  خالی *ptr;

  اگر (execfd == -1)
    {
      open_عارضی_exec_file_opts_idx = 0;
    retry_open:
      execfd = open_عارضی_exec_file ();
      اگر (execfd == -1)
	واپس MFAIL;
    }

  offset = execsize;

  اگر (allocate_space (execfd, لمبائی))
    واپس MFAIL;

  flags &= ~(MAP_نجی | MAP_ANONYMOUS);
  flags |= MAP_SHARED;

  ptr = mmap (تہی, لمبائی, (prot & ~PROT_WRITE) | PROT_EXEC,
	      flags, execfd, offset);
  اگر (ptr == MFAIL)
    {
      اگر (!offset)
	{
	  close (execfd);
	  جاؤ retry_open;
	}
      اگر (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }

      واپس MFAIL;
    }
  ورنہ اگر (!offset
	   && open_عارضی_exec_file_opts[open_عارضی_exec_file_opts_idx].repeat)
    open_عارضی_exec_file_opts_اگلا ();

  آغاز = mmap (آغاز, لمبائی, prot, flags, execfd, offset);

  اگر (آغاز == MFAIL)
    {
      munmap (ptr, لمبائی);
      اگر (ftruncate (execfd, offset) != 0)
      {
        /* Fixme : Error logs can be added here. Returning an error for
         * ftruncte() will not add any advantage as it is being
         * validating in the error case. */
      }
      واپس آغاز;
    }

  mmap_exec_انحراف ((حرف *)آغاز, لمبائی) = (حرف*)ptr - (حرف*)آغاز;

  execsize += لمبائی;

  واپس آغاز;
}

/* Map in a writable and executable chunk of memory if possible.
   Failing that, fall back to dlmmap_locked.  */
ساکن خالی *
dlmmap (خالی *آغاز, size_t لمبائی, صحیح prot,
	صحیح flags, صحیح fd, off_t offset)
{
  خالی *ptr;

  أكد (آغاز == تہی && لمبائی % malloc_getpagesize == 0
	  && prot == (PROT_READ | PROT_WRITE)
	  && flags == (MAP_نجی | MAP_ANONYMOUS)
	  && fd == -1 && offset == 0);

  اگر (execfd == -1 && ffi_tramp_is_supported ())
    {
      ptr = mmap (آغاز, لمبائی, prot & ~PROT_EXEC, flags, fd, offset);
      واپس ptr;
    }

  /* -1 != execfd hints that we already decided to use dlmmap_locked
     last time.  */
  اگر (execfd == -1 && is_mprotect_enabled ())
    {
#اگر_تعریف_شدہ FFI_MMAP_EXEC_EMUTRAMP_PAX
      اگر (is_emutramp_enabled ())
        {
          /* emutramp requires the kernel recognizing the trampoline pattern
             generated by ffi_prep_closure_loc; there is no way to test
             in advance whether this will work, so this is experimental.  */
          ptr = mmap (آغاز, لمبائی, prot & ~PROT_EXEC, flags, fd, offset);
          واپس ptr;
        }
#اگر_ختم
      /* fallback to dlmmap_locked.  */
    }
  ورنہ اگر (execfd == -1 && !is_selinux_enabled ())
    {
      ptr = mmap (آغاز, لمبائی, prot | PROT_EXEC, flags, fd, offset);

      اگر (ptr != MFAIL || (errno != EPERM && errno != EACCES))
	/* Cool, no need to mess with separate segments.  */
	واپس ptr;

      /* If MREMAP_DUP is ever introduced and implemented, try mmap
	 with ((prot & ~PROT_WRITE) | PROT_EXEC) and mremap with
	 MREMAP_DUP and prot at this point.  */
    }

  pthread_mutex_تالا (&open_عارضی_exec_file_mutex);
  ptr = dlmmap_locked (آغاز, لمبائی, prot, flags, offset);
  pthread_mutex_unlock (&open_عارضی_exec_file_mutex);

  واپس ptr;
}

/* Release memory at the given address, as well as the corresponding
   executable page if it's separate.  */
ساکن صحیح
dlmunmap (خالی *آغاز, size_t لمبائی)
{
  /* We don't bother decreasing execsize or truncating the file, since
     we can't quite tell whether we're unmapping the end of the file.
     We don't expect frequent deallocation anyway.  If we did, we
     could locate pages in the file by writing to the pages being
     deallocated and checking that the file contents change.
     Yuck.  */
  msegmentptr seg = segment_holding (gm, آغاز);
  خالی *رمز;

  اگر (seg && (رمز = جوڑو_segment_exec_انحراف (آغاز, seg)) != آغاز)
    {
      صحیح ret = munmap (رمز, لمبائی);
      اگر (ret)
	واپس ret;
    }

  واپس munmap (آغاز, لمبائی);
}

#اگر FFI_احاطہ_FREE_رمز
/* Return segment holding given code address.  */
ساکن msegmentptr
segment_holding_رمز (mstate m, حرف* addr)
{
  msegmentptr sp = &m->seg;
  برائے (;;) {
    اگر (addr >= جوڑو_segment_exec_انحراف (sp->بنیاد, sp)
	&& addr < جوڑو_segment_exec_انحراف (sp->بنیاد, sp) + sp->سائز)
      واپس sp;
    اگر ((sp = sp->اگلا) == 0)
      واپس 0;
  }
}
#اگر_ختم

#اگر_ختم /* !(defined(_WIN32) || defined(__OS2__)) || defined (__CYGWIN__) || defined(__INTERIX) */

/* Allocate a chunk of memory with the given size.  Returns a pointer
   to the writable address, and sets *CODE to the executable
   corresponding virtual address.  */
خالی *
ffi_احاطہ_alloc (size_t سائز, خالی **رمز)
{
  خالی *ptr, *ftramp;

  اگر (!رمز)
    واپس تہی;

  ptr = dlmalloc (سائز);

  اگر (ptr)
    {
      msegmentptr seg = segment_holding (gm, ptr);

      *رمز = FFI_FN (جوڑو_segment_exec_انحراف (ptr, seg));
      اگر (!ffi_tramp_is_supported ())
        واپس ptr;

      ftramp = ffi_tramp_alloc (0);
      اگر (ftramp == تہی)
      {
        dlfree (ptr);
        واپس تہی;
      }
      *رمز = FFI_FN (ffi_tramp_get_addr (ftramp));
      ((ffi_احاطہ *) ptr)->ftramp = ftramp;
    }

  واپس ptr;
}

خالی *
ffi_ڈیٹا_مقصد_رمز_اشارہ_گر (خالی *ڈیٹا)
{
  msegmentptr seg = segment_holding (gm, ڈیٹا);
  /* We expect closures to be allocated with ffi_closure_alloc(), in
     which case seg will be non-NULL.  However, some users take on the
     burden of managing this memory themselves, in which case this
     we'll just return data. */
  اگر (seg)
    {
      اگر (!ffi_tramp_is_supported ())
        واپس جوڑو_segment_exec_انحراف (ڈیٹا, seg);
      واپس ffi_tramp_get_addr (((ffi_احاطہ *) ڈیٹا)->ftramp);
    }
  ورنہ
    واپس ڈیٹا;
}

/* Release a chunk of memory allocated with ffi_closure_alloc.  If
   FFI_CLOSURE_FREE_CODE is nonzero, the given address can be the
   writable or the executable address given.  Otherwise, only the
   writable address can be provided here.  */
خالی
ffi_احاطہ_free (خالی *ptr)
{
#اگر FFI_احاطہ_FREE_رمز
  msegmentptr seg = segment_holding_رمز (gm, ptr);

  اگر (seg)
    ptr = sub_segment_exec_انحراف (ptr, seg);
#اگر_ختم
  اگر (ffi_tramp_is_supported ())
    ffi_tramp_free (((ffi_احاطہ *) ptr)->ftramp);

  dlfree (ptr);
}

صحیح
ffi_tramp_is_present (خالی *ptr)
{
  msegmentptr seg = segment_holding (gm, ptr);
  واپس seg != تہی && ffi_tramp_is_supported();
}

# ورنہ /* ! FFI_MMAP_EXEC_WRIT */

/* On many systems, memory returned by malloc is writable and
   executable, so just use it.  */

#شامل <stdlib.h>

خالی *
ffi_احاطہ_alloc (size_t سائز, خالی **رمز)
{
  خالی *c;

  اگر (!رمز)
    واپس تہی;

  c = حافظہ_دو (سائز);
  *رمز = FFI_FN (c);
  واپس c;
}

خالی
ffi_احاطہ_free (خالی *ptr)
{
  آزاد (ptr);
}

خالی *
ffi_ڈیٹا_مقصد_رمز_اشارہ_گر (خالی *ڈیٹا)
{
  واپس ڈیٹا;
}

صحیح
ffi_tramp_is_present (__attribute__((غیر_مستعمل)) خالی *ptr)
{
  واپس 0;
}

# endif /* ! FFI_MMAP_EXEC_WRIT */
#اگر_ختم /* FFI_CLOSURES */

#اگر_ختم /* NetBSD with PROT_MPROTECT */
#اگر_ختم /* __EMSCRIPTEN__ */

/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2014, 2018, 2026  Anthony Green
                 Copyright (c) 1996-2003, 2010  Red Hat, Inc.
                 Copyright (C) 2008  Free Software Foundation, Inc.

   Target configuration macros for x86 and x86-64.

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

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#وصف_ڪريو X86_ANY

#إذا defined (X86_64) && defined (__i386__)
#وصف_رد X86_64
#وصف_ڪريو X86
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل X86_WIN64
#وصف_ڪريو FFI_SIZEOF_ARG 8
#وصف_ڪريو USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#جيڪڏهن_پورو

#وصف_ڪريو FFI_هدف_SPECIFIC_ڍير_SPACE_ورهاست
#جيڪڏهن_وصف_نه_ٿيل _MSC_VER
#وصف_ڪريو FFI_هدف_HAS_COMPLEX_TYPE
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل X86_64
#وصف_ڪريو FFI_هدف_HAS_INT128
#جيڪڏهن_پورو

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#إذا defined(X86_64) && !defined(X86_WIN64)
#وصف_ڪريو FFI_هدف_HAS_ويڪٽر_TYPE
#جيڪڏهن_پورو

/* ---- Generic type definitions ----------------------------------------- */

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM
#جيڪڏهن_وصف_ٿيل X86_WIN64
#جيڪڏهن_وصف_ٿيل _MSC_VER
تعريفنوع بنانشان __int64       ffi_arg;
تعريفنوع __int64                ffi_sarg;
#وإلا
تعريفنوع بنانشان ڊگهو ڊگهو     ffi_arg;
تعريفنوع ڊگهو ڊگهو              ffi_sarg;
#جيڪڏهن_پورو
#وإلا
#إذا defined __x86_64__ && defined __ILP32__
#وصف_ڪريو FFI_SIZEOF_ARG 8
#وصف_ڪريو FFI_SIZEOF_JAVA_RAW  4
تعريفنوع بنانشان ڊگهو ڊگهو     ffi_arg;
تعريفنوع ڊگهو ڊگهو              ffi_sarg;
#وإلا
تعريفنوع بنانشان ڊگهو          ffi_arg;
تعريفنوع نشانوارو ڊگهو            ffi_sarg;
#جيڪڏهن_پورو
#جيڪڏهن_پورو

تعريفنوع تعداد ffi_abi {
#إذا defined(X86_WIN64)
  FFI_پهريون_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_آخري_ABI,
#جيڪڏهن_وصف_ٿيل __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#وإلا
  FFI_DEFAULT_ABI = FFI_WIN64
#جيڪڏهن_پورو

#وإلاإذا defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_پهريون_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_آخري_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#وإلاإذا defined(X86_WIN32)
  FFI_پهريون_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_رجسٽر  = 7,
  FFI_آخري_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#وإلا
  FFI_پهريون_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_رجسٽر  = 7,
  FFI_MS_CDECL  = 8,
  FFI_آخري_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#جيڪڏهن_پورو
} ffi_abi;
#جيڪڏهن_پورو

/* ---- Definitions for closures ----------------------------------------- */

#وصف_ڪريو FFI_CLOSURES 1
#وصف_ڪريو FFI_GO_CLOSURES 1

#وصف_ڪريو FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_آخري + 1)
#وصف_ڪريو FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_آخري + 2)
#وصف_ڪريو FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_آخري + 3)
#وصف_ڪريو FFI_TYPE_MS_STRUCT       (FFI_TYPE_آخري + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#وصف_ڪريو FFI_X86_TYPE_آخري FFI_TYPE_ويڪٽر
#إذا FFI_TYPE_آخري != FFI_X86_TYPE_آخري
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#جيڪڏهن_پورو

#إذا defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_سائيز 32
# define FFI_NATIVE_RAW_API 0
#وإلا
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_سائيز 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#جيڪڏهن_پورو

#إذا !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# إذا (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#وإلا
# define _CET_ENDBR
# define _CET_NOTRACK
#جيڪڏهن_پورو

#جيڪڏهن_پورو

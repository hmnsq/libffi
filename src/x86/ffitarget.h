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

#جے_بنا_تعریف LIBFFI_نشانہ_H
#تعریف_کرو LIBFFI_نشانہ_H

#جے_بنا_تعریف LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جے_انت

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#تعریف_کرو X86_ANY

#إذا defined (X86_64) && defined (__i386__)
#تعریف_رد X86_64
#تعریف_کرو X86
#جے_انت

#جے_تعریف_شدہ X86_WIN64
#تعریف_کرو FFI_SIZEOF_ARG 8
#تعریف_کرو USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#جے_انت

#تعریف_کرو FFI_نشانہ_SPECIFIC_ڈھیر_SPACE_ونڈ
#جے_بنا_تعریف _MSC_VER
#تعریف_کرو FFI_نشانہ_HAS_COMPLEX_TYPE
#جے_انت

#جے_تعریف_شدہ X86_64
#تعریف_کرو FFI_نشانہ_HAS_INT128
#جے_انت

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#إذا defined(X86_64) && !defined(X86_WIN64)
#تعریف_کرو FFI_نشانہ_HAS_سمتیہ_TYPE
#جے_انت

/* ---- Generic type definitions ----------------------------------------- */

#جے_بنا_تعریف LIBFFI_ASM
#جے_تعریف_شدہ X86_WIN64
#جے_تعریف_شدہ _MSC_VER
تعريفنوع بنانشانی __int64       ffi_arg;
تعريفنوع __int64                ffi_sarg;
#وإلا
تعريفنوع بنانشانی لما لما     ffi_arg;
تعريفنوع لما لما              ffi_sarg;
#جے_انت
#وإلا
#إذا defined __x86_64__ && defined __ILP32__
#تعریف_کرو FFI_SIZEOF_ARG 8
#تعریف_کرو FFI_SIZEOF_JAVA_RAW  4
تعريفنوع بنانشانی لما لما     ffi_arg;
تعريفنوع لما لما              ffi_sarg;
#وإلا
تعريفنوع بنانشانی لما          ffi_arg;
تعريفنوع نشانیوالا لما            ffi_sarg;
#جے_انت
#جے_انت

تعريفنوع تعداد ffi_abi {
#إذا defined(X86_WIN64)
  FFI_پہلا_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_آخری_ABI,
#جے_تعریف_شدہ __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#وإلا
  FFI_DEFAULT_ABI = FFI_WIN64
#جے_انت

#وإلاإذا defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_پہلا_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_آخری_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#وإلاإذا defined(X86_WIN32)
  FFI_پہلا_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_رجسٹر  = 7,
  FFI_آخری_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#وإلا
  FFI_پہلا_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_رجسٹر  = 7,
  FFI_MS_CDECL  = 8,
  FFI_آخری_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#جے_انت
} ffi_abi;
#جے_انت

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1

#تعریف_کرو FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_آخری + 1)
#تعریف_کرو FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_آخری + 2)
#تعریف_کرو FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_آخری + 3)
#تعریف_کرو FFI_TYPE_MS_STRUCT       (FFI_TYPE_آخری + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#تعریف_کرو FFI_X86_TYPE_آخری FFI_TYPE_سمتیہ
#إذا FFI_TYPE_آخری != FFI_X86_TYPE_آخری
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#جے_انت

#إذا defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_ناپ 32
# define FFI_NATIVE_RAW_API 0
#وإلا
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_ناپ 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#جے_انت

#إذا !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# إذا (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#وإلا
# define _CET_ENDBR
# define _CET_NOTRACK
#جے_انت

#جے_انت

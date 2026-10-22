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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ਨਿਸ਼ਾਨਾ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_ਨਿਸ਼ਾਨਾ_H

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_H
#ਗਲਤੀ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ X86_ANY

#ਜੇ defined (X86_64) && defined (__i386__)
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ X86_64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ X86
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ X86_WIN64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_ARG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_ਢੇਰ_SPACE_ਵੰਡ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _MSC_VER
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ X86_64
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_INT128
#ਜੇ_ਅੰਤ

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#ਜੇ defined(X86_64) && !defined(X86_WIN64)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_ਸਦਿਸ਼_TYPE
#ਜੇ_ਅੰਤ

/* ---- Generic type definitions ----------------------------------------- */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ X86_WIN64
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _MSC_VER
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ __int64       ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __int64                ffi_sarg;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ     ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਲੰਮਾ              ffi_sarg;
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਜੇ defined __x86_64__ && defined __ILP32__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_ARG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_JAVA_RAW  4
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ     ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਲੰਮਾ ਲੰਮਾ              ffi_sarg;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ          ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ            ffi_sarg;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi {
#ਜੇ defined(X86_WIN64)
  FFI_ਪਹਿਲਾ_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_ਆਖਰੀ_ABI,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_WIN64
#ਜੇ_ਅੰਤ

#ਜਾਂਜੇ defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_ਪਹਿਲਾ_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#ਜਾਂਜੇ defined(X86_WIN32)
  FFI_ਪਹਿਲਾ_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_ਰਜਿਸਟਰ  = 7,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#ਨਹੀਂਤਾਂ
  FFI_ਪਹਿਲਾ_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_ਰਜਿਸਟਰ  = 7,
  FFI_MS_CDECL  = 8,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#ਜੇ_ਅੰਤ
} ffi_abi;
#ਜੇ_ਅੰਤ

/* ---- Definitions for closures ----------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_GO_CLOSURES 1

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_ਆਖਰੀ + 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_ਆਖਰੀ + 2)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_ਆਖਰੀ + 3)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_MS_STRUCT       (FFI_TYPE_ਆਖਰੀ + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_X86_TYPE_ਆਖਰੀ FFI_TYPE_ਸਦਿਸ਼
#ਜੇ FFI_TYPE_ਆਖਰੀ != FFI_X86_TYPE_ਆਖਰੀ
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#ਜੇ_ਅੰਤ

#ਜੇ defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_ਮਾਪ 32
# define FFI_NATIVE_RAW_API 0
#ਨਹੀਂਤਾਂ
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_ਮਾਪ 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#ਜੇ_ਅੰਤ

#ਜੇ !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# ਜੇ (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#ਨਹੀਂਤਾਂ
# define _CET_ENDBR
# define _CET_NOTRACK
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

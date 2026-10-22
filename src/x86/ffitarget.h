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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ಗುರಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_ಗುರಿ_H

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_H
#ದೋಷ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ಆದರೆ_ಅಂತ್ಯ

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#ವ್ಯಾಖ್ಯಾನಿಸು X86_ANY

#ಆದರೆ defined (X86_64) && defined (__i386__)
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು X86_64
#ವ್ಯಾಖ್ಯಾನಿಸು X86
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ X86_WIN64
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_SIZEOF_ARG 8
#ವ್ಯಾಖ್ಯಾನಿಸು USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _MSC_VER
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_COMPLEX_TYPE
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ X86_64
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_INT128
#ಆದರೆ_ಅಂತ್ಯ

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#ಆದರೆ defined(X86_64) && !defined(X86_WIN64)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_ಸದಿಶ_TYPE
#ಆದರೆ_ಅಂತ್ಯ

/* ---- Generic type definitions ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ X86_WIN64
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _MSC_VER
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ __int64       ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __int64                ffi_sarg;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ     ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದೀರ್ಘ              ffi_sarg;
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ defined __x86_64__ && defined __ILP32__
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_SIZEOF_ARG 8
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_SIZEOF_JAVA_RAW  4
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ     ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದೀರ್ಘ ದೀರ್ಘ              ffi_sarg;
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ          ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ            ffi_sarg;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
#ಆದರೆ defined(X86_WIN64)
  FFI_ಮೊದಲ_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_ಕೊನೆಯ_ABI,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_WIN64
#ಆದರೆ_ಅಂತ್ಯ

#ಅಥವಾಆದರೆ defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_ಮೊದಲ_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#ಅಥವಾಆದರೆ defined(X86_WIN32)
  FFI_ಮೊದಲ_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_ರಿಜಿಸ್ಟರ್  = 7,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_ಮೊದಲ_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_ರಿಜಿಸ್ಟರ್  = 7,
  FFI_MS_CDECL  = 8,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#ಆದರೆ_ಅಂತ್ಯ
} ffi_abi;
#ಆದರೆ_ಅಂತ್ಯ

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_ಕೊನೆಯ + 1)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_ಕೊನೆಯ + 2)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_ಕೊನೆಯ + 3)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_MS_STRUCT       (FFI_TYPE_ಕೊನೆಯ + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_X86_TYPE_ಕೊನೆಯ FFI_TYPE_ಸದಿಶ
#ಆದರೆ FFI_TYPE_ಕೊನೆಯ != FFI_X86_TYPE_ಕೊನೆಯ
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_ಗಾತ್ರ 32
# define FFI_NATIVE_RAW_API 0
#ಇಲ್ಲದಿದ್ದರೆ
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_ಗಾತ್ರ 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# ಆದರೆ (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#ಇಲ್ಲದಿದ್ದರೆ
# define _CET_ENDBR
# define _CET_NOTRACK
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

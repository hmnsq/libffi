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

#నిర్వచితం_కాకపోతే LIBFFI_లక్ష్యం_H
#నిర్వచించు LIBFFI_లక్ష్యం_H

#నిర్వచితం_కాకపోతే LIBFFI_H
#దోషం "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#నిర్వచించు X86_ANY

#ఒకవేళ defined (X86_64) && defined (__i386__)
#నిర్వచనం_రద్దు X86_64
#నిర్వచించు X86
#అయితే_ముగింపు

#నిర్వచితమైతే X86_WIN64
#నిర్వచించు FFI_SIZEOF_ARG 8
#నిర్వచించు USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#అయితే_ముగింపు

#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_దొంతర_SPACE_కేటాయింపు
#నిర్వచితం_కాకపోతే _MSC_VER
#నిర్వచించు FFI_లక్ష్యం_HAS_COMPLEX_TYPE
#అయితే_ముగింపు

#నిర్వచితమైతే X86_64
#నిర్వచించు FFI_లక్ష్యం_HAS_INT128
#అయితే_ముగింపు

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#ఒకవేళ defined(X86_64) && !defined(X86_WIN64)
#నిర్వచించు FFI_లక్ష్యం_HAS_సదిశ_TYPE
#అయితే_ముగింపు

/* ---- Generic type definitions ----------------------------------------- */

#నిర్వచితం_కాకపోతే LIBFFI_ASM
#నిర్వచితమైతే X86_WIN64
#నిర్వచితమైతే _MSC_VER
రకనిర్వచనం గుర్తులేని __int64       ffi_arg;
రకనిర్వచనం __int64                ffi_sarg;
#లేకపోతే
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ     ffi_arg;
రకనిర్వచనం దీర్ఘ దీర్ఘ              ffi_sarg;
#అయితే_ముగింపు
#లేకపోతే
#ఒకవేళ defined __x86_64__ && defined __ILP32__
#నిర్వచించు FFI_SIZEOF_ARG 8
#నిర్వచించు FFI_SIZEOF_JAVA_RAW  4
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ     ffi_arg;
రకనిర్వచనం దీర్ఘ దీర్ఘ              ffi_sarg;
#లేకపోతే
రకనిర్వచనం గుర్తులేని దీర్ఘ          ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ            ffi_sarg;
#అయితే_ముగింపు
#అయితే_ముగింపు

రకనిర్వచనం గణన ffi_abi {
#ఒకవేళ defined(X86_WIN64)
  FFI_మొదటి_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_చివరి_ABI,
#నిర్వచితమైతే __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#లేకపోతే
  FFI_DEFAULT_ABI = FFI_WIN64
#అయితే_ముగింపు

#లేదావేళ defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_మొదటి_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_చివరి_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#లేదావేళ defined(X86_WIN32)
  FFI_మొదటి_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_రిజిస్టరు  = 7,
  FFI_చివరి_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#లేకపోతే
  FFI_మొదటి_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_రిజిస్టరు  = 7,
  FFI_MS_CDECL  = 8,
  FFI_చివరి_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#అయితే_ముగింపు
} ffi_abi;
#అయితే_ముగింపు

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_GO_CLOSURES 1

#నిర్వచించు FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_చివరి + 1)
#నిర్వచించు FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_చివరి + 2)
#నిర్వచించు FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_చివరి + 3)
#నిర్వచించు FFI_TYPE_MS_STRUCT       (FFI_TYPE_చివరి + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#నిర్వచించు FFI_X86_TYPE_చివరి FFI_TYPE_సదిశ
#ఒకవేళ FFI_TYPE_చివరి != FFI_X86_TYPE_చివరి
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#అయితే_ముగింపు

#ఒకవేళ defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_కొలత 32
# define FFI_NATIVE_RAW_API 0
#లేకపోతే
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_కొలత 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#అయితే_ముగింపు

#ఒకవేళ !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# ఒకవేళ (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#లేకపోతే
# define _CET_ENDBR
# define _CET_NOTRACK
#అయితే_ముగింపు

#అయితే_ముగింపు

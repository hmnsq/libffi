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

#यदि_अपरिभाषितम् LIBFFI_TARGET_H
#परिभाषय LIBFFI_TARGET_H

#यदि_अपरिभाषितम् LIBFFI_H
#दोषः "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#यदि_अन्तः

/* ---- System specific configurations ----------------------------------- */

/* For code common to all platforms on x86 and x86_64. */
#परिभाषय X86_ANY

#यदि defined (X86_64) && defined (__i386__)
#अपरिभाषय X86_64
#परिभाषय X86
#यदि_अन्तः

#यदि_परिभाषितम् X86_WIN64
#परिभाषय FFI_SIZEOF_ARG 8
#परिभाषय USE_BUILTIN_FFS 0 /* not yet implemented in mingw-64 */
#यदि_अन्तः

#परिभाषय FFI_TARGET_SPECIFIC_राशिः_SPACE_आवण्टनम्
#यदि_अपरिभाषितम् _MSC_VER
#परिभाषय FFI_TARGET_HAS_COMPLEX_TYPE
#यदि_अन्तः

#यदि_परिभाषितम् X86_64
#परिभाषय FFI_TARGET_HAS_INT128
#यदि_अन्तः

/* The System V x86-64 psABI passes 8- and 16-byte vectors in SSE registers;
   this is implemented by the ffi64.c (FFI_UNIX64) backend only.  32-bit x86
   and the Windows x86-64 backend (ffiw64.c) do not marshal vectors.  */
#यदि defined(X86_64) && !defined(X86_WIN64)
#परिभाषय FFI_TARGET_HAS_सदिशम्_TYPE
#यदि_अन्तः

/* ---- Generic type definitions ----------------------------------------- */

#यदि_अपरिभाषितम् LIBFFI_ASM
#यदि_परिभाषितम् X86_WIN64
#यदि_परिभाषितम् _MSC_VER
प्रकारनिर्देश अचिह्नित __int64       ffi_arg;
प्रकारनिर्देश __int64                ffi_sarg;
#अन्यथा
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ     ffi_arg;
प्रकारनिर्देश दीर्घ दीर्घ              ffi_sarg;
#यदि_अन्तः
#अन्यथा
#यदि defined __x86_64__ && defined __ILP32__
#परिभाषय FFI_SIZEOF_ARG 8
#परिभाषय FFI_SIZEOF_JAVA_RAW  4
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ     ffi_arg;
प्रकारनिर्देश दीर्घ दीर्घ              ffi_sarg;
#अन्यथा
प्रकारनिर्देश अचिह्नित दीर्घ          ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ            ffi_sarg;
#यदि_अन्तः
#यदि_अन्तः

प्रकारनिर्देश गणना ffi_abi {
#यदि defined(X86_WIN64)
  FFI_FIRST_ABI = 0,
  FFI_WIN64,            /* sizeof(long double) == 8  - microsoft compilers */
  FFI_GNUW64,           /* sizeof(long double) == 16 - GNU compilers */
  FFI_चरमः_ABI,
#यदि_परिभाषितम् __GNUC__
  FFI_DEFAULT_ABI = FFI_GNUW64
#अन्यथा
  FFI_DEFAULT_ABI = FFI_WIN64
#यदि_अन्तः

#नोचेद्यदि defined(X86_64) || (defined (__x86_64__) && defined (X86_DARWIN))
  FFI_FIRST_ABI = 1,
  FFI_UNIX64,
  FFI_WIN64,
  FFI_EFI64 = FFI_WIN64,
  FFI_GNUW64,
  FFI_चरमः_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX64

#नोचेद्यदि defined(X86_WIN32)
  FFI_FIRST_ABI = 0,
  FFI_SYSV      = 1,
  FFI_STDCALL   = 2,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_MS_CDECL  = 5,
  FFI_PASCAL    = 6,
  FFI_पञ्जिकाकोशः  = 7,
  FFI_चरमः_ABI,
  FFI_DEFAULT_ABI = FFI_MS_CDECL
#अन्यथा
  FFI_FIRST_ABI = 0,
  FFI_SYSV      = 1,
  FFI_THISCALL  = 3,
  FFI_FASTCALL  = 4,
  FFI_STDCALL   = 5,
  FFI_PASCAL    = 6,
  FFI_पञ्जिकाकोशः  = 7,
  FFI_MS_CDECL  = 8,
  FFI_चरमः_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
#यदि_अन्तः
} ffi_abi;
#यदि_अन्तः

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_GO_CLOSURES 1

#परिभाषय FFI_TYPE_SMALL_STRUCT_1B (FFI_TYPE_चरमः + 1)
#परिभाषय FFI_TYPE_SMALL_STRUCT_2B (FFI_TYPE_चरमः + 2)
#परिभाषय FFI_TYPE_SMALL_STRUCT_4B (FFI_TYPE_चरमः + 3)
#परिभाषय FFI_TYPE_MS_STRUCT       (FFI_TYPE_चरमः + 4)

/* Tripwire: the win64.S / win64_intel.S return-value jump tables use one
   8-byte slot per code value and place the FFI_TYPE_SMALL_STRUCT_* pseudo-types
   (which are FFI_TYPE_LAST-relative) immediately after the generic FFI_TYPE_*
   codes.  Adding a new generic type bumps FFI_TYPE_LAST, shifts those codes,
   and opens a gap in the tables that silently misaligns small-struct returns.
   When this fires: add a matching E() slot for the new type in both win64.S
   and win64_intel.S, then bump FFI_X86_TYPE_LAST to match.  */
#परिभाषय FFI_X86_TYPE_चरमः FFI_TYPE_सदिशम्
#यदि FFI_TYPE_चरमः != FFI_X86_TYPE_चरमः
# error "new FFI_TYPE_* added: sync the win64.S/win64_intel.S jump tables and bump FFI_X86_TYPE_LAST"
#यदि_अन्तः

#यदि defined (X86_64) || defined(X86_WIN64) \
    || (defined (__x86_64__) && defined (X86_DARWIN))
/* 4 bytes of ENDBR64 + 7 bytes of LEA + 6 bytes of JMP + 7 bytes of NOP
   + 8 bytes of pointer.  */
# define FFI_TRAMPOLINE_आकार 32
# define FFI_NATIVE_RAW_API 0
#अन्यथा
/* 4 bytes of ENDBR32 + 5 bytes of MOV + 5 bytes of JMP + 2 unused
   bytes.  */
# define FFI_TRAMPOLINE_आकार 16
# define FFI_NATIVE_RAW_API 1  /* x86 has native raw api support */
#यदि_अन्तः

#यदि !defined(GENERATE_LIBFFI_MAP) && defined(__CET__)
# include <cet.h>
# यदि (__CET__ & 1) != 0
#   define ENDBR_PRESENT
# endif
# define _CET_NOTRACK notrack
#अन्यथा
# define _CET_ENDBR
# define _CET_NOTRACK
#यदि_अन्तः

#यदि_अन्तः

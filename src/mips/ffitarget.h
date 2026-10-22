/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for MIPS.

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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

#که_تعریف_شوی __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#تعریف_کړه _MIPS_SIM_ABI32		1
#تعریف_کړه _MIPS_SIM_NABI32	2
#تعریف_کړه _MIPS_SIM_ABI64		3
#وإلاإذا !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#د_که_پای

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#إذا !defined(_MIPS_SIM)
# error -- something is very wrong --
#وإلا
#  إذا (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  وإلا
#    إذا (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    وإلا
#     error -- this is an unsupported platform --
#    endif
#  endif
#د_که_پای

#که_تعریف_شوی FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#وإلا
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  إذا _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#د_که_پای

#تعریف_کړه FFI_موخه_HAS_COMPLEX_TYPE 1
#تعریف_کړه FFI_FLAG_بټونه 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#تعریف_کړه FFI_دلیلونه_D   FFI_TYPE_DOUBLE
#تعریف_کړه FFI_دلیلونه_F   FFI_TYPE_FLOAT
#تعریف_کړه FFI_دلیلونه_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#تعریف_کړه FFI_دلیلونه_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#تعریف_کړه FFI_دلیلونه_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#تعریف_کړه FFI_دلیلونه_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#تعریف_کړه FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#تعریف_کړه FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#إذا 0
/* The SGI assembler can't handle this.. */
#تعریف_کړه FFI_TYPE_STRUCT_DD (( FFI_دلیلونه_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#وإلا
/* ...so we calculate these by hand! */
#تعریف_کړه FFI_TYPE_STRUCT_D      61
#تعریف_کړه FFI_TYPE_STRUCT_F      45
#تعریف_کړه FFI_TYPE_STRUCT_DD     253
#تعریف_کړه FFI_TYPE_STRUCT_FF     173
#تعریف_کړه FFI_TYPE_STRUCT_FD     237
#تعریف_کړه FFI_TYPE_STRUCT_DF     189
#تعریف_کړه FFI_TYPE_STRUCT_SMALL  93
#تعریف_کړه FFI_TYPE_STRUCT_SMALL2 109

#تعریف_کړه FFI_TYPE_COMPLEX_SMALL    95
#تعریف_کړه FFI_TYPE_COMPLEX_SMALL2   111
#تعریف_کړه FFI_TYPE_COMPLEX_FF       47
#تعریف_کړه FFI_TYPE_COMPLEX_DD       63
#تعریف_کړه FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#تعریف_کړه FFI_TYPE_STRUCT_D_SOFT      317
#تعریف_کړه FFI_TYPE_STRUCT_F_SOFT      301
#تعریف_کړه FFI_TYPE_STRUCT_DD_SOFT     509
#تعریف_کړه FFI_TYPE_STRUCT_FF_SOFT     429
#تعریف_کړه FFI_TYPE_STRUCT_FD_SOFT     493
#تعریف_کړه FFI_TYPE_STRUCT_DF_SOFT     445
#تعریف_کړه FFI_TYPE_STRUCT_SOFT        16
#د_که_پای

#که_تعریف_شوی LIBFFI_ASM
#تعریف_کړه v0 $2
#تعریف_کړه v1 $3
#تعریف_کړه a0 $4
#تعریف_کړه a1 $5
#تعریف_کړه a2 $6
#تعریف_کړه a3 $7
#تعریف_کړه a4 $8		
#تعریف_کړه a5 $9		
#تعریف_کړه a6 $10		
#تعریف_کړه a7 $11		
#تعریف_کړه t0 $8
#تعریف_کړه t1 $9
#تعریف_کړه t2 $10
#تعریف_کړه t3 $11
#تعریف_کړه t4 $12		
#تعریف_کړه t5 $13
#تعریف_کړه t6 $14	
#تعریف_کړه t7 $15
#تعریف_کړه t8 $24
#تعریف_کړه t9 $25
#تعریف_کړه ra $31		

#که_تعریف_شوی FFI_MIPS_O32
# define REG_L	lw
# define REG_S	sw
# define SUBU	subu
# define ADDU	addu
# define SRL	srl
# define LI	li
#وإلا /* !FFI_MIPS_O32 */
# define REG_L	ld
# define REG_S	sd
# define SUBU	dsubu
# define ADDU	daddu
# define SRL	dsrl
# define LI 	dli
# إذا (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_چوکاټ_ALIGN 3
#  define FDE_ADDR_بایټونه .8byte
# وإلا
#  define LA la
#  define EH_چوکاټ_ALIGN 2
#  define FDE_ADDR_بایټونه .4byte
# endif /* _MIPS_SIM==_ABI64 */
#د_که_پای /* !FFI_MIPS_O32 */
#وإلا /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
تعريفنوع بینښه صحيح     ffi_arg __attribute__((__mode__(__SI__)));
تعريفنوع نښهدار   صحيح     ffi_sarg __attribute__((__mode__(__SI__)));
#وإلا
/* N32 and N64 frames have 64bit integer args */
تعريفنوع بینښه صحيح     ffi_arg __attribute__((__mode__(__DI__)));
تعريفنوع نښهدار   صحيح     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# وإلا
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
تعريفنوع __uint32_t ffi_arg;
تعريفنوع __int32_t ffi_sarg;
#  وإلا
/* N32 and N64 frames have 64bit integer args */
تعريفنوع __uint64_t ffi_arg;
تعريفنوع __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

تعريفنوع تعداد ffi_abi {
  FFI_لومړی_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_وروستی_ABI,

#که_تعریف_شوی FFI_MIPS_O32
#که_تعریف_شوی __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#وإلا
  FFI_DEFAULT_ABI = FFI_O32
#د_که_پای
#وإلا
# إذا _MIPS_SIM==_ABI64
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N64_SOFT_FLOAT
#  وإلا
  FFI_DEFAULT_ABI = FFI_N64
#  endif
# وإلا
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N32_SOFT_FLOAT
#  وإلا
  FFI_DEFAULT_ABI = FFI_N32
#  endif
# endif
#د_که_پای
} ffi_abi;

#تعریف_کړه FFI_EXTRA_CIF_FIELDS بینښه rstruct_flag; بینښه mips_nfixedargs
#تعریف_کړه FFI_موخه_SPECIFIC_VARIADIC
#د_که_پای /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_GO_CLOSURES 1
#تعریف_کړه FFI_NATIVE_RAW_API 0

#إذا defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_اندازه 20
#وإلا
# define FFI_TRAMPOLINE_اندازه 56
#د_که_پای

#د_که_پای


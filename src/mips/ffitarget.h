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

#جے_بنا_تعریف LIBFFI_نشانہ_H
#تعریف_کرو LIBFFI_نشانہ_H

#جے_بنا_تعریف LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جے_انت

#جے_تعریف_شدہ __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#تعریف_کرو _MIPS_SIM_ABI32		1
#تعریف_کرو _MIPS_SIM_NABI32	2
#تعریف_کرو _MIPS_SIM_ABI64		3
#وإلاإذا !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#جے_انت

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
#جے_انت

#جے_تعریف_شدہ FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#وإلا
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  إذا _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#جے_انت

#تعریف_کرو FFI_نشانہ_HAS_COMPLEX_TYPE 1
#تعریف_کرو FFI_FLAG_بٹ 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#تعریف_کرو FFI_دلیلاں_D   FFI_TYPE_DOUBLE
#تعریف_کرو FFI_دلیلاں_F   FFI_TYPE_FLOAT
#تعریف_کرو FFI_دلیلاں_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#تعریف_کرو FFI_دلیلاں_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#تعریف_کرو FFI_دلیلاں_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#تعریف_کرو FFI_دلیلاں_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#تعریف_کرو FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#تعریف_کرو FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#إذا 0
/* The SGI assembler can't handle this.. */
#تعریف_کرو FFI_TYPE_STRUCT_DD (( FFI_دلیلاں_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#وإلا
/* ...so we calculate these by hand! */
#تعریف_کرو FFI_TYPE_STRUCT_D      61
#تعریف_کرو FFI_TYPE_STRUCT_F      45
#تعریف_کرو FFI_TYPE_STRUCT_DD     253
#تعریف_کرو FFI_TYPE_STRUCT_FF     173
#تعریف_کرو FFI_TYPE_STRUCT_FD     237
#تعریف_کرو FFI_TYPE_STRUCT_DF     189
#تعریف_کرو FFI_TYPE_STRUCT_SMALL  93
#تعریف_کرو FFI_TYPE_STRUCT_SMALL2 109

#تعریف_کرو FFI_TYPE_COMPLEX_SMALL    95
#تعریف_کرو FFI_TYPE_COMPLEX_SMALL2   111
#تعریف_کرو FFI_TYPE_COMPLEX_FF       47
#تعریف_کرو FFI_TYPE_COMPLEX_DD       63
#تعریف_کرو FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#تعریف_کرو FFI_TYPE_STRUCT_D_SOFT      317
#تعریف_کرو FFI_TYPE_STRUCT_F_SOFT      301
#تعریف_کرو FFI_TYPE_STRUCT_DD_SOFT     509
#تعریف_کرو FFI_TYPE_STRUCT_FF_SOFT     429
#تعریف_کرو FFI_TYPE_STRUCT_FD_SOFT     493
#تعریف_کرو FFI_TYPE_STRUCT_DF_SOFT     445
#تعریف_کرو FFI_TYPE_STRUCT_SOFT        16
#جے_انت

#جے_تعریف_شدہ LIBFFI_ASM
#تعریف_کرو v0 $2
#تعریف_کرو v1 $3
#تعریف_کرو a0 $4
#تعریف_کرو a1 $5
#تعریف_کرو a2 $6
#تعریف_کرو a3 $7
#تعریف_کرو a4 $8		
#تعریف_کرو a5 $9		
#تعریف_کرو a6 $10		
#تعریف_کرو a7 $11		
#تعریف_کرو t0 $8
#تعریف_کرو t1 $9
#تعریف_کرو t2 $10
#تعریف_کرو t3 $11
#تعریف_کرو t4 $12		
#تعریف_کرو t5 $13
#تعریف_کرو t6 $14	
#تعریف_کرو t7 $15
#تعریف_کرو t8 $24
#تعریف_کرو t9 $25
#تعریف_کرو ra $31		

#جے_تعریف_شدہ FFI_MIPS_O32
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
#  define EH_چوکھٹا_ALIGN 3
#  define FDE_ADDR_بائٹاں .8byte
# وإلا
#  define LA la
#  define EH_چوکھٹا_ALIGN 2
#  define FDE_ADDR_بائٹاں .4byte
# endif /* _MIPS_SIM==_ABI64 */
#جے_انت /* !FFI_MIPS_O32 */
#وإلا /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
تعريفنوع بنانشانی صحيح     ffi_arg __attribute__((__mode__(__SI__)));
تعريفنوع نشانیوالا   صحيح     ffi_sarg __attribute__((__mode__(__SI__)));
#وإلا
/* N32 and N64 frames have 64bit integer args */
تعريفنوع بنانشانی صحيح     ffi_arg __attribute__((__mode__(__DI__)));
تعريفنوع نشانیوالا   صحيح     ffi_sarg __attribute__((__mode__(__DI__)));
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
  FFI_پہلا_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_آخری_ABI,

#جے_تعریف_شدہ FFI_MIPS_O32
#جے_تعریف_شدہ __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#وإلا
  FFI_DEFAULT_ABI = FFI_O32
#جے_انت
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
#جے_انت
} ffi_abi;

#تعریف_کرو FFI_EXTRA_CIF_FIELDS بنانشانی rstruct_flag; بنانشانی mips_nfixedargs
#تعریف_کرو FFI_نشانہ_SPECIFIC_VARIADIC
#جے_انت /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1
#تعریف_کرو FFI_NATIVE_RAW_API 0

#إذا defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_ناپ 20
#وإلا
# define FFI_TRAMPOLINE_ناپ 56
#جے_انت

#جے_انت


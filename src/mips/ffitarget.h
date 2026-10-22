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

#నిర్వచితం_కాకపోతే LIBFFI_లక్ష్యం_H
#నిర్వచించు LIBFFI_లక్ష్యం_H

#నిర్వచితం_కాకపోతే LIBFFI_H
#దోషం "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

#నిర్వచితమైతే __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#నిర్వచించు _MIPS_SIM_ABI32		1
#నిర్వచించు _MIPS_SIM_NABI32	2
#నిర్వచించు _MIPS_SIM_ABI64		3
#లేదావేళ !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#అయితే_ముగింపు

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#ఒకవేళ !defined(_MIPS_SIM)
# error -- something is very wrong --
#లేకపోతే
#  ఒకవేళ (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  లేకపోతే
#    ఒకవేళ (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    లేకపోతే
#     error -- this is an unsupported platform --
#    endif
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#లేకపోతే
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  ఒకవేళ _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#అయితే_ముగింపు

#నిర్వచించు FFI_లక్ష్యం_HAS_COMPLEX_TYPE 1
#నిర్వచించు FFI_FLAG_బిట్లు 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#నిర్వచించు FFI_వాదనలు_D   FFI_TYPE_DOUBLE
#నిర్వచించు FFI_వాదనలు_F   FFI_TYPE_FLOAT
#నిర్వచించు FFI_వాదనలు_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#నిర్వచించు FFI_వాదనలు_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#నిర్వచించు FFI_వాదనలు_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#నిర్వచించు FFI_వాదనలు_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#నిర్వచించు FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#నిర్వచించు FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#ఒకవేళ 0
/* The SGI assembler can't handle this.. */
#నిర్వచించు FFI_TYPE_STRUCT_DD (( FFI_వాదనలు_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#లేకపోతే
/* ...so we calculate these by hand! */
#నిర్వచించు FFI_TYPE_STRUCT_D      61
#నిర్వచించు FFI_TYPE_STRUCT_F      45
#నిర్వచించు FFI_TYPE_STRUCT_DD     253
#నిర్వచించు FFI_TYPE_STRUCT_FF     173
#నిర్వచించు FFI_TYPE_STRUCT_FD     237
#నిర్వచించు FFI_TYPE_STRUCT_DF     189
#నిర్వచించు FFI_TYPE_STRUCT_SMALL  93
#నిర్వచించు FFI_TYPE_STRUCT_SMALL2 109

#నిర్వచించు FFI_TYPE_COMPLEX_SMALL    95
#నిర్వచించు FFI_TYPE_COMPLEX_SMALL2   111
#నిర్వచించు FFI_TYPE_COMPLEX_FF       47
#నిర్వచించు FFI_TYPE_COMPLEX_DD       63
#నిర్వచించు FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#నిర్వచించు FFI_TYPE_STRUCT_D_SOFT      317
#నిర్వచించు FFI_TYPE_STRUCT_F_SOFT      301
#నిర్వచించు FFI_TYPE_STRUCT_DD_SOFT     509
#నిర్వచించు FFI_TYPE_STRUCT_FF_SOFT     429
#నిర్వచించు FFI_TYPE_STRUCT_FD_SOFT     493
#నిర్వచించు FFI_TYPE_STRUCT_DF_SOFT     445
#నిర్వచించు FFI_TYPE_STRUCT_SOFT        16
#అయితే_ముగింపు

#నిర్వచితమైతే LIBFFI_ASM
#నిర్వచించు v0 $2
#నిర్వచించు v1 $3
#నిర్వచించు a0 $4
#నిర్వచించు a1 $5
#నిర్వచించు a2 $6
#నిర్వచించు a3 $7
#నిర్వచించు a4 $8		
#నిర్వచించు a5 $9		
#నిర్వచించు a6 $10		
#నిర్వచించు a7 $11		
#నిర్వచించు t0 $8
#నిర్వచించు t1 $9
#నిర్వచించు t2 $10
#నిర్వచించు t3 $11
#నిర్వచించు t4 $12		
#నిర్వచించు t5 $13
#నిర్వచించు t6 $14	
#నిర్వచించు t7 $15
#నిర్వచించు t8 $24
#నిర్వచించు t9 $25
#నిర్వచించు ra $31		

#నిర్వచితమైతే FFI_MIPS_O32
# define REG_L	lw
# define REG_S	sw
# define SUBU	subu
# define ADDU	addu
# define SRL	srl
# define LI	li
#లేకపోతే /* !FFI_MIPS_O32 */
# define REG_L	ld
# define REG_S	sd
# define SUBU	dsubu
# define ADDU	daddu
# define SRL	dsrl
# define LI 	dli
# ఒకవేళ (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_చట్రం_ALIGN 3
#  define FDE_ADDR_బైట్లు .8byte
# లేకపోతే
#  define LA la
#  define EH_చట్రం_ALIGN 2
#  define FDE_ADDR_బైట్లు .4byte
# endif /* _MIPS_SIM==_ABI64 */
#అయితే_ముగింపు /* !FFI_MIPS_O32 */
#లేకపోతే /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం     ffi_arg __attribute__((__mode__(__SI__)));
రకనిర్వచనం గుర్తుగల   పూర్ణాంకం     ffi_sarg __attribute__((__mode__(__SI__)));
#లేకపోతే
/* N32 and N64 frames have 64bit integer args */
రకనిర్వచనం గుర్తులేని పూర్ణాంకం     ffi_arg __attribute__((__mode__(__DI__)));
రకనిర్వచనం గుర్తుగల   పూర్ణాంకం     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# లేకపోతే
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
రకనిర్వచనం __uint32_t ffi_arg;
రకనిర్వచనం __int32_t ffi_sarg;
#  లేకపోతే
/* N32 and N64 frames have 64bit integer args */
రకనిర్వచనం __uint64_t ffi_arg;
రకనిర్వచనం __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

రకనిర్వచనం గణన ffi_abi {
  FFI_మొదటి_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_చివరి_ABI,

#నిర్వచితమైతే FFI_MIPS_O32
#నిర్వచితమైతే __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#లేకపోతే
  FFI_DEFAULT_ABI = FFI_O32
#అయితే_ముగింపు
#లేకపోతే
# ఒకవేళ _MIPS_SIM==_ABI64
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N64_SOFT_FLOAT
#  లేకపోతే
  FFI_DEFAULT_ABI = FFI_N64
#  endif
# లేకపోతే
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N32_SOFT_FLOAT
#  లేకపోతే
  FFI_DEFAULT_ABI = FFI_N32
#  endif
# endif
#అయితే_ముగింపు
} ffi_abi;

#నిర్వచించు FFI_EXTRA_CIF_FIELDS గుర్తులేని rstruct_flag; గుర్తులేని mips_nfixedargs
#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC
#అయితే_ముగింపు /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_GO_CLOSURES 1
#నిర్వచించు FFI_NATIVE_RAW_API 0

#ఒకవేళ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_కొలత 20
#లేకపోతే
# define FFI_TRAMPOLINE_కొలత 56
#అయితే_ముగింపు

#అయితే_ముగింపు


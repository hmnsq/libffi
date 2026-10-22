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

#यदि_अपरिभाषितम् LIBFFI_TARGET_H
#परिभाषय LIBFFI_TARGET_H

#यदि_अपरिभाषितम् LIBFFI_H
#दोषः "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#यदि_अन्तः

#यदि_परिभाषितम् __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#परिभाषय _MIPS_SIM_ABI32		1
#परिभाषय _MIPS_SIM_NABI32	2
#परिभाषय _MIPS_SIM_ABI64		3
#नोचेद्यदि !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#यदि_अन्तः

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#यदि !defined(_MIPS_SIM)
# error -- something is very wrong --
#अन्यथा
#  यदि (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  अन्यथा
#    यदि (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    अन्यथा
#     error -- this is an unsupported platform --
#    endif
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#अन्यथा
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  यदि _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#यदि_अन्तः

#परिभाषय FFI_TARGET_HAS_COMPLEX_TYPE 1
#परिभाषय FFI_FLAG_द्वयङ्काः 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#परिभाषय FFI_तर्काः_D   FFI_TYPE_DOUBLE
#परिभाषय FFI_तर्काः_F   FFI_TYPE_FLOAT
#परिभाषय FFI_तर्काः_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#परिभाषय FFI_तर्काः_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#परिभाषय FFI_तर्काः_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#परिभाषय FFI_तर्काः_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#परिभाषय FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#परिभाषय FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#यदि 0
/* The SGI assembler can't handle this.. */
#परिभाषय FFI_TYPE_STRUCT_DD (( FFI_तर्काः_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#अन्यथा
/* ...so we calculate these by hand! */
#परिभाषय FFI_TYPE_STRUCT_D      61
#परिभाषय FFI_TYPE_STRUCT_F      45
#परिभाषय FFI_TYPE_STRUCT_DD     253
#परिभाषय FFI_TYPE_STRUCT_FF     173
#परिभाषय FFI_TYPE_STRUCT_FD     237
#परिभाषय FFI_TYPE_STRUCT_DF     189
#परिभाषय FFI_TYPE_STRUCT_SMALL  93
#परिभाषय FFI_TYPE_STRUCT_SMALL2 109

#परिभाषय FFI_TYPE_COMPLEX_SMALL    95
#परिभाषय FFI_TYPE_COMPLEX_SMALL2   111
#परिभाषय FFI_TYPE_COMPLEX_FF       47
#परिभाषय FFI_TYPE_COMPLEX_DD       63
#परिभाषय FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#परिभाषय FFI_TYPE_STRUCT_D_SOFT      317
#परिभाषय FFI_TYPE_STRUCT_F_SOFT      301
#परिभाषय FFI_TYPE_STRUCT_DD_SOFT     509
#परिभाषय FFI_TYPE_STRUCT_FF_SOFT     429
#परिभाषय FFI_TYPE_STRUCT_FD_SOFT     493
#परिभाषय FFI_TYPE_STRUCT_DF_SOFT     445
#परिभाषय FFI_TYPE_STRUCT_SOFT        16
#यदि_अन्तः

#यदि_परिभाषितम् LIBFFI_ASM
#परिभाषय v0 $2
#परिभाषय v1 $3
#परिभाषय a0 $4
#परिभाषय a1 $5
#परिभाषय a2 $6
#परिभाषय a3 $7
#परिभाषय a4 $8		
#परिभाषय a5 $9		
#परिभाषय a6 $10		
#परिभाषय a7 $11		
#परिभाषय t0 $8
#परिभाषय t1 $9
#परिभाषय t2 $10
#परिभाषय t3 $11
#परिभाषय t4 $12		
#परिभाषय t5 $13
#परिभाषय t6 $14	
#परिभाषय t7 $15
#परिभाषय t8 $24
#परिभाषय t9 $25
#परिभाषय ra $31		

#यदि_परिभाषितम् FFI_MIPS_O32
# define REG_L	lw
# define REG_S	sw
# define SUBU	subu
# define ADDU	addu
# define SRL	srl
# define LI	li
#अन्यथा /* !FFI_MIPS_O32 */
# define REG_L	ld
# define REG_S	sd
# define SUBU	dsubu
# define ADDU	daddu
# define SRL	dsrl
# define LI 	dli
# यदि (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_FRAME_ALIGN 3
#  define FDE_ADDR_अष्टकानि .8byte
# अन्यथा
#  define LA la
#  define EH_FRAME_ALIGN 2
#  define FDE_ADDR_अष्टकानि .4byte
# endif /* _MIPS_SIM==_ABI64 */
#यदि_अन्तः /* !FFI_MIPS_O32 */
#अन्यथा /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
प्रकारनिर्देश अचिह्नित पूर्णाङ्क     ffi_arg __attribute__((__mode__(__SI__)));
प्रकारनिर्देश चिह्नित   पूर्णाङ्क     ffi_sarg __attribute__((__mode__(__SI__)));
#अन्यथा
/* N32 and N64 frames have 64bit integer args */
प्रकारनिर्देश अचिह्नित पूर्णाङ्क     ffi_arg __attribute__((__mode__(__DI__)));
प्रकारनिर्देश चिह्नित   पूर्णाङ्क     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# अन्यथा
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
प्रकारनिर्देश __uint32_t ffi_arg;
प्रकारनिर्देश __int32_t ffi_sarg;
#  अन्यथा
/* N32 and N64 frames have 64bit integer args */
प्रकारनिर्देश __uint64_t ffi_arg;
प्रकारनिर्देश __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

प्रकारनिर्देश गणना ffi_abi {
  FFI_FIRST_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_चरमः_ABI,

#यदि_परिभाषितम् FFI_MIPS_O32
#यदि_परिभाषितम् __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#अन्यथा
  FFI_DEFAULT_ABI = FFI_O32
#यदि_अन्तः
#अन्यथा
# यदि _MIPS_SIM==_ABI64
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N64_SOFT_FLOAT
#  अन्यथा
  FFI_DEFAULT_ABI = FFI_N64
#  endif
# अन्यथा
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N32_SOFT_FLOAT
#  अन्यथा
  FFI_DEFAULT_ABI = FFI_N32
#  endif
# endif
#यदि_अन्तः
} ffi_abi;

#परिभाषय FFI_EXTRA_CIF_FIELDS अचिह्नित rstruct_flag; अचिह्नित mips_nfixedargs
#परिभाषय FFI_TARGET_SPECIFIC_VARIADIC
#यदि_अन्तः /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_GO_CLOSURES 1
#परिभाषय FFI_NATIVE_RAW_API 0

#यदि defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_आकार 20
#अन्यथा
# define FFI_TRAMPOLINE_आकार 56
#यदि_अन्तः

#यदि_अन्तः


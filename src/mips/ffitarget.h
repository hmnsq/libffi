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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ಗುರಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_ಗುರಿ_H

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_H
#ದೋಷ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#ವ್ಯಾಖ್ಯಾನಿಸು _MIPS_SIM_ABI32		1
#ವ್ಯಾಖ್ಯಾನಿಸು _MIPS_SIM_NABI32	2
#ವ್ಯಾಖ್ಯಾನಿಸು _MIPS_SIM_ABI64		3
#ಅಥವಾಆದರೆ !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#ಆದರೆ_ಅಂತ್ಯ

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#ಆದರೆ !defined(_MIPS_SIM)
# error -- something is very wrong --
#ಇಲ್ಲದಿದ್ದರೆ
#  ಆದರೆ (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  ಇಲ್ಲದಿದ್ದರೆ
#    ಆದರೆ (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    ಇಲ್ಲದಿದ್ದರೆ
#     error -- this is an unsupported platform --
#    endif
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#ಇಲ್ಲದಿದ್ದರೆ
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  ಆದರೆ _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_COMPLEX_TYPE 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_FLAG_ಬಿಟ್ಟುಗಳು 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_D   FFI_TYPE_DOUBLE
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_F   FFI_TYPE_FLOAT
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ವಾದಗಳು_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#ಆದರೆ 0
/* The SGI assembler can't handle this.. */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_DD (( FFI_ವಾದಗಳು_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#ಇಲ್ಲದಿದ್ದರೆ
/* ...so we calculate these by hand! */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_D      61
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_F      45
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_DD     253
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_FF     173
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_FD     237
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_DF     189
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_SMALL  93
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_SMALL2 109

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_COMPLEX_SMALL    95
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_COMPLEX_SMALL2   111
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_COMPLEX_FF       47
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_COMPLEX_DD       63
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_D_SOFT      317
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_F_SOFT      301
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_DD_SOFT     509
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_FF_SOFT     429
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_FD_SOFT     493
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_DF_SOFT     445
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_STRUCT_SOFT        16
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ LIBFFI_ASM
#ವ್ಯಾಖ್ಯಾನಿಸು v0 $2
#ವ್ಯಾಖ್ಯಾನಿಸು v1 $3
#ವ್ಯಾಖ್ಯಾನಿಸು a0 $4
#ವ್ಯಾಖ್ಯಾನಿಸು a1 $5
#ವ್ಯಾಖ್ಯಾನಿಸು a2 $6
#ವ್ಯಾಖ್ಯಾನಿಸು a3 $7
#ವ್ಯಾಖ್ಯಾನಿಸು a4 $8		
#ವ್ಯಾಖ್ಯಾನಿಸು a5 $9		
#ವ್ಯಾಖ್ಯಾನಿಸು a6 $10		
#ವ್ಯಾಖ್ಯಾನಿಸು a7 $11		
#ವ್ಯಾಖ್ಯಾನಿಸು t0 $8
#ವ್ಯಾಖ್ಯಾನಿಸು t1 $9
#ವ್ಯಾಖ್ಯಾನಿಸು t2 $10
#ವ್ಯಾಖ್ಯಾನಿಸು t3 $11
#ವ್ಯಾಖ್ಯಾನಿಸು t4 $12		
#ವ್ಯಾಖ್ಯಾನಿಸು t5 $13
#ವ್ಯಾಖ್ಯಾನಿಸು t6 $14	
#ವ್ಯಾಖ್ಯಾನಿಸು t7 $15
#ವ್ಯಾಖ್ಯಾನಿಸು t8 $24
#ವ್ಯಾಖ್ಯಾನಿಸು t9 $25
#ವ್ಯಾಖ್ಯಾನಿಸು ra $31		

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
# define REG_L	lw
# define REG_S	sw
# define SUBU	subu
# define ADDU	addu
# define SRL	srl
# define LI	li
#ಇಲ್ಲದಿದ್ದರೆ /* !FFI_MIPS_O32 */
# define REG_L	ld
# define REG_S	sd
# define SUBU	dsubu
# define ADDU	daddu
# define SRL	dsrl
# define LI 	dli
# ಆದರೆ (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_ಚೌಕಟ್ಟು_ALIGN 3
#  define FDE_ADDR_ಬೈಟುಗಳು .8byte
# ಇಲ್ಲದಿದ್ದರೆ
#  define LA la
#  define EH_ಚೌಕಟ್ಟು_ALIGN 2
#  define FDE_ADDR_ಬೈಟುಗಳು .4byte
# endif /* _MIPS_SIM==_ABI64 */
#ಆದರೆ_ಅಂತ್ಯ /* !FFI_MIPS_O32 */
#ಇಲ್ಲದಿದ್ದರೆ /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ     ffi_arg __attribute__((__mode__(__SI__)));
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ   ಪೂರ್ಣಾಂಕ     ffi_sarg __attribute__((__mode__(__SI__)));
#ಇಲ್ಲದಿದ್ದರೆ
/* N32 and N64 frames have 64bit integer args */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ     ffi_arg __attribute__((__mode__(__DI__)));
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ   ಪೂರ್ಣಾಂಕ     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# ಇಲ್ಲದಿದ್ದರೆ
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __uint32_t ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __int32_t ffi_sarg;
#  ಇಲ್ಲದಿದ್ದರೆ
/* N32 and N64 frames have 64bit integer args */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __uint64_t ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
  FFI_ಮೊದಲ_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_ಕೊನೆಯ_ABI,

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_O32
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
# ಆದರೆ _MIPS_SIM==_ABI64
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N64_SOFT_FLOAT
#  ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_N64
#  endif
# ಇಲ್ಲದಿದ್ದರೆ
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N32_SOFT_FLOAT
#  ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_N32
#  endif
# endif
#ಆದರೆ_ಅಂತ್ಯ
} ffi_abi;

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EXTRA_CIF_FIELDS ಚಿಹ್ನೆರಹಿತ rstruct_flag; ಚಿಹ್ನೆರಹಿತ mips_nfixedargs
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_VARIADIC
#ಆದರೆ_ಅಂತ್ಯ /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0

#ಆದರೆ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_ಗಾತ್ರ 20
#ಇಲ್ಲದಿದ್ದರೆ
# define FFI_TRAMPOLINE_ಗಾತ್ರ 56
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ


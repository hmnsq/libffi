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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ਨਿਸ਼ਾਨਾ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_ਨਿਸ਼ਾਨਾ_H

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_H
#ਗਲਤੀ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _MIPS_SIM_ABI32		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _MIPS_SIM_NABI32	2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _MIPS_SIM_ABI64		3
#ਜਾਂਜੇ !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#ਜੇ_ਅੰਤ

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#ਜੇ !defined(_MIPS_SIM)
# error -- something is very wrong --
#ਨਹੀਂਤਾਂ
#  ਜੇ (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  ਨਹੀਂਤਾਂ
#    ਜੇ (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    ਨਹੀਂਤਾਂ
#     error -- this is an unsupported platform --
#    endif
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#ਨਹੀਂਤਾਂ
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  ਜੇ _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_FLAG_ਬਿੱਟ 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_D   FFI_TYPE_DOUBLE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_F   FFI_TYPE_FLOAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਦਲੀਲਾਂ_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#ਜੇ 0
/* The SGI assembler can't handle this.. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_DD (( FFI_ਦਲੀਲਾਂ_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#ਨਹੀਂਤਾਂ
/* ...so we calculate these by hand! */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_D      61
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_F      45
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_DD     253
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_FF     173
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_FD     237
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_DF     189
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_SMALL  93
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_SMALL2 109

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_COMPLEX_SMALL    95
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_COMPLEX_SMALL2   111
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_COMPLEX_FF       47
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_COMPLEX_DD       63
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_D_SOFT      317
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_F_SOFT      301
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_DD_SOFT     509
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_FF_SOFT     429
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_FD_SOFT     493
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_DF_SOFT     445
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_STRUCT_SOFT        16
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ v0 $2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ v1 $3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a0 $4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a1 $5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a2 $6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a3 $7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a4 $8		
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a5 $9		
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a6 $10		
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ a7 $11		
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t0 $8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t1 $9
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t2 $10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t3 $11
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t4 $12		
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t5 $13
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t6 $14	
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t7 $15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t8 $24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ t9 $25
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ra $31		

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
# define REG_L	lw
# define REG_S	sw
# define SUBU	subu
# define ADDU	addu
# define SRL	srl
# define LI	li
#ਨਹੀਂਤਾਂ /* !FFI_MIPS_O32 */
# define REG_L	ld
# define REG_S	sd
# define SUBU	dsubu
# define ADDU	daddu
# define SRL	dsrl
# define LI 	dli
# ਜੇ (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_ਚੌਖਟਾ_ALIGN 3
#  define FDE_ADDR_ਬਾਈਟਾਂ .8byte
# ਨਹੀਂਤਾਂ
#  define LA la
#  define EH_ਚੌਖਟਾ_ALIGN 2
#  define FDE_ADDR_ਬਾਈਟਾਂ .4byte
# endif /* _MIPS_SIM==_ABI64 */
#ਜੇ_ਅੰਤ /* !FFI_MIPS_O32 */
#ਨਹੀਂਤਾਂ /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ     ffi_arg __attribute__((__mode__(__SI__)));
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ   ਪੂਰਨ     ffi_sarg __attribute__((__mode__(__SI__)));
#ਨਹੀਂਤਾਂ
/* N32 and N64 frames have 64bit integer args */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ     ffi_arg __attribute__((__mode__(__DI__)));
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ   ਪੂਰਨ     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# ਨਹੀਂਤਾਂ
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __uint32_t ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __int32_t ffi_sarg;
#  ਨਹੀਂਤਾਂ
/* N32 and N64 frames have 64bit integer args */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __uint64_t ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi {
  FFI_ਪਹਿਲਾ_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_ਆਖਰੀ_ABI,

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_O32
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
# ਜੇ _MIPS_SIM==_ABI64
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N64_SOFT_FLOAT
#  ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_N64
#  endif
# ਨਹੀਂਤਾਂ
#  ifdef __mips_soft_float
  FFI_DEFAULT_ABI = FFI_N32_SOFT_FLOAT
#  ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_N32
#  endif
# endif
#ਜੇ_ਅੰਤ
} ffi_abi;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_EXTRA_CIF_FIELDS ਬਿਨਾਨਿਸ਼ਾਨੀ rstruct_flag; ਬਿਨਾਨਿਸ਼ਾਨੀ mips_nfixedargs
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_VARIADIC
#ਜੇ_ਅੰਤ /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_GO_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_NATIVE_RAW_API 0

#ਜੇ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_ਮਾਪ 20
#ਨਹੀਂਤਾਂ
# define FFI_TRAMPOLINE_ਮਾਪ 56
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ


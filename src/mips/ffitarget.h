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

#अगर_अघोषित LIBFFI_TARGET_H
#घोषणा LIBFFI_TARGET_H

#अगर_अघोषित LIBFFI_H
#त्रुटि "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#पूर्ण_अगर

#अगर_घोषित __rtems__
/*
 * Subprogram calling convention - copied from sgidefs.h
 */
#घोषणा _MIPS_SIM_ABI32		1
#घोषणा _MIPS_SIM_NABI32	2
#घोषणा _MIPS_SIM_ABI64		3
#या_अगर !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__linux__)
# include <sgidefs.h>
#पूर्ण_अगर

#  ifndef _ABIN32
#    define _ABIN32 _MIPS_SIM_NABI32
#  endif
#  ifndef _ABI64
#    define _ABI64 _MIPS_SIM_ABI64
#  endif
#  ifndef _ABIO32
#    define _ABIO32 _MIPS_SIM_ABI32
#  endif

#अगर !defined(_MIPS_SIM)
# error -- something is very wrong --
#अन्यथा
#  अगर (_MIPS_SIM==_ABIN32 && defined(_ABIN32)) || (_MIPS_SIM==_ABI64 && defined(_ABI64))
#    define FFI_MIPS_N32
#  अन्यथा
#    अगर (_MIPS_SIM==_ABIO32 && defined(_ABIO32))
#      define FFI_MIPS_O32
#    अन्यथा
#     error -- this is an unsupported platform --
#    endif
#  endif
#पूर्ण_अगर

#अगर_घोषित FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
#  define FFI_SIZEOF_ARG    4
#अन्यथा
/* N32 and N64 frames have 64bit integer args */
#  define FFI_SIZEOF_ARG    8
#  अगर _MIPS_SIM == _ABIN32
#    define FFI_SIZEOF_JAVA_RAW  4
#  endif
#पूर्ण_अगर

#घोषणा FFI_TARGET_HAS_COMPLEX_TYPE 1
#घोषणा FFI_FLAG_BITS 2

/* SGI's strange assembler requires that we multiply by 4 rather 
   than shift left by FFI_FLAG_BITS */

#घोषणा FFI_ARGS_D   FFI_TYPE_DOUBLE
#घोषणा FFI_ARGS_F   FFI_TYPE_FLOAT
#घोषणा FFI_ARGS_DD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_DOUBLE
#घोषणा FFI_ARGS_FF  FFI_TYPE_FLOAT * 4 +  FFI_TYPE_FLOAT
#घोषणा FFI_ARGS_FD  FFI_TYPE_DOUBLE * 4 + FFI_TYPE_FLOAT
#घोषणा FFI_ARGS_DF  FFI_TYPE_FLOAT * 4 + FFI_TYPE_DOUBLE

/* Needed for N32 structure returns */
#घोषणा FFI_TYPE_SMALLSTRUCT  FFI_TYPE_UINT8
#घोषणा FFI_TYPE_SMALLSTRUCT2 FFI_TYPE_SINT8

#अगर 0
/* The SGI assembler can't handle this.. */
#घोषणा FFI_TYPE_STRUCT_DD (( FFI_ARGS_DD ) << 4) + FFI_TYPE_STRUCT
/* (and so on) */
#अन्यथा
/* ...so we calculate these by hand! */
#घोषणा FFI_TYPE_STRUCT_D      61
#घोषणा FFI_TYPE_STRUCT_F      45
#घोषणा FFI_TYPE_STRUCT_DD     253
#घोषणा FFI_TYPE_STRUCT_FF     173
#घोषणा FFI_TYPE_STRUCT_FD     237
#घोषणा FFI_TYPE_STRUCT_DF     189
#घोषणा FFI_TYPE_STRUCT_SMALL  93
#घोषणा FFI_TYPE_STRUCT_SMALL2 109

#घोषणा FFI_TYPE_COMPLEX_SMALL    95
#घोषणा FFI_TYPE_COMPLEX_SMALL2   111
#घोषणा FFI_TYPE_COMPLEX_FF       47
#घोषणा FFI_TYPE_COMPLEX_DD       63
#घोषणा FFI_TYPE_COMPLEX_LDLD     79

/* and for n32 soft float, add 16 * 2^4 */
#घोषणा FFI_TYPE_STRUCT_D_SOFT      317
#घोषणा FFI_TYPE_STRUCT_F_SOFT      301
#घोषणा FFI_TYPE_STRUCT_DD_SOFT     509
#घोषणा FFI_TYPE_STRUCT_FF_SOFT     429
#घोषणा FFI_TYPE_STRUCT_FD_SOFT     493
#घोषणा FFI_TYPE_STRUCT_DF_SOFT     445
#घोषणा FFI_TYPE_STRUCT_SOFT        16
#पूर्ण_अगर

#अगर_घोषित LIBFFI_ASM
#घोषणा v0 $2
#घोषणा v1 $3
#घोषणा a0 $4
#घोषणा a1 $5
#घोषणा a2 $6
#घोषणा a3 $7
#घोषणा a4 $8		
#घोषणा a5 $9		
#घोषणा a6 $10		
#घोषणा a7 $11		
#घोषणा t0 $8
#घोषणा t1 $9
#घोषणा t2 $10
#घोषणा t3 $11
#घोषणा t4 $12		
#घोषणा t5 $13
#घोषणा t6 $14	
#घोषणा t7 $15
#घोषणा t8 $24
#घोषणा t9 $25
#घोषणा ra $31		

#अगर_घोषित FFI_MIPS_O32
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
# अगर (_MIPS_SIM==_ABI64)
#  define LA dla
#  define EH_FRAME_ALIGN 3
#  define FDE_ADDR_BYTES .8byte
# अन्यथा
#  define LA la
#  define EH_FRAME_ALIGN 2
#  define FDE_ADDR_BYTES .4byte
# endif /* _MIPS_SIM==_ABI64 */
#पूर्ण_अगर /* !FFI_MIPS_O32 */
#अन्यथा /* !LIBFFI_ASM */
# ifdef __GNUC__
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
प्रकार अचिन्हित पूर्णांक     ffi_arg __attribute__((__mode__(__SI__)));
प्रकार चिन्हित   पूर्णांक     ffi_sarg __attribute__((__mode__(__SI__)));
#अन्यथा
/* N32 and N64 frames have 64bit integer args */
प्रकार अचिन्हित पूर्णांक     ffi_arg __attribute__((__mode__(__DI__)));
प्रकार चिन्हित   पूर्णांक     ffi_sarg __attribute__((__mode__(__DI__)));
#  endif
# अन्यथा
#  ifdef FFI_MIPS_O32
/* O32 stack frames have 32bit integer args */
प्रकार __uint32_t ffi_arg;
प्रकार __int32_t ffi_sarg;
#  अन्यथा
/* N32 and N64 frames have 64bit integer args */
प्रकार __uint64_t ffi_arg;
प्रकार __int64_t ffi_sarg;
#  endif
# endif /* __GNUC__ */

प्रकार क्रमागत ffi_abi {
  FFI_FIRST_ABI = 0,
  FFI_O32,
  FFI_N32,
  FFI_N64,
  FFI_O32_SOFT_FLOAT,
  FFI_N32_SOFT_FLOAT,
  FFI_N64_SOFT_FLOAT,
  FFI_LAST_ABI,

#अगर_घोषित FFI_MIPS_O32
#अगर_घोषित __mips_soft_float
  FFI_DEFAULT_ABI = FFI_O32_SOFT_FLOAT
#अन्यथा
  FFI_DEFAULT_ABI = FFI_O32
#पूर्ण_अगर
#अन्यथा
# अगर _MIPS_SIM==_ABI64
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
#पूर्ण_अगर
} ffi_abi;

#घोषणा FFI_EXTRA_CIF_FIELDS अचिन्हित rstruct_flag; अचिन्हित mips_nfixedargs
#घोषणा FFI_TARGET_SPECIFIC_VARIADIC
#पूर्ण_अगर /* !LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#घोषणा FFI_CLOSURES 1
#घोषणा FFI_GO_CLOSURES 1
#घोषणा FFI_NATIVE_RAW_API 0

#अगर defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
# define FFI_TRAMPOLINE_आकार 20
#अन्यथा
# define FFI_TRAMPOLINE_आकार 56
#पूर्ण_अगर

#पूर्ण_अगर


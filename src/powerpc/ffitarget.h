/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2026  Anthony Green
                 Copyright (C) 2007, 2008, 2010 Free Software Foundation, Inc
                 Copyright (c) 1996-2003  Red Hat, Inc.

   Target configuration macros for PowerPC.

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

#यदि defined (POWERPC) && defined (__powerpc64__)	/* linux64 */
#यदि_अपरिभाषितम् POWERPC64
#परिभाषय POWERPC64
#यदि_अन्तः
#नोचेद्यदि defined (POWERPC_DARWIN) && defined (__ppc64__)	/* Darwin64 */
#यदि_अपरिभाषितम् POWERPC64
#परिभाषय POWERPC64
#यदि_अन्तः
#यदि_अपरिभाषितम् POWERPC_DARWIN64
#परिभाषय POWERPC_DARWIN64
#यदि_अन्तः
#नोचेद्यदि defined (POWERPC_AIX) && defined (__64BIT__)	/* AIX64 */
#यदि_अपरिभाषितम् POWERPC64
#परिभाषय POWERPC64
#यदि_अन्तः
#यदि_अन्तः

#यदि_अपरिभाषितम् LIBFFI_ASM
प्रकारनिर्देश अचिह्नित दीर्घ          ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ            ffi_sarg;

प्रकारनिर्देश गणना ffi_abi {
  FFI_FIRST_ABI = 0,

#यदि defined (POWERPC_AIX)
  FFI_AIX,
  FFI_DARWIN,
  FFI_DEFAULT_ABI = FFI_AIX,
  FFI_चरमः_ABI

#नोचेद्यदि defined (POWERPC_DARWIN)
  FFI_AIX,
  FFI_DARWIN,
  FFI_DEFAULT_ABI = FFI_DARWIN,
  FFI_चरमः_ABI

#अन्यथा
  /* The FFI_COMPAT values are used by old code.  Since libffi may be
     a shared library we have to support old values for backwards
     compatibility.  */
  FFI_COMPAT_SYSV,
  FFI_COMPAT_GCC_SYSV,
  FFI_COMPAT_LINUX64,
  FFI_COMPAT_LINUX,
  FFI_COMPAT_LINUX_SOFT_FLOAT,

# यदि defined (POWERPC64)
  /* This bit, always set in new code, must not be set in any of the
     old FFI_COMPAT values that might be used for 64-bit linux.  We
     only need worry about FFI_COMPAT_LINUX64, but to be safe avoid
     all old values.  */
  FFI_LINUX = 8,
  /* This and following bits can reuse FFI_COMPAT values.  */
  FFI_LINUX_STRUCT_ALIGN = 1,
  FFI_LINUX_LONG_DOUBLE_128 = 2,
  FFI_LINUX_LONG_DOUBLE_IEEE128 = 4,
  FFI_DEFAULT_ABI = (FFI_LINUX
#  ifdef __STRUCT_PARM_ALIGN__
		     | FFI_LINUX_STRUCT_ALIGN
#  endif
#  ifdef __LONG_DOUBLE_128__
		     | FFI_LINUX_LONG_DOUBLE_128
#   ifdef __LONG_DOUBLE_IEEE128__
		     | FFI_LINUX_LONG_DOUBLE_IEEE128
#   endif
#  endif
		     ),
  FFI_चरमः_ABI = 16

# अन्यथा
  /* This bit, always set in new code, must not be set in any of the
     old FFI_COMPAT values that might be used for 32-bit linux/sysv/bsd.  */
  FFI_SYSV = 8,
  /* This and following bits can reuse FFI_COMPAT values.  */
  FFI_SYSV_SOFT_FLOAT = 1,
  FFI_SYSV_STRUCT_RET = 2,
  FFI_SYSV_IBM_LONG_DOUBLE = 4,
  FFI_SYSV_LONG_DOUBLE_128 = 16,

  FFI_DEFAULT_ABI = (FFI_SYSV
#  ifdef __NO_FPRS__
		     | FFI_SYSV_SOFT_FLOAT
#  endif
#  यदि (defined (__SVR4_STRUCT_RETURN)					\
       || defined (POWERPC_FREEBSD) && !defined (__AIX_STRUCT_RETURN))
		     | FFI_SYSV_STRUCT_RET
#  endif
#  यदि __LDBL_MANT_DIG__ == 106
		     | FFI_SYSV_IBM_LONG_DOUBLE
#  endif
#  ifdef __LONG_DOUBLE_128__
		     | FFI_SYSV_LONG_DOUBLE_128
#  endif
		     ),
  FFI_चरमः_ABI = 32
# endif
#यदि_अन्तः

} ffi_abi;
#यदि_अन्तः

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_NATIVE_RAW_API 0
#यदि defined (POWERPC) || defined (POWERPC_FREEBSD)
# define FFI_GO_CLOSURES 1
# define FFI_TARGET_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS अचिह्नित nfixedargs
#यदि_अन्तः
#यदि defined (POWERPC_AIX)
# define FFI_GO_CLOSURES 1
#यदि_अन्तः

/* Complex types are supported on ELFv2 (the only PowerPC64 variant where
   the assembly and C-side passing/return logic has been wired up).  Under
   ELFv2, float/double _Complex are passed and returned as a 2-element
   homogeneous floating-point aggregate, but each scalar half consumes a
   GPR shadow slot of its own — i.e. the same way the underlying C ABI
   handles them, which is what GCC's split_complex_arg emits.  */
#यदि defined(POWERPC64) && _CALL_ELF == 2
# define FFI_TARGET_HAS_COMPLEX_TYPE
#यदि_अन्तः

#यदि _CALL_ELF == 2
# define FFI_TRAMPOLINE_आकार 32
#अन्यथा
# यदि defined(POWERPC64) || defined(POWERPC_AIX)
#  यदि defined(POWERPC_DARWIN64)
#    define FFI_TRAMPOLINE_आकार 48
#  अन्यथा
#    define FFI_TRAMPOLINE_आकार 24
#  endif
# अन्यथा /* POWERPC || POWERPC_AIX */
#  define FFI_TRAMPOLINE_आकार 40
# endif
#यदि_अन्तः

#यदि_अन्तः

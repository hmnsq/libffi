/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#यदि_अपरिभाषितम् LIBFFI_TARGET_H
#परिभाषय LIBFFI_TARGET_H

#यदि_अपरिभाषितम् LIBFFI_H
#दोषः "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#यदि_अन्तः

#यदि_अपरिभाषितम् LIBFFI_ASM
#यदि_परिभाषितम् __ILP32__
#परिभाषय FFI_SIZEOF_ARG 8
#परिभाषय FFI_SIZEOF_JAVA_RAW  4
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ दीर्घ ffi_sarg;
#नोचेद्यदि defined(_WIN32)
#परिभाषय FFI_SIZEOF_ARG 8
प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ दीर्घ ffi_sarg;
#अन्यथा
प्रकारनिर्देश अचिह्नित दीर्घ ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ ffi_sarg;
#यदि_अन्तः

प्रकारनिर्देश गणना ffi_abi
  {
    FFI_FIRST_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_चरमः_ABI,
#यदि defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#अन्यथा
    FFI_DEFAULT_ABI = FFI_SYSV
#यदि_अन्तः
  } ffi_abi;
#यदि_अन्तः

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_NATIVE_RAW_API 0

#यदि defined (FFI_EXEC_TRAMPOLINE_सारणी) && FFI_EXEC_TRAMPOLINE_सारणी

#यदि_परिभाषितम् __MACH__
#परिभाषय FFI_TRAMPOLINE_आकार 16
#परिभाषय FFI_TRAMPOLINE_संवृतिः_विस्थापनम् 16
#अन्यथा
#दोषः "No trampoline table implementation"
#यदि_अन्तः

#अन्यथा
#परिभाषय FFI_TRAMPOLINE_आकार 24
#परिभाषय FFI_TRAMPOLINE_संवृतिः_विस्थापनम् FFI_TRAMPOLINE_आकार
#यदि_अन्तः

#यदि_परिभाषितम् _WIN32
#परिभाषय FFI_EXTRA_CIF_FIELDS अचिह्नित is_variadic
#यदि_अन्तः
#परिभाषय FFI_TARGET_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#यदि defined (__APPLE__)
#परिभाषय FFI_EXTRA_CIF_FIELDS अचिह्नित aarch64_nfixedargs
#नोचेद्यदि !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#परिभाषय FFI_GO_CLOSURES 1
#यदि_अन्तः

#यदि_अपरिभाषितम् _WIN32
/* No complex type on Windows */
#परिभाषय FFI_TARGET_HAS_COMPLEX_TYPE
#यदि_अन्तः

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#परिभाषय FFI_TARGET_HAS_सदिशम्_TYPE

#परिभाषय FFI_TARGET_HAS_INT128 1

#यदि_अन्तः

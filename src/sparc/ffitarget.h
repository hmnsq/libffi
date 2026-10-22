/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for SPARC.

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

#यदि defined(__arch64__) || defined(__sparcv9)
#यदि_अपरिभाषितम् SPARC64
#परिभाषय SPARC64
#यदि_अन्तः
#यदि_अन्तः

#यदि_अपरिभाषितम् LIBFFI_ASM
प्रकारनिर्देश अचिह्नित दीर्घ          ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ            ffi_sarg;

प्रकारनिर्देश गणना ffi_abi {
  FFI_FIRST_ABI = 0,
#यदि_परिभाषितम् SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#अन्यथा
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#यदि_अन्तः
  FFI_चरमः_ABI
} ffi_abi;
#यदि_अन्तः

#परिभाषय FFI_TARGET_SPECIFIC_राशिः_SPACE_आवण्टनम् 1
#परिभाषय FFI_TARGET_HAS_COMPLEX_TYPE 1

#यदि_परिभाषितम् SPARC64
# define FFI_TARGET_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  अचिह्नित पूर्णाङ्क nfixedargs
#यदि_अन्तः

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_GO_CLOSURES 1
#परिभाषय FFI_NATIVE_RAW_API 0

#यदि_परिभाषितम् SPARC64
#परिभाषय FFI_TRAMPOLINE_आकार 24
#अन्यथा
#परिभाषय FFI_TRAMPOLINE_आकार 16
#यदि_अन्तः

#यदि_अन्तः


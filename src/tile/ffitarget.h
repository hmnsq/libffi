/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012 Tilera Corp.
   Target configuration macros for TILE.

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

#यदि_अपरिभाषितम् LIBFFI_ASM

#अन्तर्भाव <arch/abi.h>

प्रकारनिर्देश uint_reg_t ffi_arg;
प्रकारनिर्देश int_reg_t  ffi_sarg;

प्रकारनिर्देश गणना ffi_abi {
  FFI_FIRST_ABI = 0,
  FFI_UNIX,
  FFI_चरमः_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX
} ffi_abi;
#यदि_अन्तः

/* ---- Definitions for closures ----------------------------------------- */
#परिभाषय FFI_CLOSURES 1

#यदि_परिभाषितम् __tilegx__
/* We always pass 8-byte values, even in -m32 mode. */
# define FFI_SIZEOF_ARG 8
# ifdef __LP64__
#  define FFI_TRAMPOLINE_आकार (8 * 5)  /* 5 bundles */
# अन्यथा
#  define FFI_TRAMPOLINE_आकार (8 * 3)  /* 3 bundles */
# endif
#अन्यथा
# define FFI_SIZEOF_ARG 4
# define FFI_TRAMPOLINE_आकार 8 /* 1 bundle */
#यदि_अन्तः
#परिभाषय FFI_NATIVE_RAW_API 0

#यदि_अन्तः

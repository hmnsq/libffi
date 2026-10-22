/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - 2014 Michael Knyszek

   Target configuration macros for RISC-V.

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

#यदि_अपरिभाषितम् __riscv
#दोषः "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#यदि_अन्तः

#यदि_अपरिभाषितम् LIBFFI_ASM

प्रकारनिर्देश अचिह्नित दीर्घ ffi_arg;
प्रकारनिर्देश   चिह्नित दीर्घ ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
प्रकारनिर्देश गणना ffi_abi {
    FFI_FIRST_ABI = 0,
    FFI_SYSV,
    FFI_अप्रयुक्तम्_1,
    FFI_अप्रयुक्तम्_2,
    FFI_अप्रयुक्तम्_3,
    FFI_चरमः_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#यदि_अन्तः /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_GO_CLOSURES 1
#परिभाषय FFI_TRAMPOLINE_आकार 24
#परिभाषय FFI_NATIVE_RAW_API 0
#परिभाषय FFI_EXTRA_CIF_FIELDS अचिह्नित riscv_nfixedargs; अचिह्नित riscv_अप्रयुक्तम्
#परिभाषय FFI_TARGET_SPECIFIC_VARIADIC
#परिभाषय FFI_TARGET_HAS_INT128

#यदि_अन्तः


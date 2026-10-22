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

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

#اگر_تعریف_نشده __riscv
#خطا "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#پایان_اگر

#اگر_تعریف_نشده LIBFFI_ASM

تعريفنوع بیعلامت دراز ffi_arg;
تعريفنوع   علامتدار دراز ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
تعريفنوع تعداد ffi_abi {
    FFI_اولی_ABI = 0,
    FFI_SYSV,
    FFI_استفادهنشده_1,
    FFI_استفادهنشده_2,
    FFI_استفادهنشده_3,
    FFI_آخرین_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#پایان_اگر /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کن FFI_CLOSURES 1
#تعریف_کن FFI_GO_CLOSURES 1
#تعریف_کن FFI_TRAMPOLINE_اندازه 24
#تعریف_کن FFI_NATIVE_RAW_API 0
#تعریف_کن FFI_EXTRA_CIF_FIELDS بیعلامت riscv_nfixedargs; بیعلامت riscv_استفادهنشده
#تعریف_کن FFI_هدف_SPECIFIC_VARIADIC
#تعریف_کن FFI_هدف_HAS_INT128

#پایان_اگر


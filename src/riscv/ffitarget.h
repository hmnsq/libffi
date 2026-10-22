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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

#که_نه_تعریف_شوی __riscv
#تېروتنه "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#د_که_پای

#که_نه_تعریف_شوی LIBFFI_ASM

تعريفنوع بینښه اوږد ffi_arg;
تعريفنوع   نښهدار اوږد ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
تعريفنوع تعداد ffi_abi {
    FFI_لومړی_ABI = 0,
    FFI_SYSV,
    FFI_ناکارول_شوی_1,
    FFI_ناکارول_شوی_2,
    FFI_ناکارول_شوی_3,
    FFI_وروستی_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#د_که_پای /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_GO_CLOSURES 1
#تعریف_کړه FFI_TRAMPOLINE_اندازه 24
#تعریف_کړه FFI_NATIVE_RAW_API 0
#تعریف_کړه FFI_EXTRA_CIF_FIELDS بینښه riscv_nfixedargs; بینښه riscv_ناکارول_شوی
#تعریف_کړه FFI_موخه_SPECIFIC_VARIADIC
#تعریف_کړه FFI_موخه_HAS_INT128

#د_که_پای


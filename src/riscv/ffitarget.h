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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

#اگر_غیر_تعریف_شدہ __riscv
#غلطی "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#اگر_ختم

#اگر_غیر_تعریف_شدہ LIBFFI_ASM

قسمتعریف غیرعلامتی طویل ffi_arg;
قسمتعریف   علامتی طویل ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
قسمتعریف شمار ffi_abi {
    FFI_پہلا_ABI = 0,
    FFI_SYSV,
    FFI_غیر_مستعمل_1,
    FFI_غیر_مستعمل_2,
    FFI_غیر_مستعمل_3,
    FFI_آخری_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#اگر_ختم /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1
#تعریف_کرو FFI_TRAMPOLINE_سائز 24
#تعریف_کرو FFI_NATIVE_RAW_API 0
#تعریف_کرو FFI_EXTRA_CIF_FIELDS غیرعلامتی riscv_nfixedargs; غیرعلامتی riscv_غیر_مستعمل
#تعریف_کرو FFI_ہدف_SPECIFIC_VARIADIC
#تعریف_کرو FFI_ہدف_HAS_INT128

#اگر_ختم


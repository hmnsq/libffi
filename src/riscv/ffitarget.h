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

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل __riscv
#غلطي "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM

تعريفنوع بنانشان ڊگهو ffi_arg;
تعريفنوع   نشانوارو ڊگهو ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
تعريفنوع تعداد ffi_abi {
    FFI_پهريون_ABI = 0,
    FFI_SYSV,
    FFI_اڻ_استعمال_ٿيل_1,
    FFI_اڻ_استعمال_ٿيل_2,
    FFI_اڻ_استعمال_ٿيل_3,
    FFI_آخري_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#جيڪڏهن_پورو /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#وصف_ڪريو FFI_CLOSURES 1
#وصف_ڪريو FFI_GO_CLOSURES 1
#وصف_ڪريو FFI_TRAMPOLINE_سائيز 24
#وصف_ڪريو FFI_NATIVE_RAW_API 0
#وصف_ڪريو FFI_EXTRA_CIF_FIELDS بنانشان riscv_nfixedargs; بنانشان riscv_اڻ_استعمال_ٿيل
#وصف_ڪريو FFI_هدف_SPECIFIC_VARIADIC
#وصف_ڪريو FFI_هدف_HAS_INT128

#جيڪڏهن_پورو


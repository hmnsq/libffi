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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ಗುರಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_ಗುರಿ_H

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_H
#ದೋಷ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __riscv
#ದೋಷ "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ   ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
    FFI_ಮೊದಲ_ABI = 0,
    FFI_SYSV,
    FFI_ಬಳಸದ_1,
    FFI_ಬಳಸದ_2,
    FFI_ಬಳಸದ_3,
    FFI_ಕೊನೆಯ_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#ಆದರೆ_ಅಂತ್ಯ /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 24
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EXTRA_CIF_FIELDS ಚಿಹ್ನೆರಹಿತ riscv_nfixedargs; ಚಿಹ್ನೆರಹಿತ riscv_ಬಳಸದ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_VARIADIC
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_INT128

#ಆದರೆ_ಅಂತ್ಯ


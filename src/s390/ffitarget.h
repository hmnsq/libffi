/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2026  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for S390.

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

#ಆದರೆ defined (__s390x__)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ S390X
#ವ್ಯಾಖ್ಯಾನಿಸು S390X
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

/* ---- System specific configurations ----------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ          ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ            ffi_sarg;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
  FFI_ಮೊದಲ_ABI = 0,
  FFI_SYSV,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_COMPLEX_TYPE
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ S390X
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 32
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 16
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0

#ಆದರೆ_ಅಂತ್ಯ


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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ಗುರಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_ಗುರಿ_H

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_H
#ದೋಷ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ಆದರೆ_ಅಂತ್ಯ

/* ---- System specific configurations ----------------------------------- */

#ಆದರೆ defined(__arch64__) || defined(__sparcv9)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SPARC64
#ವ್ಯಾಖ್ಯಾನಿಸು SPARC64
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ          ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ            ffi_sarg;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
  FFI_ಮೊದಲ_ABI = 0,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#ಆದರೆ_ಅಂತ್ಯ
  FFI_ಕೊನೆಯ_ABI
} ffi_abi;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_COMPLEX_TYPE 1

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SPARC64
# define FFI_ಗುರಿ_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs
#ಆದರೆ_ಅಂತ್ಯ

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SPARC64
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 24
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 16
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ


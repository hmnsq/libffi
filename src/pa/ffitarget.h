/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2026  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for hppa.

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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ          ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ            ffi_sarg;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
  FFI_ಮೊದಲ_ABI = 0,

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_LINUX
  FFI_PA32,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA_HPUX
  FFI_PA32,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ PA64_HPUX
  FFI_PA64,
  FFI_ಕೊನೆಯ_ABI,
  FFI_DEFAULT_ABI = FFI_PA64
#ಆದರೆ_ಅಂತ್ಯ
} ffi_abi;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0
#ಆದರೆ defined(PA64_HPUX)
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 32
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 12
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT1 -1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT2 -2
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT3 -3
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT4 -4
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT5 -5
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT6 -6
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT7 -7
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPE_SMALL_STRUCT8 -8

/* The return-value jump tables in linux.S and hpux32.S are indexed by
   cif->flags, which ffi_prep_cif_machdep derives from the return type.  Any
   return type it does not handle explicitly -- including FFI_TYPE_COMPLEX and
   the 128-bit integer types FFI_TYPE_UINT128/FFI_TYPE_SINT128 -- falls through
   to the default case and is mapped to FFI_TYPE_INT, so cif->flags never
   exceeds FFI_TYPE_COMPLEX and the existing tables remain sufficient.  Bump
   FFI_PA_TYPE_LAST to the current FFI_TYPE_LAST once you have confirmed any
   newly added generic type is likewise handled (or the tables extended).

   FFI_TYPE_VECTOR (18) is likewise not reached here: PA does not define
   FFI_TARGET_HAS_VECTOR_TYPE, so ffi_prep_cif_core rejects any vector
   signature with FFI_BAD_TYPEDEF before machdep runs.  Bumping the tripwire
   past it is therefore safe.  */
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_PA_TYPE_ಕೊನೆಯ FFI_TYPE_ಸದಿಶ

/* Tripwire: when a new generic type is added FFI_TYPE_LAST changes and this
   fires, forcing a review of ffi_prep_cif_machdep and the linux.S / hpux32.S
   jump tables before FFI_PA_TYPE_LAST above is bumped.  */
#ಆದರೆ FFI_TYPE_ಕೊನೆಯ != FFI_PA_TYPE_ಕೊನೆಯ
# error "You likely have broken jump tables"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ

/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2018-2023  Hood Chatham, Brion Vibber, Kleis Auke Wolthuizen, and others.

   Target configuration macros for wasm32.

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

/* ---- Generic type definitions ----------------------------------------- */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ (*ffi_fp)(ಶೂನ್ಯ);

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi {
  FFI_ಮೊದಲ_ABI = 0,
#ಆದರೆ __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#ಅಥವಾಆದರೆ __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "Unknown pointer size"
#ಆದರೆ_ಅಂತ್ಯ
  FFI_ಕೊನೆಯ_ABI,
#ಆದರೆ __SIZEOF_POINTER__ == 4
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_WASM32
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ __SIZEOF_POINTER__ == 8
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#ಇಲ್ಲದಿದ್ದರೆ
  FFI_DEFAULT_ABI = FFI_WASM64
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "Unknown pointer size"
#ಆದರೆ_ಅಂತ್ಯ
} ffi_abi;

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 4
// #define FFI_NATIVE_RAW_API 0
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_VARIADIC 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EXTRA_CIF_FIELDS  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs

#ಆದರೆ_ಅಂತ್ಯ

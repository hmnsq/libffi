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

#ifndef LIBFFI_TARGET_H
#define LIBFFI_TARGET_H

#ifndef LIBFFI_H
#error "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#endif

/* ---- Generic type definitions ----------------------------------------- */

ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long ffi_arg;
ಪ್ರಕಾರನಿರ್ದೇಶ signed long ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ (*ffi_fp)(ಶೂನ್ಯ);

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ ffi_abi {
  FFI_FIRST_ABI = 0,
#ಯದಿ __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#ನೋಚೇದ್ಯದಿ __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#ಅನ್ಯಥಾ
#error "Unknown pointer size"
#endif
  FFI_LAST_ABI,
#ಯದಿ __SIZEOF_POINTER__ == 4
#ifdef __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#ಅನ್ಯಥಾ
  FFI_DEFAULT_ABI = FFI_WASM32
#endif
#ನೋಚೇದ್ಯದಿ __SIZEOF_POINTER__ == 8
#ifdef __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#ಅನ್ಯಥಾ
  FFI_DEFAULT_ABI = FFI_WASM64
#endif
#ಅನ್ಯಥಾ
#error "Unknown pointer size"
#endif
} ffi_abi;

#define FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#define FFI_TRAMPOLINE_SIZE 4
// #define FFI_NATIVE_RAW_API 0
#define FFI_TARGET_SPECIFIC_VARIADIC 1
#define FFI_EXTRA_CIF_FIELDS  unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs

#endif

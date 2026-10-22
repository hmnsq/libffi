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

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

/* ---- Generic type definitions ----------------------------------------- */

تعريفنوع بیعلامت دراز ffi_arg;
تعريفنوع علامتدار دراز ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
تعريفنوع فراغ (*ffi_fp)(فراغ);

تعريفنوع تعداد ffi_abi {
  FFI_اولی_ABI = 0,
#إذا __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#وإلاإذا __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#وإلا
#خطا "Unknown pointer size"
#پایان_اگر
  FFI_آخرین_ABI,
#إذا __SIZEOF_POINTER__ == 4
#اگر_تعریف_شده __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM32
#پایان_اگر
#وإلاإذا __SIZEOF_POINTER__ == 8
#اگر_تعریف_شده __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM64
#پایان_اگر
#وإلا
#خطا "Unknown pointer size"
#پایان_اگر
} ffi_abi;

#تعریف_کن FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#تعریف_کن FFI_TRAMPOLINE_اندازه 4
// #define FFI_NATIVE_RAW_API 0
#تعریف_کن FFI_هدف_SPECIFIC_VARIADIC 1
#تعریف_کن FFI_EXTRA_CIF_FIELDS  بیعلامت صحيح nfixedargs

#پایان_اگر

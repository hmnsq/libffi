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

#جے_بنا_تعریف LIBFFI_نشانہ_H
#تعریف_کرو LIBFFI_نشانہ_H

#جے_بنا_تعریف LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جے_انت

/* ---- Generic type definitions ----------------------------------------- */

تعريفنوع بنانشانی لما ffi_arg;
تعريفنوع نشانیوالا لما ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
تعريفنوع فراغ (*ffi_fp)(فراغ);

تعريفنوع تعداد ffi_abi {
  FFI_پہلا_ABI = 0,
#إذا __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#وإلاإذا __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#وإلا
#غلطی "Unknown pointer size"
#جے_انت
  FFI_آخری_ABI,
#إذا __SIZEOF_POINTER__ == 4
#جے_تعریف_شدہ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM32
#جے_انت
#وإلاإذا __SIZEOF_POINTER__ == 8
#جے_تعریف_شدہ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM64
#جے_انت
#وإلا
#غلطی "Unknown pointer size"
#جے_انت
} ffi_abi;

#تعریف_کرو FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#تعریف_کرو FFI_TRAMPOLINE_ناپ 4
// #define FFI_NATIVE_RAW_API 0
#تعریف_کرو FFI_نشانہ_SPECIFIC_VARIADIC 1
#تعریف_کرو FFI_EXTRA_CIF_FIELDS  بنانشانی صحيح nfixedargs

#جے_انت

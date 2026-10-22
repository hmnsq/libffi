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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ਨਿਸ਼ਾਨਾ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_ਨਿਸ਼ਾਨਾ_H

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_H
#ਗਲਤੀ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

/* ---- Generic type definitions ----------------------------------------- */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ (*ffi_fp)(ਖਾਲੀ);

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi {
  FFI_ਪਹਿਲਾ_ABI = 0,
#ਜੇ __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#ਜਾਂਜੇ __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "Unknown pointer size"
#ਜੇ_ਅੰਤ
  FFI_ਆਖਰੀ_ABI,
#ਜੇ __SIZEOF_POINTER__ == 4
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_WASM32
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ __SIZEOF_POINTER__ == 8
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#ਨਹੀਂਤਾਂ
  FFI_DEFAULT_ABI = FFI_WASM64
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "Unknown pointer size"
#ਜੇ_ਅੰਤ
} ffi_abi;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 4
// #define FFI_NATIVE_RAW_API 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_VARIADIC 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_EXTRA_CIF_FIELDS  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs

#ਜੇ_ਅੰਤ

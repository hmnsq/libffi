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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

/* ---- Generic type definitions ----------------------------------------- */

قسمتعریف غیرعلامتی طویل ffi_arg;
قسمتعریف علامتی طویل ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
قسمتعریف خالی (*ffi_fp)(خالی);

قسمتعریف شمار ffi_abi {
  FFI_پہلا_ABI = 0,
#اگر __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#وگرنہ_اگر __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#ورنہ
#غلطی "Unknown pointer size"
#اگر_ختم
  FFI_آخری_ABI,
#اگر __SIZEOF_POINTER__ == 4
#اگر_تعریف_شدہ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#ورنہ
  FFI_DEFAULT_ABI = FFI_WASM32
#اگر_ختم
#وگرنہ_اگر __SIZEOF_POINTER__ == 8
#اگر_تعریف_شدہ __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#ورنہ
  FFI_DEFAULT_ABI = FFI_WASM64
#اگر_ختم
#ورنہ
#غلطی "Unknown pointer size"
#اگر_ختم
} ffi_abi;

#تعریف_کرو FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#تعریف_کرو FFI_TRAMPOLINE_سائز 4
// #define FFI_NATIVE_RAW_API 0
#تعریف_کرو FFI_ہدف_SPECIFIC_VARIADIC 1
#تعریف_کرو FFI_EXTRA_CIF_FIELDS  غیرعلامتی صحیح nfixedargs

#اگر_ختم

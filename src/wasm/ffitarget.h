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

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

/* ---- Generic type definitions ----------------------------------------- */

تعريفنوع بنانشان ڊگهو ffi_arg;
تعريفنوع نشانوارو ڊگهو ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
تعريفنوع فراغ (*ffi_fp)(فراغ);

تعريفنوع تعداد ffi_abi {
  FFI_پهريون_ABI = 0,
#إذا __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#وإلاإذا __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#وإلا
#غلطي "Unknown pointer size"
#جيڪڏهن_پورو
  FFI_آخري_ABI,
#إذا __SIZEOF_POINTER__ == 4
#جيڪڏهن_وصف_ٿيل __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM32
#جيڪڏهن_پورو
#وإلاإذا __SIZEOF_POINTER__ == 8
#جيڪڏهن_وصف_ٿيل __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#وإلا
  FFI_DEFAULT_ABI = FFI_WASM64
#جيڪڏهن_پورو
#وإلا
#غلطي "Unknown pointer size"
#جيڪڏهن_پورو
} ffi_abi;

#وصف_ڪريو FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#وصف_ڪريو FFI_TRAMPOLINE_سائيز 4
// #define FFI_NATIVE_RAW_API 0
#وصف_ڪريو FFI_هدف_SPECIFIC_VARIADIC 1
#وصف_ڪريو FFI_EXTRA_CIF_FIELDS  بنانشان صحيح nfixedargs

#جيڪڏهن_پورو

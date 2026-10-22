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

#अगर_अघोषित LIBFFI_TARGET_H
#घोषणा LIBFFI_TARGET_H

#अगर_अघोषित LIBFFI_H
#त्रुटि "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#पूर्ण_अगर

/* ---- Generic type definitions ----------------------------------------- */

प्रकार अचिन्हित दीर्घ ffi_arg;
प्रकार चिन्हित दीर्घ ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
प्रकार व्योम (*ffi_fp)(व्योम);

प्रकार क्रमागत ffi_abi {
  FFI_FIRST_ABI = 0,
#अगर __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#या_अगर __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#अन्यथा
#त्रुटि "Unknown pointer size"
#पूर्ण_अगर
  FFI_LAST_ABI,
#अगर __SIZEOF_POINTER__ == 4
#अगर_घोषित __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#अन्यथा
  FFI_DEFAULT_ABI = FFI_WASM32
#पूर्ण_अगर
#या_अगर __SIZEOF_POINTER__ == 8
#अगर_घोषित __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#अन्यथा
  FFI_DEFAULT_ABI = FFI_WASM64
#पूर्ण_अगर
#अन्यथा
#त्रुटि "Unknown pointer size"
#पूर्ण_अगर
} ffi_abi;

#घोषणा FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#घोषणा FFI_TRAMPOLINE_आकार 4
// #define FFI_NATIVE_RAW_API 0
#घोषणा FFI_TARGET_SPECIFIC_VARIADIC 1
#घोषणा FFI_EXTRA_CIF_FIELDS  अचिन्हित पूर्णांक nfixedargs

#पूर्ण_अगर

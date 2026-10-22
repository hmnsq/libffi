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

#నిర్వచితం_కాకపోతే LIBFFI_లక్ష్యం_H
#నిర్వచించు LIBFFI_లక్ష్యం_H

#నిర్వచితం_కాకపోతే LIBFFI_H
#దోషం "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

/* ---- Generic type definitions ----------------------------------------- */

రకనిర్వచనం గుర్తులేని దీర్ఘ ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ ffi_sarg;

// TODO: https://github.com/emscripten-core/emscripten/issues/9868
రకనిర్వచనం శూన్య (*ffi_fp)(శూన్య);

రకనిర్వచనం గణన ffi_abi {
  FFI_మొదటి_ABI = 0,
#ఒకవేళ __SIZEOF_POINTER__ == 4
  FFI_WASM32, // "raw", no structures, varargs, or closures (not implemented!)
  FFI_WASM32_EMSCRIPTEN, // structures, varargs, and split 64-bit params
#లేదావేళ __SIZEOF_POINTER__ == 8
  FFI_WASM64,
  FFI_WASM64_EMSCRIPTEN,
#లేకపోతే
#దోషం "Unknown pointer size"
#అయితే_ముగింపు
  FFI_చివరి_ABI,
#ఒకవేళ __SIZEOF_POINTER__ == 4
#నిర్వచితమైతే __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM32_EMSCRIPTEN
#లేకపోతే
  FFI_DEFAULT_ABI = FFI_WASM32
#అయితే_ముగింపు
#లేదావేళ __SIZEOF_POINTER__ == 8
#నిర్వచితమైతే __EMSCRIPTEN__
  FFI_DEFAULT_ABI = FFI_WASM64_EMSCRIPTEN
#లేకపోతే
  FFI_DEFAULT_ABI = FFI_WASM64
#అయితే_ముగింపు
#లేకపోతే
#దోషం "Unknown pointer size"
#అయితే_ముగింపు
} ffi_abi;

#నిర్వచించు FFI_CLOSURES 1
// #define FFI_GO_CLOSURES 0
#నిర్వచించు FFI_TRAMPOLINE_కొలత 4
// #define FFI_NATIVE_RAW_API 0
#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC 1
#నిర్వచించు FFI_EXTRA_CIF_FIELDS  గుర్తులేని పూర్ణాంకం nfixedargs

#అయితే_ముగింపు

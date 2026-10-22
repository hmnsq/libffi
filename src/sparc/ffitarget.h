/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for SPARC.

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

/* ---- System specific configurations ----------------------------------- */

#अगर defined(__arch64__) || defined(__sparcv9)
#अगर_अघोषित SPARC64
#घोषणा SPARC64
#पूर्ण_अगर
#पूर्ण_अगर

#अगर_अघोषित LIBFFI_ASM
प्रकार अचिन्हित दीर्घ          ffi_arg;
प्रकार चिन्हित दीर्घ            ffi_sarg;

प्रकार क्रमागत ffi_abi {
  FFI_FIRST_ABI = 0,
#अगर_घोषित SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#अन्यथा
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#पूर्ण_अगर
  FFI_LAST_ABI
} ffi_abi;
#पूर्ण_अगर

#घोषणा FFI_TARGET_SPECIFIC_ढेर_SPACE_ALLOCATION 1
#घोषणा FFI_TARGET_HAS_COMPLEX_TYPE 1

#अगर_घोषित SPARC64
# define FFI_TARGET_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  अचिन्हित पूर्णांक nfixedargs
#पूर्ण_अगर

/* ---- Definitions for closures ----------------------------------------- */

#घोषणा FFI_CLOSURES 1
#घोषणा FFI_GO_CLOSURES 1
#घोषणा FFI_NATIVE_RAW_API 0

#अगर_घोषित SPARC64
#घोषणा FFI_TRAMPOLINE_आकार 24
#अन्यथा
#घोषणा FFI_TRAMPOLINE_आकार 16
#पूर्ण_अगर

#पूर्ण_अगर


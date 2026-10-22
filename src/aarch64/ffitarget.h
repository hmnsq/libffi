/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#अगर_अघोषित LIBFFI_TARGET_H
#घोषणा LIBFFI_TARGET_H

#अगर_अघोषित LIBFFI_H
#त्रुटि "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#पूर्ण_अगर

#अगर_अघोषित LIBFFI_ASM
#अगर_घोषित __ILP32__
#घोषणा FFI_SIZEOF_ARG 8
#घोषणा FFI_SIZEOF_JAVA_RAW  4
प्रकार अचिन्हित दीर्घ दीर्घ ffi_arg;
प्रकार चिन्हित दीर्घ दीर्घ ffi_sarg;
#या_अगर defined(_WIN32)
#घोषणा FFI_SIZEOF_ARG 8
प्रकार अचिन्हित दीर्घ दीर्घ ffi_arg;
प्रकार चिन्हित दीर्घ दीर्घ ffi_sarg;
#अन्यथा
प्रकार अचिन्हित दीर्घ ffi_arg;
प्रकार चिन्हित दीर्घ ffi_sarg;
#पूर्ण_अगर

प्रकार क्रमागत ffi_abi
  {
    FFI_FIRST_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_LAST_ABI,
#अगर defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#अन्यथा
    FFI_DEFAULT_ABI = FFI_SYSV
#पूर्ण_अगर
  } ffi_abi;
#पूर्ण_अगर

/* ---- Definitions for closures ----------------------------------------- */

#घोषणा FFI_CLOSURES 1
#घोषणा FFI_NATIVE_RAW_API 0

#अगर defined (FFI_EXEC_TRAMPOLINE_TABLE) && FFI_EXEC_TRAMPOLINE_TABLE

#अगर_घोषित __MACH__
#घोषणा FFI_TRAMPOLINE_आकार 16
#घोषणा FFI_TRAMPOLINE_CLOSURE_OFFSET 16
#अन्यथा
#त्रुटि "No trampoline table implementation"
#पूर्ण_अगर

#अन्यथा
#घोषणा FFI_TRAMPOLINE_आकार 24
#घोषणा FFI_TRAMPOLINE_CLOSURE_OFFSET FFI_TRAMPOLINE_आकार
#पूर्ण_अगर

#अगर_घोषित _WIN32
#घोषणा FFI_EXTRA_CIF_FIELDS अचिन्हित is_variadic
#पूर्ण_अगर
#घोषणा FFI_TARGET_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#अगर defined (__APPLE__)
#घोषणा FFI_EXTRA_CIF_FIELDS अचिन्हित aarch64_nfixedargs
#या_अगर !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#घोषणा FFI_GO_CLOSURES 1
#पूर्ण_अगर

#अगर_अघोषित _WIN32
/* No complex type on Windows */
#घोषणा FFI_TARGET_HAS_COMPLEX_TYPE
#पूर्ण_अगर

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#घोषणा FFI_TARGET_HAS_VECTOR_TYPE

#घोषणा FFI_TARGET_HAS_INT128 1

#पूर्ण_अगर

/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 2010  CodeSourcery
                 Copyright (c) 1996-2003  Red Hat, Inc.

   Target configuration macros for ARM.

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

#अगर_अघोषित LIBFFI_ASM
प्रकार अचिन्हित दीर्घ          ffi_arg;
प्रकार चिन्हित दीर्घ            ffi_sarg;

प्रकार क्रमागत ffi_abi {
  FFI_FIRST_ABI = 0,
  FFI_SYSV,
  FFI_VFP,
  FFI_LAST_ABI,
#अगर defined(__ARM_PCS_VFP) || defined(_WIN32)
  FFI_DEFAULT_ABI = FFI_VFP,
#अन्यथा
  FFI_DEFAULT_ABI = FFI_SYSV,
#पूर्ण_अगर
} ffi_abi;
#पूर्ण_अगर

#घोषणा FFI_EXTRA_CIF_FIELDS			\
  पूर्णांक vfp_used;					\
  अचिन्हित लघु vfp_reg_free, vfp_nargs;	\
  चिन्हित अक्षर vfp_args[16]			\

#घोषणा FFI_TARGET_SPECIFIC_VARIADIC
#अगर_अघोषित _WIN32
#घोषणा FFI_TARGET_HAS_COMPLEX_TYPE
#पूर्ण_अगर

/* ---- Definitions for closures ----------------------------------------- */

#घोषणा FFI_CLOSURES 1
#घोषणा FFI_GO_CLOSURES 1
#घोषणा FFI_NATIVE_RAW_API 0

#अगर defined (FFI_EXEC_TRAMPOLINE_TABLE) && FFI_EXEC_TRAMPOLINE_TABLE

#अगर_घोषित __MACH__
#घोषणा FFI_TRAMPOLINE_आकार 12
#घोषणा FFI_TRAMPOLINE_CLOSURE_OFFSET 8
#अन्यथा
#त्रुटि "No trampoline table implementation"
#पूर्ण_अगर

#अन्यथा
#अगर_घोषित _WIN32
#घोषणा FFI_TRAMPOLINE_आकार 16
#घोषणा FFI_TRAMPOLINE_CLOSURE_FUNCTION 12
#अन्यथा
#घोषणा FFI_TRAMPOLINE_आकार 12
#पूर्ण_अगर
#घोषणा FFI_TRAMPOLINE_CLOSURE_OFFSET FFI_TRAMPOLINE_आकार
#पूर्ण_अगर

#पूर्ण_अगर

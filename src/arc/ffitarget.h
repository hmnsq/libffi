/* -----------------------------------------------------------------------
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 2013  Synopsys, Inc. (www.synopsys.com)
   Target configuration macros for ARC.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   ----------------------------------------------------------------------- */

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

/* ---- Generic type definitions ----------------------------------------- */

#اگر_تعریف_نشده LIBFFI_ASM
تعريفنوع بیعلامت دراز ffi_arg;
تعريفنوع علامتدار دراز ffi_sarg;

تعريفنوع تعداد ffi_abi
{
  FFI_اولی_ABI = 0,
#إذا __SIZEOF_POINTER__ == 8
  FFI_ARC64,
#وإلا
  FFI_ARCOMPACT,
#پایان_اگر
  FFI_آخرین_ABI,
#إذا __SIZEOF_POINTER__ == 8
  FFI_DEFAULT_ABI = FFI_ARC64
#وإلا
  FFI_DEFAULT_ABI = FFI_ARCOMPACT
#پایان_اگر
} ffi_abi;
#پایان_اگر

#تعریف_کن FFI_CLOSURES 		1
#تعریف_کن FFI_GO_CLOSURES 1
#إذا __SIZEOF_POINTER__ == 8
#تعریف_کن FFI_TRAMPOLINE_اندازه	24
#وإلا
#تعریف_کن FFI_TRAMPOLINE_اندازه	12
#پایان_اگر

#تعریف_کن FFI_NATIVE_RAW_API 	0

#پایان_اگر

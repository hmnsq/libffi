/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 2004  Renesas Technology.
   Target configuration macros for M32R.

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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

/* ---- Generic type definitions ----------------------------------------- */

#که_نه_تعریف_شوی LIBFFI_ASM
تعريفنوع بینښه اوږد          ffi_arg;
تعريفنوع نښهدار اوږد            ffi_sarg;

تعريفنوع تعداد ffi_abi
  {
    FFI_لومړی_ABI = 0,
    FFI_SYSV,
    FFI_وروستی_ABI,
    FFI_DEFAULT_ABI = FFI_SYSV
  } ffi_abi;
#د_که_پای

#تعریف_کړه FFI_CLOSURES 		0
#تعریف_کړه FFI_TRAMPOLINE_اندازه	24
#تعریف_کړه FFI_NATIVE_RAW_API 	0

#د_که_پای

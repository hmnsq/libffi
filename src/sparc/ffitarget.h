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

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

/* ---- System specific configurations ----------------------------------- */

#إذا defined(__arch64__) || defined(__sparcv9)
#اگر_تعریف_نشده SPARC64
#تعریف_کن SPARC64
#پایان_اگر
#پایان_اگر

#اگر_تعریف_نشده LIBFFI_ASM
تعريفنوع بیعلامت دراز          ffi_arg;
تعريفنوع علامتدار دراز            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_اولی_ABI = 0,
#اگر_تعریف_شده SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#وإلا
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#پایان_اگر
  FFI_آخرین_ABI
} ffi_abi;
#پایان_اگر

#تعریف_کن FFI_هدف_SPECIFIC_پشته_SPACE_تخصیص 1
#تعریف_کن FFI_هدف_HAS_COMPLEX_TYPE 1

#اگر_تعریف_شده SPARC64
# define FFI_هدف_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  بیعلامت صحيح nfixedargs
#پایان_اگر

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کن FFI_CLOSURES 1
#تعریف_کن FFI_GO_CLOSURES 1
#تعریف_کن FFI_NATIVE_RAW_API 0

#اگر_تعریف_شده SPARC64
#تعریف_کن FFI_TRAMPOLINE_اندازه 24
#وإلا
#تعریف_کن FFI_TRAMPOLINE_اندازه 16
#پایان_اگر

#پایان_اگر


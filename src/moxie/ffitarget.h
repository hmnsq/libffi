/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2013  Anthony Green
   Target configuration macros for Moxie

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

/* ---- System specific configurations ----------------------------------- */

#اگر_تعریف_نشده LIBFFI_ASM
تعريفنوع بیعلامت دراز          ffi_arg;
تعريفنوع علامتدار دراز            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_اولی_ABI = 0,
  FFI_EABI,
  FFI_DEFAULT_ABI = FFI_EABI,
  FFI_آخرین_ABI = FFI_DEFAULT_ABI + 1
} ffi_abi;
#پایان_اگر

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کن FFI_CLOSURES 1
#تعریف_کن FFI_NATIVE_RAW_API 0

/* Trampolines are 12-bytes long.  See ffi_prep_closure_loc.  */
#تعریف_کن FFI_TRAMPOLINE_اندازه (12)

#پایان_اگر

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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

/* ---- System specific configurations ----------------------------------- */

#اگر defined(__arch64__) || defined(__sparcv9)
#اگر_غیر_تعریف_شدہ SPARC64
#تعریف_کرو SPARC64
#اگر_ختم
#اگر_ختم

#اگر_غیر_تعریف_شدہ LIBFFI_ASM
قسمتعریف غیرعلامتی طویل          ffi_arg;
قسمتعریف علامتی طویل            ffi_sarg;

قسمتعریف شمار ffi_abi {
  FFI_پہلا_ABI = 0,
#اگر_تعریف_شدہ SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#ورنہ
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#اگر_ختم
  FFI_آخری_ABI
} ffi_abi;
#اگر_ختم

#تعریف_کرو FFI_ہدف_SPECIFIC_ڈھیر_SPACE_تخصیص 1
#تعریف_کرو FFI_ہدف_HAS_COMPLEX_TYPE 1

#اگر_تعریف_شدہ SPARC64
# define FFI_ہدف_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  غیرعلامتی صحیح nfixedargs
#اگر_ختم

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1
#تعریف_کرو FFI_NATIVE_RAW_API 0

#اگر_تعریف_شدہ SPARC64
#تعریف_کرو FFI_TRAMPOLINE_سائز 24
#ورنہ
#تعریف_کرو FFI_TRAMPOLINE_سائز 16
#اگر_ختم

#اگر_ختم


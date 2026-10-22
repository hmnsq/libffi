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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

/* ---- System specific configurations ----------------------------------- */

#إذا defined(__arch64__) || defined(__sparcv9)
#که_نه_تعریف_شوی SPARC64
#تعریف_کړه SPARC64
#د_که_پای
#د_که_پای

#که_نه_تعریف_شوی LIBFFI_ASM
تعريفنوع بینښه اوږد          ffi_arg;
تعريفنوع نښهدار اوږد            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_لومړی_ABI = 0,
#که_تعریف_شوی SPARC64
  FFI_V9,
  FFI_DEFAULT_ABI = FFI_V9,
#وإلا
  FFI_V8,
  FFI_DEFAULT_ABI = FFI_V8,
#د_که_پای
  FFI_وروستی_ABI
} ffi_abi;
#د_که_پای

#تعریف_کړه FFI_موخه_SPECIFIC_ډېری_SPACE_ویش 1
#تعریف_کړه FFI_موخه_HAS_COMPLEX_TYPE 1

#که_تعریف_شوی SPARC64
# define FFI_موخه_SPECIFIC_VARIADIC 1
# define FFI_EXTRA_CIF_FIELDS  بینښه صحيح nfixedargs
#د_که_پای

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_GO_CLOSURES 1
#تعریف_کړه FFI_NATIVE_RAW_API 0

#که_تعریف_شوی SPARC64
#تعریف_کړه FFI_TRAMPOLINE_اندازه 24
#وإلا
#تعریف_کړه FFI_TRAMPOLINE_اندازه 16
#د_که_پای

#د_که_پای


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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

#که_نه_تعریف_شوی LIBFFI_ASM
#که_تعریف_شوی __ILP32__
#تعریف_کړه FFI_SIZEOF_ARG 8
#تعریف_کړه FFI_SIZEOF_JAVA_RAW  4
تعريفنوع بینښه اوږد اوږد ffi_arg;
تعريفنوع نښهدار اوږد اوږد ffi_sarg;
#وإلاإذا defined(_WIN32)
#تعریف_کړه FFI_SIZEOF_ARG 8
تعريفنوع بینښه اوږد اوږد ffi_arg;
تعريفنوع نښهدار اوږد اوږد ffi_sarg;
#وإلا
تعريفنوع بینښه اوږد ffi_arg;
تعريفنوع نښهدار اوږد ffi_sarg;
#د_که_پای

تعريفنوع تعداد ffi_abi
  {
    FFI_لومړی_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_وروستی_ABI,
#إذا defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#وإلا
    FFI_DEFAULT_ABI = FFI_SYSV
#د_که_پای
  } ffi_abi;
#د_که_پای

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_NATIVE_RAW_API 0

#إذا defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#که_تعریف_شوی __MACH__
#تعریف_کړه FFI_TRAMPOLINE_اندازه 16
#تعریف_کړه FFI_TRAMPOLINE_تړون_دننه_واټن 16
#وإلا
#تېروتنه "No trampoline table implementation"
#د_که_پای

#وإلا
#تعریف_کړه FFI_TRAMPOLINE_اندازه 24
#تعریف_کړه FFI_TRAMPOLINE_تړون_دننه_واټن FFI_TRAMPOLINE_اندازه
#د_که_پای

#که_تعریف_شوی _WIN32
#تعریف_کړه FFI_EXTRA_CIF_FIELDS بینښه is_variadic
#د_که_پای
#تعریف_کړه FFI_موخه_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#إذا defined (__APPLE__)
#تعریف_کړه FFI_EXTRA_CIF_FIELDS بینښه aarch64_nfixedargs
#وإلاإذا !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#تعریف_کړه FFI_GO_CLOSURES 1
#د_که_پای

#که_نه_تعریف_شوی _WIN32
/* No complex type on Windows */
#تعریف_کړه FFI_موخه_HAS_COMPLEX_TYPE
#د_که_پای

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#تعریف_کړه FFI_موخه_HAS_ویکټور_TYPE

#تعریف_کړه FFI_موخه_HAS_INT128 1

#د_که_پای

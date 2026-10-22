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

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

#اگر_تعریف_نشده LIBFFI_ASM
#اگر_تعریف_شده __ILP32__
#تعریف_کن FFI_SIZEOF_ARG 8
#تعریف_کن FFI_SIZEOF_JAVA_RAW  4
تعريفنوع بیعلامت دراز دراز ffi_arg;
تعريفنوع علامتدار دراز دراز ffi_sarg;
#وإلاإذا defined(_WIN32)
#تعریف_کن FFI_SIZEOF_ARG 8
تعريفنوع بیعلامت دراز دراز ffi_arg;
تعريفنوع علامتدار دراز دراز ffi_sarg;
#وإلا
تعريفنوع بیعلامت دراز ffi_arg;
تعريفنوع علامتدار دراز ffi_sarg;
#پایان_اگر

تعريفنوع تعداد ffi_abi
  {
    FFI_اولی_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_آخرین_ABI,
#إذا defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#وإلا
    FFI_DEFAULT_ABI = FFI_SYSV
#پایان_اگر
  } ffi_abi;
#پایان_اگر

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کن FFI_CLOSURES 1
#تعریف_کن FFI_NATIVE_RAW_API 0

#إذا defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شده __MACH__
#تعریف_کن FFI_TRAMPOLINE_اندازه 16
#تعریف_کن FFI_TRAMPOLINE_بستگی_جابه_جایی 16
#وإلا
#خطا "No trampoline table implementation"
#پایان_اگر

#وإلا
#تعریف_کن FFI_TRAMPOLINE_اندازه 24
#تعریف_کن FFI_TRAMPOLINE_بستگی_جابه_جایی FFI_TRAMPOLINE_اندازه
#پایان_اگر

#اگر_تعریف_شده _WIN32
#تعریف_کن FFI_EXTRA_CIF_FIELDS بیعلامت is_variadic
#پایان_اگر
#تعریف_کن FFI_هدف_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#إذا defined (__APPLE__)
#تعریف_کن FFI_EXTRA_CIF_FIELDS بیعلامت aarch64_nfixedargs
#وإلاإذا !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#تعریف_کن FFI_GO_CLOSURES 1
#پایان_اگر

#اگر_تعریف_نشده _WIN32
/* No complex type on Windows */
#تعریف_کن FFI_هدف_HAS_COMPLEX_TYPE
#پایان_اگر

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#تعریف_کن FFI_هدف_HAS_وکتور_TYPE

#تعریف_کن FFI_هدف_HAS_INT128 1

#پایان_اگر

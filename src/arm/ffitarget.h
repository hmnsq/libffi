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

#اگر_تعریف_نشده LIBFFI_هدف_H
#تعریف_کن LIBFFI_هدف_H

#اگر_تعریف_نشده LIBFFI_H
#خطا "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#پایان_اگر

#اگر_تعریف_نشده LIBFFI_ASM
تعريفنوع بیعلامت دراز          ffi_arg;
تعريفنوع علامتدار دراز            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_اولی_ABI = 0,
  FFI_SYSV,
  FFI_VFP,
  FFI_آخرین_ABI,
#إذا defined(__ARM_PCS_VFP) || defined(_WIN32)
  FFI_DEFAULT_ABI = FFI_VFP,
#وإلا
  FFI_DEFAULT_ABI = FFI_SYSV,
#پایان_اگر
} ffi_abi;
#پایان_اگر

#تعریف_کن FFI_EXTRA_CIF_FIELDS			\
  صحيح vfp_استفاده_شده;					\
  بیعلامت کوتاه vfp_reg_free, vfp_nargs;	\
  علامتدار حرف vfp_آرگومانها[16]			\

#تعریف_کن FFI_هدف_SPECIFIC_VARIADIC
#اگر_تعریف_نشده _WIN32
#تعریف_کن FFI_هدف_HAS_COMPLEX_TYPE
#پایان_اگر

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کن FFI_CLOSURES 1
#تعریف_کن FFI_GO_CLOSURES 1
#تعریف_کن FFI_NATIVE_RAW_API 0

#إذا defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شده __MACH__
#تعریف_کن FFI_TRAMPOLINE_اندازه 12
#تعریف_کن FFI_TRAMPOLINE_بستگی_جابه_جایی 8
#وإلا
#خطا "No trampoline table implementation"
#پایان_اگر

#وإلا
#اگر_تعریف_شده _WIN32
#تعریف_کن FFI_TRAMPOLINE_اندازه 16
#تعریف_کن FFI_TRAMPOLINE_بستگی_FUNCTION 12
#وإلا
#تعریف_کن FFI_TRAMPOLINE_اندازه 12
#پایان_اگر
#تعریف_کن FFI_TRAMPOLINE_بستگی_جابه_جایی FFI_TRAMPOLINE_اندازه
#پایان_اگر

#پایان_اگر

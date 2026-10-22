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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

#اگر_غیر_تعریف_شدہ LIBFFI_ASM
قسمتعریف غیرعلامتی طویل          ffi_arg;
قسمتعریف علامتی طویل            ffi_sarg;

قسمتعریف شمار ffi_abi {
  FFI_پہلا_ABI = 0,
  FFI_SYSV,
  FFI_VFP,
  FFI_آخری_ABI,
#اگر defined(__ARM_PCS_VFP) || defined(_WIN32)
  FFI_DEFAULT_ABI = FFI_VFP,
#ورنہ
  FFI_DEFAULT_ABI = FFI_SYSV,
#اگر_ختم
} ffi_abi;
#اگر_ختم

#تعریف_کرو FFI_EXTRA_CIF_FIELDS			\
  صحیح vfp_مستعمل;					\
  غیرعلامتی مختصر vfp_reg_free, vfp_nargs;	\
  علامتی حرف vfp_دلائل[16]			\

#تعریف_کرو FFI_ہدف_SPECIFIC_VARIADIC
#اگر_غیر_تعریف_شدہ _WIN32
#تعریف_کرو FFI_ہدف_HAS_COMPLEX_TYPE
#اگر_ختم

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1
#تعریف_کرو FFI_NATIVE_RAW_API 0

#اگر defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شدہ __MACH__
#تعریف_کرو FFI_TRAMPOLINE_سائز 12
#تعریف_کرو FFI_TRAMPOLINE_احاطہ_انحراف 8
#ورنہ
#غلطی "No trampoline table implementation"
#اگر_ختم

#ورنہ
#اگر_تعریف_شدہ _WIN32
#تعریف_کرو FFI_TRAMPOLINE_سائز 16
#تعریف_کرو FFI_TRAMPOLINE_احاطہ_FUNCTION 12
#ورنہ
#تعریف_کرو FFI_TRAMPOLINE_سائز 12
#اگر_ختم
#تعریف_کرو FFI_TRAMPOLINE_احاطہ_انحراف FFI_TRAMPOLINE_سائز
#اگر_ختم

#اگر_ختم

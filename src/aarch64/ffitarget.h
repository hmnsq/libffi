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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

#اگر_غیر_تعریف_شدہ LIBFFI_ASM
#اگر_تعریف_شدہ __ILP32__
#تعریف_کرو FFI_SIZEOF_ARG 8
#تعریف_کرو FFI_SIZEOF_JAVA_RAW  4
قسمتعریف غیرعلامتی طویل طویل ffi_arg;
قسمتعریف علامتی طویل طویل ffi_sarg;
#وگرنہ_اگر defined(_WIN32)
#تعریف_کرو FFI_SIZEOF_ARG 8
قسمتعریف غیرعلامتی طویل طویل ffi_arg;
قسمتعریف علامتی طویل طویل ffi_sarg;
#ورنہ
قسمتعریف غیرعلامتی طویل ffi_arg;
قسمتعریف علامتی طویل ffi_sarg;
#اگر_ختم

قسمتعریف شمار ffi_abi
  {
    FFI_پہلا_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_آخری_ABI,
#اگر defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#ورنہ
    FFI_DEFAULT_ABI = FFI_SYSV
#اگر_ختم
  } ffi_abi;
#اگر_ختم

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_NATIVE_RAW_API 0

#اگر defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شدہ __MACH__
#تعریف_کرو FFI_TRAMPOLINE_سائز 16
#تعریف_کرو FFI_TRAMPOLINE_احاطہ_انحراف 16
#ورنہ
#غلطی "No trampoline table implementation"
#اگر_ختم

#ورنہ
#تعریف_کرو FFI_TRAMPOLINE_سائز 24
#تعریف_کرو FFI_TRAMPOLINE_احاطہ_انحراف FFI_TRAMPOLINE_سائز
#اگر_ختم

#اگر_تعریف_شدہ _WIN32
#تعریف_کرو FFI_EXTRA_CIF_FIELDS غیرعلامتی is_variadic
#اگر_ختم
#تعریف_کرو FFI_ہدف_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#اگر defined (__APPLE__)
#تعریف_کرو FFI_EXTRA_CIF_FIELDS غیرعلامتی aarch64_nfixedargs
#وگرنہ_اگر !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#تعریف_کرو FFI_GO_CLOSURES 1
#اگر_ختم

#اگر_غیر_تعریف_شدہ _WIN32
/* No complex type on Windows */
#تعریف_کرو FFI_ہدف_HAS_COMPLEX_TYPE
#اگر_ختم

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#تعریف_کرو FFI_ہدف_HAS_سمتیہ_TYPE

#تعریف_کرو FFI_ہدف_HAS_INT128 1

#اگر_ختم

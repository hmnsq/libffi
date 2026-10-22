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

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM
#جيڪڏهن_وصف_ٿيل __ILP32__
#وصف_ڪريو FFI_SIZEOF_ARG 8
#وصف_ڪريو FFI_SIZEOF_JAVA_RAW  4
تعريفنوع بنانشان ڊگهو ڊگهو ffi_arg;
تعريفنوع نشانوارو ڊگهو ڊگهو ffi_sarg;
#وإلاإذا defined(_WIN32)
#وصف_ڪريو FFI_SIZEOF_ARG 8
تعريفنوع بنانشان ڊگهو ڊگهو ffi_arg;
تعريفنوع نشانوارو ڊگهو ڊگهو ffi_sarg;
#وإلا
تعريفنوع بنانشان ڊگهو ffi_arg;
تعريفنوع نشانوارو ڊگهو ffi_sarg;
#جيڪڏهن_پورو

تعريفنوع تعداد ffi_abi
  {
    FFI_پهريون_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_آخري_ABI,
#إذا defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#وإلا
    FFI_DEFAULT_ABI = FFI_SYSV
#جيڪڏهن_پورو
  } ffi_abi;
#جيڪڏهن_پورو

/* ---- Definitions for closures ----------------------------------------- */

#وصف_ڪريو FFI_CLOSURES 1
#وصف_ڪريو FFI_NATIVE_RAW_API 0

#إذا defined (FFI_EXEC_TRAMPOLINE_جدول) && FFI_EXEC_TRAMPOLINE_جدول

#جيڪڏهن_وصف_ٿيل __MACH__
#وصف_ڪريو FFI_TRAMPOLINE_سائيز 16
#وصف_ڪريو FFI_TRAMPOLINE_گهيرو_وٿي 16
#وإلا
#غلطي "No trampoline table implementation"
#جيڪڏهن_پورو

#وإلا
#وصف_ڪريو FFI_TRAMPOLINE_سائيز 24
#وصف_ڪريو FFI_TRAMPOLINE_گهيرو_وٿي FFI_TRAMPOLINE_سائيز
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل _WIN32
#وصف_ڪريو FFI_EXTRA_CIF_FIELDS بنانشان is_variadic
#جيڪڏهن_پورو
#وصف_ڪريو FFI_هدف_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#إذا defined (__APPLE__)
#وصف_ڪريو FFI_EXTRA_CIF_FIELDS بنانشان aarch64_nfixedargs
#وإلاإذا !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#وصف_ڪريو FFI_GO_CLOSURES 1
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل _WIN32
/* No complex type on Windows */
#وصف_ڪريو FFI_هدف_HAS_COMPLEX_TYPE
#جيڪڏهن_پورو

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#وصف_ڪريو FFI_هدف_HAS_ويڪٽر_TYPE

#وصف_ڪريو FFI_هدف_HAS_INT128 1

#جيڪڏهن_پورو

/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012 Tilera Corp.
   Target configuration macros for TILE.

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

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM

#ضمن <arch/abi.h>

تعريفنوع uint_reg_t ffi_arg;
تعريفنوع int_reg_t  ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_پهريون_ABI = 0,
  FFI_UNIX,
  FFI_آخري_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX
} ffi_abi;
#جيڪڏهن_پورو

/* ---- Definitions for closures ----------------------------------------- */
#وصف_ڪريو FFI_CLOSURES 1

#جيڪڏهن_وصف_ٿيل __tilegx__
/* We always pass 8-byte values, even in -m32 mode. */
# define FFI_SIZEOF_ARG 8
# ifdef __LP64__
#  define FFI_TRAMPOLINE_سائيز (8 * 5)  /* 5 bundles */
# وإلا
#  define FFI_TRAMPOLINE_سائيز (8 * 3)  /* 3 bundles */
# endif
#وإلا
# define FFI_SIZEOF_ARG 4
# define FFI_TRAMPOLINE_سائيز 8 /* 1 bundle */
#جيڪڏهن_پورو
#وصف_ڪريو FFI_NATIVE_RAW_API 0

#جيڪڏهن_پورو

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

#اگر_غیر_تعریف_شدہ LIBFFI_ہدف_H
#تعریف_کرو LIBFFI_ہدف_H

#اگر_غیر_تعریف_شدہ LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#اگر_ختم

#اگر_غیر_تعریف_شدہ LIBFFI_ASM

#شامل <arch/abi.h>

قسمتعریف uint_reg_t ffi_arg;
قسمتعریف int_reg_t  ffi_sarg;

قسمتعریف شمار ffi_abi {
  FFI_پہلا_ABI = 0,
  FFI_UNIX,
  FFI_آخری_ABI,
  FFI_DEFAULT_ABI = FFI_UNIX
} ffi_abi;
#اگر_ختم

/* ---- Definitions for closures ----------------------------------------- */
#تعریف_کرو FFI_CLOSURES 1

#اگر_تعریف_شدہ __tilegx__
/* We always pass 8-byte values, even in -m32 mode. */
# define FFI_SIZEOF_ARG 8
# ifdef __LP64__
#  define FFI_TRAMPOLINE_سائز (8 * 5)  /* 5 bundles */
# ورنہ
#  define FFI_TRAMPOLINE_سائز (8 * 3)  /* 3 bundles */
# endif
#ورنہ
# define FFI_SIZEOF_ARG 4
# define FFI_TRAMPOLINE_سائز 8 /* 1 bundle */
#اگر_ختم
#تعریف_کرو FFI_NATIVE_RAW_API 0

#اگر_ختم

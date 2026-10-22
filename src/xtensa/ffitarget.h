/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2013 Tensilica, Inc.
   Target configuration macros for XTENSA.

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

#جے_بنا_تعریف LIBFFI_نشانہ_H
#تعریف_کرو LIBFFI_نشانہ_H

#جے_بنا_تعریف LIBFFI_H
#غلطی "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جے_انت

#جے_بنا_تعریف LIBFFI_ASM
تعريفنوع بنانشانی لما	ffi_arg;
تعريفنوع نشانیوالا لما	ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_پہلا_ABI = 0,
  FFI_SYSV,
  FFI_آخری_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;
#جے_انت

#تعریف_کرو FFI_رجسٹر_NARGS	6
#تعریف_کرو XTENSA_ڈھیر_ALIGNMENT	16
#تعریف_کرو FFI_رجسٹر_دلیلاں_SPACE ((FFI_رجسٹر_NARGS * 4 + \
				  XTENSA_ڈھیر_ALIGNMENT - 1) & \
				  -XTENSA_ڈھیر_ALIGNMENT)

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_NATIVE_RAW_API 0
#تعریف_کرو FFI_TRAMPOLINE_ناپ 24

#جے_انت

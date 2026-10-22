/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 2004  Renesas Technology.
   Target configuration macros for M32R.

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.

   ----------------------------------------------------------------------- */

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_هدف_H
#وصف_ڪريو LIBFFI_هدف_H

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_H
#غلطي "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

/* ---- Generic type definitions ----------------------------------------- */

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM
تعريفنوع بنانشان ڊگهو          ffi_arg;
تعريفنوع نشانوارو ڊگهو            ffi_sarg;

تعريفنوع تعداد ffi_abi
  {
    FFI_پهريون_ABI = 0,
    FFI_SYSV,
    FFI_آخري_ABI,
    FFI_DEFAULT_ABI = FFI_SYSV
  } ffi_abi;
#جيڪڏهن_پورو

#وصف_ڪريو FFI_CLOSURES 		0
#وصف_ڪريو FFI_TRAMPOLINE_سائيز	24
#وصف_ڪريو FFI_NATIVE_RAW_API 	0

#جيڪڏهن_پورو

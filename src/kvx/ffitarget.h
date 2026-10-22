/* -----------------------------------------------------------------------
   ffitarget.h - Copyright (c) 2020 Kalray

   KVX Target configuration macros

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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source. Use ffi.h instead."
#د_که_پای

/* ---- System specific configurations ----------------------------------- */

#که_نه_تعریف_شوی LIBFFI_ASM
تعريفنوع بینښه اوږد          ffi_arg;
تعريفنوع نښهدار اوږد            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_لومړی_ABI = 0,
  FFI_SYSV,
  FFI_وروستی_ABI,
  FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

/* Those values are set depending on return type
 * they are used in the assembly code in sysv.S
 */
تعريفنوع تعداد kvx_intext_لاره {
  KVX_RET_NONE = 0,
  KVX_RET_SXBD = 1,
  KVX_RET_SXHD = 2,
  KVX_RET_SXWD = 3,
  KVX_RET_ZXBD = 4,
  KVX_RET_ZXHD = 5,
  KVX_RET_ZXWD = 6
} kvx_intext_لاره;

#د_که_پای

/* ---- Definitions for closures ----------------------------------------- */

/* This is only to allow Python to compile
 * but closures are not supported yet
 */
#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_TRAMPOLINE_اندازه 0

#تعریف_کړه FFI_NATIVE_RAW_API 0
#تعریف_کړه FFI_موخه_SPECIFIC_VARIADIC 1
#تعریف_کړه FFI_موخه_HAS_COMPLEX_TYPE

#د_که_پای


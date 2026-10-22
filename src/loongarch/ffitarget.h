/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2022 Xu Chenghua <xuchenghua@loongson.cn>
                               2022 Cheng Lulu <chenglulu@loongson.cn>

   Target configuration macros for LoongArch.

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
#غلطي \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل __loongarch__
#غلطي \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل LIBFFI_ASM

تعريفنوع بنانشان ڊگهو ffi_arg;
تعريفنوع نشانوارو ڊگهو ffi_sarg;

تعريفنوع تعداد ffi_abi
{
  FFI_پهريون_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_آخري_ABI,

#إذا __loongarch_grlen == 64
  #إذا defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #وإلاإذا defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #وإلاإذا defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #وإلا
    #غلطي unsupported LoongArch floating-نقطو ABI
  #جيڪڏهن_پورو
#وإلاإذا __loongarch_grlen == 32
  #إذا defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #وإلاإذا defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #وإلاإذا defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #وإلا
    #غلطي unsupported LoongArch floating-نقطو ABI
  #جيڪڏهن_پورو
#وإلا
  #غلطي unsupported LoongArch بنياد architecture
#جيڪڏهن_پورو
} ffi_abi;

#جيڪڏهن_پورو /* LIBFFI_ASM */

#وصف_ڪريو FFI_هدف_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#وصف_ڪريو FFI_CLOSURES 1
#وصف_ڪريو FFI_GO_CLOSURES 1
#وصف_ڪريو FFI_TRAMPOLINE_سائيز 24
#وصف_ڪريو FFI_NATIVE_RAW_API 0
#وصف_ڪريو FFI_EXTRA_CIF_FIELDS \
  بنانشان loongarch_nfixedargs; \
  بنانشان loongarch_اڻ_استعمال_ٿيل
#وصف_ڪريو FFI_هدف_SPECIFIC_VARIADIC
#جيڪڏهن_پورو

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

#جے_بنا_تعریف LIBFFI_نشانہ_H
#تعریف_کرو LIBFFI_نشانہ_H

#جے_بنا_تعریف LIBFFI_H
#غلطی \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#جے_انت

#جے_بنا_تعریف __loongarch__
#غلطی \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#جے_انت

#جے_بنا_تعریف LIBFFI_ASM

تعريفنوع بنانشانی لما ffi_arg;
تعريفنوع نشانیوالا لما ffi_sarg;

تعريفنوع تعداد ffi_abi
{
  FFI_پہلا_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_آخری_ABI,

#إذا __loongarch_grlen == 64
  #إذا defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #وإلاإذا defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #وإلاإذا defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #وإلا
    #غلطی unsupported LoongArch floating-نقطہ ABI
  #جے_انت
#وإلاإذا __loongarch_grlen == 32
  #إذا defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #وإلاإذا defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #وإلاإذا defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #وإلا
    #غلطی unsupported LoongArch floating-نقطہ ABI
  #جے_انت
#وإلا
  #غلطی unsupported LoongArch ادھار architecture
#جے_انت
} ffi_abi;

#جے_انت /* LIBFFI_ASM */

#تعریف_کرو FFI_نشانہ_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کرو FFI_CLOSURES 1
#تعریف_کرو FFI_GO_CLOSURES 1
#تعریف_کرو FFI_TRAMPOLINE_ناپ 24
#تعریف_کرو FFI_NATIVE_RAW_API 0
#تعریف_کرو FFI_EXTRA_CIF_FIELDS \
  بنانشانی loongarch_nfixedargs; \
  بنانشانی loongarch_بن_ورتیا
#تعریف_کرو FFI_نشانہ_SPECIFIC_VARIADIC
#جے_انت

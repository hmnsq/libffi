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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ਨਿਸ਼ਾਨਾ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_ਨਿਸ਼ਾਨਾ_H

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_H
#ਗਲਤੀ \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __loongarch__
#ਗਲਤੀ \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ffi_sarg;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi
{
  FFI_ਪਹਿਲਾ_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_ਆਖਰੀ_ABI,

#ਜੇ __loongarch_grlen == 64
  #ਜੇ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #ਜਾਂਜੇ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #ਜਾਂਜੇ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #ਨਹੀਂਤਾਂ
    #ਗਲਤੀ unsupported LoongArch floating-ਬਿੰਦੂ ABI
  #ਜੇ_ਅੰਤ
#ਜਾਂਜੇ __loongarch_grlen == 32
  #ਜੇ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #ਜਾਂਜੇ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #ਜਾਂਜੇ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #ਨਹੀਂਤਾਂ
    #ਗਲਤੀ unsupported LoongArch floating-ਬਿੰਦੂ ABI
  #ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
  #ਗਲਤੀ unsupported LoongArch ਅਧਾਰ architecture
#ਜੇ_ਅੰਤ
} ffi_abi;

#ਜੇ_ਅੰਤ /* LIBFFI_ASM */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_GO_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_NATIVE_RAW_API 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_EXTRA_CIF_FIELDS \
  ਬਿਨਾਨਿਸ਼ਾਨੀ loongarch_nfixedargs; \
  ਬਿਨਾਨਿਸ਼ਾਨੀ loongarch_ਅਣਵਰਤਿਆ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_VARIADIC
#ਜੇ_ਅੰਤ

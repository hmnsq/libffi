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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ಗುರಿ_H
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_ಗುರಿ_H

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_H
#ದೋಷ \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __loongarch__
#ದೋಷ \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ LIBFFI_ASM

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ffi_arg;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ದೀರ್ಘ ffi_sarg;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ ffi_abi
{
  FFI_ಮೊದಲ_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_ಕೊನೆಯ_ABI,

#ಆದರೆ __loongarch_grlen == 64
  #ಆದರೆ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #ಅಥವಾಆದರೆ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #ಅಥವಾಆದರೆ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #ಇಲ್ಲದಿದ್ದರೆ
    #ದೋಷ unsupported LoongArch floating-ಬಿಂದು ABI
  #ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ __loongarch_grlen == 32
  #ಆದರೆ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #ಅಥವಾಆದರೆ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #ಅಥವಾಆದರೆ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #ಇಲ್ಲದಿದ್ದರೆ
    #ದೋಷ unsupported LoongArch floating-ಬಿಂದು ABI
  #ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
  #ದೋಷ unsupported LoongArch ತಳ architecture
#ಆದರೆ_ಅಂತ್ಯ
} ffi_abi;

#ಆದರೆ_ಅಂತ್ಯ /* LIBFFI_ASM */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_GO_CLOSURES 1
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TRAMPOLINE_ಗಾತ್ರ 24
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_NATIVE_RAW_API 0
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_EXTRA_CIF_FIELDS \
  ಚಿಹ್ನೆರಹಿತ loongarch_nfixedargs; \
  ಚಿಹ್ನೆರಹಿತ loongarch_ಬಳಸದ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_ಗುರಿ_SPECIFIC_VARIADIC
#ಆದರೆ_ಅಂತ್ಯ

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

#यदि_अपरिभाषितम् LIBFFI_TARGET_H
#परिभाषय LIBFFI_TARGET_H

#यदि_अपरिभाषितम् LIBFFI_H
#दोषः \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#यदि_अन्तः

#यदि_अपरिभाषितम् __loongarch__
#दोषः \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#यदि_अन्तः

#यदि_अपरिभाषितम् LIBFFI_ASM

प्रकारनिर्देश अचिह्नित दीर्घ ffi_arg;
प्रकारनिर्देश चिह्नित दीर्घ ffi_sarg;

प्रकारनिर्देश गणना ffi_abi
{
  FFI_FIRST_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_चरमः_ABI,

#यदि __loongarch_grlen == 64
  #यदि defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #नोचेद्यदि defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #नोचेद्यदि defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #अन्यथा
    #दोषः unsupported LoongArch floating-point ABI
  #यदि_अन्तः
#नोचेद्यदि __loongarch_grlen == 32
  #यदि defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #नोचेद्यदि defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #नोचेद्यदि defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #अन्यथा
    #दोषः unsupported LoongArch floating-point ABI
  #यदि_अन्तः
#अन्यथा
  #दोषः unsupported LoongArch आधारः architecture
#यदि_अन्तः
} ffi_abi;

#यदि_अन्तः /* LIBFFI_ASM */

#परिभाषय FFI_TARGET_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#परिभाषय FFI_CLOSURES 1
#परिभाषय FFI_GO_CLOSURES 1
#परिभाषय FFI_TRAMPOLINE_आकार 24
#परिभाषय FFI_NATIVE_RAW_API 0
#परिभाषय FFI_EXTRA_CIF_FIELDS \
  अचिह्नित loongarch_nfixedargs; \
  अचिह्नित loongarch_अप्रयुक्तम्
#परिभाषय FFI_TARGET_SPECIFIC_VARIADIC
#यदि_अन्तः

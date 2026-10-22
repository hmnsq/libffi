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

#अगर_अघोषित LIBFFI_TARGET_H
#घोषणा LIBFFI_TARGET_H

#अगर_अघोषित LIBFFI_H
#त्रुटि \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#पूर्ण_अगर

#अगर_अघोषित __loongarch__
#त्रुटि \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#पूर्ण_अगर

#अगर_अघोषित LIBFFI_ASM

प्रकार अचिन्हित दीर्घ ffi_arg;
प्रकार चिन्हित दीर्घ ffi_sarg;

प्रकार क्रमागत ffi_abi
{
  FFI_FIRST_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_LAST_ABI,

#अगर __loongarch_grlen == 64
  #अगर defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #या_अगर defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #या_अगर defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #अन्यथा
    #त्रुटि unsupported LoongArch floating-point ABI
  #पूर्ण_अगर
#या_अगर __loongarch_grlen == 32
  #अगर defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #या_अगर defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #या_अगर defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #अन्यथा
    #त्रुटि unsupported LoongArch floating-point ABI
  #पूर्ण_अगर
#अन्यथा
  #त्रुटि unsupported LoongArch base architecture
#पूर्ण_अगर
} ffi_abi;

#पूर्ण_अगर /* LIBFFI_ASM */

#घोषणा FFI_TARGET_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#घोषणा FFI_CLOSURES 1
#घोषणा FFI_GO_CLOSURES 1
#घोषणा FFI_TRAMPOLINE_आकार 24
#घोषणा FFI_NATIVE_RAW_API 0
#घोषणा FFI_EXTRA_CIF_FIELDS \
  अचिन्हित loongarch_nfixedargs; \
  अचिन्हित loongarch_unused
#घोषणा FFI_TARGET_SPECIFIC_VARIADIC
#पूर्ण_अगर

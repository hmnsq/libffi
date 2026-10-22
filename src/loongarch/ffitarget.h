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

#నిర్వచితం_కాకపోతే LIBFFI_లక్ష్యం_H
#నిర్వచించు LIBFFI_లక్ష్యం_H

#నిర్వచితం_కాకపోతే LIBFFI_H
#దోషం \
  "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __loongarch__
#దోషం \
  "libffi was configured for a LoongArch target but this does not appear to be a LoongArch compiler."
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే LIBFFI_ASM

రకనిర్వచనం గుర్తులేని దీర్ఘ ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ ffi_sarg;

రకనిర్వచనం గణన ffi_abi
{
  FFI_మొదటి_ABI = 0,
  FFI_LP64S,
  FFI_LP64F,
  FFI_LP64D,
  FFI_ILP32S,
  FFI_ILP32F,
  FFI_ILP32D,
  FFI_చివరి_ABI,

#ఒకవేళ __loongarch_grlen == 64
  #ఒకవేళ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_LP64S
  #లేదావేళ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_LP64F
  #లేదావేళ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_LP64D
  #లేకపోతే
    #దోషం unsupported LoongArch floating-బిందువు ABI
  #అయితే_ముగింపు
#లేదావేళ __loongarch_grlen == 32
  #ఒకవేళ defined(__loongarch_soft_float)
    FFI_DEFAULT_ABI = FFI_ILP32S
  #లేదావేళ defined(__loongarch_single_float)
    FFI_DEFAULT_ABI = FFI_ILP32F
  #లేదావేళ defined(__loongarch_double_float)
    FFI_DEFAULT_ABI = FFI_ILP32D
  #లేకపోతే
    #దోషం unsupported LoongArch floating-బిందువు ABI
  #అయితే_ముగింపు
#లేకపోతే
  #దోషం unsupported LoongArch పునాది architecture
#అయితే_ముగింపు
} ffi_abi;

#అయితే_ముగింపు /* LIBFFI_ASM */

#నిర్వచించు FFI_లక్ష్యం_HAS_INT128

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_GO_CLOSURES 1
#నిర్వచించు FFI_TRAMPOLINE_కొలత 24
#నిర్వచించు FFI_NATIVE_RAW_API 0
#నిర్వచించు FFI_EXTRA_CIF_FIELDS \
  గుర్తులేని loongarch_nfixedargs; \
  గుర్తులేని loongarch_వాడని
#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC
#అయితే_ముగింపు

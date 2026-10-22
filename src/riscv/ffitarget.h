/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - 2014 Michael Knyszek

   Target configuration macros for RISC-V.

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
#దోషం "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __riscv
#దోషం "libffi was configured for a RISC-V target but this does not appear to be a RISC-V compiler."
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే LIBFFI_ASM

రకనిర్వచనం గుర్తులేని దీర్ఘ ffi_arg;
రకనిర్వచనం   గుర్తుగల దీర్ఘ ffi_sarg;

/* FFI_UNUSED_NN and riscv_unused are to maintain ABI compatibility with a
   distributed Berkeley patch from 2014, and can be removed at SONAME bump */
రకనిర్వచనం గణన ffi_abi {
    FFI_మొదటి_ABI = 0,
    FFI_SYSV,
    FFI_వాడని_1,
    FFI_వాడని_2,
    FFI_వాడని_3,
    FFI_చివరి_ABI,

    FFI_DEFAULT_ABI = FFI_SYSV
} ffi_abi;

#అయితే_ముగింపు /* LIBFFI_ASM */

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_GO_CLOSURES 1
#నిర్వచించు FFI_TRAMPOLINE_కొలత 24
#నిర్వచించు FFI_NATIVE_RAW_API 0
#నిర్వచించు FFI_EXTRA_CIF_FIELDS గుర్తులేని riscv_nfixedargs; గుర్తులేని riscv_వాడని
#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC
#నిర్వచించు FFI_లక్ష్యం_HAS_INT128

#అయితే_ముగింపు


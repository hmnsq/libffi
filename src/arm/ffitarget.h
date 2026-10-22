/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012  Anthony Green
                 Copyright (c) 2010  CodeSourcery
                 Copyright (c) 1996-2003  Red Hat, Inc.

   Target configuration macros for ARM.

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

#నిర్వచితం_కాకపోతే LIBFFI_ASM
రకనిర్వచనం గుర్తులేని దీర్ఘ          ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ            ffi_sarg;

రకనిర్వచనం గణన ffi_abi {
  FFI_మొదటి_ABI = 0,
  FFI_SYSV,
  FFI_VFP,
  FFI_చివరి_ABI,
#ఒకవేళ defined(__ARM_PCS_VFP) || defined(_WIN32)
  FFI_DEFAULT_ABI = FFI_VFP,
#లేకపోతే
  FFI_DEFAULT_ABI = FFI_SYSV,
#అయితే_ముగింపు
} ffi_abi;
#అయితే_ముగింపు

#నిర్వచించు FFI_EXTRA_CIF_FIELDS			\
  పూర్ణాంకం vfp_వాడిన;					\
  గుర్తులేని పొట్టి vfp_reg_free, vfp_nargs;	\
  గుర్తుగల అక్షరం vfp_వాదనలు[16]			\

#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC
#నిర్వచితం_కాకపోతే _WIN32
#నిర్వచించు FFI_లక్ష్యం_HAS_COMPLEX_TYPE
#అయితే_ముగింపు

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_GO_CLOSURES 1
#నిర్వచించు FFI_NATIVE_RAW_API 0

#ఒకవేళ defined (FFI_EXEC_TRAMPOLINE_పట్టిక) && FFI_EXEC_TRAMPOLINE_పట్టిక

#నిర్వచితమైతే __MACH__
#నిర్వచించు FFI_TRAMPOLINE_కొలత 12
#నిర్వచించు FFI_TRAMPOLINE_సంవృతం_స్థానభ్రంశం 8
#లేకపోతే
#దోషం "No trampoline table implementation"
#అయితే_ముగింపు

#లేకపోతే
#నిర్వచితమైతే _WIN32
#నిర్వచించు FFI_TRAMPOLINE_కొలత 16
#నిర్వచించు FFI_TRAMPOLINE_సంవృతం_FUNCTION 12
#లేకపోతే
#నిర్వచించు FFI_TRAMPOLINE_కొలత 12
#అయితే_ముగింపు
#నిర్వచించు FFI_TRAMPOLINE_సంవృతం_స్థానభ్రంశం FFI_TRAMPOLINE_కొలత
#అయితే_ముగింపు

#అయితే_ముగింపు

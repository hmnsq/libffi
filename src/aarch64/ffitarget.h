/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#నిర్వచితం_కాకపోతే LIBFFI_లక్ష్యం_H
#నిర్వచించు LIBFFI_లక్ష్యం_H

#నిర్వచితం_కాకపోతే LIBFFI_H
#దోషం "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే LIBFFI_ASM
#నిర్వచితమైతే __ILP32__
#నిర్వచించు FFI_SIZEOF_ARG 8
#నిర్వచించు FFI_SIZEOF_JAVA_RAW  4
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ దీర్ఘ ffi_sarg;
#లేదావేళ defined(_WIN32)
#నిర్వచించు FFI_SIZEOF_ARG 8
రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ దీర్ఘ ffi_sarg;
#లేకపోతే
రకనిర్వచనం గుర్తులేని దీర్ఘ ffi_arg;
రకనిర్వచనం గుర్తుగల దీర్ఘ ffi_sarg;
#అయితే_ముగింపు

రకనిర్వచనం గణన ffi_abi
  {
    FFI_మొదటి_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_చివరి_ABI,
#ఒకవేళ defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#లేకపోతే
    FFI_DEFAULT_ABI = FFI_SYSV
#అయితే_ముగింపు
  } ffi_abi;
#అయితే_ముగింపు

/* ---- Definitions for closures ----------------------------------------- */

#నిర్వచించు FFI_CLOSURES 1
#నిర్వచించు FFI_NATIVE_RAW_API 0

#ఒకవేళ defined (FFI_EXEC_TRAMPOLINE_పట్టిక) && FFI_EXEC_TRAMPOLINE_పట్టిక

#నిర్వచితమైతే __MACH__
#నిర్వచించు FFI_TRAMPOLINE_కొలత 16
#నిర్వచించు FFI_TRAMPOLINE_సంవృతం_స్థానభ్రంశం 16
#లేకపోతే
#దోషం "No trampoline table implementation"
#అయితే_ముగింపు

#లేకపోతే
#నిర్వచించు FFI_TRAMPOLINE_కొలత 24
#నిర్వచించు FFI_TRAMPOLINE_సంవృతం_స్థానభ్రంశం FFI_TRAMPOLINE_కొలత
#అయితే_ముగింపు

#నిర్వచితమైతే _WIN32
#నిర్వచించు FFI_EXTRA_CIF_FIELDS గుర్తులేని is_variadic
#అయితే_ముగింపు
#నిర్వచించు FFI_లక్ష్యం_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#ఒకవేళ defined (__APPLE__)
#నిర్వచించు FFI_EXTRA_CIF_FIELDS గుర్తులేని aarch64_nfixedargs
#లేదావేళ !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#నిర్వచించు FFI_GO_CLOSURES 1
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే _WIN32
/* No complex type on Windows */
#నిర్వచించు FFI_లక్ష్యం_HAS_COMPLEX_TYPE
#అయితే_ముగింపు

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#నిర్వచించు FFI_లక్ష్యం_HAS_సదిశ_TYPE

#నిర్వచించు FFI_లక్ష్యం_HAS_INT128 1

#అయితే_ముగింపు

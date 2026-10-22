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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ਨਿਸ਼ਾਨਾ_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_ਨਿਸ਼ਾਨਾ_H

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_H
#ਗਲਤੀ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ILP32__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_ARG 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_JAVA_RAW  4
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ਲੰਮਾ ffi_sarg;
#ਜਾਂਜੇ defined(_WIN32)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_SIZEOF_ARG 8
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ਲੰਮਾ ffi_sarg;
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ ffi_sarg;
#ਜੇ_ਅੰਤ

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi
  {
    FFI_ਪਹਿਲਾ_ABI = 0,
    FFI_SYSV,
    FFI_WIN64,
    FFI_ਆਖਰੀ_ABI,
#ਜੇ defined(_WIN32)
    FFI_DEFAULT_ABI = FFI_WIN64
#ਨਹੀਂਤਾਂ
    FFI_DEFAULT_ABI = FFI_SYSV
#ਜੇ_ਅੰਤ
  } ffi_abi;
#ਜੇ_ਅੰਤ

/* ---- Definitions for closures ----------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_NATIVE_RAW_API 0

#ਜੇ defined (FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ) && FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MACH__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 16
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਘੇਰਾ_ਵਿੱਥ 16
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "No trampoline table implementation"
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 24
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਘੇਰਾ_ਵਿੱਥ FFI_TRAMPOLINE_ਮਾਪ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_EXTRA_CIF_FIELDS ਬਿਨਾਨਿਸ਼ਾਨੀ is_variadic
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_VARIADIC

/* ---- Internal ---- */

#ਜੇ defined (__APPLE__)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_EXTRA_CIF_FIELDS ਬਿਨਾਨਿਸ਼ਾਨੀ aarch64_nfixedargs
#ਜਾਂਜੇ !defined(_WIN32) && !defined(__ANDROID__)
/* iOS, Windows and Android reserve x18 for the system.  Disable Go closures until
   a new static chain is chosen.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_GO_CLOSURES 1
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _WIN32
/* No complex type on Windows */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
#ਜੇ_ਅੰਤ

/* AAPCS64 passes 8- and 16-byte vectors in V/Q registers and homogeneous
   vector aggregates in consecutive V/Q registers; see is_vfp_type.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_ਸਦਿਸ਼_TYPE

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_HAS_INT128 1

#ਜੇ_ਅੰਤ

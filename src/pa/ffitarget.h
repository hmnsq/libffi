/* -----------------------------------------------------------------*-C-*-
   ffitarget.h - Copyright (c) 2012, 2026  Anthony Green
                 Copyright (c) 1996-2003  Red Hat, Inc.
   Target configuration macros for hppa.

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
#ਗਲਤੀ "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#ਜੇ_ਅੰਤ

/* ---- System specific configurations ----------------------------------- */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ LIBFFI_ASM
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ          ffi_arg;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ            ffi_sarg;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ ffi_abi {
  FFI_ਪਹਿਲਾ_ABI = 0,

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_LINUX
  FFI_PA32,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA_HPUX
  FFI_PA32,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ PA64_HPUX
  FFI_PA64,
  FFI_ਆਖਰੀ_ABI,
  FFI_DEFAULT_ABI = FFI_PA64
#ਜੇ_ਅੰਤ
} ffi_abi;
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_ਢੇਰ_SPACE_ਵੰਡ

/* ---- Definitions for closures ----------------------------------------- */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_CLOSURES 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_NATIVE_RAW_API 0
#ਜੇ defined(PA64_HPUX)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 32
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TRAMPOLINE_ਮਾਪ 12
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT1 -1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT2 -2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT3 -3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT4 -4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT5 -5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT6 -6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT7 -7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPE_SMALL_STRUCT8 -8

/* The return-value jump tables in linux.S and hpux32.S are indexed by
   cif->flags, which ffi_prep_cif_machdep derives from the return type.  Any
   return type it does not handle explicitly -- including FFI_TYPE_COMPLEX and
   the 128-bit integer types FFI_TYPE_UINT128/FFI_TYPE_SINT128 -- falls through
   to the default case and is mapped to FFI_TYPE_INT, so cif->flags never
   exceeds FFI_TYPE_COMPLEX and the existing tables remain sufficient.  Bump
   FFI_PA_TYPE_LAST to the current FFI_TYPE_LAST once you have confirmed any
   newly added generic type is likewise handled (or the tables extended).

   FFI_TYPE_VECTOR (18) is likewise not reached here: PA does not define
   FFI_TARGET_HAS_VECTOR_TYPE, so ffi_prep_cif_core rejects any vector
   signature with FFI_BAD_TYPEDEF before machdep runs.  Bumping the tripwire
   past it is therefore safe.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_PA_TYPE_ਆਖਰੀ FFI_TYPE_ਸਦਿਸ਼

/* Tripwire: when a new generic type is added FFI_TYPE_LAST changes and this
   fires, forcing a review of ffi_prep_cif_machdep and the linux.S / hpux32.S
   jump tables before FFI_PA_TYPE_LAST above is bumped.  */
#ਜੇ FFI_TYPE_ਆਖਰੀ != FFI_PA_TYPE_ਆਖਰੀ
# error "You likely have broken jump tables"
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ

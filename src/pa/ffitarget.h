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

#که_نه_تعریف_شوی LIBFFI_موخه_H
#تعریف_کړه LIBFFI_موخه_H

#که_نه_تعریف_شوی LIBFFI_H
#تېروتنه "Please do not include ffitarget.h directly into your source.  Use ffi.h instead."
#د_که_پای

/* ---- System specific configurations ----------------------------------- */

#که_نه_تعریف_شوی LIBFFI_ASM
تعريفنوع بینښه اوږد          ffi_arg;
تعريفنوع نښهدار اوږد            ffi_sarg;

تعريفنوع تعداد ffi_abi {
  FFI_لومړی_ABI = 0,

#که_تعریف_شوی PA_LINUX
  FFI_PA32,
  FFI_وروستی_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#د_که_پای

#که_تعریف_شوی PA_HPUX
  FFI_PA32,
  FFI_وروستی_ABI,
  FFI_DEFAULT_ABI = FFI_PA32
#د_که_پای

#که_تعریف_شوی PA64_HPUX
  FFI_PA64,
  FFI_وروستی_ABI,
  FFI_DEFAULT_ABI = FFI_PA64
#د_که_پای
} ffi_abi;
#د_که_پای

#تعریف_کړه FFI_موخه_SPECIFIC_ډېری_SPACE_ویش

/* ---- Definitions for closures ----------------------------------------- */

#تعریف_کړه FFI_CLOSURES 1
#تعریف_کړه FFI_NATIVE_RAW_API 0
#إذا defined(PA64_HPUX)
#تعریف_کړه FFI_TRAMPOLINE_اندازه 32
#وإلا
#تعریف_کړه FFI_TRAMPOLINE_اندازه 12
#د_که_پای

#تعریف_کړه FFI_TYPE_SMALL_STRUCT1 -1
#تعریف_کړه FFI_TYPE_SMALL_STRUCT2 -2
#تعریف_کړه FFI_TYPE_SMALL_STRUCT3 -3
#تعریف_کړه FFI_TYPE_SMALL_STRUCT4 -4
#تعریف_کړه FFI_TYPE_SMALL_STRUCT5 -5
#تعریف_کړه FFI_TYPE_SMALL_STRUCT6 -6
#تعریف_کړه FFI_TYPE_SMALL_STRUCT7 -7
#تعریف_کړه FFI_TYPE_SMALL_STRUCT8 -8

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
#تعریف_کړه FFI_PA_TYPE_وروستی FFI_TYPE_ویکټور

/* Tripwire: when a new generic type is added FFI_TYPE_LAST changes and this
   fires, forcing a review of ffi_prep_cif_machdep and the linux.S / hpux32.S
   jump tables before FFI_PA_TYPE_LAST above is bumped.  */
#إذا FFI_TYPE_وروستی != FFI_PA_TYPE_وروستی
# error "You likely have broken jump tables"
#د_که_پای

#د_که_پای

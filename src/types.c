/* -----------------------------------------------------------------------
   types.c - Copyright (c) 1996, 1998, 2024, 2025, 2026  Red Hat, Inc.

   Predefined ffi_types needed by libffi.

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

/* Hide the basic type definitions from the header file, so that we
   can redefine them here as "const".  */
#ವ್ಯಾಖ್ಯಾನಿಸು LIBFFI_HIDE_BASIC_TYPES

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

/* Return a version string. */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ffi_get_ಆವೃತ್ತಿ (ಶೂನ್ಯ)
{
  ಹಿಂತಿರುಗಿ FFI_ಆವೃತ್ತಿ_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ffi_get_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ (ಶೂನ್ಯ)
{
  ಹಿಂತಿರುಗಿ FFI_ಆವೃತ್ತಿ_ಸಂಖ್ಯೆ;
}

ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ffi_get_default_abi (ಶೂನ್ಯ)
{
  ಹಿಂತಿರುಗಿ FFI_DEFAULT_ABI;
}

size_t ffi_get_ಆವರಣ_ಗಾತ್ರ (ಶೂನ್ಯ)
{
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ffi_ಆವರಣ);
}

/* Type definitions */

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_TYPEDEF(name, type, id, maybe_const)\
ರಚನೆ struct_align_##name {			\
  ಅಕ್ಷರ c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  ಗಾತ್ರ(type),					\
  offsetof(ರಚನೆ struct_align_##name, x),	\
  id, ಖಾಲಿ					\
}

#ವ್ಯಾಖ್ಯಾನಿಸು FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
ಸ್ಥಿರ ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), ಖಾಲಿ		\
};							\
ರಚನೆ struct_align_complex_##name {			\
  ಅಕ್ಷರ c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  ಗಾತ್ರ(_Complex type),				\
  offsetof(ರಚನೆ struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN ಸ್ಥಿರಾಂಕ ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, ಖಾಲಿ
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, ಸ್ಥಿರಾಂಕ);

FFI_TYPEDEF(ಸೂಚಕ_2, ಶೂನ್ಯ*, FFI_TYPE_ಸೂಚಕ, ಸ್ಥಿರಾಂಕ);

FFI_TYPEDEF(ದಶಮಾಂಶ, ದಶಮಾಂಶ, FFI_TYPE_FLOAT, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(ದ್ವಿಗುಣ, ದ್ವಿಗುಣ, FFI_TYPE_DOUBLE, ಸ್ಥಿರಾಂಕ);

#ಆದರೆ !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_LDBL_CONST ಸ್ಥಿರಾಂಕ
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FFI_LDBL_CONST
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# ಆದರೆ defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ಹೊರಹೋಗು ದ date
# endif
ಸ್ಥಿರಾಂಕ ffi_type ffi_type_longdouble = { 16, 16, 4, ಖಾಲಿ };
#ಇಲ್ಲದಿದ್ದರೆ
FFI_TYPEDEF(longdouble, ದೀರ್ಘ ದ್ವಿಗುಣ, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(ದಶಮಾಂಶ, ದಶಮಾಂಶ, ಸ್ಥಿರಾಂಕ);
FFI_COMPLEX_TYPEDEF(ದ್ವಿಗುಣ, ದ್ವಿಗುಣ, ಸ್ಥಿರಾಂಕ);
FFI_COMPLEX_TYPEDEF(longdouble, ದೀರ್ಘ ದ್ವಿಗುಣ, FFI_LDBL_CONST);
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, ಸ್ಥಿರಾಂಕ);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, ಸ್ಥಿರಾಂಕ);
# ಇಲ್ಲದಿದ್ದರೆ
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
ಸ್ಥಿರಾಂಕ ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, ಖಾಲಿ };
ಸ್ಥಿರಾಂಕ ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, ಖಾಲಿ };
# endif
#ಆದರೆ_ಅಂತ್ಯ /* FFI_TARGET_HAS_INT128 */

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
#define LIBFFI_HIDE_BASIC_TYPES

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

/* Return a version string. */
ಧ್ರುವ ವರ್ಣ *ffi_get_version (ಶೂನ್ಯ)
{
  ಪ್ರತ್ಯಾಗಮ FFI_VERSION_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
unsigned long ffi_get_version_ಸಂಖ್ಯೆ (ಶೂನ್ಯ)
{
  ಪ್ರತ್ಯಾಗಮ FFI_VERSION_ಸಂಖ್ಯೆ;
}

unsigned ಪೂರ್ಣಾಙ್ಕ ffi_get_default_abi (ಶೂನ್ಯ)
{
  ಪ್ರತ್ಯಾಗಮ FFI_DEFAULT_ABI;
}

size_t ffi_get_closure_size (ಶೂನ್ಯ)
{
  ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(ffi_closure);
}

/* Type definitions */

#define FFI_TYPEDEF(name, type, id, maybe_const)\
ಸಂರಚನಾ struct_align_##name {			\
  ವರ್ಣ c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  ಪರಿಮಾಣ(type),					\
  offsetof(ಸಂರಚನಾ struct_align_##name, x),	\
  id, ರಿಕ್ತ					\
}

#define FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
ಸ್ಥಿರ ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), ರಿಕ್ತ		\
};							\
ಸಂರಚನಾ struct_align_complex_##name {			\
  ವರ್ಣ c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  ಪರಿಮಾಣ(_Complex type),				\
  offsetof(ಸಂರಚನಾ struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN ಧ್ರುವ ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, ರಿಕ್ತ
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, ಧ್ರುವ);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, ಧ್ರುವ);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, ಧ್ರುವ);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, ಧ್ರುವ);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, ಧ್ರುವ);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, ಧ್ರುವ);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, ಧ್ರುವ);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, ಧ್ರುವ);

FFI_TYPEDEF(pointer, ಶೂನ್ಯ*, FFI_TYPE_POINTER, ಧ್ರುವ);

FFI_TYPEDEF(ಭಿನ್ನ, ಭಿನ್ನ, FFI_TYPE_FLOAT, ಧ್ರುವ);
FFI_TYPEDEF(ದ್ವಿಗುಣ, ದ್ವಿಗುಣ, FFI_TYPE_DOUBLE, ಧ್ರುವ);

#ಯದಿ !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#define FFI_LDBL_CONST ಧ್ರುವ
#ಅನ್ಯಥಾ
#define FFI_LDBL_CONST
#endif

#ifdef __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# ಯದಿ defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
ಧ್ರುವ ffi_type ffi_type_longdouble = { 16, 16, 4, ರಿಕ್ತ };
#ಅನ್ಯಥಾ
FFI_TYPEDEF(longdouble, long ದ್ವಿಗುಣ, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(ಭಿನ್ನ, ಭಿನ್ನ, ಧ್ರುವ);
FFI_COMPLEX_TYPEDEF(ದ್ವಿಗುಣ, ದ್ವಿಗುಣ, ಧ್ರುವ);
FFI_COMPLEX_TYPEDEF(longdouble, long ದ್ವಿಗುಣ, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, ಧ್ರುವ);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, ಧ್ರುವ);
# ಅನ್ಯಥಾ
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
ಧ್ರುವ ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, ರಿಕ್ತ };
ಧ್ರುವ ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, ರಿಕ್ತ };
# endif
#endif /* FFI_TARGET_HAS_INT128 */

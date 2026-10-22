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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LIBFFI_HIDE_BASIC_TYPES

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

/* Return a version string. */
ਅਚਲ ਅੱਖਰ *ffi_get_ਸੰਸਕਰਣ (ਖਾਲੀ)
{
  ਵਾਪਸ FFI_ਸੰਸਕਰਣ_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ffi_get_ਸੰਸਕਰਣ_ਸੰਖਿਆ (ਖਾਲੀ)
{
  ਵਾਪਸ FFI_ਸੰਸਕਰਣ_ਸੰਖਿਆ;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ffi_get_default_abi (ਖਾਲੀ)
{
  ਵਾਪਸ FFI_DEFAULT_ABI;
}

size_t ffi_get_ਘੇਰਾ_ਮਾਪ (ਖਾਲੀ)
{
  ਵਾਪਸ ਆਕਾਰ(ffi_ਘੇਰਾ);
}

/* Type definitions */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_TYPEDEF(name, type, id, maybe_const)\
ਬਣਤਰ struct_align_##name {			\
  ਅੱਖਰ c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  ਆਕਾਰ(type),					\
  offsetof(ਬਣਤਰ struct_align_##name, x),	\
  id, ਨਿਰਾ					\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
ਸਥਿਰ ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), ਨਿਰਾ		\
};							\
ਬਣਤਰ struct_align_complex_##name {			\
  ਅੱਖਰ c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  ਆਕਾਰ(_Complex type),				\
  offsetof(ਬਣਤਰ struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN ਅਚਲ ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, ਨਿਰਾ
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, ਅਚਲ);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, ਅਚਲ);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, ਅਚਲ);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, ਅਚਲ);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, ਅਚਲ);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, ਅਚਲ);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, ਅਚਲ);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, ਅਚਲ);

FFI_TYPEDEF(ਸੂਚਕ, ਖਾਲੀ*, FFI_TYPE_ਸੂਚਕ, ਅਚਲ);

FFI_TYPEDEF(ਦਸ਼ਮਲਵ, ਦਸ਼ਮਲਵ, FFI_TYPE_FLOAT, ਅਚਲ);
FFI_TYPEDEF(ਦੁੱਗਣਾ, ਦੁੱਗਣਾ, FFI_TYPE_DOUBLE, ਅਚਲ);

#ਜੇ !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_LDBL_CONST ਅਚਲ
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_LDBL_CONST
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# ਜੇ defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
ਅਚਲ ffi_type ffi_type_longdouble = { 16, 16, 4, ਨਿਰਾ };
#ਨਹੀਂਤਾਂ
FFI_TYPEDEF(longdouble, ਲੰਮਾ ਦੁੱਗਣਾ, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(ਦਸ਼ਮਲਵ, ਦਸ਼ਮਲਵ, ਅਚਲ);
FFI_COMPLEX_TYPEDEF(ਦੁੱਗਣਾ, ਦੁੱਗਣਾ, ਅਚਲ);
FFI_COMPLEX_TYPEDEF(longdouble, ਲੰਮਾ ਦੁੱਗਣਾ, FFI_LDBL_CONST);
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, ਅਚਲ);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, ਅਚਲ);
# ਨਹੀਂਤਾਂ
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
ਅਚਲ ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, ਨਿਰਾ };
ਅਚਲ ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, ਨਿਰਾ };
# endif
#ਜੇ_ਅੰਤ /* FFI_TARGET_HAS_INT128 */

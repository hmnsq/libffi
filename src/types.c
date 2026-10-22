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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

/* Return a version string. */
અચલ અક્ષર *ffi_get_version (શૂન્ય)
{
  પાછા FFI_VERSION_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
unsigned long ffi_get_version_સંખ્યા (શૂન્ય)
{
  પાછા FFI_VERSION_સંખ્યા;
}

unsigned પૂર્ણાંક ffi_get_default_abi (શૂન્ય)
{
  પાછા FFI_DEFAULT_ABI;
}

size_t ffi_get_closure_size (શૂન્ય)
{
  પાછા કદ(ffi_closure);
}

/* Type definitions */

#define FFI_TYPEDEF(name, type, id, maybe_const)\
સંરચના struct_align_##name {			\
  અક્ષર c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  કદ(type),					\
  offsetof(સંરચના struct_align_##name, x),	\
  id, ખાલી					\
}

#define FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
સ્થિર ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), ખાલી		\
};							\
સંરચના struct_align_complex_##name {			\
  અક્ષર c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  કદ(_Complex type),				\
  offsetof(સંરચના struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN અચલ ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, ખાલી
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, અચલ);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, અચલ);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, અચલ);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, અચલ);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, અચલ);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, અચલ);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, અચલ);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, અચલ);

FFI_TYPEDEF(pointer, શૂન્ય*, FFI_TYPE_POINTER, અચલ);

FFI_TYPEDEF(દશાંશ, દશાંશ, FFI_TYPE_FLOAT, અચલ);
FFI_TYPEDEF(દ્વિગુણ, દ્વિગુણ, FFI_TYPE_DOUBLE, અચલ);

#જો !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#define FFI_LDBL_CONST અચલ
#નહીંતર
#define FFI_LDBL_CONST
#endif

#ifdef __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# જો defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
અચલ ffi_type ffi_type_longdouble = { 16, 16, 4, ખાલી };
#નહીંતર
FFI_TYPEDEF(longdouble, long દ્વિગુણ, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(દશાંશ, દશાંશ, અચલ);
FFI_COMPLEX_TYPEDEF(દ્વિગુણ, દ્વિગુણ, અચલ);
FFI_COMPLEX_TYPEDEF(longdouble, long દ્વિગુણ, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, અચલ);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, અચલ);
# નહીંતર
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
અચલ ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, ખાલી };
અચલ ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, ખાલી };
# endif
#endif /* FFI_TARGET_HAS_INT128 */

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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

/* Return a version string. */
स्थिरांक अक्षर *ffi_get_version (शून्य)
{
  परत FFI_VERSION_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
अचिन्हित दीर्घ ffi_get_version_संख्या (शून्य)
{
  परत FFI_VERSION_संख्या;
}

अचिन्हित पूर्णांक ffi_get_default_abi (शून्य)
{
  परत FFI_DEFAULT_ABI;
}

माप_प्रकार ffi_get_closure_size (शून्य)
{
  परत आकार(ffi_closure);
}

/* Type definitions */

#define FFI_TYPEDEF(name, type, id, maybe_const)\
संरचना struct_align_##name {			\
  अक्षर c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  आकार(type),					\
  दुरत्व(संरचना struct_align_##name, x),	\
  id, रिक्त					\
}

#define FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
स्थिर ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), रिक्त		\
};							\
संरचना struct_align_complex_##name {			\
  अक्षर c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  आकार(_Complex type),				\
  दुरत्व(संरचना struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN स्थिरांक ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, रिक्त
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, स्थिरांक);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, स्थिरांक);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, स्थिरांक);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, स्थिरांक);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, स्थिरांक);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, स्थिरांक);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, स्थिरांक);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, स्थिरांक);

FFI_TYPEDEF(pointer, शून्य*, FFI_TYPE_POINTER, स्थिरांक);

FFI_TYPEDEF(दशांश, दशांश, FFI_TYPE_FLOAT, स्थिरांक);
FFI_TYPEDEF(द्विगुण, द्विगुण, FFI_TYPE_DOUBLE, स्थिरांक);

#जर !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#define FFI_LDBL_CONST स्थिरांक
#अन्यथा
#define FFI_LDBL_CONST
#endif

#ifdef __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# जर defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
स्थिरांक ffi_type ffi_type_longdouble = { 16, 16, 4, रिक्त };
#अन्यथा
FFI_TYPEDEF(longdouble, दीर्घ द्विगुण, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(दशांश, दशांश, स्थिरांक);
FFI_COMPLEX_TYPEDEF(द्विगुण, द्विगुण, स्थिरांक);
FFI_COMPLEX_TYPEDEF(longdouble, दीर्घ द्विगुण, FFI_LDBL_CONST);
#endif

#ifdef FFI_TARGET_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, स्थिरांक);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, स्थिरांक);
# अन्यथा
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
स्थिरांक ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, रिक्त };
स्थिरांक ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, रिक्त };
# endif
#endif /* FFI_TARGET_HAS_INT128 */

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
#घोषणा LIBFFI_HIDE_BASIC_TYPES

#समावेश <ffi.h>
#समावेश <ffi_common.h>

/* Return a version string. */
स्थिर अक्षर *ffi_get_version (व्योम)
{
  वापस FFI_VERSION_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
अचिन्हित दीर्घ ffi_get_version_संख्या (व्योम)
{
  वापस FFI_VERSION_संख्या;
}

अचिन्हित पूर्णांक ffi_get_default_abi (व्योम)
{
  वापस FFI_DEFAULT_ABI;
}

माप_प्रकार ffi_get_closure_आकार (व्योम)
{
  वापस माप(ffi_closure);
}

/* Type definitions */

#घोषणा FFI_TYPEDEF(name, type, id, maybe_const)\
काष्ठा struct_align_##name {			\
  अक्षर c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  माप(type),					\
  दुरत्व(काष्ठा struct_align_##name, x),	\
  id, शून्य					\
}

#घोषणा FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
अटल ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), शून्य		\
};							\
काष्ठा struct_align_complex_##name {			\
  अक्षर c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  माप(_Complex type),				\
  दुरत्व(काष्ठा struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN स्थिर ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, शून्य
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, स्थिर);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, स्थिर);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, स्थिर);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, स्थिर);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, स्थिर);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, स्थिर);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, स्थिर);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, स्थिर);

FFI_TYPEDEF(pointer, व्योम*, FFI_TYPE_POINTER, स्थिर);

FFI_TYPEDEF(भग्न, भग्न, FFI_TYPE_FLOAT, स्थिर);
FFI_TYPEDEF(द्विगुन, द्विगुन, FFI_TYPE_DOUBLE, स्थिर);

#अगर !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#घोषणा FFI_LDBL_CONST स्थिर
#अन्यथा
#घोषणा FFI_LDBL_CONST
#पूर्ण_अगर

#अगर_घोषित __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# अगर defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
स्थिर ffi_type ffi_type_longdouble = { 16, 16, 4, शून्य };
#अन्यथा
FFI_TYPEDEF(longdouble, दीर्घ द्विगुन, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#पूर्ण_अगर

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(भग्न, भग्न, स्थिर);
FFI_COMPLEX_TYPEDEF(द्विगुन, द्विगुन, स्थिर);
FFI_COMPLEX_TYPEDEF(longdouble, दीर्घ द्विगुन, FFI_LDBL_CONST);
#पूर्ण_अगर

#अगर_घोषित FFI_TARGET_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, स्थिर);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, स्थिर);
# अन्यथा
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
स्थिर ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, शून्य };
स्थिर ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, शून्य };
# endif
#पूर्ण_अगर /* FFI_TARGET_HAS_INT128 */

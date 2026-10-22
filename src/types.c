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
#నిర్వచించు LIBFFI_HIDE_BASIC_TYPES

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

/* Return a version string. */
స్థిరరాశి అక్షరం *ffi_get_సంస్కరణ (శూన్య)
{
  తిరిగి FFI_సంస్కరణ_STRING;
}

/* Return the version as an unsigned long value: (x * 10000 + y * 100 + z) */
గుర్తులేని దీర్ఘ ffi_get_సంస్కరణ_సంఖ్య (శూన్య)
{
  తిరిగి FFI_సంస్కరణ_సంఖ్య;
}

గుర్తులేని పూర్ణాంకం ffi_get_default_abi (శూన్య)
{
  తిరిగి FFI_DEFAULT_ABI;
}

size_t ffi_get_సంవృతం_కొలత (శూన్య)
{
  తిరిగి పరిమాణం(ffi_సంవృతం);
}

/* Type definitions */

#నిర్వచించు FFI_TYPEDEF(name, type, id, maybe_const)\
నిర్మాణం struct_align_##name {			\
  అక్షరం c;					\
  type x;					\
};						\
FFI_EXTERN					\
maybe_const ffi_type ffi_type_##name = {	\
  పరిమాణం(type),					\
  offsetof(నిర్మాణం struct_align_##name, x),	\
  id, ఖాళీ					\
}

#నిర్వచించు FFI_COMPLEX_TYPEDEF(name, type, maybe_const)	\
స్థిర ffi_type *ffi_elements_complex_##name [2] = {	\
	(ffi_type *)(&ffi_type_##name), ఖాళీ		\
};							\
నిర్మాణం struct_align_complex_##name {			\
  అక్షరం c;						\
  _Complex type x;					\
};							\
FFI_EXTERN						\
maybe_const ffi_type ffi_type_complex_##name = {	\
  పరిమాణం(_Complex type),				\
  offsetof(నిర్మాణం struct_align_complex_##name, x),	\
  FFI_TYPE_COMPLEX,					\
  (ffi_type **)ffi_elements_complex_##name		\
}

/* Size and alignment are fake here. They must not be 0. */
FFI_EXTERN స్థిరరాశి ffi_type ffi_type_void = {
  1, 1, FFI_TYPE_VOID, ఖాళీ
};

FFI_TYPEDEF(uint8, UINT8, FFI_TYPE_UINT8, స్థిరరాశి);
FFI_TYPEDEF(sint8, SINT8, FFI_TYPE_SINT8, స్థిరరాశి);
FFI_TYPEDEF(uint16, UINT16, FFI_TYPE_UINT16, స్థిరరాశి);
FFI_TYPEDEF(sint16, SINT16, FFI_TYPE_SINT16, స్థిరరాశి);
FFI_TYPEDEF(uint32, UINT32, FFI_TYPE_UINT32, స్థిరరాశి);
FFI_TYPEDEF(sint32, SINT32, FFI_TYPE_SINT32, స్థిరరాశి);
FFI_TYPEDEF(uint64, UINT64, FFI_TYPE_UINT64, స్థిరరాశి);
FFI_TYPEDEF(sint64, SINT64, FFI_TYPE_SINT64, స్థిరరాశి);

FFI_TYPEDEF(సూచిక_2, శూన్య*, FFI_TYPE_సూచిక, స్థిరరాశి);

FFI_TYPEDEF(దశాంశం, దశాంశం, FFI_TYPE_FLOAT, స్థిరరాశి);
FFI_TYPEDEF(ద్విగుణ, ద్విగుణ, FFI_TYPE_DOUBLE, స్థిరరాశి);

#ఒకవేళ !defined HAVE_LONG_DOUBLE_VARIANT || defined __alpha__
#నిర్వచించు FFI_LDBL_CONST స్థిరరాశి
#లేకపోతే
#నిర్వచించు FFI_LDBL_CONST
#అయితే_ముగింపు

#నిర్వచితమైతే __alpha__
/* Even if we're not configured to default to 128-bit long double,
   maintain binary compatibility, as -mlong-double-128 can be used
   at any time.  */
/* Validate the hard-coded number below.  */
# ఒకవేళ defined(__LONG_DOUBLE_128__) && FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE బహిర్గతం యొక్క date
# endif
స్థిరరాశి ffi_type ffi_type_longdouble = { 16, 16, 4, ఖాళీ };
#లేకపోతే
FFI_TYPEDEF(longdouble, దీర్ఘ ద్విగుణ, FFI_TYPE_LONGDOUBLE, FFI_LDBL_CONST);
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
FFI_COMPLEX_TYPEDEF(దశాంశం, దశాంశం, స్థిరరాశి);
FFI_COMPLEX_TYPEDEF(ద్విగుణ, ద్విగుణ, స్థిరరాశి);
FFI_COMPLEX_TYPEDEF(longdouble, దీర్ఘ ద్విగుణ, FFI_LDBL_CONST);
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_INT128
# ifdef HAVE_INT128
FFI_TYPEDEF(uint128, __uint128_t, FFI_TYPE_UINT128, స్థిరరాశి);
FFI_TYPEDEF(sint128, __int128_t, FFI_TYPE_SINT128, స్థిరరాశి);
# లేకపోతే
/* We don't actually need compiler support for __int128_t, as we don't
   use the type itself within libffi.  Obviously the size is 16; make
   the conservative assumption that the alignment must also be 16. */
స్థిరరాశి ffi_type ffi_type_uint128 = { 16, 16, FFI_TYPE_UINT128, ఖాళీ };
స్థిరరాశి ffi_type ffi_type_sint128 = { 16, 16, FFI_TYPE_SINT128, ఖాళీ };
# endif
#అయితే_ముగింపు /* FFI_TARGET_HAS_INT128 */

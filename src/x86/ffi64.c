/* -----------------------------------------------------------------------
   ffi64.c - Copyright (c) 2011, 2018, 2022, 2026  Anthony Green
             Copyright (c) 2013  The Written Word, Inc.
             Copyright (c) 2008, 2010  Red Hat, Inc.
             Copyright (c) 2002, 2007  Bo Thorsen <bo@suse.de>

   x86-64 Foreign Function Interface

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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stddef.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ଅନ୍ତର୍ଭାଵ "internal64.h"

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#ଯଦି defined(__INTEL_COMPILER)
#ଅନ୍ତର୍ଭାଵ "xmmintrin.h"
#define UINT128 __m128
#ଅନ୍ୟଥା
#ଯଦି defined(__SUNPRO_C)
#ଅନ୍ତର୍ଭାଵ <sunmedia_types.h>
#define UINT128 __m128i
#ଅନ୍ୟଥା
#define UINT128 __int128_t
#endif
#endif

ସଂଘ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

ସଂରଚନା register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  ସଂଘ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_unix64 (ଶୂନ୍ୟ *args, unsigned long bytes, unsigned flags,
			     ଶୂନ୍ୟ *raddr, ଶୂନ୍ୟ (*fnaddr)(ଶୂନ୍ୟ)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
ଗଣନା x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87UP_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_MEMORY_CLASS
  };

#define MAX_CLASSES 4

#define SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#ଅନ୍ୟଥା
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ସ୍ଥିର ଗଣନା x86_64_reg_class
merge_classes (ଗଣନା x86_64_reg_class class1, ଗଣନା x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  ଯଦି (class1 == class2)
    ଫେରନ୍ତୁ class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  ଯଦି (class1 == X86_64_NO_CLASS)
    ଫେରନ୍ତୁ class2;
  ଯଦି (class2 == X86_64_NO_CLASS)
    ଫେରନ୍ତୁ class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  ଯଦି (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    ଫେରନ୍ତୁ X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  ଯଦି ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    ଫେରନ୍ତୁ X86_64_INTEGERSI_CLASS;
  ଯଦି (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    ଫେରନ୍ତୁ X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  ଯଦି (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    ଫେରନ୍ତୁ X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  ଫେରନ୍ତୁ X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ସ୍ଥିର size_t
classify_argument (ffi_type *type, ଗଣନା x86_64_reg_class classes[],
		   size_t byte_offset)
{
  ବାଛ (type->type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
    ସ୍ଥିତି FFI_TYPE_SINT8:
    ସ୍ଥିତି FFI_TYPE_UINT16:
    ସ୍ଥିତି FFI_TYPE_SINT16:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT128:
    ସ୍ଥିତି FFI_TYPE_SINT128:
    ସ୍ଥିତି FFI_TYPE_POINTER:
    do_integer:
      {
	size_t size = byte_offset + type->size;

	ଯଦି (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    ଫେରନ୍ତୁ 1;
	  }
	ଅନ୍ୟଥା ଯଦି (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    ଫେରନ୍ତୁ 1;
	  }
	ଅନ୍ୟଥା ଯଦି (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    ଫେରନ୍ତୁ 2;
	  }
	ଅନ୍ୟଥା ଯଦି (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    ଫେରନ୍ତୁ 2;
	  }
	ଅନ୍ୟଥା
	  FFI_ASSERT (0);
      }
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ଯଦି (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      ଅନ୍ୟଥା
	classes[0] = X86_64_SSE_CLASS;
      ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      ଫେରନ୍ତୁ 1;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ଯଦି FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#ଅନ୍ୟଥା
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      ଫେରନ୍ତୁ 2;
#endif
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      {
	ଅଚଳ size_t UNITS_PER_WORD = 8;
        size_t ଶବ୍ଦଗୁଡ଼ିକ = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଗଣନା x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	ଯଦି (type->size > 32)
	  ଫେରନ୍ତୁ 0;

	ପାଇଁ (i = 0; i < ଶବ୍ଦଗୁଡ଼ିକ; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	ଯଦି (!ଶବ୍ଦଗୁଡ଼ିକ)
	  {
    ସ୍ଥିତି FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    ଫେରନ୍ତୁ 1;
	  }

	/* Merge the fields of structure.  */
	ପାଇଁ (ptr = type->elements; *ptr != ଖାଲି; ptr++)
	  {
	    size_t ଅଙ୍କ, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    ଅଙ୍କ = classify_argument (*ptr, subclasses, byte_offset % 8);
	    ଯଦି (ଅଙ୍କ == 0)
	      ଫେରନ୍ତୁ 0;
            pos = byte_offset / 8;
            ପାଇଁ (i = 0; i < ଅଙ୍କ && (i + pos) < ଶବ୍ଦଗୁଡ଼ିକ; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	ଯଦି (ଶବ୍ଦଗୁଡ଼ିକ > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    ଯଦି (classes[0] != X86_64_SSE_CLASS)
	      ଫେରନ୍ତୁ 0;

	    ପାଇଁ (i = 1; i < ଶବ୍ଦଗୁଡ଼ିକ; i++)
	      ଯଦି (classes[i] != X86_64_SSEUP_CLASS)
		ଫେରନ୍ତୁ 0;
	  }

	/* Final merger cleanup.  */
	ପାଇଁ (i = 0; i < ଶବ୍ଦଗୁଡ଼ିକ; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    ଯଦି (classes[i] == X86_64_MEMORY_CLASS)
	      ଫେରନ୍ତୁ 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    ଯଦି (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    ଯଦି (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		ଫେରନ୍ତୁ 0;
	      }
	  }
	ଫେରନ୍ତୁ ଶବ୍ଦଗୁଡ଼ିକ;
      }
    ସ୍ଥିତି FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      ଯଦି (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  ଫେରନ୍ତୁ 1;
	}
      ଅନ୍ୟଥା ଯଦି (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  ଫେରନ୍ତୁ 2;
	}
      ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	ବାଛ (inner->type)
	  {
	  ସ୍ଥିତି FFI_TYPE_INT:
	  ସ୍ଥିତି FFI_TYPE_UINT8:
	  ସ୍ଥିତି FFI_TYPE_SINT8:
	  ସ୍ଥିତି FFI_TYPE_UINT16:
	  ସ୍ଥିତି FFI_TYPE_SINT16:
	  ସ୍ଥିତି FFI_TYPE_UINT32:
	  ସ୍ଥିତି FFI_TYPE_SINT32:
	  ସ୍ଥିତି FFI_TYPE_UINT64:
	  ସ୍ଥିତି FFI_TYPE_SINT64:
	    ଯାଅ do_integer;

	  ସ୍ଥିତି FFI_TYPE_SINT128:
	  ସ୍ଥିତି FFI_TYPE_UINT128:
	    ଫେରନ୍ତୁ 0;

	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    ଯଦି (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		ଫେରନ୍ତୁ 2;
	      }
	    ଫେରନ୍ତୁ 1;
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    ଫେରନ୍ତୁ 2;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ଯଦି FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    ଫେରନ୍ତୁ 1;
#endif
#endif
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

ସ୍ଥିର size_t
examine_argument (ffi_type *type, ଗଣନା x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pngpr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pnsse)
{
  size_t n;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ngpr, nsse;

  n = classify_argument (type, classes, 0);
  ଯଦି (n == 0)
    ଫେରନ୍ତୁ 0;

  ngpr = nsse = 0;
  ପାଇଁ (i = 0; i < n; ++i)
    ବାଛ (classes[i])
      {
      ସ୍ଥିତି X86_64_INTEGER_CLASS:
      ସ୍ଥିତି X86_64_INTEGERSI_CLASS:
	ngpr++;
	ଭାଙ୍ଗ;
      ସ୍ଥିତି X86_64_SSE_CLASS:
      ସ୍ଥିତି X86_64_SSESF_CLASS:
      ସ୍ଥିତି X86_64_SSEDF_CLASS:
	nsse++;
	ଭାଙ୍ଗ;
      ସ୍ଥିତି X86_64_NO_CLASS:
      ସ୍ଥିତି X86_64_SSEUP_CLASS:
	ଭାଙ୍ଗ;
      ସ୍ଥିତି X86_64_X87_CLASS:
      ସ୍ଥିତି X86_64_X87UP_CLASS:
      ସ୍ଥିତି X86_64_COMPLEX_X87_CLASS:
	ଫେରନ୍ତୁ in_return != 0;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  ଫେରନ୍ତୁ n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
ବାହ୍ୟ ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  ଗଣନା x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  ଯଦି (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ଫେରନ୍ତୁ ffi_prep_cif_machdep_efi64(cif);
#endif
  ଯଦି (cif->abi != FFI_UNIX64)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  ବାଛ (rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT128:
    ସ୍ଥିତି FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = (ଆକାର(ଶୂନ୍ୟ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      ଭାଙ୍ଗ;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ଯଦି FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#ଅନ୍ୟଥା
      flags = UNIX64_RET_X87;
#endif
      ଭାଙ୍ଗ;
#endif
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      ଯଦି (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      ଅନ୍ୟଥା
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  ଯଦି (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  ଅନ୍ୟଥା ଯଦି (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  ଅନ୍ୟଥା
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      ଯଦି (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      ଅନ୍ୟଥା ଯଦି (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      ଅନ୍ୟଥା ଯଦି (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      ଅନ୍ୟଥା
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      ଯଦି (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      ଅନ୍ୟଥା ଯଦି (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      ଅନ୍ୟଥା
	ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      ବାଛ (rtype->elements[0]->type)
	{
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_size << UNIX64_SIZE_SHIFT);
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  ଭାଙ୍ଗ;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ଯଦି FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#ଅନ୍ୟଥା
	  flags = UNIX64_RET_X87_2;
#endif
	  ଭାଙ୍ଗ;
#endif
	ସ୍ଥିତି FFI_TYPE_SINT128:
	ସ୍ଥିତି FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
	}
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  ପାଇଁ (i = 0, avn = cif->nargs; i < avn; i++)
    ଯଦି (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  ପାଇଁ (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      ଯଦି (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  ଯଦି (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      ଅନ୍ୟଥା
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  ଯଦି (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  ଫେରନ୍ତୁ FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ଗଣନା x86_64_reg_class classes[MAX_CLASSES];
  ଅକ୍ଷର *ଗଦା, *argp;
  ffi_type **arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା gprcount, ssecount, ngpr, nsse, i, avn, flags;
  ସଂରଚନା register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      ଅନ୍ୟଥା
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ଗଦା = alloca (ଆକାର (ସଂରଚନା register_args) + cif->bytes + 4*8);
  reg_args = (ସଂରଚନା register_args *) ଗଦା;
  argp = ଗଦା + ଆକାର (ସଂରଚନା register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  ଯଦି (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  ପାଇଁ (i = 0; i < avn; ++i)
    {
      size_t n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      ଯଦି (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ଯଦି (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (ଶୂନ୍ୟ *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], size);

          argp += size;
        }
      ଅନ୍ୟଥା
	{
	  /* The argument is passed entirely in registers.  */
	  ଅକ୍ଷର *a = (ଅକ୍ଷର *) avalue[i];
	  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;

	  ପାଇଁ (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      ବାଛ (classes[j])
		{
		ସ୍ଥିତି X86_64_NO_CLASS:
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((ଅକ୍ଷର *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି X86_64_INTEGER_CLASS:
		ସ୍ଥିତି X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  ବାଛ (arg_types[i]->type)
		    {
		    ସ୍ଥିତି FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      ଭାଙ୍ଗ;
		    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି X86_64_SSE_CLASS:
		ସ୍ଥିତି X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, ଆକାର(UINT64));
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, ଆକାର(UINT32));
		  ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (ଗଦା, cif->bytes + ଆକାର (ସଂରଚନା register_args),
		   flags, rvalue, fn);
}

#ifndef __ILP32__
/* =====================================================================
   Precompiled argument-placement plan, used by the ffi_call_plan API.

   ffi_prep_cif_machdep classifies the signature once, but ffi_call_int then
   re-derives the same per-argument placement on every call (~650 instructions
   for a 3-argument call).  A "plan" captures that placement as a flat move
   list, built once, so the register_args + stack buffer can be filled with no
   re-classification before handing off to the unchanged ffi_call_unix64.  For
   the common case (only 64-bit GP arguments) a direct thunk loads the values
   straight into the argument registers, skipping the buffer entirely.

   A plan is built by ffi_call_plan_alloc and applied by ffi_call_plan_invoke;
   the caller owns it and reuses it across calls.

   Scalar, pointer, int128, float and double arguments are handled; any struct,
   complex, or x87 long double argument has no plan, so the caller's invoke
   falls back to ffi_call. */

ଗଣନା ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ଗଦା                               /* copy len bytes -> stack       */
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned ଅକ୍ଷର op;
} ffi_move;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
ସଂରଚନା ffi_ret2 { UINT64 i; ଦ୍ୱିଗୁଣ d; };
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_fast_call (ସଂରଚନା register_args *img,
					   ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp0 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp1 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp2 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp3 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp4 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp5 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ବାହ୍ୟ ସଂରଚନା ffi_ret2 ffi_plan_gp6 (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) FFI_HIDDEN;
ସ୍ଥିର ସଂରଚନା ffi_ret2 (*ଅଚଳ ffi_gp_thunks[7]) (ଶୂନ୍ୟ **, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
store_ret (ଶୂନ୍ୟ *rvalue, unsigned retcode, ସଂରଚନା ffi_ret2 r)
{
  ବାଛ (retcode)
    {
    ସ୍ଥିତି UNIX64_RET_VOID:   ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); ଭାଙ୍ଗ;
    ସ୍ଥିତି UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); ଭାଙ୍ଗ;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ସ୍ଥିର ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  ଗଣନା x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  ଯଦି (cif->abi != FFI_UNIX64)
    ଫେରନ୍ତୁ ଖାଲି;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  ପାଇଁ (i = 0; i < avn; i++)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = cif->arg_types[i]->type;
      ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	ଫେରନ୍ତୁ ଖାଲି;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ଯଦି (t == FFI_TYPE_LONGDOUBLE)
	ଫେରନ୍ତୁ ଖାଲି;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = ଆକାର (ffi_plan) + ଆକାର (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  ଯଦି (plan == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  ଯଦି (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  ପାଇଁ (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t size = at->size, n, rem;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      ଯଦି (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  ଯଦି (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ଗଦା;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (ଆକାର (ସଂରଚନା register_args) + argp_off);
	  m->len = (unsigned) size;
	  argp_off += size;
	  ଜାରି;
	}

      ପାଇଁ (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  ବାଛ (classes[j])
	    {
	    ସ୍ଥିତି X86_64_NO_CLASS:
	    ସ୍ଥିତି X86_64_SSEUP_CLASS:
	      ଜାରି;			/* nothing placed for this 8-byte */
	    ସ୍ଥିତି X86_64_INTEGER_CLASS:
	    ସ୍ଥିତି X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      ବାଛ (at->type)
		{
		ସ୍ଥିତି FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; ଭାଙ୍ଗ;
		ସ୍ଥିତି FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; ଭାଙ୍ଗ;
		ସ୍ଥିତି FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		  ଯଦି (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  ଅନ୍ୟଥା
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  ଭାଙ୍ଗ;
		}
	      gprcount++;
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି X86_64_SSE_CLASS:
	    ସ୍ଥିତି X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (ସଂରଚନା register_args, sse)
				      + ssecount * ଆକାର (ସଂଘ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (ସଂରଚନା register_args, sse)
				      + ssecount * ଆକାର (ସଂଘ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      ଭାଙ୍ଗ;
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      ମୁକ୍ତ (plan);		/* X87 etc. in registers: bail */
	      ଫେରନ୍ତୁ ଖାଲି;
	    }
	  plan->moves[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->bytes = cif->bytes;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->bytes == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_in_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) avn : -1;
  ଫେରନ୍ତୁ plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ସ୍ଥିର ଧାଡ଼ିଭିତରେ __attribute__ ((always_inline)) ଶୂନ୍ୟ
plan_exec (ffi_cif *cif, ffi_plan *plan, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ),
	   ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  unsigned flags = plan->flags;
  ସଂରଚନା register_args local __attribute__ ((aligned (16)));
  ଅକ୍ଷର *ଗଦା = ଖାଲି;
  ସଂରଚନା register_args *reg_args;
  unsigned k;

  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      ଅନ୍ୟଥା
	flags = UNIX64_RET_VOID;
    }

  ଯଦି (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      ସଂରଚନା ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      ଯଦି (rvalue != ଖାଲି)
	store_ret (rvalue, plan->retcode, r);
      ଫେରନ୍ତୁ;
    }

  ଯଦି (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  ଅନ୍ୟଥା
    {
      ଗଦା = alloca (ଆକାର (ସଂରଚନା register_args) + plan->bytes + 4 * 8);
      reg_args = (ସଂରଚନା register_args *) ଗଦା;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  ଯଦି (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  ପାଇଁ (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      ଅକ୍ଷର *src = (ଅକ୍ଷର *) avalue[m->src_idx] + m->src_off;
      ଅକ୍ଷର *dst = (ଅକ୍ଷର *) reg_args + m->dst_off;
      ବାଛ (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	ସ୍ଥିତି FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_MOVE_ଗଦା: memcpy (dst, src, m->len);                          ଭାଙ୍ଗ;
	}
    }
  reg_args->rax = plan->ssecount;

  ଯଦି (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      ସଂରଚନା ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      ଯଦି (rvalue != ଖାଲି)
	store_ret (rvalue, plan->retcode, r);
      ଫେରନ୍ତୁ;
    }

  ffi_call_unix64 (ଗଦା, plan->bytes + ଆକାର (ସଂରଚନା register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
ସଂରଚନା ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (ଆକାର (ସଂରଚନା ffi_call_plan));
  ଯଦି (plan == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  ଫେରନ୍ତୁ plan;
}

ଶୂନ୍ୟ
ffi_call_plan_invoke (ffi_call_plan *plan, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ),
		      ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ଯଦି (plan->fast != ଖାଲି)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  ଅନ୍ୟଥା
    ffi_call (plan->cif, fn, rvalue, avalue);
}

ଶୂନ୍ୟ
ffi_call_plan_free (ffi_call_plan *plan)
{
  ଯଦି (plan != ଖାଲି)
    {
      ମୁକ୍ତ (plan->fast);
      ମୁକ୍ତ (plan);
    }
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  ଯଦି (plan == ଖାଲି)
    ଫେରନ୍ତୁ 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  ଫେରନ୍ତୁ ଆକାର (ସଂରଚନା ffi_call_plan)
	 + (plan->fast != ଖାଲି ? plan->fast->alloc_bytes : 0);
}

ବାହ୍ୟ ଶୂନ୍ୟ
ffi_call_efi64(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue);
#endif

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs = cif->nargs;
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      ଯଦି (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          ଅକ୍ଷର *argcopy = alloca (size);
          ଯଦି (avalue_copy == ଖାଲି)
            {
              avalue_copy = alloca (nargs * ଆକାର (ଶୂନ୍ୟ *));
              memcpy (avalue_copy, avalue, nargs * ଆକାର (ଶୂନ୍ୟ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  ଯଦି (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      ଫେରନ୍ତୁ;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
ବାହ୍ୟ ଶୂନ୍ୟ
ffi_call_go_efi64(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
		  ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure);
#endif

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
#ifndef __ILP32__
  ଯଦି (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      ଫେରନ୍ତୁ;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_unix64(ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_unix64_sse(ଶୂନ୍ୟ) FFI_HIDDEN;
#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_unix64_alt(ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_unix64_sse_alt(ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

#ifndef __ILP32__
ବାହ୍ୟ ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			   ଶୂନ୍ୟ *user_data,
			   ଶୂନ୍ୟ *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ);
  ଅକ୍ଷର *tramp = closure->tramp;

#ifndef __ILP32__
  ଯଦି (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ଫେରନ୍ତୁ ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  ଯଦି (cif->abi != FFI_UNIX64)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  ଯଦି (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  ଅନ୍ୟଥା
    dest = ffi_closure_unix64;

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଯଦି (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      ଅନ୍ୟଥା
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      ଯାଅ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, ଆକାର(trampoline));
  *(UINT64 *)(tramp + ଆକାର (trampoline)) = (uintptr_t)dest;

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			 ଶୂନ୍ୟ *user_data,
			 ଶୂନ୍ୟ *rvalue,
			 ସଂରଚନା register_args *reg_args,
			 ଅକ୍ଷର *argp)
{
  ଶୂନ୍ୟ **avalue;
  ffi_type **arg_types;
  long i, avn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା gprcount, ssecount, ngpr, nsse;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * ଆକାର(ଶୂନ୍ୟ *));
  gprcount = ssecount = 0;

  ଯଦି (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      ଶୂନ୍ୟ *r = (ଶୂନ୍ୟ *)(uintptr_t)reg_args->gpr[gprcount++];
      *(ଶୂନ୍ୟ **)rvalue = r;
      rvalue = r;
      flags = (ଆକାର(ଶୂନ୍ୟ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  ପାଇଁ (i = 0; i < avn; ++i)
    {
      ଗଣନା x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      ଯଦି (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ଯଦି (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (ଶୂନ୍ୟ *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      ଅନ୍ୟଥା ଯଦି (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  ଯଦି (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      ଅନ୍ୟଥା
	{
	  ଅକ୍ଷର *a = alloca (n * 8);
	  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;

	  avalue[i] = a;
	  ପାଇଁ (j = 0; j < n; j++, a += 8)
	    {
	      ଯଦି (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (ଅକ୍ଷର *) &reg_args->sse[ssecount - 1] + 8, 8);
	      ଅନ୍ୟଥା ଯଦି (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      ଅନ୍ୟଥା
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  ଫେରନ୍ତୁ flags;
}

#ifdef FFI_GO_CLOSURES

ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_unix64(ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_unix64_sse(ଶୂନ୍ୟ) FFI_HIDDEN;

#ifndef __ILP32__
ବାହ୍ୟ ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*))
{
#ifndef __ILP32__
  ଯଦି (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ଫେରନ୍ତୁ ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  ଯଦି (cif->abi != FFI_UNIX64)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stddef.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <tramp.h>
#অন্তর্ভাব "internal64.h"

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#যদি defined(__INTEL_COMPILER)
#অন্তর্ভাব "xmmintrin.h"
#define UINT128 __m128
#অন্যথা
#যদি defined(__SUNPRO_C)
#অন্তর্ভাব <sunmedia_types.h>
#define UINT128 __m128i
#অন্যথা
#define UINT128 __int128_t
#endif
#endif

সংঘ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

কাঠামো register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  সংঘ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

বাহ্যিক শূন্য ffi_call_unix64 (শূন্য *args, unsigned long bytes, unsigned flags,
			     শূন্য *raddr, শূন্য (*fnaddr)(শূন্য)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
গণনা x86_64_reg_class
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
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#অন্যথা
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

স্থির গণনা x86_64_reg_class
merge_classes (গণনা x86_64_reg_class class1, গণনা x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  যদি (class1 == class2)
    ফেরত class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  যদি (class1 == X86_64_NO_CLASS)
    ফেরত class2;
  যদি (class2 == X86_64_NO_CLASS)
    ফেরত class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  যদি (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    ফেরত X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  যদি ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    ফেরত X86_64_INTEGERSI_CLASS;
  যদি (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    ফেরত X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  যদি (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    ফেরত X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  ফেরত X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
স্থির size_t
classify_argument (ffi_type *type, গণনা x86_64_reg_class classes[],
		   size_t byte_offset)
{
  নির্বাচন (type->type)
    {
    ক্ষেত্র FFI_TYPE_UINT8:
    ক্ষেত্র FFI_TYPE_SINT8:
    ক্ষেত্র FFI_TYPE_UINT16:
    ক্ষেত্র FFI_TYPE_SINT16:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT128:
    ক্ষেত্র FFI_TYPE_SINT128:
    ক্ষেত্র FFI_TYPE_POINTER:
    do_integer:
      {
	size_t আকার_2 = byte_offset + type->আকার_2;

	যদি (আকার_2 <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    ফেরত 1;
	  }
	অন্যথা যদি (আকার_2 <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    ফেরত 1;
	  }
	অন্যথা যদি (আকার_2 <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    ফেরত 2;
	  }
	অন্যথা যদি (আকার_2 <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    ফেরত 2;
	  }
	অন্যথা
	  FFI_ASSERT (0);
      }
    ক্ষেত্র FFI_TYPE_FLOAT:
      যদি (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      অন্যথা
	classes[0] = X86_64_SSE_CLASS;
      ফেরত 1;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      ফেরত 1;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#যদি FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#অন্যথা
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      ফেরত 2;
#endif
    ক্ষেত্র FFI_TYPE_STRUCT:
      {
	ধ্রুবক size_t UNITS_PER_WORD = 8;
        size_t শব্দগুলি = (type->আকার_2 + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned পূর্ণসংখ্যা i;
	গণনা x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	যদি (type->আকার_2 > 32)
	  ফেরত 0;

	জন্য (i = 0; i < শব্দগুলি; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	যদি (!শব্দগুলি)
	  {
    ক্ষেত্র FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    ফেরত 1;
	  }

	/* Merge the fields of structure.  */
	জন্য (ptr = type->elements; *ptr != ফাঁকা; ptr++)
	  {
	    size_t অঙ্ক, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    অঙ্ক = classify_argument (*ptr, subclasses, byte_offset % 8);
	    যদি (অঙ্ক == 0)
	      ফেরত 0;
            pos = byte_offset / 8;
            জন্য (i = 0; i < অঙ্ক && (i + pos) < শব্দগুলি; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->আকার_2;
	  }

	যদি (শব্দগুলি > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    যদি (classes[0] != X86_64_SSE_CLASS)
	      ফেরত 0;

	    জন্য (i = 1; i < শব্দগুলি; i++)
	      যদি (classes[i] != X86_64_SSEUP_CLASS)
		ফেরত 0;
	  }

	/* Final merger cleanup.  */
	জন্য (i = 0; i < শব্দগুলি; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    যদি (classes[i] == X86_64_MEMORY_CLASS)
	      ফেরত 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    যদি (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    যদি (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		ফেরত 0;
	      }
	  }
	ফেরত শব্দগুলি;
      }
    ক্ষেত্র FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      যদি (type->আকার_2 == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  ফেরত 1;
	}
      অন্যথা যদি (type->আকার_2 == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  ফেরত 2;
	}
      ফেরত 0;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	নির্বাচন (inner->type)
	  {
	  ক্ষেত্র FFI_TYPE_INT:
	  ক্ষেত্র FFI_TYPE_UINT8:
	  ক্ষেত্র FFI_TYPE_SINT8:
	  ক্ষেত্র FFI_TYPE_UINT16:
	  ক্ষেত্র FFI_TYPE_SINT16:
	  ক্ষেত্র FFI_TYPE_UINT32:
	  ক্ষেত্র FFI_TYPE_SINT32:
	  ক্ষেত্র FFI_TYPE_UINT64:
	  ক্ষেত্র FFI_TYPE_SINT64:
	    যাও do_integer;

	  ক্ষেত্র FFI_TYPE_SINT128:
	  ক্ষেত্র FFI_TYPE_UINT128:
	    ফেরত 0;

	  ক্ষেত্র FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    যদি (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		ফেরত 2;
	      }
	    ফেরত 1;
	  ক্ষেত্র FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    ফেরত 2;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#যদি FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    ফেরত 0;
#অন্যথা
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    ফেরত 1;
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

স্থির size_t
examine_argument (ffi_type *type, গণনা x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, পূর্ণসংখ্যা *pngpr, পূর্ণসংখ্যা *pnsse)
{
  size_t n;
  unsigned পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা ngpr, nsse;

  n = classify_argument (type, classes, 0);
  যদি (n == 0)
    ফেরত 0;

  ngpr = nsse = 0;
  জন্য (i = 0; i < n; ++i)
    নির্বাচন (classes[i])
      {
      ক্ষেত্র X86_64_INTEGER_CLASS:
      ক্ষেত্র X86_64_INTEGERSI_CLASS:
	ngpr++;
	বিরতি;
      ক্ষেত্র X86_64_SSE_CLASS:
      ক্ষেত্র X86_64_SSESF_CLASS:
      ক্ষেত্র X86_64_SSEDF_CLASS:
	nsse++;
	বিরতি;
      ক্ষেত্র X86_64_NO_CLASS:
      ক্ষেত্র X86_64_SSEUP_CLASS:
	বিরতি;
      ক্ষেত্র X86_64_X87_CLASS:
      ক্ষেত্র X86_64_X87UP_CLASS:
      ক্ষেত্র X86_64_COMPLEX_X87_CLASS:
	ফেরত in_return != 0;
      পূর্বনির্ধারিত:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  ফেরত n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
বাহ্যিক ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  পূর্ণসংখ্যা gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  গণনা x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_আকার;
  ffi_type *rtype;

#ifndef __ILP32__
  যদি (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ফেরত ffi_prep_cif_machdep_efi64(cif);
#endif
  যদি (cif->abi != FFI_UNIX64)
    ফেরত FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_আকার = rtype->আকার_2;
  নির্বাচন (rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      বিরতি;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT128:
    ক্ষেত্র FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_আকার_SHIFT);
      বিরতি;
    ক্ষেত্র FFI_TYPE_POINTER:
      flags = (আকার(শূন্য *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      বিরতি;
    ক্ষেত্র FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      বিরতি;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      বিরতি;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#যদি FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#অন্যথা
      flags = UNIX64_RET_X87;
#endif
      বিরতি;
#endif
    ক্ষেত্র FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      যদি (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      অন্যথা
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  যদি (rtype_আকার == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  অন্যথা যদি (rtype_আকার == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  অন্যথা
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      যদি (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      অন্যথা যদি (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      অন্যথা যদি (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      অন্যথা
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_আকার << UNIX64_আকার_SHIFT;
	    }
	}
      বিরতি;
    ক্ষেত্র FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      যদি (rtype_আকার == 8)
	flags = UNIX64_RET_XMM64;
      অন্যথা যদি (rtype_আকার == 16)
	flags = UNIX64_RET_XMM128;
      অন্যথা
	ফেরত FFI_BAD_TYPEDEF;
      বিরতি;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      নির্বাচন (rtype->elements[0]->type)
	{
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_আকার << UNIX64_আকার_SHIFT);
	  বিরতি;
	ক্ষেত্র FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_আকার_SHIFT);
	  বিরতি;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#যদি FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#অন্যথা
	  flags = UNIX64_RET_X87_2;
#endif
	  বিরতি;
#endif
	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  বিরতি;
	পূর্বনির্ধারিত:
	  ফেরত FFI_BAD_TYPEDEF;
	}
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  জন্য (i = 0, avn = cif->nargs; i < avn; i++)
    যদি (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->আকার_2 > 16)
      ফেরত FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  জন্য (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      যদি (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  যদি (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->আকার_2;
	}
      অন্যথা
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  যদি (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  ফেরত FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  গণনা x86_64_reg_class classes[MAX_CLASSES];
  অক্ষর *স্তূপ, *argp;
  ffi_type **arg_types;
  পূর্ণসংখ্যা gprcount, ssecount, ngpr, nsse, i, avn, flags;
  কাঠামো register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  যদি (rvalue == ফাঁকা)
    {
      যদি (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->আকার_2);
      অন্যথা
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  স্তূপ = alloca (আকার (কাঠামো register_args) + cif->bytes + 4*8);
  reg_args = (কাঠামো register_args *) স্তূপ;
  argp = স্তূপ + আকার (কাঠামো register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  যদি (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  জন্য (i = 0; i < avn; ++i)
    {
      size_t n, আকার_2 = arg_types[i]->আকার_2;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      যদি (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  যদি (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (শূন্য *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], আকার_2);

          argp += আকার_2;
        }
      অন্যথা
	{
	  /* The argument is passed entirely in registers.  */
	  অক্ষর *a = (অক্ষর *) avalue[i];
	  unsigned পূর্ণসংখ্যা j;

	  জন্য (j = 0; j < n; j++, a += 8, আকার_2 -= 8)
	    {
	      নির্বাচন (classes[j])
		{
		ক্ষেত্র X86_64_NO_CLASS:
		  বিরতি;
		ক্ষেত্র X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((অক্ষর *) &reg_args->sse[ssecount - 1] + 8, a,
			  আকার_2 < 8 ? আকার_2 : 8);
		  বিরতি;
		ক্ষেত্র X86_64_INTEGER_CLASS:
		ক্ষেত্র X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  নির্বাচন (arg_types[i]->type)
		    {
		    ক্ষেত্র FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      বিরতি;
		    ক্ষেত্র FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      বিরতি;
		    ক্ষেত্র FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      বিরতি;
		    পূর্বনির্ধারিত:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, আকার_2 <= 8 ? আকার_2 : 8);
		    }
		  gprcount++;
		  বিরতি;
		ক্ষেত্র X86_64_SSE_CLASS:
		ক্ষেত্র X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, আকার(UINT64));
		  বিরতি;
		ক্ষেত্র X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, আকার(UINT32));
		  বিরতি;
		পূর্বনির্ধারিত:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (স্তূপ, cif->bytes + আকার (কাঠামো register_args),
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

গণনা ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_স্তূপ                               /* copy len bytes -> stack       */
};

প্রকারলেখ কাঠামো
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned অক্ষর op;
} ffi_move;

প্রকারলেখ কাঠামো
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  পূর্ণসংখ্যা      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
কাঠামো ffi_ret2 { UINT64 i; দ্বিগুণ d; };
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_fast_call (কাঠামো register_args *img,
					   শূন্য (*fn) (শূন্য)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp0 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp1 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp2 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp3 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp4 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp5 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
বাহ্যিক কাঠামো ffi_ret2 ffi_plan_gp6 (শূন্য **, শূন্য (*)(শূন্য)) FFI_HIDDEN;
স্থির কাঠামো ffi_ret2 (*ধ্রুবক ffi_gp_thunks[7]) (শূন্য **, শূন্য (*)(শূন্য)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
স্থির অন্তঃরেখ শূন্য
store_ret (শূন্য *rvalue, unsigned retcode, কাঠামো ffi_ret2 r)
{
  নির্বাচন (retcode)
    {
    ক্ষেত্র UNIX64_RET_VOID:   বিরতি;
    ক্ষেত্র UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; বিরতি;
    ক্ষেত্র UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); বিরতি;
    ক্ষেত্র UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); বিরতি;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
স্থির ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  গণনা x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  পূর্ণসংখ্যা all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  যদি (cif->abi != FFI_UNIX64)
    ফেরত ফাঁকা;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  জন্য (i = 0; i < avn; i++)
    {
      পূর্ণসংখ্যা t = cif->arg_types[i]->type;
      যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	ফেরত ফাঁকা;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      যদি (t == FFI_TYPE_LONGDOUBLE)
	ফেরত ফাঁকা;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = আকার (ffi_plan) + আকার (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  যদি (plan == ফাঁকা)
    ফেরত ফাঁকা;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  যদি (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  জন্য (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t আকার_2 = at->আকার_2, n, rem;
      পূর্ণসংখ্যা ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      যদি (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  যদি (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_স্তূপ;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (আকার (কাঠামো register_args) + argp_off);
	  m->len = (unsigned) আকার_2;
	  argp_off += আকার_2;
	  চালু;
	}

      জন্য (j = 0, rem = আকার_2; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  নির্বাচন (classes[j])
	    {
	    ক্ষেত্র X86_64_NO_CLASS:
	    ক্ষেত্র X86_64_SSEUP_CLASS:
	      চালু;			/* nothing placed for this 8-byte */
	    ক্ষেত্র X86_64_INTEGER_CLASS:
	    ক্ষেত্র X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      নির্বাচন (at->type)
		{
		ক্ষেত্র FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; বিরতি;
		ক্ষেত্র FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; বিরতি;
		ক্ষেত্র FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; বিরতি;
		পূর্বনির্ধারিত:
		  যদি (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  অন্যথা
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  বিরতি;
		}
	      gprcount++;
	      বিরতি;
	    ক্ষেত্র X86_64_SSE_CLASS:
	    ক্ষেত্র X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (কাঠামো register_args, sse)
				      + ssecount * আকার (সংঘ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      বিরতি;
	    ক্ষেত্র X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (কাঠামো register_args, sse)
				      + ssecount * আকার (সংঘ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      বিরতি;
	    পূর্বনির্ধারিত:
	      মুক্ত (plan);		/* X87 etc. in registers: bail */
	      ফেরত ফাঁকা;
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
    ? (পূর্ণসংখ্যা) avn : -1;
  ফেরত plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
স্থির অন্তঃরেখ __attribute__ ((always_inline)) শূন্য
plan_exec (ffi_cif *cif, ffi_plan *plan, শূন্য (*fn) (শূন্য),
	   শূন্য *rvalue, শূন্য **avalue)
{
  unsigned flags = plan->flags;
  কাঠামো register_args local __attribute__ ((aligned (16)));
  অক্ষর *স্তূপ = ফাঁকা;
  কাঠামো register_args *reg_args;
  unsigned k;

  যদি (rvalue == ফাঁকা)
    {
      যদি (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->আকার_2);
      অন্যথা
	flags = UNIX64_RET_VOID;
    }

  যদি (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      কাঠামো ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      যদি (rvalue != ফাঁকা)
	store_ret (rvalue, plan->retcode, r);
      ফেরত;
    }

  যদি (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  অন্যথা
    {
      স্তূপ = alloca (আকার (কাঠামো register_args) + plan->bytes + 4 * 8);
      reg_args = (কাঠামো register_args *) স্তূপ;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  যদি (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  জন্য (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      অক্ষর *src = (অক্ষর *) avalue[m->src_idx] + m->src_off;
      অক্ষর *dst = (অক্ষর *) reg_args + m->dst_off;
      নির্বাচন (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	ক্ষেত্র FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; বিরতি;
	ক্ষেত্র FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; বিরতি;
	ক্ষেত্র FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; বিরতি;
	ক্ষেত্র FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   বিরতি;
	ক্ষেত্র FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     বিরতি;
	ক্ষেত্র FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   বিরতি;
	ক্ষেত্র FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   বিরতি;
	ক্ষেত্র FFI_MOVE_স্তূপ: memcpy (dst, src, m->len);                          বিরতি;
	}
    }
  reg_args->rax = plan->ssecount;

  যদি (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      কাঠামো ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      যদি (rvalue != ফাঁকা)
	store_ret (rvalue, plan->retcode, r);
      ফেরত;
    }

  ffi_call_unix64 (স্তূপ, plan->bytes + আকার (কাঠামো register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
কাঠামো ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (আকার (কাঠামো ffi_call_plan));
  যদি (plan == ফাঁকা)
    ফেরত ফাঁকা;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  ফেরত plan;
}

শূন্য
ffi_call_plan_invoke (ffi_call_plan *plan, শূন্য (*fn) (শূন্য),
		      শূন্য *rvalue, শূন্য **avalue)
{
  যদি (plan->fast != ফাঁকা)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  অন্যথা
    ffi_call (plan->cif, fn, rvalue, avalue);
}

শূন্য
ffi_call_plan_free (ffi_call_plan *plan)
{
  যদি (plan != ফাঁকা)
    {
      মুক্ত (plan->fast);
      মুক্ত (plan);
    }
}

size_t
ffi_call_plan_আকার (ffi_call_plan *plan)
{
  যদি (plan == ফাঁকা)
    ফেরত 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  ফেরত আকার (কাঠামো ffi_call_plan)
	 + (plan->fast != ফাঁকা ? plan->fast->alloc_bytes : 0);
}

বাহ্যিক শূন্য
ffi_call_efi64(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue);
#endif

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;
  পূর্ণসংখ্যা i, nargs = cif->nargs;
  ধ্রুবক পূর্ণসংখ্যা max_reg_struct_আকার = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      যদি (at->type == FFI_TYPE_STRUCT && আকার_2 > max_reg_struct_আকার)
        {
          অক্ষর *argcopy = alloca (আকার_2);
          যদি (avalue_copy == ফাঁকা)
            {
              avalue_copy = alloca (nargs * আকার (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], আকার_2);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  যদি (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      ফেরত;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, ফাঁকা);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
বাহ্যিক শূন্য
ffi_call_go_efi64(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
		  শূন্য **avalue, শূন্য *closure);
#endif

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
#ifndef __ILP32__
  যদি (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      ফেরত;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

বাহ্যিক শূন্য ffi_closure_unix64(শূন্য) FFI_HIDDEN;
বাহ্যিক শূন্য ffi_closure_unix64_sse(শূন্য) FFI_HIDDEN;
#যদি defined(FFI_EXEC_STATIC_TRAMP)
বাহ্যিক শূন্য ffi_closure_unix64_alt(শূন্য) FFI_HIDDEN;
বাহ্যিক শূন্য ffi_closure_unix64_sse_alt(শূন্য) FFI_HIDDEN;
#endif

#ifndef __ILP32__
বাহ্যিক ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			   শূন্য *user_তথ্য,
			   শূন্য *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  স্থির ধ্রুবক unsigned অক্ষর trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  শূন্য (*dest)(শূন্য);
  অক্ষর *tramp = closure->tramp;

#ifndef __ILP32__
  যদি (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ফেরত ffi_prep_closure_loc_efi64(closure, cif, fun, user_তথ্য, codeloc);
#endif
  যদি (cif->abi != FFI_UNIX64)
    ফেরত FFI_BAD_ABI;

  যদি (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  অন্যথা
    dest = ffi_closure_unix64;

#যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      যদি (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      অন্যথা
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      যাও out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, আকার(trampoline));
  *(UINT64 *)(tramp + আকার (trampoline)) = (uintptr_t)dest;

#যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

পূর্ণসংখ্যা FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			 শূন্য *user_তথ্য,
			 শূন্য *rvalue,
			 কাঠামো register_args *reg_args,
			 অক্ষর *argp)
{
  শূন্য **avalue;
  ffi_type **arg_types;
  long i, avn;
  পূর্ণসংখ্যা gprcount, ssecount, ngpr, nsse;
  পূর্ণসংখ্যা flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * আকার(শূন্য *));
  gprcount = ssecount = 0;

  যদি (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      শূন্য *r = (শূন্য *)(uintptr_t)reg_args->gpr[gprcount++];
      *(শূন্য **)rvalue = r;
      rvalue = r;
      flags = (আকার(শূন্য *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  জন্য (i = 0; i < avn; ++i)
    {
      গণনা x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      যদি (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  যদি (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (শূন্য *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->আকার_2;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      অন্যথা যদি (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  যদি (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  অন্যথা
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      অন্যথা
	{
	  অক্ষর *a = alloca (n * 8);
	  unsigned পূর্ণসংখ্যা j;

	  avalue[i] = a;
	  জন্য (j = 0; j < n; j++, a += 8)
	    {
	      যদি (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (অক্ষর *) &reg_args->sse[ssecount - 1] + 8, 8);
	      অন্যথা যদি (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      অন্যথা
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_তথ্য);

  /* Tell assembly how to perform return type promotions.  */
  ফেরত flags;
}

#ifdef FFI_GO_CLOSURES

বাহ্যিক শূন্য ffi_go_closure_unix64(শূন্য) FFI_HIDDEN;
বাহ্যিক শূন্য ffi_go_closure_unix64_sse(শূন্য) FFI_HIDDEN;

#ifndef __ILP32__
বাহ্যিক ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
#ifndef __ILP32__
  যদি (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ফেরত ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  যদি (cif->abi != FFI_UNIX64)
    ফেরত FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_আকার, size_t *map_আকার)
{
  বাহ্যিক শূন্য *trampoline_code_table;

  *map_আকার = UNIX64_TRAMP_MAP_আকার;
  *tramp_আকার = UNIX64_TRAMP_আকার;
  ফেরত &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

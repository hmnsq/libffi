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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stddef.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <tramp.h>
#अन्तर्भाव "internal64.h"

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#जइ defined(__INTEL_COMPILER)
#अन्तर्भाव "xmmintrin.h"
#define UINT128 __m128
#अण्णहा
#जइ defined(__SUNPRO_C)
#अन्तर्भाव <sunmedia_types.h>
#define UINT128 __m128i
#अण्णहा
#define UINT128 __int128_t
#endif
#endif

सङ्घ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

संरचना register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  सङ्घ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

बाह्य शून्य ffi_call_unix64 (शून्य *args, unsigned long bytes, unsigned flags,
			     शून्य *raddr, शून्य (*fnaddr)(शून्य)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
गणना x86_64_reg_class
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
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#अण्णहा
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

स्थिर गणना x86_64_reg_class
merge_classes (गणना x86_64_reg_class class1, गणना x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  जइ (class1 == class2)
    प्रत्यागम class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  जइ (class1 == X86_64_NO_CLASS)
    प्रत्यागम class2;
  जइ (class2 == X86_64_NO_CLASS)
    प्रत्यागम class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  जइ (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    प्रत्यागम X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  जइ ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    प्रत्यागम X86_64_INTEGERSI_CLASS;
  जइ (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    प्रत्यागम X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  जइ (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    प्रत्यागम X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  प्रत्यागम X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
स्थिर size_t
classify_argument (ffi_type *type, गणना x86_64_reg_class classes[],
		   size_t byte_offset)
{
  विकल्प (type->type)
    {
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT128:
    पक्ष FFI_TYPE_SINT128:
    पक्ष FFI_TYPE_POINTER:
    do_integer:
      {
	size_t size = byte_offset + type->size;

	जइ (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    प्रत्यागम 1;
	  }
	अण्णहा जइ (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    प्रत्यागम 1;
	  }
	अण्णहा जइ (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    प्रत्यागम 2;
	  }
	अण्णहा जइ (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    प्रत्यागम 2;
	  }
	अण्णहा
	  FFI_ASSERT (0);
      }
    पक्ष FFI_TYPE_FLOAT:
      जइ (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      अण्णहा
	classes[0] = X86_64_SSE_CLASS;
      प्रत्यागम 1;
    पक्ष FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      प्रत्यागम 1;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
#जइ FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#अण्णहा
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      प्रत्यागम 2;
#endif
    पक्ष FFI_TYPE_STRUCT:
      {
	ध्रुव size_t UNITS_PER_WORD = 8;
        size_t शब्दाः = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned पूर्णाङ्क i;
	गणना x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	जइ (type->size > 32)
	  प्रत्यागम 0;

	प्रति (i = 0; i < शब्दाः; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	जइ (!शब्दाः)
	  {
    पक्ष FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    प्रत्यागम 1;
	  }

	/* Merge the fields of structure.  */
	प्रति (ptr = type->elements; *ptr != रिक्त; ptr++)
	  {
	    size_t अङ्कः, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    अङ्कः = classify_argument (*ptr, subclasses, byte_offset % 8);
	    जइ (अङ्कः == 0)
	      प्रत्यागम 0;
            pos = byte_offset / 8;
            प्रति (i = 0; i < अङ्कः && (i + pos) < शब्दाः; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	जइ (शब्दाः > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    जइ (classes[0] != X86_64_SSE_CLASS)
	      प्रत्यागम 0;

	    प्रति (i = 1; i < शब्दाः; i++)
	      जइ (classes[i] != X86_64_SSEUP_CLASS)
		प्रत्यागम 0;
	  }

	/* Final merger cleanup.  */
	प्रति (i = 0; i < शब्दाः; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    जइ (classes[i] == X86_64_MEMORY_CLASS)
	      प्रत्यागम 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    जइ (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    जइ (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		प्रत्यागम 0;
	      }
	  }
	प्रत्यागम शब्दाः;
      }
    पक्ष FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      जइ (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  प्रत्यागम 1;
	}
      अण्णहा जइ (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  प्रत्यागम 2;
	}
      प्रत्यागम 0;
    पक्ष FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	विकल्प (inner->type)
	  {
	  पक्ष FFI_TYPE_INT:
	  पक्ष FFI_TYPE_UINT8:
	  पक्ष FFI_TYPE_SINT8:
	  पक्ष FFI_TYPE_UINT16:
	  पक्ष FFI_TYPE_SINT16:
	  पक्ष FFI_TYPE_UINT32:
	  पक्ष FFI_TYPE_SINT32:
	  पक्ष FFI_TYPE_UINT64:
	  पक्ष FFI_TYPE_SINT64:
	    गच्छ do_integer;

	  पक्ष FFI_TYPE_SINT128:
	  पक्ष FFI_TYPE_UINT128:
	    प्रत्यागम 0;

	  पक्ष FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    जइ (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		प्रत्यागम 2;
	      }
	    प्रत्यागम 1;
	  पक्ष FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    प्रत्यागम 2;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  पक्ष FFI_TYPE_LONGDOUBLE:
#जइ FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    प्रत्यागम 0;
#अण्णहा
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    प्रत्यागम 1;
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

स्थिर size_t
examine_argument (ffi_type *type, गणना x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, पूर्णाङ्क *pngpr, पूर्णाङ्क *pnsse)
{
  size_t n;
  unsigned पूर्णाङ्क i;
  पूर्णाङ्क ngpr, nsse;

  n = classify_argument (type, classes, 0);
  जइ (n == 0)
    प्रत्यागम 0;

  ngpr = nsse = 0;
  प्रति (i = 0; i < n; ++i)
    विकल्प (classes[i])
      {
      पक्ष X86_64_INTEGER_CLASS:
      पक्ष X86_64_INTEGERSI_CLASS:
	ngpr++;
	भङ्ग;
      पक्ष X86_64_SSE_CLASS:
      पक्ष X86_64_SSESF_CLASS:
      पक्ष X86_64_SSEDF_CLASS:
	nsse++;
	भङ्ग;
      पक्ष X86_64_NO_CLASS:
      पक्ष X86_64_SSEUP_CLASS:
	भङ्ग;
      पक्ष X86_64_X87_CLASS:
      पक्ष X86_64_X87UP_CLASS:
      पक्ष X86_64_COMPLEX_X87_CLASS:
	प्रत्यागम in_return != 0;
      स्वभाव:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  प्रत्यागम n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
बाह्य ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णाङ्क gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  गणना x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  जइ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    प्रत्यागम ffi_prep_cif_machdep_efi64(cif);
#endif
  जइ (cif->abi != FFI_UNIX64)
    प्रत्यागम FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  विकल्प (rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      भङ्ग;
    पक्ष FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      भङ्ग;
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      भङ्ग;
    पक्ष FFI_TYPE_UINT128:
    पक्ष FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      भङ्ग;
    पक्ष FFI_TYPE_POINTER:
      flags = (परिमाण(शून्य *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      भङ्ग;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
#जइ FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#अण्णहा
      flags = UNIX64_RET_X87;
#endif
      भङ्ग;
#endif
    पक्ष FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      जइ (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      अण्णहा
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  जइ (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  अण्णहा जइ (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  अण्णहा
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      जइ (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      अण्णहा जइ (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      अण्णहा जइ (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      अण्णहा
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      भङ्ग;
    पक्ष FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      जइ (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      अण्णहा जइ (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      अण्णहा
	प्रत्यागम FFI_BAD_TYPEDEF;
      भङ्ग;
    पक्ष FFI_TYPE_COMPLEX:
      विकल्प (rtype->elements[0]->type)
	{
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_size << UNIX64_SIZE_SHIFT);
	  भङ्ग;
	पक्ष FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  भङ्ग;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
#जइ FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#अण्णहा
	  flags = UNIX64_RET_X87_2;
#endif
	  भङ्ग;
#endif
	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  भङ्ग;
	स्वभाव:
	  प्रत्यागम FFI_BAD_TYPEDEF;
	}
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  प्रति (i = 0, avn = cif->nargs; i < avn; i++)
    जइ (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      प्रत्यागम FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  प्रति (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      जइ (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  जइ (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      अण्णहा
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  जइ (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  प्रत्यागम FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  गणना x86_64_reg_class classes[MAX_CLASSES];
  वर्ण *राशिः, *argp;
  ffi_type **arg_types;
  पूर्णाङ्क gprcount, ssecount, ngpr, nsse, i, avn, flags;
  संरचना register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  जइ (rvalue == रिक्त)
    {
      जइ (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      अण्णहा
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  राशिः = alloca (परिमाण (संरचना register_args) + cif->bytes + 4*8);
  reg_args = (संरचना register_args *) राशिः;
  argp = राशिः + परिमाण (संरचना register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  जइ (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  प्रति (i = 0; i < avn; ++i)
    {
      size_t n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      जइ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  जइ (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (शून्य *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], size);

          argp += size;
        }
      अण्णहा
	{
	  /* The argument is passed entirely in registers.  */
	  वर्ण *a = (वर्ण *) avalue[i];
	  unsigned पूर्णाङ्क j;

	  प्रति (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      विकल्प (classes[j])
		{
		पक्ष X86_64_NO_CLASS:
		  भङ्ग;
		पक्ष X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((वर्ण *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  भङ्ग;
		पक्ष X86_64_INTEGER_CLASS:
		पक्ष X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  विकल्प (arg_types[i]->type)
		    {
		    पक्ष FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      भङ्ग;
		    पक्ष FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      भङ्ग;
		    पक्ष FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      भङ्ग;
		    स्वभाव:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  भङ्ग;
		पक्ष X86_64_SSE_CLASS:
		पक्ष X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, परिमाण(UINT64));
		  भङ्ग;
		पक्ष X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, परिमाण(UINT32));
		  भङ्ग;
		स्वभाव:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (राशिः, cif->bytes + परिमाण (संरचना register_args),
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

गणना ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_राशिः                               /* copy len bytes -> stack       */
};

प्रकारनिर्देश संरचना
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned वर्ण op;
} ffi_move;

प्रकारनिर्देश संरचना
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  पूर्णाङ्क      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
संरचना ffi_ret2 { UINT64 i; द्विगुण d; };
बाह्य संरचना ffi_ret2 ffi_plan_fast_call (संरचना register_args *img,
					   शून्य (*fn) (शून्य)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
बाह्य संरचना ffi_ret2 ffi_plan_gp0 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp1 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp2 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp3 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp4 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp5 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
बाह्य संरचना ffi_ret2 ffi_plan_gp6 (शून्य **, शून्य (*)(शून्य)) FFI_HIDDEN;
स्थिर संरचना ffi_ret2 (*ध्रुव ffi_gp_thunks[7]) (शून्य **, शून्य (*)(शून्य)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
स्थिर अन्तःस्थ शून्य
store_ret (शून्य *rvalue, unsigned retcode, संरचना ffi_ret2 r)
{
  विकल्प (retcode)
    {
    पक्ष UNIX64_RET_VOID:   भङ्ग;
    पक्ष UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; भङ्ग;
    पक्ष UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; भङ्ग;
    पक्ष UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; भङ्ग;
    पक्ष UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; भङ्ग;
    पक्ष UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; भङ्ग;
    पक्ष UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; भङ्ग;
    पक्ष UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; भङ्ग;
    पक्ष UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); भङ्ग;
    पक्ष UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); भङ्ग;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
स्थिर ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  गणना x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  पूर्णाङ्क all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  जइ (cif->abi != FFI_UNIX64)
    प्रत्यागम रिक्त;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  प्रति (i = 0; i < avn; i++)
    {
      पूर्णाङ्क t = cif->arg_types[i]->type;
      जइ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	प्रत्यागम रिक्त;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      जइ (t == FFI_TYPE_LONGDOUBLE)
	प्रत्यागम रिक्त;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = परिमाण (ffi_plan) + परिमाण (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  जइ (plan == रिक्त)
    प्रत्यागम रिक्त;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  जइ (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  प्रति (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t size = at->size, n, rem;
      पूर्णाङ्क ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      जइ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  जइ (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_राशिः;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (परिमाण (संरचना register_args) + argp_off);
	  m->len = (unsigned) size;
	  argp_off += size;
	  अनुवर्त;
	}

      प्रति (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  विकल्प (classes[j])
	    {
	    पक्ष X86_64_NO_CLASS:
	    पक्ष X86_64_SSEUP_CLASS:
	      अनुवर्त;			/* nothing placed for this 8-byte */
	    पक्ष X86_64_INTEGER_CLASS:
	    पक्ष X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      विकल्प (at->type)
		{
		पक्ष FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; भङ्ग;
		पक्ष FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; भङ्ग;
		पक्ष FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; भङ्ग;
		स्वभाव:
		  जइ (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  अण्णहा
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  भङ्ग;
		}
	      gprcount++;
	      भङ्ग;
	    पक्ष X86_64_SSE_CLASS:
	    पक्ष X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (संरचना register_args, sse)
				      + ssecount * परिमाण (सङ्घ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      भङ्ग;
	    पक्ष X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (संरचना register_args, sse)
				      + ssecount * परिमाण (सङ्घ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      भङ्ग;
	    स्वभाव:
	      मुक्त (plan);		/* X87 etc. in registers: bail */
	      प्रत्यागम रिक्त;
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
    ? (पूर्णाङ्क) avn : -1;
  प्रत्यागम plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
स्थिर अन्तःस्थ __attribute__ ((always_inline)) शून्य
plan_exec (ffi_cif *cif, ffi_plan *plan, शून्य (*fn) (शून्य),
	   शून्य *rvalue, शून्य **avalue)
{
  unsigned flags = plan->flags;
  संरचना register_args local __attribute__ ((aligned (16)));
  वर्ण *राशिः = रिक्त;
  संरचना register_args *reg_args;
  unsigned k;

  जइ (rvalue == रिक्त)
    {
      जइ (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      अण्णहा
	flags = UNIX64_RET_VOID;
    }

  जइ (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      संरचना ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      जइ (rvalue != रिक्त)
	store_ret (rvalue, plan->retcode, r);
      प्रत्यागम;
    }

  जइ (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  अण्णहा
    {
      राशिः = alloca (परिमाण (संरचना register_args) + plan->bytes + 4 * 8);
      reg_args = (संरचना register_args *) राशिः;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  जइ (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  प्रति (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      वर्ण *src = (वर्ण *) avalue[m->src_idx] + m->src_off;
      वर्ण *dst = (वर्ण *) reg_args + m->dst_off;
      विकल्प (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	पक्ष FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; भङ्ग;
	पक्ष FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; भङ्ग;
	पक्ष FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; भङ्ग;
	पक्ष FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   भङ्ग;
	पक्ष FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     भङ्ग;
	पक्ष FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   भङ्ग;
	पक्ष FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   भङ्ग;
	पक्ष FFI_MOVE_राशिः: memcpy (dst, src, m->len);                          भङ्ग;
	}
    }
  reg_args->rax = plan->ssecount;

  जइ (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      संरचना ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      जइ (rvalue != रिक्त)
	store_ret (rvalue, plan->retcode, r);
      प्रत्यागम;
    }

  ffi_call_unix64 (राशिः, plan->bytes + परिमाण (संरचना register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
संरचना ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (परिमाण (संरचना ffi_call_plan));
  जइ (plan == रिक्त)
    प्रत्यागम रिक्त;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  प्रत्यागम plan;
}

शून्य
ffi_call_plan_invoke (ffi_call_plan *plan, शून्य (*fn) (शून्य),
		      शून्य *rvalue, शून्य **avalue)
{
  जइ (plan->fast != रिक्त)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  अण्णहा
    ffi_call (plan->cif, fn, rvalue, avalue);
}

शून्य
ffi_call_plan_free (ffi_call_plan *plan)
{
  जइ (plan != रिक्त)
    {
      मुक्त (plan->fast);
      मुक्त (plan);
    }
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  जइ (plan == रिक्त)
    प्रत्यागम 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  प्रत्यागम परिमाण (संरचना ffi_call_plan)
	 + (plan->fast != रिक्त ? plan->fast->alloc_bytes : 0);
}

बाह्य शून्य
ffi_call_efi64(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue);
#endif

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णाङ्क i, nargs = cif->nargs;
  ध्रुव पूर्णाङ्क max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क size = at->size;
      जइ (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          वर्ण *argcopy = alloca (size);
          जइ (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              memcpy (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  जइ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      प्रत्यागम;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
बाह्य शून्य
ffi_call_go_efi64(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
		  शून्य **avalue, शून्य *closure);
#endif

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
#ifndef __ILP32__
  जइ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      प्रत्यागम;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

बाह्य शून्य ffi_closure_unix64(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_unix64_sse(शून्य) FFI_HIDDEN;
#जइ defined(FFI_EXEC_STATIC_TRAMP)
बाह्य शून्य ffi_closure_unix64_alt(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_unix64_sse_alt(शून्य) FFI_HIDDEN;
#endif

#ifndef __ILP32__
बाह्य ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_data,
			   शून्य *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  स्थिर ध्रुव unsigned वर्ण trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  शून्य (*dest)(शून्य);
  वर्ण *tramp = closure->tramp;

#ifndef __ILP32__
  जइ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    प्रत्यागम ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  जइ (cif->abi != FFI_UNIX64)
    प्रत्यागम FFI_BAD_ABI;

  जइ (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  अण्णहा
    dest = ffi_closure_unix64;

#जइ defined(FFI_EXEC_STATIC_TRAMP)
  जइ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जइ (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      अण्णहा
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      गच्छ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, परिमाण(trampoline));
  *(UINT64 *)(tramp + परिमाण (trampoline)) = (uintptr_t)dest;

#जइ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

पूर्णाङ्क FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			 शून्य *user_data,
			 शून्य *rvalue,
			 संरचना register_args *reg_args,
			 वर्ण *argp)
{
  शून्य **avalue;
  ffi_type **arg_types;
  long i, avn;
  पूर्णाङ्क gprcount, ssecount, ngpr, nsse;
  पूर्णाङ्क flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * परिमाण(शून्य *));
  gprcount = ssecount = 0;

  जइ (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      शून्य *r = (शून्य *)(uintptr_t)reg_args->gpr[gprcount++];
      *(शून्य **)rvalue = r;
      rvalue = r;
      flags = (परिमाण(शून्य *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  प्रति (i = 0; i < avn; ++i)
    {
      गणना x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      जइ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  जइ (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (शून्य *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      अण्णहा जइ (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  जइ (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  अण्णहा
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      अण्णहा
	{
	  वर्ण *a = alloca (n * 8);
	  unsigned पूर्णाङ्क j;

	  avalue[i] = a;
	  प्रति (j = 0; j < n; j++, a += 8)
	    {
	      जइ (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (वर्ण *) &reg_args->sse[ssecount - 1] + 8, 8);
	      अण्णहा जइ (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      अण्णहा
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  प्रत्यागम flags;
}

#ifdef FFI_GO_CLOSURES

बाह्य शून्य ffi_go_closure_unix64(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_unix64_sse(शून्य) FFI_HIDDEN;

#ifndef __ILP32__
बाह्य ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
#ifndef __ILP32__
  जइ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    प्रत्यागम ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  जइ (cif->abi != FFI_UNIX64)
    प्रत्यागम FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#जइ defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

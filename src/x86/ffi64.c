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

#जर defined(__INTEL_COMPILER)
#अन्तर्भाव "xmmintrin.h"
#define UINT128 __m128
#अन्यथा
#जर defined(__SUNPRO_C)
#अन्तर्भाव <sunmedia_types.h>
#define UINT128 __m128i
#अन्यथा
#define UINT128 __int128_t
#endif
#endif

संघ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

संरचना register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  संघ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

बाह्य शून्य ffi_call_unix64 (शून्य *args, अचिन्हित दीर्घ bytes, अचिन्हित flags,
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
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#अन्यथा
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
  जर (class1 == class2)
    परत class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  जर (class1 == X86_64_NO_CLASS)
    परत class2;
  जर (class2 == X86_64_NO_CLASS)
    परत class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  जर (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    परत X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  जर ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    परत X86_64_INTEGERSI_CLASS;
  जर (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    परत X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  जर (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    परत X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  परत X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
स्थिर माप_प्रकार
classify_argument (ffi_type *type, गणना x86_64_reg_class classes[],
		   माप_प्रकार byte_offset)
{
  निवड (type->type)
    {
    स्थिती FFI_TYPE_UINT8:
    स्थिती FFI_TYPE_SINT8:
    स्थिती FFI_TYPE_UINT16:
    स्थिती FFI_TYPE_SINT16:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT128:
    स्थिती FFI_TYPE_SINT128:
    स्थिती FFI_TYPE_POINTER:
    do_integer:
      {
	माप_प्रकार size = byte_offset + type->size;

	जर (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    परत 1;
	  }
	अन्यथा जर (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    परत 1;
	  }
	अन्यथा जर (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    परत 2;
	  }
	अन्यथा जर (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    परत 2;
	  }
	अन्यथा
	  FFI_ASSERT (0);
      }
    स्थिती FFI_TYPE_FLOAT:
      जर (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      अन्यथा
	classes[0] = X86_64_SSE_CLASS;
      परत 1;
    स्थिती FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      परत 1;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
#जर FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#अन्यथा
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      परत 2;
#endif
    स्थिती FFI_TYPE_STRUCT:
      {
	स्थिरांक माप_प्रकार UNITS_PER_WORD = 8;
        माप_प्रकार शब्द = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	अचिन्हित पूर्णांक i;
	गणना x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	जर (type->size > 32)
	  परत 0;

	साठी (i = 0; i < शब्द; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	जर (!शब्द)
	  {
    स्थिती FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    परत 1;
	  }

	/* Merge the fields of structure.  */
	साठी (ptr = type->elements; *ptr != रिक्त; ptr++)
	  {
	    माप_प्रकार अंक, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    अंक = classify_argument (*ptr, subclasses, byte_offset % 8);
	    जर (अंक == 0)
	      परत 0;
            pos = byte_offset / 8;
            साठी (i = 0; i < अंक && (i + pos) < शब्द; i++)
	      {
		माप_प्रकार pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	जर (शब्द > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    जर (classes[0] != X86_64_SSE_CLASS)
	      परत 0;

	    साठी (i = 1; i < शब्द; i++)
	      जर (classes[i] != X86_64_SSEUP_CLASS)
		परत 0;
	  }

	/* Final merger cleanup.  */
	साठी (i = 0; i < शब्द; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    जर (classes[i] == X86_64_MEMORY_CLASS)
	      परत 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    जर (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    जर (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		परत 0;
	      }
	  }
	परत शब्द;
      }
    स्थिती FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      जर (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  परत 1;
	}
      अन्यथा जर (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  परत 2;
	}
      परत 0;
    स्थिती FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	निवड (inner->type)
	  {
	  स्थिती FFI_TYPE_INT:
	  स्थिती FFI_TYPE_UINT8:
	  स्थिती FFI_TYPE_SINT8:
	  स्थिती FFI_TYPE_UINT16:
	  स्थिती FFI_TYPE_SINT16:
	  स्थिती FFI_TYPE_UINT32:
	  स्थिती FFI_TYPE_SINT32:
	  स्थिती FFI_TYPE_UINT64:
	  स्थिती FFI_TYPE_SINT64:
	    जा do_integer;

	  स्थिती FFI_TYPE_SINT128:
	  स्थिती FFI_TYPE_UINT128:
	    परत 0;

	  स्थिती FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    जर (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		परत 2;
	      }
	    परत 1;
	  स्थिती FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    परत 2;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  स्थिती FFI_TYPE_LONGDOUBLE:
#जर FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    परत 0;
#अन्यथा
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    परत 1;
#endif
#endif
	  }
      }
    }
  पात();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

स्थिर माप_प्रकार
examine_argument (ffi_type *type, गणना x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, पूर्णांक *pngpr, पूर्णांक *pnsse)
{
  माप_प्रकार n;
  अचिन्हित पूर्णांक i;
  पूर्णांक ngpr, nsse;

  n = classify_argument (type, classes, 0);
  जर (n == 0)
    परत 0;

  ngpr = nsse = 0;
  साठी (i = 0; i < n; ++i)
    निवड (classes[i])
      {
      स्थिती X86_64_INTEGER_CLASS:
      स्थिती X86_64_INTEGERSI_CLASS:
	ngpr++;
	थांबा;
      स्थिती X86_64_SSE_CLASS:
      स्थिती X86_64_SSESF_CLASS:
      स्थिती X86_64_SSEDF_CLASS:
	nsse++;
	थांबा;
      स्थिती X86_64_NO_CLASS:
      स्थिती X86_64_SSEUP_CLASS:
	थांबा;
      स्थिती X86_64_X87_CLASS:
      स्थिती X86_64_X87UP_CLASS:
      स्थिती X86_64_COMPLEX_X87_CLASS:
	परत in_return != 0;
      पूर्वनिर्धारित:
	पात ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  परत n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
बाह्य ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णांक gprcount, ssecount, i, avn, ngpr, nsse;
  अचिन्हित flags;
  गणना x86_64_reg_class classes[MAX_CLASSES];
  माप_प्रकार bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  जर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    परत ffi_prep_cif_machdep_efi64(cif);
#endif
  जर (cif->abi != FFI_UNIX64)
    परत FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  निवड (rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      थांबा;
    स्थिती FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      थांबा;
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      थांबा;
    स्थिती FFI_TYPE_UINT128:
    स्थिती FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      थांबा;
    स्थिती FFI_TYPE_POINTER:
      flags = (आकार(शून्य *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      थांबा;
    स्थिती FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      थांबा;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
#जर FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#अन्यथा
      flags = UNIX64_RET_X87;
#endif
      थांबा;
#endif
    स्थिती FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      जर (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      अन्यथा
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  जर (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  अन्यथा जर (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  अन्यथा
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      जर (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      अन्यथा जर (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      अन्यथा जर (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      अन्यथा
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      थांबा;
    स्थिती FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      जर (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      अन्यथा जर (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      अन्यथा
	परत FFI_BAD_TYPEDEF;
      थांबा;
    स्थिती FFI_TYPE_COMPLEX:
      निवड (rtype->elements[0]->type)
	{
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((अचिन्हित) rtype_size << UNIX64_SIZE_SHIFT);
	  थांबा;
	स्थिती FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  थांबा;
	स्थिती FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  थांबा;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
#जर FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#अन्यथा
	  flags = UNIX64_RET_X87_2;
#endif
	  थांबा;
#endif
	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  थांबा;
	पूर्वनिर्धारित:
	  परत FFI_BAD_TYPEDEF;
	}
      थांबा;
    पूर्वनिर्धारित:
      परत FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  साठी (i = 0, avn = cif->nargs; i < avn; i++)
    जर (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      परत FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  साठी (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      जर (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = cif->arg_types[i]->alignment;

	  जर (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      अन्यथा
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  जर (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (अचिन्हित) FFI_ALIGN (bytes, 8);

  परत FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  गणना x86_64_reg_class classes[MAX_CLASSES];
  अक्षर *चळत, *argp;
  ffi_type **arg_types;
  पूर्णांक gprcount, ssecount, ngpr, nsse, i, avn, flags;
  संरचना register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  जर (rvalue == रिक्त)
    {
      जर (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      अन्यथा
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  चळत = alloca (आकार (संरचना register_args) + cif->bytes + 4*8);
  reg_args = (संरचना register_args *) चळत;
  argp = चळत + आकार (संरचना register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  जर (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (अचिन्हित दीर्घ) rvalue;

  साठी (i = 0; i < avn; ++i)
    {
      माप_प्रकार n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      जर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  जर (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (शून्य *) FFI_ALIGN (argp, align);
          स_नकल (argp, avalue[i], size);

          argp += size;
        }
      अन्यथा
	{
	  /* The argument is passed entirely in registers.  */
	  अक्षर *a = (अक्षर *) avalue[i];
	  अचिन्हित पूर्णांक j;

	  साठी (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      निवड (classes[j])
		{
		स्थिती X86_64_NO_CLASS:
		  थांबा;
		स्थिती X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  स_नकल ((अक्षर *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  थांबा;
		स्थिती X86_64_INTEGER_CLASS:
		स्थिती X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  निवड (arg_types[i]->type)
		    {
		    स्थिती FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      थांबा;
		    स्थिती FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      थांबा;
		    स्थिती FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      थांबा;
		    पूर्वनिर्धारित:
		      reg_args->gpr[gprcount] = 0;
		      स_नकल (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  थांबा;
		स्थिती X86_64_SSE_CLASS:
		स्थिती X86_64_SSEDF_CLASS:
		  स_नकल (&reg_args->sse[ssecount++].i64, a, आकार(UINT64));
		  थांबा;
		स्थिती X86_64_SSESF_CLASS:
		  स_नकल (&reg_args->sse[ssecount++].i32, a, आकार(UINT32));
		  थांबा;
		पूर्वनिर्धारित:
		  पात();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (चळत, cif->bytes + आकार (संरचना register_args),
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
  FFI_MOVE_चळत                               /* copy len bytes -> stack       */
};

प्रकारव्याख्या संरचना
{
  अचिन्हित src_idx;     /* avalue[] index                                  */
  अचिन्हित src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  अचिन्हित dst_off;     /* byte offset within the register_args+stack buf  */
  अचिन्हित len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  अचिन्हित अक्षर op;
} ffi_move;

प्रकारव्याख्या संरचना
{
  अचिन्हित nmoves;
  अचिन्हित ssecount;    /* -> reg_args->rax                                */
  अचिन्हित bytes;       /* stack-arg area size (== cif->bytes)             */
  अचिन्हित flags;       /* == cif->flags                                   */
  अचिन्हित ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  अचिन्हित fast;        /* nonzero -> lean trampoline eligible             */
  अचिन्हित retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  पूर्णांक      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  अचिन्हित alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
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
स्थिर संरचना ffi_ret2 (*स्थिरांक ffi_gp_thunks[7]) (शून्य **, शून्य (*)(शून्य)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
स्थिर ओळीत शून्य
store_ret (शून्य *rvalue, अचिन्हित retcode, संरचना ffi_ret2 r)
{
  निवड (retcode)
    {
    स्थिती UNIX64_RET_VOID:   थांबा;
    स्थिती UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; थांबा;
    स्थिती UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; थांबा;
    स्थिती UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; थांबा;
    स्थिती UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; थांबा;
    स्थिती UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; थांबा;
    स्थिती UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; थांबा;
    स्थिती UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; थांबा;
    स्थिती UNIX64_RET_XMM32:  स_नकल (rvalue, &r.d, 4); थांबा;
    स्थिती UNIX64_RET_XMM64:  स_नकल (rvalue, &r.d, 8); थांबा;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
स्थिर ffi_plan *
build_plan (ffi_cif *cif)
{
  अचिन्हित i, avn = cif->nargs;
  गणना x86_64_reg_class classes[MAX_CLASSES];
  अचिन्हित nm, gprcount, ssecount;
  माप_प्रकार argp_off, nbytes;
  ffi_plan *plan;
  पूर्णांक all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  जर (cif->abi != FFI_UNIX64)
    परत रिक्त;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  साठी (i = 0; i < avn; i++)
    {
      पूर्णांक t = cif->arg_types[i]->type;
      जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	परत रिक्त;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      जर (t == FFI_TYPE_LONGDOUBLE)
	परत रिक्त;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = आकार (ffi_plan) + आकार (ffi_move) * (2 * avn + 1);
  plan = दो_स्मृति (nbytes);
  जर (plan == रिक्त)
    परत रिक्त;
  plan->alloc_bytes = (अचिन्हित) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  जर (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  साठी (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      माप_प्रकार size = at->size, n, rem;
      पूर्णांक ngpr, nsse;
      अचिन्हित j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      जर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  जर (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_चळत;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (अचिन्हित) (आकार (संरचना register_args) + argp_off);
	  m->len = (अचिन्हित) size;
	  argp_off += size;
	  सुरूठेवा;
	}

      साठी (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  निवड (classes[j])
	    {
	    स्थिती X86_64_NO_CLASS:
	    स्थिती X86_64_SSEUP_CLASS:
	      सुरूठेवा;			/* nothing placed for this 8-byte */
	    स्थिती X86_64_INTEGER_CLASS:
	    स्थिती X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      निवड (at->type)
		{
		स्थिती FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; थांबा;
		स्थिती FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; थांबा;
		स्थिती FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; थांबा;
		पूर्वनिर्धारित:
		  जर (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  अन्यथा
		    { m.op = FFI_MOVE_GP; m.len = (अचिन्हित) rem; all_gp64 = 0; }
		  थांबा;
		}
	      gprcount++;
	      थांबा;
	    स्थिती X86_64_SSE_CLASS:
	    स्थिती X86_64_SSEDF_CLASS:
	      m.dst_off = (अचिन्हित) (दुरत्व (संरचना register_args, sse)
				      + ssecount * आकार (संघ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      थांबा;
	    स्थिती X86_64_SSESF_CLASS:
	      m.dst_off = (अचिन्हित) (दुरत्व (संरचना register_args, sse)
				      + ssecount * आकार (संघ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      थांबा;
	    पूर्वनिर्धारित:
	      मुक्त (plan);		/* X87 etc. in registers: bail */
	      परत रिक्त;
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
    ? (पूर्णांक) avn : -1;
  परत plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
स्थिर ओळीत __attribute__ ((always_inline)) शून्य
plan_exec (ffi_cif *cif, ffi_plan *plan, शून्य (*fn) (शून्य),
	   शून्य *rvalue, शून्य **avalue)
{
  अचिन्हित flags = plan->flags;
  संरचना register_args local __attribute__ ((aligned (16)));
  अक्षर *चळत = रिक्त;
  संरचना register_args *reg_args;
  अचिन्हित k;

  जर (rvalue == रिक्त)
    {
      जर (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      अन्यथा
	flags = UNIX64_RET_VOID;
    }

  जर (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      संरचना ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      जर (rvalue != रिक्त)
	store_ret (rvalue, plan->retcode, r);
      परत;
    }

  जर (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  अन्यथा
    {
      चळत = alloca (आकार (संरचना register_args) + plan->bytes + 4 * 8);
      reg_args = (संरचना register_args *) चळत;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  जर (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  साठी (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      अक्षर *src = (अक्षर *) avalue[m->src_idx] + m->src_off;
      अक्षर *dst = (अक्षर *) reg_args + m->dst_off;
      निवड (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	स्थिती FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; थांबा;
	स्थिती FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; थांबा;
	स्थिती FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; थांबा;
	स्थिती FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   थांबा;
	स्थिती FFI_MOVE_GP:    *(UINT64 *) dst = 0; स_नकल (dst, src, m->len);     थांबा;
	स्थिती FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   थांबा;
	स्थिती FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   थांबा;
	स्थिती FFI_MOVE_चळत: स_नकल (dst, src, m->len);                          थांबा;
	}
    }
  reg_args->rax = plan->ssecount;

  जर (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      संरचना ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      जर (rvalue != रिक्त)
	store_ret (rvalue, plan->retcode, r);
      परत;
    }

  ffi_call_unix64 (चळत, plan->bytes + आकार (संरचना register_args),
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
  ffi_call_plan *plan = दो_स्मृति (आकार (संरचना ffi_call_plan));
  जर (plan == रिक्त)
    परत रिक्त;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  परत plan;
}

शून्य
ffi_call_plan_invoke (ffi_call_plan *plan, शून्य (*fn) (शून्य),
		      शून्य *rvalue, शून्य **avalue)
{
  जर (plan->fast != रिक्त)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  अन्यथा
    ffi_call (plan->cif, fn, rvalue, avalue);
}

शून्य
ffi_call_plan_free (ffi_call_plan *plan)
{
  जर (plan != रिक्त)
    {
      मुक्त (plan->fast);
      मुक्त (plan);
    }
}

माप_प्रकार
ffi_call_plan_size (ffi_call_plan *plan)
{
  जर (plan == रिक्त)
    परत 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  परत आकार (संरचना ffi_call_plan)
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
  पूर्णांक i, nargs = cif->nargs;
  स्थिरांक पूर्णांक max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक size = at->size;
      जर (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          अक्षर *argcopy = alloca (size);
          जर (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * आकार (शून्य *));
              स_नकल (avalue_copy, avalue, nargs * आकार (शून्य *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  जर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      परत;
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
  जर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      परत;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

बाह्य शून्य ffi_closure_unix64(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_unix64_sse(शून्य) FFI_HIDDEN;
#जर defined(FFI_EXEC_STATIC_TRAMP)
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
  स्थिर स्थिरांक अचिन्हित अक्षर trampoline[24] = {
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
  अक्षर *tramp = closure->tramp;

#ifndef __ILP32__
  जर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    परत ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  जर (cif->abi != FFI_UNIX64)
    परत FFI_BAD_ABI;

  जर (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  अन्यथा
    dest = ffi_closure_unix64;

#जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जर (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      अन्यथा
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      जा out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  स_नकल (tramp, trampoline, आकार(trampoline));
  *(UINT64 *)(tramp + आकार (trampoline)) = (uintptr_t)dest;

#जर defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

पूर्णांक FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			 शून्य *user_data,
			 शून्य *rvalue,
			 संरचना register_args *reg_args,
			 अक्षर *argp)
{
  शून्य **avalue;
  ffi_type **arg_types;
  दीर्घ i, avn;
  पूर्णांक gprcount, ssecount, ngpr, nsse;
  पूर्णांक flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * आकार(शून्य *));
  gprcount = ssecount = 0;

  जर (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      शून्य *r = (शून्य *)(uintptr_t)reg_args->gpr[gprcount++];
      *(शून्य **)rvalue = r;
      rvalue = r;
      flags = (आकार(शून्य *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  साठी (i = 0; i < avn; ++i)
    {
      गणना x86_64_reg_class classes[MAX_CLASSES];
      माप_प्रकार n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      जर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  जर (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (शून्य *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      अन्यथा जर (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  जर (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  अन्यथा
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      अन्यथा
	{
	  अक्षर *a = alloca (n * 8);
	  अचिन्हित पूर्णांक j;

	  avalue[i] = a;
	  साठी (j = 0; j < n; j++, a += 8)
	    {
	      जर (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		स_नकल (a, (अक्षर *) &reg_args->sse[ssecount - 1] + 8, 8);
	      अन्यथा जर (SSE_CLASS_P (classes[j]))
		स_नकल (a, &reg_args->sse[ssecount++], 8);
	      अन्यथा
		स_नकल (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  परत flags;
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
  जर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    परत ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  जर (cif->abi != FFI_UNIX64)
    परत FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  परत &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

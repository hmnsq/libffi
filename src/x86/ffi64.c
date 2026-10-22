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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <stdarg.h>
#समावेश <stdint.h>
#समावेश <stddef.h>
#समावेश <string.h>
#समावेश <tramp.h>
#समावेश "internal64.h"

#अगर_घोषित __x86_64__

#घोषणा MAX_GPR_REGS 6
#घोषणा MAX_SSE_REGS 8

#अगर defined(__INTEL_COMPILER)
#समावेश "xmmintrin.h"
#घोषणा UINT128 __m128
#अन्यथा
#अगर defined(__SUNPRO_C)
#समावेश <sunmedia_types.h>
#घोषणा UINT128 __m128i
#अन्यथा
#घोषणा UINT128 __int128_t
#पूर्ण_अगर
#पूर्ण_अगर

जोड़ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

काष्ठा register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  जोड़ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

बाह्य व्योम ffi_call_unix64 (व्योम *args, अचिन्हित दीर्घ bytes, अचिन्हित flags,
			     व्योम *raddr, व्योम (*fnaddr)(व्योम)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
क्रमागत x86_64_reg_class
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

#घोषणा MAX_CLASSES 4

#घोषणा SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#अन्यथा
# define FFI_LONGDOUBLE_BINARY128 0
#पूर्ण_अगर

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

अटल क्रमागत x86_64_reg_class
merge_classes (क्रमागत x86_64_reg_class class1, क्रमागत x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  अगर (class1 == class2)
    वापस class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  अगर (class1 == X86_64_NO_CLASS)
    वापस class2;
  अगर (class2 == X86_64_NO_CLASS)
    वापस class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  अगर (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    वापस X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  अगर ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    वापस X86_64_INTEGERSI_CLASS;
  अगर (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    वापस X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  अगर (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    वापस X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  वापस X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
अटल माप_प्रकार
classify_argument (ffi_type *type, क्रमागत x86_64_reg_class classes[],
		   माप_प्रकार byte_offset)
{
  चयन (type->type)
    {
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_UINT16:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT128:
    हाल FFI_TYPE_SINT128:
    हाल FFI_TYPE_POINTER:
    do_integer:
      {
	माप_प्रकार आकार = byte_offset + type->आकार;

	अगर (आकार <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    वापस 1;
	  }
	अन्यथा अगर (आकार <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    वापस 1;
	  }
	अन्यथा अगर (आकार <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    वापस 2;
	  }
	अन्यथा अगर (आकार <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    वापस 2;
	  }
	अन्यथा
	  FFI_ASSERT (0);
      }
    हाल FFI_TYPE_FLOAT:
      अगर (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      अन्यथा
	classes[0] = X86_64_SSE_CLASS;
      वापस 1;
    हाल FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      वापस 1;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
#अगर FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#अन्यथा
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#पूर्ण_अगर
      वापस 2;
#पूर्ण_अगर
    हाल FFI_TYPE_STRUCT:
      {
	स्थिर माप_प्रकार UNITS_PER_WORD = 8;
        माप_प्रकार शब्दगण = (type->आकार + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	अचिन्हित पूर्णांक i;
	क्रमागत x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	अगर (type->आकार > 32)
	  वापस 0;

	क्रम (i = 0; i < शब्दगण; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	अगर (!शब्दगण)
	  {
    हाल FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    वापस 1;
	  }

	/* Merge the fields of structure.  */
	क्रम (ptr = type->elements; *ptr != शून्य; ptr++)
	  {
	    माप_प्रकार अंक, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    अंक = classify_argument (*ptr, subclasses, byte_offset % 8);
	    अगर (अंक == 0)
	      वापस 0;
            pos = byte_offset / 8;
            क्रम (i = 0; i < अंक && (i + pos) < शब्दगण; i++)
	      {
		माप_प्रकार pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->आकार;
	  }

	अगर (शब्दगण > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    अगर (classes[0] != X86_64_SSE_CLASS)
	      वापस 0;

	    क्रम (i = 1; i < शब्दगण; i++)
	      अगर (classes[i] != X86_64_SSEUP_CLASS)
		वापस 0;
	  }

	/* Final merger cleanup.  */
	क्रम (i = 0; i < शब्दगण; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    अगर (classes[i] == X86_64_MEMORY_CLASS)
	      वापस 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    अगर (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    अगर (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		वापस 0;
	      }
	  }
	वापस शब्दगण;
      }
    हाल FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      अगर (type->आकार == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  वापस 1;
	}
      अन्यथा अगर (type->आकार == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  वापस 2;
	}
      वापस 0;
    हाल FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	चयन (inner->type)
	  {
	  हाल FFI_TYPE_INT:
	  हाल FFI_TYPE_UINT8:
	  हाल FFI_TYPE_SINT8:
	  हाल FFI_TYPE_UINT16:
	  हाल FFI_TYPE_SINT16:
	  हाल FFI_TYPE_UINT32:
	  हाल FFI_TYPE_SINT32:
	  हाल FFI_TYPE_UINT64:
	  हाल FFI_TYPE_SINT64:
	    जाओ do_integer;

	  हाल FFI_TYPE_SINT128:
	  हाल FFI_TYPE_UINT128:
	    वापस 0;

	  हाल FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    अगर (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		वापस 2;
	      }
	    वापस 1;
	  हाल FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    वापस 2;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  हाल FFI_TYPE_LONGDOUBLE:
#अगर FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    वापस 0;
#अन्यथा
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    वापस 1;
#पूर्ण_अगर
#पूर्ण_अगर
	  }
      }
    }
  पात();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

अटल माप_प्रकार
examine_argument (ffi_type *type, क्रमागत x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, पूर्णांक *pngpr, पूर्णांक *pnsse)
{
  माप_प्रकार n;
  अचिन्हित पूर्णांक i;
  पूर्णांक ngpr, nsse;

  n = classify_argument (type, classes, 0);
  अगर (n == 0)
    वापस 0;

  ngpr = nsse = 0;
  क्रम (i = 0; i < n; ++i)
    चयन (classes[i])
      {
      हाल X86_64_INTEGER_CLASS:
      हाल X86_64_INTEGERSI_CLASS:
	ngpr++;
	अवरोध;
      हाल X86_64_SSE_CLASS:
      हाल X86_64_SSESF_CLASS:
      हाल X86_64_SSEDF_CLASS:
	nsse++;
	अवरोध;
      हाल X86_64_NO_CLASS:
      हाल X86_64_SSEUP_CLASS:
	अवरोध;
      हाल X86_64_X87_CLASS:
      हाल X86_64_X87UP_CLASS:
      हाल X86_64_COMPLEX_X87_CLASS:
	वापस in_return != 0;
      शेष:
	पात ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  वापस n;
}

/* Perform machine dependent cif processing.  */

#अगर_अघोषित __ILP32__
बाह्य ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#पूर्ण_अगर

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  पूर्णांक gprcount, ssecount, i, avn, ngpr, nsse;
  अचिन्हित flags;
  क्रमागत x86_64_reg_class classes[MAX_CLASSES];
  माप_प्रकार bytes, n, rtype_आकार;
  ffi_type *rtype;

#अगर_अघोषित __ILP32__
  अगर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    वापस ffi_prep_cif_machdep_efi64(cif);
#पूर्ण_अगर
  अगर (cif->abi != FFI_UNIX64)
    वापस FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_आकार = rtype->आकार;
  चयन (rtype->type)
    {
    हाल FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      अवरोध;
    हाल FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      अवरोध;
    हाल FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      अवरोध;
    हाल FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      अवरोध;
    हाल FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      अवरोध;
    हाल FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      अवरोध;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      अवरोध;
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      अवरोध;
    हाल FFI_TYPE_UINT128:
    हाल FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_आकार_SHIFT);
      अवरोध;
    हाल FFI_TYPE_POINTER:
      flags = (माप(व्योम *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      अवरोध;
    हाल FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      अवरोध;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
#अगर FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#अन्यथा
      flags = UNIX64_RET_X87;
#पूर्ण_अगर
      अवरोध;
#पूर्ण_अगर
    हाल FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      अगर (n == 0)
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

	  अगर (rtype_आकार == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  अन्यथा अगर (rtype_आकार == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  अन्यथा
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      अगर (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      अन्यथा अगर (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      अन्यथा अगर (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      अन्यथा
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_आकार << UNIX64_आकार_SHIFT;
	    }
	}
      अवरोध;
    हाल FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      अगर (rtype_आकार == 8)
	flags = UNIX64_RET_XMM64;
      अन्यथा अगर (rtype_आकार == 16)
	flags = UNIX64_RET_XMM128;
      अन्यथा
	वापस FFI_BAD_TYPEDEF;
      अवरोध;
    हाल FFI_TYPE_COMPLEX:
      चयन (rtype->elements[0]->type)
	{
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((अचिन्हित) rtype_आकार << UNIX64_आकार_SHIFT);
	  अवरोध;
	हाल FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  अवरोध;
	हाल FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_आकार_SHIFT);
	  अवरोध;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
#अगर FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#अन्यथा
	  flags = UNIX64_RET_X87_2;
#पूर्ण_अगर
	  अवरोध;
#पूर्ण_अगर
	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  अवरोध;
	शेष:
	  वापस FFI_BAD_TYPEDEF;
	}
      अवरोध;
    शेष:
      वापस FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  क्रम (i = 0, avn = cif->nargs; i < avn; i++)
    अगर (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->आकार > 16)
      वापस FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  क्रम (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      अगर (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = cif->arg_types[i]->alignment;

	  अगर (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->आकार;
	}
      अन्यथा
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  अगर (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (अचिन्हित) FFI_ALIGN (bytes, 8);

  वापस FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	      व्योम **avalue, व्योम *closure)
{
  क्रमागत x86_64_reg_class classes[MAX_CLASSES];
  अक्षर *ढेर, *argp;
  ffi_type **arg_types;
  पूर्णांक gprcount, ssecount, ngpr, nsse, i, avn, flags;
  काष्ठा register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  अगर (rvalue == शून्य)
    {
      अगर (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->आकार);
      अन्यथा
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ढेर = alloca (माप (काष्ठा register_args) + cif->bytes + 4*8);
  reg_args = (काष्ठा register_args *) ढेर;
  argp = ढेर + माप (काष्ठा register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  अगर (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (अचिन्हित दीर्घ) rvalue;

  क्रम (i = 0; i < avn; ++i)
    {
      माप_प्रकार n, आकार = arg_types[i]->आकार;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      अगर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  अगर (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (व्योम *) FFI_ALIGN (argp, align);
          स_नकल (argp, avalue[i], आकार);

          argp += आकार;
        }
      अन्यथा
	{
	  /* The argument is passed entirely in registers.  */
	  अक्षर *a = (अक्षर *) avalue[i];
	  अचिन्हित पूर्णांक j;

	  क्रम (j = 0; j < n; j++, a += 8, आकार -= 8)
	    {
	      चयन (classes[j])
		{
		हाल X86_64_NO_CLASS:
		  अवरोध;
		हाल X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  स_नकल ((अक्षर *) &reg_args->sse[ssecount - 1] + 8, a,
			  आकार < 8 ? आकार : 8);
		  अवरोध;
		हाल X86_64_INTEGER_CLASS:
		हाल X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  चयन (arg_types[i]->type)
		    {
		    हाल FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      अवरोध;
		    हाल FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      अवरोध;
		    हाल FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      अवरोध;
		    शेष:
		      reg_args->gpr[gprcount] = 0;
		      स_नकल (&reg_args->gpr[gprcount], a, आकार <= 8 ? आकार : 8);
		    }
		  gprcount++;
		  अवरोध;
		हाल X86_64_SSE_CLASS:
		हाल X86_64_SSEDF_CLASS:
		  स_नकल (&reg_args->sse[ssecount++].i64, a, माप(UINT64));
		  अवरोध;
		हाल X86_64_SSESF_CLASS:
		  स_नकल (&reg_args->sse[ssecount++].i32, a, माप(UINT32));
		  अवरोध;
		शेष:
		  पात();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (ढेर, cif->bytes + माप (काष्ठा register_args),
		   flags, rvalue, fn);
}

#अगर_अघोषित __ILP32__
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

क्रमागत ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ढेर                               /* copy len bytes -> stack       */
};

प्रकार काष्ठा
{
  अचिन्हित src_idx;     /* avalue[] index                                  */
  अचिन्हित src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  अचिन्हित dst_off;     /* byte offset within the register_args+stack buf  */
  अचिन्हित len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  अचिन्हित अक्षर op;
} ffi_move;

प्रकार काष्ठा
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
काष्ठा ffi_ret2 { UINT64 i; द्विगुन d; };
बाह्य काष्ठा ffi_ret2 ffi_plan_fast_call (काष्ठा register_args *img,
					   व्योम (*fn) (व्योम)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
बाह्य काष्ठा ffi_ret2 ffi_plan_gp0 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp1 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp2 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp3 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp4 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp5 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
बाह्य काष्ठा ffi_ret2 ffi_plan_gp6 (व्योम **, व्योम (*)(व्योम)) FFI_HIDDEN;
अटल काष्ठा ffi_ret2 (*स्थिर ffi_gp_thunks[7]) (व्योम **, व्योम (*)(व्योम)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
अटल अंतरभूत व्योम
store_ret (व्योम *rvalue, अचिन्हित retcode, काष्ठा ffi_ret2 r)
{
  चयन (retcode)
    {
    हाल UNIX64_RET_VOID:   अवरोध;
    हाल UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; अवरोध;
    हाल UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; अवरोध;
    हाल UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; अवरोध;
    हाल UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; अवरोध;
    हाल UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; अवरोध;
    हाल UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; अवरोध;
    हाल UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; अवरोध;
    हाल UNIX64_RET_XMM32:  स_नकल (rvalue, &r.d, 4); अवरोध;
    हाल UNIX64_RET_XMM64:  स_नकल (rvalue, &r.d, 8); अवरोध;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
अटल ffi_plan *
build_plan (ffi_cif *cif)
{
  अचिन्हित i, avn = cif->nargs;
  क्रमागत x86_64_reg_class classes[MAX_CLASSES];
  अचिन्हित nm, gprcount, ssecount;
  माप_प्रकार argp_off, nbytes;
  ffi_plan *plan;
  पूर्णांक all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  अगर (cif->abi != FFI_UNIX64)
    वापस शून्य;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  क्रम (i = 0; i < avn; i++)
    {
      पूर्णांक t = cif->arg_types[i]->type;
      अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	वापस शून्य;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      अगर (t == FFI_TYPE_LONGDOUBLE)
	वापस शून्य;
#पूर्ण_अगर
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = माप (ffi_plan) + माप (ffi_move) * (2 * avn + 1);
  plan = दो_स्मृति (nbytes);
  अगर (plan == शून्य)
    वापस शून्य;
  plan->alloc_bytes = (अचिन्हित) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  अगर (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  क्रम (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      माप_प्रकार आकार = at->आकार, n, rem;
      पूर्णांक ngpr, nsse;
      अचिन्हित j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      अगर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  अगर (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ढेर;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (अचिन्हित) (माप (काष्ठा register_args) + argp_off);
	  m->len = (अचिन्हित) आकार;
	  argp_off += आकार;
	  जारी;
	}

      क्रम (j = 0, rem = आकार; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  चयन (classes[j])
	    {
	    हाल X86_64_NO_CLASS:
	    हाल X86_64_SSEUP_CLASS:
	      जारी;			/* nothing placed for this 8-byte */
	    हाल X86_64_INTEGER_CLASS:
	    हाल X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      चयन (at->type)
		{
		हाल FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; अवरोध;
		हाल FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; अवरोध;
		हाल FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; अवरोध;
		शेष:
		  अगर (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  अन्यथा
		    { m.op = FFI_MOVE_GP; m.len = (अचिन्हित) rem; all_gp64 = 0; }
		  अवरोध;
		}
	      gprcount++;
	      अवरोध;
	    हाल X86_64_SSE_CLASS:
	    हाल X86_64_SSEDF_CLASS:
	      m.dst_off = (अचिन्हित) (दुरत्व (काष्ठा register_args, sse)
				      + ssecount * माप (जोड़ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      अवरोध;
	    हाल X86_64_SSESF_CLASS:
	      m.dst_off = (अचिन्हित) (दुरत्व (काष्ठा register_args, sse)
				      + ssecount * माप (जोड़ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      अवरोध;
	    शेष:
	      मुक्त (plan);		/* X87 etc. in registers: bail */
	      वापस शून्य;
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
  वापस plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
अटल अंतरभूत __attribute__ ((always_inline)) व्योम
plan_exec (ffi_cif *cif, ffi_plan *plan, व्योम (*fn) (व्योम),
	   व्योम *rvalue, व्योम **avalue)
{
  अचिन्हित flags = plan->flags;
  काष्ठा register_args local __attribute__ ((aligned (16)));
  अक्षर *ढेर = शून्य;
  काष्ठा register_args *reg_args;
  अचिन्हित k;

  अगर (rvalue == शून्य)
    {
      अगर (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->आकार);
      अन्यथा
	flags = UNIX64_RET_VOID;
    }

  अगर (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      काष्ठा ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      अगर (rvalue != शून्य)
	store_ret (rvalue, plan->retcode, r);
      वापस;
    }

  अगर (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  अन्यथा
    {
      ढेर = alloca (माप (काष्ठा register_args) + plan->bytes + 4 * 8);
      reg_args = (काष्ठा register_args *) ढेर;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  अगर (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  क्रम (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      अक्षर *src = (अक्षर *) avalue[m->src_idx] + m->src_off;
      अक्षर *dst = (अक्षर *) reg_args + m->dst_off;
      चयन (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	हाल FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; अवरोध;
	हाल FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; अवरोध;
	हाल FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; अवरोध;
	हाल FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   अवरोध;
	हाल FFI_MOVE_GP:    *(UINT64 *) dst = 0; स_नकल (dst, src, m->len);     अवरोध;
	हाल FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   अवरोध;
	हाल FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   अवरोध;
	हाल FFI_MOVE_ढेर: स_नकल (dst, src, m->len);                          अवरोध;
	}
    }
  reg_args->rax = plan->ssecount;

  अगर (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      काष्ठा ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      अगर (rvalue != शून्य)
	store_ret (rvalue, plan->retcode, r);
      वापस;
    }

  ffi_call_unix64 (ढेर, plan->bytes + माप (काष्ठा register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
काष्ठा ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = दो_स्मृति (माप (काष्ठा ffi_call_plan));
  अगर (plan == शून्य)
    वापस शून्य;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  वापस plan;
}

व्योम
ffi_call_plan_invoke (ffi_call_plan *plan, व्योम (*fn) (व्योम),
		      व्योम *rvalue, व्योम **avalue)
{
  अगर (plan->fast != शून्य)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  अन्यथा
    ffi_call (plan->cif, fn, rvalue, avalue);
}

व्योम
ffi_call_plan_free (ffi_call_plan *plan)
{
  अगर (plan != शून्य)
    {
      मुक्त (plan->fast);
      मुक्त (plan);
    }
}

माप_प्रकार
ffi_call_plan_आकार (ffi_call_plan *plan)
{
  अगर (plan == शून्य)
    वापस 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  वापस माप (काष्ठा ffi_call_plan)
	 + (plan->fast != शून्य ? plan->fast->alloc_bytes : 0);
}

बाह्य व्योम
ffi_call_efi64(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue);
#पूर्ण_अगर

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue_copy = शून्य;
  पूर्णांक i, nargs = cif->nargs;
  स्थिर पूर्णांक max_reg_struct_आकार = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक आकार = at->आकार;
      अगर (at->type == FFI_TYPE_STRUCT && आकार > max_reg_struct_आकार)
        {
          अक्षर *argcopy = alloca (आकार);
          अगर (avalue_copy == शून्य)
            {
              avalue_copy = alloca (nargs * माप (व्योम *));
              स_नकल (avalue_copy, avalue, nargs * माप (व्योम *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }

#अगर_अघोषित __ILP32__
  अगर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      वापस;
    }
#पूर्ण_अगर
  ffi_call_int (cif, fn, rvalue, avalue, शून्य);
}

#अगर_घोषित FFI_GO_CLOSURES

#अगर_अघोषित __ILP32__
बाह्य व्योम
ffi_call_go_efi64(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
		  व्योम **avalue, व्योम *closure);
#पूर्ण_अगर

व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
#अगर_अघोषित __ILP32__
  अगर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      वापस;
    }
#पूर्ण_अगर
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#पूर्ण_अगर /* FFI_GO_CLOSURES */

बाह्य व्योम ffi_closure_unix64(व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_closure_unix64_sse(व्योम) FFI_HIDDEN;
#अगर defined(FFI_EXEC_STATIC_TRAMP)
बाह्य व्योम ffi_closure_unix64_alt(व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_closure_unix64_sse_alt(व्योम) FFI_HIDDEN;
#पूर्ण_अगर

#अगर_अघोषित __ILP32__
बाह्य ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
			   व्योम *user_तथ्य,
			   व्योम *codeloc);
#पूर्ण_अगर

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अटल स्थिर अचिन्हित अक्षर trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  व्योम (*dest)(व्योम);
  अक्षर *tramp = closure->tramp;

#अगर_अघोषित __ILP32__
  अगर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    वापस ffi_prep_closure_loc_efi64(closure, cif, fun, user_तथ्य, codeloc);
#पूर्ण_अगर
  अगर (cif->abi != FFI_UNIX64)
    वापस FFI_BAD_ABI;

  अगर (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  अन्यथा
    dest = ffi_closure_unix64;

#अगर defined(FFI_EXEC_STATIC_TRAMP)
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      अगर (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      अन्यथा
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      जाओ out;
    }
#पूर्ण_अगर

  /* Initialize the dynamic trampoline. */
  स_नकल (tramp, trampoline, माप(trampoline));
  *(UINT64 *)(tramp + माप (trampoline)) = (uintptr_t)dest;

#अगर defined(FFI_EXEC_STATIC_TRAMP)
out:
#पूर्ण_अगर
  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

पूर्णांक FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
			 व्योम *user_तथ्य,
			 व्योम *rvalue,
			 काष्ठा register_args *reg_args,
			 अक्षर *argp)
{
  व्योम **avalue;
  ffi_type **arg_types;
  दीर्घ i, avn;
  पूर्णांक gprcount, ssecount, ngpr, nsse;
  पूर्णांक flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * माप(व्योम *));
  gprcount = ssecount = 0;

  अगर (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      व्योम *r = (व्योम *)(uintptr_t)reg_args->gpr[gprcount++];
      *(व्योम **)rvalue = r;
      rvalue = r;
      flags = (माप(व्योम *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  क्रम (i = 0; i < avn; ++i)
    {
      क्रमागत x86_64_reg_class classes[MAX_CLASSES];
      माप_प्रकार n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      अगर (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  दीर्घ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  अगर (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (व्योम *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->आकार;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      अन्यथा अगर (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  अगर (SSE_CLASS_P (classes[0]))
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
	  क्रम (j = 0; j < n; j++, a += 8)
	    {
	      अगर (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		स_नकल (a, (अक्षर *) &reg_args->sse[ssecount - 1] + 8, 8);
	      अन्यथा अगर (SSE_CLASS_P (classes[j]))
		स_नकल (a, &reg_args->sse[ssecount++], 8);
	      अन्यथा
		स_नकल (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_तथ्य);

  /* Tell assembly how to perform return type promotions.  */
  वापस flags;
}

#अगर_घोषित FFI_GO_CLOSURES

बाह्य व्योम ffi_go_closure_unix64(व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_go_closure_unix64_sse(व्योम) FFI_HIDDEN;

#अगर_अघोषित __ILP32__
बाह्य ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*));
#पूर्ण_अगर

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*))
{
#अगर_अघोषित __ILP32__
  अगर (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    वापस ffi_prep_go_closure_efi64(closure, cif, fun);
#पूर्ण_अगर
  अगर (cif->abi != FFI_UNIX64)
    वापस FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}

#पूर्ण_अगर /* FFI_GO_CLOSURES */

#अगर defined(FFI_EXEC_STATIC_TRAMP)
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;

  *map_आकार = UNIX64_TRAMP_MAP_आकार;
  *tramp_आकार = UNIX64_TRAMP_आकार;
  वापस &trampoline_code_table;
}
#पूर्ण_अगर

#पूर्ण_अगर /* __x86_64__ */

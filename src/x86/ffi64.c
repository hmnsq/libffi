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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stddef.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <tramp.h>
#అన్తర్భావ "internal64.h"

#నిర్వచితమైతే __x86_64__

#నిర్వచించు MAX_GPR_REGS 6
#నిర్వచించు MAX_SSE_REGS 8

#ఒకవేళ defined(__INTEL_COMPILER)
#అన్తర్భావ "xmmintrin.h"
#నిర్వచించు UINT128 __m128
#లేకపోతే
#ఒకవేళ defined(__SUNPRO_C)
#అన్తర్భావ <sunmedia_types.h>
#నిర్వచించు UINT128 __m128i
#లేకపోతే
#నిర్వచించు UINT128 __int128_t
#అయితే_ముగింపు
#అయితే_ముగింపు

సంఘం big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

నిర్మాణం రిజిస్టరు_వాదనలు
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  సంఘం big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

బాహ్య శూన్య ffi_call_unix64 (శూన్య *వాదనలు, గుర్తులేని దీర్ఘ బైట్లు, గుర్తులేని flags,
			     శూన్య *raddr, శూన్య (*fnaddr)(శూన్య)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
గణన x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87_పైకి_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_జ్ఞాపకం_CLASS
  };

#నిర్వచించు MAX_CLASSES 4

#నిర్వచించు SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#లేకపోతే
# define FFI_LONGDOUBLE_BINARY128 0
#అయితే_ముగింపు

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

స్థిర గణన x86_64_reg_class
విలీనం_classes (గణన x86_64_reg_class class1, గణన x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  ఒకవేళ (class1 == class2)
    తిరిగి class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  ఒకవేళ (class1 == X86_64_NO_CLASS)
    తిరిగి class2;
  ఒకవేళ (class2 == X86_64_NO_CLASS)
    తిరిగి class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  ఒకవేళ (class1 == X86_64_జ్ఞాపకం_CLASS || class2 == X86_64_జ్ఞాపకం_CLASS)
    తిరిగి X86_64_జ్ఞాపకం_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  ఒకవేళ ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    తిరిగి X86_64_INTEGERSI_CLASS;
  ఒకవేళ (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    తిరిగి X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  ఒకవేళ (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87_పైకి_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87_పైకి_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    తిరిగి X86_64_జ్ఞాపకం_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  తిరిగి X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
స్థిర size_t
classify_వాదం (ffi_type *type, గణన x86_64_reg_class classes[],
		   size_t byte_స్థానభ్రంశం)
{
  ఎంపిక (type->type)
    {
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT128:
    స్థితి FFI_TYPE_SINT128:
    స్థితి FFI_TYPE_సూచిక:
    do_integer:
      {
	size_t కొలత = byte_స్థానభ్రంశం + type->కొలత;

	ఒకవేళ (కొలత <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    తిరిగి 1;
	  }
	లేకపోతే ఒకవేళ (కొలత <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    తిరిగి 1;
	  }
	లేకపోతే ఒకవేళ (కొలత <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    తిరిగి 2;
	  }
	లేకపోతే ఒకవేళ (కొలత <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    తిరిగి 2;
	  }
	లేకపోతే
	  FFI_ASSERT (0);
      }
    స్థితి FFI_TYPE_FLOAT:
      ఒకవేళ (!(byte_స్థానభ్రంశం % 8))
	classes[0] = X86_64_SSESF_CLASS;
      లేకపోతే
	classes[0] = X86_64_SSE_CLASS;
      తిరిగి 1;
    స్థితి FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      తిరిగి 1;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
#ఒకవేళ FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#లేకపోతే
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87_పైకి_CLASS;
#అయితే_ముగింపు
      తిరిగి 2;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_STRUCT:
      {
	స్థిరరాశి size_t UNITS_చొప్పున_మాట = 8;
        size_t పదాలు = (type->కొలత + byte_స్థానభ్రంశం + UNITS_చొప్పున_మాట - 1)
                       / UNITS_చొప్పున_మాట;
	ffi_type **ptr;
	గుర్తులేని పూర్ణాంకం i;
	గణన x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	ఒకవేళ (type->కొలత > 32)
	  తిరిగి 0;

	కొరకు (i = 0; i < పదాలు; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	ఒకవేళ (!పదాలు)
	  {
    స్థితి FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    తిరిగి 1;
	  }

	/* Merge the fields of structure.  */
	కొరకు (ptr = type->elements; *ptr != ఖాళీ; ptr++)
	  {
	    size_t అంకె, pos;

	    byte_స్థానభ్రంశం = FFI_ALIGN (byte_స్థానభ్రంశం, (*ptr)->alignment);

	    అంకె = classify_వాదం (*ptr, subclasses, byte_స్థానభ్రంశం % 8);
	    ఒకవేళ (అంకె == 0)
	      తిరిగి 0;
            pos = byte_స్థానభ్రంశం / 8;
            కొరకు (i = 0; i < అంకె && (i + pos) < పదాలు; i++)
	      {
		size_t pos = byte_స్థానభ్రంశం / 8;
		classes[i + pos] =
		  విలీనం_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_స్థానభ్రంశం += (*ptr)->కొలత;
	  }

	ఒకవేళ (పదాలు > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    ఒకవేళ (classes[0] != X86_64_SSE_CLASS)
	      తిరిగి 0;

	    కొరకు (i = 1; i < పదాలు; i++)
	      ఒకవేళ (classes[i] != X86_64_SSEUP_CLASS)
		తిరిగి 0;
	  }

	/* Final merger cleanup.  */
	కొరకు (i = 0; i < పదాలు; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    ఒకవేళ (classes[i] == X86_64_జ్ఞాపకం_CLASS)
	      తిరిగి 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    ఒకవేళ (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    ఒకవేళ (i > 1 && classes[i] == X86_64_X87_పైకి_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		తిరిగి 0;
	      }
	  }
	తిరిగి పదాలు;
      }
    స్థితి FFI_TYPE_సదిశ:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      ఒకవేళ (type->కొలత == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  తిరిగి 1;
	}
      లేకపోతే ఒకవేళ (type->కొలత == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  తిరిగి 2;
	}
      తిరిగి 0;
    స్థితి FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	ఎంపిక (inner->type)
	  {
	  స్థితి FFI_TYPE_INT:
	  స్థితి FFI_TYPE_UINT8:
	  స్థితి FFI_TYPE_SINT8:
	  స్థితి FFI_TYPE_UINT16:
	  స్థితి FFI_TYPE_SINT16:
	  స్థితి FFI_TYPE_UINT32:
	  స్థితి FFI_TYPE_SINT32:
	  స్థితి FFI_TYPE_UINT64:
	  స్థితి FFI_TYPE_SINT64:
	    వెళ్ళు do_integer;

	  స్థితి FFI_TYPE_SINT128:
	  స్థితి FFI_TYPE_UINT128:
	    తిరిగి 0;

	  స్థితి FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    ఒకవేళ (byte_స్థానభ్రంశం % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		తిరిగి 2;
	      }
	    తిరిగి 1;
	  స్థితి FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    తిరిగి 2;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  స్థితి FFI_TYPE_LONGDOUBLE:
#ఒకవేళ FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    తిరిగి 0;
#లేకపోతే
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    తిరిగి 1;
#అయితే_ముగింపు
#అయితే_ముగింపు
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

స్థిర size_t
examine_వాదం (ffi_type *type, గణన x86_64_reg_class classes[MAX_CLASSES],
		  _Bool లోపల_return, పూర్ణాంకం *pngpr, పూర్ణాంకం *pnsse)
{
  size_t n;
  గుర్తులేని పూర్ణాంకం i;
  పూర్ణాంకం ngpr, nsse;

  n = classify_వాదం (type, classes, 0);
  ఒకవేళ (n == 0)
    తిరిగి 0;

  ngpr = nsse = 0;
  కొరకు (i = 0; i < n; ++i)
    ఎంపిక (classes[i])
      {
      స్థితి X86_64_INTEGER_CLASS:
      స్థితి X86_64_INTEGERSI_CLASS:
	ngpr++;
	ఆపు;
      స్థితి X86_64_SSE_CLASS:
      స్థితి X86_64_SSESF_CLASS:
      స్థితి X86_64_SSEDF_CLASS:
	nsse++;
	ఆపు;
      స్థితి X86_64_NO_CLASS:
      స్థితి X86_64_SSEUP_CLASS:
	ఆపు;
      స్థితి X86_64_X87_CLASS:
      స్థితి X86_64_X87_పైకి_CLASS:
      స్థితి X86_64_COMPLEX_X87_CLASS:
	తిరిగి లోపల_return != 0;
      అప్రమేయ:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  తిరిగి n;
}

/* Perform machine dependent cif processing.  */

#నిర్వచితం_కాకపోతే __ILP32__
బాహ్య ffi_స్థితి
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#అయితే_ముగింపు

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  పూర్ణాంకం gprcount, ssecount, i, avn, ngpr, nsse;
  గుర్తులేని flags;
  గణన x86_64_reg_class classes[MAX_CLASSES];
  size_t బైట్లు, n, rtype_కొలత;
  ffi_type *rtype;

#నిర్వచితం_కాకపోతే __ILP32__
  ఒకవేళ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    తిరిగి ffi_prep_cif_machdep_efi64(cif);
#అయితే_ముగింపు
  ఒకవేళ (cif->abi != FFI_UNIX64)
    తిరిగి FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_కొలత = rtype->కొలత;
  ఎంపిక (rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      ఆపు;
    స్థితి FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      ఆపు;
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      ఆపు;
    స్థితి FFI_TYPE_UINT128:
    స్థితి FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_కొలత_జరుపు);
      ఆపు;
    స్థితి FFI_TYPE_సూచిక:
      flags = (పరిమాణం(శూన్య *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      ఆపు;
    స్థితి FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      ఆపు;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
#ఒకవేళ FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#లేకపోతే
      flags = UNIX64_RET_X87;
#అయితే_ముగింపు
      ఆపు;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_STRUCT:
      n = examine_వాదం (cif->rtype, classes, 1, &ngpr, &nsse);
      ఒకవేళ (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_లోపల_MEM;
	}
      లేకపోతే
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  ఒకవేళ (rtype_కొలత == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  లేకపోతే ఒకవేళ (rtype_కొలత == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  లేకపోతే
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      ఒకవేళ (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      లేకపోతే ఒకవేళ (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      లేకపోతే ఒకవేళ (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      లేకపోతే
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_కొలత << UNIX64_కొలత_జరుపు;
	    }
	}
      ఆపు;
    స్థితి FFI_TYPE_సదిశ:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      ఒకవేళ (rtype_కొలత == 8)
	flags = UNIX64_RET_XMM64;
      లేకపోతే ఒకవేళ (rtype_కొలత == 16)
	flags = UNIX64_RET_XMM128;
      లేకపోతే
	తిరిగి FFI_BAD_TYPEDEF;
      ఆపు;
    స్థితి FFI_TYPE_COMPLEX:
      ఎంపిక (rtype->elements[0]->type)
	{
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((గుర్తులేని) rtype_కొలత << UNIX64_కొలత_జరుపు);
	  ఆపు;
	స్థితి FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  ఆపు;
	స్థితి FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_కొలత_జరుపు);
	  ఆపు;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
#ఒకవేళ FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_లోపల_MEM;
#లేకపోతే
	  flags = UNIX64_RET_X87_2;
#అయితే_ముగింపు
	  ఆపు;
#అయితే_ముగింపు
	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_లోపల_MEM;
	  ఆపు;
	అప్రమేయ:
	  తిరిగి FFI_BAD_TYPEDEF;
	}
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  కొరకు (i = 0, avn = cif->nargs; i < avn; i++)
    ఒకవేళ (cif->arg_types[i]->type == FFI_TYPE_సదిశ
	&& cif->arg_types[i]->కొలత > 16)
      తిరిగి FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  కొరకు (బైట్లు = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      ఒకవేళ (examine_వాదం (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  దీర్ఘ align = cif->arg_types[i]->alignment;

	  ఒకవేళ (align < 8)
	    align = 8;

	  బైట్లు = FFI_ALIGN (బైట్లు, align);
	  బైట్లు += cif->arg_types[i]->కొలత;
	}
      లేకపోతే
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  ఒకవేళ (ssecount)
    flags |= UNIX64_FLAG_XMM_వాదనలు;

  cif->flags = flags;
  cif->బైట్లు = (గుర్తులేని) FFI_ALIGN (బైట్లు, 8);

  తిరిగి FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  గణన x86_64_reg_class classes[MAX_CLASSES];
  అక్షరం *దొంతర, *argp;
  ffi_type **arg_types;
  పూర్ణాంకం gprcount, ssecount, ngpr, nsse, i, avn, flags;
  నిర్మాణం రిజిస్టరు_వాదనలు *reg_వాదనలు;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ (flags & UNIX64_FLAG_RET_లోపల_MEM)
	rvalue = alloca (cif->rtype->కొలత);
      లేకపోతే
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  దొంతర = alloca (పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు) + cif->బైట్లు + 4*8);
  reg_వాదనలు = (నిర్మాణం రిజిస్టరు_వాదనలు *) దొంతర;
  argp = దొంతర + పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు);

  reg_వాదనలు->r10 = (uintptr_t) సంవృతం;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  ఒకవేళ (flags & UNIX64_FLAG_RET_లోపల_MEM)
    reg_వాదనలు->gpr[gprcount++] = (గుర్తులేని దీర్ఘ) rvalue;

  కొరకు (i = 0; i < avn; ++i)
    {
      size_t n, కొలత = arg_types[i]->కొలత;

      n = examine_వాదం (arg_types[i], classes, 0, &ngpr, &nsse);
      ఒకవేళ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  దీర్ఘ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ఒకవేళ (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (శూన్య *) FFI_ALIGN (argp, align);
          జ్ఞాపకం_నకలు (argp, avalue[i], కొలత);

          argp += కొలత;
        }
      లేకపోతే
	{
	  /* The argument is passed entirely in registers.  */
	  అక్షరం *a = (అక్షరం *) avalue[i];
	  గుర్తులేని పూర్ణాంకం j;

	  కొరకు (j = 0; j < n; j++, a += 8, కొలత -= 8)
	    {
	      ఎంపిక (classes[j])
		{
		స్థితి X86_64_NO_CLASS:
		  ఆపు;
		స్థితి X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  జ్ఞాపకం_నకలు ((అక్షరం *) &reg_వాదనలు->sse[ssecount - 1] + 8, a,
			  కొలత < 8 ? కొలత : 8);
		  ఆపు;
		స్థితి X86_64_INTEGER_CLASS:
		స్థితి X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  ఎంపిక (arg_types[i]->type)
		    {
		    స్థితి FFI_TYPE_SINT8:
		      reg_వాదనలు->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      ఆపు;
		    స్థితి FFI_TYPE_SINT16:
		      reg_వాదనలు->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      ఆపు;
		    స్థితి FFI_TYPE_SINT32:
		      reg_వాదనలు->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      ఆపు;
		    అప్రమేయ:
		      reg_వాదనలు->gpr[gprcount] = 0;
		      జ్ఞాపకం_నకలు (&reg_వాదనలు->gpr[gprcount], a, కొలత <= 8 ? కొలత : 8);
		    }
		  gprcount++;
		  ఆపు;
		స్థితి X86_64_SSE_CLASS:
		స్థితి X86_64_SSEDF_CLASS:
		  జ్ఞాపకం_నకలు (&reg_వాదనలు->sse[ssecount++].i64, a, పరిమాణం(UINT64));
		  ఆపు;
		స్థితి X86_64_SSESF_CLASS:
		  జ్ఞాపకం_నకలు (&reg_వాదనలు->sse[ssecount++].i32, a, పరిమాణం(UINT32));
		  ఆపు;
		అప్రమేయ:
		  abort();
		}
	    }
	}
    }
  reg_వాదనలు->rax = ssecount;

  ffi_call_unix64 (దొంతర, cif->బైట్లు + పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు),
		   flags, rvalue, fn);
}

#నిర్వచితం_కాకపోతే __ILP32__
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

గణన ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_దొంతర                               /* copy len bytes -> stack       */
};

రకనిర్వచనం నిర్మాణం
{
  గుర్తులేని src_idx;     /* avalue[] index                                  */
  గుర్తులేని src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  గుర్తులేని dst_off;     /* byte offset within the register_args+stack buf  */
  గుర్తులేని len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  గుర్తులేని అక్షరం op;
} ffi_move;

రకనిర్వచనం నిర్మాణం
{
  గుర్తులేని nmoves;
  గుర్తులేని ssecount;    /* -> reg_args->rax                                */
  గుర్తులేని బైట్లు;       /* stack-arg area size (== cif->bytes)             */
  గుర్తులేని flags;       /* == cif->flags                                   */
  గుర్తులేని ret_లోపల_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  గుర్తులేని fast;        /* nonzero -> lean trampoline eligible             */
  గుర్తులేని retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  పూర్ణాంకం      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  గుర్తులేని alloc_బైట్లు; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move కదలికలు[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
నిర్మాణం ffi_ret2 { UINT64 i; ద్విగుణ d; };
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_fast_call (నిర్మాణం రిజిస్టరు_వాదనలు *img,
					   శూన్య (*fn) (శూన్య)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp0 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp1 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp2 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp3 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp4 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp5 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
బాహ్య నిర్మాణం ffi_ret2 ffi_plan_gp6 (శూన్య **, శూన్య (*)(శూన్య)) FFI_HIDDEN;
స్థిర నిర్మాణం ffi_ret2 (*స్థిరరాశి ffi_gp_thunks[7]) (శూన్య **, శూన్య (*)(శూన్య)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
స్థిర అంతఃవరుస శూన్య
store_ret (శూన్య *rvalue, గుర్తులేని retcode, నిర్మాణం ffi_ret2 r)
{
  ఎంపిక (retcode)
    {
    స్థితి UNIX64_RET_VOID:   ఆపు;
    స్థితి UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; ఆపు;
    స్థితి UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; ఆపు;
    స్థితి UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; ఆపు;
    స్థితి UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; ఆపు;
    స్థితి UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; ఆపు;
    స్థితి UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; ఆపు;
    స్థితి UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; ఆపు;
    స్థితి UNIX64_RET_XMM32:  జ్ఞాపకం_నకలు (rvalue, &r.d, 4); ఆపు;
    స్థితి UNIX64_RET_XMM64:  జ్ఞాపకం_నకలు (rvalue, &r.d, 8); ఆపు;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
స్థిర ffi_plan *
build_plan (ffi_cif *cif)
{
  గుర్తులేని i, avn = cif->nargs;
  గణన x86_64_reg_class classes[MAX_CLASSES];
  గుర్తులేని nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  పూర్ణాంకం all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  ఒకవేళ (cif->abi != FFI_UNIX64)
    తిరిగి ఖాళీ;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  కొరకు (i = 0; i < avn; i++)
    {
      పూర్ణాంకం t = cif->arg_types[i]->type;
      ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	తిరిగి ఖాళీ;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ఒకవేళ (t == FFI_TYPE_LONGDOUBLE)
	తిరిగి ఖాళీ;
#అయితే_ముగింపు
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = పరిమాణం (ffi_plan) + పరిమాణం (ffi_move) * (2 * avn + 1);
  plan = జ్ఞాపకం_ఇవ్వు (nbytes);
  ఒకవేళ (plan == ఖాళీ)
    తిరిగి ఖాళీ;
  plan->alloc_బైట్లు = (గుర్తులేని) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_లోపల_mem = (cif->flags & UNIX64_FLAG_RET_లోపల_MEM) ? 1 : 0;
  ఒకవేళ (plan->ret_లోపల_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  కొరకు (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t కొలత = at->కొలత, n, rem;
      పూర్ణాంకం ngpr, nsse;
      గుర్తులేని j;

      n = examine_వాదం (at, classes, 0, &ngpr, &nsse);
      ఒకవేళ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  దీర్ఘ align = at->alignment;
	  ffi_move *m = &plan->కదలికలు[nm++];
	  all_gp64 = 0;
	  ఒకవేళ (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_దొంతర;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (గుర్తులేని) (పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు) + argp_off);
	  m->len = (గుర్తులేని) కొలత;
	  argp_off += కొలత;
	  కొనసాగు;
	}

      కొరకు (j = 0, rem = కొలత; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  ఎంపిక (classes[j])
	    {
	    స్థితి X86_64_NO_CLASS:
	    స్థితి X86_64_SSEUP_CLASS:
	      కొనసాగు;			/* nothing placed for this 8-byte */
	    స్థితి X86_64_INTEGER_CLASS:
	    స్థితి X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      ఎంపిక (at->type)
		{
		స్థితి FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; ఆపు;
		స్థితి FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; ఆపు;
		స్థితి FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; ఆపు;
		అప్రమేయ:
		  ఒకవేళ (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  లేకపోతే
		    { m.op = FFI_MOVE_GP; m.len = (గుర్తులేని) rem; all_gp64 = 0; }
		  ఆపు;
		}
	      gprcount++;
	      ఆపు;
	    స్థితి X86_64_SSE_CLASS:
	    స్థితి X86_64_SSEDF_CLASS:
	      m.dst_off = (గుర్తులేని) (offsetof (నిర్మాణం రిజిస్టరు_వాదనలు, sse)
				      + ssecount * పరిమాణం (సంఘం big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      ఆపు;
	    స్థితి X86_64_SSESF_CLASS:
	      m.dst_off = (గుర్తులేని) (offsetof (నిర్మాణం రిజిస్టరు_వాదనలు, sse)
				      + ssecount * పరిమాణం (సంఘం big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      ఆపు;
	    అప్రమేయ:
	      విడుదల (plan);		/* X87 etc. in registers: bail */
	      తిరిగి ఖాళీ;
	    }
	  plan->కదలికలు[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->బైట్లు = cif->బైట్లు;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->బైట్లు == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_లోపల_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (పూర్ణాంకం) avn : -1;
  తిరిగి plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
స్థిర అంతఃవరుస __attribute__ ((always_inline)) శూన్య
plan_exec (ffi_cif *cif, ffi_plan *plan, శూన్య (*fn) (శూన్య),
	   శూన్య *rvalue, శూన్య **avalue)
{
  గుర్తులేని flags = plan->flags;
  నిర్మాణం రిజిస్టరు_వాదనలు local __attribute__ ((aligned (16)));
  అక్షరం *దొంతర = ఖాళీ;
  నిర్మాణం రిజిస్టరు_వాదనలు *reg_వాదనలు;
  గుర్తులేని k;

  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ (flags & UNIX64_FLAG_RET_లోపల_MEM)
	rvalue = alloca (cif->rtype->కొలత);
      లేకపోతే
	flags = UNIX64_RET_VOID;
    }

  ఒకవేళ (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      నిర్మాణం ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      ఒకవేళ (rvalue != ఖాళీ)
	store_ret (rvalue, plan->retcode, r);
      తిరిగి;
    }

  ఒకవేళ (plan->fast)
    reg_వాదనలు = &local;			/* no stack args: fixed local image */
  లేకపోతే
    {
      దొంతర = alloca (పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు) + plan->బైట్లు + 4 * 8);
      reg_వాదనలు = (నిర్మాణం రిజిస్టరు_వాదనలు *) దొంతర;
    }
  reg_వాదనలు->r10 = 0;			/* closure (none for ffi_call) */
  ఒకవేళ (plan->ret_లోపల_mem)
    reg_వాదనలు->gpr[0] = (UINT64) (uintptr_t) rvalue;

  కొరకు (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->కదలికలు[k];
      అక్షరం *src = (అక్షరం *) avalue[m->src_idx] + m->src_off;
      అక్షరం *dst = (అక్షరం *) reg_వాదనలు + m->dst_off;
      ఎంపిక (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	స్థితి FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; ఆపు;
	స్థితి FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; ఆపు;
	స్థితి FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; ఆపు;
	స్థితి FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   ఆపు;
	స్థితి FFI_MOVE_GP:    *(UINT64 *) dst = 0; జ్ఞాపకం_నకలు (dst, src, m->len);     ఆపు;
	స్థితి FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   ఆపు;
	స్థితి FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   ఆపు;
	స్థితి FFI_MOVE_దొంతర: జ్ఞాపకం_నకలు (dst, src, m->len);                          ఆపు;
	}
    }
  reg_వాదనలు->rax = plan->ssecount;

  ఒకవేళ (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      నిర్మాణం ffi_ret2 r = ffi_plan_fast_call (reg_వాదనలు, fn);
      ఒకవేళ (rvalue != ఖాళీ)
	store_ret (rvalue, plan->retcode, r);
      తిరిగి;
    }

  ffi_call_unix64 (దొంతర, plan->బైట్లు + పరిమాణం (నిర్మాణం రిజిస్టరు_వాదనలు),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
నిర్మాణం ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = జ్ఞాపకం_ఇవ్వు (పరిమాణం (నిర్మాణం ffi_call_plan));
  ఒకవేళ (plan == ఖాళీ)
    తిరిగి ఖాళీ;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  తిరిగి plan;
}

శూన్య
ffi_call_plan_invoke (ffi_call_plan *plan, శూన్య (*fn) (శూన్య),
		      శూన్య *rvalue, శూన్య **avalue)
{
  ఒకవేళ (plan->fast != ఖాళీ)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  లేకపోతే
    ffi_call (plan->cif, fn, rvalue, avalue);
}

శూన్య
ffi_call_plan_free (ffi_call_plan *plan)
{
  ఒకవేళ (plan != ఖాళీ)
    {
      విడుదల (plan->fast);
      విడుదల (plan);
    }
}

size_t
ffi_call_plan_కొలత (ffi_call_plan *plan)
{
  ఒకవేళ (plan == ఖాళీ)
    తిరిగి 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  తిరిగి పరిమాణం (నిర్మాణం ffi_call_plan)
	 + (plan->fast != ఖాళీ ? plan->fast->alloc_బైట్లు : 0);
}

బాహ్య శూన్య
ffi_call_efi64(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue);
#అయితే_ముగింపు

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;
  పూర్ణాంకం i, nargs = cif->nargs;
  స్థిరరాశి పూర్ణాంకం max_reg_struct_కొలత = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT && కొలత > max_reg_struct_కొలత)
        {
          అక్షరం *argcopy = alloca (కొలత);
          ఒకవేళ (avalue_copy == ఖాళీ)
            {
              avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
              జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
              avalue = avalue_copy;
            }
          జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
          avalue[i] = argcopy;
        }
    }

#నిర్వచితం_కాకపోతే __ILP32__
  ఒకవేళ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      తిరిగి;
    }
#అయితే_ముగింపు
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

#నిర్వచితమైతే FFI_GO_CLOSURES

#నిర్వచితం_కాకపోతే __ILP32__
బాహ్య శూన్య
ffi_call_go_efi64(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
		  శూన్య **avalue, శూన్య *సంవృతం);
#అయితే_ముగింపు

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
#నిర్వచితం_కాకపోతే __ILP32__
  ఒకవేళ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, సంవృతం);
      తిరిగి;
    }
#అయితే_ముగింపు
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}

#అయితే_ముగింపు /* FFI_GO_CLOSURES */

బాహ్య శూన్య ffi_సంవృతం_unix64(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_సంవృతం_unix64_sse(శూన్య) FFI_HIDDEN;
#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బాహ్య శూన్య ffi_సంవృతం_unix64_alt(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_సంవృతం_unix64_sse_alt(శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __ILP32__
బాహ్య ffi_స్థితి
ffi_prep_సంవృతం_loc_efi64(ffi_సంవృతం* సంవృతం,
			   ffi_cif* cif,
			   శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			   శూన్య *user_దత్తాంశం,
			   శూన్య *codeloc);
#అయితే_ముగింపు

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  శూన్య (*dest)(శూన్య);
  అక్షరం *tramp = సంవృతం->tramp;

#నిర్వచితం_కాకపోతే __ILP32__
  ఒకవేళ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    తిరిగి ffi_prep_సంవృతం_loc_efi64(సంవృతం, cif, fun, user_దత్తాంశం, codeloc);
#అయితే_ముగింపు
  ఒకవేళ (cif->abi != FFI_UNIX64)
    తిరిగి FFI_BAD_ABI;

  ఒకవేళ (cif->flags & UNIX64_FLAG_XMM_వాదనలు)
    dest = ffi_సంవృతం_unix64_sse;
  లేకపోతే
    dest = ffi_సంవృతం_unix64;

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      ఒకవేళ (dest == ffi_సంవృతం_unix64_sse)
        dest = ffi_సంవృతం_unix64_sse_alt;
      లేకపోతే
        dest = ffi_సంవృతం_unix64_alt;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
#అయితే_ముగింపు

  /* Initialize the dynamic trampoline. */
  జ్ఞాపకం_నకలు (tramp, trampoline, పరిమాణం(trampoline));
  *(UINT64 *)(tramp + పరిమాణం (trampoline)) = (uintptr_t)dest;

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_unix64_inner(ffi_cif *cif,
			 శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			 శూన్య *user_దత్తాంశం,
			 శూన్య *rvalue,
			 నిర్మాణం రిజిస్టరు_వాదనలు *reg_వాదనలు,
			 అక్షరం *argp)
{
  శూన్య **avalue;
  ffi_type **arg_types;
  దీర్ఘ i, avn;
  పూర్ణాంకం gprcount, ssecount, ngpr, nsse;
  పూర్ణాంకం flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * పరిమాణం(శూన్య *));
  gprcount = ssecount = 0;

  ఒకవేళ (flags & UNIX64_FLAG_RET_లోపల_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      శూన్య *r = (శూన్య *)(uintptr_t)reg_వాదనలు->gpr[gprcount++];
      *(శూన్య **)rvalue = r;
      rvalue = r;
      flags = (పరిమాణం(శూన్య *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  కొరకు (i = 0; i < avn; ++i)
    {
      గణన x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_వాదం (arg_types[i], classes, 0, &ngpr, &nsse);
      ఒకవేళ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  దీర్ఘ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ఒకవేళ (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (శూన్య *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->కొలత;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      లేకపోతే ఒకవేళ (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  ఒకవేళ (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_వాదనలు->sse[ssecount];
	      ssecount += n;
	    }
	  లేకపోతే
	    {
	      avalue[i] = &reg_వాదనలు->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      లేకపోతే
	{
	  అక్షరం *a = alloca (n * 8);
	  గుర్తులేని పూర్ణాంకం j;

	  avalue[i] = a;
	  కొరకు (j = 0; j < n; j++, a += 8)
	    {
	      ఒకవేళ (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		జ్ఞాపకం_నకలు (a, (అక్షరం *) &reg_వాదనలు->sse[ssecount - 1] + 8, 8);
	      లేకపోతే ఒకవేళ (SSE_CLASS_P (classes[j]))
		జ్ఞాపకం_నకలు (a, &reg_వాదనలు->sse[ssecount++], 8);
	      లేకపోతే
		జ్ఞాపకం_నకలు (a, &reg_వాదనలు->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell assembly how to perform return type promotions.  */
  తిరిగి flags;
}

#నిర్వచితమైతే FFI_GO_CLOSURES

బాహ్య శూన్య ffi_go_సంవృతం_unix64(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_go_సంవృతం_unix64_sse(శూన్య) FFI_HIDDEN;

#నిర్వచితం_కాకపోతే __ILP32__
బాహ్య ffi_స్థితి
ffi_prep_go_సంవృతం_efi64(ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
			  శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*));
#అయితే_ముగింపు

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
#నిర్వచితం_కాకపోతే __ILP32__
  ఒకవేళ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    తిరిగి ffi_prep_go_సంవృతం_efi64(సంవృతం, cif, fun);
#అయితే_ముగింపు
  ఒకవేళ (cif->abi != FFI_UNIX64)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = (cif->flags & UNIX64_FLAG_XMM_వాదనలు
		    ? ffi_go_సంవృతం_unix64_sse
		    : ffi_go_సంవృతం_unix64);
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

#అయితే_ముగింపు /* FFI_GO_CLOSURES */

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *map_కొలత = UNIX64_TRAMP_MAP_కొలత;
  *tramp_కొలత = UNIX64_TRAMP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* __x86_64__ */

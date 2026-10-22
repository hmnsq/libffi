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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdarg.h>
#ضمن <stdint.h>
#ضمن <stddef.h>
#ضمن <string.h>
#ضمن <tramp.h>
#ضمن "internal64.h"

#جيڪڏهن_وصف_ٿيل __x86_64__

#وصف_ڪريو MAX_GPR_REGS 6
#وصف_ڪريو MAX_SSE_REGS 8

#إذا defined(__INTEL_COMPILER)
#ضمن "xmmintrin.h"
#وصف_ڪريو UINT128 __m128
#وإلا
#إذا defined(__SUNPRO_C)
#ضمن <sunmedia_types.h>
#وصف_ڪريو UINT128 __m128i
#وإلا
#وصف_ڪريو UINT128 __int128_t
#جيڪڏهن_پورو
#جيڪڏهن_پورو

اتحاد big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

بنية رجسٽر_دليلون
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  اتحاد big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

خارجي فراغ ffi_call_unix64 (فراغ *دليلون, بنانشان ڊگهو بائيٽون, بنانشان flags,
			     فراغ *raddr, فراغ (*fnaddr)(فراغ)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
تعداد x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87_مٿي_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_يادگيري_CLASS
  };

#وصف_ڪريو MAX_CLASSES 4

#وصف_ڪريو SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#وإلا
# define FFI_LONGDOUBLE_BINARY128 0
#جيڪڏهن_پورو

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ساكن تعداد x86_64_reg_class
ملايو_classes (تعداد x86_64_reg_class class1, تعداد x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  إذا (class1 == class2)
    أرجع class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  إذا (class1 == X86_64_NO_CLASS)
    أرجع class2;
  إذا (class2 == X86_64_NO_CLASS)
    أرجع class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  إذا (class1 == X86_64_يادگيري_CLASS || class2 == X86_64_يادگيري_CLASS)
    أرجع X86_64_يادگيري_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  إذا ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    أرجع X86_64_INTEGERSI_CLASS;
  إذا (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    أرجع X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  إذا (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87_مٿي_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87_مٿي_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    أرجع X86_64_يادگيري_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  أرجع X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ساكن size_t
classify_دليل (ffi_type *type, تعداد x86_64_reg_class classes[],
		   size_t byte_وٿي)
{
  بدل (type->type)
    {
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT128:
    حالة FFI_TYPE_SINT128:
    حالة FFI_TYPE_اشارو:
    do_integer:
      {
	size_t سائيز = byte_وٿي + type->سائيز;

	إذا (سائيز <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    أرجع 1;
	  }
	وإلا إذا (سائيز <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    أرجع 1;
	  }
	وإلا إذا (سائيز <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    أرجع 2;
	  }
	وإلا إذا (سائيز <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    أرجع 2;
	  }
	وإلا
	  FFI_ASSERT (0);
      }
    حالة FFI_TYPE_FLOAT:
      إذا (!(byte_وٿي % 8))
	classes[0] = X86_64_SSESF_CLASS;
      وإلا
	classes[0] = X86_64_SSE_CLASS;
      أرجع 1;
    حالة FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      أرجع 1;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
#إذا FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#وإلا
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87_مٿي_CLASS;
#جيڪڏهن_پورو
      أرجع 2;
#جيڪڏهن_پورو
    حالة FFI_TYPE_STRUCT:
      {
	ثابت size_t UNITS_في_لفظ = 8;
        size_t لفظ = (type->سائيز + byte_وٿي + UNITS_في_لفظ - 1)
                       / UNITS_في_لفظ;
	ffi_type **ptr;
	بنانشان صحيح i;
	تعداد x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	إذا (type->سائيز > 32)
	  أرجع 0;

	لكل (i = 0; i < لفظ; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	إذا (!لفظ)
	  {
    حالة FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    أرجع 1;
	  }

	/* Merge the fields of structure.  */
	لكل (ptr = type->elements; *ptr != عدم; ptr++)
	  {
	    size_t انگ, pos;

	    byte_وٿي = FFI_ALIGN (byte_وٿي, (*ptr)->alignment);

	    انگ = classify_دليل (*ptr, subclasses, byte_وٿي % 8);
	    إذا (انگ == 0)
	      أرجع 0;
            pos = byte_وٿي / 8;
            لكل (i = 0; i < انگ && (i + pos) < لفظ; i++)
	      {
		size_t pos = byte_وٿي / 8;
		classes[i + pos] =
		  ملايو_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_وٿي += (*ptr)->سائيز;
	  }

	إذا (لفظ > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    إذا (classes[0] != X86_64_SSE_CLASS)
	      أرجع 0;

	    لكل (i = 1; i < لفظ; i++)
	      إذا (classes[i] != X86_64_SSEUP_CLASS)
		أرجع 0;
	  }

	/* Final merger cleanup.  */
	لكل (i = 0; i < لفظ; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    إذا (classes[i] == X86_64_يادگيري_CLASS)
	      أرجع 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    إذا (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    إذا (i > 1 && classes[i] == X86_64_X87_مٿي_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		أرجع 0;
	      }
	  }
	أرجع لفظ;
      }
    حالة FFI_TYPE_ويڪٽر:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      إذا (type->سائيز == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  أرجع 1;
	}
      وإلا إذا (type->سائيز == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  أرجع 2;
	}
      أرجع 0;
    حالة FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	بدل (inner->type)
	  {
	  حالة FFI_TYPE_INT:
	  حالة FFI_TYPE_UINT8:
	  حالة FFI_TYPE_SINT8:
	  حالة FFI_TYPE_UINT16:
	  حالة FFI_TYPE_SINT16:
	  حالة FFI_TYPE_UINT32:
	  حالة FFI_TYPE_SINT32:
	  حالة FFI_TYPE_UINT64:
	  حالة FFI_TYPE_SINT64:
	    اذهب do_integer;

	  حالة FFI_TYPE_SINT128:
	  حالة FFI_TYPE_UINT128:
	    أرجع 0;

	  حالة FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    إذا (byte_وٿي % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		أرجع 2;
	      }
	    أرجع 1;
	  حالة FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    أرجع 2;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  حالة FFI_TYPE_LONGDOUBLE:
#إذا FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    أرجع 0;
#وإلا
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    أرجع 1;
#جيڪڏهن_پورو
#جيڪڏهن_پورو
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

ساكن size_t
examine_دليل (ffi_type *type, تعداد x86_64_reg_class classes[MAX_CLASSES],
		  _Bool اندريون_return, صحيح *pngpr, صحيح *pnsse)
{
  size_t n;
  بنانشان صحيح i;
  صحيح ngpr, nsse;

  n = classify_دليل (type, classes, 0);
  إذا (n == 0)
    أرجع 0;

  ngpr = nsse = 0;
  لكل (i = 0; i < n; ++i)
    بدل (classes[i])
      {
      حالة X86_64_INTEGER_CLASS:
      حالة X86_64_INTEGERSI_CLASS:
	ngpr++;
	اكسر;
      حالة X86_64_SSE_CLASS:
      حالة X86_64_SSESF_CLASS:
      حالة X86_64_SSEDF_CLASS:
	nsse++;
	اكسر;
      حالة X86_64_NO_CLASS:
      حالة X86_64_SSEUP_CLASS:
	اكسر;
      حالة X86_64_X87_CLASS:
      حالة X86_64_X87_مٿي_CLASS:
      حالة X86_64_COMPLEX_X87_CLASS:
	أرجع اندريون_return != 0;
      افتراضي:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  أرجع n;
}

/* Perform machine dependent cif processing.  */

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
خارجي ffi_حالت
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#جيڪڏهن_پورو

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحيح gprcount, ssecount, i, avn, ngpr, nsse;
  بنانشان flags;
  تعداد x86_64_reg_class classes[MAX_CLASSES];
  size_t بائيٽون, n, rtype_سائيز;
  ffi_type *rtype;

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
  إذا (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    أرجع ffi_prep_cif_machdep_efi64(cif);
#جيڪڏهن_پورو
  إذا (cif->abi != FFI_UNIX64)
    أرجع FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_سائيز = rtype->سائيز;
  بدل (rtype->type)
    {
    حالة FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      اكسر;
    حالة FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      اكسر;
    حالة FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      اكسر;
    حالة FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      اكسر;
    حالة FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      اكسر;
    حالة FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      اكسر;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      اكسر;
    حالة FFI_TYPE_UINT128:
    حالة FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_سائيز_سرڪايو);
      اكسر;
    حالة FFI_TYPE_اشارو:
      flags = (حجم(فراغ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      اكسر;
    حالة FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      اكسر;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
#إذا FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#وإلا
      flags = UNIX64_RET_X87;
#جيڪڏهن_پورو
      اكسر;
#جيڪڏهن_پورو
    حالة FFI_TYPE_STRUCT:
      n = examine_دليل (cif->rtype, classes, 1, &ngpr, &nsse);
      إذا (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندريون_MEM;
	}
      وإلا
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  إذا (rtype_سائيز == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  وإلا إذا (rtype_سائيز == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  وإلا
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      إذا (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      وإلا إذا (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      وإلا إذا (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      وإلا
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_سائيز << UNIX64_سائيز_سرڪايو;
	    }
	}
      اكسر;
    حالة FFI_TYPE_ويڪٽر:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      إذا (rtype_سائيز == 8)
	flags = UNIX64_RET_XMM64;
      وإلا إذا (rtype_سائيز == 16)
	flags = UNIX64_RET_XMM128;
      وإلا
	أرجع FFI_BAD_TYPEDEF;
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      بدل (rtype->elements[0]->type)
	{
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((بنانشان) rtype_سائيز << UNIX64_سائيز_سرڪايو);
	  اكسر;
	حالة FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_سائيز_سرڪايو);
	  اكسر;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
#إذا FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندريون_MEM;
#وإلا
	  flags = UNIX64_RET_X87_2;
#جيڪڏهن_پورو
	  اكسر;
#جيڪڏهن_پورو
	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندريون_MEM;
	  اكسر;
	افتراضي:
	  أرجع FFI_BAD_TYPEDEF;
	}
      اكسر;
    افتراضي:
      أرجع FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  لكل (i = 0, avn = cif->nargs; i < avn; i++)
    إذا (cif->arg_types[i]->type == FFI_TYPE_ويڪٽر
	&& cif->arg_types[i]->سائيز > 16)
      أرجع FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  لكل (بائيٽون = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      إذا (examine_دليل (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ڊگهو align = cif->arg_types[i]->alignment;

	  إذا (align < 8)
	    align = 8;

	  بائيٽون = FFI_ALIGN (بائيٽون, align);
	  بائيٽون += cif->arg_types[i]->سائيز;
	}
      وإلا
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  إذا (ssecount)
    flags |= UNIX64_FLAG_XMM_دليلون;

  cif->flags = flags;
  cif->بائيٽون = (بنانشان) FFI_ALIGN (بائيٽون, 8);

  أرجع FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گهيرو)
{
  تعداد x86_64_reg_class classes[MAX_CLASSES];
  حرف *ڍير, *argp;
  ffi_type **arg_types;
  صحيح gprcount, ssecount, ngpr, nsse, i, avn, flags;
  بنية رجسٽر_دليلون *reg_دليلون;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  إذا (rvalue == عدم)
    {
      إذا (flags & UNIX64_FLAG_RET_اندريون_MEM)
	rvalue = alloca (cif->rtype->سائيز);
      وإلا
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ڍير = alloca (حجم (بنية رجسٽر_دليلون) + cif->بائيٽون + 4*8);
  reg_دليلون = (بنية رجسٽر_دليلون *) ڍير;
  argp = ڍير + حجم (بنية رجسٽر_دليلون);

  reg_دليلون->r10 = (uintptr_t) گهيرو;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  إذا (flags & UNIX64_FLAG_RET_اندريون_MEM)
    reg_دليلون->gpr[gprcount++] = (بنانشان ڊگهو) rvalue;

  لكل (i = 0; i < avn; ++i)
    {
      size_t n, سائيز = arg_types[i]->سائيز;

      n = examine_دليل (arg_types[i], classes, 0, &ngpr, &nsse);
      إذا (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ڊگهو align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  إذا (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (فراغ *) FFI_ALIGN (argp, align);
          يادگيري_نقل (argp, avalue[i], سائيز);

          argp += سائيز;
        }
      وإلا
	{
	  /* The argument is passed entirely in registers.  */
	  حرف *a = (حرف *) avalue[i];
	  بنانشان صحيح j;

	  لكل (j = 0; j < n; j++, a += 8, سائيز -= 8)
	    {
	      بدل (classes[j])
		{
		حالة X86_64_NO_CLASS:
		  اكسر;
		حالة X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  يادگيري_نقل ((حرف *) &reg_دليلون->sse[ssecount - 1] + 8, a,
			  سائيز < 8 ? سائيز : 8);
		  اكسر;
		حالة X86_64_INTEGER_CLASS:
		حالة X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  بدل (arg_types[i]->type)
		    {
		    حالة FFI_TYPE_SINT8:
		      reg_دليلون->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      اكسر;
		    حالة FFI_TYPE_SINT16:
		      reg_دليلون->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      اكسر;
		    حالة FFI_TYPE_SINT32:
		      reg_دليلون->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      اكسر;
		    افتراضي:
		      reg_دليلون->gpr[gprcount] = 0;
		      يادگيري_نقل (&reg_دليلون->gpr[gprcount], a, سائيز <= 8 ? سائيز : 8);
		    }
		  gprcount++;
		  اكسر;
		حالة X86_64_SSE_CLASS:
		حالة X86_64_SSEDF_CLASS:
		  يادگيري_نقل (&reg_دليلون->sse[ssecount++].i64, a, حجم(UINT64));
		  اكسر;
		حالة X86_64_SSESF_CLASS:
		  يادگيري_نقل (&reg_دليلون->sse[ssecount++].i32, a, حجم(UINT32));
		  اكسر;
		افتراضي:
		  abort();
		}
	    }
	}
    }
  reg_دليلون->rax = ssecount;

  ffi_call_unix64 (ڍير, cif->بائيٽون + حجم (بنية رجسٽر_دليلون),
		   flags, rvalue, fn);
}

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
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

تعداد ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ڍير                               /* copy len bytes -> stack       */
};

تعريفنوع بنية
{
  بنانشان src_idx;     /* avalue[] index                                  */
  بنانشان src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  بنانشان dst_off;     /* byte offset within the register_args+stack buf  */
  بنانشان len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  بنانشان حرف op;
} ffi_move;

تعريفنوع بنية
{
  بنانشان nmoves;
  بنانشان ssecount;    /* -> reg_args->rax                                */
  بنانشان بائيٽون;       /* stack-arg area size (== cif->bytes)             */
  بنانشان flags;       /* == cif->flags                                   */
  بنانشان ret_اندريون_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  بنانشان fast;        /* nonzero -> lean trampoline eligible             */
  بنانشان retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  صحيح      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  بنانشان alloc_بائيٽون; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move چالون[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
بنية ffi_ret2 { UINT64 i; مزدوج d; };
خارجي بنية ffi_ret2 ffi_plan_fast_call (بنية رجسٽر_دليلون *img,
					   فراغ (*fn) (فراغ)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
خارجي بنية ffi_ret2 ffi_plan_gp0 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp1 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp2 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp3 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp4 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp5 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
خارجي بنية ffi_ret2 ffi_plan_gp6 (فراغ **, فراغ (*)(فراغ)) FFI_HIDDEN;
ساكن بنية ffi_ret2 (*ثابت ffi_gp_thunks[7]) (فراغ **, فراغ (*)(فراغ)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ساكن مضمن فراغ
store_ret (فراغ *rvalue, بنانشان retcode, بنية ffi_ret2 r)
{
  بدل (retcode)
    {
    حالة UNIX64_RET_VOID:   اكسر;
    حالة UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; اكسر;
    حالة UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; اكسر;
    حالة UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; اكسر;
    حالة UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; اكسر;
    حالة UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; اكسر;
    حالة UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; اكسر;
    حالة UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; اكسر;
    حالة UNIX64_RET_XMM32:  يادگيري_نقل (rvalue, &r.d, 4); اكسر;
    حالة UNIX64_RET_XMM64:  يادگيري_نقل (rvalue, &r.d, 8); اكسر;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ساكن ffi_plan *
build_plan (ffi_cif *cif)
{
  بنانشان i, avn = cif->nargs;
  تعداد x86_64_reg_class classes[MAX_CLASSES];
  بنانشان nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  صحيح all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  إذا (cif->abi != FFI_UNIX64)
    أرجع عدم;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  لكل (i = 0; i < avn; i++)
    {
      صحيح t = cif->arg_types[i]->type;
      إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	أرجع عدم;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      إذا (t == FFI_TYPE_LONGDOUBLE)
	أرجع عدم;
#جيڪڏهن_پورو
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = حجم (ffi_plan) + حجم (ffi_move) * (2 * avn + 1);
  plan = يادگيري_ڏيو (nbytes);
  إذا (plan == عدم)
    أرجع عدم;
  plan->alloc_بائيٽون = (بنانشان) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_اندريون_mem = (cif->flags & UNIX64_FLAG_RET_اندريون_MEM) ? 1 : 0;
  إذا (plan->ret_اندريون_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  لكل (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t سائيز = at->سائيز, n, rem;
      صحيح ngpr, nsse;
      بنانشان j;

      n = examine_دليل (at, classes, 0, &ngpr, &nsse);
      إذا (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ڊگهو align = at->alignment;
	  ffi_move *m = &plan->چالون[nm++];
	  all_gp64 = 0;
	  إذا (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ڍير;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (بنانشان) (حجم (بنية رجسٽر_دليلون) + argp_off);
	  m->len = (بنانشان) سائيز;
	  argp_off += سائيز;
	  استمر;
	}

      لكل (j = 0, rem = سائيز; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  بدل (classes[j])
	    {
	    حالة X86_64_NO_CLASS:
	    حالة X86_64_SSEUP_CLASS:
	      استمر;			/* nothing placed for this 8-byte */
	    حالة X86_64_INTEGER_CLASS:
	    حالة X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      بدل (at->type)
		{
		حالة FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; اكسر;
		حالة FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; اكسر;
		حالة FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; اكسر;
		افتراضي:
		  إذا (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  وإلا
		    { m.op = FFI_MOVE_GP; m.len = (بنانشان) rem; all_gp64 = 0; }
		  اكسر;
		}
	      gprcount++;
	      اكسر;
	    حالة X86_64_SSE_CLASS:
	    حالة X86_64_SSEDF_CLASS:
	      m.dst_off = (بنانشان) (offsetof (بنية رجسٽر_دليلون, sse)
				      + ssecount * حجم (اتحاد big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      اكسر;
	    حالة X86_64_SSESF_CLASS:
	      m.dst_off = (بنانشان) (offsetof (بنية رجسٽر_دليلون, sse)
				      + ssecount * حجم (اتحاد big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      اكسر;
	    افتراضي:
	      حرر (plan);		/* X87 etc. in registers: bail */
	      أرجع عدم;
	    }
	  plan->چالون[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->بائيٽون = cif->بائيٽون;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->بائيٽون == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_اندريون_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (صحيح) avn : -1;
  أرجع plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ساكن مضمن __attribute__ ((always_inline)) فراغ
plan_exec (ffi_cif *cif, ffi_plan *plan, فراغ (*fn) (فراغ),
	   فراغ *rvalue, فراغ **avalue)
{
  بنانشان flags = plan->flags;
  بنية رجسٽر_دليلون local __attribute__ ((aligned (16)));
  حرف *ڍير = عدم;
  بنية رجسٽر_دليلون *reg_دليلون;
  بنانشان k;

  إذا (rvalue == عدم)
    {
      إذا (flags & UNIX64_FLAG_RET_اندريون_MEM)
	rvalue = alloca (cif->rtype->سائيز);
      وإلا
	flags = UNIX64_RET_VOID;
    }

  إذا (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      بنية ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      إذا (rvalue != عدم)
	store_ret (rvalue, plan->retcode, r);
      أرجع;
    }

  إذا (plan->fast)
    reg_دليلون = &local;			/* no stack args: fixed local image */
  وإلا
    {
      ڍير = alloca (حجم (بنية رجسٽر_دليلون) + plan->بائيٽون + 4 * 8);
      reg_دليلون = (بنية رجسٽر_دليلون *) ڍير;
    }
  reg_دليلون->r10 = 0;			/* closure (none for ffi_call) */
  إذا (plan->ret_اندريون_mem)
    reg_دليلون->gpr[0] = (UINT64) (uintptr_t) rvalue;

  لكل (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->چالون[k];
      حرف *src = (حرف *) avalue[m->src_idx] + m->src_off;
      حرف *dst = (حرف *) reg_دليلون + m->dst_off;
      بدل (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	حالة FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; اكسر;
	حالة FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; اكسر;
	حالة FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; اكسر;
	حالة FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   اكسر;
	حالة FFI_MOVE_GP:    *(UINT64 *) dst = 0; يادگيري_نقل (dst, src, m->len);     اكسر;
	حالة FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   اكسر;
	حالة FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   اكسر;
	حالة FFI_MOVE_ڍير: يادگيري_نقل (dst, src, m->len);                          اكسر;
	}
    }
  reg_دليلون->rax = plan->ssecount;

  إذا (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      بنية ffi_ret2 r = ffi_plan_fast_call (reg_دليلون, fn);
      إذا (rvalue != عدم)
	store_ret (rvalue, plan->retcode, r);
      أرجع;
    }

  ffi_call_unix64 (ڍير, plan->بائيٽون + حجم (بنية رجسٽر_دليلون),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
بنية ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = يادگيري_ڏيو (حجم (بنية ffi_call_plan));
  إذا (plan == عدم)
    أرجع عدم;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  أرجع plan;
}

فراغ
ffi_call_plan_invoke (ffi_call_plan *plan, فراغ (*fn) (فراغ),
		      فراغ *rvalue, فراغ **avalue)
{
  إذا (plan->fast != عدم)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  وإلا
    ffi_call (plan->cif, fn, rvalue, avalue);
}

فراغ
ffi_call_plan_free (ffi_call_plan *plan)
{
  إذا (plan != عدم)
    {
      حرر (plan->fast);
      حرر (plan);
    }
}

size_t
ffi_call_plan_سائيز (ffi_call_plan *plan)
{
  إذا (plan == عدم)
    أرجع 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  أرجع حجم (بنية ffi_call_plan)
	 + (plan->fast != عدم ? plan->fast->alloc_بائيٽون : 0);
}

خارجي فراغ
ffi_call_efi64(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue);
#جيڪڏهن_پورو

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;
  صحيح i, nargs = cif->nargs;
  ثابت صحيح max_reg_struct_سائيز = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح سائيز = at->سائيز;
      إذا (at->type == FFI_TYPE_STRUCT && سائيز > max_reg_struct_سائيز)
        {
          حرف *argcopy = alloca (سائيز);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              يادگيري_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          يادگيري_نقل (argcopy, avalue[i], سائيز);
          avalue[i] = argcopy;
        }
    }

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
  إذا (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      أرجع;
    }
#جيڪڏهن_پورو
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
خارجي فراغ
ffi_call_go_efi64(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
		  فراغ **avalue, فراغ *گهيرو);
#جيڪڏهن_پورو

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
#جيڪڏهن_وصف_نه_ٿيل __ILP32__
  إذا (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, گهيرو);
      أرجع;
    }
#جيڪڏهن_پورو
  ffi_call_int (cif, fn, rvalue, avalue, گهيرو);
}

#جيڪڏهن_پورو /* FFI_GO_CLOSURES */

خارجي فراغ ffi_گهيرو_unix64(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_گهيرو_unix64_sse(فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
خارجي فراغ ffi_گهيرو_unix64_alt(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_گهيرو_unix64_sse_alt(فراغ) FFI_HIDDEN;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
خارجي ffi_حالت
ffi_prep_گهيرو_loc_efi64(ffi_گهيرو* گهيرو,
			   ffi_cif* cif,
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_معلومات,
			   فراغ *codeloc);
#جيڪڏهن_پورو

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  ساكن ثابت بنانشان حرف trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  فراغ (*dest)(فراغ);
  حرف *tramp = گهيرو->tramp;

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
  إذا (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    أرجع ffi_prep_گهيرو_loc_efi64(گهيرو, cif, fun, user_معلومات, codeloc);
#جيڪڏهن_پورو
  إذا (cif->abi != FFI_UNIX64)
    أرجع FFI_BAD_ABI;

  إذا (cif->flags & UNIX64_FLAG_XMM_دليلون)
    dest = ffi_گهيرو_unix64_sse;
  وإلا
    dest = ffi_گهيرو_unix64;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گهيرو))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (dest == ffi_گهيرو_unix64_sse)
        dest = ffi_گهيرو_unix64_sse_alt;
      وإلا
        dest = ffi_گهيرو_unix64_alt;
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, dest, گهيرو);
      اذهب ٻاهريون;
    }
#جيڪڏهن_پورو

  /* Initialize the dynamic trampoline. */
  يادگيري_نقل (tramp, trampoline, حجم(trampoline));
  *(UINT64 *)(tramp + حجم (trampoline)) = (uintptr_t)dest;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
ٻاهريون:
#جيڪڏهن_پورو
  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

صحيح FFI_HIDDEN
ffi_گهيرو_unix64_inner(ffi_cif *cif,
			 فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			 فراغ *user_معلومات,
			 فراغ *rvalue,
			 بنية رجسٽر_دليلون *reg_دليلون,
			 حرف *argp)
{
  فراغ **avalue;
  ffi_type **arg_types;
  ڊگهو i, avn;
  صحيح gprcount, ssecount, ngpr, nsse;
  صحيح flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * حجم(فراغ *));
  gprcount = ssecount = 0;

  إذا (flags & UNIX64_FLAG_RET_اندريون_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      فراغ *r = (فراغ *)(uintptr_t)reg_دليلون->gpr[gprcount++];
      *(فراغ **)rvalue = r;
      rvalue = r;
      flags = (حجم(فراغ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  لكل (i = 0; i < avn; ++i)
    {
      تعداد x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_دليل (arg_types[i], classes, 0, &ngpr, &nsse);
      إذا (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ڊگهو align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  إذا (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (فراغ *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->سائيز;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      وإلا إذا (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  إذا (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_دليلون->sse[ssecount];
	      ssecount += n;
	    }
	  وإلا
	    {
	      avalue[i] = &reg_دليلون->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      وإلا
	{
	  حرف *a = alloca (n * 8);
	  بنانشان صحيح j;

	  avalue[i] = a;
	  لكل (j = 0; j < n; j++, a += 8)
	    {
	      إذا (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		يادگيري_نقل (a, (حرف *) &reg_دليلون->sse[ssecount - 1] + 8, 8);
	      وإلا إذا (SSE_CLASS_P (classes[j]))
		يادگيري_نقل (a, &reg_دليلون->sse[ssecount++], 8);
	      وإلا
		يادگيري_نقل (a, &reg_دليلون->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_معلومات);

  /* Tell assembly how to perform return type promotions.  */
  أرجع flags;
}

#جيڪڏهن_وصف_ٿيل FFI_GO_CLOSURES

خارجي فراغ ffi_go_گهيرو_unix64(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_گهيرو_unix64_sse(فراغ) FFI_HIDDEN;

#جيڪڏهن_وصف_نه_ٿيل __ILP32__
خارجي ffi_حالت
ffi_prep_go_گهيرو_efi64(ffi_go_گهيرو* گهيرو, ffi_cif* cif,
			  فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*));
#جيڪڏهن_پورو

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو* گهيرو, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
#جيڪڏهن_وصف_نه_ٿيل __ILP32__
  إذا (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    أرجع ffi_prep_go_گهيرو_efi64(گهيرو, cif, fun);
#جيڪڏهن_پورو
  إذا (cif->abi != FFI_UNIX64)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = (cif->flags & UNIX64_FLAG_XMM_دليلون
		    ? ffi_go_گهيرو_unix64_sse
		    : ffi_go_گهيرو_unix64);
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

#جيڪڏهن_پورو /* FFI_GO_CLOSURES */

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_سائيز, size_t *map_سائيز)
{
  خارجي فراغ *trampoline_ڪوڊ_جدول;

  *map_سائيز = UNIX64_TRAMP_MAP_سائيز;
  *tramp_سائيز = UNIX64_TRAMP_سائيز;
  أرجع &trampoline_ڪوڊ_جدول;
}
#جيڪڏهن_پورو

#جيڪڏهن_پورو /* __x86_64__ */

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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <stdarg.h>
#شامل <stdint.h>
#شامل <stddef.h>
#شامل <string.h>
#شامل <tramp.h>
#شامل "internal64.h"

#اگر_تعریف_شدہ __x86_64__

#تعریف_کرو MAX_GPR_REGS 6
#تعریف_کرو MAX_SSE_REGS 8

#اگر defined(__INTEL_COMPILER)
#شامل "xmmintrin.h"
#تعریف_کرو UINT128 __m128
#ورنہ
#اگر defined(__SUNPRO_C)
#شامل <sunmedia_types.h>
#تعریف_کرو UINT128 __m128i
#ورنہ
#تعریف_کرو UINT128 __int128_t
#اگر_ختم
#اگر_ختم

اتحاد big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

ساخت رجسٹر_دلائل
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  اتحاد big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

خارجی خالی ffi_call_unix64 (خالی *دلائل, غیرعلامتی طویل بائٹیں, غیرعلامتی flags,
			     خالی *raddr, خالی (*fnaddr)(خالی)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
شمار x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87_اوپر_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_حافظہ_CLASS
  };

#تعریف_کرو MAX_CLASSES 4

#تعریف_کرو SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#ورنہ
# define FFI_LONGDOUBLE_BINARY128 0
#اگر_ختم

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ساکن شمار x86_64_reg_class
ملاؤ_classes (شمار x86_64_reg_class class1, شمار x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  اگر (class1 == class2)
    واپس class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  اگر (class1 == X86_64_NO_CLASS)
    واپس class2;
  اگر (class2 == X86_64_NO_CLASS)
    واپس class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  اگر (class1 == X86_64_حافظہ_CLASS || class2 == X86_64_حافظہ_CLASS)
    واپس X86_64_حافظہ_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  اگر ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    واپس X86_64_INTEGERSI_CLASS;
  اگر (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    واپس X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  اگر (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87_اوپر_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87_اوپر_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    واپس X86_64_حافظہ_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  واپس X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ساکن size_t
classify_دلیل (ffi_type *type, شمار x86_64_reg_class classes[],
		   size_t byte_انحراف)
{
  انتخاب (type->type)
    {
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_SINT128:
    حالت FFI_TYPE_اشارہ_گر:
    do_integer:
      {
	size_t سائز = byte_انحراف + type->سائز;

	اگر (سائز <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    واپس 1;
	  }
	ورنہ اگر (سائز <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    واپس 1;
	  }
	ورنہ اگر (سائز <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    واپس 2;
	  }
	ورنہ اگر (سائز <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    واپس 2;
	  }
	ورنہ
	  FFI_ASSERT (0);
      }
    حالت FFI_TYPE_FLOAT:
      اگر (!(byte_انحراف % 8))
	classes[0] = X86_64_SSESF_CLASS;
      ورنہ
	classes[0] = X86_64_SSE_CLASS;
      واپس 1;
    حالت FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      واپس 1;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#ورنہ
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87_اوپر_CLASS;
#اگر_ختم
      واپس 2;
#اگر_ختم
    حالت FFI_TYPE_STRUCT:
      {
	ثابت size_t UNITS_فی_لفظ = 8;
        size_t الفاظ = (type->سائز + byte_انحراف + UNITS_فی_لفظ - 1)
                       / UNITS_فی_لفظ;
	ffi_type **ptr;
	غیرعلامتی صحیح i;
	شمار x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	اگر (type->سائز > 32)
	  واپس 0;

	برائے (i = 0; i < الفاظ; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	اگر (!الفاظ)
	  {
    حالت FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    واپس 1;
	  }

	/* Merge the fields of structure.  */
	برائے (ptr = type->elements; *ptr != تہی; ptr++)
	  {
	    size_t عدد, pos;

	    byte_انحراف = FFI_ALIGN (byte_انحراف, (*ptr)->alignment);

	    عدد = classify_دلیل (*ptr, subclasses, byte_انحراف % 8);
	    اگر (عدد == 0)
	      واپس 0;
            pos = byte_انحراف / 8;
            برائے (i = 0; i < عدد && (i + pos) < الفاظ; i++)
	      {
		size_t pos = byte_انحراف / 8;
		classes[i + pos] =
		  ملاؤ_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_انحراف += (*ptr)->سائز;
	  }

	اگر (الفاظ > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    اگر (classes[0] != X86_64_SSE_CLASS)
	      واپس 0;

	    برائے (i = 1; i < الفاظ; i++)
	      اگر (classes[i] != X86_64_SSEUP_CLASS)
		واپس 0;
	  }

	/* Final merger cleanup.  */
	برائے (i = 0; i < الفاظ; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    اگر (classes[i] == X86_64_حافظہ_CLASS)
	      واپس 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    اگر (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    اگر (i > 1 && classes[i] == X86_64_X87_اوپر_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		واپس 0;
	      }
	  }
	واپس الفاظ;
      }
    حالت FFI_TYPE_سمتیہ:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      اگر (type->سائز == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  واپس 1;
	}
      ورنہ اگر (type->سائز == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  واپس 2;
	}
      واپس 0;
    حالت FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	انتخاب (inner->type)
	  {
	  حالت FFI_TYPE_INT:
	  حالت FFI_TYPE_UINT8:
	  حالت FFI_TYPE_SINT8:
	  حالت FFI_TYPE_UINT16:
	  حالت FFI_TYPE_SINT16:
	  حالت FFI_TYPE_UINT32:
	  حالت FFI_TYPE_SINT32:
	  حالت FFI_TYPE_UINT64:
	  حالت FFI_TYPE_SINT64:
	    جاؤ do_integer;

	  حالت FFI_TYPE_SINT128:
	  حالت FFI_TYPE_UINT128:
	    واپس 0;

	  حالت FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    اگر (byte_انحراف % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		واپس 2;
	      }
	    واپس 1;
	  حالت FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    واپس 2;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    واپس 0;
#ورنہ
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    واپس 1;
#اگر_ختم
#اگر_ختم
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

ساکن size_t
examine_دلیل (ffi_type *type, شمار x86_64_reg_class classes[MAX_CLASSES],
		  _Bool اندر_return, صحیح *pngpr, صحیح *pnsse)
{
  size_t n;
  غیرعلامتی صحیح i;
  صحیح ngpr, nsse;

  n = classify_دلیل (type, classes, 0);
  اگر (n == 0)
    واپس 0;

  ngpr = nsse = 0;
  برائے (i = 0; i < n; ++i)
    انتخاب (classes[i])
      {
      حالت X86_64_INTEGER_CLASS:
      حالت X86_64_INTEGERSI_CLASS:
	ngpr++;
	توڑو;
      حالت X86_64_SSE_CLASS:
      حالت X86_64_SSESF_CLASS:
      حالت X86_64_SSEDF_CLASS:
	nsse++;
	توڑو;
      حالت X86_64_NO_CLASS:
      حالت X86_64_SSEUP_CLASS:
	توڑو;
      حالت X86_64_X87_CLASS:
      حالت X86_64_X87_اوپر_CLASS:
      حالت X86_64_COMPLEX_X87_CLASS:
	واپس اندر_return != 0;
      طےشدہ:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  واپس n;
}

/* Perform machine dependent cif processing.  */

#اگر_غیر_تعریف_شدہ __ILP32__
خارجی ffi_حالت
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#اگر_ختم

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحیح gprcount, ssecount, i, avn, ngpr, nsse;
  غیرعلامتی flags;
  شمار x86_64_reg_class classes[MAX_CLASSES];
  size_t بائٹیں, n, rtype_سائز;
  ffi_type *rtype;

#اگر_غیر_تعریف_شدہ __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    واپس ffi_prep_cif_machdep_efi64(cif);
#اگر_ختم
  اگر (cif->abi != FFI_UNIX64)
    واپس FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_سائز = rtype->سائز;
  انتخاب (rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      توڑو;
    حالت FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      توڑو;
    حالت FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      توڑو;
    حالت FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      توڑو;
    حالت FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      توڑو;
    حالت FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      توڑو;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      توڑو;
    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_سائز_سرکاؤ);
      توڑو;
    حالت FFI_TYPE_اشارہ_گر:
      flags = (حجم(خالی *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      توڑو;
    حالت FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      توڑو;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#ورنہ
      flags = UNIX64_RET_X87;
#اگر_ختم
      توڑو;
#اگر_ختم
    حالت FFI_TYPE_STRUCT:
      n = examine_دلیل (cif->rtype, classes, 1, &ngpr, &nsse);
      اگر (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندر_MEM;
	}
      ورنہ
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  اگر (rtype_سائز == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  ورنہ اگر (rtype_سائز == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  ورنہ
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      اگر (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      ورنہ اگر (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      ورنہ اگر (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      ورنہ
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_سائز << UNIX64_سائز_سرکاؤ;
	    }
	}
      توڑو;
    حالت FFI_TYPE_سمتیہ:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      اگر (rtype_سائز == 8)
	flags = UNIX64_RET_XMM64;
      ورنہ اگر (rtype_سائز == 16)
	flags = UNIX64_RET_XMM128;
      ورنہ
	واپس FFI_BAD_TYPEDEF;
      توڑو;
    حالت FFI_TYPE_COMPLEX:
      انتخاب (rtype->elements[0]->type)
	{
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((غیرعلامتی) rtype_سائز << UNIX64_سائز_سرکاؤ);
	  توڑو;
	حالت FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  توڑو;
	حالت FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_سائز_سرکاؤ);
	  توڑو;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندر_MEM;
#ورنہ
	  flags = UNIX64_RET_X87_2;
#اگر_ختم
	  توڑو;
#اگر_ختم
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_اندر_MEM;
	  توڑو;
	طےشدہ:
	  واپس FFI_BAD_TYPEDEF;
	}
      توڑو;
    طےشدہ:
      واپس FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  برائے (i = 0, avn = cif->nargs; i < avn; i++)
    اگر (cif->arg_types[i]->type == FFI_TYPE_سمتیہ
	&& cif->arg_types[i]->سائز > 16)
      واپس FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  برائے (بائٹیں = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      اگر (examine_دلیل (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  طویل align = cif->arg_types[i]->alignment;

	  اگر (align < 8)
	    align = 8;

	  بائٹیں = FFI_ALIGN (بائٹیں, align);
	  بائٹیں += cif->arg_types[i]->سائز;
	}
      ورنہ
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  اگر (ssecount)
    flags |= UNIX64_FLAG_XMM_دلائل;

  cif->flags = flags;
  cif->بائٹیں = (غیرعلامتی) FFI_ALIGN (بائٹیں, 8);

  واپس FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  شمار x86_64_reg_class classes[MAX_CLASSES];
  حرف *ڈھیر, *argp;
  ffi_type **arg_types;
  صحیح gprcount, ssecount, ngpr, nsse, i, avn, flags;
  ساخت رجسٹر_دلائل *reg_دلائل;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  اگر (rvalue == تہی)
    {
      اگر (flags & UNIX64_FLAG_RET_اندر_MEM)
	rvalue = alloca (cif->rtype->سائز);
      ورنہ
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ڈھیر = alloca (حجم (ساخت رجسٹر_دلائل) + cif->بائٹیں + 4*8);
  reg_دلائل = (ساخت رجسٹر_دلائل *) ڈھیر;
  argp = ڈھیر + حجم (ساخت رجسٹر_دلائل);

  reg_دلائل->r10 = (uintptr_t) احاطہ;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  اگر (flags & UNIX64_FLAG_RET_اندر_MEM)
    reg_دلائل->gpr[gprcount++] = (غیرعلامتی طویل) rvalue;

  برائے (i = 0; i < avn; ++i)
    {
      size_t n, سائز = arg_types[i]->سائز;

      n = examine_دلیل (arg_types[i], classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  طویل align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  اگر (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (خالی *) FFI_ALIGN (argp, align);
          حافظہ_نقل (argp, avalue[i], سائز);

          argp += سائز;
        }
      ورنہ
	{
	  /* The argument is passed entirely in registers.  */
	  حرف *a = (حرف *) avalue[i];
	  غیرعلامتی صحیح j;

	  برائے (j = 0; j < n; j++, a += 8, سائز -= 8)
	    {
	      انتخاب (classes[j])
		{
		حالت X86_64_NO_CLASS:
		  توڑو;
		حالت X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  حافظہ_نقل ((حرف *) &reg_دلائل->sse[ssecount - 1] + 8, a,
			  سائز < 8 ? سائز : 8);
		  توڑو;
		حالت X86_64_INTEGER_CLASS:
		حالت X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  انتخاب (arg_types[i]->type)
		    {
		    حالت FFI_TYPE_SINT8:
		      reg_دلائل->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      توڑو;
		    حالت FFI_TYPE_SINT16:
		      reg_دلائل->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      توڑو;
		    حالت FFI_TYPE_SINT32:
		      reg_دلائل->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      توڑو;
		    طےشدہ:
		      reg_دلائل->gpr[gprcount] = 0;
		      حافظہ_نقل (&reg_دلائل->gpr[gprcount], a, سائز <= 8 ? سائز : 8);
		    }
		  gprcount++;
		  توڑو;
		حالت X86_64_SSE_CLASS:
		حالت X86_64_SSEDF_CLASS:
		  حافظہ_نقل (&reg_دلائل->sse[ssecount++].i64, a, حجم(UINT64));
		  توڑو;
		حالت X86_64_SSESF_CLASS:
		  حافظہ_نقل (&reg_دلائل->sse[ssecount++].i32, a, حجم(UINT32));
		  توڑو;
		طےشدہ:
		  abort();
		}
	    }
	}
    }
  reg_دلائل->rax = ssecount;

  ffi_call_unix64 (ڈھیر, cif->بائٹیں + حجم (ساخت رجسٹر_دلائل),
		   flags, rvalue, fn);
}

#اگر_غیر_تعریف_شدہ __ILP32__
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

شمار ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ڈھیر                               /* copy len bytes -> stack       */
};

قسمتعریف ساخت
{
  غیرعلامتی src_idx;     /* avalue[] index                                  */
  غیرعلامتی src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  غیرعلامتی dst_off;     /* byte offset within the register_args+stack buf  */
  غیرعلامتی len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  غیرعلامتی حرف op;
} ffi_move;

قسمتعریف ساخت
{
  غیرعلامتی nmoves;
  غیرعلامتی ssecount;    /* -> reg_args->rax                                */
  غیرعلامتی بائٹیں;       /* stack-arg area size (== cif->bytes)             */
  غیرعلامتی flags;       /* == cif->flags                                   */
  غیرعلامتی ret_اندر_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  غیرعلامتی fast;        /* nonzero -> lean trampoline eligible             */
  غیرعلامتی retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  صحیح      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  غیرعلامتی alloc_بائٹیں; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move چالیں[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
ساخت ffi_ret2 { UINT64 i; دوہرا d; };
خارجی ساخت ffi_ret2 ffi_plan_fast_call (ساخت رجسٹر_دلائل *img,
					   خالی (*fn) (خالی)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
خارجی ساخت ffi_ret2 ffi_plan_gp0 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp1 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp2 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp3 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp4 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp5 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
خارجی ساخت ffi_ret2 ffi_plan_gp6 (خالی **, خالی (*)(خالی)) FFI_HIDDEN;
ساکن ساخت ffi_ret2 (*ثابت ffi_gp_thunks[7]) (خالی **, خالی (*)(خالی)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ساکن اندرونی خالی
store_ret (خالی *rvalue, غیرعلامتی retcode, ساخت ffi_ret2 r)
{
  انتخاب (retcode)
    {
    حالت UNIX64_RET_VOID:   توڑو;
    حالت UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; توڑو;
    حالت UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; توڑو;
    حالت UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; توڑو;
    حالت UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; توڑو;
    حالت UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; توڑو;
    حالت UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; توڑو;
    حالت UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; توڑو;
    حالت UNIX64_RET_XMM32:  حافظہ_نقل (rvalue, &r.d, 4); توڑو;
    حالت UNIX64_RET_XMM64:  حافظہ_نقل (rvalue, &r.d, 8); توڑو;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ساکن ffi_plan *
build_plan (ffi_cif *cif)
{
  غیرعلامتی i, avn = cif->nargs;
  شمار x86_64_reg_class classes[MAX_CLASSES];
  غیرعلامتی nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  صحیح all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  اگر (cif->abi != FFI_UNIX64)
    واپس تہی;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  برائے (i = 0; i < avn; i++)
    {
      صحیح t = cif->arg_types[i]->type;
      اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	واپس تہی;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      اگر (t == FFI_TYPE_LONGDOUBLE)
	واپس تہی;
#اگر_ختم
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = حجم (ffi_plan) + حجم (ffi_move) * (2 * avn + 1);
  plan = حافظہ_دو (nbytes);
  اگر (plan == تہی)
    واپس تہی;
  plan->alloc_بائٹیں = (غیرعلامتی) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_اندر_mem = (cif->flags & UNIX64_FLAG_RET_اندر_MEM) ? 1 : 0;
  اگر (plan->ret_اندر_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  برائے (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t سائز = at->سائز, n, rem;
      صحیح ngpr, nsse;
      غیرعلامتی j;

      n = examine_دلیل (at, classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  طویل align = at->alignment;
	  ffi_move *m = &plan->چالیں[nm++];
	  all_gp64 = 0;
	  اگر (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ڈھیر;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (غیرعلامتی) (حجم (ساخت رجسٹر_دلائل) + argp_off);
	  m->len = (غیرعلامتی) سائز;
	  argp_off += سائز;
	  جاری;
	}

      برائے (j = 0, rem = سائز; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  انتخاب (classes[j])
	    {
	    حالت X86_64_NO_CLASS:
	    حالت X86_64_SSEUP_CLASS:
	      جاری;			/* nothing placed for this 8-byte */
	    حالت X86_64_INTEGER_CLASS:
	    حالت X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      انتخاب (at->type)
		{
		حالت FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; توڑو;
		حالت FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; توڑو;
		حالت FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; توڑو;
		طےشدہ:
		  اگر (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  ورنہ
		    { m.op = FFI_MOVE_GP; m.len = (غیرعلامتی) rem; all_gp64 = 0; }
		  توڑو;
		}
	      gprcount++;
	      توڑو;
	    حالت X86_64_SSE_CLASS:
	    حالت X86_64_SSEDF_CLASS:
	      m.dst_off = (غیرعلامتی) (offsetof (ساخت رجسٹر_دلائل, sse)
				      + ssecount * حجم (اتحاد big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      توڑو;
	    حالت X86_64_SSESF_CLASS:
	      m.dst_off = (غیرعلامتی) (offsetof (ساخت رجسٹر_دلائل, sse)
				      + ssecount * حجم (اتحاد big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      توڑو;
	    طےشدہ:
	      آزاد (plan);		/* X87 etc. in registers: bail */
	      واپس تہی;
	    }
	  plan->چالیں[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->بائٹیں = cif->بائٹیں;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->بائٹیں == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_اندر_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (صحیح) avn : -1;
  واپس plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ساکن اندرونی __attribute__ ((always_inline)) خالی
plan_exec (ffi_cif *cif, ffi_plan *plan, خالی (*fn) (خالی),
	   خالی *rvalue, خالی **avalue)
{
  غیرعلامتی flags = plan->flags;
  ساخت رجسٹر_دلائل local __attribute__ ((aligned (16)));
  حرف *ڈھیر = تہی;
  ساخت رجسٹر_دلائل *reg_دلائل;
  غیرعلامتی k;

  اگر (rvalue == تہی)
    {
      اگر (flags & UNIX64_FLAG_RET_اندر_MEM)
	rvalue = alloca (cif->rtype->سائز);
      ورنہ
	flags = UNIX64_RET_VOID;
    }

  اگر (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      ساخت ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      اگر (rvalue != تہی)
	store_ret (rvalue, plan->retcode, r);
      واپس;
    }

  اگر (plan->fast)
    reg_دلائل = &local;			/* no stack args: fixed local image */
  ورنہ
    {
      ڈھیر = alloca (حجم (ساخت رجسٹر_دلائل) + plan->بائٹیں + 4 * 8);
      reg_دلائل = (ساخت رجسٹر_دلائل *) ڈھیر;
    }
  reg_دلائل->r10 = 0;			/* closure (none for ffi_call) */
  اگر (plan->ret_اندر_mem)
    reg_دلائل->gpr[0] = (UINT64) (uintptr_t) rvalue;

  برائے (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->چالیں[k];
      حرف *src = (حرف *) avalue[m->src_idx] + m->src_off;
      حرف *dst = (حرف *) reg_دلائل + m->dst_off;
      انتخاب (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	حالت FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; توڑو;
	حالت FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; توڑو;
	حالت FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; توڑو;
	حالت FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   توڑو;
	حالت FFI_MOVE_GP:    *(UINT64 *) dst = 0; حافظہ_نقل (dst, src, m->len);     توڑو;
	حالت FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   توڑو;
	حالت FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   توڑو;
	حالت FFI_MOVE_ڈھیر: حافظہ_نقل (dst, src, m->len);                          توڑو;
	}
    }
  reg_دلائل->rax = plan->ssecount;

  اگر (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      ساخت ffi_ret2 r = ffi_plan_fast_call (reg_دلائل, fn);
      اگر (rvalue != تہی)
	store_ret (rvalue, plan->retcode, r);
      واپس;
    }

  ffi_call_unix64 (ڈھیر, plan->بائٹیں + حجم (ساخت رجسٹر_دلائل),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
ساخت ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = حافظہ_دو (حجم (ساخت ffi_call_plan));
  اگر (plan == تہی)
    واپس تہی;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  واپس plan;
}

خالی
ffi_call_plan_invoke (ffi_call_plan *plan, خالی (*fn) (خالی),
		      خالی *rvalue, خالی **avalue)
{
  اگر (plan->fast != تہی)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  ورنہ
    ffi_call (plan->cif, fn, rvalue, avalue);
}

خالی
ffi_call_plan_free (ffi_call_plan *plan)
{
  اگر (plan != تہی)
    {
      آزاد (plan->fast);
      آزاد (plan);
    }
}

size_t
ffi_call_plan_سائز (ffi_call_plan *plan)
{
  اگر (plan == تہی)
    واپس 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  واپس حجم (ساخت ffi_call_plan)
	 + (plan->fast != تہی ? plan->fast->alloc_بائٹیں : 0);
}

خارجی خالی
ffi_call_efi64(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue);
#اگر_ختم

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;
  صحیح i, nargs = cif->nargs;
  ثابت صحیح max_reg_struct_سائز = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      اگر (at->type == FFI_TYPE_STRUCT && سائز > max_reg_struct_سائز)
        {
          حرف *argcopy = alloca (سائز);
          اگر (avalue_copy == تہی)
            {
              avalue_copy = alloca (nargs * حجم (خالی *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (خالی *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], سائز);
          avalue[i] = argcopy;
        }
    }

#اگر_غیر_تعریف_شدہ __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      واپس;
    }
#اگر_ختم
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES

#اگر_غیر_تعریف_شدہ __ILP32__
خارجی خالی
ffi_call_go_efi64(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
		  خالی **avalue, خالی *احاطہ);
#اگر_ختم

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
#اگر_غیر_تعریف_شدہ __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, احاطہ);
      واپس;
    }
#اگر_ختم
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}

#اگر_ختم /* FFI_GO_CLOSURES */

خارجی خالی ffi_احاطہ_unix64(خالی) FFI_HIDDEN;
خارجی خالی ffi_احاطہ_unix64_sse(خالی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
خارجی خالی ffi_احاطہ_unix64_alt(خالی) FFI_HIDDEN;
خارجی خالی ffi_احاطہ_unix64_sse_alt(خالی) FFI_HIDDEN;
#اگر_ختم

#اگر_غیر_تعریف_شدہ __ILP32__
خارجی ffi_حالت
ffi_prep_احاطہ_loc_efi64(ffi_احاطہ* احاطہ,
			   ffi_cif* cif,
			   خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			   خالی *user_ڈیٹا,
			   خالی *codeloc);
#اگر_ختم

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  ساکن ثابت غیرعلامتی حرف trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  خالی (*dest)(خالی);
  حرف *tramp = احاطہ->tramp;

#اگر_غیر_تعریف_شدہ __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    واپس ffi_prep_احاطہ_loc_efi64(احاطہ, cif, fun, user_ڈیٹا, codeloc);
#اگر_ختم
  اگر (cif->abi != FFI_UNIX64)
    واپس FFI_BAD_ABI;

  اگر (cif->flags & UNIX64_FLAG_XMM_دلائل)
    dest = ffi_احاطہ_unix64_sse;
  ورنہ
    dest = ffi_احاطہ_unix64;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (dest == ffi_احاطہ_unix64_sse)
        dest = ffi_احاطہ_unix64_sse_alt;
      ورنہ
        dest = ffi_احاطہ_unix64_alt;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
      جاؤ برآمد;
    }
#اگر_ختم

  /* Initialize the dynamic trampoline. */
  حافظہ_نقل (tramp, trampoline, حجم(trampoline));
  *(UINT64 *)(tramp + حجم (trampoline)) = (uintptr_t)dest;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
#اگر_ختم
  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

صحیح FFI_HIDDEN
ffi_احاطہ_unix64_inner(ffi_cif *cif,
			 خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			 خالی *user_ڈیٹا,
			 خالی *rvalue,
			 ساخت رجسٹر_دلائل *reg_دلائل,
			 حرف *argp)
{
  خالی **avalue;
  ffi_type **arg_types;
  طویل i, avn;
  صحیح gprcount, ssecount, ngpr, nsse;
  صحیح flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * حجم(خالی *));
  gprcount = ssecount = 0;

  اگر (flags & UNIX64_FLAG_RET_اندر_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      خالی *r = (خالی *)(uintptr_t)reg_دلائل->gpr[gprcount++];
      *(خالی **)rvalue = r;
      rvalue = r;
      flags = (حجم(خالی *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  برائے (i = 0; i < avn; ++i)
    {
      شمار x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_دلیل (arg_types[i], classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  طویل align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  اگر (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (خالی *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->سائز;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      ورنہ اگر (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  اگر (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_دلائل->sse[ssecount];
	      ssecount += n;
	    }
	  ورنہ
	    {
	      avalue[i] = &reg_دلائل->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      ورنہ
	{
	  حرف *a = alloca (n * 8);
	  غیرعلامتی صحیح j;

	  avalue[i] = a;
	  برائے (j = 0; j < n; j++, a += 8)
	    {
	      اگر (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		حافظہ_نقل (a, (حرف *) &reg_دلائل->sse[ssecount - 1] + 8, 8);
	      ورنہ اگر (SSE_CLASS_P (classes[j]))
		حافظہ_نقل (a, &reg_دلائل->sse[ssecount++], 8);
	      ورنہ
		حافظہ_نقل (a, &reg_دلائل->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell assembly how to perform return type promotions.  */
  واپس flags;
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES

خارجی خالی ffi_go_احاطہ_unix64(خالی) FFI_HIDDEN;
خارجی خالی ffi_go_احاطہ_unix64_sse(خالی) FFI_HIDDEN;

#اگر_غیر_تعریف_شدہ __ILP32__
خارجی ffi_حالت
ffi_prep_go_احاطہ_efi64(ffi_go_احاطہ* احاطہ, ffi_cif* cif,
			  خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*));
#اگر_ختم

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ, ffi_cif* cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
#اگر_غیر_تعریف_شدہ __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    واپس ffi_prep_go_احاطہ_efi64(احاطہ, cif, fun);
#اگر_ختم
  اگر (cif->abi != FFI_UNIX64)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = (cif->flags & UNIX64_FLAG_XMM_دلائل
		    ? ffi_go_احاطہ_unix64_sse
		    : ffi_go_احاطہ_unix64);
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

#اگر_ختم /* FFI_GO_CLOSURES */

#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;

  *map_سائز = UNIX64_TRAMP_MAP_سائز;
  *tramp_سائز = UNIX64_TRAMP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم

#اگر_ختم /* __x86_64__ */

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

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#اگر defined(__INTEL_COMPILER)
#شامل "xmmintrin.h"
#define UINT128 __m128
#وگرنه
#اگر defined(__SUNPRO_C)
#شامل <sunmedia_types.h>
#define UINT128 __m128i
#وگرنه
#define UINT128 __int128_t
#endif
#endif

اجتماع big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

ساختار register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  اجتماع big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

بیرونی تهی ffi_call_unix64 (تهی *args, unsigned long bytes, unsigned flags,
			     تهی *raddr, تهی (*fnaddr)(تهی)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
شمارش x86_64_reg_class
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
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#وگرنه
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ایستا شمارش x86_64_reg_class
merge_classes (شمارش x86_64_reg_class class1, شمارش x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  اگر (class1 == class2)
    بازگشت class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  اگر (class1 == X86_64_NO_CLASS)
    بازگشت class2;
  اگر (class2 == X86_64_NO_CLASS)
    بازگشت class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  اگر (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    بازگشت X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  اگر ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    بازگشت X86_64_INTEGERSI_CLASS;
  اگر (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    بازگشت X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  اگر (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    بازگشت X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  بازگشت X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ایستا size_t
classify_argument (ffi_type *type, شمارش x86_64_reg_class classes[],
		   size_t byte_offset)
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
    حالت FFI_TYPE_POINTER:
    do_integer:
      {
	size_t size = byte_offset + type->size;

	اگر (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    بازگشت 1;
	  }
	وگرنه اگر (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    بازگشت 1;
	  }
	وگرنه اگر (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    بازگشت 2;
	  }
	وگرنه اگر (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    بازگشت 2;
	  }
	وگرنه
	  FFI_ASSERT (0);
      }
    حالت FFI_TYPE_FLOAT:
      اگر (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      وگرنه
	classes[0] = X86_64_SSE_CLASS;
      بازگشت 1;
    حالت FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      بازگشت 1;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#وگرنه
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      بازگشت 2;
#endif
    حالت FFI_TYPE_STRUCT:
      {
	ثابت size_t UNITS_PER_WORD = 8;
        size_t واژگان = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned صحیح i;
	شمارش x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	اگر (type->size > 32)
	  بازگشت 0;

	برای (i = 0; i < واژگان; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	اگر (!واژگان)
	  {
    حالت FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    بازگشت 1;
	  }

	/* Merge the fields of structure.  */
	برای (ptr = type->elements; *ptr != پوچ; ptr++)
	  {
	    size_t رقم, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    رقم = classify_argument (*ptr, subclasses, byte_offset % 8);
	    اگر (رقم == 0)
	      بازگشت 0;
            pos = byte_offset / 8;
            برای (i = 0; i < رقم && (i + pos) < واژگان; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	اگر (واژگان > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    اگر (classes[0] != X86_64_SSE_CLASS)
	      بازگشت 0;

	    برای (i = 1; i < واژگان; i++)
	      اگر (classes[i] != X86_64_SSEUP_CLASS)
		بازگشت 0;
	  }

	/* Final merger cleanup.  */
	برای (i = 0; i < واژگان; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    اگر (classes[i] == X86_64_MEMORY_CLASS)
	      بازگشت 0;

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
	    اگر (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		بازگشت 0;
	      }
	  }
	بازگشت واژگان;
      }
    حالت FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      اگر (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  بازگشت 1;
	}
      وگرنه اگر (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  بازگشت 2;
	}
      بازگشت 0;
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
	    برو do_integer;

	  حالت FFI_TYPE_SINT128:
	  حالت FFI_TYPE_UINT128:
	    بازگشت 0;

	  حالت FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    اگر (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		بازگشت 2;
	      }
	    بازگشت 1;
	  حالت FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    بازگشت 2;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    بازگشت 0;
#وگرنه
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    بازگشت 1;
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

ایستا size_t
examine_argument (ffi_type *type, شمارش x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, صحیح *pngpr, صحیح *pnsse)
{
  size_t n;
  unsigned صحیح i;
  صحیح ngpr, nsse;

  n = classify_argument (type, classes, 0);
  اگر (n == 0)
    بازگشت 0;

  ngpr = nsse = 0;
  برای (i = 0; i < n; ++i)
    انتخاب (classes[i])
      {
      حالت X86_64_INTEGER_CLASS:
      حالت X86_64_INTEGERSI_CLASS:
	ngpr++;
	بشکن;
      حالت X86_64_SSE_CLASS:
      حالت X86_64_SSESF_CLASS:
      حالت X86_64_SSEDF_CLASS:
	nsse++;
	بشکن;
      حالت X86_64_NO_CLASS:
      حالت X86_64_SSEUP_CLASS:
	بشکن;
      حالت X86_64_X87_CLASS:
      حالت X86_64_X87UP_CLASS:
      حالت X86_64_COMPLEX_X87_CLASS:
	بازگشت in_return != 0;
      پیشفرض:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  بازگشت n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
بیرونی ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  صحیح gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  شمارش x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    بازگشت ffi_prep_cif_machdep_efi64(cif);
#endif
  اگر (cif->abi != FFI_UNIX64)
    بازگشت FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  انتخاب (rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      بشکن;
    حالت FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      بشکن;
    حالت FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      بشکن;
    حالت FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      بشکن;
    حالت FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      بشکن;
    حالت FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      بشکن;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      بشکن;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      بشکن;
    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      بشکن;
    حالت FFI_TYPE_POINTER:
      flags = (اندازه(تهی *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      بشکن;
    حالت FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      بشکن;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#وگرنه
      flags = UNIX64_RET_X87;
#endif
      بشکن;
#endif
    حالت FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      اگر (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      وگرنه
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  اگر (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  وگرنه اگر (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  وگرنه
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      اگر (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      وگرنه اگر (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      وگرنه اگر (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      وگرنه
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      بشکن;
    حالت FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      اگر (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      وگرنه اگر (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      وگرنه
	بازگشت FFI_BAD_TYPEDEF;
      بشکن;
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
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_size << UNIX64_SIZE_SHIFT);
	  بشکن;
	حالت FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  بشکن;
	حالت FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  بشکن;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#اگر FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#وگرنه
	  flags = UNIX64_RET_X87_2;
#endif
	  بشکن;
#endif
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  بشکن;
	پیشفرض:
	  بازگشت FFI_BAD_TYPEDEF;
	}
      بشکن;
    پیشفرض:
      بازگشت FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  برای (i = 0, avn = cif->nargs; i < avn; i++)
    اگر (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      بازگشت FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  برای (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      اگر (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  اگر (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      وگرنه
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  اگر (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  بازگشت FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	      تهی **avalue, تهی *closure)
{
  شمارش x86_64_reg_class classes[MAX_CLASSES];
  نویسه *پشته, *argp;
  ffi_type **arg_types;
  صحیح gprcount, ssecount, ngpr, nsse, i, avn, flags;
  ساختار register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  اگر (rvalue == پوچ)
    {
      اگر (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      وگرنه
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  پشته = alloca (اندازه (ساختار register_args) + cif->bytes + 4*8);
  reg_args = (ساختار register_args *) پشته;
  argp = پشته + اندازه (ساختار register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  اگر (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  برای (i = 0; i < avn; ++i)
    {
      size_t n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  اگر (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (تهی *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], size);

          argp += size;
        }
      وگرنه
	{
	  /* The argument is passed entirely in registers.  */
	  نویسه *a = (نویسه *) avalue[i];
	  unsigned صحیح j;

	  برای (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      انتخاب (classes[j])
		{
		حالت X86_64_NO_CLASS:
		  بشکن;
		حالت X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((نویسه *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  بشکن;
		حالت X86_64_INTEGER_CLASS:
		حالت X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  انتخاب (arg_types[i]->type)
		    {
		    حالت FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      بشکن;
		    حالت FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      بشکن;
		    حالت FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      بشکن;
		    پیشفرض:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  بشکن;
		حالت X86_64_SSE_CLASS:
		حالت X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, اندازه(UINT64));
		  بشکن;
		حالت X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, اندازه(UINT32));
		  بشکن;
		پیشفرض:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (پشته, cif->bytes + اندازه (ساختار register_args),
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

شمارش ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_پشته                               /* copy len bytes -> stack       */
};

تعریفنوع ساختار
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned نویسه op;
} ffi_move;

تعریفنوع ساختار
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  صحیح      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
ساختار ffi_ret2 { UINT64 i; دوگانه d; };
بیرونی ساختار ffi_ret2 ffi_plan_fast_call (ساختار register_args *img,
					   تهی (*fn) (تهی)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
بیرونی ساختار ffi_ret2 ffi_plan_gp0 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp1 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp2 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp3 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp4 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp5 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
بیرونی ساختار ffi_ret2 ffi_plan_gp6 (تهی **, تهی (*)(تهی)) FFI_HIDDEN;
ایستا ساختار ffi_ret2 (*ثابت ffi_gp_thunks[7]) (تهی **, تهی (*)(تهی)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ایستا درونخطی تهی
store_ret (تهی *rvalue, unsigned retcode, ساختار ffi_ret2 r)
{
  انتخاب (retcode)
    {
    حالت UNIX64_RET_VOID:   بشکن;
    حالت UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; بشکن;
    حالت UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; بشکن;
    حالت UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; بشکن;
    حالت UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; بشکن;
    حالت UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; بشکن;
    حالت UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; بشکن;
    حالت UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; بشکن;
    حالت UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); بشکن;
    حالت UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); بشکن;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ایستا ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  شمارش x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  صحیح all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  اگر (cif->abi != FFI_UNIX64)
    بازگشت پوچ;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  برای (i = 0; i < avn; i++)
    {
      صحیح t = cif->arg_types[i]->type;
      اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	بازگشت پوچ;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      اگر (t == FFI_TYPE_LONGDOUBLE)
	بازگشت پوچ;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = اندازه (ffi_plan) + اندازه (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  اگر (plan == پوچ)
    بازگشت پوچ;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  اگر (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  برای (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t size = at->size, n, rem;
      صحیح ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  اگر (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_پشته;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (اندازه (ساختار register_args) + argp_off);
	  m->len = (unsigned) size;
	  argp_off += size;
	  ادامه;
	}

      برای (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  انتخاب (classes[j])
	    {
	    حالت X86_64_NO_CLASS:
	    حالت X86_64_SSEUP_CLASS:
	      ادامه;			/* nothing placed for this 8-byte */
	    حالت X86_64_INTEGER_CLASS:
	    حالت X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      انتخاب (at->type)
		{
		حالت FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; بشکن;
		حالت FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; بشکن;
		حالت FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; بشکن;
		پیشفرض:
		  اگر (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  وگرنه
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  بشکن;
		}
	      gprcount++;
	      بشکن;
	    حالت X86_64_SSE_CLASS:
	    حالت X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (ساختار register_args, sse)
				      + ssecount * اندازه (اجتماع big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      بشکن;
	    حالت X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (ساختار register_args, sse)
				      + ssecount * اندازه (اجتماع big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      بشکن;
	    پیشفرض:
	      آزاد (plan);		/* X87 etc. in registers: bail */
	      بازگشت پوچ;
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
    ? (صحیح) avn : -1;
  بازگشت plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ایستا درونخطی __attribute__ ((always_inline)) تهی
plan_exec (ffi_cif *cif, ffi_plan *plan, تهی (*fn) (تهی),
	   تهی *rvalue, تهی **avalue)
{
  unsigned flags = plan->flags;
  ساختار register_args local __attribute__ ((aligned (16)));
  نویسه *پشته = پوچ;
  ساختار register_args *reg_args;
  unsigned k;

  اگر (rvalue == پوچ)
    {
      اگر (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      وگرنه
	flags = UNIX64_RET_VOID;
    }

  اگر (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      ساختار ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      اگر (rvalue != پوچ)
	store_ret (rvalue, plan->retcode, r);
      بازگشت;
    }

  اگر (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  وگرنه
    {
      پشته = alloca (اندازه (ساختار register_args) + plan->bytes + 4 * 8);
      reg_args = (ساختار register_args *) پشته;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  اگر (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  برای (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      نویسه *src = (نویسه *) avalue[m->src_idx] + m->src_off;
      نویسه *dst = (نویسه *) reg_args + m->dst_off;
      انتخاب (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	حالت FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; بشکن;
	حالت FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; بشکن;
	حالت FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; بشکن;
	حالت FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   بشکن;
	حالت FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     بشکن;
	حالت FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   بشکن;
	حالت FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   بشکن;
	حالت FFI_MOVE_پشته: memcpy (dst, src, m->len);                          بشکن;
	}
    }
  reg_args->rax = plan->ssecount;

  اگر (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      ساختار ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      اگر (rvalue != پوچ)
	store_ret (rvalue, plan->retcode, r);
      بازگشت;
    }

  ffi_call_unix64 (پشته, plan->bytes + اندازه (ساختار register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
ساختار ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (اندازه (ساختار ffi_call_plan));
  اگر (plan == پوچ)
    بازگشت پوچ;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  بازگشت plan;
}

تهی
ffi_call_plan_invoke (ffi_call_plan *plan, تهی (*fn) (تهی),
		      تهی *rvalue, تهی **avalue)
{
  اگر (plan->fast != پوچ)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  وگرنه
    ffi_call (plan->cif, fn, rvalue, avalue);
}

تهی
ffi_call_plan_free (ffi_call_plan *plan)
{
  اگر (plan != پوچ)
    {
      آزاد (plan->fast);
      آزاد (plan);
    }
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  اگر (plan == پوچ)
    بازگشت 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  بازگشت اندازه (ساختار ffi_call_plan)
	 + (plan->fast != پوچ ? plan->fast->alloc_bytes : 0);
}

بیرونی تهی
ffi_call_efi64(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue);
#endif

تهی
ffi_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue_copy = پوچ;
  صحیح i, nargs = cif->nargs;
  ثابت صحیح max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برای (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح size = at->size;
      اگر (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          نویسه *argcopy = alloca (size);
          اگر (avalue_copy == پوچ)
            {
              avalue_copy = alloca (nargs * اندازه (تهی *));
              memcpy (avalue_copy, avalue, nargs * اندازه (تهی *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      بازگشت;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, پوچ);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
بیرونی تهی
ffi_call_go_efi64(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
		  تهی **avalue, تهی *closure);
#endif

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
#ifndef __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      بازگشت;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

بیرونی تهی ffi_closure_unix64(تهی) FFI_HIDDEN;
بیرونی تهی ffi_closure_unix64_sse(تهی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
بیرونی تهی ffi_closure_unix64_alt(تهی) FFI_HIDDEN;
بیرونی تهی ffi_closure_unix64_sse_alt(تهی) FFI_HIDDEN;
#endif

#ifndef __ILP32__
بیرونی ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
			   تهی *user_data,
			   تهی *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  ایستا ثابت unsigned نویسه trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  تهی (*dest)(تهی);
  نویسه *tramp = closure->tramp;

#ifndef __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    بازگشت ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  اگر (cif->abi != FFI_UNIX64)
    بازگشت FFI_BAD_ABI;

  اگر (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  وگرنه
    dest = ffi_closure_unix64;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      وگرنه
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      برو out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, اندازه(trampoline));
  *(UINT64 *)(tramp + اندازه (trampoline)) = (uintptr_t)dest;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

صحیح FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
			 تهی *user_data,
			 تهی *rvalue,
			 ساختار register_args *reg_args,
			 نویسه *argp)
{
  تهی **avalue;
  ffi_type **arg_types;
  long i, avn;
  صحیح gprcount, ssecount, ngpr, nsse;
  صحیح flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * اندازه(تهی *));
  gprcount = ssecount = 0;

  اگر (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      تهی *r = (تهی *)(uintptr_t)reg_args->gpr[gprcount++];
      *(تهی **)rvalue = r;
      rvalue = r;
      flags = (اندازه(تهی *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  برای (i = 0; i < avn; ++i)
    {
      شمارش x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      اگر (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  اگر (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (تهی *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      وگرنه اگر (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  اگر (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  وگرنه
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      وگرنه
	{
	  نویسه *a = alloca (n * 8);
	  unsigned صحیح j;

	  avalue[i] = a;
	  برای (j = 0; j < n; j++, a += 8)
	    {
	      اگر (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (نویسه *) &reg_args->sse[ssecount - 1] + 8, 8);
	      وگرنه اگر (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      وگرنه
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  بازگشت flags;
}

#ifdef FFI_GO_CLOSURES

بیرونی تهی ffi_go_closure_unix64(تهی) FFI_HIDDEN;
بیرونی تهی ffi_go_closure_unix64_sse(تهی) FFI_HIDDEN;

#ifndef __ILP32__
بیرونی ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*))
{
#ifndef __ILP32__
  اگر (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    بازگشت ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  اگر (cif->abi != FFI_UNIX64)
    بازگشت FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  بازگشت &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

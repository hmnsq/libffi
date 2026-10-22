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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stddef.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <tramp.h>
#அந்தர்பாவ "internal64.h"

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#எனில் defined(__INTEL_COMPILER)
#அந்தர்பாவ "xmmintrin.h"
#define UINT128 __m128
#இல்லையெனில்
#எனில் defined(__SUNPRO_C)
#அந்தர்பாவ <sunmedia_types.h>
#define UINT128 __m128i
#இல்லையெனில்
#define UINT128 __int128_t
#endif
#endif

ஒன்றிப்பு big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

கட்டமைப்பு register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  ஒன்றிப்பு big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

வெளி வெறுமை ffi_call_unix64 (வெறுமை *args, unsigned long bytes, unsigned flags,
			     வெறுமை *raddr, வெறுமை (*fnaddr)(வெறுமை)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
எண்ணிக்கை x86_64_reg_class
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
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#இல்லையெனில்
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

நிலையான எண்ணிக்கை x86_64_reg_class
merge_classes (எண்ணிக்கை x86_64_reg_class class1, எண்ணிக்கை x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  எனில் (class1 == class2)
    திரும்பு class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  எனில் (class1 == X86_64_NO_CLASS)
    திரும்பு class2;
  எனில் (class2 == X86_64_NO_CLASS)
    திரும்பு class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  எனில் (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    திரும்பு X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  எனில் ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    திரும்பு X86_64_INTEGERSI_CLASS;
  எனில் (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    திரும்பு X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  எனில் (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    திரும்பு X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  திரும்பு X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
நிலையான size_t
classify_argument (ffi_type *type, எண்ணிக்கை x86_64_reg_class classes[],
		   size_t byte_offset)
{
  தேர்வு (type->type)
    {
    நிலை FFI_TYPE_UINT8:
    நிலை FFI_TYPE_SINT8:
    நிலை FFI_TYPE_UINT16:
    நிலை FFI_TYPE_SINT16:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT128:
    நிலை FFI_TYPE_SINT128:
    நிலை FFI_TYPE_POINTER:
    do_integer:
      {
	size_t size = byte_offset + type->size;

	எனில் (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    திரும்பு 1;
	  }
	இல்லையெனில் எனில் (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    திரும்பு 1;
	  }
	இல்லையெனில் எனில் (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    திரும்பு 2;
	  }
	இல்லையெனில் எனில் (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    திரும்பு 2;
	  }
	இல்லையெனில்
	  FFI_ASSERT (0);
      }
    நிலை FFI_TYPE_FLOAT:
      எனில் (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      இல்லையெனில்
	classes[0] = X86_64_SSE_CLASS;
      திரும்பு 1;
    நிலை FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      திரும்பு 1;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
#எனில் FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#இல்லையெனில்
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      திரும்பு 2;
#endif
    நிலை FFI_TYPE_STRUCT:
      {
	மாறிலி size_t UNITS_PER_WORD = 8;
        size_t சொற்கள் = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned முழு i;
	எண்ணிக்கை x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	எனில் (type->size > 32)
	  திரும்பு 0;

	க்கு (i = 0; i < சொற்கள்; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	எனில் (!சொற்கள்)
	  {
    நிலை FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    திரும்பு 1;
	  }

	/* Merge the fields of structure.  */
	க்கு (ptr = type->elements; *ptr != வெற்று; ptr++)
	  {
	    size_t இலக்கம், pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    இலக்கம் = classify_argument (*ptr, subclasses, byte_offset % 8);
	    எனில் (இலக்கம் == 0)
	      திரும்பு 0;
            pos = byte_offset / 8;
            க்கு (i = 0; i < இலக்கம் && (i + pos) < சொற்கள்; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	எனில் (சொற்கள் > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    எனில் (classes[0] != X86_64_SSE_CLASS)
	      திரும்பு 0;

	    க்கு (i = 1; i < சொற்கள்; i++)
	      எனில் (classes[i] != X86_64_SSEUP_CLASS)
		திரும்பு 0;
	  }

	/* Final merger cleanup.  */
	க்கு (i = 0; i < சொற்கள்; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    எனில் (classes[i] == X86_64_MEMORY_CLASS)
	      திரும்பு 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    எனில் (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    எனில் (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		திரும்பு 0;
	      }
	  }
	திரும்பு சொற்கள்;
      }
    நிலை FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      எனில் (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  திரும்பு 1;
	}
      இல்லையெனில் எனில் (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  திரும்பு 2;
	}
      திரும்பு 0;
    நிலை FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	தேர்வு (inner->type)
	  {
	  நிலை FFI_TYPE_INT:
	  நிலை FFI_TYPE_UINT8:
	  நிலை FFI_TYPE_SINT8:
	  நிலை FFI_TYPE_UINT16:
	  நிலை FFI_TYPE_SINT16:
	  நிலை FFI_TYPE_UINT32:
	  நிலை FFI_TYPE_SINT32:
	  நிலை FFI_TYPE_UINT64:
	  நிலை FFI_TYPE_SINT64:
	    செல் do_integer;

	  நிலை FFI_TYPE_SINT128:
	  நிலை FFI_TYPE_UINT128:
	    திரும்பு 0;

	  நிலை FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    எனில் (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		திரும்பு 2;
	      }
	    திரும்பு 1;
	  நிலை FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    திரும்பு 2;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  நிலை FFI_TYPE_LONGDOUBLE:
#எனில் FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    திரும்பு 0;
#இல்லையெனில்
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    திரும்பு 1;
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

நிலையான size_t
examine_argument (ffi_type *type, எண்ணிக்கை x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, முழு *pngpr, முழு *pnsse)
{
  size_t n;
  unsigned முழு i;
  முழு ngpr, nsse;

  n = classify_argument (type, classes, 0);
  எனில் (n == 0)
    திரும்பு 0;

  ngpr = nsse = 0;
  க்கு (i = 0; i < n; ++i)
    தேர்வு (classes[i])
      {
      நிலை X86_64_INTEGER_CLASS:
      நிலை X86_64_INTEGERSI_CLASS:
	ngpr++;
	நிறுத்து;
      நிலை X86_64_SSE_CLASS:
      நிலை X86_64_SSESF_CLASS:
      நிலை X86_64_SSEDF_CLASS:
	nsse++;
	நிறுத்து;
      நிலை X86_64_NO_CLASS:
      நிலை X86_64_SSEUP_CLASS:
	நிறுத்து;
      நிலை X86_64_X87_CLASS:
      நிலை X86_64_X87UP_CLASS:
      நிலை X86_64_COMPLEX_X87_CLASS:
	திரும்பு in_return != 0;
      இயல்பு:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  திரும்பு n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
வெளி ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  முழு gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  எண்ணிக்கை x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  எனில் (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    திரும்பு ffi_prep_cif_machdep_efi64(cif);
#endif
  எனில் (cif->abi != FFI_UNIX64)
    திரும்பு FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  தேர்வு (rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      நிறுத்து;
    நிலை FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      நிறுத்து;
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      நிறுத்து;
    நிலை FFI_TYPE_UINT128:
    நிலை FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      நிறுத்து;
    நிலை FFI_TYPE_POINTER:
      flags = (அளவு(வெறுமை *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      நிறுத்து;
    நிலை FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      நிறுத்து;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
#எனில் FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#இல்லையெனில்
      flags = UNIX64_RET_X87;
#endif
      நிறுத்து;
#endif
    நிலை FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      எனில் (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      இல்லையெனில்
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  எனில் (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  இல்லையெனில் எனில் (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  இல்லையெனில்
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      எனில் (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      இல்லையெனில் எனில் (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      இல்லையெனில் எனில் (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      இல்லையெனில்
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      நிறுத்து;
    நிலை FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      எனில் (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      இல்லையெனில் எனில் (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      இல்லையெனில்
	திரும்பு FFI_BAD_TYPEDEF;
      நிறுத்து;
    நிலை FFI_TYPE_COMPLEX:
      தேர்வு (rtype->elements[0]->type)
	{
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_size << UNIX64_SIZE_SHIFT);
	  நிறுத்து;
	நிலை FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  நிறுத்து;
	நிலை FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  நிறுத்து;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
#எனில் FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#இல்லையெனில்
	  flags = UNIX64_RET_X87_2;
#endif
	  நிறுத்து;
#endif
	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  நிறுத்து;
	இயல்பு:
	  திரும்பு FFI_BAD_TYPEDEF;
	}
      நிறுத்து;
    இயல்பு:
      திரும்பு FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  க்கு (i = 0, avn = cif->nargs; i < avn; i++)
    எனில் (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      திரும்பு FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  க்கு (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      எனில் (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  எனில் (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      இல்லையெனில்
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  எனில் (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  திரும்பு FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  எண்ணிக்கை x86_64_reg_class classes[MAX_CLASSES];
  எழுத்து *அடுக்கு, *argp;
  ffi_type **arg_types;
  முழு gprcount, ssecount, ngpr, nsse, i, avn, flags;
  கட்டமைப்பு register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  எனில் (rvalue == வெற்று)
    {
      எனில் (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      இல்லையெனில்
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  அடுக்கு = alloca (அளவு (கட்டமைப்பு register_args) + cif->bytes + 4*8);
  reg_args = (கட்டமைப்பு register_args *) அடுக்கு;
  argp = அடுக்கு + அளவு (கட்டமைப்பு register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  எனில் (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  க்கு (i = 0; i < avn; ++i)
    {
      size_t n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      எனில் (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  எனில் (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (வெறுமை *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], size);

          argp += size;
        }
      இல்லையெனில்
	{
	  /* The argument is passed entirely in registers.  */
	  எழுத்து *a = (எழுத்து *) avalue[i];
	  unsigned முழு j;

	  க்கு (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      தேர்வு (classes[j])
		{
		நிலை X86_64_NO_CLASS:
		  நிறுத்து;
		நிலை X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((எழுத்து *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  நிறுத்து;
		நிலை X86_64_INTEGER_CLASS:
		நிலை X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  தேர்வு (arg_types[i]->type)
		    {
		    நிலை FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      நிறுத்து;
		    நிலை FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      நிறுத்து;
		    நிலை FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      நிறுத்து;
		    இயல்பு:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  நிறுத்து;
		நிலை X86_64_SSE_CLASS:
		நிலை X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, அளவு(UINT64));
		  நிறுத்து;
		நிலை X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, அளவு(UINT32));
		  நிறுத்து;
		இயல்பு:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (அடுக்கு, cif->bytes + அளவு (கட்டமைப்பு register_args),
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

எண்ணிக்கை ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_அடுக்கு                               /* copy len bytes -> stack       */
};

வகைவரை கட்டமைப்பு
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned எழுத்து op;
} ffi_move;

வகைவரை கட்டமைப்பு
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  முழு      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
கட்டமைப்பு ffi_ret2 { UINT64 i; இரட்டை d; };
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_fast_call (கட்டமைப்பு register_args *img,
					   வெறுமை (*fn) (வெறுமை)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp0 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp1 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp2 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp3 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp4 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp5 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
வெளி கட்டமைப்பு ffi_ret2 ffi_plan_gp6 (வெறுமை **, வெறுமை (*)(வெறுமை)) FFI_HIDDEN;
நிலையான கட்டமைப்பு ffi_ret2 (*மாறிலி ffi_gp_thunks[7]) (வெறுமை **, வெறுமை (*)(வெறுமை)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
நிலையான உள்வரி வெறுமை
store_ret (வெறுமை *rvalue, unsigned retcode, கட்டமைப்பு ffi_ret2 r)
{
  தேர்வு (retcode)
    {
    நிலை UNIX64_RET_VOID:   நிறுத்து;
    நிலை UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; நிறுத்து;
    நிலை UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; நிறுத்து;
    நிலை UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; நிறுத்து;
    நிலை UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; நிறுத்து;
    நிலை UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; நிறுத்து;
    நிலை UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; நிறுத்து;
    நிலை UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; நிறுத்து;
    நிலை UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); நிறுத்து;
    நிலை UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); நிறுத்து;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
நிலையான ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  எண்ணிக்கை x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  முழு all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  எனில் (cif->abi != FFI_UNIX64)
    திரும்பு வெற்று;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  க்கு (i = 0; i < avn; i++)
    {
      முழு t = cif->arg_types[i]->type;
      எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	திரும்பு வெற்று;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      எனில் (t == FFI_TYPE_LONGDOUBLE)
	திரும்பு வெற்று;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = அளவு (ffi_plan) + அளவு (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  எனில் (plan == வெற்று)
    திரும்பு வெற்று;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  எனில் (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  க்கு (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t size = at->size, n, rem;
      முழு ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      எனில் (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  எனில் (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_அடுக்கு;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (அளவு (கட்டமைப்பு register_args) + argp_off);
	  m->len = (unsigned) size;
	  argp_off += size;
	  தொடர்;
	}

      க்கு (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  தேர்வு (classes[j])
	    {
	    நிலை X86_64_NO_CLASS:
	    நிலை X86_64_SSEUP_CLASS:
	      தொடர்;			/* nothing placed for this 8-byte */
	    நிலை X86_64_INTEGER_CLASS:
	    நிலை X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      தேர்வு (at->type)
		{
		நிலை FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; நிறுத்து;
		நிலை FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; நிறுத்து;
		நிலை FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; நிறுத்து;
		இயல்பு:
		  எனில் (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  இல்லையெனில்
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  நிறுத்து;
		}
	      gprcount++;
	      நிறுத்து;
	    நிலை X86_64_SSE_CLASS:
	    நிலை X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (கட்டமைப்பு register_args, sse)
				      + ssecount * அளவு (ஒன்றிப்பு big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      நிறுத்து;
	    நிலை X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (கட்டமைப்பு register_args, sse)
				      + ssecount * அளவு (ஒன்றிப்பு big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      நிறுத்து;
	    இயல்பு:
	      விடுவி (plan);		/* X87 etc. in registers: bail */
	      திரும்பு வெற்று;
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
    ? (முழு) avn : -1;
  திரும்பு plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
நிலையான உள்வரி __attribute__ ((always_inline)) வெறுமை
plan_exec (ffi_cif *cif, ffi_plan *plan, வெறுமை (*fn) (வெறுமை),
	   வெறுமை *rvalue, வெறுமை **avalue)
{
  unsigned flags = plan->flags;
  கட்டமைப்பு register_args local __attribute__ ((aligned (16)));
  எழுத்து *அடுக்கு = வெற்று;
  கட்டமைப்பு register_args *reg_args;
  unsigned k;

  எனில் (rvalue == வெற்று)
    {
      எனில் (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      இல்லையெனில்
	flags = UNIX64_RET_VOID;
    }

  எனில் (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      கட்டமைப்பு ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      எனில் (rvalue != வெற்று)
	store_ret (rvalue, plan->retcode, r);
      திரும்பு;
    }

  எனில் (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  இல்லையெனில்
    {
      அடுக்கு = alloca (அளவு (கட்டமைப்பு register_args) + plan->bytes + 4 * 8);
      reg_args = (கட்டமைப்பு register_args *) அடுக்கு;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  எனில் (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  க்கு (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      எழுத்து *src = (எழுத்து *) avalue[m->src_idx] + m->src_off;
      எழுத்து *dst = (எழுத்து *) reg_args + m->dst_off;
      தேர்வு (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	நிலை FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; நிறுத்து;
	நிலை FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; நிறுத்து;
	நிலை FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; நிறுத்து;
	நிலை FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   நிறுத்து;
	நிலை FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     நிறுத்து;
	நிலை FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   நிறுத்து;
	நிலை FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   நிறுத்து;
	நிலை FFI_MOVE_அடுக்கு: memcpy (dst, src, m->len);                          நிறுத்து;
	}
    }
  reg_args->rax = plan->ssecount;

  எனில் (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      கட்டமைப்பு ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      எனில் (rvalue != வெற்று)
	store_ret (rvalue, plan->retcode, r);
      திரும்பு;
    }

  ffi_call_unix64 (அடுக்கு, plan->bytes + அளவு (கட்டமைப்பு register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
கட்டமைப்பு ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (அளவு (கட்டமைப்பு ffi_call_plan));
  எனில் (plan == வெற்று)
    திரும்பு வெற்று;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  திரும்பு plan;
}

வெறுமை
ffi_call_plan_invoke (ffi_call_plan *plan, வெறுமை (*fn) (வெறுமை),
		      வெறுமை *rvalue, வெறுமை **avalue)
{
  எனில் (plan->fast != வெற்று)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  இல்லையெனில்
    ffi_call (plan->cif, fn, rvalue, avalue);
}

வெறுமை
ffi_call_plan_free (ffi_call_plan *plan)
{
  எனில் (plan != வெற்று)
    {
      விடுவி (plan->fast);
      விடுவி (plan);
    }
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  எனில் (plan == வெற்று)
    திரும்பு 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  திரும்பு அளவு (கட்டமைப்பு ffi_call_plan)
	 + (plan->fast != வெற்று ? plan->fast->alloc_bytes : 0);
}

வெளி வெறுமை
ffi_call_efi64(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue);
#endif

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue_copy = வெற்று;
  முழு i, nargs = cif->nargs;
  மாறிலி முழு max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      முழு size = at->size;
      எனில் (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          எழுத்து *argcopy = alloca (size);
          எனில் (avalue_copy == வெற்று)
            {
              avalue_copy = alloca (nargs * அளவு (வெறுமை *));
              memcpy (avalue_copy, avalue, nargs * அளவு (வெறுமை *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  எனில் (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      திரும்பு;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
வெளி வெறுமை
ffi_call_go_efi64(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
		  வெறுமை **avalue, வெறுமை *closure);
#endif

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
#ifndef __ILP32__
  எனில் (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      திரும்பு;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

வெளி வெறுமை ffi_closure_unix64(வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_closure_unix64_sse(வெறுமை) FFI_HIDDEN;
#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெளி வெறுமை ffi_closure_unix64_alt(வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_closure_unix64_sse_alt(வெறுமை) FFI_HIDDEN;
#endif

#ifndef __ILP32__
வெளி ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			   வெறுமை *user_data,
			   வெறுமை *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  நிலையான மாறிலி unsigned எழுத்து trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  வெறுமை (*dest)(வெறுமை);
  எழுத்து *tramp = closure->tramp;

#ifndef __ILP32__
  எனில் (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    திரும்பு ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  எனில் (cif->abi != FFI_UNIX64)
    திரும்பு FFI_BAD_ABI;

  எனில் (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  இல்லையெனில்
    dest = ffi_closure_unix64;

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      எனில் (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      இல்லையெனில்
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      செல் out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, அளவு(trampoline));
  *(UINT64 *)(tramp + அளவு (trampoline)) = (uintptr_t)dest;

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

முழு FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			 வெறுமை *user_data,
			 வெறுமை *rvalue,
			 கட்டமைப்பு register_args *reg_args,
			 எழுத்து *argp)
{
  வெறுமை **avalue;
  ffi_type **arg_types;
  long i, avn;
  முழு gprcount, ssecount, ngpr, nsse;
  முழு flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * அளவு(வெறுமை *));
  gprcount = ssecount = 0;

  எனில் (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      வெறுமை *r = (வெறுமை *)(uintptr_t)reg_args->gpr[gprcount++];
      *(வெறுமை **)rvalue = r;
      rvalue = r;
      flags = (அளவு(வெறுமை *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  க்கு (i = 0; i < avn; ++i)
    {
      எண்ணிக்கை x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      எனில் (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  எனில் (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (வெறுமை *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      இல்லையெனில் எனில் (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  எனில் (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  இல்லையெனில்
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      இல்லையெனில்
	{
	  எழுத்து *a = alloca (n * 8);
	  unsigned முழு j;

	  avalue[i] = a;
	  க்கு (j = 0; j < n; j++, a += 8)
	    {
	      எனில் (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (எழுத்து *) &reg_args->sse[ssecount - 1] + 8, 8);
	      இல்லையெனில் எனில் (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      இல்லையெனில்
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  திரும்பு flags;
}

#ifdef FFI_GO_CLOSURES

வெளி வெறுமை ffi_go_closure_unix64(வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_go_closure_unix64_sse(வெறுமை) FFI_HIDDEN;

#ifndef __ILP32__
வெளி ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*))
{
#ifndef __ILP32__
  எனில் (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    திரும்பு ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  எனில் (cif->abi != FFI_UNIX64)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stddef.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <tramp.h>
#અન્તર્ભાવ "internal64.h"

#ifdef __x86_64__

#define MAX_GPR_REGS 6
#define MAX_SSE_REGS 8

#જો defined(__INTEL_COMPILER)
#અન્તર્ભાવ "xmmintrin.h"
#define UINT128 __m128
#નહીંતર
#જો defined(__SUNPRO_C)
#અન્તર્ભાવ <sunmedia_types.h>
#define UINT128 __m128i
#નહીંતર
#define UINT128 __int128_t
#endif
#endif

સંઘ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

સંરચના register_args
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  સંઘ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

બાહ્ય શૂન્ય ffi_call_unix64 (શૂન્ય *args, unsigned long bytes, unsigned flags,
			     શૂન્ય *raddr, શૂન્ય (*fnaddr)(શૂન્ય)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
ગણના x86_64_reg_class
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
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#નહીંતર
# define FFI_LONGDOUBLE_BINARY128 0
#endif

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

સ્થિર ગણના x86_64_reg_class
merge_classes (ગણના x86_64_reg_class class1, ગણના x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  જો (class1 == class2)
    પાછા class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  જો (class1 == X86_64_NO_CLASS)
    પાછા class2;
  જો (class2 == X86_64_NO_CLASS)
    પાછા class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  જો (class1 == X86_64_MEMORY_CLASS || class2 == X86_64_MEMORY_CLASS)
    પાછા X86_64_MEMORY_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  જો ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    પાછા X86_64_INTEGERSI_CLASS;
  જો (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    પાછા X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  જો (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87UP_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87UP_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    પાછા X86_64_MEMORY_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  પાછા X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
સ્થિર size_t
classify_argument (ffi_type *type, ગણના x86_64_reg_class classes[],
		   size_t byte_offset)
{
  પસંદગી (type->type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
    સ્થિતિ FFI_TYPE_SINT8:
    સ્થિતિ FFI_TYPE_UINT16:
    સ્થિતિ FFI_TYPE_SINT16:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT128:
    સ્થિતિ FFI_TYPE_SINT128:
    સ્થિતિ FFI_TYPE_POINTER:
    do_integer:
      {
	size_t size = byte_offset + type->size;

	જો (size <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    પાછા 1;
	  }
	નહીંતર જો (size <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    પાછા 1;
	  }
	નહીંતર જો (size <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    પાછા 2;
	  }
	નહીંતર જો (size <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    પાછા 2;
	  }
	નહીંતર
	  FFI_ASSERT (0);
      }
    સ્થિતિ FFI_TYPE_FLOAT:
      જો (!(byte_offset % 8))
	classes[0] = X86_64_SSESF_CLASS;
      નહીંતર
	classes[0] = X86_64_SSE_CLASS;
      પાછા 1;
    સ્થિતિ FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      પાછા 1;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
#જો FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#નહીંતર
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87UP_CLASS;
#endif
      પાછા 2;
#endif
    સ્થિતિ FFI_TYPE_STRUCT:
      {
	અચલ size_t UNITS_PER_WORD = 8;
        size_t શબ્દો = (type->size + byte_offset + UNITS_PER_WORD - 1)
                       / UNITS_PER_WORD;
	ffi_type **ptr;
	unsigned પૂર્ણાંક i;
	ગણના x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	જો (type->size > 32)
	  પાછા 0;

	માટે (i = 0; i < શબ્દો; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	જો (!શબ્દો)
	  {
    સ્થિતિ FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    પાછા 1;
	  }

	/* Merge the fields of structure.  */
	માટે (ptr = type->elements; *ptr != ખાલી; ptr++)
	  {
	    size_t અંક, pos;

	    byte_offset = FFI_ALIGN (byte_offset, (*ptr)->alignment);

	    અંક = classify_argument (*ptr, subclasses, byte_offset % 8);
	    જો (અંક == 0)
	      પાછા 0;
            pos = byte_offset / 8;
            માટે (i = 0; i < અંક && (i + pos) < શબ્દો; i++)
	      {
		size_t pos = byte_offset / 8;
		classes[i + pos] =
		  merge_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_offset += (*ptr)->size;
	  }

	જો (શબ્દો > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    જો (classes[0] != X86_64_SSE_CLASS)
	      પાછા 0;

	    માટે (i = 1; i < શબ્દો; i++)
	      જો (classes[i] != X86_64_SSEUP_CLASS)
		પાછા 0;
	  }

	/* Final merger cleanup.  */
	માટે (i = 0; i < શબ્દો; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    જો (classes[i] == X86_64_MEMORY_CLASS)
	      પાછા 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    જો (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    જો (i > 1 && classes[i] == X86_64_X87UP_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		પાછા 0;
	      }
	  }
	પાછા શબ્દો;
      }
    સ્થિતિ FFI_TYPE_VECTOR:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      જો (type->size == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  પાછા 1;
	}
      નહીંતર જો (type->size == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  પાછા 2;
	}
      પાછા 0;
    સ્થિતિ FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	પસંદગી (inner->type)
	  {
	  સ્થિતિ FFI_TYPE_INT:
	  સ્થિતિ FFI_TYPE_UINT8:
	  સ્થિતિ FFI_TYPE_SINT8:
	  સ્થિતિ FFI_TYPE_UINT16:
	  સ્થિતિ FFI_TYPE_SINT16:
	  સ્થિતિ FFI_TYPE_UINT32:
	  સ્થિતિ FFI_TYPE_SINT32:
	  સ્થિતિ FFI_TYPE_UINT64:
	  સ્થિતિ FFI_TYPE_SINT64:
	    જાઓ do_integer;

	  સ્થિતિ FFI_TYPE_SINT128:
	  સ્થિતિ FFI_TYPE_UINT128:
	    પાછા 0;

	  સ્થિતિ FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    જો (byte_offset % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		પાછા 2;
	      }
	    પાછા 1;
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    પાછા 2;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  સ્થિતિ FFI_TYPE_LONGDOUBLE:
#જો FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    પાછા 0;
#નહીંતર
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    પાછા 1;
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

સ્થિર size_t
examine_argument (ffi_type *type, ગણના x86_64_reg_class classes[MAX_CLASSES],
		  _Bool in_return, પૂર્ણાંક *pngpr, પૂર્ણાંક *pnsse)
{
  size_t n;
  unsigned પૂર્ણાંક i;
  પૂર્ણાંક ngpr, nsse;

  n = classify_argument (type, classes, 0);
  જો (n == 0)
    પાછા 0;

  ngpr = nsse = 0;
  માટે (i = 0; i < n; ++i)
    પસંદગી (classes[i])
      {
      સ્થિતિ X86_64_INTEGER_CLASS:
      સ્થિતિ X86_64_INTEGERSI_CLASS:
	ngpr++;
	તોડો;
      સ્થિતિ X86_64_SSE_CLASS:
      સ્થિતિ X86_64_SSESF_CLASS:
      સ્થિતિ X86_64_SSEDF_CLASS:
	nsse++;
	તોડો;
      સ્થિતિ X86_64_NO_CLASS:
      સ્થિતિ X86_64_SSEUP_CLASS:
	તોડો;
      સ્થિતિ X86_64_X87_CLASS:
      સ્થિતિ X86_64_X87UP_CLASS:
      સ્થિતિ X86_64_COMPLEX_X87_CLASS:
	પાછા in_return != 0;
      મૂળભૂત:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  પાછા n;
}

/* Perform machine dependent cif processing.  */

#ifndef __ILP32__
બાહ્ય ffi_status
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#endif

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  પૂર્ણાંક gprcount, ssecount, i, avn, ngpr, nsse;
  unsigned flags;
  ગણના x86_64_reg_class classes[MAX_CLASSES];
  size_t bytes, n, rtype_size;
  ffi_type *rtype;

#ifndef __ILP32__
  જો (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    પાછા ffi_prep_cif_machdep_efi64(cif);
#endif
  જો (cif->abi != FFI_UNIX64)
    પાછા FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_size = rtype->size;
  પસંદગી (rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT128:
    સ્થિતિ FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_SIZE_SHIFT);
      તોડો;
    સ્થિતિ FFI_TYPE_POINTER:
      flags = (કદ(શૂન્ય *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      તોડો;
    સ્થિતિ FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      તોડો;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
#જો FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#નહીંતર
      flags = UNIX64_RET_X87;
#endif
      તોડો;
#endif
    સ્થિતિ FFI_TYPE_STRUCT:
      n = examine_argument (cif->rtype, classes, 1, &ngpr, &nsse);
      જો (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	}
      નહીંતર
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  જો (rtype_size == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  નહીંતર જો (rtype_size == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  નહીંતર
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      જો (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      નહીંતર જો (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      નહીંતર જો (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      નહીંતર
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_size << UNIX64_SIZE_SHIFT;
	    }
	}
      તોડો;
    સ્થિતિ FFI_TYPE_VECTOR:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      જો (rtype_size == 8)
	flags = UNIX64_RET_XMM64;
      નહીંતર જો (rtype_size == 16)
	flags = UNIX64_RET_XMM128;
      નહીંતર
	પાછા FFI_BAD_TYPEDEF;
      તોડો;
    સ્થિતિ FFI_TYPE_COMPLEX:
      પસંદગી (rtype->elements[0]->type)
	{
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((unsigned) rtype_size << UNIX64_SIZE_SHIFT);
	  તોડો;
	સ્થિતિ FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  તોડો;
	સ્થિતિ FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_SIZE_SHIFT);
	  તોડો;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
#જો FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
#નહીંતર
	  flags = UNIX64_RET_X87_2;
#endif
	  તોડો;
#endif
	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_IN_MEM;
	  તોડો;
	મૂળભૂત:
	  પાછા FFI_BAD_TYPEDEF;
	}
      તોડો;
    મૂળભૂત:
      પાછા FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  માટે (i = 0, avn = cif->nargs; i < avn; i++)
    જો (cif->arg_types[i]->type == FFI_TYPE_VECTOR
	&& cif->arg_types[i]->size > 16)
      પાછા FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  માટે (bytes = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      જો (examine_argument (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = cif->arg_types[i]->alignment;

	  જો (align < 8)
	    align = 8;

	  bytes = FFI_ALIGN (bytes, align);
	  bytes += cif->arg_types[i]->size;
	}
      નહીંતર
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  જો (ssecount)
    flags |= UNIX64_FLAG_XMM_ARGS;

  cif->flags = flags;
  cif->bytes = (unsigned) FFI_ALIGN (bytes, 8);

  પાછા FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  ગણના x86_64_reg_class classes[MAX_CLASSES];
  અક્ષર *ઢગલો, *argp;
  ffi_type **arg_types;
  પૂર્ણાંક gprcount, ssecount, ngpr, nsse, i, avn, flags;
  સંરચના register_args *reg_args;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  જો (rvalue == ખાલી)
    {
      જો (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      નહીંતર
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ઢગલો = alloca (કદ (સંરચના register_args) + cif->bytes + 4*8);
  reg_args = (સંરચના register_args *) ઢગલો;
  argp = ઢગલો + કદ (સંરચના register_args);

  reg_args->r10 = (uintptr_t) closure;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  જો (flags & UNIX64_FLAG_RET_IN_MEM)
    reg_args->gpr[gprcount++] = (unsigned long) rvalue;

  માટે (i = 0; i < avn; ++i)
    {
      size_t n, size = arg_types[i]->size;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      જો (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  જો (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (શૂન્ય *) FFI_ALIGN (argp, align);
          memcpy (argp, avalue[i], size);

          argp += size;
        }
      નહીંતર
	{
	  /* The argument is passed entirely in registers.  */
	  અક્ષર *a = (અક્ષર *) avalue[i];
	  unsigned પૂર્ણાંક j;

	  માટે (j = 0; j < n; j++, a += 8, size -= 8)
	    {
	      પસંદગી (classes[j])
		{
		સ્થિતિ X86_64_NO_CLASS:
		  તોડો;
		સ્થિતિ X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  memcpy ((અક્ષર *) &reg_args->sse[ssecount - 1] + 8, a,
			  size < 8 ? size : 8);
		  તોડો;
		સ્થિતિ X86_64_INTEGER_CLASS:
		સ્થિતિ X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  પસંદગી (arg_types[i]->type)
		    {
		    સ્થિતિ FFI_TYPE_SINT8:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      તોડો;
		    સ્થિતિ FFI_TYPE_SINT16:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      તોડો;
		    સ્થિતિ FFI_TYPE_SINT32:
		      reg_args->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      તોડો;
		    મૂળભૂત:
		      reg_args->gpr[gprcount] = 0;
		      memcpy (&reg_args->gpr[gprcount], a, size <= 8 ? size : 8);
		    }
		  gprcount++;
		  તોડો;
		સ્થિતિ X86_64_SSE_CLASS:
		સ્થિતિ X86_64_SSEDF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i64, a, કદ(UINT64));
		  તોડો;
		સ્થિતિ X86_64_SSESF_CLASS:
		  memcpy (&reg_args->sse[ssecount++].i32, a, કદ(UINT32));
		  તોડો;
		મૂળભૂત:
		  abort();
		}
	    }
	}
    }
  reg_args->rax = ssecount;

  ffi_call_unix64 (ઢગલો, cif->bytes + કદ (સંરચના register_args),
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

ગણના ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ઢગલો                               /* copy len bytes -> stack       */
};

પ્રકારવ્યાખ્યા સંરચના
{
  unsigned src_idx;     /* avalue[] index                                  */
  unsigned src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  unsigned dst_off;     /* byte offset within the register_args+stack buf  */
  unsigned len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  unsigned અક્ષર op;
} ffi_move;

પ્રકારવ્યાખ્યા સંરચના
{
  unsigned nmoves;
  unsigned ssecount;    /* -> reg_args->rax                                */
  unsigned bytes;       /* stack-arg area size (== cif->bytes)             */
  unsigned flags;       /* == cif->flags                                   */
  unsigned ret_in_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  unsigned fast;        /* nonzero -> lean trampoline eligible             */
  unsigned retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  પૂર્ણાંક      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  unsigned alloc_bytes; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move moves[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
સંરચના ffi_ret2 { UINT64 i; દ્વિગુણ d; };
બાહ્ય સંરચના ffi_ret2 ffi_plan_fast_call (સંરચના register_args *img,
					   શૂન્ય (*fn) (શૂન્ય)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp0 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp1 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp2 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp3 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp4 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp5 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
બાહ્ય સંરચના ffi_ret2 ffi_plan_gp6 (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) FFI_HIDDEN;
સ્થિર સંરચના ffi_ret2 (*અચલ ffi_gp_thunks[7]) (શૂન્ય **, શૂન્ય (*)(શૂન્ય)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
સ્થિર અંતઃરેખ શૂન્ય
store_ret (શૂન્ય *rvalue, unsigned retcode, સંરચના ffi_ret2 r)
{
  પસંદગી (retcode)
    {
    સ્થિતિ UNIX64_RET_VOID:   તોડો;
    સ્થિતિ UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; તોડો;
    સ્થિતિ UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; તોડો;
    સ્થિતિ UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; તોડો;
    સ્થિતિ UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; તોડો;
    સ્થિતિ UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; તોડો;
    સ્થિતિ UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; તોડો;
    સ્થિતિ UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; તોડો;
    સ્થિતિ UNIX64_RET_XMM32:  memcpy (rvalue, &r.d, 4); તોડો;
    સ્થિતિ UNIX64_RET_XMM64:  memcpy (rvalue, &r.d, 8); તોડો;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
સ્થિર ffi_plan *
build_plan (ffi_cif *cif)
{
  unsigned i, avn = cif->nargs;
  ગણના x86_64_reg_class classes[MAX_CLASSES];
  unsigned nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  પૂર્ણાંક all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  જો (cif->abi != FFI_UNIX64)
    પાછા ખાલી;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  માટે (i = 0; i < avn; i++)
    {
      પૂર્ણાંક t = cif->arg_types[i]->type;
      જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	પાછા ખાલી;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      જો (t == FFI_TYPE_LONGDOUBLE)
	પાછા ખાલી;
#endif
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = કદ (ffi_plan) + કદ (ffi_move) * (2 * avn + 1);
  plan = malloc (nbytes);
  જો (plan == ખાલી)
    પાછા ખાલી;
  plan->alloc_bytes = (unsigned) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_in_mem = (cif->flags & UNIX64_FLAG_RET_IN_MEM) ? 1 : 0;
  જો (plan->ret_in_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  માટે (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t size = at->size, n, rem;
      પૂર્ણાંક ngpr, nsse;
      unsigned j;

      n = examine_argument (at, classes, 0, &ngpr, &nsse);
      જો (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = at->alignment;
	  ffi_move *m = &plan->moves[nm++];
	  all_gp64 = 0;
	  જો (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ઢગલો;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (unsigned) (કદ (સંરચના register_args) + argp_off);
	  m->len = (unsigned) size;
	  argp_off += size;
	  ચાલુ;
	}

      માટે (j = 0, rem = size; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  પસંદગી (classes[j])
	    {
	    સ્થિતિ X86_64_NO_CLASS:
	    સ્થિતિ X86_64_SSEUP_CLASS:
	      ચાલુ;			/* nothing placed for this 8-byte */
	    સ્થિતિ X86_64_INTEGER_CLASS:
	    સ્થિતિ X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      પસંદગી (at->type)
		{
		સ્થિતિ FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; તોડો;
		સ્થિતિ FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; તોડો;
		સ્થિતિ FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; તોડો;
		મૂળભૂત:
		  જો (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  નહીંતર
		    { m.op = FFI_MOVE_GP; m.len = (unsigned) rem; all_gp64 = 0; }
		  તોડો;
		}
	      gprcount++;
	      તોડો;
	    સ્થિતિ X86_64_SSE_CLASS:
	    સ્થિતિ X86_64_SSEDF_CLASS:
	      m.dst_off = (unsigned) (offsetof (સંરચના register_args, sse)
				      + ssecount * કદ (સંઘ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      તોડો;
	    સ્થિતિ X86_64_SSESF_CLASS:
	      m.dst_off = (unsigned) (offsetof (સંરચના register_args, sse)
				      + ssecount * કદ (સંઘ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      તોડો;
	    મૂળભૂત:
	      મુક્ત (plan);		/* X87 etc. in registers: bail */
	      પાછા ખાલી;
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
    ? (પૂર્ણાંક) avn : -1;
  પાછા plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
સ્થિર અંતઃરેખ __attribute__ ((always_inline)) શૂન્ય
plan_exec (ffi_cif *cif, ffi_plan *plan, શૂન્ય (*fn) (શૂન્ય),
	   શૂન્ય *rvalue, શૂન્ય **avalue)
{
  unsigned flags = plan->flags;
  સંરચના register_args local __attribute__ ((aligned (16)));
  અક્ષર *ઢગલો = ખાલી;
  સંરચના register_args *reg_args;
  unsigned k;

  જો (rvalue == ખાલી)
    {
      જો (flags & UNIX64_FLAG_RET_IN_MEM)
	rvalue = alloca (cif->rtype->size);
      નહીંતર
	flags = UNIX64_RET_VOID;
    }

  જો (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      સંરચના ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      જો (rvalue != ખાલી)
	store_ret (rvalue, plan->retcode, r);
      પાછા;
    }

  જો (plan->fast)
    reg_args = &local;			/* no stack args: fixed local image */
  નહીંતર
    {
      ઢગલો = alloca (કદ (સંરચના register_args) + plan->bytes + 4 * 8);
      reg_args = (સંરચના register_args *) ઢગલો;
    }
  reg_args->r10 = 0;			/* closure (none for ffi_call) */
  જો (plan->ret_in_mem)
    reg_args->gpr[0] = (UINT64) (uintptr_t) rvalue;

  માટે (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->moves[k];
      અક્ષર *src = (અક્ષર *) avalue[m->src_idx] + m->src_off;
      અક્ષર *dst = (અક્ષર *) reg_args + m->dst_off;
      પસંદગી (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	સ્થિતિ FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; તોડો;
	સ્થિતિ FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; તોડો;
	સ્થિતિ FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; તોડો;
	સ્થિતિ FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   તોડો;
	સ્થિતિ FFI_MOVE_GP:    *(UINT64 *) dst = 0; memcpy (dst, src, m->len);     તોડો;
	સ્થિતિ FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   તોડો;
	સ્થિતિ FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   તોડો;
	સ્થિતિ FFI_MOVE_ઢગલો: memcpy (dst, src, m->len);                          તોડો;
	}
    }
  reg_args->rax = plan->ssecount;

  જો (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      સંરચના ffi_ret2 r = ffi_plan_fast_call (reg_args, fn);
      જો (rvalue != ખાલી)
	store_ret (rvalue, plan->retcode, r);
      પાછા;
    }

  ffi_call_unix64 (ઢગલો, plan->bytes + કદ (સંરચના register_args),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
સંરચના ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (કદ (સંરચના ffi_call_plan));
  જો (plan == ખાલી)
    પાછા ખાલી;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  પાછા plan;
}

શૂન્ય
ffi_call_plan_invoke (ffi_call_plan *plan, શૂન્ય (*fn) (શૂન્ય),
		      શૂન્ય *rvalue, શૂન્ય **avalue)
{
  જો (plan->fast != ખાલી)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  નહીંતર
    ffi_call (plan->cif, fn, rvalue, avalue);
}

શૂન્ય
ffi_call_plan_free (ffi_call_plan *plan)
{
  જો (plan != ખાલી)
    {
      મુક્ત (plan->fast);
      મુક્ત (plan);
    }
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  જો (plan == ખાલી)
    પાછા 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  પાછા કદ (સંરચના ffi_call_plan)
	 + (plan->fast != ખાલી ? plan->fast->alloc_bytes : 0);
}

બાહ્ય શૂન્ય
ffi_call_efi64(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue);
#endif

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;
  પૂર્ણાંક i, nargs = cif->nargs;
  અચલ પૂર્ણાંક max_reg_struct_size = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      જો (at->type == FFI_TYPE_STRUCT && size > max_reg_struct_size)
        {
          અક્ષર *argcopy = alloca (size);
          જો (avalue_copy == ખાલી)
            {
              avalue_copy = alloca (nargs * કદ (શૂન્ય *));
              memcpy (avalue_copy, avalue, nargs * કદ (શૂન્ય *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

#ifndef __ILP32__
  જો (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      પાછા;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

#ifdef FFI_GO_CLOSURES

#ifndef __ILP32__
બાહ્ય શૂન્ય
ffi_call_go_efi64(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
		  શૂન્ય **avalue, શૂન્ય *closure);
#endif

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
#ifndef __ILP32__
  જો (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, closure);
      પાછા;
    }
#endif
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#endif /* FFI_GO_CLOSURES */

બાહ્ય શૂન્ય ffi_closure_unix64(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_closure_unix64_sse(શૂન્ય) FFI_HIDDEN;
#જો defined(FFI_EXEC_STATIC_TRAMP)
બાહ્ય શૂન્ય ffi_closure_unix64_alt(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_closure_unix64_sse_alt(શૂન્ય) FFI_HIDDEN;
#endif

#ifndef __ILP32__
બાહ્ય ffi_status
ffi_prep_closure_loc_efi64(ffi_closure* closure,
			   ffi_cif* cif,
			   શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			   શૂન્ય *user_data,
			   શૂન્ય *codeloc);
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  સ્થિર અચલ unsigned અક્ષર trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  શૂન્ય (*dest)(શૂન્ય);
  અક્ષર *tramp = closure->tramp;

#ifndef __ILP32__
  જો (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    પાછા ffi_prep_closure_loc_efi64(closure, cif, fun, user_data, codeloc);
#endif
  જો (cif->abi != FFI_UNIX64)
    પાછા FFI_BAD_ABI;

  જો (cif->flags & UNIX64_FLAG_XMM_ARGS)
    dest = ffi_closure_unix64_sse;
  નહીંતર
    dest = ffi_closure_unix64;

#જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      જો (dest == ffi_closure_unix64_sse)
        dest = ffi_closure_unix64_sse_alt;
      નહીંતર
        dest = ffi_closure_unix64_alt;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      જાઓ out;
    }
#endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, કદ(trampoline));
  *(UINT64 *)(tramp + કદ (trampoline)) = (uintptr_t)dest;

#જો defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

પૂર્ણાંક FFI_HIDDEN
ffi_closure_unix64_inner(ffi_cif *cif,
			 શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			 શૂન્ય *user_data,
			 શૂન્ય *rvalue,
			 સંરચના register_args *reg_args,
			 અક્ષર *argp)
{
  શૂન્ય **avalue;
  ffi_type **arg_types;
  long i, avn;
  પૂર્ણાંક gprcount, ssecount, ngpr, nsse;
  પૂર્ણાંક flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * કદ(શૂન્ય *));
  gprcount = ssecount = 0;

  જો (flags & UNIX64_FLAG_RET_IN_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      શૂન્ય *r = (શૂન્ય *)(uintptr_t)reg_args->gpr[gprcount++];
      *(શૂન્ય **)rvalue = r;
      rvalue = r;
      flags = (કદ(શૂન્ય *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  માટે (i = 0; i < avn; ++i)
    {
      ગણના x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_argument (arg_types[i], classes, 0, &ngpr, &nsse);
      જો (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  long align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  જો (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (શૂન્ય *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->size;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      નહીંતર જો (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  જો (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_args->sse[ssecount];
	      ssecount += n;
	    }
	  નહીંતર
	    {
	      avalue[i] = &reg_args->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      નહીંતર
	{
	  અક્ષર *a = alloca (n * 8);
	  unsigned પૂર્ણાંક j;

	  avalue[i] = a;
	  માટે (j = 0; j < n; j++, a += 8)
	    {
	      જો (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		memcpy (a, (અક્ષર *) &reg_args->sse[ssecount - 1] + 8, 8);
	      નહીંતર જો (SSE_CLASS_P (classes[j]))
		memcpy (a, &reg_args->sse[ssecount++], 8);
	      નહીંતર
		memcpy (a, &reg_args->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell assembly how to perform return type promotions.  */
  પાછા flags;
}

#ifdef FFI_GO_CLOSURES

બાહ્ય શૂન્ય ffi_go_closure_unix64(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_go_closure_unix64_sse(શૂન્ય) FFI_HIDDEN;

#ifndef __ILP32__
બાહ્ય ffi_status
ffi_prep_go_closure_efi64(ffi_go_closure* closure, ffi_cif* cif,
			  શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*));
#endif

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
#ifndef __ILP32__
  જો (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    પાછા ffi_prep_go_closure_efi64(closure, cif, fun);
#endif
  જો (cif->abi != FFI_UNIX64)
    પાછા FFI_BAD_ABI;

  closure->tramp = (cif->flags & UNIX64_FLAG_XMM_ARGS
		    ? ffi_go_closure_unix64_sse
		    : ffi_go_closure_unix64);
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

#endif /* FFI_GO_CLOSURES */

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *map_size = UNIX64_TRAMP_MAP_SIZE;
  *tramp_size = UNIX64_TRAMP_SIZE;
  પાછા &trampoline_code_table;
}
#endif

#endif /* __x86_64__ */

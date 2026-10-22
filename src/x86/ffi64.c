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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stddef.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal64.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __x86_64__

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_GPR_REGS 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SSE_REGS 8

#ਜੇ defined(__INTEL_COMPILER)
#ਅਨ੍ਤਰ੍ਭਾਵ "xmmintrin.h"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT128 __m128
#ਨਹੀਂਤਾਂ
#ਜੇ defined(__SUNPRO_C)
#ਅਨ੍ਤਰ੍ਭਾਵ <sunmedia_types.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT128 __m128i
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ UINT128 __int128_t
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਸੰਘ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  ਸੰਘ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

ਬਾਹਰੀ ਖਾਲੀ ffi_call_unix64 (ਖਾਲੀ *ਦਲੀਲਾਂ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਬਾਈਟਾਂ, ਬਿਨਾਨਿਸ਼ਾਨੀ flags,
			     ਖਾਲੀ *raddr, ਖਾਲੀ (*fnaddr)(ਖਾਲੀ)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
ਗਿਣਤੀ x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87_ਉੱਪਰ_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_ਸਮ੍ਰਿਤੀ_CLASS
  };

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_CLASSES 4

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#ਨਹੀਂਤਾਂ
# define FFI_LONGDOUBLE_BINARY128 0
#ਜੇ_ਅੰਤ

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ਸਥਿਰ ਗਿਣਤੀ x86_64_reg_class
ਮਿਲਾਓ_classes (ਗਿਣਤੀ x86_64_reg_class class1, ਗਿਣਤੀ x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  ਜੇ (class1 == class2)
    ਵਾਪਸ class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  ਜੇ (class1 == X86_64_NO_CLASS)
    ਵਾਪਸ class2;
  ਜੇ (class2 == X86_64_NO_CLASS)
    ਵਾਪਸ class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  ਜੇ (class1 == X86_64_ਸਮ੍ਰਿਤੀ_CLASS || class2 == X86_64_ਸਮ੍ਰਿਤੀ_CLASS)
    ਵਾਪਸ X86_64_ਸਮ੍ਰਿਤੀ_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  ਜੇ ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    ਵਾਪਸ X86_64_INTEGERSI_CLASS;
  ਜੇ (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    ਵਾਪਸ X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  ਜੇ (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87_ਉੱਪਰ_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87_ਉੱਪਰ_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    ਵਾਪਸ X86_64_ਸਮ੍ਰਿਤੀ_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  ਵਾਪਸ X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ਸਥਿਰ size_t
classify_ਦਲੀਲ (ffi_type *type, ਗਿਣਤੀ x86_64_reg_class classes[],
		   size_t byte_ਵਿੱਥ)
{
  ਚੋਣ (type->type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT128:
    ਹਾਲਤ FFI_TYPE_SINT128:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
    do_integer:
      {
	size_t ਮਾਪ = byte_ਵਿੱਥ + type->ਮਾਪ;

	ਜੇ (ਮਾਪ <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    ਵਾਪਸ 1;
	  }
	ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    ਵਾਪਸ 1;
	  }
	ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    ਵਾਪਸ 2;
	  }
	ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    ਵਾਪਸ 2;
	  }
	ਨਹੀਂਤਾਂ
	  FFI_ASSERT (0);
      }
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਜੇ (!(byte_ਵਿੱਥ % 8))
	classes[0] = X86_64_SSESF_CLASS;
      ਨਹੀਂਤਾਂ
	classes[0] = X86_64_SSE_CLASS;
      ਵਾਪਸ 1;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      ਵਾਪਸ 1;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#ਨਹੀਂਤਾਂ
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87_ਉੱਪਰ_CLASS;
#ਜੇ_ਅੰਤ
      ਵਾਪਸ 2;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_STRUCT:
      {
	ਅਚਲ size_t UNITS_ਪ੍ਰਤੀ_ਸ਼ਬਦ = 8;
        size_t ਸ਼ਬਦ = (type->ਮਾਪ + byte_ਵਿੱਥ + UNITS_ਪ੍ਰਤੀ_ਸ਼ਬਦ - 1)
                       / UNITS_ਪ੍ਰਤੀ_ਸ਼ਬਦ;
	ffi_type **ptr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
	ਗਿਣਤੀ x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	ਜੇ (type->ਮਾਪ > 32)
	  ਵਾਪਸ 0;

	ਲਈ (i = 0; i < ਸ਼ਬਦ; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	ਜੇ (!ਸ਼ਬਦ)
	  {
    ਹਾਲਤ FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    ਵਾਪਸ 1;
	  }

	/* Merge the fields of structure.  */
	ਲਈ (ptr = type->elements; *ptr != ਨਿਰਾ; ptr++)
	  {
	    size_t ਅੰਕ, pos;

	    byte_ਵਿੱਥ = FFI_ALIGN (byte_ਵਿੱਥ, (*ptr)->alignment);

	    ਅੰਕ = classify_ਦਲੀਲ (*ptr, subclasses, byte_ਵਿੱਥ % 8);
	    ਜੇ (ਅੰਕ == 0)
	      ਵਾਪਸ 0;
            pos = byte_ਵਿੱਥ / 8;
            ਲਈ (i = 0; i < ਅੰਕ && (i + pos) < ਸ਼ਬਦ; i++)
	      {
		size_t pos = byte_ਵਿੱਥ / 8;
		classes[i + pos] =
		  ਮਿਲਾਓ_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_ਵਿੱਥ += (*ptr)->ਮਾਪ;
	  }

	ਜੇ (ਸ਼ਬਦ > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    ਜੇ (classes[0] != X86_64_SSE_CLASS)
	      ਵਾਪਸ 0;

	    ਲਈ (i = 1; i < ਸ਼ਬਦ; i++)
	      ਜੇ (classes[i] != X86_64_SSEUP_CLASS)
		ਵਾਪਸ 0;
	  }

	/* Final merger cleanup.  */
	ਲਈ (i = 0; i < ਸ਼ਬਦ; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    ਜੇ (classes[i] == X86_64_ਸਮ੍ਰਿਤੀ_CLASS)
	      ਵਾਪਸ 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    ਜੇ (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    ਜੇ (i > 1 && classes[i] == X86_64_X87_ਉੱਪਰ_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		ਵਾਪਸ 0;
	      }
	  }
	ਵਾਪਸ ਸ਼ਬਦ;
      }
    ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      ਜੇ (type->ਮਾਪ == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  ਵਾਪਸ 1;
	}
      ਨਹੀਂਤਾਂ ਜੇ (type->ਮਾਪ == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  ਵਾਪਸ 2;
	}
      ਵਾਪਸ 0;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	ਚੋਣ (inner->type)
	  {
	  ਹਾਲਤ FFI_TYPE_INT:
	  ਹਾਲਤ FFI_TYPE_UINT8:
	  ਹਾਲਤ FFI_TYPE_SINT8:
	  ਹਾਲਤ FFI_TYPE_UINT16:
	  ਹਾਲਤ FFI_TYPE_SINT16:
	  ਹਾਲਤ FFI_TYPE_UINT32:
	  ਹਾਲਤ FFI_TYPE_SINT32:
	  ਹਾਲਤ FFI_TYPE_UINT64:
	  ਹਾਲਤ FFI_TYPE_SINT64:
	    ਜਾਓ do_integer;

	  ਹਾਲਤ FFI_TYPE_SINT128:
	  ਹਾਲਤ FFI_TYPE_UINT128:
	    ਵਾਪਸ 0;

	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    ਜੇ (byte_ਵਿੱਥ % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		ਵਾਪਸ 2;
	      }
	    ਵਾਪਸ 1;
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    ਵਾਪਸ 2;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

ਸਥਿਰ size_t
examine_ਦਲੀਲ (ffi_type *type, ਗਿਣਤੀ x86_64_reg_class classes[MAX_CLASSES],
		  _Bool ਵਿੱਚ_return, ਪੂਰਨ *pngpr, ਪੂਰਨ *pnsse)
{
  size_t n;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਪੂਰਨ ngpr, nsse;

  n = classify_ਦਲੀਲ (type, classes, 0);
  ਜੇ (n == 0)
    ਵਾਪਸ 0;

  ngpr = nsse = 0;
  ਲਈ (i = 0; i < n; ++i)
    ਚੋਣ (classes[i])
      {
      ਹਾਲਤ X86_64_INTEGER_CLASS:
      ਹਾਲਤ X86_64_INTEGERSI_CLASS:
	ngpr++;
	ਰੋਕੋ;
      ਹਾਲਤ X86_64_SSE_CLASS:
      ਹਾਲਤ X86_64_SSESF_CLASS:
      ਹਾਲਤ X86_64_SSEDF_CLASS:
	nsse++;
	ਰੋਕੋ;
      ਹਾਲਤ X86_64_NO_CLASS:
      ਹਾਲਤ X86_64_SSEUP_CLASS:
	ਰੋਕੋ;
      ਹਾਲਤ X86_64_X87_CLASS:
      ਹਾਲਤ X86_64_X87_ਉੱਪਰ_CLASS:
      ਹਾਲਤ X86_64_COMPLEX_X87_CLASS:
	ਵਾਪਸ ਵਿੱਚ_return != 0;
      ਮੂਲ:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  ਵਾਪਸ n;
}

/* Perform machine dependent cif processing.  */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
ਬਾਹਰੀ ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ਪੂਰਨ gprcount, ssecount, i, avn, ngpr, nsse;
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
  ਗਿਣਤੀ x86_64_reg_class classes[MAX_CLASSES];
  size_t ਬਾਈਟਾਂ, n, rtype_ਮਾਪ;
  ffi_type *rtype;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
  ਜੇ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ਵਾਪਸ ffi_prep_cif_machdep_efi64(cif);
#ਜੇ_ਅੰਤ
  ਜੇ (cif->abi != FFI_UNIX64)
    ਵਾਪਸ FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_ਮਾਪ = rtype->ਮਾਪ;
  ਚੋਣ (rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT128:
    ਹਾਲਤ FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_ਮਾਪ_ਖਿਸਕਾਓ);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = (ਆਕਾਰ(ਖਾਲੀ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      ਰੋਕੋ;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#ਨਹੀਂਤਾਂ
      flags = UNIX64_RET_X87;
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_STRUCT:
      n = examine_ਦਲੀਲ (cif->rtype, classes, 1, &ngpr, &nsse);
      ਜੇ (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ਵਿੱਚ_MEM;
	}
      ਨਹੀਂਤਾਂ
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  ਜੇ (rtype_ਮਾਪ == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  ਨਹੀਂਤਾਂ ਜੇ (rtype_ਮਾਪ == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  ਨਹੀਂਤਾਂ
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      ਜੇ (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      ਨਹੀਂਤਾਂ ਜੇ (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      ਨਹੀਂਤਾਂ ਜੇ (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      ਨਹੀਂਤਾਂ
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_ਮਾਪ << UNIX64_ਮਾਪ_ਖਿਸਕਾਓ;
	    }
	}
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      ਜੇ (rtype_ਮਾਪ == 8)
	flags = UNIX64_RET_XMM64;
      ਨਹੀਂਤਾਂ ਜੇ (rtype_ਮਾਪ == 16)
	flags = UNIX64_RET_XMM128;
      ਨਹੀਂਤਾਂ
	ਵਾਪਸ FFI_BAD_TYPEDEF;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      ਚੋਣ (rtype->elements[0]->type)
	{
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((ਬਿਨਾਨਿਸ਼ਾਨੀ) rtype_ਮਾਪ << UNIX64_ਮਾਪ_ਖਿਸਕਾਓ);
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_ਮਾਪ_ਖਿਸਕਾਓ);
	  ਰੋਕੋ;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ਵਿੱਚ_MEM;
#ਨਹੀਂਤਾਂ
	  flags = UNIX64_RET_X87_2;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ਵਿੱਚ_MEM;
	  ਰੋਕੋ;
	ਮੂਲ:
	  ਵਾਪਸ FFI_BAD_TYPEDEF;
	}
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  ਲਈ (i = 0, avn = cif->nargs; i < avn; i++)
    ਜੇ (cif->arg_types[i]->type == FFI_TYPE_ਸਦਿਸ਼
	&& cif->arg_types[i]->ਮਾਪ > 16)
      ਵਾਪਸ FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  ਲਈ (ਬਾਈਟਾਂ = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      ਜੇ (examine_ਦਲੀਲ (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ਲੰਮਾ align = cif->arg_types[i]->alignment;

	  ਜੇ (align < 8)
	    align = 8;

	  ਬਾਈਟਾਂ = FFI_ALIGN (ਬਾਈਟਾਂ, align);
	  ਬਾਈਟਾਂ += cif->arg_types[i]->ਮਾਪ;
	}
      ਨਹੀਂਤਾਂ
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  ਜੇ (ssecount)
    flags |= UNIX64_FLAG_XMM_ਦਲੀਲਾਂ;

  cif->flags = flags;
  cif->ਬਾਈਟਾਂ = (ਬਿਨਾਨਿਸ਼ਾਨੀ) FFI_ALIGN (ਬਾਈਟਾਂ, 8);

  ਵਾਪਸ FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ਗਿਣਤੀ x86_64_reg_class classes[MAX_CLASSES];
  ਅੱਖਰ *ਢੇਰ, *argp;
  ffi_type **arg_types;
  ਪੂਰਨ gprcount, ssecount, ngpr, nsse, i, avn, flags;
  ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *reg_ਦਲੀਲਾਂ;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ (flags & UNIX64_FLAG_RET_ਵਿੱਚ_MEM)
	rvalue = alloca (cif->rtype->ਮਾਪ);
      ਨਹੀਂਤਾਂ
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ਢੇਰ = alloca (ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ) + cif->ਬਾਈਟਾਂ + 4*8);
  reg_ਦਲੀਲਾਂ = (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *) ਢੇਰ;
  argp = ਢੇਰ + ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ);

  reg_ਦਲੀਲਾਂ->r10 = (uintptr_t) ਘੇਰਾ;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  ਜੇ (flags & UNIX64_FLAG_RET_ਵਿੱਚ_MEM)
    reg_ਦਲੀਲਾਂ->gpr[gprcount++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) rvalue;

  ਲਈ (i = 0; i < avn; ++i)
    {
      size_t n, ਮਾਪ = arg_types[i]->ਮਾਪ;

      n = examine_ਦਲੀਲ (arg_types[i], classes, 0, &ngpr, &nsse);
      ਜੇ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ਲੰਮਾ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ਜੇ (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (ਖਾਲੀ *) FFI_ALIGN (argp, align);
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argp, avalue[i], ਮਾਪ);

          argp += ਮਾਪ;
        }
      ਨਹੀਂਤਾਂ
	{
	  /* The argument is passed entirely in registers.  */
	  ਅੱਖਰ *a = (ਅੱਖਰ *) avalue[i];
	  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;

	  ਲਈ (j = 0; j < n; j++, a += 8, ਮਾਪ -= 8)
	    {
	      ਚੋਣ (classes[j])
		{
		ਹਾਲਤ X86_64_NO_CLASS:
		  ਰੋਕੋ;
		ਹਾਲਤ X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) &reg_ਦਲੀਲਾਂ->sse[ssecount - 1] + 8, a,
			  ਮਾਪ < 8 ? ਮਾਪ : 8);
		  ਰੋਕੋ;
		ਹਾਲਤ X86_64_INTEGER_CLASS:
		ਹਾਲਤ X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  ਚੋਣ (arg_types[i]->type)
		    {
		    ਹਾਲਤ FFI_TYPE_SINT8:
		      reg_ਦਲੀਲਾਂ->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_SINT16:
		      reg_ਦਲੀਲਾਂ->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_SINT32:
		      reg_ਦਲੀਲਾਂ->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      ਰੋਕੋ;
		    ਮੂਲ:
		      reg_ਦਲੀਲਾਂ->gpr[gprcount] = 0;
		      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&reg_ਦਲੀਲਾਂ->gpr[gprcount], a, ਮਾਪ <= 8 ? ਮਾਪ : 8);
		    }
		  gprcount++;
		  ਰੋਕੋ;
		ਹਾਲਤ X86_64_SSE_CLASS:
		ਹਾਲਤ X86_64_SSEDF_CLASS:
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (&reg_ਦਲੀਲਾਂ->sse[ssecount++].i64, a, ਆਕਾਰ(UINT64));
		  ਰੋਕੋ;
		ਹਾਲਤ X86_64_SSESF_CLASS:
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (&reg_ਦਲੀਲਾਂ->sse[ssecount++].i32, a, ਆਕਾਰ(UINT32));
		  ਰੋਕੋ;
		ਮੂਲ:
		  abort();
		}
	    }
	}
    }
  reg_ਦਲੀਲਾਂ->rax = ssecount;

  ffi_call_unix64 (ਢੇਰ, cif->ਬਾਈਟਾਂ + ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ),
		   flags, rvalue, fn);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
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

ਗਿਣਤੀ ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ਢੇਰ                               /* copy len bytes -> stack       */
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ src_idx;     /* avalue[] index                                  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ dst_off;     /* byte offset within the register_args+stack buf  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ op;
} ffi_move;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ nmoves;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ssecount;    /* -> reg_args->rax                                */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ;       /* stack-arg area size (== cif->bytes)             */
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags;       /* == cif->flags                                   */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ret_ਵਿੱਚ_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  ਬਿਨਾਨਿਸ਼ਾਨੀ fast;        /* nonzero -> lean trampoline eligible             */
  ਬਿਨਾਨਿਸ਼ਾਨੀ retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  ਪੂਰਨ      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  ਬਿਨਾਨਿਸ਼ਾਨੀ alloc_ਬਾਈਟਾਂ; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move ਚਾਲਾਂ[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
ਬਣਤਰ ffi_ret2 { UINT64 i; ਦੁੱਗਣਾ d; };
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_fast_call (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *img,
					   ਖਾਲੀ (*fn) (ਖਾਲੀ)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp0 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp1 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp2 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp3 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp4 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp5 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਬਾਹਰੀ ਬਣਤਰ ffi_ret2 ffi_plan_gp6 (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) FFI_HIDDEN;
ਸਥਿਰ ਬਣਤਰ ffi_ret2 (*ਅਚਲ ffi_gp_thunks[7]) (ਖਾਲੀ **, ਖਾਲੀ (*)(ਖਾਲੀ)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
store_ret (ਖਾਲੀ *rvalue, ਬਿਨਾਨਿਸ਼ਾਨੀ retcode, ਬਣਤਰ ffi_ret2 r)
{
  ਚੋਣ (retcode)
    {
    ਹਾਲਤ UNIX64_RET_VOID:   ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_XMM32:  ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &r.d, 4); ਰੋਕੋ;
    ਹਾਲਤ UNIX64_RET_XMM64:  ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &r.d, 8); ਰੋਕੋ;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ਸਥਿਰ ffi_plan *
build_plan (ffi_cif *cif)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ i, avn = cif->nargs;
  ਗਿਣਤੀ x86_64_reg_class classes[MAX_CLASSES];
  ਬਿਨਾਨਿਸ਼ਾਨੀ nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  ਪੂਰਨ all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  ਜੇ (cif->abi != FFI_UNIX64)
    ਵਾਪਸ ਨਿਰਾ;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  ਲਈ (i = 0; i < avn; i++)
    {
      ਪੂਰਨ t = cif->arg_types[i]->type;
      ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	ਵਾਪਸ ਨਿਰਾ;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ਜੇ (t == FFI_TYPE_LONGDOUBLE)
	ਵਾਪਸ ਨਿਰਾ;
#ਜੇ_ਅੰਤ
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = ਆਕਾਰ (ffi_plan) + ਆਕਾਰ (ffi_move) * (2 * avn + 1);
  plan = ਸਮ੍ਰਿਤੀ_ਦਿਓ (nbytes);
  ਜੇ (plan == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;
  plan->alloc_ਬਾਈਟਾਂ = (ਬਿਨਾਨਿਸ਼ਾਨੀ) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_ਵਿੱਚ_mem = (cif->flags & UNIX64_FLAG_RET_ਵਿੱਚ_MEM) ? 1 : 0;
  ਜੇ (plan->ret_ਵਿੱਚ_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  ਲਈ (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t ਮਾਪ = at->ਮਾਪ, n, rem;
      ਪੂਰਨ ngpr, nsse;
      ਬਿਨਾਨਿਸ਼ਾਨੀ j;

      n = examine_ਦਲੀਲ (at, classes, 0, &ngpr, &nsse);
      ਜੇ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ਲੰਮਾ align = at->alignment;
	  ffi_move *m = &plan->ਚਾਲਾਂ[nm++];
	  all_gp64 = 0;
	  ਜੇ (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ਢੇਰ;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (ਬਿਨਾਨਿਸ਼ਾਨੀ) (ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ) + argp_off);
	  m->len = (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਮਾਪ;
	  argp_off += ਮਾਪ;
	  ਜਾਰੀ;
	}

      ਲਈ (j = 0, rem = ਮਾਪ; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  ਚੋਣ (classes[j])
	    {
	    ਹਾਲਤ X86_64_NO_CLASS:
	    ਹਾਲਤ X86_64_SSEUP_CLASS:
	      ਜਾਰੀ;			/* nothing placed for this 8-byte */
	    ਹਾਲਤ X86_64_INTEGER_CLASS:
	    ਹਾਲਤ X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      ਚੋਣ (at->type)
		{
		ਹਾਲਤ FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; ਰੋਕੋ;
		ਹਾਲਤ FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; ਰੋਕੋ;
		ਹਾਲਤ FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; ਰੋਕੋ;
		ਮੂਲ:
		  ਜੇ (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  ਨਹੀਂਤਾਂ
		    { m.op = FFI_MOVE_GP; m.len = (ਬਿਨਾਨਿਸ਼ਾਨੀ) rem; all_gp64 = 0; }
		  ਰੋਕੋ;
		}
	      gprcount++;
	      ਰੋਕੋ;
	    ਹਾਲਤ X86_64_SSE_CLASS:
	    ਹਾਲਤ X86_64_SSEDF_CLASS:
	      m.dst_off = (ਬਿਨਾਨਿਸ਼ਾਨੀ) (offsetof (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ, sse)
				      + ssecount * ਆਕਾਰ (ਸੰਘ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      ਰੋਕੋ;
	    ਹਾਲਤ X86_64_SSESF_CLASS:
	      m.dst_off = (ਬਿਨਾਨਿਸ਼ਾਨੀ) (offsetof (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ, sse)
				      + ssecount * ਆਕਾਰ (ਸੰਘ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      ਰੋਕੋ;
	    ਮੂਲ:
	      ਮੁਕਤ (plan);		/* X87 etc. in registers: bail */
	      ਵਾਪਸ ਨਿਰਾ;
	    }
	  plan->ਚਾਲਾਂ[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->ਬਾਈਟਾਂ == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_ਵਿੱਚ_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (ਪੂਰਨ) avn : -1;
  ਵਾਪਸ plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ਸਥਿਰ ਸਤਰਅੰਦਰ __attribute__ ((always_inline)) ਖਾਲੀ
plan_exec (ffi_cif *cif, ffi_plan *plan, ਖਾਲੀ (*fn) (ਖਾਲੀ),
	   ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = plan->flags;
  ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ local __attribute__ ((aligned (16)));
  ਅੱਖਰ *ਢੇਰ = ਨਿਰਾ;
  ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *reg_ਦਲੀਲਾਂ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ k;

  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ (flags & UNIX64_FLAG_RET_ਵਿੱਚ_MEM)
	rvalue = alloca (cif->rtype->ਮਾਪ);
      ਨਹੀਂਤਾਂ
	flags = UNIX64_RET_VOID;
    }

  ਜੇ (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      ਬਣਤਰ ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      ਜੇ (rvalue != ਨਿਰਾ)
	store_ret (rvalue, plan->retcode, r);
      ਵਾਪਸ;
    }

  ਜੇ (plan->fast)
    reg_ਦਲੀਲਾਂ = &local;			/* no stack args: fixed local image */
  ਨਹੀਂਤਾਂ
    {
      ਢੇਰ = alloca (ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ) + plan->ਬਾਈਟਾਂ + 4 * 8);
      reg_ਦਲੀਲਾਂ = (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *) ਢੇਰ;
    }
  reg_ਦਲੀਲਾਂ->r10 = 0;			/* closure (none for ffi_call) */
  ਜੇ (plan->ret_ਵਿੱਚ_mem)
    reg_ਦਲੀਲਾਂ->gpr[0] = (UINT64) (uintptr_t) rvalue;

  ਲਈ (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->ਚਾਲਾਂ[k];
      ਅੱਖਰ *src = (ਅੱਖਰ *) avalue[m->src_idx] + m->src_off;
      ਅੱਖਰ *dst = (ਅੱਖਰ *) reg_ਦਲੀਲਾਂ + m->dst_off;
      ਚੋਣ (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	ਹਾਲਤ FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_GP:    *(UINT64 *) dst = 0; ਸਮ੍ਰਿਤੀ_ਨਕਲ (dst, src, m->len);     ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   ਰੋਕੋ;
	ਹਾਲਤ FFI_MOVE_ਢੇਰ: ਸਮ੍ਰਿਤੀ_ਨਕਲ (dst, src, m->len);                          ਰੋਕੋ;
	}
    }
  reg_ਦਲੀਲਾਂ->rax = plan->ssecount;

  ਜੇ (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      ਬਣਤਰ ffi_ret2 r = ffi_plan_fast_call (reg_ਦਲੀਲਾਂ, fn);
      ਜੇ (rvalue != ਨਿਰਾ)
	store_ret (rvalue, plan->retcode, r);
      ਵਾਪਸ;
    }

  ffi_call_unix64 (ਢੇਰ, plan->ਬਾਈਟਾਂ + ਆਕਾਰ (ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
ਬਣਤਰ ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਆਕਾਰ (ਬਣਤਰ ffi_call_plan));
  ਜੇ (plan == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  ਵਾਪਸ plan;
}

ਖਾਲੀ
ffi_call_plan_invoke (ffi_call_plan *plan, ਖਾਲੀ (*fn) (ਖਾਲੀ),
		      ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ਜੇ (plan->fast != ਨਿਰਾ)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  ਨਹੀਂਤਾਂ
    ffi_call (plan->cif, fn, rvalue, avalue);
}

ਖਾਲੀ
ffi_call_plan_free (ffi_call_plan *plan)
{
  ਜੇ (plan != ਨਿਰਾ)
    {
      ਮੁਕਤ (plan->fast);
      ਮੁਕਤ (plan);
    }
}

size_t
ffi_call_plan_ਮਾਪ (ffi_call_plan *plan)
{
  ਜੇ (plan == ਨਿਰਾ)
    ਵਾਪਸ 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  ਵਾਪਸ ਆਕਾਰ (ਬਣਤਰ ffi_call_plan)
	 + (plan->fast != ਨਿਰਾ ? plan->fast->alloc_ਬਾਈਟਾਂ : 0);
}

ਬਾਹਰੀ ਖਾਲੀ
ffi_call_efi64(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue);
#ਜੇ_ਅੰਤ

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;
  ਪੂਰਨ i, nargs = cif->nargs;
  ਅਚਲ ਪੂਰਨ max_reg_struct_ਮਾਪ = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT && ਮਾਪ > max_reg_struct_ਮਾਪ)
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
  ਜੇ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
ਬਾਹਰੀ ਖਾਲੀ
ffi_call_go_efi64(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
		  ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ);
#ਜੇ_ਅੰਤ

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
  ਜੇ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, ਘੇਰਾ);
      ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}

#ਜੇ_ਅੰਤ /* FFI_GO_CLOSURES */

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_unix64(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_unix64_sse(ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_unix64_alt(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_unix64_sse_alt(ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
ਬਾਹਰੀ ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc_efi64(ffi_ਘੇਰਾ* ਘੇਰਾ,
			   ffi_cif* cif,
			   ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			   ਖਾਲੀ *user_ਅੰਕੜੇ,
			   ਖਾਲੀ *codeloc);
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ਖਾਲੀ (*dest)(ਖਾਲੀ);
  ਅੱਖਰ *tramp = ਘੇਰਾ->tramp;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
  ਜੇ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ਵਾਪਸ ffi_prep_ਘੇਰਾ_loc_efi64(ਘੇਰਾ, cif, fun, user_ਅੰਕੜੇ, codeloc);
#ਜੇ_ਅੰਤ
  ਜੇ (cif->abi != FFI_UNIX64)
    ਵਾਪਸ FFI_BAD_ABI;

  ਜੇ (cif->flags & UNIX64_FLAG_XMM_ਦਲੀਲਾਂ)
    dest = ffi_ਘੇਰਾ_unix64_sse;
  ਨਹੀਂਤਾਂ
    dest = ffi_ਘੇਰਾ_unix64;

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਜੇ (dest == ffi_ਘੇਰਾ_unix64_sse)
        dest = ffi_ਘੇਰਾ_unix64_sse_alt;
      ਨਹੀਂਤਾਂ
        dest = ffi_ਘੇਰਾ_unix64_alt;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
#ਜੇ_ਅੰਤ

  /* Initialize the dynamic trampoline. */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tramp, trampoline, ਆਕਾਰ(trampoline));
  *(UINT64 *)(tramp + ਆਕਾਰ (trampoline)) = (uintptr_t)dest;

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_unix64_inner(ffi_cif *cif,
			 ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			 ਖਾਲੀ *user_ਅੰਕੜੇ,
			 ਖਾਲੀ *rvalue,
			 ਬਣਤਰ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ *reg_ਦਲੀਲਾਂ,
			 ਅੱਖਰ *argp)
{
  ਖਾਲੀ **avalue;
  ffi_type **arg_types;
  ਲੰਮਾ i, avn;
  ਪੂਰਨ gprcount, ssecount, ngpr, nsse;
  ਪੂਰਨ flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * ਆਕਾਰ(ਖਾਲੀ *));
  gprcount = ssecount = 0;

  ਜੇ (flags & UNIX64_FLAG_RET_ਵਿੱਚ_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      ਖਾਲੀ *r = (ਖਾਲੀ *)(uintptr_t)reg_ਦਲੀਲਾਂ->gpr[gprcount++];
      *(ਖਾਲੀ **)rvalue = r;
      rvalue = r;
      flags = (ਆਕਾਰ(ਖਾਲੀ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  ਲਈ (i = 0; i < avn; ++i)
    {
      ਗਿਣਤੀ x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_ਦਲੀਲ (arg_types[i], classes, 0, &ngpr, &nsse);
      ਜੇ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ਲੰਮਾ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ਜੇ (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (ਖਾਲੀ *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->ਮਾਪ;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      ਨਹੀਂਤਾਂ ਜੇ (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  ਜੇ (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_ਦਲੀਲਾਂ->sse[ssecount];
	      ssecount += n;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = &reg_ਦਲੀਲਾਂ->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      ਨਹੀਂਤਾਂ
	{
	  ਅੱਖਰ *a = alloca (n * 8);
	  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;

	  avalue[i] = a;
	  ਲਈ (j = 0; j < n; j++, a += 8)
	    {
	      ਜੇ (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (a, (ਅੱਖਰ *) &reg_ਦਲੀਲਾਂ->sse[ssecount - 1] + 8, 8);
	      ਨਹੀਂਤਾਂ ਜੇ (SSE_CLASS_P (classes[j]))
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (a, &reg_ਦਲੀਲਾਂ->sse[ssecount++], 8);
	      ਨਹੀਂਤਾਂ
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (a, &reg_ਦਲੀਲਾਂ->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell assembly how to perform return type promotions.  */
  ਵਾਪਸ flags;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES

ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_unix64(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_unix64_sse(ਖਾਲੀ) FFI_HIDDEN;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
ਬਾਹਰੀ ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ_efi64(ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
			  ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*));
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __ILP32__
  ਜੇ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ਵਾਪਸ ffi_prep_go_ਘੇਰਾ_efi64(ਘੇਰਾ, cif, fun);
#ਜੇ_ਅੰਤ
  ਜੇ (cif->abi != FFI_UNIX64)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = (cif->flags & UNIX64_FLAG_XMM_ਦਲੀਲਾਂ
		    ? ffi_go_ਘੇਰਾ_unix64_sse
		    : ffi_go_ਘੇਰਾ_unix64);
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਅੰਤ /* FFI_GO_CLOSURES */

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *map_ਮਾਪ = UNIX64_TRAMP_MAP_ਮਾਪ;
  *tramp_ਮਾਪ = UNIX64_TRAMP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* __x86_64__ */

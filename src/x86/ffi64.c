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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stddef.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ "internal64.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __x86_64__

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_GPR_REGS 6
#ವ್ಯಾಖ್ಯಾನಿಸು MAX_SSE_REGS 8

#ಆದರೆ defined(__INTEL_COMPILER)
#ಅನ್ತರ್ಭಾವ "xmmintrin.h"
#ವ್ಯಾಖ್ಯಾನಿಸು UINT128 __m128
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ defined(__SUNPRO_C)
#ಅನ್ತರ್ಭಾವ <sunmedia_types.h>
#ವ್ಯಾಖ್ಯಾನಿಸು UINT128 __m128i
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು UINT128 __int128_t
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಒಕ್ಕೂಟ big_int_union
{
  UINT32 i32;
  UINT64 i64;
  UINT128 i128;
};

ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು
{
  /* Registers for argument passing.  */
  UINT64 gpr[MAX_GPR_REGS];
  ಒಕ್ಕೂಟ big_int_union sse[MAX_SSE_REGS];
  UINT64 rax;	/* ssecount */
  UINT64 r10;	/* static chain */
};

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_unix64 (ಶೂನ್ಯ *ವಾದಗಳು, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಬೈಟುಗಳು, ಚಿಹ್ನೆರಹಿತ flags,
			     ಶೂನ್ಯ *raddr, ಶೂನ್ಯ (*fnaddr)(ಶೂನ್ಯ)) FFI_HIDDEN;

/* All reference to register classes here is identical to the code in
   gcc/config/i386/i386.c. Do *not* change one without the other.  */

/* Register class used for passing given 64bit part of the argument.
   These represent classes as documented by the PS ABI, with the
   exception of SSESF, SSEDF classes, that are basically SSE class,
   just gcc will use SF or DFmode move instead of DImode to avoid
   reformatting penalties.

   Similary we play games with INTEGERSI_CLASS to use cheaper SImode moves
   whenever possible (upper half does contain padding).  */
ಎಣಿಕೆ x86_64_reg_class
  {
    X86_64_NO_CLASS,
    X86_64_INTEGER_CLASS,
    X86_64_INTEGERSI_CLASS,
    X86_64_SSE_CLASS,
    X86_64_SSESF_CLASS,
    X86_64_SSEDF_CLASS,
    X86_64_SSEUP_CLASS,
    X86_64_X87_CLASS,
    X86_64_X87_ಮೇಲೆ_CLASS,
    X86_64_COMPLEX_X87_CLASS,
    X86_64_ಸ್ಮರಣೆ_CLASS
  };

#ವ್ಯಾಖ್ಯಾನಿಸು MAX_CLASSES 4

#ವ್ಯಾಖ್ಯಾನಿಸು SSE_CLASS_P(X)	((X) >= X86_64_SSE_CLASS && X <= X86_64_SSEUP_CLASS)

/* On most x86-64 targets `long double` is the 80-bit x87 type: classified
   X87/X87UP, passed in memory, returned in st(0).  But some targets (notably
   x86_64 Android, and anything built with -mlong-double-128) make `long double`
   the IEEE binary128 quad type, which the psABI passes and returns in SSE
   registers exactly like __float128 (class SSE/SSEUP -> one %xmm register).
   Detect that at compile time and classify long double accordingly.  A mantissa
   of 113 bits uniquely identifies binary128 (x87 extended is 64).  */
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE \
    && defined(__LDBL_MANT_DIG__) && __LDBL_MANT_DIG__ == 113
# define FFI_LONGDOUBLE_BINARY128 1
#ಇಲ್ಲದಿದ್ದರೆ
# define FFI_LONGDOUBLE_BINARY128 0
#ಆದರೆ_ಅಂತ್ಯ

/* x86-64 register passing implementation.  See x86-64 ABI for details.  Goal
   of this code is to classify each 8bytes of incoming argument by the register
   class and assign registers accordingly.  */

/* Return the union class of CLASS1 and CLASS2.
   See the x86-64 PS ABI for details.  */

ಸ್ಥಿರ ಎಣಿಕೆ x86_64_reg_class
ವಿಲೀನ_classes (ಎಣಿಕೆ x86_64_reg_class class1, ಎಣಿಕೆ x86_64_reg_class class2)
{
  /* Rule #1: If both classes are equal, this is the resulting class.  */
  ಆದರೆ (class1 == class2)
    ಹಿಂತಿರುಗಿ class1;

  /* Rule #2: If one of the classes is NO_CLASS, the resulting class is
     the other class.  */
  ಆದರೆ (class1 == X86_64_NO_CLASS)
    ಹಿಂತಿರುಗಿ class2;
  ಆದರೆ (class2 == X86_64_NO_CLASS)
    ಹಿಂತಿರುಗಿ class1;

  /* Rule #3: If one of the classes is MEMORY, the result is MEMORY.  */
  ಆದರೆ (class1 == X86_64_ಸ್ಮರಣೆ_CLASS || class2 == X86_64_ಸ್ಮರಣೆ_CLASS)
    ಹಿಂತಿರುಗಿ X86_64_ಸ್ಮರಣೆ_CLASS;

  /* Rule #4: If one of the classes is INTEGER, the result is INTEGER.  */
  ಆದರೆ ((class1 == X86_64_INTEGERSI_CLASS && class2 == X86_64_SSESF_CLASS)
      || (class2 == X86_64_INTEGERSI_CLASS && class1 == X86_64_SSESF_CLASS))
    ಹಿಂತಿರುಗಿ X86_64_INTEGERSI_CLASS;
  ಆದರೆ (class1 == X86_64_INTEGER_CLASS || class1 == X86_64_INTEGERSI_CLASS
      || class2 == X86_64_INTEGER_CLASS || class2 == X86_64_INTEGERSI_CLASS)
    ಹಿಂತಿರುಗಿ X86_64_INTEGER_CLASS;

  /* Rule #5: If one of the classes is X87, X87UP, or COMPLEX_X87 class,
     MEMORY is used.  */
  ಆದರೆ (class1 == X86_64_X87_CLASS
      || class1 == X86_64_X87_ಮೇಲೆ_CLASS
      || class1 == X86_64_COMPLEX_X87_CLASS
      || class2 == X86_64_X87_CLASS
      || class2 == X86_64_X87_ಮೇಲೆ_CLASS
      || class2 == X86_64_COMPLEX_X87_CLASS)
    ಹಿಂತಿರುಗಿ X86_64_ಸ್ಮರಣೆ_CLASS;

  /* Rule #6: Otherwise class SSE is used.  */
  ಹಿಂತಿರುಗಿ X86_64_SSE_CLASS;
}

/* Classify the argument of type TYPE and mode MODE.
   CLASSES will be filled by the register class used to pass each word
   of the operand.  The number of words is returned.  In case the parameter
   should be passed in memory, 0 is returned. As a special case for zero
   sized containers, classes[0] will be NO_CLASS and 1 is returned.

   See the x86-64 PS ABI for details.
*/
ಸ್ಥಿರ size_t
classify_ವಾದ (ffi_type *type, ಎಣಿಕೆ x86_64_reg_class classes[],
		   size_t byte_ಸ್ಥಾನಾಂತರ)
{
  ಆಯ್ಕೆ (type->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
    ಸ್ಥಿತಿ FFI_TYPE_SINT128:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
    do_integer:
      {
	size_t ಗಾತ್ರ_2 = byte_ಸ್ಥಾನಾಂತರ + type->ಗಾತ್ರ_2;

	ಆದರೆ (ಗಾತ್ರ_2 <= 4)
	  {
	    classes[0] = X86_64_INTEGERSI_CLASS;
	    ಹಿಂತಿರುಗಿ 1;
	  }
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 <= 8)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    ಹಿಂತಿರುಗಿ 1;
	  }
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 <= 12)
	  {
	    classes[0] = X86_64_INTEGER_CLASS;
	    classes[1] = X86_64_INTEGERSI_CLASS;
	    ಹಿಂತಿರುಗಿ 2;
	  }
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 <= 16)
	  {
	    classes[0] = classes[1] = X86_64_INTEGER_CLASS;
	    ಹಿಂತಿರುಗಿ 2;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  FFI_ASSERT (0);
      }
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಆದರೆ (!(byte_ಸ್ಥಾನಾಂತರ % 8))
	classes[0] = X86_64_SSESF_CLASS;
      ಇಲ್ಲದಿದ್ದರೆ
	classes[0] = X86_64_SSE_CLASS;
      ಹಿಂತಿರುಗಿ 1;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      classes[0] = X86_64_SSEDF_CLASS;
      ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ FFI_LONGDOUBLE_BINARY128
      /* IEEE binary128: one %xmm register, like __float128.  */
      classes[0] = X86_64_SSE_CLASS;
      classes[1] = X86_64_SSEUP_CLASS;
#ಇಲ್ಲದಿದ್ದರೆ
      classes[0] = X86_64_X87_CLASS;
      classes[1] = X86_64_X87_ಮೇಲೆ_CLASS;
#ಆದರೆ_ಅಂತ್ಯ
      ಹಿಂತಿರುಗಿ 2;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      {
	ಸ್ಥಿರಾಂಕ size_t UNITS_ಪ್ರತಿ_ಶಬ್ದ = 8;
        size_t ಪದಗಳು = (type->ಗಾತ್ರ_2 + byte_ಸ್ಥಾನಾಂತರ + UNITS_ಪ್ರತಿ_ಶಬ್ದ - 1)
                       / UNITS_ಪ್ರತಿ_ಶಬ್ದ;
	ffi_type **ptr;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
	ಎಣಿಕೆ x86_64_reg_class subclasses[MAX_CLASSES];

	/* If the struct is larger than 32 bytes, pass it on the stack.  */
	ಆದರೆ (type->ಗಾತ್ರ_2 > 32)
	  ಹಿಂತಿರುಗಿ 0;

	ಗಾಗಿ (i = 0; i < ಪದಗಳು; i++)
	  classes[i] = X86_64_NO_CLASS;

	/* Zero sized arrays or structures are NO_CLASS.  We return 0 to
	   signalize memory class, so handle it as special case.  */
	ಆದರೆ (!ಪದಗಳು)
	  {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
	    classes[0] = X86_64_NO_CLASS;
	    ಹಿಂತಿರುಗಿ 1;
	  }

	/* Merge the fields of structure.  */
	ಗಾಗಿ (ptr = type->elements; *ptr != ಖಾಲಿ; ptr++)
	  {
	    size_t ಅಂಕೆ, pos;

	    byte_ಸ್ಥಾನಾಂತರ = FFI_ALIGN (byte_ಸ್ಥಾನಾಂತರ, (*ptr)->alignment);

	    ಅಂಕೆ = classify_ವಾದ (*ptr, subclasses, byte_ಸ್ಥಾನಾಂತರ % 8);
	    ಆದರೆ (ಅಂಕೆ == 0)
	      ಹಿಂತಿರುಗಿ 0;
            pos = byte_ಸ್ಥಾನಾಂತರ / 8;
            ಗಾಗಿ (i = 0; i < ಅಂಕೆ && (i + pos) < ಪದಗಳು; i++)
	      {
		size_t pos = byte_ಸ್ಥಾನಾಂತರ / 8;
		classes[i + pos] =
		  ವಿಲೀನ_classes (subclasses[i], classes[i + pos]);
	      }

	    byte_ಸ್ಥಾನಾಂತರ += (*ptr)->ಗಾತ್ರ_2;
	  }

	ಆದರೆ (ಪದಗಳು > 2)
	  {
	    /* When size > 16 bytes, if the first one isn't
	       X86_64_SSE_CLASS or any other ones aren't
	       X86_64_SSEUP_CLASS, everything should be passed in
	       memory.  */
	    ಆದರೆ (classes[0] != X86_64_SSE_CLASS)
	      ಹಿಂತಿರುಗಿ 0;

	    ಗಾಗಿ (i = 1; i < ಪದಗಳು; i++)
	      ಆದರೆ (classes[i] != X86_64_SSEUP_CLASS)
		ಹಿಂತಿರುಗಿ 0;
	  }

	/* Final merger cleanup.  */
	ಗಾಗಿ (i = 0; i < ಪದಗಳು; i++)
	  {
	    /* If one class is MEMORY, everything should be passed in
	       memory.  */
	    ಆದರೆ (classes[i] == X86_64_ಸ್ಮರಣೆ_CLASS)
	      ಹಿಂತಿರುಗಿ 0;

	    /* The X86_64_SSEUP_CLASS should be always preceded by
	       X86_64_SSE_CLASS or X86_64_SSEUP_CLASS.  */
	    ಆದರೆ (i > 1 && classes[i] == X86_64_SSEUP_CLASS
		&& classes[i - 1] != X86_64_SSE_CLASS
		&& classes[i - 1] != X86_64_SSEUP_CLASS)
	      {
		/* The first one should never be X86_64_SSEUP_CLASS.  */
		FFI_ASSERT (i != 0);
		classes[i] = X86_64_SSE_CLASS;
	      }

	    /*  If X86_64_X87UP_CLASS isn't preceded by X86_64_X87_CLASS,
		everything should be passed in memory.  */
	    ಆದರೆ (i > 1 && classes[i] == X86_64_X87_ಮೇಲೆ_CLASS
		&& (classes[i - 1] != X86_64_X87_CLASS))
	      {
		/* The first one should never be X86_64_X87UP_CLASS.  */
		FFI_ASSERT (i != 0);
		ಹಿಂತಿರುಗಿ 0;
	      }
	  }
	ಹಿಂತಿರುಗಿ ಪದಗಳು;
      }
    ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
      /* A Short Vector occupies SSE registers: an 8-byte vector is a single
	 SSE eightbyte; a 16-byte vector is one %xmm register (SSE + SSEUP).
	 Wider vectors would need %ymm/%zmm handling this port does not
	 implement; classify them as memory here and reject them outright in
	 ffi_prep_cif_machdep so the caller gets FFI_BAD_TYPEDEF, not a
	 silently wrong in-memory pass.  */
      ಆದರೆ (type->ಗಾತ್ರ_2 == 8)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  ಹಿಂತಿರುಗಿ 1;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (type->ಗಾತ್ರ_2 == 16)
	{
	  classes[0] = X86_64_SSE_CLASS;
	  classes[1] = X86_64_SSEUP_CLASS;
	  ಹಿಂತಿರುಗಿ 2;
	}
      ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      {
	ffi_type *inner = type->elements[0];
	ಆಯ್ಕೆ (inner->type)
	  {
	  ಸ್ಥಿತಿ FFI_TYPE_INT:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	    ಹೋಗು do_integer;

	  ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	  ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	    ಹಿಂತಿರುಗಿ 0;

	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    classes[0] = X86_64_SSE_CLASS;
	    ಆದರೆ (byte_ಸ್ಥಾನಾಂತರ % 8)
	      {
		classes[1] = X86_64_SSESF_CLASS;
		ಹಿಂತಿರುಗಿ 2;
	      }
	    ಹಿಂತಿರುಗಿ 1;
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    classes[0] = classes[1] = X86_64_SSEDF_CLASS;
	    ಹಿಂತಿರುಗಿ 2;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ FFI_LONGDOUBLE_BINARY128
	    /* _Complex binary128 is 32 bytes -> passed/returned in memory.  */
	    ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	    classes[0] = X86_64_COMPLEX_X87_CLASS;
	    ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
	  }
      }
    }
  abort();
}

/* Examine the argument and return set number of register required in each
   class.  Return zero iff parameter should be passed in memory, otherwise
   the number of registers.  */

ಸ್ಥಿರ size_t
examine_ವಾದ (ffi_type *type, ಎಣಿಕೆ x86_64_reg_class classes[MAX_CLASSES],
		  _Bool ಒಳ_return, ಪೂರ್ಣಾಂಕ *pngpr, ಪೂರ್ಣಾಂಕ *pnsse)
{
  size_t n;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ngpr, nsse;

  n = classify_ವಾದ (type, classes, 0);
  ಆದರೆ (n == 0)
    ಹಿಂತಿರುಗಿ 0;

  ngpr = nsse = 0;
  ಗಾಗಿ (i = 0; i < n; ++i)
    ಆಯ್ಕೆ (classes[i])
      {
      ಸ್ಥಿತಿ X86_64_INTEGER_CLASS:
      ಸ್ಥಿತಿ X86_64_INTEGERSI_CLASS:
	ngpr++;
	ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ X86_64_SSE_CLASS:
      ಸ್ಥಿತಿ X86_64_SSESF_CLASS:
      ಸ್ಥಿತಿ X86_64_SSEDF_CLASS:
	nsse++;
	ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ X86_64_NO_CLASS:
      ಸ್ಥಿತಿ X86_64_SSEUP_CLASS:
	ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ X86_64_X87_CLASS:
      ಸ್ಥಿತಿ X86_64_X87_ಮೇಲೆ_CLASS:
      ಸ್ಥಿತಿ X86_64_COMPLEX_X87_CLASS:
	ಹಿಂತಿರುಗಿ ಒಳ_return != 0;
      ಪೂರ್ವನಿಯೋಜಿತ:
	abort ();
      }

  *pngpr = ngpr;
  *pnsse = nsse;

  ಹಿಂತಿರುಗಿ n;
}

/* Perform machine dependent cif processing.  */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
ಬಾಹ್ಯ ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep_efi64(ffi_cif *cif);
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ gprcount, ssecount, i, avn, ngpr, nsse;
  ಚಿಹ್ನೆರಹಿತ flags;
  ಎಣಿಕೆ x86_64_reg_class classes[MAX_CLASSES];
  size_t ಬೈಟುಗಳು, n, rtype_ಗಾತ್ರ;
  ffi_type *rtype;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
  ಆದರೆ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_efi64(cif);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (cif->abi != FFI_UNIX64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  gprcount = ssecount = 0;

  rtype = cif->rtype;
  rtype_ಗಾತ್ರ = rtype->ಗಾತ್ರ_2;
  ಆಯ್ಕೆ (rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = UNIX64_RET_VOID;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      flags = UNIX64_RET_UINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      flags = UNIX64_RET_SINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      flags = UNIX64_RET_UINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      flags = UNIX64_RET_SINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      flags = UNIX64_RET_UINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      flags = UNIX64_RET_SINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      flags = UNIX64_RET_INT64;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
    ಸ್ಥಿತಿ FFI_TYPE_SINT128:
      flags = UNIX64_RET_ST_RAX_RDX | (16 << UNIX64_ಗಾತ್ರ_ಸರಿಸು);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = (ಗಾತ್ರ(ಶೂನ್ಯ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags = UNIX64_RET_XMM32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags = UNIX64_RET_XMM64;
      ನಿಲ್ಲಿಸು;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ FFI_LONGDOUBLE_BINARY128
      flags = UNIX64_RET_XMM128;	/* returned in %xmm0 (16 bytes) */
#ಇಲ್ಲದಿದ್ದರೆ
      flags = UNIX64_RET_X87;
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      n = examine_ವಾದ (cif->rtype, classes, 1, &ngpr, &nsse);
      ಆದರೆ (n == 0)
	{
	  /* The return value is passed in memory.  A pointer to that
	     memory is the first argument.  Allocate a register for it.  */
	  gprcount++;
	  /* We don't have to do anything in asm for the return.  */
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ಒಳ_MEM;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  _Bool sse0 = SSE_CLASS_P (classes[0]);

	  ಆದರೆ (rtype_ಗಾತ್ರ == 4 && sse0)
	    flags = UNIX64_RET_XMM32;
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rtype_ಗಾತ್ರ == 8)
	    flags = sse0 ? UNIX64_RET_XMM64 : UNIX64_RET_INT64;
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      _Bool sse1 = n == 2 && SSE_CLASS_P (classes[1]);
	      ಆದರೆ (sse0 && sse1)
		flags = UNIX64_RET_ST_XMM0_XMM1;
	      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sse0)
		flags = UNIX64_RET_ST_XMM0_RAX;
	      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sse1)
		flags = UNIX64_RET_ST_RAX_XMM0;
	      ಇಲ್ಲದಿದ್ದರೆ
		flags = UNIX64_RET_ST_RAX_RDX;
	      flags |= rtype_ಗಾತ್ರ << UNIX64_ಗಾತ್ರ_ಸರಿಸು;
	    }
	}
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
      /* An 8-byte vector returns in the low half of %xmm0; a 16-byte vector
	 fills %xmm0 (SSE + SSEUP).  Wider vectors are unsupported here.  */
      ಆದರೆ (rtype_ಗಾತ್ರ == 8)
	flags = UNIX64_RET_XMM64;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rtype_ಗಾತ್ರ == 16)
	flags = UNIX64_RET_XMM128;
      ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      ಆಯ್ಕೆ (rtype->elements[0]->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  flags = UNIX64_RET_ST_RAX_RDX | ((ಚಿಹ್ನೆರಹಿತ) rtype_ಗಾತ್ರ << UNIX64_ಗಾತ್ರ_ಸರಿಸು);
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  flags = UNIX64_RET_XMM64;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  flags = UNIX64_RET_ST_XMM0_XMM1 | (16 << UNIX64_ಗಾತ್ರ_ಸರಿಸು);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ಆದರೆ FFI_LONGDOUBLE_BINARY128
	  /* _Complex binary128 (32 bytes) is returned in memory.  */
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ಒಳ_MEM;
#ಇಲ್ಲದಿದ್ದರೆ
	  flags = UNIX64_RET_X87_2;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	  gprcount++;
	  flags = UNIX64_RET_VOID | UNIX64_FLAG_RET_ಒಳ_MEM;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
	}
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
    }

  /* Reject vectors wider than 16 bytes as arguments: correct %ymm/%zmm
     passing needs unix64.S register-save changes that are out of scope for
     this port, and classify_argument would otherwise silently treat them as
     an in-memory aggregate.  */
  ಗಾಗಿ (i = 0, avn = cif->nargs; i < avn; i++)
    ಆದರೆ (cif->arg_types[i]->type == FFI_TYPE_ಸದಿಶ
	&& cif->arg_types[i]->ಗಾತ್ರ_2 > 16)
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  /* Go over all arguments and determine the way they should be passed.
     If it's in a register and there is space for it, let that be so. If
     not, add it's size to the stack byte count.  */
  ಗಾಗಿ (ಬೈಟುಗಳು = 0, i = 0, avn = cif->nargs; i < avn; i++)
    {
      ಆದರೆ (examine_ವಾದ (cif->arg_types[i], classes, 0, &ngpr, &nsse) == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ದೀರ್ಘ align = cif->arg_types[i]->alignment;

	  ಆದರೆ (align < 8)
	    align = 8;

	  ಬೈಟುಗಳು = FFI_ALIGN (ಬೈಟುಗಳು, align);
	  ಬೈಟುಗಳು += cif->arg_types[i]->ಗಾತ್ರ_2;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  gprcount += ngpr;
	  ssecount += nsse;
	}
    }
  ಆದರೆ (ssecount)
    flags |= UNIX64_FLAG_XMM_ವಾದಗಳು;

  cif->flags = flags;
  cif->ಬೈಟುಗಳು = (ಚಿಹ್ನೆರಹಿತ) FFI_ALIGN (ಬೈಟುಗಳು, 8);

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* n.b. ffi_call_unix64 will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ಎಣಿಕೆ x86_64_reg_class classes[MAX_CLASSES];
  ಅಕ್ಷರ *ರಾಶಿ, *argp;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಂಕ gprcount, ssecount, ngpr, nsse, i, avn, flags;
  ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *reg_ವಾದಗಳು;

  /* Can't call 32-bit mode from 64-bit mode.  */
  FFI_ASSERT (cif->abi == FFI_UNIX64);

  /* If the return value is a struct and we don't have a return value
     address then we need to make one.  Otherwise we can ignore it.  */
  flags = cif->flags;
  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ (flags & UNIX64_FLAG_RET_ಒಳ_MEM)
	rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
      ಇಲ್ಲದಿದ್ದರೆ
	flags = UNIX64_RET_VOID;
    }

  arg_types = cif->arg_types;
  avn = cif->nargs;

  /* Allocate the space for the arguments, plus 4 words of temp space.  */
  ರಾಶಿ = alloca (ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು) + cif->ಬೈಟುಗಳು + 4*8);
  reg_ವಾದಗಳು = (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *) ರಾಶಿ;
  argp = ರಾಶಿ + ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು);

  reg_ವಾದಗಳು->r10 = (uintptr_t) ಆವರಣ;

  gprcount = ssecount = 0;

  /* If the return value is passed in memory, add the pointer as the
     first integer argument.  */
  ಆದರೆ (flags & UNIX64_FLAG_RET_ಒಳ_MEM)
    reg_ವಾದಗಳು->gpr[gprcount++] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) rvalue;

  ಗಾಗಿ (i = 0; i < avn; ++i)
    {
      size_t n, ಗಾತ್ರ_2 = arg_types[i]->ಗಾತ್ರ_2;

      n = examine_ವಾದ (arg_types[i], classes, 0, &ngpr, &nsse);
      ಆದರೆ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ದೀರ್ಘ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ಆದರೆ (align < 8)
	    align = 8;

          /* Pass this argument in memory.  */
          argp = (ಶೂನ್ಯ *) FFI_ALIGN (argp, align);
          ಸ್ಮರಣೆ_ನಕಲು (argp, avalue[i], ಗಾತ್ರ_2);

          argp += ಗಾತ್ರ_2;
        }
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  /* The argument is passed entirely in registers.  */
	  ಅಕ್ಷರ *a = (ಅಕ್ಷರ *) avalue[i];
	  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;

	  ಗಾಗಿ (j = 0; j < n; j++, a += 8, ಗಾತ್ರ_2 -= 8)
	    {
	      ಆಯ್ಕೆ (classes[j])
		{
		ಸ್ಥಿತಿ X86_64_NO_CLASS:
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ X86_64_SSEUP_CLASS:
		  /* The upper 8 bytes of the same %xmm register written by
		     the preceding SSE class (e.g. the high half of a
		     binary128 long double).  */
		  ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) &reg_ವಾದಗಳು->sse[ssecount - 1] + 8, a,
			  ಗಾತ್ರ_2 < 8 ? ಗಾತ್ರ_2 : 8);
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ X86_64_INTEGER_CLASS:
		ಸ್ಥಿತಿ X86_64_INTEGERSI_CLASS:
		  /* Sign-extend integer arguments passed in general
		     purpose registers, to cope with the fact that
		     LLVM incorrectly assumes that this will be done
		     (the x86-64 PS ABI does not specify this). */
		  ಆಯ್ಕೆ (arg_types[i]->type)
		    {
		    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		      reg_ವಾದಗಳು->gpr[gprcount] = (SINT64) *((SINT8 *) a);
		      ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		      reg_ವಾದಗಳು->gpr[gprcount] = (SINT64) *((SINT16 *) a);
		      ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
		      reg_ವಾದಗಳು->gpr[gprcount] = (SINT64) *((SINT32 *) a);
		      ನಿಲ್ಲಿಸು;
		    ಪೂರ್ವನಿಯೋಜಿತ:
		      reg_ವಾದಗಳು->gpr[gprcount] = 0;
		      ಸ್ಮರಣೆ_ನಕಲು (&reg_ವಾದಗಳು->gpr[gprcount], a, ಗಾತ್ರ_2 <= 8 ? ಗಾತ್ರ_2 : 8);
		    }
		  gprcount++;
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ X86_64_SSE_CLASS:
		ಸ್ಥಿತಿ X86_64_SSEDF_CLASS:
		  ಸ್ಮರಣೆ_ನಕಲು (&reg_ವಾದಗಳು->sse[ssecount++].i64, a, ಗಾತ್ರ(UINT64));
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ X86_64_SSESF_CLASS:
		  ಸ್ಮರಣೆ_ನಕಲು (&reg_ವಾದಗಳು->sse[ssecount++].i32, a, ಗಾತ್ರ(UINT32));
		  ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
		  abort();
		}
	    }
	}
    }
  reg_ವಾದಗಳು->rax = ssecount;

  ffi_call_unix64 (ರಾಶಿ, cif->ಬೈಟುಗಳು + ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು),
		   flags, rvalue, fn);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
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

ಎಣಿಕೆ ffi_move_op
{
  FFI_MOVE_SE8, FFI_MOVE_SE16, FFI_MOVE_SE32,  /* sign-extend N bytes -> gpr   */
  FFI_MOVE_GP64,                               /* copy a full 8-byte word -> gpr */
  FFI_MOVE_GP,                                 /* zero gpr, copy len(<8) bytes  */
  FFI_MOVE_SSE64, FFI_MOVE_SSE32,              /* copy 8/4 bytes -> sse slot    */
  FFI_MOVE_ರಾಶಿ                               /* copy len bytes -> stack       */
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
  ಚಿಹ್ನೆರಹಿತ src_idx;     /* avalue[] index                                  */
  ಚಿಹ್ನೆರಹಿತ src_off;     /* byte offset within avalue[src_idx] (chunk * 8)  */
  ಚಿಹ್ನೆರಹಿತ dst_off;     /* byte offset within the register_args+stack buf  */
  ಚಿಹ್ನೆರಹಿತ len;         /* bytes for FFI_MOVE_GP / FFI_MOVE_STACK          */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ op;
} ffi_move;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
  ಚಿಹ್ನೆರಹಿತ nmoves;
  ಚಿಹ್ನೆರಹಿತ ssecount;    /* -> reg_args->rax                                */
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು;       /* stack-arg area size (== cif->bytes)             */
  ಚಿಹ್ನೆರಹಿತ flags;       /* == cif->flags                                   */
  ಚಿಹ್ನೆರಹಿತ ret_ಒಳ_mem;  /* nonzero -> reg_args->gpr[0] = rvalue            */
  ಚಿಹ್ನೆರಹಿತ fast;        /* nonzero -> lean trampoline eligible             */
  ಚಿಹ್ನೆರಹಿತ retcode;     /* UNIX64_RET_* (low byte of flags) for the store  */
  ಪೂರ್ಣಾಂಕ      thunk_n;     /* >=0 -> ffi_gp_thunks[thunk_n], else -1          */
  ಚಿಹ್ನೆರಹಿತ alloc_ಬೈಟುಗಳು; /* malloc'd size, reported by ffi_call_plan_size   */
  ffi_move ಚಲನೆಗಳು[];
} ffi_plan;

/* Return of the lean trampoline / direct thunks: callee's rax in .i, xmm0 in .d. */
ರಚನೆ ffi_ret2 { UINT64 i; ದ್ವಿಗುಣ d; };
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_fast_call (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *img,
					   ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ)) FFI_HIDDEN;

/* Count-based direct thunks: load avalue[0..N-1] into arg registers, call. */
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp0 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp1 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp2 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp3 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp4 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp5 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಬಾಹ್ಯ ರಚನೆ ffi_ret2 ffi_plan_gp6 (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) FFI_HIDDEN;
ಸ್ಥಿರ ರಚನೆ ffi_ret2 (*ಸ್ಥಿರಾಂಕ ffi_gp_thunks[7]) (ಶೂನ್ಯ **, ಶೂನ್ಯ (*)(ಶೂನ್ಯ)) =
  { ffi_plan_gp0, ffi_plan_gp1, ffi_plan_gp2, ffi_plan_gp3,
    ffi_plan_gp4, ffi_plan_gp5, ffi_plan_gp6 };

/* Store the callee return value, replicating the unix64.S store_table widths. */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
store_ret (ಶೂನ್ಯ *rvalue, ಚಿಹ್ನೆರಹಿತ retcode, ರಚನೆ ffi_ret2 r)
{
  ಆಯ್ಕೆ (retcode)
    {
    ಸ್ಥಿತಿ UNIX64_RET_VOID:   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_UINT8:  *(UINT64 *) rvalue = (UINT8)  r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_UINT16: *(UINT64 *) rvalue = (UINT16) r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_UINT32: *(UINT64 *) rvalue = (UINT32) r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_SINT8:  *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT8)  r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_SINT16: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT16) r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_SINT32: *(UINT64 *) rvalue = (UINT64)(SINT64)(SINT32) r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_INT64:  *(UINT64 *) rvalue = r.i; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_XMM32:  ಸ್ಮರಣೆ_ನಕಲು (rvalue, &r.d, 4); ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ UNIX64_RET_XMM64:  ಸ್ಮರಣೆ_ನಕಲು (rvalue, &r.d, 8); ನಿಲ್ಲಿಸು;
    }
}

/* Build the move-list for CIF, or NULL if not plan-able (caller falls back). */
ಸ್ಥಿರ ffi_plan *
build_plan (ffi_cif *cif)
{
  ಚಿಹ್ನೆರಹಿತ i, avn = cif->nargs;
  ಎಣಿಕೆ x86_64_reg_class classes[MAX_CLASSES];
  ಚಿಹ್ನೆರಹಿತ nm, gprcount, ssecount;
  size_t argp_off, nbytes;
  ffi_plan *plan;
  ಪೂರ್ಣಾಂಕ all_gp64 = 1;	/* every arg is exactly one 64-bit GP move? */

  ಆದರೆ (cif->abi != FFI_UNIX64)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  /* Reject arg types this cut doesn't encode; returns are handled by flags. */
  ಗಾಗಿ (i = 0; i < avn; i++)
    {
      ಪೂರ್ಣಾಂಕ t = cif->arg_types[i]->type;
      ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಆದರೆ (t == FFI_TYPE_LONGDOUBLE)
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
    }

  /* One self-contained allocation: header + moves, released with plain free(). */
  nbytes = ಗಾತ್ರ (ffi_plan) + ಗಾತ್ರ (ffi_move) * (2 * avn + 1);
  plan = ಸ್ಮರಣೆ_ಕೊಡು (nbytes);
  ಆದರೆ (plan == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  plan->alloc_ಬೈಟುಗಳು = (ಚಿಹ್ನೆರಹಿತ) nbytes;

  nm = gprcount = ssecount = 0;
  argp_off = 0;
  plan->ret_ಒಳ_mem = (cif->flags & UNIX64_FLAG_RET_ಒಳ_MEM) ? 1 : 0;
  ಆದರೆ (plan->ret_ಒಳ_mem)
    gprcount++;				/* sret pointer occupies gpr[0] */

  ಗಾಗಿ (i = 0; i < avn; i++)
    {
      ffi_type *at = cif->arg_types[i];
      size_t ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2, n, rem;
      ಪೂರ್ಣಾಂಕ ngpr, nsse;
      ಚಿಹ್ನೆರಹಿತ j;

      n = examine_ವಾದ (at, classes, 0, &ngpr, &nsse);
      ಆದರೆ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ದೀರ್ಘ align = at->alignment;
	  ffi_move *m = &plan->ಚಲನೆಗಳು[nm++];
	  all_gp64 = 0;
	  ಆದರೆ (align < 8)
	    align = 8;
	  argp_off = FFI_ALIGN (argp_off, align);
	  m->op = FFI_MOVE_ರಾಶಿ;
	  m->src_idx = i;
	  m->src_off = 0;
	  m->dst_off = (ಚಿಹ್ನೆರಹಿತ) (ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು) + argp_off);
	  m->len = (ಚಿಹ್ನೆರಹಿತ) ಗಾತ್ರ_2;
	  argp_off += ಗಾತ್ರ_2;
	  ಮುಂದುವರಿ;
	}

      ಗಾಗಿ (j = 0, rem = ಗಾತ್ರ_2; j < n; j++, rem -= 8)
	{
	  ffi_move m;
	  m.src_idx = i;
	  m.src_off = j * 8;
	  ಆಯ್ಕೆ (classes[j])
	    {
	    ಸ್ಥಿತಿ X86_64_NO_CLASS:
	    ಸ್ಥಿತಿ X86_64_SSEUP_CLASS:
	      ಮುಂದುವರಿ;			/* nothing placed for this 8-byte */
	    ಸ್ಥಿತಿ X86_64_INTEGER_CLASS:
	    ಸ್ಥಿತಿ X86_64_INTEGERSI_CLASS:
	      m.dst_off = gprcount * 8;	/* offsetof(register_args,gpr) == 0 */
	      ಆಯ್ಕೆ (at->type)
		{
		ಸ್ಥಿತಿ FFI_TYPE_SINT8:  m.op = FFI_MOVE_SE8;  all_gp64 = 0; ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FFI_TYPE_SINT16: m.op = FFI_MOVE_SE16; all_gp64 = 0; ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FFI_TYPE_SINT32: m.op = FFI_MOVE_SE32; all_gp64 = 0; ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
		  ಆದರೆ (rem >= 8)
		    m.op = FFI_MOVE_GP64;
		  ಇಲ್ಲದಿದ್ದರೆ
		    { m.op = FFI_MOVE_GP; m.len = (ಚಿಹ್ನೆರಹಿತ) rem; all_gp64 = 0; }
		  ನಿಲ್ಲಿಸು;
		}
	      gprcount++;
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ X86_64_SSE_CLASS:
	    ಸ್ಥಿತಿ X86_64_SSEDF_CLASS:
	      m.dst_off = (ಚಿಹ್ನೆರಹಿತ) (offsetof (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು, sse)
				      + ssecount * ಗಾತ್ರ (ಒಕ್ಕೂಟ big_int_union));
	      m.op = FFI_MOVE_SSE64;
	      ssecount++;
	      all_gp64 = 0;
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ X86_64_SSESF_CLASS:
	      m.dst_off = (ಚಿಹ್ನೆರಹಿತ) (offsetof (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು, sse)
				      + ssecount * ಗಾತ್ರ (ಒಕ್ಕೂಟ big_int_union));
	      m.op = FFI_MOVE_SSE32;
	      ssecount++;
	      all_gp64 = 0;
	      ನಿಲ್ಲಿಸು;
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      ಬಿಡುಗಡೆ (plan);		/* X87 etc. in registers: bail */
	      ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	    }
	  plan->ಚಲನೆಗಳು[nm++] = m;
	}
    }

  plan->nmoves = nm;
  plan->ssecount = ssecount;
  plan->ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  plan->flags = cif->flags;
  plan->retcode = cif->flags & 0xff;	/* UNIX64_RET_* */
  /* Lean-trampoline eligible: no spilled stack args and a simple return
     (VOID..XMM64, codes 0..9; RET_IN_MEM has low byte VOID).  Struct-in-regs
     (>=12) and x87 (10,11) returns stay on ffi_call_unix64. */
  plan->fast = (cif->ಬೈಟುಗಳು == 0 && plan->retcode <= UNIX64_RET_XMM64) ? 1 : 0;
  /* Pure-GP64 direct thunk: every arg is one 64-bit GP value (so a plain load
     per arg is exact), <=6 of them, no sret, simple return -> load avalue
     straight into the arg registers, no register image. */
  plan->thunk_n =
    (all_gp64 && !plan->ret_ಒಳ_mem && nm == avn && avn <= MAX_GPR_REGS
     && plan->fast)
    ? (ಪೂರ್ಣಾಂಕ) avn : -1;
  ಹಿಂತಿರುಗಿ plan;
}

/* Execute PLAN: rebuild register_args + stack buffer, then ffi_call_unix64. */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಸಾಲೊಳಗೆ __attribute__ ((always_inline)) ಶೂನ್ಯ
plan_exec (ffi_cif *cif, ffi_plan *plan, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ),
	   ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಚಿಹ್ನೆರಹಿತ flags = plan->flags;
  ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು local __attribute__ ((aligned (16)));
  ಅಕ್ಷರ *ರಾಶಿ = ಖಾಲಿ;
  ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *reg_ವಾದಗಳು;
  ಚಿಹ್ನೆರಹಿತ k;

  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ (flags & UNIX64_FLAG_RET_ಒಳ_MEM)
	rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
      ಇಲ್ಲದಿದ್ದರೆ
	flags = UNIX64_RET_VOID;
    }

  ಆದರೆ (plan->thunk_n >= 0)
    {
      /* Pure-GP64: load avalue straight into arg regs, no image at all. */
      ರಚನೆ ffi_ret2 r = ffi_gp_thunks[plan->thunk_n] (avalue, fn);
      ಆದರೆ (rvalue != ಖಾಲಿ)
	store_ret (rvalue, plan->retcode, r);
      ಹಿಂತಿರುಗಿ;
    }

  ಆದರೆ (plan->fast)
    reg_ವಾದಗಳು = &local;			/* no stack args: fixed local image */
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ರಾಶಿ = alloca (ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು) + plan->ಬೈಟುಗಳು + 4 * 8);
      reg_ವಾದಗಳು = (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *) ರಾಶಿ;
    }
  reg_ವಾದಗಳು->r10 = 0;			/* closure (none for ffi_call) */
  ಆದರೆ (plan->ret_ಒಳ_mem)
    reg_ವಾದಗಳು->gpr[0] = (UINT64) (uintptr_t) rvalue;

  ಗಾಗಿ (k = 0; k < plan->nmoves; k++)
    {
      ffi_move *m = &plan->ಚಲನೆಗಳು[k];
      ಅಕ್ಷರ *src = (ಅಕ್ಷರ *) avalue[m->src_idx] + m->src_off;
      ಅಕ್ಷರ *dst = (ಅಕ್ಷರ *) reg_ವಾದಗಳು + m->dst_off;
      ಆಯ್ಕೆ (m->op)
	{
	/* x86-64: unaligned scalar loads from avalue[] are fine. */
	ಸ್ಥಿತಿ FFI_MOVE_SE8:   *(UINT64 *) dst = (UINT64) (SINT64) *(SINT8 *)  src; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_SE16:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT16 *) src; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_SE32:  *(UINT64 *) dst = (UINT64) (SINT64) *(SINT32 *) src; ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_GP64:  *(UINT64 *) dst = *(UINT64 *) src;                   ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_GP:    *(UINT64 *) dst = 0; ಸ್ಮರಣೆ_ನಕಲು (dst, src, m->len);     ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_SSE64: *(UINT64 *) dst = *(UINT64 *) src;                   ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_SSE32: *(UINT32 *) dst = *(UINT32 *) src;                   ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_MOVE_ರಾಶಿ: ಸ್ಮರಣೆ_ನಕಲು (dst, src, m->len);                          ನಿಲ್ಲಿಸು;
	}
    }
  reg_ವಾದಗಳು->rax = plan->ssecount;

  ಆದರೆ (plan->fast)
    {
      /* No stack args; lean trampoline + return store replicating the
	 unix64.S store_table widths.  ret_in_mem already wrote gpr[0]. */
      ರಚನೆ ffi_ret2 r = ffi_plan_fast_call (reg_ವಾದಗಳು, fn);
      ಆದರೆ (rvalue != ಖಾಲಿ)
	store_ret (rvalue, plan->retcode, r);
      ಹಿಂತಿರುಗಿ;
    }

  ffi_call_unix64 (ರಾಶಿ, plan->ಬೈಟುಗಳು + ಗಾತ್ರ (ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು),
		   flags, rvalue, fn);
}

/* Reusable call plan: an opaque, caller-owned handle wrapping a prebuilt plan.
   ffi_call_plan_invoke applies it directly, skipping the per-call argument
   classification ffi_call does every time.  Signatures with no fast path
   (FAST is NULL) fall back to ffi_call.  The plan is immutable after alloc, so
   it carries no per-thread state and can be invoked from any thread.  */
ರಚನೆ ffi_call_plan
{
  ffi_cif  *cif;
  ffi_plan *fast;		/* prebuilt plan, or NULL -> fall back to ffi_call */
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ (ರಚನೆ ffi_call_plan));
  ಆದರೆ (plan == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;
  plan->cif  = cif;
  plan->fast = build_plan (cif);	/* NULL if this signature has no fast path */
  ಹಿಂತಿರುಗಿ plan;
}

ಶೂನ್ಯ
ffi_call_plan_invoke (ffi_call_plan *plan, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ),
		      ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಆದರೆ (plan->fast != ಖಾಲಿ)
    plan_exec (plan->cif, plan->fast, fn, rvalue, avalue);
  ಇಲ್ಲದಿದ್ದರೆ
    ffi_call (plan->cif, fn, rvalue, avalue);
}

ಶೂನ್ಯ
ffi_call_plan_free (ffi_call_plan *plan)
{
  ಆದರೆ (plan != ಖಾಲಿ)
    {
      ಬಿಡುಗಡೆ (plan->fast);
      ಬಿಡುಗಡೆ (plan);
    }
}

size_t
ffi_call_plan_ಗಾತ್ರ (ffi_call_plan *plan)
{
  ಆದರೆ (plan == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;
  /* The move-list carries its own size; a signature with no fast path owns
     nothing beyond the handle.  */
  ಹಿಂತಿರುಗಿ ಗಾತ್ರ (ರಚನೆ ffi_call_plan)
	 + (plan->fast != ಖಾಲಿ ? plan->fast->alloc_ಬೈಟುಗಳು : 0);
}

ಬಾಹ್ಯ ಶೂನ್ಯ
ffi_call_efi64(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue);
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;
  ಪೂರ್ಣಾಂಕ i, nargs = cif->nargs;
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ max_reg_struct_ಗಾತ್ರ = cif->abi == FFI_GNUW64 ? 8 : 16;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT && ಗಾತ್ರ_2 > max_reg_struct_ಗಾತ್ರ)
        {
          ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
          ಆದರೆ (avalue_copy == ಖಾಲಿ)
            {
              avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
          avalue[i] = argcopy;
        }
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
  ಆದರೆ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_efi64(cif, fn, rvalue, avalue);
      ಹಿಂತಿರುಗಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
ಬಾಹ್ಯ ಶೂನ್ಯ
ffi_call_go_efi64(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
		  ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ);
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
  ಆದರೆ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    {
      ffi_call_go_efi64(cif, fn, rvalue, avalue, ಆವರಣ);
      ಹಿಂತಿರುಗಿ;
    }
#ಆದರೆ_ಅಂತ್ಯ
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_GO_CLOSURES */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_unix64(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_unix64_sse(ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_unix64_alt(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_unix64_sse_alt(ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
ಬಾಹ್ಯ ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc_efi64(ffi_ಆವರಣ* ಆವರಣ,
			   ffi_cif* cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			   ಶೂನ್ಯ *codeloc);
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ trampoline[24] = {
    /* endbr64 */
    0xf3, 0x0f, 0x1e, 0xfa,
    /* leaq  -0xb(%rip),%r10   # 0x0  */
    0x4c, 0x8d, 0x15, 0xf5, 0xff, 0xff, 0xff,
    /* jmpq  *0x7(%rip)        # 0x18 */
    0xff, 0x25, 0x07, 0x00, 0x00, 0x00,
    /* nopl  0(%rax) */
    0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00
  };
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  ಅಕ್ಷರ *tramp = ಆವರಣ->tramp;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
  ಆದರೆ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ಹಿಂತಿರುಗಿ ffi_prep_ಆವರಣ_loc_efi64(ಆವರಣ, cif, fun, user_ದತ್ತಾಂಶ, codeloc);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (cif->abi != FFI_UNIX64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆದರೆ (cif->flags & UNIX64_FLAG_XMM_ವಾದಗಳು)
    dest = ffi_ಆವರಣ_unix64_sse;
  ಇಲ್ಲದಿದ್ದರೆ
    dest = ffi_ಆವರಣ_unix64;

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಆದರೆ (dest == ffi_ಆವರಣ_unix64_sse)
        dest = ffi_ಆವರಣ_unix64_sse_alt;
      ಇಲ್ಲದಿದ್ದರೆ
        dest = ffi_ಆವರಣ_unix64_alt;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, dest, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the dynamic trampoline. */
  ಸ್ಮರಣೆ_ನಕಲು (tramp, trampoline, ಗಾತ್ರ(trampoline));
  *(UINT64 *)(tramp + ಗಾತ್ರ (trampoline)) = (uintptr_t)dest;

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_unix64_inner(ffi_cif *cif,
			 ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			 ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			 ಶೂನ್ಯ *rvalue,
			 ರಚನೆ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು *reg_ವಾದಗಳು,
			 ಅಕ್ಷರ *argp)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **arg_types;
  ದೀರ್ಘ i, avn;
  ಪೂರ್ಣಾಂಕ gprcount, ssecount, ngpr, nsse;
  ಪೂರ್ಣಾಂಕ flags;

  avn = cif->nargs;
  flags = cif->flags;

  avalue = alloca(avn * ಗಾತ್ರ(ಶೂನ್ಯ *));
  gprcount = ssecount = 0;

  ಆದರೆ (flags & UNIX64_FLAG_RET_ಒಳ_MEM)
    {
      /* On return, %rax will contain the address that was passed
	 by the caller in %rdi.  */
      ಶೂನ್ಯ *r = (ಶೂನ್ಯ *)(uintptr_t)reg_ವಾದಗಳು->gpr[gprcount++];
      *(ಶೂನ್ಯ **)rvalue = r;
      rvalue = r;
      flags = (ಗಾತ್ರ(ಶೂನ್ಯ *) == 4 ? UNIX64_RET_UINT32 : UNIX64_RET_INT64);
    }

  arg_types = cif->arg_types;
  ಗಾಗಿ (i = 0; i < avn; ++i)
    {
      ಎಣಿಕೆ x86_64_reg_class classes[MAX_CLASSES];
      size_t n;

      n = examine_ವಾದ (arg_types[i], classes, 0, &ngpr, &nsse);
      ಆದರೆ (n == 0
	  || gprcount + ngpr > MAX_GPR_REGS
	  || ssecount + nsse > MAX_SSE_REGS)
	{
	  ದೀರ್ಘ align = arg_types[i]->alignment;

	  /* Stack arguments are *always* at least 8 byte aligned.  */
	  ಆದರೆ (align < 8)
	    align = 8;

	  /* Pass this argument in memory.  */
	  argp = (ಶೂನ್ಯ *) FFI_ALIGN (argp, align);
	  avalue[i] = argp;
	  argp += arg_types[i]->ಗಾತ್ರ_2;
	}
      /* If the argument is in a single register, or two consecutive
	 integer registers, then we can use that address directly.  */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (n == 1
	       || (n == 2 && !(SSE_CLASS_P (classes[0])
			       || SSE_CLASS_P (classes[1]))))
	{
	  /* The argument is in a single register.  */
	  ಆದರೆ (SSE_CLASS_P (classes[0]))
	    {
	      avalue[i] = &reg_ವಾದಗಳು->sse[ssecount];
	      ssecount += n;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = &reg_ವಾದಗಳು->gpr[gprcount];
	      gprcount += n;
	    }
	}
      /* Otherwise, allocate space to make them consecutive.  */
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಅಕ್ಷರ *a = alloca (n * 8);
	  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;

	  avalue[i] = a;
	  ಗಾಗಿ (j = 0; j < n; j++, a += 8)
	    {
	      ಆದರೆ (classes[j] == X86_64_SSEUP_CLASS)
		/* The high half of the same %xmm register as the preceding
		   SSE class (e.g. the upper bits of a binary128 long double);
		   it does not consume another register.  */
		ಸ್ಮರಣೆ_ನಕಲು (a, (ಅಕ್ಷರ *) &reg_ವಾದಗಳು->sse[ssecount - 1] + 8, 8);
	      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (SSE_CLASS_P (classes[j]))
		ಸ್ಮರಣೆ_ನಕಲು (a, &reg_ವಾದಗಳು->sse[ssecount++], 8);
	      ಇಲ್ಲದಿದ್ದರೆ
		ಸ್ಮರಣೆ_ನಕಲು (a, &reg_ವಾದಗಳು->gpr[gprcount++], 8);
	    }
	}
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell assembly how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ flags;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_unix64(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_unix64_sse(ಶೂನ್ಯ) FFI_HIDDEN;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
ಬಾಹ್ಯ ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ_efi64(ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
			  ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*));
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __ILP32__
  ಆದರೆ (cif->abi == FFI_EFI64 || cif->abi == FFI_GNUW64)
    ಹಿಂತಿರುಗಿ ffi_prep_go_ಆವರಣ_efi64(ಆವರಣ, cif, fun);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (cif->abi != FFI_UNIX64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = (cif->flags & UNIX64_FLAG_XMM_ವಾದಗಳು
		    ? ffi_go_ಆವರಣ_unix64_sse
		    : ffi_go_ಆವರಣ_unix64);
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_GO_CLOSURES */

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *map_ಗಾತ್ರ = UNIX64_TRAMP_MAP_ಗಾತ್ರ;
  *tramp_ಗಾತ್ರ = UNIX64_TRAMP_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* __x86_64__ */

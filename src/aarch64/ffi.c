/* Copyright (c) 2009, 2010, 2011, 2012 ARM Ltd.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
``Software''), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#अगर defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <stdint.h>
#समावेश <fficonfig.h>
#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश "internal.h"
#अगर_घोषित _WIN32
#घोषणा WIN32_LEAN_AND_MEAN
#समावेश <windows.h> /* FlushInstructionCache */
#पूर्ण_अगर
#समावेश <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#अगर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# अगर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#पूर्ण_अगर

जोड़ _d
{
  UINT64 d;
  UINT32 s[2];
};

काष्ठा _v
{
  जोड़ _d d[2] __attribute__((aligned(16)));
};

काष्ठा call_context
{
  काष्ठा _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#अगर FFI_EXEC_TRAMPOLINE_TABLE

#अगर_घोषित __MACH__
#अगर_घोषित HAVE_ARM64E_PTRAUTH
#समावेश <ptrauth.h>
#पूर्ण_अगर
#समावेश <mach/vm_param.h>
#पूर्ण_अगर

#अन्यथा

#अगर defined (__clang__) && defined (__APPLE__)
बाह्य व्योम sys_icache_invalidate (व्योम *start, माप_प्रकार len);
#पूर्ण_अगर

अटल अंतरभूत व्योम
ffi_clear_cache (व्योम *start, व्योम *end)
{
#अगर defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (अक्षर *)end - (अक्षर *)start);
#या_अगर defined (__GNUC__)
  __builtin___clear_cache (start, end);
#या_अगर defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (अक्षर*)end - (अक्षर*)start);
#अन्यथा
#त्रुटि "Missing builtin to flush instruction cache"
#पूर्ण_अगर
}

#पूर्ण_अगर

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

अटल पूर्णांक
intlog2 (पूर्णांक n)
{
  पूर्णांक level = 0;
  जबतक (n >>= 1)
    ++level;
  वापस level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

अटल पूर्णांक
is_hfa0 (स्थिर ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i, ret = -1;

  अगर (elements != शून्य)
    क्रम (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        अगर (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            अगर (ret < 0)
              जारी;
          }
        अवरोध;
      }

  वापस ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

अटल माप_प्रकार
is_simd (स्थिर ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक i;

  अगर (ty->type == FFI_TYPE_VECTOR)
    वापस ty->आकार;

  elements = ty->elements;
  अगर (elements != शून्य)
    क्रम (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          वापस is_simd (elements[i]);
      }

  वापस 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

अटल पूर्णांक
is_hfa1 (स्थिर ffi_type *ty, पूर्णांक candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i;

  अगर (elements != शून्य)
    क्रम (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            अगर (!is_hfa1 (elements[i], candidate))
              वापस 0;
          }
        अन्यथा अगर (t != candidate)
          वापस 0;
      }

  वापस 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

अटल पूर्णांक
is_vfp_type (स्थिर ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक candidate, i;
  माप_प्रकार आकार, ele_गिनती, simd_आकार;

  /* Quickest tests first.  */
  candidate = ty->type;
  चयन (candidate)
    {
    शेष:
      वापस 0;
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_LONGDOUBLE:
      ele_गिनती = 1;
      जाओ done;
    हाल FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      चयन (candidate)
	{
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	  ele_गिनती = 2;
	  जाओ done;
	}
      वापस 0;
    हाल FFI_TYPE_STRUCT:
    हाल FFI_TYPE_VECTOR:
      अवरोध;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  आकार = ty->आकार;
  अगर (आकार < 4 || आकार > 64)
    वापस 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_आकार = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  अगर (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      क्रम (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          अगर (candidate >= 0)
            अवरोध;
        }
    }

  अगर (simd_आकार)
    {
      /* Vector or homogeneous vector aggregate (HVA).  A single Neon slot is
	 at most 16 bytes (a Q register).  A bare vector wider than 16 bytes
	 (e.g. a 32-byte double4) has no short-vector register class under
	 AAPCS64, so bail and let the generic composite path pass it by
	 reference / return it in memory -- matching what current compilers do.
	 The scalar lane type does not affect register selection (an integer
	 and a floating-point 16-byte vector both occupy one Q register), so,
	 unlike the floating-point HFA path below, CANDIDATE is used only to
	 confirm the lanes are homogeneous.  */
      माप_प्रकार reg_आकार = simd_आकार;
      पूर्णांक अंक_registers;
      पूर्णांक first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      अगर (reg_आकार < 4 || reg_आकार > 16 || आकार % reg_आकार != 0)
	वापस 0;
      अंक_registers = (पूर्णांक) (आकार / reg_आकार);
      अगर (अंक_registers > 4)
	वापस 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      अगर (ty->type != FFI_TYPE_VECTOR)
	क्रम (i = 0; elements[i]; ++i)
	  अगर (is_simd (elements[i]) != reg_आकार)
	    वापस 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      क्रम (i = 0; elements[i]; ++i)
	{
	  पूर्णांक t = elements[i]->type;
	  अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      अगर (!is_hfa1 (elements[i], candidate))
		वापस 0;
	    }
	  अन्यथा अगर (t != candidate)
	    वापस 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((पूर्णांक) reg_आकार) - 2;
      वापस first_level_element_type * 4 + (4 - अंक_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  चयन (candidate)
    {
    हाल FFI_TYPE_FLOAT:
      ele_गिनती = आकार / माप(भग्न);
      अगर (आकार != ele_गिनती * माप(भग्न))
        वापस 0;
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      ele_गिनती = आकार / माप(द्विगुन);
      अगर (आकार != ele_गिनती * माप(द्विगुन))
        वापस 0;
      अवरोध;
    हाल FFI_TYPE_LONGDOUBLE:
      ele_गिनती = आकार / माप(दीर्घ द्विगुन);
      अगर (आकार != ele_गिनती * माप(दीर्घ द्विगुन))
        वापस 0;
      अवरोध;
    शेष:
      वापस 0;
    }
  अगर (ele_गिनती > 4)
    वापस 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  क्रम (i = 0; elements[i]; ++i)
    {
      पूर्णांक t = elements[i]->type;
      अगर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          अगर (!is_hfa1 (elements[i], candidate))
            वापस 0;
        }
      अन्यथा अगर (t != candidate)
        वापस 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  वापस candidate * 4 + (4 - (पूर्णांक)ele_गिनती);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

काष्ठा arg_state
{
  अचिन्हित ngrn;                /* Next general-purpose register number. */
  अचिन्हित nsrn;                /* Next vector register number. */
  माप_प्रकार nsaa;                  /* Next stack offset. */
  माप_प्रकार अग्र_struct_area;	/* Place to allocate big structs. */

#अगर defined (__APPLE__)
  अचिन्हित allocating_variadic;
#पूर्ण_अगर
};

/* Initialize a procedure call argument marshalling state.  */
अटल व्योम
arg_init (काष्ठा arg_state *state, माप_प्रकार आकार)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->अग्र_struct_area = आकार;
#अगर defined (__APPLE__)
  state->allocating_variadic = 0;
#पूर्ण_अगर
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
अटल व्योम *
allocate_to_ढेर (काष्ठा arg_state *state, व्योम *ढेर,
		   माप_प्रकार alignment, माप_प्रकार आकार)
{
  माप_प्रकार nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#अगर defined (__APPLE__)
  अगर (state->allocating_variadic && alignment < 8)
    alignment = 8;
#अन्यथा
  अगर (alignment < 8)
    alignment = 8;
#पूर्ण_अगर

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + आकार;

  वापस (अक्षर *)ढेर + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
अटल व्योम *
allocate_and_copy_struct_to_ढेर (काष्ठा arg_state *state, व्योम *ढेर,
				   माप_प्रकार alignment, माप_प्रकार आकार, व्योम *मूल्य)
{
  माप_प्रकार dest = state->अग्र_struct_area - आकार;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->अग्र_struct_area = dest;

  वापस स_नकल ((अक्षर *) ढेर + dest, मूल्य, आकार);
}

अटल ffi_arg
extend_integer_type (व्योम *source, पूर्णांक type)
{
  चयन (type)
    {
    हाल FFI_TYPE_UINT8:
      {
        UINT8 u8;
        स_नकल (&u8, source, माप (u8));
        वापस u8;
      }
    हाल FFI_TYPE_SINT8:
      {
        SINT8 s8;
        स_नकल (&s8, source, माप (s8));
        वापस s8;
      }
    हाल FFI_TYPE_UINT16:
      {
        UINT16 u16;
        स_नकल (&u16, source, माप (u16));
        वापस u16;
      }
    हाल FFI_TYPE_SINT16:
      {
        SINT16 s16;
        स_नकल (&s16, source, माप (s16));
        वापस s16;
      }
    हाल FFI_TYPE_UINT32:
      {
        UINT32 u32;
        स_नकल (&u32, source, माप (u32));
        वापस u32;
      }
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      {
        SINT32 s32;
        स_नकल (&s32, source, माप (s32));
        वापस s32;
      }
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      {
        UINT64 u64;
        स_नकल (&u64, source, माप (u64));
        वापस u64;
      }
    हाल FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        स_नकल (&uptr, source, माप (uptr));
        वापस uptr;
      }
    शेष:
      पात();
    }
}

#अगर defined(_MSC_VER) && !defined(__clang__)
व्योम extend_hfa_type (व्योम *dest, व्योम *src, पूर्णांक h);
#अन्यथा
अटल व्योम
extend_hfa_type (व्योम *dest, व्योम *src, पूर्णांक h)
{
  सूचक_भेद_प्रकार f = h - AARCH64_RET_S4;
  व्योम *x0;

#घोषणा BTI_J "hint #36"
  __asm__ अस्थिर (
	"adr	%0, 0f\n"
"	add	%0, %0, %1\n"
"	br	%0\n"
"0:	"BTI_J"\n"			/* S4 */
"	ldp	s16, s17, [%3]\n"
"	ldp	s18, s19, [%3, #8]\n"
"	b	4f\n"
"	"BTI_J"\n"			/* S3 */
"	ldp	s16, s17, [%3]\n"
"	ldr	s18, [%3, #8]\n"
"	b	3f\n"
"	"BTI_J"\n"			/* S2 */
"	ldp	s16, s17, [%3]\n"
"	b	2f\n"
"	nop\n"
"	"BTI_J"\n"			/* S1 */
"	ldr	s16, [%3]\n"
"	b	1f\n"
"	nop\n"
"	"BTI_J"\n"			/* D4 */
"	ldp	d16, d17, [%3]\n"
"	ldp	d18, d19, [%3, #16]\n"
"	b	4f\n"
"	"BTI_J"\n"			/* D3 */
"	ldp	d16, d17, [%3]\n"
"	ldr	d18, [%3, #16]\n"
"	b	3f\n"
"	"BTI_J"\n"			/* D2 */
"	ldp	d16, d17, [%3]\n"
"	b	2f\n"
"	nop\n"
"	"BTI_J"\n"			/* D1 */
"	ldr	d16, [%3]\n"
"	b	1f\n"
"	nop\n"
"	"BTI_J"\n"			/* Q4 */
"	ldp	q16, q17, [%3]\n"
"	ldp	q18, q19, [%3, #32]\n"
"	b	4f\n"
"	"BTI_J"\n"			/* Q3 */
"	ldp	q16, q17, [%3]\n"
"	ldr	q18, [%3, #32]\n"
"	b	3f\n"
"	"BTI_J"\n"			/* Q2 */
"	ldp	q16, q17, [%3]\n"
"	b	2f\n"
"	nop\n"
"	"BTI_J"\n"			/* Q1 */
"	ldr	q16, [%3]\n"
"	b	1f\n"
"4:	str	q19, [%2, #48]\n"
"3:	str	q18, [%2, #32]\n"
"2:	str	q17, [%2, #16]\n"
"1:	str	q16, [%2]"
    : "=&r"(x0)
    : "r"(f * 16), "r"(dest), "r"(src)
    : "memory", "v16", "v17", "v18", "v19");
}
#पूर्ण_अगर

#अगर defined(_MSC_VER) && !defined(__clang__)
व्योम* compress_hfa_type (व्योम *dest, व्योम *src, पूर्णांक h);
#अन्यथा
अटल व्योम *
compress_hfa_type (व्योम *dest, व्योम *reg, पूर्णांक h)
{
  चयन (h)
    {
    हाल AARCH64_RET_S1:
      अगर (dest == reg)
	वापस (अक्षर *)dest + BE (12);
      अन्यथा
	*(भग्न *)dest = *(भग्न *)((अक्षर *)reg + BE (12));
      अवरोध;
    हाल AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      अवरोध;
    हाल AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      अवरोध;
    हाल AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      अवरोध;

    हाल AARCH64_RET_D1:
      अगर (dest == reg)
	वापस (अक्षर *)dest + BE (8);
      अन्यथा
	*(द्विगुन *)dest = *(द्विगुन *)((अक्षर *)reg + BE (8));
      अवरोध;
    हाल AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      अवरोध;
    हाल AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      अवरोध;
    हाल AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      अवरोध;

    शेष:
      अगर (dest != reg)
	वापस स_नकल (dest, reg, 16 * (4 - (h & 3)));
      अवरोध;
    }
  वापस dest;
}
#पूर्ण_अगर

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

अटल व्योम *
allocate_int_to_reg_or_ढेर (काष्ठा call_context *context,
			      काष्ठा arg_state *state,
			      व्योम *ढेर, माप_प्रकार आकार)
{
  अगर (state->ngrn < N_X_ARG_REG)
    वापस (अक्षर *)&context->x[state->ngrn++] + BE (माप (UINT64) - आकार);

  state->ngrn = N_X_ARG_REG;
  वापस (अक्षर *)allocate_to_ढेर (state, ढेर, आकार, आकार)
	 + BE (माप (UINT64) - आकार);
}

अटल व्योम *
allocate_int128_to_reg_or_ढेर (काष्ठा call_context *context,
			         काष्ठा arg_state *state, व्योम *ढेर)
{
  अचिन्हित ngrn = state->ngrn;
  व्योम *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#अगर_अघोषित __APPLE__
  ngrn += ngrn & 1;
#पूर्ण_अगर

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  अगर (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  अन्यथा
    {
      ret = allocate_to_ढेर (state, ढेर, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  वापस ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  माप_प्रकार bytes = cif->bytes;
  पूर्णांक flags, i, n;

  चयन (rtype->type)
    {
    हाल FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      अवरोध;
    हाल FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      अवरोध;
    हाल FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      अवरोध;
    हाल FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      अवरोध;
    हाल FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      अवरोध;
    हाल FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      अवरोध;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      अवरोध;
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      अवरोध;
    हाल FFI_TYPE_POINTER:
      flags = (माप(व्योम *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      अवरोध;

    हाल FFI_TYPE_UINT128:
    हाल FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      अवरोध;

    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_LONGDOUBLE:
    हाल FFI_TYPE_STRUCT:
    हाल FFI_TYPE_VECTOR:
    हाल FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      अगर (flags == 0)
	{
	  माप_प्रकार s = rtype->आकार;
	  अगर (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  अन्यथा अगर (s == 16)
	    flags = AARCH64_RET_INT128;
	  अन्यथा अगर (s == 8)
	    flags = AARCH64_RET_INT64;
	  अन्यथा
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      अवरोध;

    शेष:
      पात();
    }

  क्रम (i = 0, n = cif->nargs; i < n; i++)
    अगर (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	अवरोध;
      }

  /* Composites larger than 16 bytes (that are not HFAs) are passed by
     invisible reference: ffi_call copies the payload into the argument
     slab (next_struct_area, growing down) and, once the X registers are
     exhausted, also spills the by-ref pointer into the same slab (the
     NSAA, growing up).  The generic prep_cif accounting in cif->bytes
     only charges the payload copy, not that 8-byte pointer slot, so the
     two regions can collide and a later struct copy can overwrite an
     already-spilled pointer with the copied payload bytes.  Reserve an
     extra 8 bytes per such argument so the slab is always large enough
     for both.  */
  क्रम (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      अगर (ty->आकार > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (अचिन्हित) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#अगर defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#पूर्ण_अगर

  वापस FFI_OK;
}

#अगर defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs,
			 अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  वापस status;
}
#अन्यथा
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  वापस status;
}
#पूर्ण_अगर /* __APPLE__ */

बाह्य व्योम ffi_call_SYSV (काष्ठा call_context *context, व्योम *frame,
			   व्योम (*fn)(व्योम), व्योम *rvalue, पूर्णांक flags,
			   व्योम *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *orig_rvalue,
	      व्योम **avalue, व्योम *closure)
{
  काष्ठा call_context *context;
  व्योम *ढेर, *frame, *rvalue;
  काष्ठा arg_state state;
  माप_प्रकार ढेर_bytes, rtype_आकार, rsize;
  पूर्णांक i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_आकार = rtype->आकार;
  ढेर_bytes = cif->bytes;

  अगर (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  अगर (flags & AARCH64_RET_IN_MEM)
    {
      अगर (orig_rvalue == शून्य)
	rsize = rtype_आकार;
    }
  अन्यथा अगर (orig_rvalue == शून्य)
    flags &= AARCH64_FLAG_ARG_V;
  अन्यथा अगर (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (माप(काष्ठा call_context) + ढेर_bytes + 40 + rsize);
  ढेर = context + 1;
  frame = (व्योम*)((uintptr_t)ढेर + (uintptr_t)ढेर_bytes);
  rvalue = (rsize ? (व्योम*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, ढेर_bytes);
  क्रम (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      माप_प्रकार s = ty->आकार;
      व्योम *a = avalue[i];
      पूर्णांक h, t;
      व्योम *dest;

      t = ty->type;
      चयन (t)
	{
	हाल FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  अवरोध;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    अगर (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    अन्यथा
	      {
		व्योम *d = allocate_to_ढेर (&state, ढेर, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#अगर_घोषित __APPLE__
		स_नकल(d, a, s);
#अन्यथा
		*(ffi_arg *)d = ext;
#पूर्ण_अगर
	      }
	  }
	  अवरोध;

	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_ढेर (context, &state, ढेर);
	  स_नकल (dest, a, 16);
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	हाल FFI_TYPE_STRUCT:
	हाल FFI_TYPE_VECTOR:
	हाल FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    अगर (h)
	      {
              पूर्णांक elems = 4 - (h & 3);
              अगर (cif->abi == FFI_WIN64 && isvariadic)
              {
                अगर (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  अवरोध;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_ढेर(&state, ढेर, ty->alignment, s);
              }
              अन्यथा
              {
                अगर (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  अवरोध;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (अक्षर *)allocate_to_ढेर (&state, ढेर, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (माप (UINT64) - s) : 0);
              }
	      }
	    अन्यथा अगर (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_ढेर (&state, ढेर,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = माप (व्योम *);
		जाओ do_pointer;
	      }
	    अन्यथा
	      {
		माप_प्रकार n = (s + 7) / 8;
		अगर (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (अचिन्हित पूर्णांक)n;
		  }
		अन्यथा
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_ढेर (&state, ढेर, ty->alignment, s);
		  }
		}
	      स_नकल (dest, a, s);
	    }
	  अवरोध;

	शेष:
	  पात();
	}

#अगर defined (__APPLE__)
      अगर (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#पूर्ण_अगर
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  अगर (flags & AARCH64_RET_NEED_COPY)
    स_नकल (orig_rvalue, rvalue, rtype_आकार);
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, शून्य);
}

#अगर FFI_CLOSURES

#अगर_घोषित FFI_GO_CLOSURES
व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#पूर्ण_अगर /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

बाह्य व्योम ffi_closure_SYSV (व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_closure_SYSV_V (व्योम) FFI_HIDDEN;
#अगर defined(FFI_EXEC_STATIC_TRAMP)
बाह्य व्योम ffi_closure_SYSV_alt (व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_closure_SYSV_V_alt (व्योम) FFI_HIDDEN;
#पूर्ण_अगर

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
                      व्योम *user_तथ्य,
                      व्योम *codeloc)
{
  अगर (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    वापस FFI_BAD_ABI;

  व्योम (*start)(व्योम);

  अगर (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  अन्यथा
    start = ffi_closure_SYSV;

#अगर FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_तथ्य(codeloc, ptrauth_कुंजी_function_pointer, 0);
#  endif
  व्योम **config = (व्योम **)((uint8_t *)codeloc - PAGE_MAX_आकार);
  config[0] = closure;
  config[1] = start;
# endif
#अन्यथा
  अटल स्थिर अचिन्हित अक्षर trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  अक्षर *tramp = closure->tramp;

# अगर defined(FFI_EXEC_STATIC_TRAMP)
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      अगर (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      अन्यथा
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      जाओ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  स_नकल (tramp, trampoline, माप(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_आकार);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  अचिन्हित अक्षर *tramp_code = tramp;
# अन्यथा
  अचिन्हित अक्षर *tramp_code = ffi_तथ्य_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_आकार);
# अगर defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#पूर्ण_अगर

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

#अगर_घोषित FFI_GO_CLOSURES
बाह्य व्योम ffi_go_closure_SYSV (व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_go_closure_SYSV_V (व्योम) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*))
{
  व्योम (*start)(व्योम);

  अगर (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    वापस FFI_BAD_ABI;

  अगर (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  अन्यथा
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}
#पूर्ण_अगर /* FFI_GO_CLOSURES */

/* Primary handler to setup and invoke a function within a closure.

   A closure when invoked enters via the assembler wrapper
   ffi_closure_SYSV(). The wrapper allocates a call context on the
   stack, saves the interesting registers (from the perspective of
   the calling convention) into the context then passes control to
   ffi_closure_SYSV_inner() passing the saved context and a pointer to
   the stack at the point ffi_closure_SYSV() was invoked.

   On the return path the assembler wrapper will reload call context
   registers.

   ffi_closure_SYSV_inner() marshalls the call context into ffi value
   descriptors, invokes the wrapped function, then marshalls the return
   value back into the call context.  */

पूर्णांक FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
			व्योम *user_तथ्य,
			काष्ठा call_context *context,
			व्योम *ढेर, व्योम *rvalue, व्योम *struct_rvalue)
{
  व्योम **avalue = (व्योम**) alloca (cif->nargs * माप (व्योम*));
  पूर्णांक i, h, nargs, flags, isvariadic = 0;
  काष्ठा arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  अगर (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  क्रम (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      पूर्णांक t = ty->type;
      माप_प्रकार n, s = ty->आकार;

      चयन (t)
	{
	हाल FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  अवरोध;

	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_ढेर (context, &state, ढेर, s);
	  अवरोध;

	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_ढेर (context, &state, ढेर);
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	हाल FFI_TYPE_STRUCT:
	हाल FFI_TYPE_VECTOR:
	हाल FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  अगर (h)
	    {
	      n = 4 - (h & 3);
              अगर (cif->abi == FFI_WIN64 && isvariadic)
                {
                  अगर (state.ngrn + n <= N_X_ARG_REG)
                    {
                      व्योम *reg = &context->x[state.ngrn];
                      state.ngrn += (अचिन्हित पूर्णांक)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अन्यथा
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (अक्षर *)allocate_to_ढेर (&state, ढेर,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (माप (UINT64) - s) : 0);
                    }
                }
              अन्यथा
                {
                  अगर (state.nsrn + n <= N_V_ARG_REG)
                    {
                      व्योम *reg = &context->v[state.nsrn];
                      state.nsrn += (अचिन्हित पूर्णांक)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अन्यथा
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (अक्षर *)allocate_to_ढेर (&state, ढेर,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (माप (UINT64) - s) : 0);
                    }
                }
            }
          अन्यथा अगर (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#अगर_घोषित __ILP32__
             UINT64 avalue_tmp;
             स_नकल (&avalue_tmp,
                  allocate_int_to_reg_or_ढेर (context, &state,
                                               ढेर, माप (व्योम *)),
                  माप (UINT64));
             avalue[i] = (व्योम *)(UINT32)avalue_tmp;
#अन्यथा
              avalue[i] = *(व्योम **)
              allocate_int_to_reg_or_ढेर (context, &state, ढेर,
                                         माप (व्योम *));
#पूर्ण_अगर
            }
          अन्यथा
            {
              n = (s + 7) / 8;
              अगर (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (अचिन्हित पूर्णांक)n;
                }
              अन्यथा
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_ढेर(&state, ढेर,
                                           ty->alignment, s);
                }
            }
          अवरोध;

        शेष:
          पात();
      }

#अगर defined (__APPLE__)
      अगर (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#पूर्ण_अगर
    }

  अगर (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_तथ्य);

  वापस flags;
}

#अगर defined(FFI_EXEC_STATIC_TRAMP)
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;

  *tramp_आकार = AARCH64_TRAMP_आकार;
  *map_आकार = AARCH64_TRAMP_MAP_आकार;
  वापस &trampoline_code_table;
}
#पूर्ण_अगर

#पूर्ण_अगर /* FFI_CLOSURES */

#पूर्ण_अगर /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

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

#जइ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#अन्तर्भाव <windows.h> /* FlushInstructionCache */
#endif
#अन्तर्भाव <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#जइ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# जइ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अण्णहा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

सङ्घ _d
{
  UINT64 d;
  UINT32 s[2];
};

संरचना _v
{
  सङ्घ _d d[2] __attribute__((aligned(16)));
};

संरचना call_context
{
  संरचना _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#जइ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#अन्तर्भाव <ptrauth.h>
#endif
#अन्तर्भाव <mach/vm_param.h>
#endif

#अण्णहा

#जइ defined (__clang__) && defined (__APPLE__)
बाह्य शून्य sys_icache_invalidate (शून्य *start, size_t len);
#endif

स्थिर अन्तःस्थ शून्य
ffi_clear_cache (शून्य *start, शून्य *end)
{
#जइ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (वर्ण *)end - (वर्ण *)start);
#नोचेद्यदि defined (__GNUC__)
  __builtin___clear_cache (start, end);
#नोचेद्यदि defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (वर्ण*)end - (वर्ण*)start);
#अण्णहा
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

स्थिर पूर्णाङ्क
intlog2 (पूर्णाङ्क n)
{
  पूर्णाङ्क level = 0;
  जाव (n >>= 1)
    ++level;
  प्रत्यागम level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

स्थिर पूर्णाङ्क
is_hfa0 (ध्रुव ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णाङ्क i, ret = -1;

  जइ (elements != रिक्त)
    प्रति (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        जइ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            जइ (ret < 0)
              अनुवर्त;
          }
        भङ्ग;
      }

  प्रत्यागम ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

स्थिर size_t
is_simd (ध्रुव ffi_type *ty)
{
  ffi_type **elements;
  पूर्णाङ्क i;

  जइ (ty->type == FFI_TYPE_VECTOR)
    प्रत्यागम ty->size;

  elements = ty->elements;
  जइ (elements != रिक्त)
    प्रति (i = 0; elements[i]; ++i)
      {
        पूर्णाङ्क t = elements[i]->type;
        जइ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          प्रत्यागम is_simd (elements[i]);
      }

  प्रत्यागम 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

स्थिर पूर्णाङ्क
is_hfa1 (ध्रुव ffi_type *ty, पूर्णाङ्क candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णाङ्क i;

  जइ (elements != रिक्त)
    प्रति (i = 0; elements[i]; ++i)
      {
        पूर्णाङ्क t = elements[i]->type;
        जइ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            जइ (!is_hfa1 (elements[i], candidate))
              प्रत्यागम 0;
          }
        अण्णहा जइ (t != candidate)
          प्रत्यागम 0;
      }

  प्रत्यागम 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

स्थिर पूर्णाङ्क
is_vfp_type (ध्रुव ffi_type *ty)
{
  ffi_type **elements;
  पूर्णाङ्क candidate, i;
  size_t size, ele_गणनम्, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  विकल्प (candidate)
    {
    स्वभाव:
      प्रत्यागम 0;
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_LONGDOUBLE:
      ele_गणनम् = 1;
      गच्छ done;
    पक्ष FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      विकल्प (candidate)
	{
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  ele_गणनम् = 2;
	  गच्छ done;
	}
      प्रत्यागम 0;
    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_VECTOR:
      भङ्ग;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  जइ (size < 4 || size > 64)
    प्रत्यागम 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  जइ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      प्रति (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          जइ (candidate >= 0)
            भङ्ग;
        }
    }

  जइ (simd_size)
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
      size_t reg_size = simd_size;
      पूर्णाङ्क अङ्कः_registers;
      पूर्णाङ्क first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      जइ (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	प्रत्यागम 0;
      अङ्कः_registers = (पूर्णाङ्क) (size / reg_size);
      जइ (अङ्कः_registers > 4)
	प्रत्यागम 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      जइ (ty->type != FFI_TYPE_VECTOR)
	प्रति (i = 0; elements[i]; ++i)
	  जइ (is_simd (elements[i]) != reg_size)
	    प्रत्यागम 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      प्रति (i = 0; elements[i]; ++i)
	{
	  पूर्णाङ्क t = elements[i]->type;
	  जइ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      जइ (!is_hfa1 (elements[i], candidate))
		प्रत्यागम 0;
	    }
	  अण्णहा जइ (t != candidate)
	    प्रत्यागम 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((पूर्णाङ्क) reg_size) - 2;
      प्रत्यागम first_level_element_type * 4 + (4 - अङ्कः_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  विकल्प (candidate)
    {
    पक्ष FFI_TYPE_FLOAT:
      ele_गणनम् = size / परिमाण(भिन्न);
      जइ (size != ele_गणनम् * परिमाण(भिन्न))
        प्रत्यागम 0;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      ele_गणनम् = size / परिमाण(द्विगुण);
      जइ (size != ele_गणनम् * परिमाण(द्विगुण))
        प्रत्यागम 0;
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      ele_गणनम् = size / परिमाण(long द्विगुण);
      जइ (size != ele_गणनम् * परिमाण(long द्विगुण))
        प्रत्यागम 0;
      भङ्ग;
    स्वभाव:
      प्रत्यागम 0;
    }
  जइ (ele_गणनम् > 4)
    प्रत्यागम 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  प्रति (i = 0; elements[i]; ++i)
    {
      पूर्णाङ्क t = elements[i]->type;
      जइ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          जइ (!is_hfa1 (elements[i], candidate))
            प्रत्यागम 0;
        }
      अण्णहा जइ (t != candidate)
        प्रत्यागम 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  प्रत्यागम candidate * 4 + (4 - (पूर्णाङ्क)ele_गणनम्);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

संरचना arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#जइ defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
स्थिर शून्य
arg_init (संरचना arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#जइ defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
स्थिर शून्य *
allocate_to_राशिः (संरचना arg_state *state, शून्य *राशिः,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#जइ defined (__APPLE__)
  जइ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#अण्णहा
  जइ (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  प्रत्यागम (वर्ण *)राशिः + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
स्थिर शून्य *
allocate_and_copy_struct_to_राशिः (संरचना arg_state *state, शून्य *राशिः,
				   size_t alignment, size_t size, शून्य *मानम्)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  प्रत्यागम memcpy ((वर्ण *) राशिः + dest, मानम्, size);
}

स्थिर ffi_arg
extend_integer_type (शून्य *source, पूर्णाङ्क type)
{
  विकल्प (type)
    {
    पक्ष FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, परिमाण (u8));
        प्रत्यागम u8;
      }
    पक्ष FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, परिमाण (s8));
        प्रत्यागम s8;
      }
    पक्ष FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, परिमाण (u16));
        प्रत्यागम u16;
      }
    पक्ष FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, परिमाण (s16));
        प्रत्यागम s16;
      }
    पक्ष FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, परिमाण (u32));
        प्रत्यागम u32;
      }
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, परिमाण (s32));
        प्रत्यागम s32;
      }
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, परिमाण (u64));
        प्रत्यागम u64;
      }
    पक्ष FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, परिमाण (uptr));
        प्रत्यागम uptr;
      }
    स्वभाव:
      abort();
    }
}

#जइ defined(_MSC_VER) && !defined(__clang__)
शून्य extend_hfa_type (शून्य *dest, शून्य *src, पूर्णाङ्क h);
#अण्णहा
स्थिर शून्य
extend_hfa_type (शून्य *dest, शून्य *src, पूर्णाङ्क h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  शून्य *x0;

#define BTI_J "hint #36"
  __asm__ चञ्चल (
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
#endif

#जइ defined(_MSC_VER) && !defined(__clang__)
शून्य* compress_hfa_type (शून्य *dest, शून्य *src, पूर्णाङ्क h);
#अण्णहा
स्थिर शून्य *
compress_hfa_type (शून्य *dest, शून्य *reg, पूर्णाङ्क h)
{
  विकल्प (h)
    {
    पक्ष AARCH64_RET_S1:
      जइ (dest == reg)
	प्रत्यागम (वर्ण *)dest + BE (12);
      अण्णहा
	*(भिन्न *)dest = *(भिन्न *)((वर्ण *)reg + BE (12));
      भङ्ग;
    पक्ष AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      भङ्ग;
    पक्ष AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      भङ्ग;
    पक्ष AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      भङ्ग;

    पक्ष AARCH64_RET_D1:
      जइ (dest == reg)
	प्रत्यागम (वर्ण *)dest + BE (8);
      अण्णहा
	*(द्विगुण *)dest = *(द्विगुण *)((वर्ण *)reg + BE (8));
      भङ्ग;
    पक्ष AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      भङ्ग;
    पक्ष AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      भङ्ग;
    पक्ष AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      भङ्ग;

    स्वभाव:
      जइ (dest != reg)
	प्रत्यागम memcpy (dest, reg, 16 * (4 - (h & 3)));
      भङ्ग;
    }
  प्रत्यागम dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

स्थिर शून्य *
allocate_int_to_reg_or_राशिः (संरचना call_context *context,
			      संरचना arg_state *state,
			      शून्य *राशिः, size_t size)
{
  जइ (state->ngrn < N_X_ARG_REG)
    प्रत्यागम (वर्ण *)&context->x[state->ngrn++] + BE (परिमाण (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  प्रत्यागम (वर्ण *)allocate_to_राशिः (state, राशिः, size, size)
	 + BE (परिमाण (UINT64) - size);
}

स्थिर शून्य *
allocate_int128_to_reg_or_राशिः (संरचना call_context *context,
			         संरचना arg_state *state, शून्य *राशिः)
{
  unsigned ngrn = state->ngrn;
  शून्य *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  जइ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  अण्णहा
    {
      ret = allocate_to_राशिः (state, राशिः, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  प्रत्यागम ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  पूर्णाङ्क flags, i, n;

  विकल्प (rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      भङ्ग;
    पक्ष FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      भङ्ग;
    पक्ष FFI_TYPE_POINTER:
      flags = (परिमाण(शून्य *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      भङ्ग;

    पक्ष FFI_TYPE_UINT128:
    पक्ष FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_LONGDOUBLE:
    पक्ष FFI_TYPE_STRUCT:
    पक्ष FFI_TYPE_VECTOR:
    पक्ष FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      जइ (flags == 0)
	{
	  size_t s = rtype->size;
	  जइ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  अण्णहा जइ (s == 16)
	    flags = AARCH64_RET_INT128;
	  अण्णहा जइ (s == 8)
	    flags = AARCH64_RET_INT64;
	  अण्णहा
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      भङ्ग;

    स्वभाव:
      abort();
    }

  प्रति (i = 0, n = cif->nargs; i < n; i++)
    जइ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	भङ्ग;
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
  प्रति (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      जइ (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#जइ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  प्रत्यागम FFI_OK;
}

#जइ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned पूर्णाङ्क nfixedargs,
			 unsigned पूर्णाङ्क ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  प्रत्यागम status;
}
#अण्णहा
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned पूर्णाङ्क nfixedargs, unsigned पूर्णाङ्क ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  प्रत्यागम status;
}
#endif /* __APPLE__ */

बाह्य शून्य ffi_call_SYSV (संरचना call_context *context, शून्य *frame,
			   शून्य (*fn)(शून्य), शून्य *rvalue, पूर्णाङ्क flags,
			   शून्य *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *orig_rvalue,
	      शून्य **avalue, शून्य *closure)
{
  संरचना call_context *context;
  शून्य *राशिः, *frame, *rvalue;
  संरचना arg_state state;
  size_t राशिः_bytes, rtype_size, rsize;
  पूर्णाङ्क i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  राशिः_bytes = cif->bytes;

  जइ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  जइ (flags & AARCH64_RET_IN_MEM)
    {
      जइ (orig_rvalue == रिक्त)
	rsize = rtype_size;
    }
  अण्णहा जइ (orig_rvalue == रिक्त)
    flags &= AARCH64_FLAG_ARG_V;
  अण्णहा जइ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (परिमाण(संरचना call_context) + राशिः_bytes + 40 + rsize);
  राशिः = context + 1;
  frame = (शून्य*)((uintptr_t)राशिः + (uintptr_t)राशिः_bytes);
  rvalue = (rsize ? (शून्य*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, राशिः_bytes);
  प्रति (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      शून्य *a = avalue[i];
      पूर्णाङ्क h, t;
      शून्य *dest;

      t = ty->type;
      विकल्प (t)
	{
	पक्ष FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  भङ्ग;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    जइ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    अण्णहा
	      {
		शून्य *d = allocate_to_राशिः (&state, राशिः, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#अण्णहा
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  भङ्ग;

	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_राशिः (context, &state, राशिः);
	  memcpy (dest, a, 16);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_VECTOR:
	पक्ष FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    जइ (h)
	      {
              पूर्णाङ्क elems = 4 - (h & 3);
              जइ (cif->abi == FFI_WIN64 && isvariadic)
              {
                जइ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  भङ्ग;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_राशिः(&state, राशिः, ty->alignment, s);
              }
              अण्णहा
              {
                जइ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  भङ्ग;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (वर्ण *)allocate_to_राशिः (&state, राशिः, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (परिमाण (UINT64) - s) : 0);
              }
	      }
	    अण्णहा जइ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_राशिः (&state, राशिः,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = परिमाण (शून्य *);
		गच्छ do_pointer;
	      }
	    अण्णहा
	      {
		size_t n = (s + 7) / 8;
		जइ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned पूर्णाङ्क)n;
		  }
		अण्णहा
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_राशिः (&state, राशिः, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  भङ्ग;

	स्वभाव:
	  abort();
	}

#जइ defined (__APPLE__)
      जइ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  जइ (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#जइ FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

बाह्य शून्य ffi_closure_SYSV (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_SYSV_V (शून्य) FFI_HIDDEN;
#जइ defined(FFI_EXEC_STATIC_TRAMP)
बाह्य शून्य ffi_closure_SYSV_alt (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_SYSV_V_alt (शून्य) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  जइ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    प्रत्यागम FFI_BAD_ABI;

  शून्य (*start)(शून्य);

  जइ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  अण्णहा
    start = ffi_closure_SYSV;

#जइ FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_कुञ्चिका_function_pointer, 0);
#  endif
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#अण्णहा
  स्थिर ध्रुव unsigned वर्ण trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  वर्ण *tramp = closure->tramp;

# जइ defined(FFI_EXEC_STATIC_TRAMP)
  जइ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जइ (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      अण्णहा
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      गच्छ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, परिमाण(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned वर्ण *tramp_code = tramp;
# अण्णहा
  unsigned वर्ण *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# जइ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

#ifdef FFI_GO_CLOSURES
बाह्य शून्य ffi_go_closure_SYSV (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_SYSV_V (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य (*start)(शून्य);

  जइ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    प्रत्यागम FFI_BAD_ABI;

  जइ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  अण्णहा
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}
#endif /* FFI_GO_CLOSURES */

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

पूर्णाङ्क FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
			शून्य *user_data,
			संरचना call_context *context,
			शून्य *राशिः, शून्य *rvalue, शून्य *struct_rvalue)
{
  शून्य **avalue = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));
  पूर्णाङ्क i, h, nargs, flags, isvariadic = 0;
  संरचना arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  जइ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  प्रति (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      पूर्णाङ्क t = ty->type;
      size_t n, s = ty->size;

      विकल्प (t)
	{
	पक्ष FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  भङ्ग;

	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_राशिः (context, &state, राशिः, s);
	  भङ्ग;

	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_राशिः (context, &state, राशिः);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	पक्ष FFI_TYPE_STRUCT:
	पक्ष FFI_TYPE_VECTOR:
	पक्ष FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  जइ (h)
	    {
	      n = 4 - (h & 3);
              जइ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  जइ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      शून्य *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned पूर्णाङ्क)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अण्णहा
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (वर्ण *)allocate_to_राशिः (&state, राशिः,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (परिमाण (UINT64) - s) : 0);
                    }
                }
              अण्णहा
                {
                  जइ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      शून्य *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned पूर्णाङ्क)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अण्णहा
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (वर्ण *)allocate_to_राशिः (&state, राशिः,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (परिमाण (UINT64) - s) : 0);
                    }
                }
            }
          अण्णहा जइ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_राशिः (context, &state,
                                               राशिः, परिमाण (शून्य *)),
                  परिमाण (UINT64));
             avalue[i] = (शून्य *)(UINT32)avalue_tmp;
#अण्णहा
              avalue[i] = *(शून्य **)
              allocate_int_to_reg_or_राशिः (context, &state, राशिः,
                                         परिमाण (शून्य *));
#endif
            }
          अण्णहा
            {
              n = (s + 7) / 8;
              जइ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned पूर्णाङ्क)n;
                }
              अण्णहा
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_राशिः(&state, राशिः,
                                           ty->alignment, s);
                }
            }
          भङ्ग;

        स्वभाव:
          abort();
      }

#जइ defined (__APPLE__)
      जइ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  जइ (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  प्रत्यागम flags;
}

#जइ defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  प्रत्यागम &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

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

#எனில் defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <fficonfig.h>
#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#அந்தர்பாவ <windows.h> /* FlushInstructionCache */
#endif
#அந்தர்பாவ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#எனில் FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# எனில் FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#இல்லையெனில்
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

ஒன்றிப்பு _d
{
  UINT64 d;
  UINT32 s[2];
};

கட்டமைப்பு _v
{
  ஒன்றிப்பு _d d[2] __attribute__((aligned(16)));
};

கட்டமைப்பு call_context
{
  கட்டமைப்பு _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#எனில் FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#அந்தர்பாவ <ptrauth.h>
#endif
#அந்தர்பாவ <mach/vm_param.h>
#endif

#இல்லையெனில்

#எனில் defined (__clang__) && defined (__APPLE__)
வெளி வெறுமை sys_icache_invalidate (வெறுமை *start, size_t len);
#endif

நிலையான உள்வரி வெறுமை
ffi_clear_cache (வெறுமை *start, வெறுமை *end)
{
#எனில் defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (எழுத்து *)end - (எழுத்து *)start);
#அல்லதெனில் defined (__GNUC__)
  __builtin___clear_cache (start, end);
#அல்லதெனில் defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (எழுத்து*)end - (எழுத்து*)start);
#இல்லையெனில்
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

நிலையான முழு
intlog2 (முழு n)
{
  முழு level = 0;
  போது (n >>= 1)
    ++level;
  திரும்பு level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

நிலையான முழு
is_hfa0 (மாறிலி ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  முழு i, ret = -1;

  எனில் (elements != வெற்று)
    க்கு (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        எனில் (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            எனில் (ret < 0)
              தொடர்;
          }
        நிறுத்து;
      }

  திரும்பு ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

நிலையான size_t
is_simd (மாறிலி ffi_type *ty)
{
  ffi_type **elements;
  முழு i;

  எனில் (ty->type == FFI_TYPE_VECTOR)
    திரும்பு ty->size;

  elements = ty->elements;
  எனில் (elements != வெற்று)
    க்கு (i = 0; elements[i]; ++i)
      {
        முழு t = elements[i]->type;
        எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          திரும்பு is_simd (elements[i]);
      }

  திரும்பு 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

நிலையான முழு
is_hfa1 (மாறிலி ffi_type *ty, முழு candidate)
{
  ffi_type **elements = ty->elements;
  முழு i;

  எனில் (elements != வெற்று)
    க்கு (i = 0; elements[i]; ++i)
      {
        முழு t = elements[i]->type;
        எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            எனில் (!is_hfa1 (elements[i], candidate))
              திரும்பு 0;
          }
        இல்லையெனில் எனில் (t != candidate)
          திரும்பு 0;
      }

  திரும்பு 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

நிலையான முழு
is_vfp_type (மாறிலி ffi_type *ty)
{
  ffi_type **elements;
  முழு candidate, i;
  size_t size, ele_மொத்தம், simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  தேர்வு (candidate)
    {
    இயல்பு:
      திரும்பு 0;
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_LONGDOUBLE:
      ele_மொத்தம் = 1;
      செல் done;
    நிலை FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      தேர்வு (candidate)
	{
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	  ele_மொத்தம் = 2;
	  செல் done;
	}
      திரும்பு 0;
    நிலை FFI_TYPE_STRUCT:
    நிலை FFI_TYPE_VECTOR:
      நிறுத்து;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  எனில் (size < 4 || size > 64)
    திரும்பு 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  எனில் (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      க்கு (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          எனில் (candidate >= 0)
            நிறுத்து;
        }
    }

  எனில் (simd_size)
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
      முழு இலக்கம்_registers;
      முழு first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      எனில் (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	திரும்பு 0;
      இலக்கம்_registers = (முழு) (size / reg_size);
      எனில் (இலக்கம்_registers > 4)
	திரும்பு 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      எனில் (ty->type != FFI_TYPE_VECTOR)
	க்கு (i = 0; elements[i]; ++i)
	  எனில் (is_simd (elements[i]) != reg_size)
	    திரும்பு 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      க்கு (i = 0; elements[i]; ++i)
	{
	  முழு t = elements[i]->type;
	  எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      எனில் (!is_hfa1 (elements[i], candidate))
		திரும்பு 0;
	    }
	  இல்லையெனில் எனில் (t != candidate)
	    திரும்பு 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((முழு) reg_size) - 2;
      திரும்பு first_level_element_type * 4 + (4 - இலக்கம்_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  தேர்வு (candidate)
    {
    நிலை FFI_TYPE_FLOAT:
      ele_மொத்தம் = size / அளவு(தசம);
      எனில் (size != ele_மொத்தம் * அளவு(தசம))
        திரும்பு 0;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      ele_மொத்தம் = size / அளவு(இரட்டை);
      எனில் (size != ele_மொத்தம் * அளவு(இரட்டை))
        திரும்பு 0;
      நிறுத்து;
    நிலை FFI_TYPE_LONGDOUBLE:
      ele_மொத்தம் = size / அளவு(long இரட்டை);
      எனில் (size != ele_மொத்தம் * அளவு(long இரட்டை))
        திரும்பு 0;
      நிறுத்து;
    இயல்பு:
      திரும்பு 0;
    }
  எனில் (ele_மொத்தம் > 4)
    திரும்பு 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  க்கு (i = 0; elements[i]; ++i)
    {
      முழு t = elements[i]->type;
      எனில் (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          எனில் (!is_hfa1 (elements[i], candidate))
            திரும்பு 0;
        }
      இல்லையெனில் எனில் (t != candidate)
        திரும்பு 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  திரும்பு candidate * 4 + (4 - (முழு)ele_மொத்தம்);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

கட்டமைப்பு arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#எனில் defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
நிலையான வெறுமை
arg_init (கட்டமைப்பு arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#எனில் defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
நிலையான வெறுமை *
allocate_to_அடுக்கு (கட்டமைப்பு arg_state *state, வெறுமை *அடுக்கு,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#எனில் defined (__APPLE__)
  எனில் (state->allocating_variadic && alignment < 8)
    alignment = 8;
#இல்லையெனில்
  எனில் (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  திரும்பு (எழுத்து *)அடுக்கு + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
நிலையான வெறுமை *
allocate_and_copy_struct_to_அடுக்கு (கட்டமைப்பு arg_state *state, வெறுமை *அடுக்கு,
				   size_t alignment, size_t size, வெறுமை *மதிப்பு)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  திரும்பு memcpy ((எழுத்து *) அடுக்கு + dest, மதிப்பு, size);
}

நிலையான ffi_arg
extend_integer_type (வெறுமை *source, முழு type)
{
  தேர்வு (type)
    {
    நிலை FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, அளவு (u8));
        திரும்பு u8;
      }
    நிலை FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, அளவு (s8));
        திரும்பு s8;
      }
    நிலை FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, அளவு (u16));
        திரும்பு u16;
      }
    நிலை FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, அளவு (s16));
        திரும்பு s16;
      }
    நிலை FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, அளவு (u32));
        திரும்பு u32;
      }
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, அளவு (s32));
        திரும்பு s32;
      }
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, அளவு (u64));
        திரும்பு u64;
      }
    நிலை FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, அளவு (uptr));
        திரும்பு uptr;
      }
    இயல்பு:
      abort();
    }
}

#எனில் defined(_MSC_VER) && !defined(__clang__)
வெறுமை extend_hfa_type (வெறுமை *dest, வெறுமை *src, முழு h);
#இல்லையெனில்
நிலையான வெறுமை
extend_hfa_type (வெறுமை *dest, வெறுமை *src, முழு h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  வெறுமை *x0;

#define BTI_J "hint #36"
  __asm__ நிலையற்ற (
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

#எனில் defined(_MSC_VER) && !defined(__clang__)
வெறுமை* compress_hfa_type (வெறுமை *dest, வெறுமை *src, முழு h);
#இல்லையெனில்
நிலையான வெறுமை *
compress_hfa_type (வெறுமை *dest, வெறுமை *reg, முழு h)
{
  தேர்வு (h)
    {
    நிலை AARCH64_RET_S1:
      எனில் (dest == reg)
	திரும்பு (எழுத்து *)dest + BE (12);
      இல்லையெனில்
	*(தசம *)dest = *(தசம *)((எழுத்து *)reg + BE (12));
      நிறுத்து;
    நிலை AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      நிறுத்து;
    நிலை AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      நிறுத்து;
    நிலை AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      நிறுத்து;

    நிலை AARCH64_RET_D1:
      எனில் (dest == reg)
	திரும்பு (எழுத்து *)dest + BE (8);
      இல்லையெனில்
	*(இரட்டை *)dest = *(இரட்டை *)((எழுத்து *)reg + BE (8));
      நிறுத்து;
    நிலை AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      நிறுத்து;
    நிலை AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      நிறுத்து;
    நிலை AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      நிறுத்து;

    இயல்பு:
      எனில் (dest != reg)
	திரும்பு memcpy (dest, reg, 16 * (4 - (h & 3)));
      நிறுத்து;
    }
  திரும்பு dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

நிலையான வெறுமை *
allocate_int_to_reg_or_அடுக்கு (கட்டமைப்பு call_context *context,
			      கட்டமைப்பு arg_state *state,
			      வெறுமை *அடுக்கு, size_t size)
{
  எனில் (state->ngrn < N_X_ARG_REG)
    திரும்பு (எழுத்து *)&context->x[state->ngrn++] + BE (அளவு (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  திரும்பு (எழுத்து *)allocate_to_அடுக்கு (state, அடுக்கு, size, size)
	 + BE (அளவு (UINT64) - size);
}

நிலையான வெறுமை *
allocate_int128_to_reg_or_அடுக்கு (கட்டமைப்பு call_context *context,
			         கட்டமைப்பு arg_state *state, வெறுமை *அடுக்கு)
{
  unsigned ngrn = state->ngrn;
  வெறுமை *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  எனில் (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  இல்லையெனில்
    {
      ret = allocate_to_அடுக்கு (state, அடுக்கு, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  திரும்பு ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  முழு flags, i, n;

  தேர்வு (rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      நிறுத்து;
    நிலை FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      நிறுத்து;
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      நிறுத்து;
    நிலை FFI_TYPE_POINTER:
      flags = (அளவு(வெறுமை *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      நிறுத்து;

    நிலை FFI_TYPE_UINT128:
    நிலை FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      நிறுத்து;

    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_LONGDOUBLE:
    நிலை FFI_TYPE_STRUCT:
    நிலை FFI_TYPE_VECTOR:
    நிலை FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      எனில் (flags == 0)
	{
	  size_t s = rtype->size;
	  எனில் (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  இல்லையெனில் எனில் (s == 16)
	    flags = AARCH64_RET_INT128;
	  இல்லையெனில் எனில் (s == 8)
	    flags = AARCH64_RET_INT64;
	  இல்லையெனில்
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      நிறுத்து;

    இயல்பு:
      abort();
    }

  க்கு (i = 0, n = cif->nargs; i < n; i++)
    எனில் (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	நிறுத்து;
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
  க்கு (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      எனில் (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#எனில் defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  திரும்பு FFI_OK;
}

#எனில் defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned முழு nfixedargs,
			 unsigned முழு ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  திரும்பு status;
}
#இல்லையெனில்
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned முழு nfixedargs, unsigned முழு ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  திரும்பு status;
}
#endif /* __APPLE__ */

வெளி வெறுமை ffi_call_SYSV (கட்டமைப்பு call_context *context, வெறுமை *frame,
			   வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, முழு flags,
			   வெறுமை *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *orig_rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  கட்டமைப்பு call_context *context;
  வெறுமை *அடுக்கு, *frame, *rvalue;
  கட்டமைப்பு arg_state state;
  size_t அடுக்கு_bytes, rtype_size, rsize;
  முழு i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  அடுக்கு_bytes = cif->bytes;

  எனில் (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  எனில் (flags & AARCH64_RET_IN_MEM)
    {
      எனில் (orig_rvalue == வெற்று)
	rsize = rtype_size;
    }
  இல்லையெனில் எனில் (orig_rvalue == வெற்று)
    flags &= AARCH64_FLAG_ARG_V;
  இல்லையெனில் எனில் (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (அளவு(கட்டமைப்பு call_context) + அடுக்கு_bytes + 40 + rsize);
  அடுக்கு = context + 1;
  frame = (வெறுமை*)((uintptr_t)அடுக்கு + (uintptr_t)அடுக்கு_bytes);
  rvalue = (rsize ? (வெறுமை*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, அடுக்கு_bytes);
  க்கு (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      வெறுமை *a = avalue[i];
      முழு h, t;
      வெறுமை *dest;

      t = ty->type;
      தேர்வு (t)
	{
	நிலை FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  நிறுத்து;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    எனில் (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    இல்லையெனில்
	      {
		வெறுமை *d = allocate_to_அடுக்கு (&state, அடுக்கு, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#இல்லையெனில்
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  நிறுத்து;

	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_அடுக்கு (context, &state, அடுக்கு);
	  memcpy (dest, a, 16);
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	நிலை FFI_TYPE_STRUCT:
	நிலை FFI_TYPE_VECTOR:
	நிலை FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    எனில் (h)
	      {
              முழு elems = 4 - (h & 3);
              எனில் (cif->abi == FFI_WIN64 && isvariadic)
              {
                எனில் (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  நிறுத்து;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_அடுக்கு(&state, அடுக்கு, ty->alignment, s);
              }
              இல்லையெனில்
              {
                எனில் (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  நிறுத்து;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (எழுத்து *)allocate_to_அடுக்கு (&state, அடுக்கு, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (அளவு (UINT64) - s) : 0);
              }
	      }
	    இல்லையெனில் எனில் (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_அடுக்கு (&state, அடுக்கு,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = அளவு (வெறுமை *);
		செல் do_pointer;
	      }
	    இல்லையெனில்
	      {
		size_t n = (s + 7) / 8;
		எனில் (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned முழு)n;
		  }
		இல்லையெனில்
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_அடுக்கு (&state, அடுக்கு, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  நிறுத்து;

	இயல்பு:
	  abort();
	}

#எனில் defined (__APPLE__)
      எனில் (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  எனில் (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

#எனில் FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

வெளி வெறுமை ffi_closure_SYSV (வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_closure_SYSV_V (வெறுமை) FFI_HIDDEN;
#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெளி வெறுமை ffi_closure_SYSV_alt (வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_closure_SYSV_V_alt (வெறுமை) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
  எனில் (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    திரும்பு FFI_BAD_ABI;

  வெறுமை (*start)(வெறுமை);

  எனில் (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  இல்லையெனில்
    start = ffi_closure_SYSV;

#எனில் FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_சாவி_function_pointer, 0);
#  endif
  வெறுமை **config = (வெறுமை **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#இல்லையெனில்
  நிலையான மாறிலி unsigned எழுத்து trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  எழுத்து *tramp = closure->tramp;

# எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      எனில் (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      இல்லையெனில்
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      செல் out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, அளவு(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned எழுத்து *tramp_code = tramp;
# இல்லையெனில்
  unsigned எழுத்து *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

#ifdef FFI_GO_CLOSURES
வெளி வெறுமை ffi_go_closure_SYSV (வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_go_closure_SYSV_V (வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*))
{
  வெறுமை (*start)(வெறுமை);

  எனில் (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    திரும்பு FFI_BAD_ABI;

  எனில் (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  இல்லையெனில்
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
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

முழு FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
			வெறுமை *user_data,
			கட்டமைப்பு call_context *context,
			வெறுமை *அடுக்கு, வெறுமை *rvalue, வெறுமை *struct_rvalue)
{
  வெறுமை **avalue = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை*));
  முழு i, h, nargs, flags, isvariadic = 0;
  கட்டமைப்பு arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  எனில் (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  க்கு (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      முழு t = ty->type;
      size_t n, s = ty->size;

      தேர்வு (t)
	{
	நிலை FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  நிறுத்து;

	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_அடுக்கு (context, &state, அடுக்கு, s);
	  நிறுத்து;

	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_அடுக்கு (context, &state, அடுக்கு);
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	நிலை FFI_TYPE_STRUCT:
	நிலை FFI_TYPE_VECTOR:
	நிலை FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  எனில் (h)
	    {
	      n = 4 - (h & 3);
              எனில் (cif->abi == FFI_WIN64 && isvariadic)
                {
                  எனில் (state.ngrn + n <= N_X_ARG_REG)
                    {
                      வெறுமை *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned முழு)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  இல்லையெனில்
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (எழுத்து *)allocate_to_அடுக்கு (&state, அடுக்கு,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (அளவு (UINT64) - s) : 0);
                    }
                }
              இல்லையெனில்
                {
                  எனில் (state.nsrn + n <= N_V_ARG_REG)
                    {
                      வெறுமை *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned முழு)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  இல்லையெனில்
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (எழுத்து *)allocate_to_அடுக்கு (&state, அடுக்கு,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (அளவு (UINT64) - s) : 0);
                    }
                }
            }
          இல்லையெனில் எனில் (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_அடுக்கு (context, &state,
                                               அடுக்கு, அளவு (வெறுமை *)),
                  அளவு (UINT64));
             avalue[i] = (வெறுமை *)(UINT32)avalue_tmp;
#இல்லையெனில்
              avalue[i] = *(வெறுமை **)
              allocate_int_to_reg_or_அடுக்கு (context, &state, அடுக்கு,
                                         அளவு (வெறுமை *));
#endif
            }
          இல்லையெனில்
            {
              n = (s + 7) / 8;
              எனில் (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned முழு)n;
                }
              இல்லையெனில்
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_அடுக்கு(&state, அடுக்கு,
                                           ty->alignment, s);
                }
            }
          நிறுத்து;

        இயல்பு:
          abort();
      }

#எனில் defined (__APPLE__)
      எனில் (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  எனில் (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  திரும்பு flags;
}

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

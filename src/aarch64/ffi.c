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

#যদি defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <fficonfig.h>
#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>
#অন্তর্ভাৱ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#অন্তর্ভাৱ <windows.h> /* FlushInstructionCache */
#endif
#অন্তর্ভাৱ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#যদি FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# যদি FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#অন্যথা
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

সঙ্ঘ _d
{
  UINT64 d;
  UINT32 s[2];
};

সংরচনা _v
{
  সঙ্ঘ _d d[2] __attribute__((aligned(16)));
};

সংরচনা call_context
{
  সংরচনা _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#যদি FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#অন্তর্ভাৱ <ptrauth.h>
#endif
#অন্তর্ভাৱ <mach/vm_param.h>
#endif

#অন্যথা

#যদি defined (__clang__) && defined (__APPLE__)
বাহ্য শূন্য sys_icache_invalidate (শূন্য *start, size_t len);
#endif

স্থির অন্তঃস্থ শূন্য
ffi_clear_cache (শূন্য *start, শূন্য *end)
{
#যদি defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (ৱর্ণ *)end - (ৱর্ণ *)start);
#নোচেদ্যদি defined (__GNUC__)
  __builtin___clear_cache (start, end);
#নোচেদ্যদি defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (ৱর্ণ*)end - (ৱর্ণ*)start);
#অন্যথা
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

স্থির পূর্ণাঙ্ক
intlog2 (পূর্ণাঙ্ক n)
{
  পূর্ণাঙ্ক level = 0;
  যেতিয়ালৈ (n >>= 1)
    ++level;
  ঘূৰাই level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

স্থির পূর্ণাঙ্ক
is_hfa0 (ধ্রুৱ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  পূর্ণাঙ্ক i, ret = -1;

  যদি (elements != রিক্ত)
    বাবে (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        যদি (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            যদি (ret < 0)
              অনুৱর্ত;
          }
        ভঙ্গ;
      }

  ঘূৰাই ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

স্থির size_t
is_simd (ধ্রুৱ ffi_type *ty)
{
  ffi_type **elements;
  পূর্ণাঙ্ক i;

  যদি (ty->type == FFI_TYPE_VECTOR)
    ঘূৰাই ty->size;

  elements = ty->elements;
  যদি (elements != রিক্ত)
    বাবে (i = 0; elements[i]; ++i)
      {
        পূর্ণাঙ্ক t = elements[i]->type;
        যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          ঘূৰাই is_simd (elements[i]);
      }

  ঘূৰাই 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

স্থির পূর্ণাঙ্ক
is_hfa1 (ধ্রুৱ ffi_type *ty, পূর্ণাঙ্ক candidate)
{
  ffi_type **elements = ty->elements;
  পূর্ণাঙ্ক i;

  যদি (elements != রিক্ত)
    বাবে (i = 0; elements[i]; ++i)
      {
        পূর্ণাঙ্ক t = elements[i]->type;
        যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            যদি (!is_hfa1 (elements[i], candidate))
              ঘূৰাই 0;
          }
        অন্যথা যদি (t != candidate)
          ঘূৰাই 0;
      }

  ঘূৰাই 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

স্থির পূর্ণাঙ্ক
is_vfp_type (ধ্রুৱ ffi_type *ty)
{
  ffi_type **elements;
  পূর্ণাঙ্ক candidate, i;
  size_t size, ele_গুনতি, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  ৱিকল্প (candidate)
    {
    স্ৱভাৱ:
      ঘূৰাই 0;
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_LONGDOUBLE:
      ele_গুনতি = 1;
      গচ্ছ done;
    পক্ষ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ৱিকল্প (candidate)
	{
	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  ele_গুনতি = 2;
	  গচ্ছ done;
	}
      ঘূৰাই 0;
    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_VECTOR:
      ভঙ্গ;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  যদি (size < 4 || size > 64)
    ঘূৰাই 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  যদি (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      বাবে (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          যদি (candidate >= 0)
            ভঙ্গ;
        }
    }

  যদি (simd_size)
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
      পূর্ণাঙ্ক অঙ্ক_registers;
      পূর্ণাঙ্ক first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      যদি (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	ঘূৰাই 0;
      অঙ্ক_registers = (পূর্ণাঙ্ক) (size / reg_size);
      যদি (অঙ্ক_registers > 4)
	ঘূৰাই 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      যদি (ty->type != FFI_TYPE_VECTOR)
	বাবে (i = 0; elements[i]; ++i)
	  যদি (is_simd (elements[i]) != reg_size)
	    ঘূৰাই 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      বাবে (i = 0; elements[i]; ++i)
	{
	  পূর্ণাঙ্ক t = elements[i]->type;
	  যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      যদি (!is_hfa1 (elements[i], candidate))
		ঘূৰাই 0;
	    }
	  অন্যথা যদি (t != candidate)
	    ঘূৰাই 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((পূর্ণাঙ্ক) reg_size) - 2;
      ঘূৰাই first_level_element_type * 4 + (4 - অঙ্ক_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ৱিকল্প (candidate)
    {
    পক্ষ FFI_TYPE_FLOAT:
      ele_গুনতি = size / পরিমাণ(ভিন্ন);
      যদি (size != ele_গুনতি * পরিমাণ(ভিন্ন))
        ঘূৰাই 0;
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      ele_গুনতি = size / পরিমাণ(দ্ৱিগুণ);
      যদি (size != ele_গুনতি * পরিমাণ(দ্ৱিগুণ))
        ঘূৰাই 0;
      ভঙ্গ;
    পক্ষ FFI_TYPE_LONGDOUBLE:
      ele_গুনতি = size / পরিমাণ(long দ্ৱিগুণ);
      যদি (size != ele_গুনতি * পরিমাণ(long দ্ৱিগুণ))
        ঘূৰাই 0;
      ভঙ্গ;
    স্ৱভাৱ:
      ঘূৰাই 0;
    }
  যদি (ele_গুনতি > 4)
    ঘূৰাই 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  বাবে (i = 0; elements[i]; ++i)
    {
      পূর্ণাঙ্ক t = elements[i]->type;
      যদি (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          যদি (!is_hfa1 (elements[i], candidate))
            ঘূৰাই 0;
        }
      অন্যথা যদি (t != candidate)
        ঘূৰাই 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ঘূৰাই candidate * 4 + (4 - (পূর্ণাঙ্ক)ele_গুনতি);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

সংরচনা arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#যদি defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
স্থির শূন্য
arg_init (সংরচনা arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#যদি defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
স্থির শূন্য *
allocate_to_স্তূপ (সংরচনা arg_state *state, শূন্য *স্তূপ,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#যদি defined (__APPLE__)
  যদি (state->allocating_variadic && alignment < 8)
    alignment = 8;
#অন্যথা
  যদি (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  ঘূৰাই (ৱর্ণ *)স্তূপ + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
স্থির শূন্য *
allocate_and_copy_struct_to_স্তূপ (সংরচনা arg_state *state, শূন্য *স্তূপ,
				   size_t alignment, size_t size, শূন্য *মান)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  ঘূৰাই memcpy ((ৱর্ণ *) স্তূপ + dest, মান, size);
}

স্থির ffi_arg
extend_integer_type (শূন্য *source, পূর্ণাঙ্ক type)
{
  ৱিকল্প (type)
    {
    পক্ষ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, পরিমাণ (u8));
        ঘূৰাই u8;
      }
    পক্ষ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, পরিমাণ (s8));
        ঘূৰাই s8;
      }
    পক্ষ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, পরিমাণ (u16));
        ঘূৰাই u16;
      }
    পক্ষ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, পরিমাণ (s16));
        ঘূৰাই s16;
      }
    পক্ষ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, পরিমাণ (u32));
        ঘূৰাই u32;
      }
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, পরিমাণ (s32));
        ঘূৰাই s32;
      }
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, পরিমাণ (u64));
        ঘূৰাই u64;
      }
    পক্ষ FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, পরিমাণ (uptr));
        ঘূৰাই uptr;
      }
    স্ৱভাৱ:
      abort();
    }
}

#যদি defined(_MSC_VER) && !defined(__clang__)
শূন্য extend_hfa_type (শূন্য *dest, শূন্য *src, পূর্ণাঙ্ক h);
#অন্যথা
স্থির শূন্য
extend_hfa_type (শূন্য *dest, শূন্য *src, পূর্ণাঙ্ক h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  শূন্য *x0;

#define BTI_J "hint #36"
  __asm__ চঞ্চল (
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

#যদি defined(_MSC_VER) && !defined(__clang__)
শূন্য* compress_hfa_type (শূন্য *dest, শূন্য *src, পূর্ণাঙ্ক h);
#অন্যথা
স্থির শূন্য *
compress_hfa_type (শূন্য *dest, শূন্য *reg, পূর্ণাঙ্ক h)
{
  ৱিকল্প (h)
    {
    পক্ষ AARCH64_RET_S1:
      যদি (dest == reg)
	ঘূৰাই (ৱর্ণ *)dest + BE (12);
      অন্যথা
	*(ভিন্ন *)dest = *(ভিন্ন *)((ৱর্ণ *)reg + BE (12));
      ভঙ্গ;
    পক্ষ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ভঙ্গ;
    পক্ষ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ভঙ্গ;
    পক্ষ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ভঙ্গ;

    পক্ষ AARCH64_RET_D1:
      যদি (dest == reg)
	ঘূৰাই (ৱর্ণ *)dest + BE (8);
      অন্যথা
	*(দ্ৱিগুণ *)dest = *(দ্ৱিগুণ *)((ৱর্ণ *)reg + BE (8));
      ভঙ্গ;
    পক্ষ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ভঙ্গ;
    পক্ষ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ভঙ্গ;
    পক্ষ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ভঙ্গ;

    স্ৱভাৱ:
      যদি (dest != reg)
	ঘূৰাই memcpy (dest, reg, 16 * (4 - (h & 3)));
      ভঙ্গ;
    }
  ঘূৰাই dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

স্থির শূন্য *
allocate_int_to_reg_or_স্তূপ (সংরচনা call_context *context,
			      সংরচনা arg_state *state,
			      শূন্য *স্তূপ, size_t size)
{
  যদি (state->ngrn < N_X_ARG_REG)
    ঘূৰাই (ৱর্ণ *)&context->x[state->ngrn++] + BE (পরিমাণ (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  ঘূৰাই (ৱর্ণ *)allocate_to_স্তূপ (state, স্তূপ, size, size)
	 + BE (পরিমাণ (UINT64) - size);
}

স্থির শূন্য *
allocate_int128_to_reg_or_স্তূপ (সংরচনা call_context *context,
			         সংরচনা arg_state *state, শূন্য *স্তূপ)
{
  unsigned ngrn = state->ngrn;
  শূন্য *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  যদি (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  অন্যথা
    {
      ret = allocate_to_স্তূপ (state, স্তূপ, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ঘূৰাই ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  পূর্ণাঙ্ক flags, i, n;

  ৱিকল্প (rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ভঙ্গ;
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ভঙ্গ;
    পক্ষ FFI_TYPE_POINTER:
      flags = (পরিমাণ(শূন্য *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ভঙ্গ;

    পক্ষ FFI_TYPE_UINT128:
    পক্ষ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ভঙ্গ;

    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_LONGDOUBLE:
    পক্ষ FFI_TYPE_STRUCT:
    পক্ষ FFI_TYPE_VECTOR:
    পক্ষ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      যদি (flags == 0)
	{
	  size_t s = rtype->size;
	  যদি (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  অন্যথা যদি (s == 16)
	    flags = AARCH64_RET_INT128;
	  অন্যথা যদি (s == 8)
	    flags = AARCH64_RET_INT64;
	  অন্যথা
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ভঙ্গ;

    স্ৱভাৱ:
      abort();
    }

  বাবে (i = 0, n = cif->nargs; i < n; i++)
    যদি (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ভঙ্গ;
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
  বাবে (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      যদি (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#যদি defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  ঘূৰাই FFI_OK;
}

#যদি defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণাঙ্ক nfixedargs,
			 unsigned পূর্ণাঙ্ক ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ঘূৰাই status;
}
#অন্যথা
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned পূর্ণাঙ্ক nfixedargs, unsigned পূর্ণাঙ্ক ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ঘূৰাই status;
}
#endif /* __APPLE__ */

বাহ্য শূন্য ffi_call_SYSV (সংরচনা call_context *context, শূন্য *frame,
			   শূন্য (*fn)(শূন্য), শূন্য *rvalue, পূর্ণাঙ্ক flags,
			   শূন্য *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *orig_rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  সংরচনা call_context *context;
  শূন্য *স্তূপ, *frame, *rvalue;
  সংরচনা arg_state state;
  size_t স্তূপ_bytes, rtype_size, rsize;
  পূর্ণাঙ্ক i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  স্তূপ_bytes = cif->bytes;

  যদি (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  যদি (flags & AARCH64_RET_IN_MEM)
    {
      যদি (orig_rvalue == রিক্ত)
	rsize = rtype_size;
    }
  অন্যথা যদি (orig_rvalue == রিক্ত)
    flags &= AARCH64_FLAG_ARG_V;
  অন্যথা যদি (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (পরিমাণ(সংরচনা call_context) + স্তূপ_bytes + 40 + rsize);
  স্তূপ = context + 1;
  frame = (শূন্য*)((uintptr_t)স্তূপ + (uintptr_t)স্তূপ_bytes);
  rvalue = (rsize ? (শূন্য*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, স্তূপ_bytes);
  বাবে (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      শূন্য *a = avalue[i];
      পূর্ণাঙ্ক h, t;
      শূন্য *dest;

      t = ty->type;
      ৱিকল্প (t)
	{
	পক্ষ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ভঙ্গ;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    যদি (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    অন্যথা
	      {
		শূন্য *d = allocate_to_স্তূপ (&state, স্তূপ, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#অন্যথা
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT128:
	পক্ষ FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_স্তূপ (context, &state, স্তূপ);
	  memcpy (dest, a, 16);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_LONGDOUBLE:
	পক্ষ FFI_TYPE_STRUCT:
	পক্ষ FFI_TYPE_VECTOR:
	পক্ষ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    যদি (h)
	      {
              পূর্ণাঙ্ক elems = 4 - (h & 3);
              যদি (cif->abi == FFI_WIN64 && isvariadic)
              {
                যদি (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ভঙ্গ;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_স্তূপ(&state, স্তূপ, ty->alignment, s);
              }
              অন্যথা
              {
                যদি (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ভঙ্গ;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (ৱর্ণ *)allocate_to_স্তূপ (&state, স্তূপ, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (পরিমাণ (UINT64) - s) : 0);
              }
	      }
	    অন্যথা যদি (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_স্তূপ (&state, স্তূপ,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = পরিমাণ (শূন্য *);
		গচ্ছ do_pointer;
	      }
	    অন্যথা
	      {
		size_t n = (s + 7) / 8;
		যদি (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned পূর্ণাঙ্ক)n;
		  }
		অন্যথা
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_স্তূপ (&state, স্তূপ, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  ভঙ্গ;

	স্ৱভাৱ:
	  abort();
	}

#যদি defined (__APPLE__)
      যদি (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  যদি (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, রিক্ত);
}

#যদি FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

বাহ্য শূন্য ffi_closure_SYSV (শূন্য) FFI_HIDDEN;
বাহ্য শূন্য ffi_closure_SYSV_V (শূন্য) FFI_HIDDEN;
#যদি defined(FFI_EXEC_STATIC_TRAMP)
বাহ্য শূন্য ffi_closure_SYSV_alt (শূন্য) FFI_HIDDEN;
বাহ্য শূন্য ffi_closure_SYSV_V_alt (শূন্য) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
                      শূন্য *user_data,
                      শূন্য *codeloc)
{
  যদি (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ঘূৰাই FFI_BAD_ABI;

  শূন্য (*start)(শূন্য);

  যদি (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  অন্যথা
    start = ffi_closure_SYSV;

#যদি FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_চাবি_function_pointer, 0);
#  endif
  শূন্য **config = (শূন্য **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#অন্যথা
  স্থির ধ্রুৱ unsigned ৱর্ণ trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  ৱর্ণ *tramp = closure->tramp;

# যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      যদি (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      অন্যথা
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      গচ্ছ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, পরিমাণ(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned ৱর্ণ *tramp_code = tramp;
# অন্যথা
  unsigned ৱর্ণ *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ঘূৰাই FFI_OK;
}

#ifdef FFI_GO_CLOSURES
বাহ্য শূন্য ffi_go_closure_SYSV (শূন্য) FFI_HIDDEN;
বাহ্য শূন্য ffi_go_closure_SYSV_V (শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*))
{
  শূন্য (*start)(শূন্য);

  যদি (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ঘূৰাই FFI_BAD_ABI;

  যদি (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  অন্যথা
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  ঘূৰাই FFI_OK;
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

পূর্ণাঙ্ক FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
			শূন্য *user_data,
			সংরচনা call_context *context,
			শূন্য *স্তূপ, শূন্য *rvalue, শূন্য *struct_rvalue)
{
  শূন্য **avalue = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য*));
  পূর্ণাঙ্ক i, h, nargs, flags, isvariadic = 0;
  সংরচনা arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  যদি (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  বাবে (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      পূর্ণাঙ্ক t = ty->type;
      size_t n, s = ty->size;

      ৱিকল্প (t)
	{
	পক্ষ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_স্তূপ (context, &state, স্তূপ, s);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT128:
	পক্ষ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_স্তূপ (context, &state, স্তূপ);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_LONGDOUBLE:
	পক্ষ FFI_TYPE_STRUCT:
	পক্ষ FFI_TYPE_VECTOR:
	পক্ষ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  যদি (h)
	    {
	      n = 4 - (h & 3);
              যদি (cif->abi == FFI_WIN64 && isvariadic)
                {
                  যদি (state.ngrn + n <= N_X_ARG_REG)
                    {
                      শূন্য *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned পূর্ণাঙ্ক)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  অন্যথা
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ৱর্ণ *)allocate_to_স্তূপ (&state, স্তূপ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (পরিমাণ (UINT64) - s) : 0);
                    }
                }
              অন্যথা
                {
                  যদি (state.nsrn + n <= N_V_ARG_REG)
                    {
                      শূন্য *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned পূর্ণাঙ্ক)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  অন্যথা
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ৱর্ণ *)allocate_to_স্তূপ (&state, স্তূপ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (পরিমাণ (UINT64) - s) : 0);
                    }
                }
            }
          অন্যথা যদি (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_স্তূপ (context, &state,
                                               স্তূপ, পরিমাণ (শূন্য *)),
                  পরিমাণ (UINT64));
             avalue[i] = (শূন্য *)(UINT32)avalue_tmp;
#অন্যথা
              avalue[i] = *(শূন্য **)
              allocate_int_to_reg_or_স্তূপ (context, &state, স্তূপ,
                                         পরিমাণ (শূন্য *));
#endif
            }
          অন্যথা
            {
              n = (s + 7) / 8;
              যদি (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned পূর্ণাঙ্ক)n;
                }
              অন্যথা
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_স্তূপ(&state, স্তূপ,
                                           ty->alignment, s);
                }
            }
          ভঙ্গ;

        স্ৱভাৱ:
          abort();
      }

#যদি defined (__APPLE__)
      যদি (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  যদি (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  ঘূৰাই flags;
}

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  বাহ্য শূন্য *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  ঘূৰাই &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

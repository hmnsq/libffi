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

#ଯଦି defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <fficonfig.h>
#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ଅନ୍ତର୍ଭାଵ <windows.h> /* FlushInstructionCache */
#endif
#ଅନ୍ତର୍ଭାଵ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ଯଦି FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ଯଦି FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ଅନ୍ୟଥା
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

ସଂଘ _d
{
  UINT64 d;
  UINT32 s[2];
};

ସଂରଚନା _v
{
  ସଂଘ _d d[2] __attribute__((aligned(16)));
};

ସଂରଚନା call_context
{
  ସଂରଚନା _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ଯଦି FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#ଅନ୍ତର୍ଭାଵ <ptrauth.h>
#endif
#ଅନ୍ତର୍ଭାଵ <mach/vm_param.h>
#endif

#ଅନ୍ୟଥା

#ଯଦି defined (__clang__) && defined (__APPLE__)
ବାହ୍ୟ ଶୂନ୍ୟ sys_icache_invalidate (ଶୂନ୍ୟ *start, size_t len);
#endif

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
ffi_clear_cache (ଶୂନ୍ୟ *start, ଶୂନ୍ୟ *end)
{
#ଯଦି defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (ଅକ୍ଷର *)end - (ଅକ୍ଷର *)start);
#ନଚେତଯଦି defined (__GNUC__)
  __builtin___clear_cache (start, end);
#ନଚେତଯଦି defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (ଅକ୍ଷର*)end - (ଅକ୍ଷର*)start);
#ଅନ୍ୟଥା
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
intlog2 (ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା level = 0;
  ଯେତେବେଳେ (n >>= 1)
    ++level;
  ଫେରନ୍ତୁ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_hfa0 (ଅଚଳ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ret = -1;

  ଯଦି (elements != ଖାଲି)
    ପାଇଁ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ଯଦି (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ଯଦି (ret < 0)
              ଜାରି;
          }
        ଭାଙ୍ଗ;
      }

  ଫେରନ୍ତୁ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ସ୍ଥିର size_t
is_simd (ଅଚଳ ffi_type *ty)
{
  ffi_type **elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (ty->type == FFI_TYPE_VECTOR)
    ଫେରନ୍ତୁ ty->size;

  elements = ty->elements;
  ଯଦି (elements != ଖାଲି)
    ପାଇଁ (i = 0; elements[i]; ++i)
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
        ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          ଫେରନ୍ତୁ is_simd (elements[i]);
      }

  ଫେରନ୍ତୁ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_hfa1 (ଅଚଳ ffi_type *ty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା candidate)
{
  ffi_type **elements = ty->elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (elements != ଖାଲି)
    ପାଇଁ (i = 0; elements[i]; ++i)
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
        ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            ଯଦି (!is_hfa1 (elements[i], candidate))
              ଫେରନ୍ତୁ 0;
          }
        ଅନ୍ୟଥା ଯଦି (t != candidate)
          ଫେରନ୍ତୁ 0;
      }

  ଫେରନ୍ତୁ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
is_vfp_type (ଅଚଳ ffi_type *ty)
{
  ffi_type **elements;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା candidate, i;
  size_t size, ele_ଗଣନା, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  ବାଛ (candidate)
    {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      ele_ଗଣନା = 1;
      ଯାଅ done;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ବାଛ (candidate)
	{
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  ele_ଗଣନା = 2;
	  ଯାଅ done;
	}
      ଫେରନ୍ତୁ 0;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
    ସ୍ଥିତି FFI_TYPE_VECTOR:
      ଭାଙ୍ଗ;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  ଯଦି (size < 4 || size > 64)
    ଫେରନ୍ତୁ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ଯଦି (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      ପାଇଁ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ଯଦି (candidate >= 0)
            ଭାଙ୍ଗ;
        }
    }

  ଯଦି (simd_size)
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
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_registers;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ଯଦି (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	ଫେରନ୍ତୁ 0;
      ଅଙ୍କ_registers = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (size / reg_size);
      ଯଦି (ଅଙ୍କ_registers > 4)
	ଫେରନ୍ତୁ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ଯଦି (ty->type != FFI_TYPE_VECTOR)
	ପାଇଁ (i = 0; elements[i]; ++i)
	  ଯଦି (is_simd (elements[i]) != reg_size)
	    ଫେରନ୍ତୁ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      ପାଇଁ (i = 0; elements[i]; ++i)
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
	  ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ଯଦି (!is_hfa1 (elements[i], candidate))
		ଫେରନ୍ତୁ 0;
	    }
	  ଅନ୍ୟଥା ଯଦି (t != candidate)
	    ଫେରନ୍ତୁ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) reg_size) - 2;
      ଫେରନ୍ତୁ first_level_element_type * 4 + (4 - ଅଙ୍କ_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ବାଛ (candidate)
    {
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ele_ଗଣନା = size / ଆକାର(ଦଶମିକ);
      ଯଦି (size != ele_ଗଣନା * ଆକାର(ଦଶମିକ))
        ଫେରନ୍ତୁ 0;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ele_ଗଣନା = size / ଆକାର(ଦ୍ୱିଗୁଣ);
      ଯଦି (size != ele_ଗଣନା * ଆକାର(ଦ୍ୱିଗୁଣ))
        ଫେରନ୍ତୁ 0;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      ele_ଗଣନା = size / ଆକାର(long ଦ୍ୱିଗୁଣ);
      ଯଦି (size != ele_ଗଣନା * ଆକାର(long ଦ୍ୱିଗୁଣ))
        ଫେରନ୍ତୁ 0;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
    }
  ଯଦି (ele_ଗଣନା > 4)
    ଫେରନ୍ତୁ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ପାଇଁ (i = 0; elements[i]; ++i)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = elements[i]->type;
      ଯଦି (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ଯଦି (!is_hfa1 (elements[i], candidate))
            ଫେରନ୍ତୁ 0;
        }
      ଅନ୍ୟଥା ଯଦି (t != candidate)
        ଫେରନ୍ତୁ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ଫେରନ୍ତୁ candidate * 4 + (4 - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ele_ଗଣନା);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ସଂରଚନା arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#ଯଦି defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
ସ୍ଥିର ଶୂନ୍ୟ
arg_init (ସଂରଚନା arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#ଯଦି defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ସ୍ଥିର ଶୂନ୍ୟ *
allocate_to_ଗଦା (ସଂରଚନା arg_state *state, ଶୂନ୍ୟ *ଗଦା,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ଯଦି defined (__APPLE__)
  ଯଦି (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ଅନ୍ୟଥା
  ଯଦି (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  ଫେରନ୍ତୁ (ଅକ୍ଷର *)ଗଦା + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ସ୍ଥିର ଶୂନ୍ୟ *
allocate_and_copy_struct_to_ଗଦା (ସଂରଚନା arg_state *state, ଶୂନ୍ୟ *ଗଦା,
				   size_t alignment, size_t size, ଶୂନ୍ୟ *ମୂଲ୍ୟ)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  ଫେରନ୍ତୁ memcpy ((ଅକ୍ଷର *) ଗଦା + dest, ମୂଲ୍ୟ, size);
}

ସ୍ଥିର ffi_arg
extend_integer_type (ଶୂନ୍ୟ *source, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, ଆକାର (u8));
        ଫେରନ୍ତୁ u8;
      }
    ସ୍ଥିତି FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, ଆକାର (s8));
        ଫେରନ୍ତୁ s8;
      }
    ସ୍ଥିତି FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, ଆକାର (u16));
        ଫେରନ୍ତୁ u16;
      }
    ସ୍ଥିତି FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, ଆକାର (s16));
        ଫେରନ୍ତୁ s16;
      }
    ସ୍ଥିତି FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, ଆକାର (u32));
        ଫେରନ୍ତୁ u32;
      }
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, ଆକାର (s32));
        ଫେରନ୍ତୁ s32;
      }
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, ଆକାର (u64));
        ଫେରନ୍ତୁ u64;
      }
    ସ୍ଥିତି FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, ଆକାର (uptr));
        ଫେରନ୍ତୁ uptr;
      }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }
}

#ଯଦି defined(_MSC_VER) && !defined(__clang__)
ଶୂନ୍ୟ extend_hfa_type (ଶୂନ୍ୟ *dest, ଶୂନ୍ୟ *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା h);
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ
extend_hfa_type (ଶୂନ୍ୟ *dest, ଶୂନ୍ୟ *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  ଶୂନ୍ୟ *x0;

#define BTI_J "hint #36"
  __asm__ ଅସ୍ଥିର (
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

#ଯଦି defined(_MSC_VER) && !defined(__clang__)
ଶୂନ୍ୟ* compress_hfa_type (ଶୂନ୍ୟ *dest, ଶୂନ୍ୟ *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା h);
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ *
compress_hfa_type (ଶୂନ୍ୟ *dest, ଶୂନ୍ୟ *reg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା h)
{
  ବାଛ (h)
    {
    ସ୍ଥିତି AARCH64_RET_S1:
      ଯଦି (dest == reg)
	ଫେରନ୍ତୁ (ଅକ୍ଷର *)dest + BE (12);
      ଅନ୍ୟଥା
	*(ଦଶମିକ *)dest = *(ଦଶମିକ *)((ଅକ୍ଷର *)reg + BE (12));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ଭାଙ୍ଗ;

    ସ୍ଥିତି AARCH64_RET_D1:
      ଯଦି (dest == reg)
	ଫେରନ୍ତୁ (ଅକ୍ଷର *)dest + BE (8);
      ଅନ୍ୟଥା
	*(ଦ୍ୱିଗୁଣ *)dest = *(ଦ୍ୱିଗୁଣ *)((ଅକ୍ଷର *)reg + BE (8));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଯଦି (dest != reg)
	ଫେରନ୍ତୁ memcpy (dest, reg, 16 * (4 - (h & 3)));
      ଭାଙ୍ଗ;
    }
  ଫେରନ୍ତୁ dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ସ୍ଥିର ଶୂନ୍ୟ *
allocate_int_to_reg_or_ଗଦା (ସଂରଚନା call_context *context,
			      ସଂରଚନା arg_state *state,
			      ଶୂନ୍ୟ *ଗଦା, size_t size)
{
  ଯଦି (state->ngrn < N_X_ARG_REG)
    ଫେରନ୍ତୁ (ଅକ୍ଷର *)&context->x[state->ngrn++] + BE (ଆକାର (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  ଫେରନ୍ତୁ (ଅକ୍ଷର *)allocate_to_ଗଦା (state, ଗଦା, size, size)
	 + BE (ଆକାର (UINT64) - size);
}

ସ୍ଥିର ଶୂନ୍ୟ *
allocate_int128_to_reg_or_ଗଦା (ସଂରଚନା call_context *context,
			         ସଂରଚନା arg_state *state, ଶୂନ୍ୟ *ଗଦା)
{
  unsigned ngrn = state->ngrn;
  ଶୂନ୍ୟ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ଯଦି (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ଅନ୍ୟଥା
    {
      ret = allocate_to_ଗଦା (state, ଗଦା, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ଫେରନ୍ତୁ ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, i, n;

  ବାଛ (rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = (ଆକାର(ଶୂନ୍ୟ *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_UINT128:
    ସ୍ଥିତି FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
    ସ୍ଥିତି FFI_TYPE_STRUCT:
    ସ୍ଥିତି FFI_TYPE_VECTOR:
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ଯଦି (flags == 0)
	{
	  size_t s = rtype->size;
	  ଯଦି (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  ଅନ୍ୟଥା ଯଦି (s == 16)
	    flags = AARCH64_RET_INT128;
	  ଅନ୍ୟଥା ଯଦି (s == 8)
	    flags = AARCH64_RET_INT64;
	  ଅନ୍ୟଥା
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }

  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    ଯଦି (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ଭାଙ୍ଗ;
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
  ପାଇଁ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ଯଦି (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#ଯଦି defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  ଫେରନ୍ତୁ FFI_OK;
}

#ଯଦି defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
			 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ଫେରନ୍ତୁ status;
}
#ଅନ୍ୟଥା
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ଫେରନ୍ତୁ status;
}
#endif /* __APPLE__ */

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV (ସଂରଚନା call_context *context, ଶୂନ୍ୟ *frame,
			   ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
			   ଶୂନ୍ୟ *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *orig_rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ସଂରଚନା call_context *context;
  ଶୂନ୍ୟ *ଗଦା, *frame, *rvalue;
  ସଂରଚନା arg_state state;
  size_t ଗଦା_bytes, rtype_size, rsize;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  ଗଦା_bytes = cif->bytes;

  ଯଦି (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ଯଦି (flags & AARCH64_RET_IN_MEM)
    {
      ଯଦି (orig_rvalue == ଖାଲି)
	rsize = rtype_size;
    }
  ଅନ୍ୟଥା ଯଦି (orig_rvalue == ଖାଲି)
    flags &= AARCH64_FLAG_ARG_V;
  ଅନ୍ୟଥା ଯଦି (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (ଆକାର(ସଂରଚନା call_context) + ଗଦା_bytes + 40 + rsize);
  ଗଦା = context + 1;
  frame = (ଶୂନ୍ୟ*)((uintptr_t)ଗଦା + (uintptr_t)ଗଦା_bytes);
  rvalue = (rsize ? (ଶୂନ୍ୟ*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, ଗଦା_bytes);
  ପାଇଁ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      ଶୂନ୍ୟ *a = avalue[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା h, t;
      ଶୂନ୍ୟ *dest;

      t = ty->type;
      ବାଛ (t)
	{
	ସ୍ଥିତି FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ଭାଙ୍ଗ;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ଯଦି (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ଅନ୍ୟଥା
	      {
		ଶୂନ୍ୟ *d = allocate_to_ଗଦା (&state, ଗଦା, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#ଅନ୍ୟଥା
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT128:
	ସ୍ଥିତି FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_ଗଦା (context, &state, ଗଦା);
	  memcpy (dest, a, 16);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_VECTOR:
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ଯଦି (h)
	      {
              ପୂର୍ଣ୍ଣସଂଖ୍ୟା elems = 4 - (h & 3);
              ଯଦି (cif->abi == FFI_WIN64 && isvariadic)
              {
                ଯଦି (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ଭାଙ୍ଗ;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_ଗଦା(&state, ଗଦା, ty->alignment, s);
              }
              ଅନ୍ୟଥା
              {
                ଯଦି (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ଭାଙ୍ଗ;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (ଅକ୍ଷର *)allocate_to_ଗଦା (&state, ଗଦା, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (ଆକାର (UINT64) - s) : 0);
              }
	      }
	    ଅନ୍ୟଥା ଯଦି (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_ଗଦା (&state, ଗଦା,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = ଆକାର (ଶୂନ୍ୟ *);
		ଯାଅ do_pointer;
	      }
	    ଅନ୍ୟଥା
	      {
		size_t n = (s + 7) / 8;
		ଯଦି (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
		  }
		ଅନ୍ୟଥା
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_ଗଦା (&state, ଗଦା, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}

#ଯଦି defined (__APPLE__)
      ଯଦି (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  ଯଦି (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

#ଯଦି FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV (ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV_V (ଶୂନ୍ୟ) FFI_HIDDEN;
#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV_alt (ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_SYSV_V_alt (ଶୂନ୍ୟ) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
                      ଶୂନ୍ୟ *user_data,
                      ଶୂନ୍ୟ *codeloc)
{
  ଯଦି (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  ଶୂନ୍ୟ (*start)(ଶୂନ୍ୟ);

  ଯଦି (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  ଅନ୍ୟଥା
    start = ffi_closure_SYSV;

#ଯଦି FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_ଚାବି_function_pointer, 0);
#  endif
  ଶୂନ୍ୟ **config = (ଶୂନ୍ୟ **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#ଅନ୍ୟଥା
  ସ୍ଥିର ଅଚଳ unsigned ଅକ୍ଷର trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  ଅକ୍ଷର *tramp = closure->tramp;

# ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଯଦି (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      ଅନ୍ୟଥା
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      ଯାଅ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, ଆକାର(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned ଅକ୍ଷର *tramp_code = tramp;
# ଅନ୍ୟଥା
  unsigned ଅକ୍ଷର *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_SYSV (ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_SYSV_V (ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*))
{
  ଶୂନ୍ୟ (*start)(ଶୂନ୍ୟ);

  ଯଦି (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  ଯଦି (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  ଅନ୍ୟଥା
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
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

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
			ଶୂନ୍ୟ *user_data,
			ସଂରଚନା call_context *context,
			ଶୂନ୍ୟ *ଗଦା, ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ *struct_rvalue)
{
  ଶୂନ୍ୟ **avalue = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ*));
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, h, nargs, flags, isvariadic = 0;
  ସଂରଚନା arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  ଯଦି (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  ପାଇଁ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା t = ty->type;
      size_t n, s = ty->size;

      ବାଛ (t)
	{
	ସ୍ଥିତି FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_ଗଦା (context, &state, ଗଦା, s);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT128:
	ସ୍ଥିତି FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_ଗଦା (context, &state, ଗଦା);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_VECTOR:
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ଯଦି (h)
	    {
	      n = 4 - (h & 3);
              ଯଦି (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ଯଦି (state.ngrn + n <= N_X_ARG_REG)
                    {
                      ଶୂନ୍ୟ *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ଅନ୍ୟଥା
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ଅକ୍ଷର *)allocate_to_ଗଦା (&state, ଗଦା,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ଆକାର (UINT64) - s) : 0);
                    }
                }
              ଅନ୍ୟଥା
                {
                  ଯଦି (state.nsrn + n <= N_V_ARG_REG)
                    {
                      ଶୂନ୍ୟ *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ଅନ୍ୟଥା
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ଅକ୍ଷର *)allocate_to_ଗଦା (&state, ଗଦା,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ଆକାର (UINT64) - s) : 0);
                    }
                }
            }
          ଅନ୍ୟଥା ଯଦି (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_ଗଦା (context, &state,
                                               ଗଦା, ଆକାର (ଶୂନ୍ୟ *)),
                  ଆକାର (UINT64));
             avalue[i] = (ଶୂନ୍ୟ *)(UINT32)avalue_tmp;
#ଅନ୍ୟଥା
              avalue[i] = *(ଶୂନ୍ୟ **)
              allocate_int_to_reg_or_ଗଦା (context, &state, ଗଦା,
                                         ଆକାର (ଶୂନ୍ୟ *));
#endif
            }
          ଅନ୍ୟଥା
            {
              n = (s + 7) / 8;
              ଯଦି (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n;
                }
              ଅନ୍ୟଥା
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_ଗଦା(&state, ଗଦା,
                                           ty->alignment, s);
                }
            }
          ଭାଙ୍ଗ;

        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          abort();
      }

#ଯଦି defined (__APPLE__)
      ଯଦି (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ଯଦି (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  ଫେରନ୍ତୁ flags;
}

#ଯଦି defined(FFI_EXEC_STATIC_TRAMP)
ଶୂନ୍ୟ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ବାହ୍ୟ ଶୂନ୍ୟ *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  ଫେରନ୍ତୁ &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

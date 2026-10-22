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

#ಯದಿ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ಅನ್ತರ್ಭಾವ <windows.h> /* FlushInstructionCache */
#endif
#ಅನ್ತರ್ಭಾವ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ಯದಿ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ಯದಿ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ಅನ್ಯಥಾ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

ಸಙ್ಘ _d
{
  UINT64 d;
  UINT32 s[2];
};

ಸಂರಚನಾ _v
{
  ಸಙ್ಘ _d d[2] __attribute__((aligned(16)));
};

ಸಂರಚನಾ call_context
{
  ಸಂರಚನಾ _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ಯದಿ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#ಅನ್ತರ್ಭಾವ <ptrauth.h>
#endif
#ಅನ್ತರ್ಭಾವ <mach/vm_param.h>
#endif

#ಅನ್ಯಥಾ

#ಯದಿ defined (__clang__) && defined (__APPLE__)
ಬಾಹ್ಯ ಶೂನ್ಯ sys_icache_invalidate (ಶೂನ್ಯ *start, size_t len);
#endif

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ
ffi_clear_cache (ಶೂನ್ಯ *start, ಶೂನ್ಯ *end)
{
#ಯದಿ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (ವರ್ಣ *)end - (ವರ್ಣ *)start);
#ನೋಚೇದ್ಯದಿ defined (__GNUC__)
  __builtin___clear_cache (start, end);
#ನೋಚೇದ್ಯದಿ defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (ವರ್ಣ*)end - (ವರ್ಣ*)start);
#ಅನ್ಯಥಾ
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
intlog2 (ಪೂರ್ಣಾಙ್ಕ n)
{
  ಪೂರ್ಣಾಙ್ಕ level = 0;
  ಯಾವತ್ (n >>= 1)
    ++level;
  ಪ್ರತ್ಯಾಗಮ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_hfa0 (ಧ್ರುವ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಙ್ಕ i, ret = -1;

  ಯದಿ (elements != ರಿಕ್ತ)
    ಪ್ರತಿ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ಯದಿ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ಯದಿ (ret < 0)
              ಅನುವರ್ತ;
          }
        ಭಙ್ಗ;
      }

  ಪ್ರತ್ಯಾಗಮ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ಸ್ಥಿರ size_t
is_simd (ಧ್ರುವ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (ty->type == FFI_TYPE_VECTOR)
    ಪ್ರತ್ಯಾಗಮ ty->size;

  elements = ty->elements;
  ಯದಿ (elements != ರಿಕ್ತ)
    ಪ್ರತಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
        ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          ಪ್ರತ್ಯಾಗಮ is_simd (elements[i]);
      }

  ಪ್ರತ್ಯಾಗಮ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_hfa1 (ಧ್ರುವ ffi_type *ty, ಪೂರ್ಣಾಙ್ಕ candidate)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (elements != ರಿಕ್ತ)
    ಪ್ರತಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
        ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            ಯದಿ (!is_hfa1 (elements[i], candidate))
              ಪ್ರತ್ಯಾಗಮ 0;
          }
        ಅನ್ಯಥಾ ಯದಿ (t != candidate)
          ಪ್ರತ್ಯಾಗಮ 0;
      }

  ಪ್ರತ್ಯಾಗಮ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
is_vfp_type (ಧ್ರುವ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಙ್ಕ candidate, i;
  size_t size, ele_ಎಣಿಕೆ, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  ವಿಕಲ್ಪ (candidate)
    {
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      ele_ಎಣಿಕೆ = 1;
      ಗಚ್ಛ done;
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ವಿಕಲ್ಪ (candidate)
	{
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  ele_ಎಣಿಕೆ = 2;
	  ಗಚ್ಛ done;
	}
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
    ಪಕ್ಷ FFI_TYPE_VECTOR:
      ಭಙ್ಗ;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  ಯದಿ (size < 4 || size > 64)
    ಪ್ರತ್ಯಾಗಮ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ಯದಿ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      ಪ್ರತಿ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ಯದಿ (candidate >= 0)
            ಭಙ್ಗ;
        }
    }

  ಯದಿ (simd_size)
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
      ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_registers;
      ಪೂರ್ಣಾಙ್ಕ first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ಯದಿ (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	ಪ್ರತ್ಯಾಗಮ 0;
      ಅಂಕೆ_registers = (ಪೂರ್ಣಾಙ್ಕ) (size / reg_size);
      ಯದಿ (ಅಂಕೆ_registers > 4)
	ಪ್ರತ್ಯಾಗಮ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ಯದಿ (ty->type != FFI_TYPE_VECTOR)
	ಪ್ರತಿ (i = 0; elements[i]; ++i)
	  ಯದಿ (is_simd (elements[i]) != reg_size)
	    ಪ್ರತ್ಯಾಗಮ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      ಪ್ರತಿ (i = 0; elements[i]; ++i)
	{
	  ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
	  ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ಯದಿ (!is_hfa1 (elements[i], candidate))
		ಪ್ರತ್ಯಾಗಮ 0;
	    }
	  ಅನ್ಯಥಾ ಯದಿ (t != candidate)
	    ಪ್ರತ್ಯಾಗಮ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((ಪೂರ್ಣಾಙ್ಕ) reg_size) - 2;
      ಪ್ರತ್ಯಾಗಮ first_level_element_type * 4 + (4 - ಅಂಕೆ_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ವಿಕಲ್ಪ (candidate)
    {
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ele_ಎಣಿಕೆ = size / ಪರಿಮಾಣ(ಭಿನ್ನ);
      ಯದಿ (size != ele_ಎಣಿಕೆ * ಪರಿಮಾಣ(ಭಿನ್ನ))
        ಪ್ರತ್ಯಾಗಮ 0;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = size / ಪರಿಮಾಣ(ದ್ವಿಗುಣ);
      ಯದಿ (size != ele_ಎಣಿಕೆ * ಪರಿಮಾಣ(ದ್ವಿಗುಣ))
        ಪ್ರತ್ಯಾಗಮ 0;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      ele_ಎಣಿಕೆ = size / ಪರಿಮಾಣ(long ದ್ವಿಗುಣ);
      ಯದಿ (size != ele_ಎಣಿಕೆ * ಪರಿಮಾಣ(long ದ್ವಿಗುಣ))
        ಪ್ರತ್ಯಾಗಮ 0;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 0;
    }
  ಯದಿ (ele_ಎಣಿಕೆ > 4)
    ಪ್ರತ್ಯಾಗಮ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ಪ್ರತಿ (i = 0; elements[i]; ++i)
    {
      ಪೂರ್ಣಾಙ್ಕ t = elements[i]->type;
      ಯದಿ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ಯದಿ (!is_hfa1 (elements[i], candidate))
            ಪ್ರತ್ಯಾಗಮ 0;
        }
      ಅನ್ಯಥಾ ಯದಿ (t != candidate)
        ಪ್ರತ್ಯಾಗಮ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ಪ್ರತ್ಯಾಗಮ candidate * 4 + (4 - (ಪೂರ್ಣಾಙ್ಕ)ele_ಎಣಿಕೆ);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ಸಂರಚನಾ arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#ಯದಿ defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
ಸ್ಥಿರ ಶೂನ್ಯ
arg_init (ಸಂರಚನಾ arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#ಯದಿ defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_to_ರಾಶಿ (ಸಂರಚನಾ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ಯದಿ defined (__APPLE__)
  ಯದಿ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ಅನ್ಯಥಾ
  ಯದಿ (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)ರಾಶಿ + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_and_copy_struct_to_ರಾಶಿ (ಸಂರಚನಾ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ,
				   size_t alignment, size_t size, ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  ಪ್ರತ್ಯಾಗಮ memcpy ((ವರ್ಣ *) ರಾಶಿ + dest, ಮೌಲ್ಯ, size);
}

ಸ್ಥಿರ ffi_arg
extend_integer_type (ಶೂನ್ಯ *source, ಪೂರ್ಣಾಙ್ಕ type)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, ಪರಿಮಾಣ (u8));
        ಪ್ರತ್ಯಾಗಮ u8;
      }
    ಪಕ್ಷ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, ಪರಿಮಾಣ (s8));
        ಪ್ರತ್ಯಾಗಮ s8;
      }
    ಪಕ್ಷ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, ಪರಿಮಾಣ (u16));
        ಪ್ರತ್ಯಾಗಮ u16;
      }
    ಪಕ್ಷ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, ಪರಿಮಾಣ (s16));
        ಪ್ರತ್ಯಾಗಮ s16;
      }
    ಪಕ್ಷ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, ಪರಿಮಾಣ (u32));
        ಪ್ರತ್ಯಾಗಮ u32;
      }
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, ಪರಿಮಾಣ (s32));
        ಪ್ರತ್ಯಾಗಮ s32;
      }
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, ಪರಿಮಾಣ (u64));
        ಪ್ರತ್ಯಾಗಮ u64;
      }
    ಪಕ್ಷ FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, ಪರಿಮಾಣ (uptr));
        ಪ್ರತ್ಯಾಗಮ uptr;
      }
    ಸ್ವಭಾವ:
      abort();
    }
}

#ಯದಿ defined(_MSC_VER) && !defined(__clang__)
ಶೂನ್ಯ extend_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಙ್ಕ h);
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ
extend_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಙ್ಕ h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  ಶೂನ್ಯ *x0;

#define BTI_J "hint #36"
  __asm__ ಚಞ್ಚಲ (
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

#ಯದಿ defined(_MSC_VER) && !defined(__clang__)
ಶೂನ್ಯ* compress_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಙ್ಕ h);
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ *
compress_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *reg, ಪೂರ್ಣಾಙ್ಕ h)
{
  ವಿಕಲ್ಪ (h)
    {
    ಪಕ್ಷ AARCH64_RET_S1:
      ಯದಿ (dest == reg)
	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)dest + BE (12);
      ಅನ್ಯಥಾ
	*(ಭಿನ್ನ *)dest = *(ಭಿನ್ನ *)((ವರ್ಣ *)reg + BE (12));
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ಭಙ್ಗ;

    ಪಕ್ಷ AARCH64_RET_D1:
      ಯದಿ (dest == reg)
	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)dest + BE (8);
      ಅನ್ಯಥಾ
	*(ದ್ವಿಗುಣ *)dest = *(ದ್ವಿಗುಣ *)((ವರ್ಣ *)reg + BE (8));
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ಭಙ್ಗ;
    ಪಕ್ಷ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಯದಿ (dest != reg)
	ಪ್ರತ್ಯಾಗಮ memcpy (dest, reg, 16 * (4 - (h & 3)));
      ಭಙ್ಗ;
    }
  ಪ್ರತ್ಯಾಗಮ dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_int_to_reg_or_ರಾಶಿ (ಸಂರಚನಾ call_context *context,
			      ಸಂರಚನಾ arg_state *state,
			      ಶೂನ್ಯ *ರಾಶಿ, size_t size)
{
  ಯದಿ (state->ngrn < N_X_ARG_REG)
    ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)&context->x[state->ngrn++] + BE (ಪರಿಮಾಣ (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)allocate_to_ರಾಶಿ (state, ರಾಶಿ, size, size)
	 + BE (ಪರಿಮಾಣ (UINT64) - size);
}

ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_int128_to_reg_or_ರಾಶಿ (ಸಂರಚನಾ call_context *context,
			         ಸಂರಚನಾ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ)
{
  unsigned ngrn = state->ngrn;
  ಶೂನ್ಯ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ಯದಿ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ಅನ್ಯಥಾ
    {
      ret = allocate_to_ರಾಶಿ (state, ರಾಶಿ, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ಪ್ರತ್ಯಾಗಮ ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  ಪೂರ್ಣಾಙ್ಕ flags, i, n;

  ವಿಕಲ್ಪ (rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_POINTER:
      flags = (ಪರಿಮಾಣ(ಶೂನ್ಯ *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_UINT128:
    ಪಕ್ಷ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
    ಪಕ್ಷ FFI_TYPE_STRUCT:
    ಪಕ್ಷ FFI_TYPE_VECTOR:
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ಯದಿ (flags == 0)
	{
	  size_t s = rtype->size;
	  ಯದಿ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  ಅನ್ಯಥಾ ಯದಿ (s == 16)
	    flags = AARCH64_RET_INT128;
	  ಅನ್ಯಥಾ ಯದಿ (s == 8)
	    flags = AARCH64_RET_INT64;
	  ಅನ್ಯಥಾ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      abort();
    }

  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    ಯದಿ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ಭಙ್ಗ;
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
  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ಯದಿ (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#ಯದಿ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ಯದಿ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
			 unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ಪ್ರತ್ಯಾಗಮ status;
}
#ಅನ್ಯಥಾ
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs, unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ಪ್ರತ್ಯಾಗಮ status;
}
#endif /* __APPLE__ */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಸಂರಚನಾ call_context *context, ಶೂನ್ಯ *frame,
			   ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಪೂರ್ಣಾಙ್ಕ flags,
			   ಶೂನ್ಯ *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *orig_rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ಸಂರಚನಾ call_context *context;
  ಶೂನ್ಯ *ರಾಶಿ, *frame, *rvalue;
  ಸಂರಚನಾ arg_state state;
  size_t ರಾಶಿ_bytes, rtype_size, rsize;
  ಪೂರ್ಣಾಙ್ಕ i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  ರಾಶಿ_bytes = cif->bytes;

  ಯದಿ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ಯದಿ (flags & AARCH64_RET_IN_MEM)
    {
      ಯದಿ (orig_rvalue == ರಿಕ್ತ)
	rsize = rtype_size;
    }
  ಅನ್ಯಥಾ ಯದಿ (orig_rvalue == ರಿಕ್ತ)
    flags &= AARCH64_FLAG_ARG_V;
  ಅನ್ಯಥಾ ಯದಿ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (ಪರಿಮಾಣ(ಸಂರಚನಾ call_context) + ರಾಶಿ_bytes + 40 + rsize);
  ರಾಶಿ = context + 1;
  frame = (ಶೂನ್ಯ*)((uintptr_t)ರಾಶಿ + (uintptr_t)ರಾಶಿ_bytes);
  rvalue = (rsize ? (ಶೂನ್ಯ*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, ರಾಶಿ_bytes);
  ಪ್ರತಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      ಶೂನ್ಯ *a = avalue[i];
      ಪೂರ್ಣಾಙ್ಕ h, t;
      ಶೂನ್ಯ *dest;

      t = ty->type;
      ವಿಕಲ್ಪ (t)
	{
	ಪಕ್ಷ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ಭಙ್ಗ;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ಯದಿ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ಅನ್ಯಥಾ
	      {
		ಶೂನ್ಯ *d = allocate_to_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#ಅನ್ಯಥಾ
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ);
	  memcpy (dest, a, 16);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	ಪಕ್ಷ FFI_TYPE_VECTOR:
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ಯದಿ (h)
	      {
              ಪೂರ್ಣಾಙ್ಕ elems = 4 - (h & 3);
              ಯದಿ (cif->abi == FFI_WIN64 && isvariadic)
              {
                ಯದಿ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ಭಙ್ಗ;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_ರಾಶಿ(&state, ರಾಶಿ, ty->alignment, s);
              }
              ಅನ್ಯಥಾ
              {
                ಯದಿ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ಭಙ್ಗ;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (ವರ್ಣ *)allocate_to_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (ಪರಿಮಾಣ (UINT64) - s) : 0);
              }
	      }
	    ಅನ್ಯಥಾ ಯದಿ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_ರಾಶಿ (&state, ರಾಶಿ,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = ಪರಿಮಾಣ (ಶೂನ್ಯ *);
		ಗಚ್ಛ do_pointer;
	      }
	    ಅನ್ಯಥಾ
	      {
		size_t n = (s + 7) / 8;
		ಯದಿ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned ಪೂರ್ಣಾಙ್ಕ)n;
		  }
		ಅನ್ಯಥಾ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  abort();
	}

#ಯದಿ defined (__APPLE__)
      ಯದಿ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  ಯದಿ (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

#ಯದಿ FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV_V (ಶೂನ್ಯ) FFI_HIDDEN;
#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV_alt (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV_V_alt (ಶೂನ್ಯ) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_data,
                      ಶೂನ್ಯ *codeloc)
{
  ಯದಿ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  ಶೂನ್ಯ (*start)(ಶೂನ್ಯ);

  ಯದಿ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  ಅನ್ಯಥಾ
    start = ffi_closure_SYSV;

#ಯದಿ FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_ಕೀಲಿ_function_pointer, 0);
#  endif
  ಶೂನ್ಯ **config = (ಶೂನ್ಯ **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#ಅನ್ಯಥಾ
  ಸ್ಥಿರ ಧ್ರುವ unsigned ವರ್ಣ trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  ವರ್ಣ *tramp = closure->tramp;

# ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ಯದಿ (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      ಅನ್ಯಥಾ
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      ಗಚ್ಛ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, ಪರಿಮಾಣ(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned ವರ್ಣ *tramp_code = tramp;
# ಅನ್ಯಥಾ
  unsigned ವರ್ಣ *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_SYSV_V (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ (*start)(ಶೂನ್ಯ);

  ಯದಿ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  ಯದಿ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  ಅನ್ಯಥಾ
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
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

ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
			ಶೂನ್ಯ *user_data,
			ಸಂರಚನಾ call_context *context,
			ಶೂನ್ಯ *ರಾಶಿ, ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ *struct_rvalue)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಙ್ಕ i, h, nargs, flags, isvariadic = 0;
  ಸಂರಚನಾ arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  ಯದಿ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  ಪ್ರತಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ t = ty->type;
      size_t n, s = ty->size;

      ವಿಕಲ್ಪ (t)
	{
	ಪಕ್ಷ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ, s);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	ಪಕ್ಷ FFI_TYPE_VECTOR:
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ಯದಿ (h)
	    {
	      n = 4 - (h & 3);
              ಯದಿ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ಯದಿ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      ಶೂನ್ಯ *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned ಪೂರ್ಣಾಙ್ಕ)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ಅನ್ಯಥಾ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ವರ್ಣ *)allocate_to_ರಾಶಿ (&state, ರಾಶಿ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ಪರಿಮಾಣ (UINT64) - s) : 0);
                    }
                }
              ಅನ್ಯಥಾ
                {
                  ಯದಿ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      ಶೂನ್ಯ *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned ಪೂರ್ಣಾಙ್ಕ)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ಅನ್ಯಥಾ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ವರ್ಣ *)allocate_to_ರಾಶಿ (&state, ರಾಶಿ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ಪರಿಮಾಣ (UINT64) - s) : 0);
                    }
                }
            }
          ಅನ್ಯಥಾ ಯದಿ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_ರಾಶಿ (context, &state,
                                               ರಾಶಿ, ಪರಿಮಾಣ (ಶೂನ್ಯ *)),
                  ಪರಿಮಾಣ (UINT64));
             avalue[i] = (ಶೂನ್ಯ *)(UINT32)avalue_tmp;
#ಅನ್ಯಥಾ
              avalue[i] = *(ಶೂನ್ಯ **)
              allocate_int_to_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ,
                                         ಪರಿಮಾಣ (ಶೂನ್ಯ *));
#endif
            }
          ಅನ್ಯಥಾ
            {
              n = (s + 7) / 8;
              ಯದಿ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned ಪೂರ್ಣಾಙ್ಕ)n;
                }
              ಅನ್ಯಥಾ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_ರಾಶಿ(&state, ರಾಶಿ,
                                           ty->alignment, s);
                }
            }
          ಭಙ್ಗ;

        ಸ್ವಭಾವ:
          abort();
      }

#ಯದಿ defined (__APPLE__)
      ಯದಿ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ಯದಿ (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  ಪ್ರತ್ಯಾಗಮ flags;
}

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

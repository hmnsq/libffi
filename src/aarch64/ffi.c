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

#ಆದರೆ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <fficonfig.h>
#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ "internal.h"
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ವ್ಯಾಖ್ಯಾನಿಸು WIN32_LEAN_AND_ಸರಾಸರಿ
#ಅನ್ತರ್ಭಾವ <windows.h> /* FlushInstructionCache */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ಆದರೆ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ಆದರೆ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ಹೊರಹೋಗು ದ date
# endif
#ಇಲ್ಲದಿದ್ದರೆ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ಆದರೆ_ಅಂತ್ಯ

ಒಕ್ಕೂಟ _d
{
  UINT64 d;
  UINT32 s[2];
};

ರಚನೆ _v
{
  ಒಕ್ಕೂಟ _d d[2] __attribute__((aligned(16)));
};

ರಚನೆ call_context
{
  ರಚನೆ _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ಆದರೆ FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MACH__
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_ARM64E_PTRAUTH
#ಅನ್ತರ್ಭಾವ <ptrauth.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <mach/vm_param.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಇಲ್ಲದಿದ್ದರೆ

#ಆದರೆ defined (__clang__) && defined (__APPLE__)
ಬಾಹ್ಯ ಶೂನ್ಯ sys_icache_invalidate (ಶೂನ್ಯ *ಆರಂಭ, size_t len);
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ffi_clear_cache (ಶೂನ್ಯ *ಆರಂಭ, ಶೂನ್ಯ *ಕೊನೆ)
{
#ಆದರೆ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (ಆರಂಭ, (ಅಕ್ಷರ *)ಕೊನೆ - (ಅಕ್ಷರ *)ಆರಂಭ);
#ಅಥವಾಆದರೆ defined (__GNUC__)
  __builtin___clear_cache (ಆರಂಭ, ಕೊನೆ);
#ಅಥವಾಆದರೆ defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), ಆರಂಭ, (ಅಕ್ಷರ*)ಕೊನೆ - (ಅಕ್ಷರ*)ಆರಂಭ);
#ಇಲ್ಲದಿದ್ದರೆ
#ದೋಷ "Missing builtin to flush instruction cache"
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ_ಅಂತ್ಯ

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
intlog2 (ಪೂರ್ಣಾಂಕ n)
{
  ಪೂರ್ಣಾಂಕ level = 0;
  ಆಗ (n >>= 1)
    ++level;
  ಹಿಂತಿರುಗಿ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_hfa0 (ಸ್ಥಿರಾಂಕ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಂಕ i, ret = -1;

  ಆದರೆ (elements != ಖಾಲಿ)
    ಗಾಗಿ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ಆದರೆ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_ಸದಿಶ
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ಆದರೆ (ret < 0)
              ಮುಂದುವರಿ;
          }
        ನಿಲ್ಲಿಸು;
      }

  ಹಿಂತಿರುಗಿ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ಸ್ಥಿರ size_t
is_simd (ಸ್ಥಿರಾಂಕ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (ty->type == FFI_TYPE_ಸದಿಶ)
    ಹಿಂತಿರುಗಿ ty->ಗಾತ್ರ_2;

  elements = ty->elements;
  ಆದರೆ (elements != ಖಾಲಿ)
    ಗಾಗಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಂಕ t = elements[i]->type;
        ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_ಸದಿಶ)
          ಹಿಂತಿರುಗಿ is_simd (elements[i]);
      }

  ಹಿಂತಿರುಗಿ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_hfa1 (ಸ್ಥಿರಾಂಕ ffi_type *ty, ಪೂರ್ಣಾಂಕ candidate)
{
  ffi_type **elements = ty->elements;
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (elements != ಖಾಲಿ)
    ಗಾಗಿ (i = 0; elements[i]; ++i)
      {
        ಪೂರ್ಣಾಂಕ t = elements[i]->type;
        ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_ಸದಿಶ
	    || t == FFI_TYPE_COMPLEX)
          {
            ಆದರೆ (!is_hfa1 (elements[i], candidate))
              ಹಿಂತಿರುಗಿ 0;
          }
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t != candidate)
          ಹಿಂತಿರುಗಿ 0;
      }

  ಹಿಂತಿರುಗಿ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
is_vfp_type (ಸ್ಥಿರಾಂಕ ffi_type *ty)
{
  ffi_type **elements;
  ಪೂರ್ಣಾಂಕ candidate, i;
  size_t ಗಾತ್ರ_2, ele_ಎಣಿಕೆ, simd_ಗಾತ್ರ;

  /* Quickest tests first.  */
  candidate = ty->type;
  ಆಯ್ಕೆ (candidate)
    {
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ele_ಎಣಿಕೆ = 1;
      ಹೋಗು ಮುಗಿದ;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ಆಯ್ಕೆ (candidate)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  ele_ಎಣಿಕೆ = 2;
	  ಹೋಗು ಮುಗಿದ;
	}
      ಹಿಂತಿರುಗಿ 0;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
      ನಿಲ್ಲಿಸು;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  ಗಾತ್ರ_2 = ty->ಗಾತ್ರ_2;
  ಆದರೆ (ಗಾತ್ರ_2 < 4 || ಗಾತ್ರ_2 > 64)
    ಹಿಂತಿರುಗಿ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_ಗಾತ್ರ = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ಆದರೆ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_ಸದಿಶ
      || candidate == FFI_TYPE_COMPLEX)
    {
      ಗಾಗಿ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ಆದರೆ (candidate >= 0)
            ನಿಲ್ಲಿಸು;
        }
    }

  ಆದರೆ (simd_ಗಾತ್ರ)
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
      size_t reg_ಗಾತ್ರ = simd_ಗಾತ್ರ;
      ಪೂರ್ಣಾಂಕ ಅಂಕೆ_registers;
      ಪೂರ್ಣಾಂಕ ಮೊದಲ_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ಆದರೆ (reg_ಗಾತ್ರ < 4 || reg_ಗಾತ್ರ > 16 || ಗಾತ್ರ_2 % reg_ಗಾತ್ರ != 0)
	ಹಿಂತಿರುಗಿ 0;
      ಅಂಕೆ_registers = (ಪೂರ್ಣಾಂಕ) (ಗಾತ್ರ_2 / reg_ಗಾತ್ರ);
      ಆದರೆ (ಅಂಕೆ_registers > 4)
	ಹಿಂತಿರುಗಿ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ಆದರೆ (ty->type != FFI_TYPE_ಸದಿಶ)
	ಗಾಗಿ (i = 0; elements[i]; ++i)
	  ಆದರೆ (is_simd (elements[i]) != reg_ಗಾತ್ರ)
	    ಹಿಂತಿರುಗಿ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      ಗಾಗಿ (i = 0; elements[i]; ++i)
	{
	  ಪೂರ್ಣಾಂಕ t = elements[i]->type;
	  ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_ಸದಿಶ
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ಆದರೆ (!is_hfa1 (elements[i], candidate))
		ಹಿಂತಿರುಗಿ 0;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t != candidate)
	    ಹಿಂತಿರುಗಿ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      ಮೊದಲ_level_element_type = FFI_TYPE_FLOAT + intlog2 ((ಪೂರ್ಣಾಂಕ) reg_ಗಾತ್ರ) - 2;
      ಹಿಂತಿರುಗಿ ಮೊದಲ_level_element_type * 4 + (4 - ಅಂಕೆ_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ಆಯ್ಕೆ (candidate)
    {
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ele_ಎಣಿಕೆ = ಗಾತ್ರ_2 / ಗಾತ್ರ(ದಶಮಾಂಶ);
      ಆದರೆ (ಗಾತ್ರ_2 != ele_ಎಣಿಕೆ * ಗಾತ್ರ(ದಶಮಾಂಶ))
        ಹಿಂತಿರುಗಿ 0;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ele_ಎಣಿಕೆ = ಗಾತ್ರ_2 / ಗಾತ್ರ(ದ್ವಿಗುಣ);
      ಆದರೆ (ಗಾತ್ರ_2 != ele_ಎಣಿಕೆ * ಗಾತ್ರ(ದ್ವಿಗುಣ))
        ಹಿಂತಿರುಗಿ 0;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ele_ಎಣಿಕೆ = ಗಾತ್ರ_2 / ಗಾತ್ರ(ದೀರ್ಘ ದ್ವಿಗುಣ);
      ಆದರೆ (ಗಾತ್ರ_2 != ele_ಎಣಿಕೆ * ಗಾತ್ರ(ದೀರ್ಘ ದ್ವಿಗುಣ))
        ಹಿಂತಿರುಗಿ 0;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
    }
  ಆದರೆ (ele_ಎಣಿಕೆ > 4)
    ಹಿಂತಿರುಗಿ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ಗಾಗಿ (i = 0; elements[i]; ++i)
    {
      ಪೂರ್ಣಾಂಕ t = elements[i]->type;
      ಆದರೆ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ಆದರೆ (!is_hfa1 (elements[i], candidate))
            ಹಿಂತಿರುಗಿ 0;
        }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (t != candidate)
        ಹಿಂತಿರುಗಿ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 ಮುಗಿದ:
  ಹಿಂತಿರುಗಿ candidate * 4 + (4 - (ಪೂರ್ಣಾಂಕ)ele_ಎಣಿಕೆ);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ರಚನೆ arg_state
{
  ಚಿಹ್ನೆರಹಿತ ngrn;                /* Next general-purpose register number. */
  ಚಿಹ್ನೆರಹಿತ nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t ಮುಂದಿನ_struct_area;	/* Place to allocate big structs. */

#ಆದರೆ defined (__APPLE__)
  ಚಿಹ್ನೆರಹಿತ allocating_variadic;
#ಆದರೆ_ಅಂತ್ಯ
};

/* Initialize a procedure call argument marshalling state.  */
ಸ್ಥಿರ ಶೂನ್ಯ
arg_init (ರಚನೆ arg_state *state, size_t ಗಾತ್ರ_2)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->ಮುಂದಿನ_struct_area = ಗಾತ್ರ_2;
#ಆದರೆ defined (__APPLE__)
  state->allocating_variadic = 0;
#ಆದರೆ_ಅಂತ್ಯ
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_ಗಮ್ಯ_ರಾಶಿ (ರಚನೆ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ,
		   size_t alignment, size_t ಗಾತ್ರ_2)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ಆದರೆ defined (__APPLE__)
  ಆದರೆ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (alignment < 8)
    alignment = 8;
#ಆದರೆ_ಅಂತ್ಯ

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + ಗಾತ್ರ_2;

  ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)ರಾಶಿ + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_and_copy_struct_ಗಮ್ಯ_ರಾಶಿ (ರಚನೆ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ,
				   size_t alignment, size_t ಗಾತ್ರ_2, ಶೂನ್ಯ *ಮೌಲ್ಯ)
{
  size_t dest = state->ಮುಂದಿನ_struct_area - ಗಾತ್ರ_2;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_ಕೆಳಕ್ಕೆ (dest, alignment);
  state->ಮುಂದಿನ_struct_area = dest;

  ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) ರಾಶಿ + dest, ಮೌಲ್ಯ, ಗಾತ್ರ_2);
}

ಸ್ಥಿರ ffi_arg
extend_integer_type (ಶೂನ್ಯ *ಮೂಲ, ಪೂರ್ಣಾಂಕ type)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        ಸ್ಮರಣೆ_ನಕಲು (&u8, ಮೂಲ, ಗಾತ್ರ (u8));
        ಹಿಂತಿರುಗಿ u8;
      }
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        ಸ್ಮರಣೆ_ನಕಲು (&s8, ಮೂಲ, ಗಾತ್ರ (s8));
        ಹಿಂತಿರುಗಿ s8;
      }
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        ಸ್ಮರಣೆ_ನಕಲು (&u16, ಮೂಲ, ಗಾತ್ರ (u16));
        ಹಿಂತಿರುಗಿ u16;
      }
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        ಸ್ಮರಣೆ_ನಕಲು (&s16, ಮೂಲ, ಗಾತ್ರ (s16));
        ಹಿಂತಿರುಗಿ s16;
      }
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        ಸ್ಮರಣೆ_ನಕಲು (&u32, ಮೂಲ, ಗಾತ್ರ (u32));
        ಹಿಂತಿರುಗಿ u32;
      }
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        ಸ್ಮರಣೆ_ನಕಲು (&s32, ಮೂಲ, ಗಾತ್ರ (s32));
        ಹಿಂತಿರುಗಿ s32;
      }
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        ಸ್ಮರಣೆ_ನಕಲು (&u64, ಮೂಲ, ಗಾತ್ರ (u64));
        ಹಿಂತಿರುಗಿ u64;
      }
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      {
        uintptr_t uptr;
        ಸ್ಮರಣೆ_ನಕಲು (&uptr, ಮೂಲ, ಗಾತ್ರ (uptr));
        ಹಿಂತಿರುಗಿ uptr;
      }
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
}

#ಆದರೆ defined(_MSC_VER) && !defined(__clang__)
ಶೂನ್ಯ extend_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಂಕ h);
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ
extend_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಂಕ h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  ಶೂನ್ಯ *x0;

#ವ್ಯಾಖ್ಯಾನಿಸು BTI_J "hint #36"
  __asm__ ಅಸ್ಥಿರ (
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
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_MSC_VER) && !defined(__clang__)
ಶೂನ್ಯ* compress_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *src, ಪೂರ್ಣಾಂಕ h);
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ *
compress_hfa_type (ಶೂನ್ಯ *dest, ಶೂನ್ಯ *reg, ಪೂರ್ಣಾಂಕ h)
{
  ಆಯ್ಕೆ (h)
    {
    ಸ್ಥಿತಿ AARCH64_RET_S1:
      ಆದರೆ (dest == reg)
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)dest + BE (12);
      ಇಲ್ಲದಿದ್ದರೆ
	*(ದಶಮಾಂಶ *)dest = *(ದಶಮಾಂಶ *)((ಅಕ್ಷರ *)reg + BE (12));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ AARCH64_RET_D1:
      ಆದರೆ (dest == reg)
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)dest + BE (8);
      ಇಲ್ಲದಿದ್ದರೆ
	*(ದ್ವಿಗುಣ *)dest = *(ದ್ವಿಗುಣ *)((ಅಕ್ಷರ *)reg + BE (8));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಆದರೆ (dest != reg)
	ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು (dest, reg, 16 * (4 - (h & 3)));
      ನಿಲ್ಲಿಸು;
    }
  ಹಿಂತಿರುಗಿ dest;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_int_ಗಮ್ಯ_reg_or_ರಾಶಿ (ರಚನೆ call_context *context,
			      ರಚನೆ arg_state *state,
			      ಶೂನ್ಯ *ರಾಶಿ, size_t ಗಾತ್ರ_2)
{
  ಆದರೆ (state->ngrn < N_X_ARG_REG)
    ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)&context->x[state->ngrn++] + BE (ಗಾತ್ರ (UINT64) - ಗಾತ್ರ_2);

  state->ngrn = N_X_ARG_REG;
  ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)allocate_ಗಮ್ಯ_ರಾಶಿ (state, ರಾಶಿ, ಗಾತ್ರ_2, ಗಾತ್ರ_2)
	 + BE (ಗಾತ್ರ (UINT64) - ಗಾತ್ರ_2);
}

ಸ್ಥಿರ ಶೂನ್ಯ *
allocate_int128_ಗಮ್ಯ_reg_or_ರಾಶಿ (ರಚನೆ call_context *context,
			         ರಚನೆ arg_state *state, ಶೂನ್ಯ *ರಾಶಿ)
{
  ಚಿಹ್ನೆರಹಿತ ngrn = state->ngrn;
  ಶೂನ್ಯ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __APPLE__
  ngrn += ngrn & 1;
#ಆದರೆ_ಅಂತ್ಯ

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ಆದರೆ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ret = allocate_ಗಮ್ಯ_ರಾಶಿ (state, ರಾಶಿ, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ಹಿಂತಿರುಗಿ ret;
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  ಪೂರ್ಣಾಂಕ flags, i, n;

  ಆಯ್ಕೆ (rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = (ಗಾತ್ರ(ಶೂನ್ಯ *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
    ಸ್ಥಿತಿ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
    ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ಆದರೆ (flags == 0)
	{
	  size_t s = rtype->ಗಾತ್ರ_2;
	  ಆದರೆ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_ಒಳ_MEM;
	      ಬೈಟುಗಳು += 8;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s == 16)
	    flags = AARCH64_RET_INT128;
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s == 8)
	    flags = AARCH64_RET_INT64;
	  ಇಲ್ಲದಿದ್ದರೆ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }

  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    ಆದರೆ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ನಿಲ್ಲಿಸು;
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
  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ಆದರೆ (ty->ಗಾತ್ರ_2 > 16 && !is_vfp_type (ty))
	ಬೈಟುಗಳು += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->ಬೈಟುಗಳು = (ಚಿಹ್ನೆರಹಿತ) FFI_ALIGN(ಬೈಟುಗಳು, 16);
  cif->flags = flags;
#ಆದರೆ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
			 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs)
{
  ffi_ಸ್ಥಿತಿ status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ಹಿಂತಿರುಗಿ status;
}
#ಇಲ್ಲದಿದ್ದರೆ
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs)
{
  ffi_ಸ್ಥಿತಿ status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ಹಿಂತಿರುಗಿ status;
}
#ಆದರೆ_ಅಂತ್ಯ /* __APPLE__ */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ರಚನೆ call_context *context, ಶೂನ್ಯ *ಚೌಕಟ್ಟು,
			   ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಪೂರ್ಣಾಂಕ flags,
			   ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *orig_rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ರಚನೆ call_context *context;
  ಶೂನ್ಯ *ರಾಶಿ, *ಚೌಕಟ್ಟು, *rvalue;
  ರಚನೆ arg_state state;
  size_t ರಾಶಿ_ಬೈಟುಗಳು, rtype_ಗಾತ್ರ, rsize;
  ಪೂರ್ಣಾಂಕ i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_ಗಾತ್ರ = rtype->ಗಾತ್ರ_2;
  ರಾಶಿ_ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;

  ಆದರೆ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ಆದರೆ (flags & AARCH64_RET_ಒಳ_MEM)
    {
      ಆದರೆ (orig_rvalue == ಖಾಲಿ)
	rsize = rtype_ಗಾತ್ರ;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (orig_rvalue == ಖಾಲಿ)
    flags &= AARCH64_FLAG_ARG_V;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (ಗಾತ್ರ(ರಚನೆ call_context) + ರಾಶಿ_ಬೈಟುಗಳು + 40 + rsize);
  ರಾಶಿ = context + 1;
  ಚೌಕಟ್ಟು = (ಶೂನ್ಯ*)((uintptr_t)ರಾಶಿ + (uintptr_t)ರಾಶಿ_ಬೈಟುಗಳು);
  rvalue = (rsize ? (ಶೂನ್ಯ*)((uintptr_t)ಚೌಕಟ್ಟು + 40) : orig_rvalue);

  arg_init (&state, ರಾಶಿ_ಬೈಟುಗಳು);
  ಗಾಗಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->ಗಾತ್ರ_2;
      ಶೂನ್ಯ *a = avalue[i];
      ಪೂರ್ಣಾಂಕ h, t;
      ಶೂನ್ಯ *dest;

      t = ty->type;
      ಆಯ್ಕೆ (t)
	{
	ಸ್ಥಿತಿ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ನಿಲ್ಲಿಸು;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	do_ಸೂಚಕ:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ಆದರೆ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		ಶೂನ್ಯ *d = allocate_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __APPLE__
		ಸ್ಮರಣೆ_ನಕಲು(d, a, s);
#ಇಲ್ಲದಿದ್ದರೆ
		*(ffi_arg *)d = ext;
#ಆದರೆ_ಅಂತ್ಯ
	      }
	  }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	  dest = allocate_int128_ಗಮ್ಯ_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ);
	  ಸ್ಮರಣೆ_ನಕಲು (dest, a, 16);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ಆದರೆ (h)
	      {
              ಪೂರ್ಣಾಂಕ elems = 4 - (h & 3);
              ಆದರೆ (cif->abi == FFI_WIN64 && isvariadic)
              {
                ಆದರೆ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ನಿಲ್ಲಿಸು;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_ಗಮ್ಯ_ರಾಶಿ(&state, ರಾಶಿ, ty->alignment, s);
              }
              ಇಲ್ಲದಿದ್ದರೆ
              {
                ಆದರೆ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ನಿಲ್ಲಿಸು;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (ಅಕ್ಷರ *)allocate_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (ಗಾತ್ರ (UINT64) - s) : 0);
              }
	      }
	    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_ಸೂಚಕ;
		s = ಗಾತ್ರ (ಶೂನ್ಯ *);
		ಹೋಗು do_ಸೂಚಕ;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		size_t n = (s + 7) / 8;
		ಆದರೆ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)n;
		  }
		ಇಲ್ಲದಿದ್ದರೆ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ, ty->alignment, s);
		  }
		}
	      ಸ್ಮರಣೆ_ನಕಲು (dest, a, s);
	    }
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}

#ಆದರೆ defined (__APPLE__)
      ಆದರೆ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#ಆದರೆ_ಅಂತ್ಯ
    }

  ffi_call_SYSV (context, ಚೌಕಟ್ಟು, fn, rvalue, flags, ಆವರಣ);

  ಆದರೆ (flags & AARCH64_RET_NEED_COPY)
    ಸ್ಮರಣೆ_ನಕಲು (orig_rvalue, rvalue, rtype_ಗಾತ್ರ);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

#ಆದರೆ FFI_CLOSURES

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}
#ಆದರೆ_ಅಂತ್ಯ /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV_V (ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV_alt (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_SYSV_V_alt (ಶೂನ್ಯ) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
                      ಶೂನ್ಯ *codeloc)
{
  ಆದರೆ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಶೂನ್ಯ (*ಆರಂಭ)(ಶೂನ್ಯ);

  ಆದರೆ (cif->flags & AARCH64_FLAG_ARG_V)
    ಆರಂಭ = ffi_ಆವರಣ_SYSV_V;
  ಇಲ್ಲದಿದ್ದರೆ
    ಆರಂಭ = ffi_ಆವರಣ_SYSV;

#ಆದರೆ FFI_EXEC_TRAMPOLINE_ಕೋಷ್ಟಕ
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_ದತ್ತಾಂಶ(codeloc, ptrauth_ಕೀಲಿ_function_ಸೂಚಕ, 0);
#  endif
  ಶೂನ್ಯ **config = (ಶೂನ್ಯ **)((uint8_t *)codeloc - ಪುಟ_MAX_ಗಾತ್ರ);
  config[0] = ಆವರಣ;
  config[1] = ಆರಂಭ;
# endif
#ಇಲ್ಲದಿದ್ದರೆ
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  ಅಕ್ಷರ *tramp = ಆವರಣ->tramp;

# ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಆದರೆ (ಆರಂಭ == ffi_ಆವರಣ_SYSV_V)
          ಆರಂಭ = ffi_ಆವರಣ_SYSV_V_alt;
      ಇಲ್ಲದಿದ್ದರೆ
          ಆರಂಭ = ffi_ಆವರಣ_SYSV_alt;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, ಆರಂಭ, ಆವರಣ);
      ಹೋಗು ಹೊರಹೋಗು;
    }
# endif

  /* Initialize the dynamic trampoline. */
  ಸ್ಮರಣೆ_ನಕಲು (tramp, trampoline, ಗಾತ್ರ(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)ಆರಂಭ;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_ಗಾತ್ರ);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *tramp_ಸಂಕೇತ = tramp;
# ಇಲ್ಲದಿದ್ದರೆ
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *tramp_ಸಂಕೇತ = ffi_ದತ್ತಾಂಶ_ಗಮ್ಯ_ಸಂಕೇತ_ಸೂಚಕ (tramp);
# endif
  ffi_clear_cache (tramp_ಸಂಕೇತ, tramp_ಸಂಕೇತ + FFI_TRAMPOLINE_ಗಾತ್ರ);
# ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಹೊರಹೋಗು:
# endif
#ಆದರೆ_ಅಂತ್ಯ

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_GO_CLOSURES
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_SYSV (ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_SYSV_V (ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ, ffi_cif* cif,
                     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ (*ಆರಂಭ)(ಶೂನ್ಯ);

  ಆದರೆ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆದರೆ (cif->flags & AARCH64_FLAG_ARG_V)
    ಆರಂಭ = ffi_go_ಆವರಣ_SYSV_V;
  ಇಲ್ಲದಿದ್ದರೆ
    ಆರಂಭ = ffi_go_ಆವರಣ_SYSV;

  ಆವರಣ->tramp = ಆರಂಭ;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ /* FFI_GO_CLOSURES */

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

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_SYSV_inner (ffi_cif *cif,
			ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
			ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			ರಚನೆ call_context *context,
			ಶೂನ್ಯ *ರಾಶಿ, ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ *struct_rvalue)
{
  ಶೂನ್ಯ **avalue = (ಶೂನ್ಯ**) alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ*));
  ಪೂರ್ಣಾಂಕ i, h, nargs, flags, isvariadic = 0;
  ರಚನೆ arg_state state;

  arg_init (&state, cif->ಬೈಟುಗಳು);

  flags = cif->flags;
  ಆದರೆ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  ಗಾಗಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ಪೂರ್ಣಾಂಕ t = ty->type;
      size_t n, s = ty->ಗಾತ್ರ_2;

      ಆಯ್ಕೆ (t)
	{
	ಸ್ಥಿತಿ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = allocate_int_ಗಮ್ಯ_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ, s);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_ಗಮ್ಯ_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_ಸದಿಶ:
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ಆದರೆ (h)
	    {
	      n = 4 - (h & 3);
              ಆದರೆ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ಆದರೆ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      ಶೂನ್ಯ *reg = &context->x[state.ngrn];
                      state.ngrn += (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ಇಲ್ಲದಿದ್ದರೆ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ಅಕ್ಷರ *)allocate_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ಗಾತ್ರ (UINT64) - s) : 0);
                    }
                }
              ಇಲ್ಲದಿದ್ದರೆ
                {
                  ಆದರೆ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      ಶೂನ್ಯ *reg = &context->v[state.nsrn];
                      state.nsrn += (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ಇಲ್ಲದಿದ್ದರೆ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ಅಕ್ಷರ *)allocate_ಗಮ್ಯ_ರಾಶಿ (&state, ರಾಶಿ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ಗಾತ್ರ (UINT64) - s) : 0);
                    }
                }
            }
          ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ILP32__
             UINT64 avalue_tmp;
             ಸ್ಮರಣೆ_ನಕಲು (&avalue_tmp,
                  allocate_int_ಗಮ್ಯ_reg_or_ರಾಶಿ (context, &state,
                                               ರಾಶಿ, ಗಾತ್ರ (ಶೂನ್ಯ *)),
                  ಗಾತ್ರ (UINT64));
             avalue[i] = (ಶೂನ್ಯ *)(UINT32)avalue_tmp;
#ಇಲ್ಲದಿದ್ದರೆ
              avalue[i] = *(ಶೂನ್ಯ **)
              allocate_int_ಗಮ್ಯ_reg_or_ರಾಶಿ (context, &state, ರಾಶಿ,
                                         ಗಾತ್ರ (ಶೂನ್ಯ *));
#ಆದರೆ_ಅಂತ್ಯ
            }
          ಇಲ್ಲದಿದ್ದರೆ
            {
              n = (s + 7) / 8;
              ಆದರೆ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)n;
                }
              ಇಲ್ಲದಿದ್ದರೆ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_ಗಮ್ಯ_ರಾಶಿ(&state, ರಾಶಿ,
                                           ty->alignment, s);
                }
            }
          ನಿಲ್ಲಿಸು;

        ಪೂರ್ವನಿಯೋಜಿತ:
          abort();
      }

#ಆದರೆ defined (__APPLE__)
      ಆದರೆ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#ಆದರೆ_ಅಂತ್ಯ
    }

  ಆದರೆ (flags & AARCH64_RET_ಒಳ_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ flags;
}

#ಆದರೆ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;

  *tramp_ಗಾತ್ರ = AARCH64_TRAMP_ಗಾತ್ರ;
  *map_ಗಾತ್ರ = AARCH64_TRAMP_MAP_ಗಾತ್ರ;
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */

#ಆದರೆ_ಅಂತ್ಯ /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

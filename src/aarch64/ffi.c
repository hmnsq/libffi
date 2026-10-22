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

#ਜੇ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fficonfig.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ WIN32_LEAN_AND_ਔਸਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <windows.h> /* FlushInstructionCache */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ਜੇ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ਜੇ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
#ਨਹੀਂਤਾਂ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ਜੇ_ਅੰਤ

ਸੰਘ _d
{
  UINT64 d;
  UINT32 s[2];
};

ਬਣਤਰ _v
{
  ਸੰਘ _d d[2] __attribute__((aligned(16)));
};

ਬਣਤਰ call_context
{
  ਬਣਤਰ _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ਜੇ FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MACH__
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_ARM64E_PTRAUTH
#ਅਨ੍ਤਰ੍ਭਾਵ <ptrauth.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <mach/vm_param.h>
#ਜੇ_ਅੰਤ

#ਨਹੀਂਤਾਂ

#ਜੇ defined (__clang__) && defined (__APPLE__)
ਬਾਹਰੀ ਖਾਲੀ sys_icache_invalidate (ਖਾਲੀ *ਸ਼ੁਰੂ, size_t len);
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ffi_clear_cache (ਖਾਲੀ *ਸ਼ੁਰੂ, ਖਾਲੀ *ਸਿਰਾ)
{
#ਜੇ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (ਸ਼ੁਰੂ, (ਅੱਖਰ *)ਸਿਰਾ - (ਅੱਖਰ *)ਸ਼ੁਰੂ);
#ਜਾਂਜੇ defined (__GNUC__)
  __builtin___clear_cache (ਸ਼ੁਰੂ, ਸਿਰਾ);
#ਜਾਂਜੇ defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), ਸ਼ੁਰੂ, (ਅੱਖਰ*)ਸਿਰਾ - (ਅੱਖਰ*)ਸ਼ੁਰੂ);
#ਨਹੀਂਤਾਂ
#ਗਲਤੀ "Missing builtin to flush instruction cache"
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅੰਤ

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ਸਥਿਰ ਪੂਰਨ
intlog2 (ਪੂਰਨ n)
{
  ਪੂਰਨ level = 0;
  ਜਦੋਂ (n >>= 1)
    ++level;
  ਵਾਪਸ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ਸਥਿਰ ਪੂਰਨ
is_hfa0 (ਅਚਲ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  ਪੂਰਨ i, ret = -1;

  ਜੇ (elements != ਨਿਰਾ)
    ਲਈ (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ਜੇ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_ਸਦਿਸ਼
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ਜੇ (ret < 0)
              ਜਾਰੀ;
          }
        ਰੋਕੋ;
      }

  ਵਾਪਸ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ਸਥਿਰ size_t
is_simd (ਅਚਲ ffi_type *ty)
{
  ffi_type **elements;
  ਪੂਰਨ i;

  ਜੇ (ty->type == FFI_TYPE_ਸਦਿਸ਼)
    ਵਾਪਸ ty->ਮਾਪ;

  elements = ty->elements;
  ਜੇ (elements != ਨਿਰਾ)
    ਲਈ (i = 0; elements[i]; ++i)
      {
        ਪੂਰਨ t = elements[i]->type;
        ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_ਸਦਿਸ਼)
          ਵਾਪਸ is_simd (elements[i]);
      }

  ਵਾਪਸ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ਸਥਿਰ ਪੂਰਨ
is_hfa1 (ਅਚਲ ffi_type *ty, ਪੂਰਨ candidate)
{
  ffi_type **elements = ty->elements;
  ਪੂਰਨ i;

  ਜੇ (elements != ਨਿਰਾ)
    ਲਈ (i = 0; elements[i]; ++i)
      {
        ਪੂਰਨ t = elements[i]->type;
        ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_ਸਦਿਸ਼
	    || t == FFI_TYPE_COMPLEX)
          {
            ਜੇ (!is_hfa1 (elements[i], candidate))
              ਵਾਪਸ 0;
          }
        ਨਹੀਂਤਾਂ ਜੇ (t != candidate)
          ਵਾਪਸ 0;
      }

  ਵਾਪਸ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ਸਥਿਰ ਪੂਰਨ
is_vfp_type (ਅਚਲ ffi_type *ty)
{
  ffi_type **elements;
  ਪੂਰਨ candidate, i;
  size_t ਮਾਪ, ele_ਗਿਣਤੀ, simd_ਮਾਪ;

  /* Quickest tests first.  */
  candidate = ty->type;
  ਚੋਣ (candidate)
    {
    ਮੂਲ:
      ਵਾਪਸ 0;
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ele_ਗਿਣਤੀ = 1;
      ਜਾਓ ਪੂਰਾ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ਚੋਣ (candidate)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  ele_ਗਿਣਤੀ = 2;
	  ਜਾਓ ਪੂਰਾ;
	}
      ਵਾਪਸ 0;
    ਹਾਲਤ FFI_TYPE_STRUCT:
    ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
      ਰੋਕੋ;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  ਮਾਪ = ty->ਮਾਪ;
  ਜੇ (ਮਾਪ < 4 || ਮਾਪ > 64)
    ਵਾਪਸ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_ਮਾਪ = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ਜੇ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_ਸਦਿਸ਼
      || candidate == FFI_TYPE_COMPLEX)
    {
      ਲਈ (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ਜੇ (candidate >= 0)
            ਰੋਕੋ;
        }
    }

  ਜੇ (simd_ਮਾਪ)
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
      size_t reg_ਮਾਪ = simd_ਮਾਪ;
      ਪੂਰਨ ਅੰਕ_registers;
      ਪੂਰਨ ਪਹਿਲਾ_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ਜੇ (reg_ਮਾਪ < 4 || reg_ਮਾਪ > 16 || ਮਾਪ % reg_ਮਾਪ != 0)
	ਵਾਪਸ 0;
      ਅੰਕ_registers = (ਪੂਰਨ) (ਮਾਪ / reg_ਮਾਪ);
      ਜੇ (ਅੰਕ_registers > 4)
	ਵਾਪਸ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ਜੇ (ty->type != FFI_TYPE_ਸਦਿਸ਼)
	ਲਈ (i = 0; elements[i]; ++i)
	  ਜੇ (is_simd (elements[i]) != reg_ਮਾਪ)
	    ਵਾਪਸ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      ਲਈ (i = 0; elements[i]; ++i)
	{
	  ਪੂਰਨ t = elements[i]->type;
	  ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_ਸਦਿਸ਼
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ਜੇ (!is_hfa1 (elements[i], candidate))
		ਵਾਪਸ 0;
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (t != candidate)
	    ਵਾਪਸ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      ਪਹਿਲਾ_level_element_type = FFI_TYPE_FLOAT + intlog2 ((ਪੂਰਨ) reg_ਮਾਪ) - 2;
      ਵਾਪਸ ਪਹਿਲਾ_level_element_type * 4 + (4 - ਅੰਕ_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ਚੋਣ (candidate)
    {
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ele_ਗਿਣਤੀ = ਮਾਪ / ਆਕਾਰ(ਦਸ਼ਮਲਵ);
      ਜੇ (ਮਾਪ != ele_ਗਿਣਤੀ * ਆਕਾਰ(ਦਸ਼ਮਲਵ))
        ਵਾਪਸ 0;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ele_ਗਿਣਤੀ = ਮਾਪ / ਆਕਾਰ(ਦੁੱਗਣਾ);
      ਜੇ (ਮਾਪ != ele_ਗਿਣਤੀ * ਆਕਾਰ(ਦੁੱਗਣਾ))
        ਵਾਪਸ 0;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ele_ਗਿਣਤੀ = ਮਾਪ / ਆਕਾਰ(ਲੰਮਾ ਦੁੱਗਣਾ);
      ਜੇ (ਮਾਪ != ele_ਗਿਣਤੀ * ਆਕਾਰ(ਲੰਮਾ ਦੁੱਗਣਾ))
        ਵਾਪਸ 0;
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ 0;
    }
  ਜੇ (ele_ਗਿਣਤੀ > 4)
    ਵਾਪਸ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  ਲਈ (i = 0; elements[i]; ++i)
    {
      ਪੂਰਨ t = elements[i]->type;
      ਜੇ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ਜੇ (!is_hfa1 (elements[i], candidate))
            ਵਾਪਸ 0;
        }
      ਨਹੀਂਤਾਂ ਜੇ (t != candidate)
        ਵਾਪਸ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 ਪੂਰਾ:
  ਵਾਪਸ candidate * 4 + (4 - (ਪੂਰਨ)ele_ਗਿਣਤੀ);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ਬਣਤਰ arg_state
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ngrn;                /* Next general-purpose register number. */
  ਬਿਨਾਨਿਸ਼ਾਨੀ nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t ਅਗਲਾ_struct_area;	/* Place to allocate big structs. */

#ਜੇ defined (__APPLE__)
  ਬਿਨਾਨਿਸ਼ਾਨੀ allocating_variadic;
#ਜੇ_ਅੰਤ
};

/* Initialize a procedure call argument marshalling state.  */
ਸਥਿਰ ਖਾਲੀ
arg_init (ਬਣਤਰ arg_state *state, size_t ਮਾਪ)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->ਅਗਲਾ_struct_area = ਮਾਪ;
#ਜੇ defined (__APPLE__)
  state->allocating_variadic = 0;
#ਜੇ_ਅੰਤ
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ਸਥਿਰ ਖਾਲੀ *
allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (ਬਣਤਰ arg_state *state, ਖਾਲੀ *ਢੇਰ,
		   size_t alignment, size_t ਮਾਪ)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ਜੇ defined (__APPLE__)
  ਜੇ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ਨਹੀਂਤਾਂ
  ਜੇ (alignment < 8)
    alignment = 8;
#ਜੇ_ਅੰਤ

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + ਮਾਪ;

  ਵਾਪਸ (ਅੱਖਰ *)ਢੇਰ + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ਸਥਿਰ ਖਾਲੀ *
allocate_and_copy_struct_ਮੰਜ਼ਿਲ_ਢੇਰ (ਬਣਤਰ arg_state *state, ਖਾਲੀ *ਢੇਰ,
				   size_t alignment, size_t ਮਾਪ, ਖਾਲੀ *ਮੁੱਲ)
{
  size_t dest = state->ਅਗਲਾ_struct_area - ਮਾਪ;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_ਥੱਲੇ (dest, alignment);
  state->ਅਗਲਾ_struct_area = dest;

  ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) ਢੇਰ + dest, ਮੁੱਲ, ਮਾਪ);
}

ਸਥਿਰ ffi_arg
extend_integer_type (ਖਾਲੀ *ਸਰੋਤ, ਪੂਰਨ type)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&u8, ਸਰੋਤ, ਆਕਾਰ (u8));
        ਵਾਪਸ u8;
      }
    ਹਾਲਤ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&s8, ਸਰੋਤ, ਆਕਾਰ (s8));
        ਵਾਪਸ s8;
      }
    ਹਾਲਤ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&u16, ਸਰੋਤ, ਆਕਾਰ (u16));
        ਵਾਪਸ u16;
      }
    ਹਾਲਤ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&s16, ਸਰੋਤ, ਆਕਾਰ (s16));
        ਵਾਪਸ s16;
      }
    ਹਾਲਤ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&u32, ਸਰੋਤ, ਆਕਾਰ (u32));
        ਵਾਪਸ u32;
      }
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&s32, ਸਰੋਤ, ਆਕਾਰ (s32));
        ਵਾਪਸ s32;
      }
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&u64, ਸਰੋਤ, ਆਕਾਰ (u64));
        ਵਾਪਸ u64;
      }
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      {
        uintptr_t uptr;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ (&uptr, ਸਰੋਤ, ਆਕਾਰ (uptr));
        ਵਾਪਸ uptr;
      }
    ਮੂਲ:
      abort();
    }
}

#ਜੇ defined(_MSC_VER) && !defined(__clang__)
ਖਾਲੀ extend_hfa_type (ਖਾਲੀ *dest, ਖਾਲੀ *src, ਪੂਰਨ h);
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ
extend_hfa_type (ਖਾਲੀ *dest, ਖਾਲੀ *src, ਪੂਰਨ h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  ਖਾਲੀ *x0;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BTI_J "hint #36"
  __asm__ ਅਸਥਿਰ (
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
#ਜੇ_ਅੰਤ

#ਜੇ defined(_MSC_VER) && !defined(__clang__)
ਖਾਲੀ* compress_hfa_type (ਖਾਲੀ *dest, ਖਾਲੀ *src, ਪੂਰਨ h);
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ *
compress_hfa_type (ਖਾਲੀ *dest, ਖਾਲੀ *reg, ਪੂਰਨ h)
{
  ਚੋਣ (h)
    {
    ਹਾਲਤ AARCH64_RET_S1:
      ਜੇ (dest == reg)
	ਵਾਪਸ (ਅੱਖਰ *)dest + BE (12);
      ਨਹੀਂਤਾਂ
	*(ਦਸ਼ਮਲਵ *)dest = *(ਦਸ਼ਮਲਵ *)((ਅੱਖਰ *)reg + BE (12));
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ਰੋਕੋ;

    ਹਾਲਤ AARCH64_RET_D1:
      ਜੇ (dest == reg)
	ਵਾਪਸ (ਅੱਖਰ *)dest + BE (8);
      ਨਹੀਂਤਾਂ
	*(ਦੁੱਗਣਾ *)dest = *(ਦੁੱਗਣਾ *)((ਅੱਖਰ *)reg + BE (8));
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ਰੋਕੋ;
    ਹਾਲਤ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ਰੋਕੋ;

    ਮੂਲ:
      ਜੇ (dest != reg)
	ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ (dest, reg, 16 * (4 - (h & 3)));
      ਰੋਕੋ;
    }
  ਵਾਪਸ dest;
}
#ਜੇ_ਅੰਤ

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ਸਥਿਰ ਖਾਲੀ *
allocate_int_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (ਬਣਤਰ call_context *context,
			      ਬਣਤਰ arg_state *state,
			      ਖਾਲੀ *ਢੇਰ, size_t ਮਾਪ)
{
  ਜੇ (state->ngrn < N_X_ARG_REG)
    ਵਾਪਸ (ਅੱਖਰ *)&context->x[state->ngrn++] + BE (ਆਕਾਰ (UINT64) - ਮਾਪ);

  state->ngrn = N_X_ARG_REG;
  ਵਾਪਸ (ਅੱਖਰ *)allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (state, ਢੇਰ, ਮਾਪ, ਮਾਪ)
	 + BE (ਆਕਾਰ (UINT64) - ਮਾਪ);
}

ਸਥਿਰ ਖਾਲੀ *
allocate_int128_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (ਬਣਤਰ call_context *context,
			         ਬਣਤਰ arg_state *state, ਖਾਲੀ *ਢੇਰ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ngrn = state->ngrn;
  ਖਾਲੀ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __APPLE__
  ngrn += ngrn & 1;
#ਜੇ_ਅੰਤ

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ਜੇ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ਨਹੀਂਤਾਂ
    {
      ret = allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (state, ਢੇਰ, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ਵਾਪਸ ret;
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  ਪੂਰਨ flags, i, n;

  ਚੋਣ (rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = (ਆਕਾਰ(ਖਾਲੀ *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_UINT128:
    ਹਾਲਤ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
    ਹਾਲਤ FFI_TYPE_STRUCT:
    ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ਜੇ (flags == 0)
	{
	  size_t s = rtype->ਮਾਪ;
	  ਜੇ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_ਵਿੱਚ_MEM;
	      ਬਾਈਟਾਂ += 8;
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (s == 16)
	    flags = AARCH64_RET_INT128;
	  ਨਹੀਂਤਾਂ ਜੇ (s == 8)
	    flags = AARCH64_RET_INT64;
	  ਨਹੀਂਤਾਂ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ਰੋਕੋ;

    ਮੂਲ:
      abort();
    }

  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    ਜੇ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ਰੋਕੋ;
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
  ਲਈ (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ਜੇ (ty->ਮਾਪ > 16 && !is_vfp_type (ty))
	ਬਾਈਟਾਂ += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->ਬਾਈਟਾਂ = (ਬਿਨਾਨਿਸ਼ਾਨੀ) FFI_ALIGN(ਬਾਈਟਾਂ, 16);
  cif->flags = flags;
#ਜੇ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#ਜੇ_ਅੰਤ

  ਵਾਪਸ FFI_OK;
}

#ਜੇ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  ffi_ਸਥਿਤੀ status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ਵਾਪਸ status;
}
#ਨਹੀਂਤਾਂ
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  ffi_ਸਥਿਤੀ status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ਵਾਪਸ status;
}
#ਜੇ_ਅੰਤ /* __APPLE__ */

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV (ਬਣਤਰ call_context *context, ਖਾਲੀ *ਚੌਖਟਾ,
			   ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਪੂਰਨ flags,
			   ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *orig_rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ਬਣਤਰ call_context *context;
  ਖਾਲੀ *ਢੇਰ, *ਚੌਖਟਾ, *rvalue;
  ਬਣਤਰ arg_state state;
  size_t ਢੇਰ_ਬਾਈਟਾਂ, rtype_ਮਾਪ, rsize;
  ਪੂਰਨ i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_ਮਾਪ = rtype->ਮਾਪ;
  ਢੇਰ_ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;

  ਜੇ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ਜੇ (flags & AARCH64_RET_ਵਿੱਚ_MEM)
    {
      ਜੇ (orig_rvalue == ਨਿਰਾ)
	rsize = rtype_ਮਾਪ;
    }
  ਨਹੀਂਤਾਂ ਜੇ (orig_rvalue == ਨਿਰਾ)
    flags &= AARCH64_FLAG_ARG_V;
  ਨਹੀਂਤਾਂ ਜੇ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (ਆਕਾਰ(ਬਣਤਰ call_context) + ਢੇਰ_ਬਾਈਟਾਂ + 40 + rsize);
  ਢੇਰ = context + 1;
  ਚੌਖਟਾ = (ਖਾਲੀ*)((uintptr_t)ਢੇਰ + (uintptr_t)ਢੇਰ_ਬਾਈਟਾਂ);
  rvalue = (rsize ? (ਖਾਲੀ*)((uintptr_t)ਚੌਖਟਾ + 40) : orig_rvalue);

  arg_init (&state, ਢੇਰ_ਬਾਈਟਾਂ);
  ਲਈ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->ਮਾਪ;
      ਖਾਲੀ *a = avalue[i];
      ਪੂਰਨ h, t;
      ਖਾਲੀ *dest;

      t = ty->type;
      ਚੋਣ (t)
	{
	ਹਾਲਤ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ਰੋਕੋ;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	do_ਸੂਚਕ:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ਜੇ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ਨਹੀਂਤਾਂ
	      {
		ਖਾਲੀ *d = allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __APPLE__
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(d, a, s);
#ਨਹੀਂਤਾਂ
		*(ffi_arg *)d = ext;
#ਜੇ_ਅੰਤ
	      }
	  }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_SINT128:
	  dest = allocate_int128_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (context, &state, ਢੇਰ);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (dest, a, 16);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ਜੇ (h)
	      {
              ਪੂਰਨ elems = 4 - (h & 3);
              ਜੇ (cif->abi == FFI_WIN64 && isvariadic)
              {
                ਜੇ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ਰੋਕੋ;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_ਮੰਜ਼ਿਲ_ਢੇਰ(&state, ਢੇਰ, ty->alignment, s);
              }
              ਨਹੀਂਤਾਂ
              {
                ਜੇ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ਰੋਕੋ;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (ਅੱਖਰ *)allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (ਆਕਾਰ (UINT64) - s) : 0);
              }
	      }
	    ਨਹੀਂਤਾਂ ਜੇ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_ਸੂਚਕ;
		s = ਆਕਾਰ (ਖਾਲੀ *);
		ਜਾਓ do_ਸੂਚਕ;
	      }
	    ਨਹੀਂਤਾਂ
	      {
		size_t n = (s + 7) / 8;
		ਜੇ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)n;
		  }
		ਨਹੀਂਤਾਂ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ, ty->alignment, s);
		  }
		}
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (dest, a, s);
	    }
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}

#ਜੇ defined (__APPLE__)
      ਜੇ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#ਜੇ_ਅੰਤ
    }

  ffi_call_SYSV (context, ਚੌਖਟਾ, fn, rvalue, flags, ਘੇਰਾ);

  ਜੇ (flags & AARCH64_RET_NEED_COPY)
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (orig_rvalue, rvalue, rtype_ਮਾਪ);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

#ਜੇ FFI_CLOSURES

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}
#ਜੇ_ਅੰਤ /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV_V (ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV_alt (ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV_V_alt (ਖਾਲੀ) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
                      ffi_cif* cif,
                      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
  ਜੇ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ਵਾਪਸ FFI_BAD_ABI;

  ਖਾਲੀ (*ਸ਼ੁਰੂ)(ਖਾਲੀ);

  ਜੇ (cif->flags & AARCH64_FLAG_ARG_V)
    ਸ਼ੁਰੂ = ffi_ਘੇਰਾ_SYSV_V;
  ਨਹੀਂਤਾਂ
    ਸ਼ੁਰੂ = ffi_ਘੇਰਾ_SYSV;

#ਜੇ FFI_EXEC_TRAMPOLINE_ਸਾਰਣੀ
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_ਅੰਕੜੇ(codeloc, ptrauth_ਕੁੰਜੀ_function_ਸੂਚਕ, 0);
#  endif
  ਖਾਲੀ **config = (ਖਾਲੀ **)((uint8_t *)codeloc - ਪੰਨਾ_MAX_ਮਾਪ);
  config[0] = ਘੇਰਾ;
  config[1] = ਸ਼ੁਰੂ;
# endif
#ਨਹੀਂਤਾਂ
  ਸਥਿਰ ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  ਅੱਖਰ *tramp = ਘੇਰਾ->tramp;

# ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਜੇ (ਸ਼ੁਰੂ == ffi_ਘੇਰਾ_SYSV_V)
          ਸ਼ੁਰੂ = ffi_ਘੇਰਾ_SYSV_V_alt;
      ਨਹੀਂਤਾਂ
          ਸ਼ੁਰੂ = ffi_ਘੇਰਾ_SYSV_alt;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, ਸ਼ੁਰੂ, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
# endif

  /* Initialize the dynamic trampoline. */
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tramp, trampoline, ਆਕਾਰ(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)ਸ਼ੁਰੂ;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_ਮਾਪ);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *tramp_ਕੋਡ = tramp;
# ਨਹੀਂਤਾਂ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *tramp_ਕੋਡ = ffi_ਅੰਕੜੇ_ਮੰਜ਼ਿਲ_ਕੋਡ_ਸੂਚਕ (tramp);
# endif
  ffi_clear_cache (tramp_ਕੋਡ, tramp_ਕੋਡ + FFI_TRAMPOLINE_ਮਾਪ);
# ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
# endif
#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_GO_CLOSURES
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_SYSV (ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_SYSV_V (ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif* cif,
                     ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*))
{
  ਖਾਲੀ (*ਸ਼ੁਰੂ)(ਖਾਲੀ);

  ਜੇ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ਵਾਪਸ FFI_BAD_ABI;

  ਜੇ (cif->flags & AARCH64_FLAG_ARG_V)
    ਸ਼ੁਰੂ = ffi_go_ਘੇਰਾ_SYSV_V;
  ਨਹੀਂਤਾਂ
    ਸ਼ੁਰੂ = ffi_go_ਘੇਰਾ_SYSV;

  ਘੇਰਾ->tramp = ਸ਼ੁਰੂ;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ /* FFI_GO_CLOSURES */

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

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_SYSV_inner (ffi_cif *cif,
			ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
			ਖਾਲੀ *user_ਅੰਕੜੇ,
			ਬਣਤਰ call_context *context,
			ਖਾਲੀ *ਢੇਰ, ਖਾਲੀ *rvalue, ਖਾਲੀ *struct_rvalue)
{
  ਖਾਲੀ **avalue = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ*));
  ਪੂਰਨ i, h, nargs, flags, isvariadic = 0;
  ਬਣਤਰ arg_state state;

  arg_init (&state, cif->ਬਾਈਟਾਂ);

  flags = cif->flags;
  ਜੇ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  ਲਈ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ਪੂਰਨ t = ty->type;
      size_t n, s = ty->ਮਾਪ;

      ਚੋਣ (t)
	{
	ਹਾਲਤ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = allocate_int_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (context, &state, ਢੇਰ, s);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (context, &state, ਢੇਰ);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_ਸਦਿਸ਼:
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ਜੇ (h)
	    {
	      n = 4 - (h & 3);
              ਜੇ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ਜੇ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      ਖਾਲੀ *reg = &context->x[state.ngrn];
                      state.ngrn += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ਨਹੀਂਤਾਂ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ਅੱਖਰ *)allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ਆਕਾਰ (UINT64) - s) : 0);
                    }
                }
              ਨਹੀਂਤਾਂ
                {
                  ਜੇ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      ਖਾਲੀ *reg = &context->v[state.nsrn];
                      state.nsrn += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ਨਹੀਂਤਾਂ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (ਅੱਖਰ *)allocate_ਮੰਜ਼ਿਲ_ਢੇਰ (&state, ਢੇਰ,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (ਆਕਾਰ (UINT64) - s) : 0);
                    }
                }
            }
          ਨਹੀਂਤਾਂ ਜੇ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ILP32__
             UINT64 avalue_tmp;
             ਸਮ੍ਰਿਤੀ_ਨਕਲ (&avalue_tmp,
                  allocate_int_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (context, &state,
                                               ਢੇਰ, ਆਕਾਰ (ਖਾਲੀ *)),
                  ਆਕਾਰ (UINT64));
             avalue[i] = (ਖਾਲੀ *)(UINT32)avalue_tmp;
#ਨਹੀਂਤਾਂ
              avalue[i] = *(ਖਾਲੀ **)
              allocate_int_ਮੰਜ਼ਿਲ_reg_or_ਢੇਰ (context, &state, ਢੇਰ,
                                         ਆਕਾਰ (ਖਾਲੀ *));
#ਜੇ_ਅੰਤ
            }
          ਨਹੀਂਤਾਂ
            {
              n = (s + 7) / 8;
              ਜੇ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)n;
                }
              ਨਹੀਂਤਾਂ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_ਮੰਜ਼ਿਲ_ਢੇਰ(&state, ਢੇਰ,
                                           ty->alignment, s);
                }
            }
          ਰੋਕੋ;

        ਮੂਲ:
          abort();
      }

#ਜੇ defined (__APPLE__)
      ਜੇ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#ਜੇ_ਅੰਤ
    }

  ਜੇ (flags & AARCH64_RET_ਵਿੱਚ_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  ਵਾਪਸ flags;
}

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *tramp_ਮਾਪ = AARCH64_TRAMP_ਮਾਪ;
  *map_ਮਾਪ = AARCH64_TRAMP_MAP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */

#ਜੇ_ਅੰਤ /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

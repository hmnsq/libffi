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

#ᨀᨘ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fficonfig.h>
#include <ffi.h>
#include <ffi_common.h>
#include "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h> /* FlushInstructionCache */
#endif
#include <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ᨀᨘ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ᨀᨘ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ᨀᨘᨁ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

sa_ngha _d
{
  UINT64 d;
  UINT32 s[2];
};

sa_nracanaa _v
{
  sa_ngha _d d[2] __attribute__((aligned(16)));
};

sa_nracanaa call_context
{
  sa_nracanaa _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ᨀᨘ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#include <ptrauth.h>
#endif
#include <mach/vm_param.h>
#endif

#ᨀᨘᨁ

#ᨀᨘ defined (__clang__) && defined (__APPLE__)
extern void sys_icache_invalidate (void *start, size_t len);
#endif

static anta_hstha void
ffi_clear_cache (void *start, void *end)
{
#ᨀᨘ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (var_na *)end - (var_na *)start);
#elif defined (__GNUC__)
  __builtin___clear_cache (start, end);
#elif defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (var_na*)end - (var_na*)start);
#ᨀᨘᨁ
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

static puur_naa_nka
intlog2 (puur_naa_nka n)
{
  puur_naa_nka level = 0;
  ᨀᨛ (n >>= 1)
    ++level;
  ᨀᨘᨒ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

static puur_naa_nka
is_hfa0 (const ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  puur_naa_nka i, ret = -1;

  ᨀᨘ (elements != NULL)
    for (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ᨀᨘ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ᨀᨘ (ret < 0)
              continue;
          }
        bha_nga;
      }

  ᨀᨘᨒ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

static size_t
is_simd (const ffi_type *ty)
{
  ffi_type **elements;
  puur_naa_nka i;

  ᨀᨘ (ty->type == FFI_TYPE_VECTOR)
    ᨀᨘᨒ ty->size;

  elements = ty->elements;
  ᨀᨘ (elements != NULL)
    for (i = 0; elements[i]; ++i)
      {
        puur_naa_nka t = elements[i]->type;
        ᨀᨘ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          ᨀᨘᨒ is_simd (elements[i]);
      }

  ᨀᨘᨒ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

static puur_naa_nka
is_hfa1 (const ffi_type *ty, puur_naa_nka candidate)
{
  ffi_type **elements = ty->elements;
  puur_naa_nka i;

  ᨀᨘ (elements != NULL)
    for (i = 0; elements[i]; ++i)
      {
        puur_naa_nka t = elements[i]->type;
        ᨀᨘ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            ᨀᨘ (!is_hfa1 (elements[i], candidate))
              ᨀᨘᨒ 0;
          }
        ᨀᨘᨁ ᨀᨘ (t != candidate)
          ᨀᨘᨒ 0;
      }

  ᨀᨘᨒ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

static puur_naa_nka
is_vfp_type (const ffi_type *ty)
{
  ffi_type **elements;
  puur_naa_nka candidate, i;
  size_t size, ele_count, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  switch (candidate)
    {
    default:
      ᨀᨘᨒ 0;
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_LONGDOUBLE:
      ele_count = 1;
      goto done;
    pak_sa FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      switch (candidate)
	{
	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_LONGDOUBLE:
	  ele_count = 2;
	  goto done;
	}
      ᨀᨘᨒ 0;
    pak_sa FFI_TYPE_STRUCT:
    pak_sa FFI_TYPE_VECTOR:
      bha_nga;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  ᨀᨘ (size < 4 || size > 64)
    ᨀᨘᨒ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ᨀᨘ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      for (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ᨀᨘ (candidate >= 0)
            bha_nga;
        }
    }

  ᨀᨘ (simd_size)
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
      puur_naa_nka num_registers;
      puur_naa_nka first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ᨀᨘ (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	ᨀᨘᨒ 0;
      num_registers = (puur_naa_nka) (size / reg_size);
      ᨀᨘ (num_registers > 4)
	ᨀᨘᨒ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ᨀᨘ (ty->type != FFI_TYPE_VECTOR)
	for (i = 0; elements[i]; ++i)
	  ᨀᨘ (is_simd (elements[i]) != reg_size)
	    ᨀᨘᨒ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      for (i = 0; elements[i]; ++i)
	{
	  puur_naa_nka t = elements[i]->type;
	  ᨀᨘ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ᨀᨘ (!is_hfa1 (elements[i], candidate))
		ᨀᨘᨒ 0;
	    }
	  ᨀᨘᨁ ᨀᨘ (t != candidate)
	    ᨀᨘᨒ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((puur_naa_nka) reg_size) - 2;
      ᨀᨘᨒ first_level_element_type * 4 + (4 - num_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  switch (candidate)
    {
    pak_sa FFI_TYPE_FLOAT:
      ele_count = size / parimaa_na(float);
      ᨀᨘ (size != ele_count * parimaa_na(float))
        ᨀᨘᨒ 0;
      bha_nga;
    pak_sa FFI_TYPE_DOUBLE:
      ele_count = size / parimaa_na(dviguu_na);
      ᨀᨘ (size != ele_count * parimaa_na(dviguu_na))
        ᨀᨘᨒ 0;
      bha_nga;
    pak_sa FFI_TYPE_LONGDOUBLE:
      ele_count = size / parimaa_na(long dviguu_na);
      ᨀᨘ (size != ele_count * parimaa_na(long dviguu_na))
        ᨀᨘᨒ 0;
      bha_nga;
    default:
      ᨀᨘᨒ 0;
    }
  ᨀᨘ (ele_count > 4)
    ᨀᨘᨒ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  for (i = 0; elements[i]; ++i)
    {
      puur_naa_nka t = elements[i]->type;
      ᨀᨘ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ᨀᨘ (!is_hfa1 (elements[i], candidate))
            ᨀᨘᨒ 0;
        }
      ᨀᨘᨁ ᨀᨘ (t != candidate)
        ᨀᨘᨒ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  ᨀᨘᨒ candidate * 4 + (4 - (puur_naa_nka)ele_count);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

sa_nracanaa arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#ᨀᨘ defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
static void
arg_init (sa_nracanaa arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#ᨀᨘ defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
static void *
allocate_to_stack (sa_nracanaa arg_state *state, void *stack,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ᨀᨘ defined (__APPLE__)
  ᨀᨘ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ᨀᨘᨁ
  ᨀᨘ (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  ᨀᨘᨒ (var_na *)stack + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
static void *
allocate_and_copy_struct_to_stack (sa_nracanaa arg_state *state, void *stack,
				   size_t alignment, size_t size, void *value)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  ᨀᨘᨒ memcpy ((var_na *) stack + dest, value, size);
}

static ffi_arg
extend_integer_type (void *source, puur_naa_nka type)
{
  switch (type)
    {
    pak_sa FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, parimaa_na (u8));
        ᨀᨘᨒ u8;
      }
    pak_sa FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, parimaa_na (s8));
        ᨀᨘᨒ s8;
      }
    pak_sa FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, parimaa_na (u16));
        ᨀᨘᨒ u16;
      }
    pak_sa FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, parimaa_na (s16));
        ᨀᨘᨒ s16;
      }
    pak_sa FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, parimaa_na (u32));
        ᨀᨘᨒ u32;
      }
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, parimaa_na (s32));
        ᨀᨘᨒ s32;
      }
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, parimaa_na (u64));
        ᨀᨘᨒ u64;
      }
    pak_sa FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, parimaa_na (uptr));
        ᨀᨘᨒ uptr;
      }
    default:
      abort();
    }
}

#ᨀᨘ defined(_MSC_VER) && !defined(__clang__)
void extend_hfa_type (void *dest, void *src, puur_naa_nka h);
#ᨀᨘᨁ
static void
extend_hfa_type (void *dest, void *src, puur_naa_nka h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  void *x0;

#define BTI_J "hint #36"
  __asm__ ca_ncala (
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

#ᨀᨘ defined(_MSC_VER) && !defined(__clang__)
void* compress_hfa_type (void *dest, void *src, puur_naa_nka h);
#ᨀᨘᨁ
static void *
compress_hfa_type (void *dest, void *reg, puur_naa_nka h)
{
  switch (h)
    {
    pak_sa AARCH64_RET_S1:
      ᨀᨘ (dest == reg)
	ᨀᨘᨒ (var_na *)dest + BE (12);
      ᨀᨘᨁ
	*(float *)dest = *(float *)((var_na *)reg + BE (12));
      bha_nga;
    pak_sa AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      bha_nga;
    pak_sa AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      bha_nga;
    pak_sa AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      bha_nga;

    pak_sa AARCH64_RET_D1:
      ᨀᨘ (dest == reg)
	ᨀᨘᨒ (var_na *)dest + BE (8);
      ᨀᨘᨁ
	*(dviguu_na *)dest = *(dviguu_na *)((var_na *)reg + BE (8));
      bha_nga;
    pak_sa AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      bha_nga;
    pak_sa AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      bha_nga;
    pak_sa AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      bha_nga;

    default:
      ᨀᨘ (dest != reg)
	ᨀᨘᨒ memcpy (dest, reg, 16 * (4 - (h & 3)));
      bha_nga;
    }
  ᨀᨘᨒ dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

static void *
allocate_int_to_reg_or_stack (sa_nracanaa call_context *context,
			      sa_nracanaa arg_state *state,
			      void *stack, size_t size)
{
  ᨀᨘ (state->ngrn < N_X_ARG_REG)
    ᨀᨘᨒ (var_na *)&context->x[state->ngrn++] + BE (parimaa_na (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  ᨀᨘᨒ (var_na *)allocate_to_stack (state, stack, size, size)
	 + BE (parimaa_na (UINT64) - size);
}

static void *
allocate_int128_to_reg_or_stack (sa_nracanaa call_context *context,
			         sa_nracanaa arg_state *state, void *stack)
{
  unsigned ngrn = state->ngrn;
  void *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ᨀᨘ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ᨀᨘᨁ
    {
      ret = allocate_to_stack (state, stack, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  ᨀᨘᨒ ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  puur_naa_nka flags, i, n;

  switch (rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      bha_nga;
    pak_sa FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      bha_nga;
    pak_sa FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      bha_nga;
    pak_sa FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      bha_nga;
    pak_sa FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      bha_nga;
    pak_sa FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      bha_nga;
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      bha_nga;
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      bha_nga;
    pak_sa FFI_TYPE_POINTER:
      flags = (parimaa_na(void *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      bha_nga;

    pak_sa FFI_TYPE_UINT128:
    pak_sa FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      bha_nga;

    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_LONGDOUBLE:
    pak_sa FFI_TYPE_STRUCT:
    pak_sa FFI_TYPE_VECTOR:
    pak_sa FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ᨀᨘ (flags == 0)
	{
	  size_t s = rtype->size;
	  ᨀᨘ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  ᨀᨘᨁ ᨀᨘ (s == 16)
	    flags = AARCH64_RET_INT128;
	  ᨀᨘᨁ ᨀᨘ (s == 8)
	    flags = AARCH64_RET_INT64;
	  ᨀᨘᨁ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      bha_nga;

    default:
      abort();
    }

  for (i = 0, n = cif->nargs; i < n; i++)
    ᨀᨘ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	bha_nga;
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
  for (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ᨀᨘ (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#ᨀᨘ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  ᨀᨘᨒ FFI_OK;
}

#ᨀᨘ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned puur_naa_nka nfixedargs,
			 unsigned puur_naa_nka ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  ᨀᨘᨒ status;
}
#ᨀᨘᨁ
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned puur_naa_nka nfixedargs, unsigned puur_naa_nka ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  ᨀᨘᨒ status;
}
#endif /* __APPLE__ */

extern void ffi_call_SYSV (sa_nracanaa call_context *context, void *frame,
			   void (*fn)(void), void *rvalue, puur_naa_nka flags,
			   void *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
static void
ffi_call_int (ffi_cif *cif, void (*fn)(void), void *orig_rvalue,
	      void **avalue, void *closure)
{
  sa_nracanaa call_context *context;
  void *stack, *frame, *rvalue;
  sa_nracanaa arg_state state;
  size_t stack_bytes, rtype_size, rsize;
  puur_naa_nka i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  stack_bytes = cif->bytes;

  ᨀᨘ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ᨀᨘ (flags & AARCH64_RET_IN_MEM)
    {
      ᨀᨘ (orig_rvalue == NULL)
	rsize = rtype_size;
    }
  ᨀᨘᨁ ᨀᨘ (orig_rvalue == NULL)
    flags &= AARCH64_FLAG_ARG_V;
  ᨀᨘᨁ ᨀᨘ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (parimaa_na(sa_nracanaa call_context) + stack_bytes + 40 + rsize);
  stack = context + 1;
  frame = (void*)((uintptr_t)stack + (uintptr_t)stack_bytes);
  rvalue = (rsize ? (void*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, stack_bytes);
  for (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      void *a = avalue[i];
      puur_naa_nka h, t;
      void *dest;

      t = ty->type;
      switch (t)
	{
	pak_sa FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  bha_nga;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ᨀᨘ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ᨀᨘᨁ
	      {
		void *d = allocate_to_stack (&state, stack, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#ᨀᨘᨁ
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  bha_nga;

	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_stack (context, &state, stack);
	  memcpy (dest, a, 16);
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_LONGDOUBLE:
	pak_sa FFI_TYPE_STRUCT:
	pak_sa FFI_TYPE_VECTOR:
	pak_sa FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ᨀᨘ (h)
	      {
              puur_naa_nka elems = 4 - (h & 3);
              ᨀᨘ (cif->abi == FFI_WIN64 && isvariadic)
              {
                ᨀᨘ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  bha_nga;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_stack(&state, stack, ty->alignment, s);
              }
              ᨀᨘᨁ
              {
                ᨀᨘ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  bha_nga;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (var_na *)allocate_to_stack (&state, stack, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (parimaa_na (UINT64) - s) : 0);
              }
	      }
	    ᨀᨘᨁ ᨀᨘ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_stack (&state, stack,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = parimaa_na (void *);
		goto do_pointer;
	      }
	    ᨀᨘᨁ
	      {
		size_t n = (s + 7) / 8;
		ᨀᨘ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned puur_naa_nka)n;
		  }
		ᨀᨘᨁ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_stack (&state, stack, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  bha_nga;

	default:
	  abort();
	}

#ᨀᨘ defined (__APPLE__)
      ᨀᨘ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  ᨀᨘ (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

void
ffi_call (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

#ᨀᨘ FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
void
ffi_call_go (ffi_cif *cif, void (*fn) (void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

extern void ffi_closure_SYSV (void) FFI_HIDDEN;
extern void ffi_closure_SYSV_V (void) FFI_HIDDEN;
#ᨀᨘ defined(FFI_EXEC_STATIC_TRAMP)
extern void ffi_closure_SYSV_alt (void) FFI_HIDDEN;
extern void ffi_closure_SYSV_V_alt (void) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      void (*fun)(ffi_cif*,void*,void**,void*),
                      void *user_data,
                      void *codeloc)
{
  ᨀᨘ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ᨀᨘᨒ FFI_BAD_ABI;

  void (*start)(void);

  ᨀᨘ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  ᨀᨘᨁ
    start = ffi_closure_SYSV;

#ᨀᨘ FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_key_function_pointer, 0);
#  endif
  void **config = (void **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#ᨀᨘᨁ
  static const unsigned var_na trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  var_na *tramp = closure->tramp;

# ᨀᨘ defined(FFI_EXEC_STATIC_TRAMP)
  ᨀᨘ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ᨀᨘ (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      ᨀᨘᨁ
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      goto out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, parimaa_na(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned var_na *tramp_code = tramp;
# ᨀᨘᨁ
  unsigned var_na *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# ᨀᨘ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ᨀᨘᨒ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
extern void ffi_go_closure_SYSV (void) FFI_HIDDEN;
extern void ffi_go_closure_SYSV_V (void) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     void (*fun)(ffi_cif*,void*,void**,void*))
{
  void (*start)(void);

  ᨀᨘ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    ᨀᨘᨒ FFI_BAD_ABI;

  ᨀᨘ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  ᨀᨘᨁ
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  ᨀᨘᨒ FFI_OK;
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

puur_naa_nka FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			void (*fun)(ffi_cif*,void*,void**,void*),
			void *user_data,
			sa_nracanaa call_context *context,
			void *stack, void *rvalue, void *struct_rvalue)
{
  void **avalue = (void**) alloca (cif->nargs * parimaa_na (void*));
  puur_naa_nka i, h, nargs, flags, isvariadic = 0;
  sa_nracanaa arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  ᨀᨘ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  for (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      puur_naa_nka t = ty->type;
      size_t n, s = ty->size;

      switch (t)
	{
	pak_sa FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  bha_nga;

	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_stack (context, &state, stack, s);
	  bha_nga;

	pak_sa FFI_TYPE_UINT128:
	pak_sa FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_stack (context, &state, stack);
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_LONGDOUBLE:
	pak_sa FFI_TYPE_STRUCT:
	pak_sa FFI_TYPE_VECTOR:
	pak_sa FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ᨀᨘ (h)
	    {
	      n = 4 - (h & 3);
              ᨀᨘ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ᨀᨘ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      void *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned puur_naa_nka)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ᨀᨘᨁ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (var_na *)allocate_to_stack (&state, stack,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (parimaa_na (UINT64) - s) : 0);
                    }
                }
              ᨀᨘᨁ
                {
                  ᨀᨘ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      void *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned puur_naa_nka)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ᨀᨘᨁ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (var_na *)allocate_to_stack (&state, stack,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (parimaa_na (UINT64) - s) : 0);
                    }
                }
            }
          ᨀᨘᨁ ᨀᨘ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_stack (context, &state,
                                               stack, parimaa_na (void *)),
                  parimaa_na (UINT64));
             avalue[i] = (void *)(UINT32)avalue_tmp;
#ᨀᨘᨁ
              avalue[i] = *(void **)
              allocate_int_to_reg_or_stack (context, &state, stack,
                                         parimaa_na (void *));
#endif
            }
          ᨀᨘᨁ
            {
              n = (s + 7) / 8;
              ᨀᨘ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned puur_naa_nka)n;
                }
              ᨀᨘᨁ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_stack(&state, stack,
                                           ty->alignment, s);
                }
            }
          bha_nga;

        default:
          abort();
      }

#ᨀᨘ defined (__APPLE__)
      ᨀᨘ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ᨀᨘ (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  ᨀᨘᨒ flags;
}

#ᨀᨘ defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  ᨀᨘᨒ &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

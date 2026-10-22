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

#만약 defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
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
#만약 FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# 만약 FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#아니면
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

union _d
{
  UINT64 d;
  UINT32 s[2];
};

struct _v
{
  union _d d[2] __attribute__((aligned(16)));
};

struct call_context
{
  struct _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#만약 FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#include <ptrauth.h>
#endif
#include <mach/vm_param.h>
#endif

#아니면

#만약 defined (__clang__) && defined (__APPLE__)
extern void sys_icache_invalidate (void *start, size_t len);
#endif

static inline void
ffi_clear_cache (void *start, void *end)
{
#만약 defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (char *)end - (char *)start);
#elif defined (__GNUC__)
  __builtin___clear_cache (start, end);
#elif defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (char*)end - (char*)start);
#아니면
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

static 정수
intlog2 (정수 n)
{
  정수 level = 0;
  동안 (n >>= 1)
    ++level;
  반환 level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

static 정수
is_hfa0 (const ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  정수 i, ret = -1;

  만약 (elements != NULL)
    위해 (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        만약 (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            만약 (ret < 0)
              continue;
          }
        break;
      }

  반환 ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

static size_t
is_simd (const ffi_type *ty)
{
  ffi_type **elements;
  정수 i;

  만약 (ty->type == FFI_TYPE_VECTOR)
    반환 ty->size;

  elements = ty->elements;
  만약 (elements != NULL)
    위해 (i = 0; elements[i]; ++i)
      {
        정수 t = elements[i]->type;
        만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          반환 is_simd (elements[i]);
      }

  반환 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

static 정수
is_hfa1 (const ffi_type *ty, 정수 candidate)
{
  ffi_type **elements = ty->elements;
  정수 i;

  만약 (elements != NULL)
    위해 (i = 0; elements[i]; ++i)
      {
        정수 t = elements[i]->type;
        만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            만약 (!is_hfa1 (elements[i], candidate))
              반환 0;
          }
        아니면 만약 (t != candidate)
          반환 0;
      }

  반환 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

static 정수
is_vfp_type (const ffi_type *ty)
{
  ffi_type **elements;
  정수 candidate, i;
  size_t size, ele_count, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  switch (candidate)
    {
    default:
      반환 0;
    case FFI_TYPE_FLOAT:
    case FFI_TYPE_DOUBLE:
    case FFI_TYPE_LONGDOUBLE:
      ele_count = 1;
      goto done;
    case FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      switch (candidate)
	{
	case FFI_TYPE_FLOAT:
	case FFI_TYPE_DOUBLE:
	case FFI_TYPE_LONGDOUBLE:
	  ele_count = 2;
	  goto done;
	}
      반환 0;
    case FFI_TYPE_STRUCT:
    case FFI_TYPE_VECTOR:
      break;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  만약 (size < 4 || size > 64)
    반환 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  만약 (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      위해 (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          만약 (candidate >= 0)
            break;
        }
    }

  만약 (simd_size)
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
      정수 num_registers;
      정수 first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      만약 (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	반환 0;
      num_registers = (정수) (size / reg_size);
      만약 (num_registers > 4)
	반환 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      만약 (ty->type != FFI_TYPE_VECTOR)
	위해 (i = 0; elements[i]; ++i)
	  만약 (is_simd (elements[i]) != reg_size)
	    반환 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      위해 (i = 0; elements[i]; ++i)
	{
	  정수 t = elements[i]->type;
	  만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      만약 (!is_hfa1 (elements[i], candidate))
		반환 0;
	    }
	  아니면 만약 (t != candidate)
	    반환 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((정수) reg_size) - 2;
      반환 first_level_element_type * 4 + (4 - num_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  switch (candidate)
    {
    case FFI_TYPE_FLOAT:
      ele_count = size / sizeof(float);
      만약 (size != ele_count * sizeof(float))
        반환 0;
      break;
    case FFI_TYPE_DOUBLE:
      ele_count = size / sizeof(double);
      만약 (size != ele_count * sizeof(double))
        반환 0;
      break;
    case FFI_TYPE_LONGDOUBLE:
      ele_count = size / sizeof(long double);
      만약 (size != ele_count * sizeof(long double))
        반환 0;
      break;
    default:
      반환 0;
    }
  만약 (ele_count > 4)
    반환 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  위해 (i = 0; elements[i]; ++i)
    {
      정수 t = elements[i]->type;
      만약 (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          만약 (!is_hfa1 (elements[i], candidate))
            반환 0;
        }
      아니면 만약 (t != candidate)
        반환 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  반환 candidate * 4 + (4 - (정수)ele_count);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

struct arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#만약 defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
static void
arg_init (struct arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#만약 defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
static void *
allocate_to_stack (struct arg_state *state, void *stack,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#만약 defined (__APPLE__)
  만약 (state->allocating_variadic && alignment < 8)
    alignment = 8;
#아니면
  만약 (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  반환 (char *)stack + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
static void *
allocate_and_copy_struct_to_stack (struct arg_state *state, void *stack,
				   size_t alignment, size_t size, void *value)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  반환 memcpy ((char *) stack + dest, value, size);
}

static ffi_arg
extend_integer_type (void *source, 정수 type)
{
  switch (type)
    {
    case FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, sizeof (u8));
        반환 u8;
      }
    case FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, sizeof (s8));
        반환 s8;
      }
    case FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, sizeof (u16));
        반환 u16;
      }
    case FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, sizeof (s16));
        반환 s16;
      }
    case FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, sizeof (u32));
        반환 u32;
      }
    case FFI_TYPE_INT:
    case FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, sizeof (s32));
        반환 s32;
      }
    case FFI_TYPE_UINT64:
    case FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, sizeof (u64));
        반환 u64;
      }
    case FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, sizeof (uptr));
        반환 uptr;
      }
    default:
      abort();
    }
}

#만약 defined(_MSC_VER) && !defined(__clang__)
void extend_hfa_type (void *dest, void *src, 정수 h);
#아니면
static void
extend_hfa_type (void *dest, void *src, 정수 h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  void *x0;

#define BTI_J "hint #36"
  __asm__ volatile (
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

#만약 defined(_MSC_VER) && !defined(__clang__)
void* compress_hfa_type (void *dest, void *src, 정수 h);
#아니면
static void *
compress_hfa_type (void *dest, void *reg, 정수 h)
{
  switch (h)
    {
    case AARCH64_RET_S1:
      만약 (dest == reg)
	반환 (char *)dest + BE (12);
      아니면
	*(float *)dest = *(float *)((char *)reg + BE (12));
      break;
    case AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      break;
    case AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      break;
    case AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      break;

    case AARCH64_RET_D1:
      만약 (dest == reg)
	반환 (char *)dest + BE (8);
      아니면
	*(double *)dest = *(double *)((char *)reg + BE (8));
      break;
    case AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      break;
    case AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      break;
    case AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      break;

    default:
      만약 (dest != reg)
	반환 memcpy (dest, reg, 16 * (4 - (h & 3)));
      break;
    }
  반환 dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

static void *
allocate_int_to_reg_or_stack (struct call_context *context,
			      struct arg_state *state,
			      void *stack, size_t size)
{
  만약 (state->ngrn < N_X_ARG_REG)
    반환 (char *)&context->x[state->ngrn++] + BE (sizeof (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  반환 (char *)allocate_to_stack (state, stack, size, size)
	 + BE (sizeof (UINT64) - size);
}

static void *
allocate_int128_to_reg_or_stack (struct call_context *context,
			         struct arg_state *state, void *stack)
{
  unsigned ngrn = state->ngrn;
  void *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  만약 (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  아니면
    {
      ret = allocate_to_stack (state, stack, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  반환 ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  정수 flags, i, n;

  switch (rtype->type)
    {
    case FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      break;
    case FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      break;
    case FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      break;
    case FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      break;
    case FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      break;
    case FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      break;
    case FFI_TYPE_INT:
    case FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      break;
    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      break;
    case FFI_TYPE_POINTER:
      flags = (sizeof(void *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      break;

    case FFI_TYPE_UINT128:
    case FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      break;

    case FFI_TYPE_FLOAT:
    case FFI_TYPE_DOUBLE:
    case FFI_TYPE_LONGDOUBLE:
    case FFI_TYPE_STRUCT:
    case FFI_TYPE_VECTOR:
    case FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      만약 (flags == 0)
	{
	  size_t s = rtype->size;
	  만약 (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  아니면 만약 (s == 16)
	    flags = AARCH64_RET_INT128;
	  아니면 만약 (s == 8)
	    flags = AARCH64_RET_INT64;
	  아니면
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      break;

    default:
      abort();
    }

  위해 (i = 0, n = cif->nargs; i < n; i++)
    만약 (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	break;
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
  위해 (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      만약 (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#만약 defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  반환 FFI_OK;
}

#만약 defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned 정수 nfixedargs,
			 unsigned 정수 ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  반환 status;
}
#아니면
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned 정수 nfixedargs, unsigned 정수 ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  반환 status;
}
#endif /* __APPLE__ */

extern void ffi_call_SYSV (struct call_context *context, void *frame,
			   void (*fn)(void), void *rvalue, 정수 flags,
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
  struct call_context *context;
  void *stack, *frame, *rvalue;
  struct arg_state state;
  size_t stack_bytes, rtype_size, rsize;
  정수 i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  stack_bytes = cif->bytes;

  만약 (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  만약 (flags & AARCH64_RET_IN_MEM)
    {
      만약 (orig_rvalue == NULL)
	rsize = rtype_size;
    }
  아니면 만약 (orig_rvalue == NULL)
    flags &= AARCH64_FLAG_ARG_V;
  아니면 만약 (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (sizeof(struct call_context) + stack_bytes + 40 + rsize);
  stack = context + 1;
  frame = (void*)((uintptr_t)stack + (uintptr_t)stack_bytes);
  rvalue = (rsize ? (void*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, stack_bytes);
  위해 (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      void *a = avalue[i];
      정수 h, t;
      void *dest;

      t = ty->type;
      switch (t)
	{
	case FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  break;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	case FFI_TYPE_INT:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT32:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT64:
	case FFI_TYPE_SINT64:
	case FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    만약 (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    아니면
	      {
		void *d = allocate_to_stack (&state, stack, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#아니면
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  break;

	case FFI_TYPE_UINT128:
	case FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_stack (context, &state, stack);
	  memcpy (dest, a, 16);
	  break;

	case FFI_TYPE_FLOAT:
	case FFI_TYPE_DOUBLE:
	case FFI_TYPE_LONGDOUBLE:
	case FFI_TYPE_STRUCT:
	case FFI_TYPE_VECTOR:
	case FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    만약 (h)
	      {
              정수 elems = 4 - (h & 3);
              만약 (cif->abi == FFI_WIN64 && isvariadic)
              {
                만약 (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  break;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_stack(&state, stack, ty->alignment, s);
              }
              아니면
              {
                만약 (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  break;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (char *)allocate_to_stack (&state, stack, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (sizeof (UINT64) - s) : 0);
              }
	      }
	    아니면 만약 (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_stack (&state, stack,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = sizeof (void *);
		goto do_pointer;
	      }
	    아니면
	      {
		size_t n = (s + 7) / 8;
		만약 (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned 정수)n;
		  }
		아니면
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
	  break;

	default:
	  abort();
	}

#만약 defined (__APPLE__)
      만약 (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  만약 (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

void
ffi_call (ffi_cif *cif, void (*fn) (void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

#만약 FFI_CLOSURES

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
#만약 defined(FFI_EXEC_STATIC_TRAMP)
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
  만약 (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    반환 FFI_BAD_ABI;

  void (*start)(void);

  만약 (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  아니면
    start = ffi_closure_SYSV;

#만약 FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_key_function_pointer, 0);
#  endif
  void **config = (void **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#아니면
  static const unsigned char trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  char *tramp = closure->tramp;

# 만약 defined(FFI_EXEC_STATIC_TRAMP)
  만약 (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      만약 (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      아니면
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      goto out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, sizeof(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned char *tramp_code = tramp;
# 아니면
  unsigned char *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# 만약 defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  반환 FFI_OK;
}

#ifdef FFI_GO_CLOSURES
extern void ffi_go_closure_SYSV (void) FFI_HIDDEN;
extern void ffi_go_closure_SYSV_V (void) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     void (*fun)(ffi_cif*,void*,void**,void*))
{
  void (*start)(void);

  만약 (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    반환 FFI_BAD_ABI;

  만약 (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  아니면
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  반환 FFI_OK;
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

정수 FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			void (*fun)(ffi_cif*,void*,void**,void*),
			void *user_data,
			struct call_context *context,
			void *stack, void *rvalue, void *struct_rvalue)
{
  void **avalue = (void**) alloca (cif->nargs * sizeof (void*));
  정수 i, h, nargs, flags, isvariadic = 0;
  struct arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  만약 (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  위해 (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      정수 t = ty->type;
      size_t n, s = ty->size;

      switch (t)
	{
	case FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  break;

	case FFI_TYPE_INT:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT32:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT64:
	case FFI_TYPE_SINT64:
	case FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_stack (context, &state, stack, s);
	  break;

	case FFI_TYPE_UINT128:
	case FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_stack (context, &state, stack);
	  break;

	case FFI_TYPE_FLOAT:
	case FFI_TYPE_DOUBLE:
	case FFI_TYPE_LONGDOUBLE:
	case FFI_TYPE_STRUCT:
	case FFI_TYPE_VECTOR:
	case FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  만약 (h)
	    {
	      n = 4 - (h & 3);
              만약 (cif->abi == FFI_WIN64 && isvariadic)
                {
                  만약 (state.ngrn + n <= N_X_ARG_REG)
                    {
                      void *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned 정수)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  아니면
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (char *)allocate_to_stack (&state, stack,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (sizeof (UINT64) - s) : 0);
                    }
                }
              아니면
                {
                  만약 (state.nsrn + n <= N_V_ARG_REG)
                    {
                      void *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned 정수)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  아니면
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (char *)allocate_to_stack (&state, stack,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (sizeof (UINT64) - s) : 0);
                    }
                }
            }
          아니면 만약 (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_stack (context, &state,
                                               stack, sizeof (void *)),
                  sizeof (UINT64));
             avalue[i] = (void *)(UINT32)avalue_tmp;
#아니면
              avalue[i] = *(void **)
              allocate_int_to_reg_or_stack (context, &state, stack,
                                         sizeof (void *));
#endif
            }
          아니면
            {
              n = (s + 7) / 8;
              만약 (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned 정수)n;
                }
              아니면
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_stack(&state, stack,
                                           ty->alignment, s);
                }
            }
          break;

        default:
          abort();
      }

#만약 defined (__APPLE__)
      만약 (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  만약 (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  반환 flags;
}

#만약 defined(FFI_EXEC_STATIC_TRAMP)
void *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  extern void *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  반환 &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

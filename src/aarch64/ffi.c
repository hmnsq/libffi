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

#إذا defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <stdint.h>
#ضمن <fficonfig.h>
#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ضمن <windows.h> /* FlushInstructionCache */
#endif
#ضمن <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#إذا FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

اتحاد _d
{
  UINT64 d;
  UINT32 s[2];
};

بنية _v
{
  اتحاد _d d[2] __attribute__((aligned(16)));
};

بنية call_context
{
  بنية _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#إذا FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#ضمن <ptrauth.h>
#endif
#ضمن <mach/vm_param.h>
#endif

#وإلا

#إذا defined (__clang__) && defined (__APPLE__)
خارجي فراغ sys_icache_invalidate (فراغ *start, size_t len);
#endif

ساكن مضمن فراغ
ffi_clear_cache (فراغ *start, فراغ *end)
{
#إذا defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (حرف *)end - (حرف *)start);
#وإلاإذا defined (__GNUC__)
  __builtin___clear_cache (start, end);
#وإلاإذا defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (حرف*)end - (حرف*)start);
#وإلا
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ساكن صحيح
intlog2 (صحيح n)
{
  صحيح level = 0;
  بينما (n >>= 1)
    ++level;
  إرجاع level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ساكن صحيح
is_hfa0 (ثابت ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  صحيح i, ret = -1;

  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        إذا (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            إذا (ret < 0)
              استمر;
          }
        اكسر;
      }

  إرجاع ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ساكن size_t
is_simd (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحيح i;

  إذا (ty->type == FFI_TYPE_VECTOR)
    إرجاع ty->size;

  elements = ty->elements;
  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        صحيح t = elements[i]->type;
        إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          إرجاع is_simd (elements[i]);
      }

  إرجاع 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ساكن صحيح
is_hfa1 (ثابت ffi_type *ty, صحيح candidate)
{
  ffi_type **elements = ty->elements;
  صحيح i;

  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        صحيح t = elements[i]->type;
        إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            إذا (!is_hfa1 (elements[i], candidate))
              إرجاع 0;
          }
        وإلا إذا (t != candidate)
          إرجاع 0;
      }

  إرجاع 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ساكن صحيح
is_vfp_type (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحيح candidate, i;
  size_t size, ele_تعداد, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  بدل (candidate)
    {
    افتراضي:
      إرجاع 0;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_LONGDOUBLE:
      ele_تعداد = 1;
      اذهب done;
    حالة FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      بدل (candidate)
	{
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  ele_تعداد = 2;
	  اذهب done;
	}
      إرجاع 0;
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_VECTOR:
      اكسر;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  إذا (size < 4 || size > 64)
    إرجاع 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  إذا (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      لكل (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          إذا (candidate >= 0)
            اكسر;
        }
    }

  إذا (simd_size)
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
      صحيح رقم_registers;
      صحيح first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      إذا (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	إرجاع 0;
      رقم_registers = (صحيح) (size / reg_size);
      إذا (رقم_registers > 4)
	إرجاع 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      إذا (ty->type != FFI_TYPE_VECTOR)
	لكل (i = 0; elements[i]; ++i)
	  إذا (is_simd (elements[i]) != reg_size)
	    إرجاع 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      لكل (i = 0; elements[i]; ++i)
	{
	  صحيح t = elements[i]->type;
	  إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      إذا (!is_hfa1 (elements[i], candidate))
		إرجاع 0;
	    }
	  وإلا إذا (t != candidate)
	    إرجاع 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((صحيح) reg_size) - 2;
      إرجاع first_level_element_type * 4 + (4 - رقم_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  بدل (candidate)
    {
    حالة FFI_TYPE_FLOAT:
      ele_تعداد = size / حجم(عائم);
      إذا (size != ele_تعداد * حجم(عائم))
        إرجاع 0;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      ele_تعداد = size / حجم(مزدوج);
      إذا (size != ele_تعداد * حجم(مزدوج))
        إرجاع 0;
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      ele_تعداد = size / حجم(long مزدوج);
      إذا (size != ele_تعداد * حجم(long مزدوج))
        إرجاع 0;
      اكسر;
    افتراضي:
      إرجاع 0;
    }
  إذا (ele_تعداد > 4)
    إرجاع 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  لكل (i = 0; elements[i]; ++i)
    {
      صحيح t = elements[i]->type;
      إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          إذا (!is_hfa1 (elements[i], candidate))
            إرجاع 0;
        }
      وإلا إذا (t != candidate)
        إرجاع 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  إرجاع candidate * 4 + (4 - (صحيح)ele_تعداد);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

بنية arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#إذا defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
ساكن فراغ
arg_init (بنية arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#إذا defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ساكن فراغ *
allocate_to_مكدس (بنية arg_state *state, فراغ *مكدس,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#إذا defined (__APPLE__)
  إذا (state->allocating_variadic && alignment < 8)
    alignment = 8;
#وإلا
  إذا (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  إرجاع (حرف *)مكدس + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_to_مكدس (بنية arg_state *state, فراغ *مكدس,
				   size_t alignment, size_t size, فراغ *قيمة)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  إرجاع memcpy ((حرف *) مكدس + dest, قيمة, size);
}

ساكن ffi_arg
extend_integer_type (فراغ *source, صحيح type)
{
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, حجم (u8));
        إرجاع u8;
      }
    حالة FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, حجم (s8));
        إرجاع s8;
      }
    حالة FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, حجم (u16));
        إرجاع u16;
      }
    حالة FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, حجم (s16));
        إرجاع s16;
      }
    حالة FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, حجم (u32));
        إرجاع u32;
      }
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, حجم (s32));
        إرجاع s32;
      }
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, حجم (u64));
        إرجاع u64;
      }
    حالة FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, حجم (uptr));
        إرجاع uptr;
      }
    افتراضي:
      abort();
    }
}

#إذا defined(_MSC_VER) && !defined(__clang__)
فراغ extend_hfa_type (فراغ *dest, فراغ *src, صحيح h);
#وإلا
ساكن فراغ
extend_hfa_type (فراغ *dest, فراغ *src, صحيح h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  فراغ *x0;

#define BTI_J "hint #36"
  __asm__ متطاير (
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

#إذا defined(_MSC_VER) && !defined(__clang__)
فراغ* compress_hfa_type (فراغ *dest, فراغ *src, صحيح h);
#وإلا
ساكن فراغ *
compress_hfa_type (فراغ *dest, فراغ *reg, صحيح h)
{
  بدل (h)
    {
    حالة AARCH64_RET_S1:
      إذا (dest == reg)
	إرجاع (حرف *)dest + BE (12);
      وإلا
	*(عائم *)dest = *(عائم *)((حرف *)reg + BE (12));
      اكسر;
    حالة AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      اكسر;
    حالة AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      اكسر;
    حالة AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      اكسر;

    حالة AARCH64_RET_D1:
      إذا (dest == reg)
	إرجاع (حرف *)dest + BE (8);
      وإلا
	*(مزدوج *)dest = *(مزدوج *)((حرف *)reg + BE (8));
      اكسر;
    حالة AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      اكسر;
    حالة AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      اكسر;
    حالة AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      اكسر;

    افتراضي:
      إذا (dest != reg)
	إرجاع memcpy (dest, reg, 16 * (4 - (h & 3)));
      اكسر;
    }
  إرجاع dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ساكن فراغ *
allocate_int_to_reg_or_مكدس (بنية call_context *context,
			      بنية arg_state *state,
			      فراغ *مكدس, size_t size)
{
  إذا (state->ngrn < N_X_ARG_REG)
    إرجاع (حرف *)&context->x[state->ngrn++] + BE (حجم (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  إرجاع (حرف *)allocate_to_مكدس (state, مكدس, size, size)
	 + BE (حجم (UINT64) - size);
}

ساكن فراغ *
allocate_int128_to_reg_or_مكدس (بنية call_context *context,
			         بنية arg_state *state, فراغ *مكدس)
{
  unsigned ngrn = state->ngrn;
  فراغ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  إذا (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  وإلا
    {
      ret = allocate_to_مكدس (state, مكدس, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  إرجاع ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  صحيح flags, i, n;

  بدل (rtype->type)
    {
    حالة FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      اكسر;
    حالة FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      اكسر;
    حالة FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      اكسر;
    حالة FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      اكسر;
    حالة FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      اكسر;
    حالة FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      اكسر;
    حالة FFI_TYPE_POINTER:
      flags = (حجم(فراغ *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      اكسر;

    حالة FFI_TYPE_UINT128:
    حالة FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      اكسر;

    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_LONGDOUBLE:
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_VECTOR:
    حالة FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      إذا (flags == 0)
	{
	  size_t s = rtype->size;
	  إذا (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  وإلا إذا (s == 16)
	    flags = AARCH64_RET_INT128;
	  وإلا إذا (s == 8)
	    flags = AARCH64_RET_INT64;
	  وإلا
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      اكسر;

    افتراضي:
      abort();
    }

  لكل (i = 0, n = cif->nargs; i < n; i++)
    إذا (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	اكسر;
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
  لكل (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      إذا (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#إذا defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  إرجاع FFI_OK;
}

#إذا defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحيح nfixedargs,
			 unsigned صحيح ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  إرجاع status;
}
#وإلا
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned صحيح nfixedargs, unsigned صحيح ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  إرجاع status;
}
#endif /* __APPLE__ */

خارجي فراغ ffi_call_SYSV (بنية call_context *context, فراغ *frame,
			   فراغ (*fn)(فراغ), فراغ *rvalue, صحيح flags,
			   فراغ *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *orig_rvalue,
	      فراغ **avalue, فراغ *closure)
{
  بنية call_context *context;
  فراغ *مكدس, *frame, *rvalue;
  بنية arg_state state;
  size_t مكدس_bytes, rtype_size, rsize;
  صحيح i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  مكدس_bytes = cif->bytes;

  إذا (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  إذا (flags & AARCH64_RET_IN_MEM)
    {
      إذا (orig_rvalue == عدم)
	rsize = rtype_size;
    }
  وإلا إذا (orig_rvalue == عدم)
    flags &= AARCH64_FLAG_ARG_V;
  وإلا إذا (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (حجم(بنية call_context) + مكدس_bytes + 40 + rsize);
  مكدس = context + 1;
  frame = (فراغ*)((uintptr_t)مكدس + (uintptr_t)مكدس_bytes);
  rvalue = (rsize ? (فراغ*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, مكدس_bytes);
  لكل (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      فراغ *a = avalue[i];
      صحيح h, t;
      فراغ *dest;

      t = ty->type;
      بدل (t)
	{
	حالة FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  اكسر;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    إذا (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    وإلا
	      {
		فراغ *d = allocate_to_مكدس (&state, مكدس, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#وإلا
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  اكسر;

	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_مكدس (context, &state, مكدس);
	  memcpy (dest, a, 16);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_VECTOR:
	حالة FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    إذا (h)
	      {
              صحيح elems = 4 - (h & 3);
              إذا (cif->abi == FFI_WIN64 && isvariadic)
              {
                إذا (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  اكسر;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_مكدس(&state, مكدس, ty->alignment, s);
              }
              وإلا
              {
                إذا (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  اكسر;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (حرف *)allocate_to_مكدس (&state, مكدس, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
              }
	      }
	    وإلا إذا (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_مكدس (&state, مكدس,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = حجم (فراغ *);
		اذهب do_pointer;
	      }
	    وإلا
	      {
		size_t n = (s + 7) / 8;
		إذا (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned صحيح)n;
		  }
		وإلا
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_مكدس (&state, مكدس, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  اكسر;

	افتراضي:
	  abort();
	}

#إذا defined (__APPLE__)
      إذا (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  إذا (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#إذا FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

خارجي فراغ ffi_closure_SYSV (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_closure_SYSV_V (فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
خارجي فراغ ffi_closure_SYSV_alt (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_closure_SYSV_V_alt (فراغ) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_data,
                      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    إرجاع FFI_BAD_ABI;

  فراغ (*start)(فراغ);

  إذا (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  وإلا
    start = ffi_closure_SYSV;

#إذا FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_مفتاح_function_pointer, 0);
#  endif
  فراغ **config = (فراغ **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#وإلا
  ساكن ثابت unsigned حرف trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  حرف *tramp = closure->tramp;

# إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      وإلا
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      اذهب out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, حجم(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned حرف *tramp_code = tramp;
# وإلا
  unsigned حرف *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# إذا defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  إرجاع FFI_OK;
}

#ifdef FFI_GO_CLOSURES
خارجي فراغ ffi_go_closure_SYSV (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_closure_SYSV_V (فراغ) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  فراغ (*start)(فراغ);

  إذا (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    إرجاع FFI_BAD_ABI;

  إذا (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  وإلا
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  إرجاع FFI_OK;
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

صحيح FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
			فراغ *user_data,
			بنية call_context *context,
			فراغ *مكدس, فراغ *rvalue, فراغ *struct_rvalue)
{
  فراغ **avalue = (فراغ**) alloca (cif->nargs * حجم (فراغ*));
  صحيح i, h, nargs, flags, isvariadic = 0;
  بنية arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  إذا (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  لكل (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      صحيح t = ty->type;
      size_t n, s = ty->size;

      بدل (t)
	{
	حالة FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_مكدس (context, &state, مكدس, s);
	  اكسر;

	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_مكدس (context, &state, مكدس);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_VECTOR:
	حالة FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  إذا (h)
	    {
	      n = 4 - (h & 3);
              إذا (cif->abi == FFI_WIN64 && isvariadic)
                {
                  إذا (state.ngrn + n <= N_X_ARG_REG)
                    {
                      فراغ *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned صحيح)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  وإلا
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (حرف *)allocate_to_مكدس (&state, مكدس,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
              وإلا
                {
                  إذا (state.nsrn + n <= N_V_ARG_REG)
                    {
                      فراغ *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned صحيح)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  وإلا
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (حرف *)allocate_to_مكدس (&state, مكدس,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
            }
          وإلا إذا (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_مكدس (context, &state,
                                               مكدس, حجم (فراغ *)),
                  حجم (UINT64));
             avalue[i] = (فراغ *)(UINT32)avalue_tmp;
#وإلا
              avalue[i] = *(فراغ **)
              allocate_int_to_reg_or_مكدس (context, &state, مكدس,
                                         حجم (فراغ *));
#endif
            }
          وإلا
            {
              n = (s + 7) / 8;
              إذا (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned صحيح)n;
                }
              وإلا
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_مكدس(&state, مكدس,
                                           ty->alignment, s);
                }
            }
          اكسر;

        افتراضي:
          abort();
      }

#إذا defined (__APPLE__)
      إذا (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  إذا (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  إرجاع flags;
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  خارجي فراغ *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  إرجاع &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

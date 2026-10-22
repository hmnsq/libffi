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
#جے_تعریف_شدہ _WIN32
#تعریف_کرو WIN32_LEAN_AND_اوسط
#ضمن <windows.h> /* FlushInstructionCache */
#جے_انت
#ضمن <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#إذا FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE باہرلا دا date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#جے_انت

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

#إذا FFI_EXEC_TRAMPOLINE_جدول

#جے_تعریف_شدہ __MACH__
#جے_تعریف_شدہ HAVE_ARM64E_PTRAUTH
#ضمن <ptrauth.h>
#جے_انت
#ضمن <mach/vm_param.h>
#جے_انت

#وإلا

#إذا defined (__clang__) && defined (__APPLE__)
خارجي فراغ sys_icache_invalidate (فراغ *شروع, size_t len);
#جے_انت

ساكن مضمن فراغ
ffi_clear_cache (فراغ *شروع, فراغ *سرا)
{
#إذا defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (شروع, (حرف *)سرا - (حرف *)شروع);
#وإلاإذا defined (__GNUC__)
  __builtin___clear_cache (شروع, سرا);
#وإلاإذا defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), شروع, (حرف*)سرا - (حرف*)شروع);
#وإلا
#غلطی "Missing builtin to flush instruction cache"
#جے_انت
}

#جے_انت

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ساكن صحيح
intlog2 (صحيح n)
{
  صحيح level = 0;
  طالما (n >>= 1)
    ++level;
  أرجع level;
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
        إذا (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_سمتیہ
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            إذا (ret < 0)
              استمر;
          }
        اكسر;
      }

  أرجع ret;
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

  إذا (ty->type == FFI_TYPE_سمتیہ)
    أرجع ty->ناپ;

  elements = ty->elements;
  إذا (elements != عدم)
    لكل (i = 0; elements[i]; ++i)
      {
        صحيح t = elements[i]->type;
        إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_سمتیہ)
          أرجع is_simd (elements[i]);
      }

  أرجع 0;
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
        إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_سمتیہ
	    || t == FFI_TYPE_COMPLEX)
          {
            إذا (!is_hfa1 (elements[i], candidate))
              أرجع 0;
          }
        وإلا إذا (t != candidate)
          أرجع 0;
      }

  أرجع 1;
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
  size_t ناپ, ele_گنتی, simd_ناپ;

  /* Quickest tests first.  */
  candidate = ty->type;
  بدل (candidate)
    {
    افتراضي:
      أرجع 0;
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_LONGDOUBLE:
      ele_گنتی = 1;
      اذهب پورا;
    حالة FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      بدل (candidate)
	{
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  ele_گنتی = 2;
	  اذهب پورا;
	}
      أرجع 0;
    حالة FFI_TYPE_STRUCT:
    حالة FFI_TYPE_سمتیہ:
      اكسر;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  ناپ = ty->ناپ;
  إذا (ناپ < 4 || ناپ > 64)
    أرجع 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_ناپ = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  إذا (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_سمتیہ
      || candidate == FFI_TYPE_COMPLEX)
    {
      لكل (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          إذا (candidate >= 0)
            اكسر;
        }
    }

  إذا (simd_ناپ)
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
      size_t reg_ناپ = simd_ناپ;
      صحيح عدد_registers;
      صحيح پہلا_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      إذا (reg_ناپ < 4 || reg_ناپ > 16 || ناپ % reg_ناپ != 0)
	أرجع 0;
      عدد_registers = (صحيح) (ناپ / reg_ناپ);
      إذا (عدد_registers > 4)
	أرجع 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      إذا (ty->type != FFI_TYPE_سمتیہ)
	لكل (i = 0; elements[i]; ++i)
	  إذا (is_simd (elements[i]) != reg_ناپ)
	    أرجع 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      لكل (i = 0; elements[i]; ++i)
	{
	  صحيح t = elements[i]->type;
	  إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_سمتیہ
	      || t == FFI_TYPE_COMPLEX)
	    {
	      إذا (!is_hfa1 (elements[i], candidate))
		أرجع 0;
	    }
	  وإلا إذا (t != candidate)
	    أرجع 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      پہلا_level_element_type = FFI_TYPE_FLOAT + intlog2 ((صحيح) reg_ناپ) - 2;
      أرجع پہلا_level_element_type * 4 + (4 - عدد_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  بدل (candidate)
    {
    حالة FFI_TYPE_FLOAT:
      ele_گنتی = ناپ / حجم(عائم);
      إذا (ناپ != ele_گنتی * حجم(عائم))
        أرجع 0;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      ele_گنتی = ناپ / حجم(مزدوج);
      إذا (ناپ != ele_گنتی * حجم(مزدوج))
        أرجع 0;
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      ele_گنتی = ناپ / حجم(لما مزدوج);
      إذا (ناپ != ele_گنتی * حجم(لما مزدوج))
        أرجع 0;
      اكسر;
    افتراضي:
      أرجع 0;
    }
  إذا (ele_گنتی > 4)
    أرجع 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  لكل (i = 0; elements[i]; ++i)
    {
      صحيح t = elements[i]->type;
      إذا (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          إذا (!is_hfa1 (elements[i], candidate))
            أرجع 0;
        }
      وإلا إذا (t != candidate)
        أرجع 0;
    }

  /* All tests succeeded.  Encode the result.  */
 پورا:
  أرجع candidate * 4 + (4 - (صحيح)ele_گنتی);
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
  بنانشانی ngrn;                /* Next general-purpose register number. */
  بنانشانی nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t اگلا_struct_area;	/* Place to allocate big structs. */

#إذا defined (__APPLE__)
  بنانشانی allocating_variadic;
#جے_انت
};

/* Initialize a procedure call argument marshalling state.  */
ساكن فراغ
arg_init (بنية arg_state *state, size_t ناپ)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->اگلا_struct_area = ناپ;
#إذا defined (__APPLE__)
  state->allocating_variadic = 0;
#جے_انت
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ساكن فراغ *
allocate_منزل_ڈھیر (بنية arg_state *state, فراغ *ڈھیر,
		   size_t alignment, size_t ناپ)
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
#جے_انت

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + ناپ;

  أرجع (حرف *)ڈھیر + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساكن فراغ *
allocate_and_copy_struct_منزل_ڈھیر (بنية arg_state *state, فراغ *ڈھیر,
				   size_t alignment, size_t ناپ, فراغ *قدر)
{
  size_t dest = state->اگلا_struct_area - ناپ;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_تھلے (dest, alignment);
  state->اگلا_struct_area = dest;

  أرجع حافظہ_نقل ((حرف *) ڈھیر + dest, قدر, ناپ);
}

ساكن ffi_arg
extend_integer_type (فراغ *سروت, صحيح type)
{
  بدل (type)
    {
    حالة FFI_TYPE_UINT8:
      {
        UINT8 u8;
        حافظہ_نقل (&u8, سروت, حجم (u8));
        أرجع u8;
      }
    حالة FFI_TYPE_SINT8:
      {
        SINT8 s8;
        حافظہ_نقل (&s8, سروت, حجم (s8));
        أرجع s8;
      }
    حالة FFI_TYPE_UINT16:
      {
        UINT16 u16;
        حافظہ_نقل (&u16, سروت, حجم (u16));
        أرجع u16;
      }
    حالة FFI_TYPE_SINT16:
      {
        SINT16 s16;
        حافظہ_نقل (&s16, سروت, حجم (s16));
        أرجع s16;
      }
    حالة FFI_TYPE_UINT32:
      {
        UINT32 u32;
        حافظہ_نقل (&u32, سروت, حجم (u32));
        أرجع u32;
      }
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      {
        SINT32 s32;
        حافظہ_نقل (&s32, سروت, حجم (s32));
        أرجع s32;
      }
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      {
        UINT64 u64;
        حافظہ_نقل (&u64, سروت, حجم (u64));
        أرجع u64;
      }
    حالة FFI_TYPE_اشارہ_گر:
      {
        uintptr_t uptr;
        حافظہ_نقل (&uptr, سروت, حجم (uptr));
        أرجع uptr;
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

#تعریف_کرو BTI_J "hint #36"
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
#جے_انت

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
	أرجع (حرف *)dest + BE (12);
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
	أرجع (حرف *)dest + BE (8);
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
	أرجع حافظہ_نقل (dest, reg, 16 * (4 - (h & 3)));
      اكسر;
    }
  أرجع dest;
}
#جے_انت

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ساكن فراغ *
allocate_int_منزل_reg_or_ڈھیر (بنية call_context *context,
			      بنية arg_state *state,
			      فراغ *ڈھیر, size_t ناپ)
{
  إذا (state->ngrn < N_X_ARG_REG)
    أرجع (حرف *)&context->x[state->ngrn++] + BE (حجم (UINT64) - ناپ);

  state->ngrn = N_X_ARG_REG;
  أرجع (حرف *)allocate_منزل_ڈھیر (state, ڈھیر, ناپ, ناپ)
	 + BE (حجم (UINT64) - ناپ);
}

ساكن فراغ *
allocate_int128_منزل_reg_or_ڈھیر (بنية call_context *context,
			         بنية arg_state *state, فراغ *ڈھیر)
{
  بنانشانی ngrn = state->ngrn;
  فراغ *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#جے_بنا_تعریف __APPLE__
  ngrn += ngrn & 1;
#جے_انت

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  إذا (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  وإلا
    {
      ret = allocate_منزل_ڈھیر (state, ڈھیر, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  أرجع ret;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t بائٹاں = cif->بائٹاں;
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
    حالة FFI_TYPE_اشارہ_گر:
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
    حالة FFI_TYPE_سمتیہ:
    حالة FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      إذا (flags == 0)
	{
	  size_t s = rtype->ناپ;
	  إذا (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_وچ_MEM;
	      بائٹاں += 8;
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
      إذا (ty->ناپ > 16 && !is_vfp_type (ty))
	بائٹاں += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->بائٹاں = (بنانشانی) FFI_ALIGN(بائٹاں, 16);
  cif->flags = flags;
#إذا defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#جے_انت

  أرجع FFI_OK;
}

#إذا defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشانی صحيح nfixedargs,
			 بنانشانی صحيح ntotalargs)
{
  ffi_حالت status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  أرجع status;
}
#وإلا
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشانی صحيح nfixedargs, بنانشانی صحيح ntotalargs)
{
  ffi_حالت status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  أرجع status;
}
#جے_انت /* __APPLE__ */

خارجي فراغ ffi_call_SYSV (بنية call_context *context, فراغ *چوکھٹا,
			   فراغ (*fn)(فراغ), فراغ *rvalue, صحيح flags,
			   فراغ *گھیرا) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *orig_rvalue,
	      فراغ **avalue, فراغ *گھیرا)
{
  بنية call_context *context;
  فراغ *ڈھیر, *چوکھٹا, *rvalue;
  بنية arg_state state;
  size_t ڈھیر_بائٹاں, rtype_ناپ, rsize;
  صحيح i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_ناپ = rtype->ناپ;
  ڈھیر_بائٹاں = cif->بائٹاں;

  إذا (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  إذا (flags & AARCH64_RET_وچ_MEM)
    {
      إذا (orig_rvalue == عدم)
	rsize = rtype_ناپ;
    }
  وإلا إذا (orig_rvalue == عدم)
    flags &= AARCH64_FLAG_ARG_V;
  وإلا إذا (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (حجم(بنية call_context) + ڈھیر_بائٹاں + 40 + rsize);
  ڈھیر = context + 1;
  چوکھٹا = (فراغ*)((uintptr_t)ڈھیر + (uintptr_t)ڈھیر_بائٹاں);
  rvalue = (rsize ? (فراغ*)((uintptr_t)چوکھٹا + 40) : orig_rvalue);

  arg_init (&state, ڈھیر_بائٹاں);
  لكل (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->ناپ;
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
	حالة FFI_TYPE_اشارہ_گر:
	do_اشارہ_گر:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    إذا (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    وإلا
	      {
		فراغ *d = allocate_منزل_ڈھیر (&state, ڈھیر, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#جے_تعریف_شدہ __APPLE__
		حافظہ_نقل(d, a, s);
#وإلا
		*(ffi_arg *)d = ext;
#جے_انت
	      }
	  }
	  اكسر;

	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_SINT128:
	  dest = allocate_int128_منزل_reg_or_ڈھیر (context, &state, ڈھیر);
	  حافظہ_نقل (dest, a, 16);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_سمتیہ:
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
                dest = allocate_منزل_ڈھیر(&state, ڈھیر, ty->alignment, s);
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
                dest = (حرف *)allocate_منزل_ڈھیر (&state, ڈھیر, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
              }
	      }
	    وإلا إذا (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_منزل_ڈھیر (&state, ڈھیر,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_اشارہ_گر;
		s = حجم (فراغ *);
		اذهب do_اشارہ_گر;
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
                    state.ngrn += (بنانشانی صحيح)n;
		  }
		وإلا
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_منزل_ڈھیر (&state, ڈھیر, ty->alignment, s);
		  }
		}
	      حافظہ_نقل (dest, a, s);
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
#جے_انت
    }

  ffi_call_SYSV (context, چوکھٹا, fn, rvalue, flags, گھیرا);

  إذا (flags & AARCH64_RET_NEED_COPY)
    حافظہ_نقل (orig_rvalue, rvalue, rtype_ناپ);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

#إذا FFI_CLOSURES

#جے_تعریف_شدہ FFI_GO_CLOSURES
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گھیرا)
{
  ffi_call_int (cif, fn, rvalue, avalue, گھیرا);
}
#جے_انت /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

خارجي فراغ ffi_گھیرا_SYSV (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_گھیرا_SYSV_V (فراغ) FFI_HIDDEN;
#إذا defined(FFI_EXEC_STATIC_TRAMP)
خارجي فراغ ffi_گھیرا_SYSV_alt (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_گھیرا_SYSV_V_alt (فراغ) FFI_HIDDEN;
#جے_انت

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_انکڑے,
                      فراغ *codeloc)
{
  إذا (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    أرجع FFI_BAD_ABI;

  فراغ (*شروع)(فراغ);

  إذا (cif->flags & AARCH64_FLAG_ARG_V)
    شروع = ffi_گھیرا_SYSV_V;
  وإلا
    شروع = ffi_گھیرا_SYSV;

#إذا FFI_EXEC_TRAMPOLINE_جدول
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_انکڑے(codeloc, ptrauth_کلید_function_اشارہ_گر, 0);
#  endif
  فراغ **config = (فراغ **)((uint8_t *)codeloc - ورقہ_MAX_ناپ);
  config[0] = گھیرا;
  config[1] = شروع;
# endif
#وإلا
  ساكن ثابت بنانشانی حرف trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  حرف *tramp = گھیرا->tramp;

# إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      إذا (شروع == ffi_گھیرا_SYSV_V)
          شروع = ffi_گھیرا_SYSV_V_alt;
      وإلا
          شروع = ffi_گھیرا_SYSV_alt;
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, شروع, گھیرا);
      اذهب باہرلا;
    }
# endif

  /* Initialize the dynamic trampoline. */
  حافظہ_نقل (tramp, trampoline, حجم(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)شروع;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_ناپ);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  بنانشانی حرف *tramp_کوڈ = tramp;
# وإلا
  بنانشانی حرف *tramp_کوڈ = ffi_انکڑے_منزل_کوڈ_اشارہ_گر (tramp);
# endif
  ffi_clear_cache (tramp_کوڈ, tramp_کوڈ + FFI_TRAMPOLINE_ناپ);
# إذا defined(FFI_EXEC_STATIC_TRAMP)
باہرلا:
# endif
#جے_انت

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

#جے_تعریف_شدہ FFI_GO_CLOSURES
خارجي فراغ ffi_go_گھیرا_SYSV (فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_گھیرا_SYSV_V (فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا, ffi_cif* cif,
                     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  فراغ (*شروع)(فراغ);

  إذا (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    أرجع FFI_BAD_ABI;

  إذا (cif->flags & AARCH64_FLAG_ARG_V)
    شروع = ffi_go_گھیرا_SYSV_V;
  وإلا
    شروع = ffi_go_گھیرا_SYSV;

  گھیرا->tramp = شروع;
  گھیرا->cif = cif;
  گھیرا->fun = fun;

  أرجع FFI_OK;
}
#جے_انت /* FFI_GO_CLOSURES */

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
ffi_گھیرا_SYSV_inner (ffi_cif *cif,
			فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
			فراغ *user_انکڑے,
			بنية call_context *context,
			فراغ *ڈھیر, فراغ *rvalue, فراغ *struct_rvalue)
{
  فراغ **avalue = (فراغ**) alloca (cif->nargs * حجم (فراغ*));
  صحيح i, h, nargs, flags, isvariadic = 0;
  بنية arg_state state;

  arg_init (&state, cif->بائٹاں);

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
      size_t n, s = ty->ناپ;

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
	حالة FFI_TYPE_اشارہ_گر:
	  avalue[i] = allocate_int_منزل_reg_or_ڈھیر (context, &state, ڈھیر, s);
	  اكسر;

	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_منزل_reg_or_ڈھیر (context, &state, ڈھیر);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_سمتیہ:
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
                      state.ngrn += (بنانشانی صحيح)n;

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
                      avalue[i] = (حرف *)allocate_منزل_ڈھیر (&state, ڈھیر,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
              وإلا
                {
                  إذا (state.nsrn + n <= N_V_ARG_REG)
                    {
                      فراغ *reg = &context->v[state.nsrn];
                      state.nsrn += (بنانشانی صحيح)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  وإلا
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (حرف *)allocate_منزل_ڈھیر (&state, ڈھیر,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
            }
          وإلا إذا (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#جے_تعریف_شدہ __ILP32__
             UINT64 avalue_tmp;
             حافظہ_نقل (&avalue_tmp,
                  allocate_int_منزل_reg_or_ڈھیر (context, &state,
                                               ڈھیر, حجم (فراغ *)),
                  حجم (UINT64));
             avalue[i] = (فراغ *)(UINT32)avalue_tmp;
#وإلا
              avalue[i] = *(فراغ **)
              allocate_int_منزل_reg_or_ڈھیر (context, &state, ڈھیر,
                                         حجم (فراغ *));
#جے_انت
            }
          وإلا
            {
              n = (s + 7) / 8;
              إذا (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (بنانشانی صحيح)n;
                }
              وإلا
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_منزل_ڈھیر(&state, ڈھیر,
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
#جے_انت
    }

  إذا (flags & AARCH64_RET_وچ_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_انکڑے);

  أرجع flags;
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;

  *tramp_ناپ = AARCH64_TRAMP_ناپ;
  *map_ناپ = AARCH64_TRAMP_MAP_ناپ;
  أرجع &trampoline_کوڈ_جدول;
}
#جے_انت

#جے_انت /* FFI_CLOSURES */

#جے_انت /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

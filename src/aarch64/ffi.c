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

#اگر defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <stdint.h>
#شامل <fficonfig.h>
#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل "internal.h"
#اگر_تعریف_شدہ _WIN32
#تعریف_کرو WIN32_LEAN_AND_اوسط
#شامل <windows.h> /* FlushInstructionCache */
#اگر_ختم
#شامل <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#اگر FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# اگر FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE برآمد کا date
# endif
#ورنہ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#اگر_ختم

اتحاد _d
{
  UINT64 d;
  UINT32 s[2];
};

ساخت _v
{
  اتحاد _d d[2] __attribute__((aligned(16)));
};

ساخت call_context
{
  ساخت _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#اگر FFI_EXEC_TRAMPOLINE_جدول

#اگر_تعریف_شدہ __MACH__
#اگر_تعریف_شدہ HAVE_ARM64E_PTRAUTH
#شامل <ptrauth.h>
#اگر_ختم
#شامل <mach/vm_param.h>
#اگر_ختم

#ورنہ

#اگر defined (__clang__) && defined (__APPLE__)
خارجی خالی sys_icache_invalidate (خالی *آغاز, size_t len);
#اگر_ختم

ساکن اندرونی خالی
ffi_clear_cache (خالی *آغاز, خالی *انتہا)
{
#اگر defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (آغاز, (حرف *)انتہا - (حرف *)آغاز);
#وگرنہ_اگر defined (__GNUC__)
  __builtin___clear_cache (آغاز, انتہا);
#وگرنہ_اگر defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), آغاز, (حرف*)انتہا - (حرف*)آغاز);
#ورنہ
#غلطی "Missing builtin to flush instruction cache"
#اگر_ختم
}

#اگر_ختم

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

ساکن صحیح
intlog2 (صحیح n)
{
  صحیح level = 0;
  جب (n >>= 1)
    ++level;
  واپس level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

ساکن صحیح
is_hfa0 (ثابت ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  صحیح i, ret = -1;

  اگر (elements != تہی)
    برائے (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        اگر (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_سمتیہ
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            اگر (ret < 0)
              جاری;
          }
        توڑو;
      }

  واپس ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

ساکن size_t
is_simd (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحیح i;

  اگر (ty->type == FFI_TYPE_سمتیہ)
    واپس ty->سائز;

  elements = ty->elements;
  اگر (elements != تہی)
    برائے (i = 0; elements[i]; ++i)
      {
        صحیح t = elements[i]->type;
        اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_سمتیہ)
          واپس is_simd (elements[i]);
      }

  واپس 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

ساکن صحیح
is_hfa1 (ثابت ffi_type *ty, صحیح candidate)
{
  ffi_type **elements = ty->elements;
  صحیح i;

  اگر (elements != تہی)
    برائے (i = 0; elements[i]; ++i)
      {
        صحیح t = elements[i]->type;
        اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_سمتیہ
	    || t == FFI_TYPE_COMPLEX)
          {
            اگر (!is_hfa1 (elements[i], candidate))
              واپس 0;
          }
        ورنہ اگر (t != candidate)
          واپس 0;
      }

  واپس 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

ساکن صحیح
is_vfp_type (ثابت ffi_type *ty)
{
  ffi_type **elements;
  صحیح candidate, i;
  size_t سائز, ele_گنتی, simd_سائز;

  /* Quickest tests first.  */
  candidate = ty->type;
  انتخاب (candidate)
    {
    طےشدہ:
      واپس 0;
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_LONGDOUBLE:
      ele_گنتی = 1;
      جاؤ مکمل;
    حالت FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      انتخاب (candidate)
	{
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  ele_گنتی = 2;
	  جاؤ مکمل;
	}
      واپس 0;
    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_سمتیہ:
      توڑو;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  سائز = ty->سائز;
  اگر (سائز < 4 || سائز > 64)
    واپس 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_سائز = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  اگر (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_سمتیہ
      || candidate == FFI_TYPE_COMPLEX)
    {
      برائے (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          اگر (candidate >= 0)
            توڑو;
        }
    }

  اگر (simd_سائز)
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
      size_t reg_سائز = simd_سائز;
      صحیح عدد_registers;
      صحیح پہلا_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      اگر (reg_سائز < 4 || reg_سائز > 16 || سائز % reg_سائز != 0)
	واپس 0;
      عدد_registers = (صحیح) (سائز / reg_سائز);
      اگر (عدد_registers > 4)
	واپس 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      اگر (ty->type != FFI_TYPE_سمتیہ)
	برائے (i = 0; elements[i]; ++i)
	  اگر (is_simd (elements[i]) != reg_سائز)
	    واپس 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      برائے (i = 0; elements[i]; ++i)
	{
	  صحیح t = elements[i]->type;
	  اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_سمتیہ
	      || t == FFI_TYPE_COMPLEX)
	    {
	      اگر (!is_hfa1 (elements[i], candidate))
		واپس 0;
	    }
	  ورنہ اگر (t != candidate)
	    واپس 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      پہلا_level_element_type = FFI_TYPE_FLOAT + intlog2 ((صحیح) reg_سائز) - 2;
      واپس پہلا_level_element_type * 4 + (4 - عدد_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  انتخاب (candidate)
    {
    حالت FFI_TYPE_FLOAT:
      ele_گنتی = سائز / حجم(اعشاری);
      اگر (سائز != ele_گنتی * حجم(اعشاری))
        واپس 0;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      ele_گنتی = سائز / حجم(دوہرا);
      اگر (سائز != ele_گنتی * حجم(دوہرا))
        واپس 0;
      توڑو;
    حالت FFI_TYPE_LONGDOUBLE:
      ele_گنتی = سائز / حجم(طویل دوہرا);
      اگر (سائز != ele_گنتی * حجم(طویل دوہرا))
        واپس 0;
      توڑو;
    طےشدہ:
      واپس 0;
    }
  اگر (ele_گنتی > 4)
    واپس 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  برائے (i = 0; elements[i]; ++i)
    {
      صحیح t = elements[i]->type;
      اگر (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          اگر (!is_hfa1 (elements[i], candidate))
            واپس 0;
        }
      ورنہ اگر (t != candidate)
        واپس 0;
    }

  /* All tests succeeded.  Encode the result.  */
 مکمل:
  واپس candidate * 4 + (4 - (صحیح)ele_گنتی);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ساخت arg_state
{
  غیرعلامتی ngrn;                /* Next general-purpose register number. */
  غیرعلامتی nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t اگلا_struct_area;	/* Place to allocate big structs. */

#اگر defined (__APPLE__)
  غیرعلامتی allocating_variadic;
#اگر_ختم
};

/* Initialize a procedure call argument marshalling state.  */
ساکن خالی
arg_init (ساخت arg_state *state, size_t سائز)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->اگلا_struct_area = سائز;
#اگر defined (__APPLE__)
  state->allocating_variadic = 0;
#اگر_ختم
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
ساکن خالی *
allocate_مقصد_ڈھیر (ساخت arg_state *state, خالی *ڈھیر,
		   size_t alignment, size_t سائز)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#اگر defined (__APPLE__)
  اگر (state->allocating_variadic && alignment < 8)
    alignment = 8;
#ورنہ
  اگر (alignment < 8)
    alignment = 8;
#اگر_ختم

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + سائز;

  واپس (حرف *)ڈھیر + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
ساکن خالی *
allocate_and_copy_struct_مقصد_ڈھیر (ساخت arg_state *state, خالی *ڈھیر,
				   size_t alignment, size_t سائز, خالی *قدر)
{
  size_t dest = state->اگلا_struct_area - سائز;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_نیچے_کو (dest, alignment);
  state->اگلا_struct_area = dest;

  واپس حافظہ_نقل ((حرف *) ڈھیر + dest, قدر, سائز);
}

ساکن ffi_arg
extend_integer_type (خالی *ماخذ, صحیح type)
{
  انتخاب (type)
    {
    حالت FFI_TYPE_UINT8:
      {
        UINT8 u8;
        حافظہ_نقل (&u8, ماخذ, حجم (u8));
        واپس u8;
      }
    حالت FFI_TYPE_SINT8:
      {
        SINT8 s8;
        حافظہ_نقل (&s8, ماخذ, حجم (s8));
        واپس s8;
      }
    حالت FFI_TYPE_UINT16:
      {
        UINT16 u16;
        حافظہ_نقل (&u16, ماخذ, حجم (u16));
        واپس u16;
      }
    حالت FFI_TYPE_SINT16:
      {
        SINT16 s16;
        حافظہ_نقل (&s16, ماخذ, حجم (s16));
        واپس s16;
      }
    حالت FFI_TYPE_UINT32:
      {
        UINT32 u32;
        حافظہ_نقل (&u32, ماخذ, حجم (u32));
        واپس u32;
      }
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      {
        SINT32 s32;
        حافظہ_نقل (&s32, ماخذ, حجم (s32));
        واپس s32;
      }
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      {
        UINT64 u64;
        حافظہ_نقل (&u64, ماخذ, حجم (u64));
        واپس u64;
      }
    حالت FFI_TYPE_اشارہ_گر:
      {
        uintptr_t uptr;
        حافظہ_نقل (&uptr, ماخذ, حجم (uptr));
        واپس uptr;
      }
    طےشدہ:
      abort();
    }
}

#اگر defined(_MSC_VER) && !defined(__clang__)
خالی extend_hfa_type (خالی *dest, خالی *src, صحیح h);
#ورنہ
ساکن خالی
extend_hfa_type (خالی *dest, خالی *src, صحیح h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  خالی *x0;

#تعریف_کرو BTI_J "hint #36"
  __asm__ متلون (
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
#اگر_ختم

#اگر defined(_MSC_VER) && !defined(__clang__)
خالی* compress_hfa_type (خالی *dest, خالی *src, صحیح h);
#ورنہ
ساکن خالی *
compress_hfa_type (خالی *dest, خالی *reg, صحیح h)
{
  انتخاب (h)
    {
    حالت AARCH64_RET_S1:
      اگر (dest == reg)
	واپس (حرف *)dest + BE (12);
      ورنہ
	*(اعشاری *)dest = *(اعشاری *)((حرف *)reg + BE (12));
      توڑو;
    حالت AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      توڑو;
    حالت AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      توڑو;
    حالت AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      توڑو;

    حالت AARCH64_RET_D1:
      اگر (dest == reg)
	واپس (حرف *)dest + BE (8);
      ورنہ
	*(دوہرا *)dest = *(دوہرا *)((حرف *)reg + BE (8));
      توڑو;
    حالت AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      توڑو;
    حالت AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      توڑو;
    حالت AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      توڑو;

    طےشدہ:
      اگر (dest != reg)
	واپس حافظہ_نقل (dest, reg, 16 * (4 - (h & 3)));
      توڑو;
    }
  واپس dest;
}
#اگر_ختم

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

ساکن خالی *
allocate_int_مقصد_reg_or_ڈھیر (ساخت call_context *context,
			      ساخت arg_state *state,
			      خالی *ڈھیر, size_t سائز)
{
  اگر (state->ngrn < N_X_ARG_REG)
    واپس (حرف *)&context->x[state->ngrn++] + BE (حجم (UINT64) - سائز);

  state->ngrn = N_X_ARG_REG;
  واپس (حرف *)allocate_مقصد_ڈھیر (state, ڈھیر, سائز, سائز)
	 + BE (حجم (UINT64) - سائز);
}

ساکن خالی *
allocate_int128_مقصد_reg_or_ڈھیر (ساخت call_context *context,
			         ساخت arg_state *state, خالی *ڈھیر)
{
  غیرعلامتی ngrn = state->ngrn;
  خالی *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#اگر_غیر_تعریف_شدہ __APPLE__
  ngrn += ngrn & 1;
#اگر_ختم

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  اگر (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  ورنہ
    {
      ret = allocate_مقصد_ڈھیر (state, ڈھیر, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  واپس ret;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t بائٹیں = cif->بائٹیں;
  صحیح flags, i, n;

  انتخاب (rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      توڑو;
    حالت FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      توڑو;
    حالت FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      توڑو;
    حالت FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      توڑو;
    حالت FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      توڑو;
    حالت FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      توڑو;
    حالت FFI_TYPE_اشارہ_گر:
      flags = (حجم(خالی *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      توڑو;

    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      توڑو;

    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_LONGDOUBLE:
    حالت FFI_TYPE_STRUCT:
    حالت FFI_TYPE_سمتیہ:
    حالت FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      اگر (flags == 0)
	{
	  size_t s = rtype->سائز;
	  اگر (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_اندر_MEM;
	      بائٹیں += 8;
	    }
	  ورنہ اگر (s == 16)
	    flags = AARCH64_RET_INT128;
	  ورنہ اگر (s == 8)
	    flags = AARCH64_RET_INT64;
	  ورنہ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      توڑو;

    طےشدہ:
      abort();
    }

  برائے (i = 0, n = cif->nargs; i < n; i++)
    اگر (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	توڑو;
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
  برائے (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      اگر (ty->سائز > 16 && !is_vfp_type (ty))
	بائٹیں += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->بائٹیں = (غیرعلامتی) FFI_ALIGN(بائٹیں, 16);
  cif->flags = flags;
#اگر defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#اگر_ختم

  واپس FFI_OK;
}

#اگر defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, غیرعلامتی صحیح nfixedargs,
			 غیرعلامتی صحیح ntotalargs)
{
  ffi_حالت status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  واپس status;
}
#ورنہ
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, غیرعلامتی صحیح nfixedargs, غیرعلامتی صحیح ntotalargs)
{
  ffi_حالت status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  واپس status;
}
#اگر_ختم /* __APPLE__ */

خارجی خالی ffi_call_SYSV (ساخت call_context *context, خالی *چوکھٹا,
			   خالی (*fn)(خالی), خالی *rvalue, صحیح flags,
			   خالی *احاطہ) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *orig_rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  ساخت call_context *context;
  خالی *ڈھیر, *چوکھٹا, *rvalue;
  ساخت arg_state state;
  size_t ڈھیر_بائٹیں, rtype_سائز, rsize;
  صحیح i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_سائز = rtype->سائز;
  ڈھیر_بائٹیں = cif->بائٹیں;

  اگر (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  اگر (flags & AARCH64_RET_اندر_MEM)
    {
      اگر (orig_rvalue == تہی)
	rsize = rtype_سائز;
    }
  ورنہ اگر (orig_rvalue == تہی)
    flags &= AARCH64_FLAG_ARG_V;
  ورنہ اگر (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (حجم(ساخت call_context) + ڈھیر_بائٹیں + 40 + rsize);
  ڈھیر = context + 1;
  چوکھٹا = (خالی*)((uintptr_t)ڈھیر + (uintptr_t)ڈھیر_بائٹیں);
  rvalue = (rsize ? (خالی*)((uintptr_t)چوکھٹا + 40) : orig_rvalue);

  arg_init (&state, ڈھیر_بائٹیں);
  برائے (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->سائز;
      خالی *a = avalue[i];
      صحیح h, t;
      خالی *dest;

      t = ty->type;
      انتخاب (t)
	{
	حالت FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  توڑو;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_اشارہ_گر:
	do_اشارہ_گر:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    اگر (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    ورنہ
	      {
		خالی *d = allocate_مقصد_ڈھیر (&state, ڈھیر, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#اگر_تعریف_شدہ __APPLE__
		حافظہ_نقل(d, a, s);
#ورنہ
		*(ffi_arg *)d = ext;
#اگر_ختم
	      }
	  }
	  توڑو;

	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_SINT128:
	  dest = allocate_int128_مقصد_reg_or_ڈھیر (context, &state, ڈھیر);
	  حافظہ_نقل (dest, a, 16);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_سمتیہ:
	حالت FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    اگر (h)
	      {
              صحیح elems = 4 - (h & 3);
              اگر (cif->abi == FFI_WIN64 && isvariadic)
              {
                اگر (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  توڑو;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_مقصد_ڈھیر(&state, ڈھیر, ty->alignment, s);
              }
              ورنہ
              {
                اگر (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  توڑو;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (حرف *)allocate_مقصد_ڈھیر (&state, ڈھیر, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
              }
	      }
	    ورنہ اگر (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_مقصد_ڈھیر (&state, ڈھیر,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_اشارہ_گر;
		s = حجم (خالی *);
		جاؤ do_اشارہ_گر;
	      }
	    ورنہ
	      {
		size_t n = (s + 7) / 8;
		اگر (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (غیرعلامتی صحیح)n;
		  }
		ورنہ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_مقصد_ڈھیر (&state, ڈھیر, ty->alignment, s);
		  }
		}
	      حافظہ_نقل (dest, a, s);
	    }
	  توڑو;

	طےشدہ:
	  abort();
	}

#اگر defined (__APPLE__)
      اگر (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#اگر_ختم
    }

  ffi_call_SYSV (context, چوکھٹا, fn, rvalue, flags, احاطہ);

  اگر (flags & AARCH64_RET_NEED_COPY)
    حافظہ_نقل (orig_rvalue, rvalue, rtype_سائز);
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

#اگر FFI_CLOSURES

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}
#اگر_ختم /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

خارجی خالی ffi_احاطہ_SYSV (خالی) FFI_HIDDEN;
خارجی خالی ffi_احاطہ_SYSV_V (خالی) FFI_HIDDEN;
#اگر defined(FFI_EXEC_STATIC_TRAMP)
خارجی خالی ffi_احاطہ_SYSV_alt (خالی) FFI_HIDDEN;
خارجی خالی ffi_احاطہ_SYSV_V_alt (خالی) FFI_HIDDEN;
#اگر_ختم

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
                      ffi_cif* cif,
                      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
                      خالی *user_ڈیٹا,
                      خالی *codeloc)
{
  اگر (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    واپس FFI_BAD_ABI;

  خالی (*آغاز)(خالی);

  اگر (cif->flags & AARCH64_FLAG_ARG_V)
    آغاز = ffi_احاطہ_SYSV_V;
  ورنہ
    آغاز = ffi_احاطہ_SYSV;

#اگر FFI_EXEC_TRAMPOLINE_جدول
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_ڈیٹا(codeloc, ptrauth_کلید_function_اشارہ_گر, 0);
#  endif
  خالی **config = (خالی **)((uint8_t *)codeloc - صفحہ_MAX_سائز);
  config[0] = احاطہ;
  config[1] = آغاز;
# endif
#ورنہ
  ساکن ثابت غیرعلامتی حرف trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  حرف *tramp = احاطہ->tramp;

# اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      اگر (آغاز == ffi_احاطہ_SYSV_V)
          آغاز = ffi_احاطہ_SYSV_V_alt;
      ورنہ
          آغاز = ffi_احاطہ_SYSV_alt;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, آغاز, احاطہ);
      جاؤ برآمد;
    }
# endif

  /* Initialize the dynamic trampoline. */
  حافظہ_نقل (tramp, trampoline, حجم(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)آغاز;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_سائز);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  غیرعلامتی حرف *tramp_رمز = tramp;
# ورنہ
  غیرعلامتی حرف *tramp_رمز = ffi_ڈیٹا_مقصد_رمز_اشارہ_گر (tramp);
# endif
  ffi_clear_cache (tramp_رمز, tramp_رمز + FFI_TRAMPOLINE_سائز);
# اگر defined(FFI_EXEC_STATIC_TRAMP)
برآمد:
# endif
#اگر_ختم

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

#اگر_تعریف_شدہ FFI_GO_CLOSURES
خارجی خالی ffi_go_احاطہ_SYSV (خالی) FFI_HIDDEN;
خارجی خالی ffi_go_احاطہ_SYSV_V (خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif* cif,
                     خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*))
{
  خالی (*آغاز)(خالی);

  اگر (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    واپس FFI_BAD_ABI;

  اگر (cif->flags & AARCH64_FLAG_ARG_V)
    آغاز = ffi_go_احاطہ_SYSV_V;
  ورنہ
    آغاز = ffi_go_احاطہ_SYSV;

  احاطہ->tramp = آغاز;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}
#اگر_ختم /* FFI_GO_CLOSURES */

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

صحیح FFI_HIDDEN
ffi_احاطہ_SYSV_inner (ffi_cif *cif,
			خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
			خالی *user_ڈیٹا,
			ساخت call_context *context,
			خالی *ڈھیر, خالی *rvalue, خالی *struct_rvalue)
{
  خالی **avalue = (خالی**) alloca (cif->nargs * حجم (خالی*));
  صحیح i, h, nargs, flags, isvariadic = 0;
  ساخت arg_state state;

  arg_init (&state, cif->بائٹیں);

  flags = cif->flags;
  اگر (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  برائے (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      صحیح t = ty->type;
      size_t n, s = ty->سائز;

      انتخاب (t)
	{
	حالت FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  توڑو;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = allocate_int_مقصد_reg_or_ڈھیر (context, &state, ڈھیر, s);
	  توڑو;

	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_مقصد_reg_or_ڈھیر (context, &state, ڈھیر);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_سمتیہ:
	حالت FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  اگر (h)
	    {
	      n = 4 - (h & 3);
              اگر (cif->abi == FFI_WIN64 && isvariadic)
                {
                  اگر (state.ngrn + n <= N_X_ARG_REG)
                    {
                      خالی *reg = &context->x[state.ngrn];
                      state.ngrn += (غیرعلامتی صحیح)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ورنہ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (حرف *)allocate_مقصد_ڈھیر (&state, ڈھیر,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
              ورنہ
                {
                  اگر (state.nsrn + n <= N_V_ARG_REG)
                    {
                      خالی *reg = &context->v[state.nsrn];
                      state.nsrn += (غیرعلامتی صحیح)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  ورنہ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (حرف *)allocate_مقصد_ڈھیر (&state, ڈھیر,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (حجم (UINT64) - s) : 0);
                    }
                }
            }
          ورنہ اگر (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#اگر_تعریف_شدہ __ILP32__
             UINT64 avalue_tmp;
             حافظہ_نقل (&avalue_tmp,
                  allocate_int_مقصد_reg_or_ڈھیر (context, &state,
                                               ڈھیر, حجم (خالی *)),
                  حجم (UINT64));
             avalue[i] = (خالی *)(UINT32)avalue_tmp;
#ورنہ
              avalue[i] = *(خالی **)
              allocate_int_مقصد_reg_or_ڈھیر (context, &state, ڈھیر,
                                         حجم (خالی *));
#اگر_ختم
            }
          ورنہ
            {
              n = (s + 7) / 8;
              اگر (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (غیرعلامتی صحیح)n;
                }
              ورنہ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_مقصد_ڈھیر(&state, ڈھیر,
                                           ty->alignment, s);
                }
            }
          توڑو;

        طےشدہ:
          abort();
      }

#اگر defined (__APPLE__)
      اگر (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#اگر_ختم
    }

  اگر (flags & AARCH64_RET_اندر_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_ڈیٹا);

  واپس flags;
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;

  *tramp_سائز = AARCH64_TRAMP_سائز;
  *map_سائز = AARCH64_TRAMP_MAP_سائز;
  واپس &trampoline_رمز_جدول;
}
#اگر_ختم

#اگر_ختم /* FFI_CLOSURES */

#اگر_ختم /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

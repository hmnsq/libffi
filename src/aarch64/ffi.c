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

#ఒకవేళ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdint.h>
#అన్తర్భావ <fficonfig.h>
#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ "internal.h"
#నిర్వచితమైతే _WIN32
#నిర్వచించు WIN32_LEAN_AND_సగటు
#అన్తర్భావ <windows.h> /* FlushInstructionCache */
#అయితే_ముగింపు
#అన్తర్భావ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ఒకవేళ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# ఒకవేళ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE బహిర్గతం యొక్క date
# endif
#లేకపోతే
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#అయితే_ముగింపు

సంఘం _d
{
  UINT64 d;
  UINT32 s[2];
};

నిర్మాణం _v
{
  సంఘం _d d[2] __attribute__((aligned(16)));
};

నిర్మాణం call_context
{
  నిర్మాణం _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#ఒకవేళ FFI_EXEC_TRAMPOLINE_పట్టిక

#నిర్వచితమైతే __MACH__
#నిర్వచితమైతే HAVE_ARM64E_PTRAUTH
#అన్తర్భావ <ptrauth.h>
#అయితే_ముగింపు
#అన్తర్భావ <mach/vm_param.h>
#అయితే_ముగింపు

#లేకపోతే

#ఒకవేళ defined (__clang__) && defined (__APPLE__)
బాహ్య శూన్య sys_icache_invalidate (శూన్య *ప్రారంభం, size_t len);
#అయితే_ముగింపు

స్థిర అంతఃవరుస శూన్య
ffi_clear_cache (శూన్య *ప్రారంభం, శూన్య *చివర)
{
#ఒకవేళ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (ప్రారంభం, (అక్షరం *)చివర - (అక్షరం *)ప్రారంభం);
#లేదావేళ defined (__GNUC__)
  __builtin___clear_cache (ప్రారంభం, చివర);
#లేదావేళ defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), ప్రారంభం, (అక్షరం*)చివర - (అక్షరం*)ప్రారంభం);
#లేకపోతే
#దోషం "Missing builtin to flush instruction cache"
#అయితే_ముగింపు
}

#అయితే_ముగింపు

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

స్థిర పూర్ణాంకం
intlog2 (పూర్ణాంకం n)
{
  పూర్ణాంకం level = 0;
  అయితే (n >>= 1)
    ++level;
  తిరిగి level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

స్థిర పూర్ణాంకం
is_hfa0 (స్థిరరాశి ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  పూర్ణాంకం i, ret = -1;

  ఒకవేళ (elements != ఖాళీ)
    కొరకు (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        ఒకవేళ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_సదిశ
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            ఒకవేళ (ret < 0)
              కొనసాగు;
          }
        ఆపు;
      }

  తిరిగి ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

స్థిర size_t
is_simd (స్థిరరాశి ffi_type *ty)
{
  ffi_type **elements;
  పూర్ణాంకం i;

  ఒకవేళ (ty->type == FFI_TYPE_సదిశ)
    తిరిగి ty->కొలత;

  elements = ty->elements;
  ఒకవేళ (elements != ఖాళీ)
    కొరకు (i = 0; elements[i]; ++i)
      {
        పూర్ణాంకం t = elements[i]->type;
        ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_సదిశ)
          తిరిగి is_simd (elements[i]);
      }

  తిరిగి 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

స్థిర పూర్ణాంకం
is_hfa1 (స్థిరరాశి ffi_type *ty, పూర్ణాంకం candidate)
{
  ffi_type **elements = ty->elements;
  పూర్ణాంకం i;

  ఒకవేళ (elements != ఖాళీ)
    కొరకు (i = 0; elements[i]; ++i)
      {
        పూర్ణాంకం t = elements[i]->type;
        ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_సదిశ
	    || t == FFI_TYPE_COMPLEX)
          {
            ఒకవేళ (!is_hfa1 (elements[i], candidate))
              తిరిగి 0;
          }
        లేకపోతే ఒకవేళ (t != candidate)
          తిరిగి 0;
      }

  తిరిగి 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

స్థిర పూర్ణాంకం
is_vfp_type (స్థిరరాశి ffi_type *ty)
{
  ffi_type **elements;
  పూర్ణాంకం candidate, i;
  size_t కొలత, ele_లెక్క, simd_కొలత;

  /* Quickest tests first.  */
  candidate = ty->type;
  ఎంపిక (candidate)
    {
    అప్రమేయ:
      తిరిగి 0;
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_LONGDOUBLE:
      ele_లెక్క = 1;
      వెళ్ళు పూర్తి;
    స్థితి FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      ఎంపిక (candidate)
	{
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	  ele_లెక్క = 2;
	  వెళ్ళు పూర్తి;
	}
      తిరిగి 0;
    స్థితి FFI_TYPE_STRUCT:
    స్థితి FFI_TYPE_సదిశ:
      ఆపు;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  కొలత = ty->కొలత;
  ఒకవేళ (కొలత < 4 || కొలత > 64)
    తిరిగి 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_కొలత = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  ఒకవేళ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_సదిశ
      || candidate == FFI_TYPE_COMPLEX)
    {
      కొరకు (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          ఒకవేళ (candidate >= 0)
            ఆపు;
        }
    }

  ఒకవేళ (simd_కొలత)
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
      size_t reg_కొలత = simd_కొలత;
      పూర్ణాంకం అంకె_registers;
      పూర్ణాంకం మొదటి_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      ఒకవేళ (reg_కొలత < 4 || reg_కొలత > 16 || కొలత % reg_కొలత != 0)
	తిరిగి 0;
      అంకె_registers = (పూర్ణాంకం) (కొలత / reg_కొలత);
      ఒకవేళ (అంకె_registers > 4)
	తిరిగి 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      ఒకవేళ (ty->type != FFI_TYPE_సదిశ)
	కొరకు (i = 0; elements[i]; ++i)
	  ఒకవేళ (is_simd (elements[i]) != reg_కొలత)
	    తిరిగి 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      కొరకు (i = 0; elements[i]; ++i)
	{
	  పూర్ణాంకం t = elements[i]->type;
	  ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_సదిశ
	      || t == FFI_TYPE_COMPLEX)
	    {
	      ఒకవేళ (!is_hfa1 (elements[i], candidate))
		తిరిగి 0;
	    }
	  లేకపోతే ఒకవేళ (t != candidate)
	    తిరిగి 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      మొదటి_level_element_type = FFI_TYPE_FLOAT + intlog2 ((పూర్ణాంకం) reg_కొలత) - 2;
      తిరిగి మొదటి_level_element_type * 4 + (4 - అంకె_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  ఎంపిక (candidate)
    {
    స్థితి FFI_TYPE_FLOAT:
      ele_లెక్క = కొలత / పరిమాణం(దశాంశం);
      ఒకవేళ (కొలత != ele_లెక్క * పరిమాణం(దశాంశం))
        తిరిగి 0;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      ele_లెక్క = కొలత / పరిమాణం(ద్విగుణ);
      ఒకవేళ (కొలత != ele_లెక్క * పరిమాణం(ద్విగుణ))
        తిరిగి 0;
      ఆపు;
    స్థితి FFI_TYPE_LONGDOUBLE:
      ele_లెక్క = కొలత / పరిమాణం(దీర్ఘ ద్విగుణ);
      ఒకవేళ (కొలత != ele_లెక్క * పరిమాణం(దీర్ఘ ద్విగుణ))
        తిరిగి 0;
      ఆపు;
    అప్రమేయ:
      తిరిగి 0;
    }
  ఒకవేళ (ele_లెక్క > 4)
    తిరిగి 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  కొరకు (i = 0; elements[i]; ++i)
    {
      పూర్ణాంకం t = elements[i]->type;
      ఒకవేళ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          ఒకవేళ (!is_hfa1 (elements[i], candidate))
            తిరిగి 0;
        }
      లేకపోతే ఒకవేళ (t != candidate)
        తిరిగి 0;
    }

  /* All tests succeeded.  Encode the result.  */
 పూర్తి:
  తిరిగి candidate * 4 + (4 - (పూర్ణాంకం)ele_లెక్క);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

నిర్మాణం arg_state
{
  గుర్తులేని ngrn;                /* Next general-purpose register number. */
  గుర్తులేని nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t తదుపరి_struct_area;	/* Place to allocate big structs. */

#ఒకవేళ defined (__APPLE__)
  గుర్తులేని allocating_variadic;
#అయితే_ముగింపు
};

/* Initialize a procedure call argument marshalling state.  */
స్థిర శూన్య
arg_init (నిర్మాణం arg_state *state, size_t కొలత)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->తదుపరి_struct_area = కొలత;
#ఒకవేళ defined (__APPLE__)
  state->allocating_variadic = 0;
#అయితే_ముగింపు
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
స్థిర శూన్య *
allocate_గమ్యం_దొంతర (నిర్మాణం arg_state *state, శూన్య *దొంతర,
		   size_t alignment, size_t కొలత)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#ఒకవేళ defined (__APPLE__)
  ఒకవేళ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#లేకపోతే
  ఒకవేళ (alignment < 8)
    alignment = 8;
#అయితే_ముగింపు

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + కొలత;

  తిరిగి (అక్షరం *)దొంతర + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
స్థిర శూన్య *
allocate_and_copy_struct_గమ్యం_దొంతర (నిర్మాణం arg_state *state, శూన్య *దొంతర,
				   size_t alignment, size_t కొలత, శూన్య *విలువ)
{
  size_t dest = state->తదుపరి_struct_area - కొలత;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_కిందికి (dest, alignment);
  state->తదుపరి_struct_area = dest;

  తిరిగి జ్ఞాపకం_నకలు ((అక్షరం *) దొంతర + dest, విలువ, కొలత);
}

స్థిర ffi_arg
extend_integer_type (శూన్య *ఆధారం, పూర్ణాంకం type)
{
  ఎంపిక (type)
    {
    స్థితి FFI_TYPE_UINT8:
      {
        UINT8 u8;
        జ్ఞాపకం_నకలు (&u8, ఆధారం, పరిమాణం (u8));
        తిరిగి u8;
      }
    స్థితి FFI_TYPE_SINT8:
      {
        SINT8 s8;
        జ్ఞాపకం_నకలు (&s8, ఆధారం, పరిమాణం (s8));
        తిరిగి s8;
      }
    స్థితి FFI_TYPE_UINT16:
      {
        UINT16 u16;
        జ్ఞాపకం_నకలు (&u16, ఆధారం, పరిమాణం (u16));
        తిరిగి u16;
      }
    స్థితి FFI_TYPE_SINT16:
      {
        SINT16 s16;
        జ్ఞాపకం_నకలు (&s16, ఆధారం, పరిమాణం (s16));
        తిరిగి s16;
      }
    స్థితి FFI_TYPE_UINT32:
      {
        UINT32 u32;
        జ్ఞాపకం_నకలు (&u32, ఆధారం, పరిమాణం (u32));
        తిరిగి u32;
      }
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      {
        SINT32 s32;
        జ్ఞాపకం_నకలు (&s32, ఆధారం, పరిమాణం (s32));
        తిరిగి s32;
      }
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      {
        UINT64 u64;
        జ్ఞాపకం_నకలు (&u64, ఆధారం, పరిమాణం (u64));
        తిరిగి u64;
      }
    స్థితి FFI_TYPE_సూచిక:
      {
        uintptr_t uptr;
        జ్ఞాపకం_నకలు (&uptr, ఆధారం, పరిమాణం (uptr));
        తిరిగి uptr;
      }
    అప్రమేయ:
      abort();
    }
}

#ఒకవేళ defined(_MSC_VER) && !defined(__clang__)
శూన్య extend_hfa_type (శూన్య *dest, శూన్య *src, పూర్ణాంకం h);
#లేకపోతే
స్థిర శూన్య
extend_hfa_type (శూన్య *dest, శూన్య *src, పూర్ణాంకం h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  శూన్య *x0;

#నిర్వచించు BTI_J "hint #36"
  __asm__ అస్థిర (
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
#అయితే_ముగింపు

#ఒకవేళ defined(_MSC_VER) && !defined(__clang__)
శూన్య* compress_hfa_type (శూన్య *dest, శూన్య *src, పూర్ణాంకం h);
#లేకపోతే
స్థిర శూన్య *
compress_hfa_type (శూన్య *dest, శూన్య *reg, పూర్ణాంకం h)
{
  ఎంపిక (h)
    {
    స్థితి AARCH64_RET_S1:
      ఒకవేళ (dest == reg)
	తిరిగి (అక్షరం *)dest + BE (12);
      లేకపోతే
	*(దశాంశం *)dest = *(దశాంశం *)((అక్షరం *)reg + BE (12));
      ఆపు;
    స్థితి AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ఆపు;
    స్థితి AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ఆపు;
    స్థితి AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ఆపు;

    స్థితి AARCH64_RET_D1:
      ఒకవేళ (dest == reg)
	తిరిగి (అక్షరం *)dest + BE (8);
      లేకపోతే
	*(ద్విగుణ *)dest = *(ద్విగుణ *)((అక్షరం *)reg + BE (8));
      ఆపు;
    స్థితి AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      ఆపు;
    స్థితి AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      ఆపు;
    స్థితి AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      ఆపు;

    అప్రమేయ:
      ఒకవేళ (dest != reg)
	తిరిగి జ్ఞాపకం_నకలు (dest, reg, 16 * (4 - (h & 3)));
      ఆపు;
    }
  తిరిగి dest;
}
#అయితే_ముగింపు

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

స్థిర శూన్య *
allocate_int_గమ్యం_reg_or_దొంతర (నిర్మాణం call_context *context,
			      నిర్మాణం arg_state *state,
			      శూన్య *దొంతర, size_t కొలత)
{
  ఒకవేళ (state->ngrn < N_X_ARG_REG)
    తిరిగి (అక్షరం *)&context->x[state->ngrn++] + BE (పరిమాణం (UINT64) - కొలత);

  state->ngrn = N_X_ARG_REG;
  తిరిగి (అక్షరం *)allocate_గమ్యం_దొంతర (state, దొంతర, కొలత, కొలత)
	 + BE (పరిమాణం (UINT64) - కొలత);
}

స్థిర శూన్య *
allocate_int128_గమ్యం_reg_or_దొంతర (నిర్మాణం call_context *context,
			         నిర్మాణం arg_state *state, శూన్య *దొంతర)
{
  గుర్తులేని ngrn = state->ngrn;
  శూన్య *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#నిర్వచితం_కాకపోతే __APPLE__
  ngrn += ngrn & 1;
#అయితే_ముగింపు

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  ఒకవేళ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  లేకపోతే
    {
      ret = allocate_గమ్యం_దొంతర (state, దొంతర, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  తిరిగి ret;
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t బైట్లు = cif->బైట్లు;
  పూర్ణాంకం flags, i, n;

  ఎంపిక (rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      ఆపు;
    స్థితి FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      ఆపు;
    స్థితి FFI_TYPE_సూచిక:
      flags = (పరిమాణం(శూన్య *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      ఆపు;

    స్థితి FFI_TYPE_UINT128:
    స్థితి FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_LONGDOUBLE:
    స్థితి FFI_TYPE_STRUCT:
    స్థితి FFI_TYPE_సదిశ:
    స్థితి FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      ఒకవేళ (flags == 0)
	{
	  size_t s = rtype->కొలత;
	  ఒకవేళ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_లోపల_MEM;
	      బైట్లు += 8;
	    }
	  లేకపోతే ఒకవేళ (s == 16)
	    flags = AARCH64_RET_INT128;
	  లేకపోతే ఒకవేళ (s == 8)
	    flags = AARCH64_RET_INT64;
	  లేకపోతే
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      ఆపు;

    అప్రమేయ:
      abort();
    }

  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    ఒకవేళ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	ఆపు;
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
  కొరకు (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      ఒకవేళ (ty->కొలత > 16 && !is_vfp_type (ty))
	బైట్లు += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->బైట్లు = (గుర్తులేని) FFI_ALIGN(బైట్లు, 16);
  cif->flags = flags;
#ఒకవేళ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#అయితే_ముగింపు

  తిరిగి FFI_OK;
}

#ఒకవేళ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, గుర్తులేని పూర్ణాంకం nfixedargs,
			 గుర్తులేని పూర్ణాంకం ntotalargs)
{
  ffi_స్థితి status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  తిరిగి status;
}
#లేకపోతే
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, గుర్తులేని పూర్ణాంకం nfixedargs, గుర్తులేని పూర్ణాంకం ntotalargs)
{
  ffi_స్థితి status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  తిరిగి status;
}
#అయితే_ముగింపు /* __APPLE__ */

బాహ్య శూన్య ffi_call_SYSV (నిర్మాణం call_context *context, శూన్య *చట్రం,
			   శూన్య (*fn)(శూన్య), శూన్య *rvalue, పూర్ణాంకం flags,
			   శూన్య *సంవృతం) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *orig_rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  నిర్మాణం call_context *context;
  శూన్య *దొంతర, *చట్రం, *rvalue;
  నిర్మాణం arg_state state;
  size_t దొంతర_బైట్లు, rtype_కొలత, rsize;
  పూర్ణాంకం i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_కొలత = rtype->కొలత;
  దొంతర_బైట్లు = cif->బైట్లు;

  ఒకవేళ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  ఒకవేళ (flags & AARCH64_RET_లోపల_MEM)
    {
      ఒకవేళ (orig_rvalue == ఖాళీ)
	rsize = rtype_కొలత;
    }
  లేకపోతే ఒకవేళ (orig_rvalue == ఖాళీ)
    flags &= AARCH64_FLAG_ARG_V;
  లేకపోతే ఒకవేళ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (పరిమాణం(నిర్మాణం call_context) + దొంతర_బైట్లు + 40 + rsize);
  దొంతర = context + 1;
  చట్రం = (శూన్య*)((uintptr_t)దొంతర + (uintptr_t)దొంతర_బైట్లు);
  rvalue = (rsize ? (శూన్య*)((uintptr_t)చట్రం + 40) : orig_rvalue);

  arg_init (&state, దొంతర_బైట్లు);
  కొరకు (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->కొలత;
      శూన్య *a = avalue[i];
      పూర్ణాంకం h, t;
      శూన్య *dest;

      t = ty->type;
      ఎంపిక (t)
	{
	స్థితి FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ఆపు;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_సూచిక:
	do_సూచిక:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    ఒకవేళ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    లేకపోతే
	      {
		శూన్య *d = allocate_గమ్యం_దొంతర (&state, దొంతర, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#నిర్వచితమైతే __APPLE__
		జ్ఞాపకం_నకలు(d, a, s);
#లేకపోతే
		*(ffi_arg *)d = ext;
#అయితే_ముగింపు
	      }
	  }
	  ఆపు;

	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_SINT128:
	  dest = allocate_int128_గమ్యం_reg_or_దొంతర (context, &state, దొంతర);
	  జ్ఞాపకం_నకలు (dest, a, 16);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_సదిశ:
	స్థితి FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    ఒకవేళ (h)
	      {
              పూర్ణాంకం elems = 4 - (h & 3);
              ఒకవేళ (cif->abi == FFI_WIN64 && isvariadic)
              {
                ఒకవేళ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  ఆపు;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_గమ్యం_దొంతర(&state, దొంతర, ty->alignment, s);
              }
              లేకపోతే
              {
                ఒకవేళ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  ఆపు;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (అక్షరం *)allocate_గమ్యం_దొంతర (&state, దొంతర, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (పరిమాణం (UINT64) - s) : 0);
              }
	      }
	    లేకపోతే ఒకవేళ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_గమ్యం_దొంతర (&state, దొంతర,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_సూచిక;
		s = పరిమాణం (శూన్య *);
		వెళ్ళు do_సూచిక;
	      }
	    లేకపోతే
	      {
		size_t n = (s + 7) / 8;
		ఒకవేళ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (గుర్తులేని పూర్ణాంకం)n;
		  }
		లేకపోతే
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_గమ్యం_దొంతర (&state, దొంతర, ty->alignment, s);
		  }
		}
	      జ్ఞాపకం_నకలు (dest, a, s);
	    }
	  ఆపు;

	అప్రమేయ:
	  abort();
	}

#ఒకవేళ defined (__APPLE__)
      ఒకవేళ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#అయితే_ముగింపు
    }

  ffi_call_SYSV (context, చట్రం, fn, rvalue, flags, సంవృతం);

  ఒకవేళ (flags & AARCH64_RET_NEED_COPY)
    జ్ఞాపకం_నకలు (orig_rvalue, rvalue, rtype_కొలత);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

#ఒకవేళ FFI_CLOSURES

#నిర్వచితమైతే FFI_GO_CLOSURES
శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}
#అయితే_ముగింపు /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

బాహ్య శూన్య ffi_సంవృతం_SYSV (శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_సంవృతం_SYSV_V (శూన్య) FFI_HIDDEN;
#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బాహ్య శూన్య ffi_సంవృతం_SYSV_alt (శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_సంవృతం_SYSV_V_alt (శూన్య) FFI_HIDDEN;
#అయితే_ముగింపు

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
                      ffi_cif* cif,
                      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
                      శూన్య *user_దత్తాంశం,
                      శూన్య *codeloc)
{
  ఒకవేళ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    తిరిగి FFI_BAD_ABI;

  శూన్య (*ప్రారంభం)(శూన్య);

  ఒకవేళ (cif->flags & AARCH64_FLAG_ARG_V)
    ప్రారంభం = ffi_సంవృతం_SYSV_V;
  లేకపోతే
    ప్రారంభం = ffi_సంవృతం_SYSV;

#ఒకవేళ FFI_EXEC_TRAMPOLINE_పట్టిక
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_దత్తాంశం(codeloc, ptrauth_కీలకం_function_సూచిక, 0);
#  endif
  శూన్య **config = (శూన్య **)((uint8_t *)codeloc - పుట_MAX_కొలత);
  config[0] = సంవృతం;
  config[1] = ప్రారంభం;
# endif
#లేకపోతే
  స్థిర స్థిరరాశి గుర్తులేని అక్షరం trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  అక్షరం *tramp = సంవృతం->tramp;

# ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      ఒకవేళ (ప్రారంభం == ffi_సంవృతం_SYSV_V)
          ప్రారంభం = ffi_సంవృతం_SYSV_V_alt;
      లేకపోతే
          ప్రారంభం = ffi_సంవృతం_SYSV_alt;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, ప్రారంభం, సంవృతం);
      వెళ్ళు బహిర్గతం;
    }
# endif

  /* Initialize the dynamic trampoline. */
  జ్ఞాపకం_నకలు (tramp, trampoline, పరిమాణం(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)ప్రారంభం;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_కొలత);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  గుర్తులేని అక్షరం *tramp_సంకేతం = tramp;
# లేకపోతే
  గుర్తులేని అక్షరం *tramp_సంకేతం = ffi_దత్తాంశం_గమ్యం_సంకేతం_సూచిక (tramp);
# endif
  ffi_clear_cache (tramp_సంకేతం, tramp_సంకేతం + FFI_TRAMPOLINE_కొలత);
# ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
బహిర్గతం:
# endif
#అయితే_ముగింపు

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

#నిర్వచితమైతే FFI_GO_CLOSURES
బాహ్య శూన్య ffi_go_సంవృతం_SYSV (శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_go_సంవృతం_SYSV_V (శూన్య) FFI_HIDDEN;

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం, ffi_cif* cif,
                     శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*))
{
  శూన్య (*ప్రారంభం)(శూన్య);

  ఒకవేళ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    తిరిగి FFI_BAD_ABI;

  ఒకవేళ (cif->flags & AARCH64_FLAG_ARG_V)
    ప్రారంభం = ffi_go_సంవృతం_SYSV_V;
  లేకపోతే
    ప్రారంభం = ffi_go_సంవృతం_SYSV;

  సంవృతం->tramp = ప్రారంభం;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}
#అయితే_ముగింపు /* FFI_GO_CLOSURES */

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

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_SYSV_inner (ffi_cif *cif,
			శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
			శూన్య *user_దత్తాంశం,
			నిర్మాణం call_context *context,
			శూన్య *దొంతర, శూన్య *rvalue, శూన్య *struct_rvalue)
{
  శూన్య **avalue = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య*));
  పూర్ణాంకం i, h, nargs, flags, isvariadic = 0;
  నిర్మాణం arg_state state;

  arg_init (&state, cif->బైట్లు);

  flags = cif->flags;
  ఒకవేళ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  కొరకు (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      పూర్ణాంకం t = ty->type;
      size_t n, s = ty->కొలత;

      ఎంపిక (t)
	{
	స్థితి FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  ఆపు;

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = allocate_int_గమ్యం_reg_or_దొంతర (context, &state, దొంతర, s);
	  ఆపు;

	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_గమ్యం_reg_or_దొంతర (context, &state, దొంతర);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_సదిశ:
	స్థితి FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  ఒకవేళ (h)
	    {
	      n = 4 - (h & 3);
              ఒకవేళ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  ఒకవేళ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      శూన్య *reg = &context->x[state.ngrn];
                      state.ngrn += (గుర్తులేని పూర్ణాంకం)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  లేకపోతే
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (అక్షరం *)allocate_గమ్యం_దొంతర (&state, దొంతర,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (పరిమాణం (UINT64) - s) : 0);
                    }
                }
              లేకపోతే
                {
                  ఒకవేళ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      శూన్య *reg = &context->v[state.nsrn];
                      state.nsrn += (గుర్తులేని పూర్ణాంకం)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  లేకపోతే
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (అక్షరం *)allocate_గమ్యం_దొంతర (&state, దొంతర,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (పరిమాణం (UINT64) - s) : 0);
                    }
                }
            }
          లేకపోతే ఒకవేళ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#నిర్వచితమైతే __ILP32__
             UINT64 avalue_tmp;
             జ్ఞాపకం_నకలు (&avalue_tmp,
                  allocate_int_గమ్యం_reg_or_దొంతర (context, &state,
                                               దొంతర, పరిమాణం (శూన్య *)),
                  పరిమాణం (UINT64));
             avalue[i] = (శూన్య *)(UINT32)avalue_tmp;
#లేకపోతే
              avalue[i] = *(శూన్య **)
              allocate_int_గమ్యం_reg_or_దొంతర (context, &state, దొంతర,
                                         పరిమాణం (శూన్య *));
#అయితే_ముగింపు
            }
          లేకపోతే
            {
              n = (s + 7) / 8;
              ఒకవేళ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (గుర్తులేని పూర్ణాంకం)n;
                }
              లేకపోతే
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_గమ్యం_దొంతర(&state, దొంతర,
                                           ty->alignment, s);
                }
            }
          ఆపు;

        అప్రమేయ:
          abort();
      }

#ఒకవేళ defined (__APPLE__)
      ఒకవేళ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#అయితే_ముగింపు
    }

  ఒకవేళ (flags & AARCH64_RET_లోపల_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_దత్తాంశం);

  తిరిగి flags;
}

#ఒకవేళ defined(FFI_EXEC_STATIC_TRAMP)
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;

  *tramp_కొలత = AARCH64_TRAMP_కొలత;
  *map_కొలత = AARCH64_TRAMP_MAP_కొలత;
  తిరిగి &trampoline_సంకేతం_పట్టిక;
}
#అయితే_ముగింపు

#అయితే_ముగింపు /* FFI_CLOSURES */

#అయితే_ముగింపు /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

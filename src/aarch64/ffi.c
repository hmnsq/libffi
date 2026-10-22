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

#जर defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdint.h>
#अन्तर्भाव <fficonfig.h>
#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#अन्तर्भाव <windows.h> /* FlushInstructionCache */
#endif
#अन्तर्भाव <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#जर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# जर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

संघ _d
{
  UINT64 d;
  UINT32 s[2];
};

संरचना _v
{
  संघ _d d[2] __attribute__((aligned(16)));
};

संरचना call_context
{
  संरचना _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#जर FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#अन्तर्भाव <ptrauth.h>
#endif
#अन्तर्भाव <mach/vm_param.h>
#endif

#अन्यथा

#जर defined (__clang__) && defined (__APPLE__)
बाह्य शून्य sys_icache_invalidate (शून्य *start, माप_प्रकार len);
#endif

स्थिर ओळीत शून्य
ffi_clear_cache (शून्य *start, शून्य *end)
{
#जर defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (अक्षर *)end - (अक्षर *)start);
#किंवाजर defined (__GNUC__)
  __builtin___clear_cache (start, end);
#किंवाजर defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (अक्षर*)end - (अक्षर*)start);
#अन्यथा
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

स्थिर पूर्णांक
intlog2 (पूर्णांक n)
{
  पूर्णांक level = 0;
  पर्यंत (n >>= 1)
    ++level;
  परत level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

स्थिर पूर्णांक
is_hfa0 (स्थिरांक ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i, ret = -1;

  जर (elements != रिक्त)
    साठी (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        जर (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            जर (ret < 0)
              सुरूठेवा;
          }
        थांबा;
      }

  परत ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

स्थिर माप_प्रकार
is_simd (स्थिरांक ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक i;

  जर (ty->type == FFI_TYPE_VECTOR)
    परत ty->size;

  elements = ty->elements;
  जर (elements != रिक्त)
    साठी (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          परत is_simd (elements[i]);
      }

  परत 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

स्थिर पूर्णांक
is_hfa1 (स्थिरांक ffi_type *ty, पूर्णांक candidate)
{
  ffi_type **elements = ty->elements;
  पूर्णांक i;

  जर (elements != रिक्त)
    साठी (i = 0; elements[i]; ++i)
      {
        पूर्णांक t = elements[i]->type;
        जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            जर (!is_hfa1 (elements[i], candidate))
              परत 0;
          }
        अन्यथा जर (t != candidate)
          परत 0;
      }

  परत 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

स्थिर पूर्णांक
is_vfp_type (स्थिरांक ffi_type *ty)
{
  ffi_type **elements;
  पूर्णांक candidate, i;
  माप_प्रकार size, ele_मोजणी, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  निवड (candidate)
    {
    पूर्वनिर्धारित:
      परत 0;
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_LONGDOUBLE:
      ele_मोजणी = 1;
      जा done;
    स्थिती FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      निवड (candidate)
	{
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_LONGDOUBLE:
	  ele_मोजणी = 2;
	  जा done;
	}
      परत 0;
    स्थिती FFI_TYPE_STRUCT:
    स्थिती FFI_TYPE_VECTOR:
      थांबा;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  जर (size < 4 || size > 64)
    परत 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  जर (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      साठी (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          जर (candidate >= 0)
            थांबा;
        }
    }

  जर (simd_size)
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
      माप_प्रकार reg_size = simd_size;
      पूर्णांक अंक_registers;
      पूर्णांक first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      जर (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	परत 0;
      अंक_registers = (पूर्णांक) (size / reg_size);
      जर (अंक_registers > 4)
	परत 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      जर (ty->type != FFI_TYPE_VECTOR)
	साठी (i = 0; elements[i]; ++i)
	  जर (is_simd (elements[i]) != reg_size)
	    परत 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      साठी (i = 0; elements[i]; ++i)
	{
	  पूर्णांक t = elements[i]->type;
	  जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      जर (!is_hfa1 (elements[i], candidate))
		परत 0;
	    }
	  अन्यथा जर (t != candidate)
	    परत 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((पूर्णांक) reg_size) - 2;
      परत first_level_element_type * 4 + (4 - अंक_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  निवड (candidate)
    {
    स्थिती FFI_TYPE_FLOAT:
      ele_मोजणी = size / आकार(दशांश);
      जर (size != ele_मोजणी * आकार(दशांश))
        परत 0;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      ele_मोजणी = size / आकार(द्विगुण);
      जर (size != ele_मोजणी * आकार(द्विगुण))
        परत 0;
      थांबा;
    स्थिती FFI_TYPE_LONGDOUBLE:
      ele_मोजणी = size / आकार(दीर्घ द्विगुण);
      जर (size != ele_मोजणी * आकार(दीर्घ द्विगुण))
        परत 0;
      थांबा;
    पूर्वनिर्धारित:
      परत 0;
    }
  जर (ele_मोजणी > 4)
    परत 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  साठी (i = 0; elements[i]; ++i)
    {
      पूर्णांक t = elements[i]->type;
      जर (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          जर (!is_hfa1 (elements[i], candidate))
            परत 0;
        }
      अन्यथा जर (t != candidate)
        परत 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  परत candidate * 4 + (4 - (पूर्णांक)ele_मोजणी);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

संरचना arg_state
{
  अचिन्हित ngrn;                /* Next general-purpose register number. */
  अचिन्हित nsrn;                /* Next vector register number. */
  माप_प्रकार nsaa;                  /* Next stack offset. */
  माप_प्रकार next_struct_area;	/* Place to allocate big structs. */

#जर defined (__APPLE__)
  अचिन्हित allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
स्थिर शून्य
arg_init (संरचना arg_state *state, माप_प्रकार size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#जर defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
स्थिर शून्य *
allocate_to_चळत (संरचना arg_state *state, शून्य *चळत,
		   माप_प्रकार alignment, माप_प्रकार size)
{
  माप_प्रकार nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#जर defined (__APPLE__)
  जर (state->allocating_variadic && alignment < 8)
    alignment = 8;
#अन्यथा
  जर (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  परत (अक्षर *)चळत + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
स्थिर शून्य *
allocate_and_copy_struct_to_चळत (संरचना arg_state *state, शून्य *चळत,
				   माप_प्रकार alignment, माप_प्रकार size, शून्य *मूल्य)
{
  माप_प्रकार dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  परत स_नकल ((अक्षर *) चळत + dest, मूल्य, size);
}

स्थिर ffi_arg
extend_integer_type (शून्य *source, पूर्णांक type)
{
  निवड (type)
    {
    स्थिती FFI_TYPE_UINT8:
      {
        UINT8 u8;
        स_नकल (&u8, source, आकार (u8));
        परत u8;
      }
    स्थिती FFI_TYPE_SINT8:
      {
        SINT8 s8;
        स_नकल (&s8, source, आकार (s8));
        परत s8;
      }
    स्थिती FFI_TYPE_UINT16:
      {
        UINT16 u16;
        स_नकल (&u16, source, आकार (u16));
        परत u16;
      }
    स्थिती FFI_TYPE_SINT16:
      {
        SINT16 s16;
        स_नकल (&s16, source, आकार (s16));
        परत s16;
      }
    स्थिती FFI_TYPE_UINT32:
      {
        UINT32 u32;
        स_नकल (&u32, source, आकार (u32));
        परत u32;
      }
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      {
        SINT32 s32;
        स_नकल (&s32, source, आकार (s32));
        परत s32;
      }
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      {
        UINT64 u64;
        स_नकल (&u64, source, आकार (u64));
        परत u64;
      }
    स्थिती FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        स_नकल (&uptr, source, आकार (uptr));
        परत uptr;
      }
    पूर्वनिर्धारित:
      पात();
    }
}

#जर defined(_MSC_VER) && !defined(__clang__)
शून्य extend_hfa_type (शून्य *dest, शून्य *src, पूर्णांक h);
#अन्यथा
स्थिर शून्य
extend_hfa_type (शून्य *dest, शून्य *src, पूर्णांक h)
{
  सूचक_भेद_प्रकार f = h - AARCH64_RET_S4;
  शून्य *x0;

#define BTI_J "hint #36"
  __asm__ अस्थिर (
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

#जर defined(_MSC_VER) && !defined(__clang__)
शून्य* compress_hfa_type (शून्य *dest, शून्य *src, पूर्णांक h);
#अन्यथा
स्थिर शून्य *
compress_hfa_type (शून्य *dest, शून्य *reg, पूर्णांक h)
{
  निवड (h)
    {
    स्थिती AARCH64_RET_S1:
      जर (dest == reg)
	परत (अक्षर *)dest + BE (12);
      अन्यथा
	*(दशांश *)dest = *(दशांश *)((अक्षर *)reg + BE (12));
      थांबा;
    स्थिती AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      थांबा;
    स्थिती AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      थांबा;
    स्थिती AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      थांबा;

    स्थिती AARCH64_RET_D1:
      जर (dest == reg)
	परत (अक्षर *)dest + BE (8);
      अन्यथा
	*(द्विगुण *)dest = *(द्विगुण *)((अक्षर *)reg + BE (8));
      थांबा;
    स्थिती AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      थांबा;
    स्थिती AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      थांबा;
    स्थिती AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      थांबा;

    पूर्वनिर्धारित:
      जर (dest != reg)
	परत स_नकल (dest, reg, 16 * (4 - (h & 3)));
      थांबा;
    }
  परत dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

स्थिर शून्य *
allocate_int_to_reg_or_चळत (संरचना call_context *context,
			      संरचना arg_state *state,
			      शून्य *चळत, माप_प्रकार size)
{
  जर (state->ngrn < N_X_ARG_REG)
    परत (अक्षर *)&context->x[state->ngrn++] + BE (आकार (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  परत (अक्षर *)allocate_to_चळत (state, चळत, size, size)
	 + BE (आकार (UINT64) - size);
}

स्थिर शून्य *
allocate_int128_to_reg_or_चळत (संरचना call_context *context,
			         संरचना arg_state *state, शून्य *चळत)
{
  अचिन्हित ngrn = state->ngrn;
  शून्य *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  जर (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  अन्यथा
    {
      ret = allocate_to_चळत (state, चळत, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  परत ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  माप_प्रकार bytes = cif->bytes;
  पूर्णांक flags, i, n;

  निवड (rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      थांबा;
    स्थिती FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      थांबा;
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      थांबा;
    स्थिती FFI_TYPE_POINTER:
      flags = (आकार(शून्य *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      थांबा;

    स्थिती FFI_TYPE_UINT128:
    स्थिती FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_LONGDOUBLE:
    स्थिती FFI_TYPE_STRUCT:
    स्थिती FFI_TYPE_VECTOR:
    स्थिती FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      जर (flags == 0)
	{
	  माप_प्रकार s = rtype->size;
	  जर (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  अन्यथा जर (s == 16)
	    flags = AARCH64_RET_INT128;
	  अन्यथा जर (s == 8)
	    flags = AARCH64_RET_INT64;
	  अन्यथा
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      थांबा;

    पूर्वनिर्धारित:
      पात();
    }

  साठी (i = 0, n = cif->nargs; i < n; i++)
    जर (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	थांबा;
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
  साठी (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      जर (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (अचिन्हित) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#जर defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  परत FFI_OK;
}

#जर defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs,
			 अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  परत status;
}
#अन्यथा
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  परत status;
}
#endif /* __APPLE__ */

बाह्य शून्य ffi_call_SYSV (संरचना call_context *context, शून्य *frame,
			   शून्य (*fn)(शून्य), शून्य *rvalue, पूर्णांक flags,
			   शून्य *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *orig_rvalue,
	      शून्य **avalue, शून्य *closure)
{
  संरचना call_context *context;
  शून्य *चळत, *frame, *rvalue;
  संरचना arg_state state;
  माप_प्रकार चळत_bytes, rtype_size, rsize;
  पूर्णांक i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  चळत_bytes = cif->bytes;

  जर (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  जर (flags & AARCH64_RET_IN_MEM)
    {
      जर (orig_rvalue == रिक्त)
	rsize = rtype_size;
    }
  अन्यथा जर (orig_rvalue == रिक्त)
    flags &= AARCH64_FLAG_ARG_V;
  अन्यथा जर (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (आकार(संरचना call_context) + चळत_bytes + 40 + rsize);
  चळत = context + 1;
  frame = (शून्य*)((uintptr_t)चळत + (uintptr_t)चळत_bytes);
  rvalue = (rsize ? (शून्य*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, चळत_bytes);
  साठी (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      माप_प्रकार s = ty->size;
      शून्य *a = avalue[i];
      पूर्णांक h, t;
      शून्य *dest;

      t = ty->type;
      निवड (t)
	{
	स्थिती FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  थांबा;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    जर (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    अन्यथा
	      {
		शून्य *d = allocate_to_चळत (&state, चळत, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		स_नकल(d, a, s);
#अन्यथा
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  थांबा;

	स्थिती FFI_TYPE_UINT128:
	स्थिती FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_चळत (context, &state, चळत);
	  स_नकल (dest, a, 16);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_LONGDOUBLE:
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_VECTOR:
	स्थिती FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    जर (h)
	      {
              पूर्णांक elems = 4 - (h & 3);
              जर (cif->abi == FFI_WIN64 && isvariadic)
              {
                जर (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  थांबा;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_चळत(&state, चळत, ty->alignment, s);
              }
              अन्यथा
              {
                जर (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  थांबा;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (अक्षर *)allocate_to_चळत (&state, चळत, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (आकार (UINT64) - s) : 0);
              }
	      }
	    अन्यथा जर (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_चळत (&state, चळत,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = आकार (शून्य *);
		जा do_pointer;
	      }
	    अन्यथा
	      {
		माप_प्रकार n = (s + 7) / 8;
		जर (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (अचिन्हित पूर्णांक)n;
		  }
		अन्यथा
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_चळत (&state, चळत, ty->alignment, s);
		  }
		}
	      स_नकल (dest, a, s);
	    }
	  थांबा;

	पूर्वनिर्धारित:
	  पात();
	}

#जर defined (__APPLE__)
      जर (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  जर (flags & AARCH64_RET_NEED_COPY)
    स_नकल (orig_rvalue, rvalue, rtype_size);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

#जर FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

बाह्य शून्य ffi_closure_SYSV (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_SYSV_V (शून्य) FFI_HIDDEN;
#जर defined(FFI_EXEC_STATIC_TRAMP)
बाह्य शून्य ffi_closure_SYSV_alt (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_closure_SYSV_V_alt (शून्य) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  जर (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    परत FFI_BAD_ABI;

  शून्य (*start)(शून्य);

  जर (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  अन्यथा
    start = ffi_closure_SYSV;

#जर FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_किल्ली_function_pointer, 0);
#  endif
  शून्य **config = (शून्य **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#अन्यथा
  स्थिर स्थिरांक अचिन्हित अक्षर trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  अक्षर *tramp = closure->tramp;

# जर defined(FFI_EXEC_STATIC_TRAMP)
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      जर (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      अन्यथा
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      जा out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  स_नकल (tramp, trampoline, आकार(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  अचिन्हित अक्षर *tramp_code = tramp;
# अन्यथा
  अचिन्हित अक्षर *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# जर defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

#ifdef FFI_GO_CLOSURES
बाह्य शून्य ffi_go_closure_SYSV (शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_SYSV_V (शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य (*start)(शून्य);

  जर (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    परत FFI_BAD_ABI;

  जर (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  अन्यथा
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
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

पूर्णांक FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
			शून्य *user_data,
			संरचना call_context *context,
			शून्य *चळत, शून्य *rvalue, शून्य *struct_rvalue)
{
  शून्य **avalue = (शून्य**) alloca (cif->nargs * आकार (शून्य*));
  पूर्णांक i, h, nargs, flags, isvariadic = 0;
  संरचना arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  जर (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  साठी (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      पूर्णांक t = ty->type;
      माप_प्रकार n, s = ty->size;

      निवड (t)
	{
	स्थिती FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  थांबा;

	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_चळत (context, &state, चळत, s);
	  थांबा;

	स्थिती FFI_TYPE_UINT128:
	स्थिती FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_चळत (context, &state, चळत);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_LONGDOUBLE:
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_VECTOR:
	स्थिती FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  जर (h)
	    {
	      n = 4 - (h & 3);
              जर (cif->abi == FFI_WIN64 && isvariadic)
                {
                  जर (state.ngrn + n <= N_X_ARG_REG)
                    {
                      शून्य *reg = &context->x[state.ngrn];
                      state.ngrn += (अचिन्हित पूर्णांक)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अन्यथा
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (अक्षर *)allocate_to_चळत (&state, चळत,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (आकार (UINT64) - s) : 0);
                    }
                }
              अन्यथा
                {
                  जर (state.nsrn + n <= N_V_ARG_REG)
                    {
                      शून्य *reg = &context->v[state.nsrn];
                      state.nsrn += (अचिन्हित पूर्णांक)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  अन्यथा
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (अक्षर *)allocate_to_चळत (&state, चळत,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (आकार (UINT64) - s) : 0);
                    }
                }
            }
          अन्यथा जर (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             स_नकल (&avalue_tmp,
                  allocate_int_to_reg_or_चळत (context, &state,
                                               चळत, आकार (शून्य *)),
                  आकार (UINT64));
             avalue[i] = (शून्य *)(UINT32)avalue_tmp;
#अन्यथा
              avalue[i] = *(शून्य **)
              allocate_int_to_reg_or_चळत (context, &state, चळत,
                                         आकार (शून्य *));
#endif
            }
          अन्यथा
            {
              n = (s + 7) / 8;
              जर (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (अचिन्हित पूर्णांक)n;
                }
              अन्यथा
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_चळत(&state, चळत,
                                           ty->alignment, s);
                }
            }
          थांबा;

        पूर्वनिर्धारित:
          पात();
      }

#जर defined (__APPLE__)
      जर (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  जर (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  परत flags;
}

#जर defined(FFI_EXEC_STATIC_TRAMP)
शून्य *
ffi_tramp_arch (माप_प्रकार *tramp_size, माप_प्रकार *map_size)
{
  बाह्य शून्य *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  परत &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

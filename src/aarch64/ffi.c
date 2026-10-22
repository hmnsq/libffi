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

#എങ്കിൽ defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <fficonfig.h>
#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#അന്തര്ഭാവ <windows.h> /* FlushInstructionCache */
#endif
#അന്തര്ഭാവ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#എങ്കിൽ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# എങ്കിൽ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#അല്ലെങ്കിൽ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

സംഘം _d
{
  UINT64 d;
  UINT32 s[2];
};

ഘടന _v
{
  സംഘം _d d[2] __attribute__((aligned(16)));
};

ഘടന call_context
{
  ഘടന _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#എങ്കിൽ FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#അന്തര്ഭാവ <ptrauth.h>
#endif
#അന്തര്ഭാവ <mach/vm_param.h>
#endif

#അല്ലെങ്കിൽ

#എങ്കിൽ defined (__clang__) && defined (__APPLE__)
ബാഹ്യ ശൂന്യം sys_icache_invalidate (ശൂന്യം *start, size_t len);
#endif

സ്ഥിതം വരിയിൽ ശൂന്യം
ffi_clear_cache (ശൂന്യം *start, ശൂന്യം *end)
{
#എങ്കിൽ defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (അക്ഷരം *)end - (അക്ഷരം *)start);
#അല്ലെങ്കിലെങ്കിൽ defined (__GNUC__)
  __builtin___clear_cache (start, end);
#അല്ലെങ്കിലെങ്കിൽ defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (അക്ഷരം*)end - (അക്ഷരം*)start);
#അല്ലെങ്കിൽ
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
intlog2 (പൂർണ്ണസംഖ്യ n)
{
  പൂർണ്ണസംഖ്യ level = 0;
  ആകുമ്പോൾ (n >>= 1)
    ++level;
  തിരികെ level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
is_hfa0 (സ്ഥിരം ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  പൂർണ്ണസംഖ്യ i, ret = -1;

  എങ്കിൽ (elements != ശൂന്യ)
    വേണ്ടി (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        എങ്കിൽ (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            എങ്കിൽ (ret < 0)
              തുടരുക;
          }
        നിർത്തുക;
      }

  തിരികെ ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

സ്ഥിതം size_t
is_simd (സ്ഥിരം ffi_type *ty)
{
  ffi_type **elements;
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (ty->type == FFI_TYPE_VECTOR)
    തിരികെ ty->size;

  elements = ty->elements;
  എങ്കിൽ (elements != ശൂന്യ)
    വേണ്ടി (i = 0; elements[i]; ++i)
      {
        പൂർണ്ണസംഖ്യ t = elements[i]->type;
        എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          തിരികെ is_simd (elements[i]);
      }

  തിരികെ 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
is_hfa1 (സ്ഥിരം ffi_type *ty, പൂർണ്ണസംഖ്യ candidate)
{
  ffi_type **elements = ty->elements;
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (elements != ശൂന്യ)
    വേണ്ടി (i = 0; elements[i]; ++i)
      {
        പൂർണ്ണസംഖ്യ t = elements[i]->type;
        എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            എങ്കിൽ (!is_hfa1 (elements[i], candidate))
              തിരികെ 0;
          }
        അല്ലെങ്കിൽ എങ്കിൽ (t != candidate)
          തിരികെ 0;
      }

  തിരികെ 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
is_vfp_type (സ്ഥിരം ffi_type *ty)
{
  ffi_type **elements;
  പൂർണ്ണസംഖ്യ candidate, i;
  size_t size, ele_എണ്ണം, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  തിരഞ്ഞെടുപ്പ് (candidate)
    {
    സ്ഥിരസ്ഥിതി:
      തിരികെ 0;
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      ele_എണ്ണം = 1;
      പോകുക done;
    അവസ്ഥ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      തിരഞ്ഞെടുപ്പ് (candidate)
	{
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  ele_എണ്ണം = 2;
	  പോകുക done;
	}
      തിരികെ 0;
    അവസ്ഥ FFI_TYPE_STRUCT:
    അവസ്ഥ FFI_TYPE_VECTOR:
      നിർത്തുക;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  എങ്കിൽ (size < 4 || size > 64)
    തിരികെ 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  എങ്കിൽ (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      വേണ്ടി (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          എങ്കിൽ (candidate >= 0)
            നിർത്തുക;
        }
    }

  എങ്കിൽ (simd_size)
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
      പൂർണ്ണസംഖ്യ അക്കം_registers;
      പൂർണ്ണസംഖ്യ first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      എങ്കിൽ (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	തിരികെ 0;
      അക്കം_registers = (പൂർണ്ണസംഖ്യ) (size / reg_size);
      എങ്കിൽ (അക്കം_registers > 4)
	തിരികെ 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      എങ്കിൽ (ty->type != FFI_TYPE_VECTOR)
	വേണ്ടി (i = 0; elements[i]; ++i)
	  എങ്കിൽ (is_simd (elements[i]) != reg_size)
	    തിരികെ 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      വേണ്ടി (i = 0; elements[i]; ++i)
	{
	  പൂർണ്ണസംഖ്യ t = elements[i]->type;
	  എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      എങ്കിൽ (!is_hfa1 (elements[i], candidate))
		തിരികെ 0;
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (t != candidate)
	    തിരികെ 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((പൂർണ്ണസംഖ്യ) reg_size) - 2;
      തിരികെ first_level_element_type * 4 + (4 - അക്കം_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  തിരഞ്ഞെടുപ്പ് (candidate)
    {
    അവസ്ഥ FFI_TYPE_FLOAT:
      ele_എണ്ണം = size / വലുപ്പം(ദശാംശം);
      എങ്കിൽ (size != ele_എണ്ണം * വലുപ്പം(ദശാംശം))
        തിരികെ 0;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      ele_എണ്ണം = size / വലുപ്പം(ഇരട്ട);
      എങ്കിൽ (size != ele_എണ്ണം * വലുപ്പം(ഇരട്ട))
        തിരികെ 0;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      ele_എണ്ണം = size / വലുപ്പം(long ഇരട്ട);
      എങ്കിൽ (size != ele_എണ്ണം * വലുപ്പം(long ഇരട്ട))
        തിരികെ 0;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ 0;
    }
  എങ്കിൽ (ele_എണ്ണം > 4)
    തിരികെ 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  വേണ്ടി (i = 0; elements[i]; ++i)
    {
      പൂർണ്ണസംഖ്യ t = elements[i]->type;
      എങ്കിൽ (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          എങ്കിൽ (!is_hfa1 (elements[i], candidate))
            തിരികെ 0;
        }
      അല്ലെങ്കിൽ എങ്കിൽ (t != candidate)
        തിരികെ 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  തിരികെ candidate * 4 + (4 - (പൂർണ്ണസംഖ്യ)ele_എണ്ണം);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

ഘടന arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#എങ്കിൽ defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
സ്ഥിതം ശൂന്യം
arg_init (ഘടന arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#എങ്കിൽ defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
സ്ഥിതം ശൂന്യം *
allocate_to_അട്ടി (ഘടന arg_state *state, ശൂന്യം *അട്ടി,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#എങ്കിൽ defined (__APPLE__)
  എങ്കിൽ (state->allocating_variadic && alignment < 8)
    alignment = 8;
#അല്ലെങ്കിൽ
  എങ്കിൽ (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  തിരികെ (അക്ഷരം *)അട്ടി + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
സ്ഥിതം ശൂന്യം *
allocate_and_copy_struct_to_അട്ടി (ഘടന arg_state *state, ശൂന്യം *അട്ടി,
				   size_t alignment, size_t size, ശൂന്യം *മൂല്യം)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  തിരികെ memcpy ((അക്ഷരം *) അട്ടി + dest, മൂല്യം, size);
}

സ്ഥിതം ffi_arg
extend_integer_type (ശൂന്യം *source, പൂർണ്ണസംഖ്യ type)
{
  തിരഞ്ഞെടുപ്പ് (type)
    {
    അവസ്ഥ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, വലുപ്പം (u8));
        തിരികെ u8;
      }
    അവസ്ഥ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, വലുപ്പം (s8));
        തിരികെ s8;
      }
    അവസ്ഥ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, വലുപ്പം (u16));
        തിരികെ u16;
      }
    അവസ്ഥ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, വലുപ്പം (s16));
        തിരികെ s16;
      }
    അവസ്ഥ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, വലുപ്പം (u32));
        തിരികെ u32;
      }
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, വലുപ്പം (s32));
        തിരികെ s32;
      }
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, വലുപ്പം (u64));
        തിരികെ u64;
      }
    അവസ്ഥ FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, വലുപ്പം (uptr));
        തിരികെ uptr;
      }
    സ്ഥിരസ്ഥിതി:
      abort();
    }
}

#എങ്കിൽ defined(_MSC_VER) && !defined(__clang__)
ശൂന്യം extend_hfa_type (ശൂന്യം *dest, ശൂന്യം *src, പൂർണ്ണസംഖ്യ h);
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം
extend_hfa_type (ശൂന്യം *dest, ശൂന്യം *src, പൂർണ്ണസംഖ്യ h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  ശൂന്യം *x0;

#define BTI_J "hint #36"
  __asm__ അസ്ഥിര (
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

#എങ്കിൽ defined(_MSC_VER) && !defined(__clang__)
ശൂന്യം* compress_hfa_type (ശൂന്യം *dest, ശൂന്യം *src, പൂർണ്ണസംഖ്യ h);
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം *
compress_hfa_type (ശൂന്യം *dest, ശൂന്യം *reg, പൂർണ്ണസംഖ്യ h)
{
  തിരഞ്ഞെടുപ്പ് (h)
    {
    അവസ്ഥ AARCH64_RET_S1:
      എങ്കിൽ (dest == reg)
	തിരികെ (അക്ഷരം *)dest + BE (12);
      അല്ലെങ്കിൽ
	*(ദശാംശം *)dest = *(ദശാംശം *)((അക്ഷരം *)reg + BE (12));
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      നിർത്തുക;

    അവസ്ഥ AARCH64_RET_D1:
      എങ്കിൽ (dest == reg)
	തിരികെ (അക്ഷരം *)dest + BE (8);
      അല്ലെങ്കിൽ
	*(ഇരട്ട *)dest = *(ഇരട്ട *)((അക്ഷരം *)reg + BE (8));
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      നിർത്തുക;
    അവസ്ഥ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      എങ്കിൽ (dest != reg)
	തിരികെ memcpy (dest, reg, 16 * (4 - (h & 3)));
      നിർത്തുക;
    }
  തിരികെ dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

സ്ഥിതം ശൂന്യം *
allocate_int_to_reg_or_അട്ടി (ഘടന call_context *context,
			      ഘടന arg_state *state,
			      ശൂന്യം *അട്ടി, size_t size)
{
  എങ്കിൽ (state->ngrn < N_X_ARG_REG)
    തിരികെ (അക്ഷരം *)&context->x[state->ngrn++] + BE (വലുപ്പം (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  തിരികെ (അക്ഷരം *)allocate_to_അട്ടി (state, അട്ടി, size, size)
	 + BE (വലുപ്പം (UINT64) - size);
}

സ്ഥിതം ശൂന്യം *
allocate_int128_to_reg_or_അട്ടി (ഘടന call_context *context,
			         ഘടന arg_state *state, ശൂന്യം *അട്ടി)
{
  unsigned ngrn = state->ngrn;
  ശൂന്യം *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  എങ്കിൽ (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  അല്ലെങ്കിൽ
    {
      ret = allocate_to_അട്ടി (state, അട്ടി, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  തിരികെ ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  പൂർണ്ണസംഖ്യ flags, i, n;

  തിരഞ്ഞെടുപ്പ് (rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_POINTER:
      flags = (വലുപ്പം(ശൂന്യം *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_UINT128:
    അവസ്ഥ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
    അവസ്ഥ FFI_TYPE_STRUCT:
    അവസ്ഥ FFI_TYPE_VECTOR:
    അവസ്ഥ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      എങ്കിൽ (flags == 0)
	{
	  size_t s = rtype->size;
	  എങ്കിൽ (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (s == 16)
	    flags = AARCH64_RET_INT128;
	  അല്ലെങ്കിൽ എങ്കിൽ (s == 8)
	    flags = AARCH64_RET_INT64;
	  അല്ലെങ്കിൽ
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      abort();
    }

  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    എങ്കിൽ (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	നിർത്തുക;
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
  വേണ്ടി (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      എങ്കിൽ (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#എങ്കിൽ defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  തിരികെ FFI_OK;
}

#എങ്കിൽ defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned പൂർണ്ണസംഖ്യ nfixedargs,
			 unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  തിരികെ status;
}
#അല്ലെങ്കിൽ
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned പൂർണ്ണസംഖ്യ nfixedargs, unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  തിരികെ status;
}
#endif /* __APPLE__ */

ബാഹ്യ ശൂന്യം ffi_call_SYSV (ഘടന call_context *context, ശൂന്യം *frame,
			   ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, പൂർണ്ണസംഖ്യ flags,
			   ശൂന്യം *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *orig_rvalue,
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ഘടന call_context *context;
  ശൂന്യം *അട്ടി, *frame, *rvalue;
  ഘടന arg_state state;
  size_t അട്ടി_bytes, rtype_size, rsize;
  പൂർണ്ണസംഖ്യ i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  അട്ടി_bytes = cif->bytes;

  എങ്കിൽ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  എങ്കിൽ (flags & AARCH64_RET_IN_MEM)
    {
      എങ്കിൽ (orig_rvalue == ശൂന്യ)
	rsize = rtype_size;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (orig_rvalue == ശൂന്യ)
    flags &= AARCH64_FLAG_ARG_V;
  അല്ലെങ്കിൽ എങ്കിൽ (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (വലുപ്പം(ഘടന call_context) + അട്ടി_bytes + 40 + rsize);
  അട്ടി = context + 1;
  frame = (ശൂന്യം*)((uintptr_t)അട്ടി + (uintptr_t)അട്ടി_bytes);
  rvalue = (rsize ? (ശൂന്യം*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, അട്ടി_bytes);
  വേണ്ടി (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      ശൂന്യം *a = avalue[i];
      പൂർണ്ണസംഖ്യ h, t;
      ശൂന്യം *dest;

      t = ty->type;
      തിരഞ്ഞെടുപ്പ് (t)
	{
	അവസ്ഥ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  നിർത്തുക;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    എങ്കിൽ (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    അല്ലെങ്കിൽ
	      {
		ശൂന്യം *d = allocate_to_അട്ടി (&state, അട്ടി, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#അല്ലെങ്കിൽ
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_അട്ടി (context, &state, അട്ടി);
	  memcpy (dest, a, 16);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	അവസ്ഥ FFI_TYPE_STRUCT:
	അവസ്ഥ FFI_TYPE_VECTOR:
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    എങ്കിൽ (h)
	      {
              പൂർണ്ണസംഖ്യ elems = 4 - (h & 3);
              എങ്കിൽ (cif->abi == FFI_WIN64 && isvariadic)
              {
                എങ്കിൽ (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  നിർത്തുക;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_അട്ടി(&state, അട്ടി, ty->alignment, s);
              }
              അല്ലെങ്കിൽ
              {
                എങ്കിൽ (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  നിർത്തുക;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (അക്ഷരം *)allocate_to_അട്ടി (&state, അട്ടി, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (വലുപ്പം (UINT64) - s) : 0);
              }
	      }
	    അല്ലെങ്കിൽ എങ്കിൽ (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_അട്ടി (&state, അട്ടി,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = വലുപ്പം (ശൂന്യം *);
		പോകുക do_pointer;
	      }
	    അല്ലെങ്കിൽ
	      {
		size_t n = (s + 7) / 8;
		എങ്കിൽ (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned പൂർണ്ണസംഖ്യ)n;
		  }
		അല്ലെങ്കിൽ
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_അട്ടി (&state, അട്ടി, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort();
	}

#എങ്കിൽ defined (__APPLE__)
      എങ്കിൽ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  എങ്കിൽ (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

#എങ്കിൽ FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

ബാഹ്യ ശൂന്യം ffi_closure_SYSV (ശൂന്യം) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_closure_SYSV_V (ശൂന്യം) FFI_HIDDEN;
#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ബാഹ്യ ശൂന്യം ffi_closure_SYSV_alt (ശൂന്യം) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_closure_SYSV_V_alt (ശൂന്യം) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
                      ശൂന്യം *user_data,
                      ശൂന്യം *codeloc)
{
  എങ്കിൽ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    തിരികെ FFI_BAD_ABI;

  ശൂന്യം (*start)(ശൂന്യം);

  എങ്കിൽ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  അല്ലെങ്കിൽ
    start = ffi_closure_SYSV;

#എങ്കിൽ FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_താക്കോൽ_function_pointer, 0);
#  endif
  ശൂന്യം **config = (ശൂന്യം **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#അല്ലെങ്കിൽ
  സ്ഥിതം സ്ഥിരം unsigned അക്ഷരം trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  അക്ഷരം *tramp = closure->tramp;

# എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      എങ്കിൽ (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      അല്ലെങ്കിൽ
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      പോകുക out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, വലുപ്പം(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned അക്ഷരം *tramp_code = tramp;
# അല്ലെങ്കിൽ
  unsigned അക്ഷരം *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

#ifdef FFI_GO_CLOSURES
ബാഹ്യ ശൂന്യം ffi_go_closure_SYSV (ശൂന്യം) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_go_closure_SYSV_V (ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*))
{
  ശൂന്യം (*start)(ശൂന്യം);

  എങ്കിൽ (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    തിരികെ FFI_BAD_ABI;

  എങ്കിൽ (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  അല്ലെങ്കിൽ
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
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

പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
			ശൂന്യം *user_data,
			ഘടന call_context *context,
			ശൂന്യം *അട്ടി, ശൂന്യം *rvalue, ശൂന്യം *struct_rvalue)
{
  ശൂന്യം **avalue = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം*));
  പൂർണ്ണസംഖ്യ i, h, nargs, flags, isvariadic = 0;
  ഘടന arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  എങ്കിൽ (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  വേണ്ടി (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      പൂർണ്ണസംഖ്യ t = ty->type;
      size_t n, s = ty->size;

      തിരഞ്ഞെടുപ്പ് (t)
	{
	അവസ്ഥ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_അട്ടി (context, &state, അട്ടി, s);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_അട്ടി (context, &state, അട്ടി);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	അവസ്ഥ FFI_TYPE_STRUCT:
	അവസ്ഥ FFI_TYPE_VECTOR:
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  എങ്കിൽ (h)
	    {
	      n = 4 - (h & 3);
              എങ്കിൽ (cif->abi == FFI_WIN64 && isvariadic)
                {
                  എങ്കിൽ (state.ngrn + n <= N_X_ARG_REG)
                    {
                      ശൂന്യം *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned പൂർണ്ണസംഖ്യ)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  അല്ലെങ്കിൽ
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (അക്ഷരം *)allocate_to_അട്ടി (&state, അട്ടി,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (വലുപ്പം (UINT64) - s) : 0);
                    }
                }
              അല്ലെങ്കിൽ
                {
                  എങ്കിൽ (state.nsrn + n <= N_V_ARG_REG)
                    {
                      ശൂന്യം *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned പൂർണ്ണസംഖ്യ)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  അല്ലെങ്കിൽ
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (അക്ഷരം *)allocate_to_അട്ടി (&state, അട്ടി,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (വലുപ്പം (UINT64) - s) : 0);
                    }
                }
            }
          അല്ലെങ്കിൽ എങ്കിൽ (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_അട്ടി (context, &state,
                                               അട്ടി, വലുപ്പം (ശൂന്യം *)),
                  വലുപ്പം (UINT64));
             avalue[i] = (ശൂന്യം *)(UINT32)avalue_tmp;
#അല്ലെങ്കിൽ
              avalue[i] = *(ശൂന്യം **)
              allocate_int_to_reg_or_അട്ടി (context, &state, അട്ടി,
                                         വലുപ്പം (ശൂന്യം *));
#endif
            }
          അല്ലെങ്കിൽ
            {
              n = (s + 7) / 8;
              എങ്കിൽ (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned പൂർണ്ണസംഖ്യ)n;
                }
              അല്ലെങ്കിൽ
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_അട്ടി(&state, അട്ടി,
                                           ty->alignment, s);
                }
            }
          നിർത്തുക;

        സ്ഥിരസ്ഥിതി:
          abort();
      }

#എങ്കിൽ defined (__APPLE__)
      എങ്കിൽ (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  എങ്കിൽ (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  തിരികെ flags;
}

#എങ്കിൽ defined(FFI_EXEC_STATIC_TRAMP)
ശൂന്യം *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ബാഹ്യ ശൂന്യം *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  തിരികെ &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

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

#જો defined(__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <fficonfig.h>
#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ "internal.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#અન્તર્ભાવ <windows.h> /* FlushInstructionCache */
#endif
#અન્તર્ભાવ <tramp.h>

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#જો FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
# જો FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#નહીંતર
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

સંઘ _d
{
  UINT64 d;
  UINT32 s[2];
};

સંરચના _v
{
  સંઘ _d d[2] __attribute__((aligned(16)));
};

સંરચના call_context
{
  સંરચના _v v[N_V_ARG_REG];
  UINT64 x[N_X_ARG_REG];
};

#જો FFI_EXEC_TRAMPOLINE_TABLE

#ifdef __MACH__
#ifdef HAVE_ARM64E_PTRAUTH
#અન્તર્ભાવ <ptrauth.h>
#endif
#અન્તર્ભાવ <mach/vm_param.h>
#endif

#નહીંતર

#જો defined (__clang__) && defined (__APPLE__)
બાહ્ય શૂન્ય sys_icache_invalidate (શૂન્ય *start, size_t len);
#endif

સ્થિર અંતઃરેખ શૂન્ય
ffi_clear_cache (શૂન્ય *start, શૂન્ય *end)
{
#જો defined (__clang__) && defined (__APPLE__)
  sys_icache_invalidate (start, (અક્ષર *)end - (અક્ષર *)start);
#અથવાજો defined (__GNUC__)
  __builtin___clear_cache (start, end);
#અથવાજો defined (_WIN32)
  FlushInstructionCache(GetCurrentProcess(), start, (અક્ષર*)end - (અક્ષર*)start);
#નહીંતર
#error "Missing builtin to flush instruction cache"
#endif
}

#endif

/* Return the base-2 logarithm of N (N assumed to be a power of two).  Used
   to map a vector register width (8 or 16 bytes) onto the D-/Q-register
   AARCH64_RET_* encoding.  */

સ્થિર પૂર્ણાંક
intlog2 (પૂર્ણાંક n)
{
  પૂર્ણાંક level = 0;
  જ્યારે (n >>= 1)
    ++level;
  પાછા level;
}

/* A subroutine of is_vfp_type.  Given a structure type, return the type code
   of the first non-structure element.  Recurse for structure elements.
   Return -1 if the structure is in fact empty, i.e. no nested elements.  */

સ્થિર પૂર્ણાંક
is_hfa0 (અચલ ffi_type *ty)
{
  ffi_type **elements = ty->elements;
  પૂર્ણાંક i, ret = -1;

  જો (elements != ખાલી)
    માટે (i = 0; elements[i]; ++i)
      {
        ret = elements[i]->type;
        જો (ret == FFI_TYPE_STRUCT || ret == FFI_TYPE_VECTOR
	    || ret == FFI_TYPE_COMPLEX)
          {
            ret = is_hfa0 (elements[i]);
            જો (ret < 0)
              ચાલુ;
          }
        તોડો;
      }

  પાછા ret;
}

/* A subroutine of is_vfp_type.  Return the size in bytes of the vector (SIMD)
   member of TY, i.e. the width of a single Neon register slot, or 0 if TY
   neither is nor contains a vector.  For a bare vector this is its whole size;
   for a homogeneous vector aggregate it is the size of one lane vector.  */

સ્થિર size_t
is_simd (અચલ ffi_type *ty)
{
  ffi_type **elements;
  પૂર્ણાંક i;

  જો (ty->type == FFI_TYPE_VECTOR)
    પાછા ty->size;

  elements = ty->elements;
  જો (elements != ખાલી)
    માટે (i = 0; elements[i]; ++i)
      {
        પૂર્ણાંક t = elements[i]->type;
        જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX
	    || t == FFI_TYPE_VECTOR)
          પાછા is_simd (elements[i]);
      }

  પાછા 0;
}

/* A subroutine of is_vfp_type.  Given a structure type, return true if all
   of the non-structure elements are the same as CANDIDATE.  */

સ્થિર પૂર્ણાંક
is_hfa1 (અચલ ffi_type *ty, પૂર્ણાંક candidate)
{
  ffi_type **elements = ty->elements;
  પૂર્ણાંક i;

  જો (elements != ખાલી)
    માટે (i = 0; elements[i]; ++i)
      {
        પૂર્ણાંક t = elements[i]->type;
        જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	    || t == FFI_TYPE_COMPLEX)
          {
            જો (!is_hfa1 (elements[i], candidate))
              પાછા 0;
          }
        નહીંતર જો (t != candidate)
          પાછા 0;
      }

  પાછા 1;
}

/* Determine if TY may be allocated to the FP registers.  This is both an
   fp scalar type as well as an homogenous floating point aggregate (HFA).
   That is, a structure consisting of 1 to 4 members of all the same type,
   where that type is an fp scalar.

   Returns non-zero iff TY is an HFA.  The result is the AARCH64_RET_*
   constant for the type.  */

સ્થિર પૂર્ણાંક
is_vfp_type (અચલ ffi_type *ty)
{
  ffi_type **elements;
  પૂર્ણાંક candidate, i;
  size_t size, ele_ગણતરી, simd_size;

  /* Quickest tests first.  */
  candidate = ty->type;
  પસંદગી (candidate)
    {
    મૂળભૂત:
      પાછા 0;
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      ele_ગણતરી = 1;
      જાઓ done;
    સ્થિતિ FFI_TYPE_COMPLEX:
      candidate = ty->elements[0]->type;
      પસંદગી (candidate)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  ele_ગણતરી = 2;
	  જાઓ done;
	}
      પાછા 0;
    સ્થિતિ FFI_TYPE_STRUCT:
    સ્થિતિ FFI_TYPE_VECTOR:
      તોડો;
    }

  /* No HFA/HVA types are smaller than 4 bytes, or larger than 64 bytes.  */
  size = ty->size;
  જો (size < 4 || size > 64)
    પાછા 0;

  /* Determine the width of the vector (SIMD) member, if any: 0 for a plain
     floating-point HFA, else the size in bytes of one Neon register slot.  */
  simd_size = is_simd (ty);

  /* Find the type of the first non-aggregate member.  */
  elements = ty->elements;
  candidate = elements[0]->type;
  જો (candidate == FFI_TYPE_STRUCT || candidate == FFI_TYPE_VECTOR
      || candidate == FFI_TYPE_COMPLEX)
    {
      માટે (i = 0; ; ++i)
        {
          candidate = is_hfa0 (elements[i]);
          જો (candidate >= 0)
            તોડો;
        }
    }

  જો (simd_size)
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
      પૂર્ણાંક અંક_registers;
      પૂર્ણાંક first_level_element_type;

      /* A Neon register slot is an S (4B), D (8B) or Q (16B).  A lane narrower
	 than 4 bytes has no short-vector register class under AAPCS64 and would
	 map below AARCH64_RET_S4, making extend_hfa_type() branch before its
	 jump table; reject it and let the generic aggregate path handle it.  */
      જો (reg_size < 4 || reg_size > 16 || size % reg_size != 0)
	પાછા 0;
      અંક_registers = (પૂર્ણાંક) (size / reg_size);
      જો (અંક_registers > 4)
	પાછા 0;

      /* For an aggregate, every member must itself be a vector (or nested
	 vector aggregate) of the same register width: this rejects a struct
	 that mixes a bare scalar with a vector even when the scalar's type
	 matches the vector's lane type.  A bare vector needs no such check --
	 its lanes were validated when its layout was computed.  */
      જો (ty->type != FFI_TYPE_VECTOR)
	માટે (i = 0; elements[i]; ++i)
	  જો (is_simd (elements[i]) != reg_size)
	    પાછા 0;

      /* Every lane must be the identical scalar type across the whole HVA
	 (this rejects, e.g., an aggregate mixing float and integer vectors).  */
      માટે (i = 0; elements[i]; ++i)
	{
	  પૂર્ણાંક t = elements[i]->type;
	  જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_VECTOR
	      || t == FFI_TYPE_COMPLEX)
	    {
	      જો (!is_hfa1 (elements[i], candidate))
		પાછા 0;
	    }
	  નહીંતર જો (t != candidate)
	    પાછા 0;
	}

      /* Reuse the AARCH64_RET_{S,D,Q}* codes, which are laid out as
	 (type * 4) + (4 - count) with FLOAT->S(4B), DOUBLE->D(8B),
	 LONGDOUBLE->Q(16B).  Map the register width onto that type axis.  */
      first_level_element_type = FFI_TYPE_FLOAT + intlog2 ((પૂર્ણાંક) reg_size) - 2;
      પાછા first_level_element_type * 4 + (4 - અંક_registers);
    }

  /* If the first member is not a floating point type, it's not an HFA.
     Also quickly re-check the size of the structure.  */
  પસંદગી (candidate)
    {
    સ્થિતિ FFI_TYPE_FLOAT:
      ele_ગણતરી = size / કદ(દશાંશ);
      જો (size != ele_ગણતરી * કદ(દશાંશ))
        પાછા 0;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      ele_ગણતરી = size / કદ(દ્વિગુણ);
      જો (size != ele_ગણતરી * કદ(દ્વિગુણ))
        પાછા 0;
      તોડો;
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      ele_ગણતરી = size / કદ(long દ્વિગુણ);
      જો (size != ele_ગણતરી * કદ(long દ્વિગુણ))
        પાછા 0;
      તોડો;
    મૂળભૂત:
      પાછા 0;
    }
  જો (ele_ગણતરી > 4)
    પાછા 0;

  /* Finally, make sure that all scalar elements are the same type.  */
  માટે (i = 0; elements[i]; ++i)
    {
      પૂર્ણાંક t = elements[i]->type;
      જો (t == FFI_TYPE_STRUCT || t == FFI_TYPE_COMPLEX)
        {
          જો (!is_hfa1 (elements[i], candidate))
            પાછા 0;
        }
      નહીંતર જો (t != candidate)
        પાછા 0;
    }

  /* All tests succeeded.  Encode the result.  */
 done:
  પાછા candidate * 4 + (4 - (પૂર્ણાંક)ele_ગણતરી);
}

/* Representation of the procedure call argument marshalling
   state.

   The terse state variable names match the names used in the AARCH64
   PCS.

   The struct area is allocated downwards from the top of the argument
   area.  It is used to hold copies of structures passed by value that are
   bigger than 16 bytes.  */

સંરચના arg_state
{
  unsigned ngrn;                /* Next general-purpose register number. */
  unsigned nsrn;                /* Next vector register number. */
  size_t nsaa;                  /* Next stack offset. */
  size_t next_struct_area;	/* Place to allocate big structs. */

#જો defined (__APPLE__)
  unsigned allocating_variadic;
#endif
};

/* Initialize a procedure call argument marshalling state.  */
સ્થિર શૂન્ય
arg_init (સંરચના arg_state *state, size_t size)
{
  state->ngrn = 0;
  state->nsrn = 0;
  state->nsaa = 0;
  state->next_struct_area = size;
#જો defined (__APPLE__)
  state->allocating_variadic = 0;
#endif
}

/* Allocate an aligned slot on the stack and return a pointer to it.  */
સ્થિર શૂન્ય *
allocate_to_ઢગલો (સંરચના arg_state *state, શૂન્ય *ઢગલો,
		   size_t alignment, size_t size)
{
  size_t nsaa = state->nsaa;

  /* Round up the NSAA to the larger of 8 or the natural
     alignment of the argument's type.  */
#જો defined (__APPLE__)
  જો (state->allocating_variadic && alignment < 8)
    alignment = 8;
#નહીંતર
  જો (alignment < 8)
    alignment = 8;
#endif

  nsaa = FFI_ALIGN (nsaa, alignment);
  state->nsaa = nsaa + size;

  પાછા (અક્ષર *)ઢગલો + nsaa;
}

/* Allocate and copy a structure that is passed by value on the stack and
   return a pointer to it.  */
સ્થિર શૂન્ય *
allocate_and_copy_struct_to_ઢગલો (સંરચના arg_state *state, શૂન્ય *ઢગલો,
				   size_t alignment, size_t size, શૂન્ય *કિંમત)
{
  size_t dest = state->next_struct_area - size;

  /* Round down to the natural alignment of the value.  */
  dest = FFI_ALIGN_DOWN (dest, alignment);
  state->next_struct_area = dest;

  પાછા memcpy ((અક્ષર *) ઢગલો + dest, કિંમત, size);
}

સ્થિર ffi_arg
extend_integer_type (શૂન્ય *source, પૂર્ણાંક type)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_TYPE_UINT8:
      {
        UINT8 u8;
        memcpy (&u8, source, કદ (u8));
        પાછા u8;
      }
    સ્થિતિ FFI_TYPE_SINT8:
      {
        SINT8 s8;
        memcpy (&s8, source, કદ (s8));
        પાછા s8;
      }
    સ્થિતિ FFI_TYPE_UINT16:
      {
        UINT16 u16;
        memcpy (&u16, source, કદ (u16));
        પાછા u16;
      }
    સ્થિતિ FFI_TYPE_SINT16:
      {
        SINT16 s16;
        memcpy (&s16, source, કદ (s16));
        પાછા s16;
      }
    સ્થિતિ FFI_TYPE_UINT32:
      {
        UINT32 u32;
        memcpy (&u32, source, કદ (u32));
        પાછા u32;
      }
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      {
        SINT32 s32;
        memcpy (&s32, source, કદ (s32));
        પાછા s32;
      }
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
      {
        UINT64 u64;
        memcpy (&u64, source, કદ (u64));
        પાછા u64;
      }
    સ્થિતિ FFI_TYPE_POINTER:
      {
        uintptr_t uptr;
        memcpy (&uptr, source, કદ (uptr));
        પાછા uptr;
      }
    મૂળભૂત:
      abort();
    }
}

#જો defined(_MSC_VER) && !defined(__clang__)
શૂન્ય extend_hfa_type (શૂન્ય *dest, શૂન્ય *src, પૂર્ણાંક h);
#નહીંતર
સ્થિર શૂન્ય
extend_hfa_type (શૂન્ય *dest, શૂન્ય *src, પૂર્ણાંક h)
{
  ptrdiff_t f = h - AARCH64_RET_S4;
  શૂન્ય *x0;

#define BTI_J "hint #36"
  __asm__ અસ્થિર (
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

#જો defined(_MSC_VER) && !defined(__clang__)
શૂન્ય* compress_hfa_type (શૂન્ય *dest, શૂન્ય *src, પૂર્ણાંક h);
#નહીંતર
સ્થિર શૂન્ય *
compress_hfa_type (શૂન્ય *dest, શૂન્ય *reg, પૂર્ણાંક h)
{
  પસંદગી (h)
    {
    સ્થિતિ AARCH64_RET_S1:
      જો (dest == reg)
	પાછા (અક્ષર *)dest + BE (12);
      નહીંતર
	*(દશાંશ *)dest = *(દશાંશ *)((અક્ષર *)reg + BE (12));
      તોડો;
    સ્થિતિ AARCH64_RET_S2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.s, v17.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      તોડો;
    સ્થિતિ AARCH64_RET_S3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.s, v17.s, v18.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      તોડો;
    સ્થિતિ AARCH64_RET_S4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.s, v17.s, v18.s, v19.s }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      તોડો;

    સ્થિતિ AARCH64_RET_D1:
      જો (dest == reg)
	પાછા (અક્ષર *)dest + BE (8);
      નહીંતર
	*(દ્વિગુણ *)dest = *(દ્વિગુણ *)((અક્ષર *)reg + BE (8));
      તોડો;
    સ્થિતિ AARCH64_RET_D2:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "st2 { v16.d, v17.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17");
      તોડો;
    સ્થિતિ AARCH64_RET_D3:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldr q18, [%1, #32]\n\t"
	   "st3 { v16.d, v17.d, v18.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18");
      તોડો;
    સ્થિતિ AARCH64_RET_D4:
      __asm__ ("ldp q16, q17, [%1]\n\t"
	   "ldp q18, q19, [%1, #32]\n\t"
	   "st4 { v16.d, v17.d, v18.d, v19.d }[0], [%0]"
	   : : "r"(dest), "r"(reg) : "memory", "v16", "v17", "v18", "v19");
      તોડો;

    મૂળભૂત:
      જો (dest != reg)
	પાછા memcpy (dest, reg, 16 * (4 - (h & 3)));
      તોડો;
    }
  પાછા dest;
}
#endif

/* Either allocate an appropriate register for the argument type, or if
   none are available, allocate a stack slot and return a pointer
   to the allocated space.  */

સ્થિર શૂન્ય *
allocate_int_to_reg_or_ઢગલો (સંરચના call_context *context,
			      સંરચના arg_state *state,
			      શૂન્ય *ઢગલો, size_t size)
{
  જો (state->ngrn < N_X_ARG_REG)
    પાછા (અક્ષર *)&context->x[state->ngrn++] + BE (કદ (UINT64) - size);

  state->ngrn = N_X_ARG_REG;
  પાછા (અક્ષર *)allocate_to_ઢગલો (state, ઢગલો, size, size)
	 + BE (કદ (UINT64) - size);
}

સ્થિર શૂન્ય *
allocate_int128_to_reg_or_ઢગલો (સંરચના call_context *context,
			         સંરચના arg_state *state, શૂન્ય *ઢગલો)
{
  unsigned ngrn = state->ngrn;
  શૂન્ય *ret;

  /* The normal AArch64 ABI requires even Xreg; Darwin does not. */
#ifndef __APPLE__
  ngrn += ngrn & 1;
#endif

  /* The value must fit entirely in registers, i.e. the low half may
     not be allocated to x7 with the high half spilled to the stack.  */
  જો (ngrn + 2 <= N_X_ARG_REG)
    {
      ret = &context->x[ngrn];
      ngrn += 2;
    }
  નહીંતર
    {
      ret = allocate_to_ઢગલો (state, ઢગલો, 16, 16);
      ngrn = N_X_ARG_REG;
    }
  state->ngrn = ngrn;
  પાછા ret;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  size_t bytes = cif->bytes;
  પૂર્ણાંક flags, i, n;

  પસંદગી (rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = AARCH64_RET_VOID;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      flags = AARCH64_RET_UINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      flags = AARCH64_RET_UINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT32:
      flags = AARCH64_RET_UINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      flags = AARCH64_RET_SINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      flags = AARCH64_RET_SINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      flags = AARCH64_RET_SINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      flags = AARCH64_RET_INT64;
      તોડો;
    સ્થિતિ FFI_TYPE_POINTER:
      flags = (કદ(શૂન્ય *) == 4 ? AARCH64_RET_UINT32 : AARCH64_RET_INT64);
      તોડો;

    સ્થિતિ FFI_TYPE_UINT128:
    સ્થિતિ FFI_TYPE_SINT128:
      flags = AARCH64_RET_INT128;
      તોડો;

    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
    સ્થિતિ FFI_TYPE_STRUCT:
    સ્થિતિ FFI_TYPE_VECTOR:
    સ્થિતિ FFI_TYPE_COMPLEX:
      flags = is_vfp_type (rtype);
      જો (flags == 0)
	{
	  size_t s = rtype->size;
	  જો (s > 16)
	    {
	      flags = AARCH64_RET_VOID | AARCH64_RET_IN_MEM;
	      bytes += 8;
	    }
	  નહીંતર જો (s == 16)
	    flags = AARCH64_RET_INT128;
	  નહીંતર જો (s == 8)
	    flags = AARCH64_RET_INT64;
	  નહીંતર
	    flags = AARCH64_RET_INT128 | AARCH64_RET_NEED_COPY;
	}
      તોડો;

    મૂળભૂત:
      abort();
    }

  માટે (i = 0, n = cif->nargs; i < n; i++)
    જો (is_vfp_type (cif->arg_types[i]))
      {
	flags |= AARCH64_FLAG_ARG_V;
	તોડો;
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
  માટે (i = 0, n = cif->nargs; i < n; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      જો (ty->size > 16 && !is_vfp_type (ty))
	bytes += 8;
    }

  /* Round the stack up to a multiple of the stack alignment requirement. */
  cif->bytes = (unsigned) FFI_ALIGN(bytes, 16);
  cif->flags = flags;
#જો defined (__APPLE__)
  cif->aarch64_nfixedargs = 0;
#endif

  પાછા FFI_OK;
}

#જો defined (__APPLE__)
/* Perform Apple-specific cif processing for variadic calls */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned પૂર્ણાંક nfixedargs,
			 unsigned પૂર્ણાંક ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->aarch64_nfixedargs = nfixedargs;
  પાછા status;
}
#નહીંતર
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned પૂર્ણાંક nfixedargs, unsigned પૂર્ણાંક ntotalargs)
{
  ffi_status status = ffi_prep_cif_machdep (cif);
  cif->flags |= AARCH64_FLAG_VARARG;
  પાછા status;
}
#endif /* __APPLE__ */

બાહ્ય શૂન્ય ffi_call_SYSV (સંરચના call_context *context, શૂન્ય *frame,
			   શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, પૂર્ણાંક flags,
			   શૂન્ય *closure) FFI_HIDDEN;

/* Call a function with the provided arguments and capture the return
   value.
   n.b. ffi_call_SYSV will steal the alloca'd `stack` variable here for use
   _as its own stack_ - so we need to compile this function without ASAN */
FFI_ASAN_NO_SANITIZE
સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *orig_rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  સંરચના call_context *context;
  શૂન્ય *ઢગલો, *frame, *rvalue;
  સંરચના arg_state state;
  size_t ઢગલો_bytes, rtype_size, rsize;
  પૂર્ણાંક i, nargs, flags, isvariadic = 0;
  ffi_type *rtype;

  flags = cif->flags;
  rtype = cif->rtype;
  rtype_size = rtype->size;
  ઢગલો_bytes = cif->bytes;

  જો (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  /* If the target function returns a structure via hidden pointer,
     then we cannot allow a null rvalue.  Otherwise, mash a null
     rvalue to void return type.  */
  rsize = 0;
  જો (flags & AARCH64_RET_IN_MEM)
    {
      જો (orig_rvalue == ખાલી)
	rsize = rtype_size;
    }
  નહીંતર જો (orig_rvalue == ખાલી)
    flags &= AARCH64_FLAG_ARG_V;
  નહીંતર જો (flags & AARCH64_RET_NEED_COPY)
    rsize = 16;

  /* Allocate consecutive stack for everything we'll need.
     The frame uses 40 bytes for: lr, fp, rvalue, flags, sp */
  context = alloca (કદ(સંરચના call_context) + ઢગલો_bytes + 40 + rsize);
  ઢગલો = context + 1;
  frame = (શૂન્ય*)((uintptr_t)ઢગલો + (uintptr_t)ઢગલો_bytes);
  rvalue = (rsize ? (શૂન્ય*)((uintptr_t)frame + 40) : orig_rvalue);

  arg_init (&state, ઢગલો_bytes);
  માટે (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t s = ty->size;
      શૂન્ય *a = avalue[i];
      પૂર્ણાંક h, t;
      શૂન્ય *dest;

      t = ty->type;
      પસંદગી (t)
	{
	સ્થિતિ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  તોડો;

	/* If the argument is a basic type the argument is allocated to an
	   appropriate register, or if none are available, to the stack.  */
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	do_pointer:
	  {
	    ffi_arg ext = extend_integer_type (a, t);
	    જો (state.ngrn < N_X_ARG_REG)
	      context->x[state.ngrn++] = ext;
	    નહીંતર
	      {
		શૂન્ય *d = allocate_to_ઢગલો (&state, ઢગલો, ty->alignment, s);
		state.ngrn = N_X_ARG_REG;
		/* Note that the default abi extends each argument
		   to a full 64-bit slot, while the iOS abi allocates
		   only enough space. */
#ifdef __APPLE__
		memcpy(d, a, s);
#નહીંતર
		*(ffi_arg *)d = ext;
#endif
	      }
	  }
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_SINT128:
	  dest = allocate_int128_to_reg_or_ઢગલો (context, &state, ઢગલો);
	  memcpy (dest, a, 16);
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_VECTOR:
	સ્થિતિ FFI_TYPE_COMPLEX:
	  {
	    h = is_vfp_type (ty);
	    જો (h)
	      {
              પૂર્ણાંક elems = 4 - (h & 3);
              જો (cif->abi == FFI_WIN64 && isvariadic)
              {
                જો (state.ngrn + elems <= N_X_ARG_REG)
                {
                  dest = &context->x[state.ngrn];
                  state.ngrn += elems;
                  extend_hfa_type(dest, a, h);
                  તોડો;
                }
                state.nsrn = N_X_ARG_REG;
                dest = allocate_to_ઢગલો(&state, ઢગલો, ty->alignment, s);
              }
              નહીંતર
              {
                જો (state.nsrn + elems <= N_V_ARG_REG)
                {
                  dest = &context->v[state.nsrn];
                  state.nsrn += elems;
                  extend_hfa_type (dest, a, h);
                  તોડો;
                }
                state.nsrn = N_V_ARG_REG;
                dest = (અક્ષર *)allocate_to_ઢગલો (&state, ઢગલો, ty->alignment, s)
                       + (t == FFI_TYPE_FLOAT ? BE (કદ (UINT64) - s) : 0);
              }
	      }
	    નહીંતર જો (s > 16)
	      {
		/* If the argument is a composite type that is larger than 16
		   bytes, then the argument is copied to memory, and
		   the argument is replaced by a pointer to the copy.  */
		dest = allocate_and_copy_struct_to_ઢગલો (&state, ઢગલો,
							  ty->alignment, s,
							  avalue[i]);
		a = &dest;
		t = FFI_TYPE_POINTER;
		s = કદ (શૂન્ય *);
		જાઓ do_pointer;
	      }
	    નહીંતર
	      {
		size_t n = (s + 7) / 8;
		જો (state.ngrn + n <= N_X_ARG_REG)
		  {
		    /* If the argument is a composite type and the size in
		       double-words is not more than the number of available
		       X registers, then the argument is copied into
		       consecutive X registers.  */
		    dest = &context->x[state.ngrn];
                    state.ngrn += (unsigned પૂર્ણાંક)n;
		  }
		નહીંતર
		  {
		    /* Otherwise, there are insufficient X registers. Further
		       X register allocations are prevented, the NSAA is
		       adjusted and the argument is copied to memory at the
		       adjusted NSAA.  */
		    state.ngrn = N_X_ARG_REG;
		    dest = allocate_to_ઢગલો (&state, ઢગલો, ty->alignment, s);
		  }
		}
	      memcpy (dest, a, s);
	    }
	  તોડો;

	મૂળભૂત:
	  abort();
	}

#જો defined (__APPLE__)
      જો (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  ffi_call_SYSV (context, frame, fn, rvalue, flags, closure);

  જો (flags & AARCH64_RET_NEED_COPY)
    memcpy (orig_rvalue, rvalue, rtype_size);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

#જો FFI_CLOSURES

#ifdef FFI_GO_CLOSURES
શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}
#endif /* FFI_GO_CLOSURES */

/* Build a trampoline.  */

બાહ્ય શૂન્ય ffi_closure_SYSV (શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_closure_SYSV_V (શૂન્ય) FFI_HIDDEN;
#જો defined(FFI_EXEC_STATIC_TRAMP)
બાહ્ય શૂન્ય ffi_closure_SYSV_alt (શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_closure_SYSV_V_alt (શૂન્ય) FFI_HIDDEN;
#endif

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
                      ffi_cif* cif,
                      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
                      શૂન્ય *user_data,
                      શૂન્ય *codeloc)
{
  જો (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    પાછા FFI_BAD_ABI;

  શૂન્ય (*start)(શૂન્ય);

  જો (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_closure_SYSV_V;
  નહીંતર
    start = ffi_closure_SYSV;

#જો FFI_EXEC_TRAMPOLINE_TABLE
# ifdef __MACH__
#  ifdef HAVE_ARM64E_PTRAUTH
  codeloc = ptrauth_auth_data(codeloc, ptrauth_ચાવી_function_pointer, 0);
#  endif
  શૂન્ય **config = (શૂન્ય **)((uint8_t *)codeloc - PAGE_MAX_SIZE);
  config[0] = closure;
  config[1] = start;
# endif
#નહીંતર
  સ્થિર અચલ unsigned અક્ષર trampoline[16] = {
    0x90, 0x00, 0x00, 0x58,	/* ldr	x16, tramp+16	*/
    0xf1, 0xff, 0xff, 0x10,	/* adr	x17, tramp+0	*/
    0x00, 0x02, 0x1f, 0xd6	/* br	x16		*/
  };
  અક્ષર *tramp = closure->tramp;

# જો defined(FFI_EXEC_STATIC_TRAMP)
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      જો (start == ffi_closure_SYSV_V)
          start = ffi_closure_SYSV_V_alt;
      નહીંતર
          start = ffi_closure_SYSV_alt;
      ffi_tramp_set_parms (closure->ftramp, start, closure);
      જાઓ out;
    }
# endif

  /* Initialize the dynamic trampoline. */
  memcpy (tramp, trampoline, કદ(trampoline));

  *(UINT64 *)(tramp + 16) = (uintptr_t)start;

  ffi_clear_cache(tramp, tramp + FFI_TRAMPOLINE_SIZE);

  /* Also flush the cache for code mapping.  */
# ifdef _WIN32
  // Not using dlmalloc.c for Windows ARM64 builds
  // so calling ffi_data_to_code_pointer() isn't necessary
  unsigned અક્ષર *tramp_code = tramp;
# નહીંતર
  unsigned અક્ષર *tramp_code = ffi_data_to_code_pointer (tramp);
# endif
  ffi_clear_cache (tramp_code, tramp_code + FFI_TRAMPOLINE_SIZE);
# જો defined(FFI_EXEC_STATIC_TRAMP)
out:
# endif
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

#ifdef FFI_GO_CLOSURES
બાહ્ય શૂન્ય ffi_go_closure_SYSV (શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_go_closure_SYSV_V (શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif* cif,
                     શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*))
{
  શૂન્ય (*start)(શૂન્ય);

  જો (cif->abi != FFI_SYSV && cif->abi != FFI_WIN64)
    પાછા FFI_BAD_ABI;

  જો (cif->flags & AARCH64_FLAG_ARG_V)
    start = ffi_go_closure_SYSV_V;
  નહીંતર
    start = ffi_go_closure_SYSV;

  closure->tramp = start;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
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

પૂર્ણાંક FFI_HIDDEN
ffi_closure_SYSV_inner (ffi_cif *cif,
			શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
			શૂન્ય *user_data,
			સંરચના call_context *context,
			શૂન્ય *ઢગલો, શૂન્ય *rvalue, શૂન્ય *struct_rvalue)
{
  શૂન્ય **avalue = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય*));
  પૂર્ણાંક i, h, nargs, flags, isvariadic = 0;
  સંરચના arg_state state;

  arg_init (&state, cif->bytes);

  flags = cif->flags;
  જો (flags & AARCH64_FLAG_VARARG)
  {
    isvariadic = 1;
    flags &= ~AARCH64_FLAG_VARARG;
  }

  માટે (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = cif->arg_types[i];
      પૂર્ણાંક t = ty->type;
      size_t n, s = ty->size;

      પસંદગી (t)
	{
	સ્થિતિ FFI_TYPE_VOID:
	  FFI_ASSERT (0);
	  તોડો;

	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = allocate_int_to_reg_or_ઢગલો (context, &state, ઢગલો, s);
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_SINT128:
	  avalue[i] = allocate_int128_to_reg_or_ઢગલો (context, &state, ઢગલો);
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_VECTOR:
	સ્થિતિ FFI_TYPE_COMPLEX:
	  h = is_vfp_type (ty);
	  જો (h)
	    {
	      n = 4 - (h & 3);
              જો (cif->abi == FFI_WIN64 && isvariadic)
                {
                  જો (state.ngrn + n <= N_X_ARG_REG)
                    {
                      શૂન્ય *reg = &context->x[state.ngrn];
                      state.ngrn += (unsigned પૂર્ણાંક)n;

                      /* Eeek! We need a pointer to the structure, however the
                       homogeneous float elements are being passed in individual
                       registers, therefore for float and double the structure
                       is not represented as a contiguous sequence of bytes in
                       our saved register context.  We don't need the original
                       contents of the register storage, so we reformat the
                       structure into the same memory.  */
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  નહીંતર
                    {
                      state.ngrn = N_X_ARG_REG;
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (અક્ષર *)allocate_to_ઢગલો (&state, ઢગલો,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (કદ (UINT64) - s) : 0);
                    }
                }
              નહીંતર
                {
                  જો (state.nsrn + n <= N_V_ARG_REG)
                    {
                      શૂન્ય *reg = &context->v[state.nsrn];
                      state.nsrn += (unsigned પૂર્ણાંક)n;
                      avalue[i] = compress_hfa_type(reg, reg, h);
                    }
                  નહીંતર
                    {
                      state.nsrn = N_V_ARG_REG;
                      avalue[i] = (અક્ષર *)allocate_to_ઢગલો (&state, ઢગલો,
                                                             ty->alignment, s)
                                  + (t == FFI_TYPE_FLOAT ? BE (કદ (UINT64) - s) : 0);
                    }
                }
            }
          નહીંતર જો (s > 16)
            {
              /* Replace Composite type of size greater than 16 with a
                  pointer.  */
#ifdef __ILP32__
             UINT64 avalue_tmp;
             memcpy (&avalue_tmp,
                  allocate_int_to_reg_or_ઢગલો (context, &state,
                                               ઢગલો, કદ (શૂન્ય *)),
                  કદ (UINT64));
             avalue[i] = (શૂન્ય *)(UINT32)avalue_tmp;
#નહીંતર
              avalue[i] = *(શૂન્ય **)
              allocate_int_to_reg_or_ઢગલો (context, &state, ઢગલો,
                                         કદ (શૂન્ય *));
#endif
            }
          નહીંતર
            {
              n = (s + 7) / 8;
              જો (state.ngrn + n <= N_X_ARG_REG)
                {
                  avalue[i] = &context->x[state.ngrn];
                  state.ngrn += (unsigned પૂર્ણાંક)n;
                }
              નહીંતર
                {
                  state.ngrn = N_X_ARG_REG;
                  avalue[i] = allocate_to_ઢગલો(&state, ઢગલો,
                                           ty->alignment, s);
                }
            }
          તોડો;

        મૂળભૂત:
          abort();
      }

#જો defined (__APPLE__)
      જો (i + 1 == cif->aarch64_nfixedargs)
	{
	  state.ngrn = N_X_ARG_REG;
	  state.nsrn = N_V_ARG_REG;
	  state.allocating_variadic = 1;
	}
#endif
    }

  જો (flags & AARCH64_RET_IN_MEM)
    rvalue = struct_rvalue;

  fun (cif, rvalue, avalue, user_data);

  પાછા flags;
}

#જો defined(FFI_EXEC_STATIC_TRAMP)
શૂન્ય *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  બાહ્ય શૂન્ય *trampoline_code_table;

  *tramp_size = AARCH64_TRAMP_SIZE;
  *map_size = AARCH64_TRAMP_MAP_SIZE;
  પાછા &trampoline_code_table;
}
#endif

#endif /* FFI_CLOSURES */

#endif /* (__aarch64__) || defined(__arm64__)|| defined (_M_ARM64)*/

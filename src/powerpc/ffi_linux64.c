/* -----------------------------------------------------------------------
   ffi_linux64.c - Copyright (C) 2013 IBM
                   Copyright (C) 2011, 2026 Anthony Green
                   Copyright (C) 2011 Kyle Moffett
                   Copyright (C) 2008 Red Hat, Inc
                   Copyright (C) 2007, 2008 Free Software Foundation, Inc
                   Copyright (c) 1998 Geoffrey Keating

   PowerPC Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#include "ffi.h"
#include <tramp.h>
#include <stdlib.h>

#ifdef POWERPC64
#include "ffi_common.h"
#include "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
enum {
  NUM_GPR_ARG_REGISTERS64 = 8,
  NUM_FPR_ARG_REGISTERS64 = 13,
  NUM_VEC_ARG_REGISTERS64 = 12,
};
enum { ASM_NEEDS_REGISTERS64 = 4 };


#якщо HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
void FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  якщо ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  інакше
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.size = 2 * ffi_type_longdouble.size;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#endif
}
#endif


static unsigned ціле
discover_homogeneous_aggregate (ffi_abi abi,
                                const ffi_type *t,
                                unsigned ціле *elnum)
{
  switch (t->type)
    {
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    case FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      якщо ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          повернути FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      інакше якщо ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          повернути FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    case FFI_TYPE_FLOAT:
    case FFI_TYPE_DOUBLE:
      *elnum = 1;
      повернути (ціле) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    case FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned ціле inner_elnum = 0;
	unsigned ціле inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	якщо (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    повернути inner;
	  }
	повернути 0;
      }
#endif

    case FFI_TYPE_STRUCT:;
      {
	unsigned ціле base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	поки (*el)
	  {
	    unsigned ціле el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    якщо (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      повернути 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#якщо _CALL_ELF == 2
	    якщо (total_elnum > 8)
	      повернути 0;
#інакше
	    якщо (total_elnum > 1)
	      повернути 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	повернути base_elt;
      }

    default:
      повернути 0;
    }
}


/* Perform machine dependent cif processing */
static ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_count = 0, intarg_count = 0, vecarg_count = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#якщо FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    повернути FFI_BAD_ABI;
#elif !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    повернути FFI_BAD_ABI;
#інакше
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    повернути FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#якщо _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * sizeof (long);

  /* Space for the general registers.  */
  bytes += NUM_GPR_ARG_REGISTERS64 * sizeof (long);
#інакше
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * sizeof (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * NUM_GPR_ARG_REGISTERS64 * sizeof (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#якщо _CALL_ELF == 2
homogeneous:
#endif
  switch (rtype)
    {
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    case FFI_TYPE_LONGDOUBLE:
      якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          break;
        }
      якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    case FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    case FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      break;

    case FFI_TYPE_UINT64:
    case FFI_TYPE_SINT64:
    case FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      break;

    case FFI_TYPE_STRUCT:
#якщо _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      якщо (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          goto homogeneous;
        }
      якщо (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          break;
        }
#endif
      intarg_count++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    case FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      break;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    case FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      switch (rtype)
        {
        case FFI_TYPE_FLOAT:
        case FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          goto homogeneous;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        case FFI_TYPE_LONGDOUBLE:
          якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              break;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          goto homogeneous;
#endif
        case FFI_TYPE_INT:
        case FFI_TYPE_SINT8:  case FFI_TYPE_UINT8:
        case FFI_TYPE_SINT16: case FFI_TYPE_UINT16:
        case FFI_TYPE_SINT32: case FFI_TYPE_UINT32:
        case FFI_TYPE_SINT64: case FFI_TYPE_UINT64:
        case FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          break;
        default:
          повернути FFI_BAD_TYPEDEF;
        }
      break;
#endif

    default:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      break;
    }

  для (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned ціле align;

      switch ((*ptr)->type)
	{
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	case FFI_TYPE_LONGDOUBLE:
          якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_count = (intarg_count + 3) & ~0x1;
              якщо (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              break;
            }
	  якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_count++;
	      intarg_count++;
	    }
	  /* Fall through.  */
#endif
	case FFI_TYPE_DOUBLE:
	case FFI_TYPE_FLOAT:
	  fparg_count++;
	  intarg_count++;
	  якщо (fparg_count > NUM_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  break;

	case FFI_TYPE_STRUCT:
	  якщо ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      якщо (align > 16)
		align = 16;
	      align = align / 8;
	      якщо (align > 1)
		intarg_count = FFI_ALIGN (intarg_count, align);
	    }
	  intarg_count += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          якщо (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count += elnum;
              якщо (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              break;
            }
	  інакше
#endif
	  якщо (elt)
	    {
	      fparg_count += elnum;
	      якщо (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  інакше
	    {
	      якщо (intarg_count > NUM_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  break;

	case FFI_TYPE_POINTER:
	case FFI_TYPE_UINT64:
	case FFI_TYPE_SINT64:
	case FFI_TYPE_INT:
	case FFI_TYPE_UINT32:
	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT16:
	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT8:
	case FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_count++;
	  якщо (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  break;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	case FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  якщо (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  switch (elt)
	    {
	    case FFI_TYPE_FLOAT:
	    case FFI_TYPE_DOUBLE:
	      fparg_count += 2;
	      intarg_count += 2;
	      якщо (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      break;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    case FFI_TYPE_LONGDOUBLE:
	      якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_count += 2;
		  intarg_count = (intarg_count + 1) & ~0x1;
		  intarg_count += 4;
		  якщо (vecarg_count > NUM_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  break;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_count += 4;
	      intarg_count += 4;
	      якщо (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      break;
#endif
	    case FFI_TYPE_INT:
	    case FFI_TYPE_SINT8:  case FFI_TYPE_UINT8:
	    case FFI_TYPE_SINT16: case FFI_TYPE_UINT16:
	    case FFI_TYPE_SINT32: case FFI_TYPE_UINT32:
	    case FFI_TYPE_SINT64: case FFI_TYPE_UINT64:
	    case FFI_TYPE_POINTER:
	      intarg_count += 2;
	      break;
	    default:
	      повернути FFI_BAD_TYPEDEF;
	    }
	  якщо (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  break;
#endif

	default:
	  FFI_ASSERT (0);
	}
    }

  якщо (fparg_count != 0)
    flags |= FLAG_FP_ARGUMENTS;
  якщо (intarg_count > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  якщо (vecarg_count != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  якщо (fparg_count != 0)
    bytes += NUM_FPR_ARG_REGISTERS64 * sizeof (double);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  якщо (vecarg_count != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += NUM_VEC_ARG_REGISTERS64 * sizeof (float128);
  }

  /* Stack space.  */
#якщо _CALL_ELF == 2
  якщо ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_count * sizeof (long);
#інакше
  якщо (intarg_count > NUM_GPR_ARG_REGISTERS64)
    bytes += (intarg_count - NUM_GPR_ARG_REGISTERS64) * sizeof (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  повернути FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  якщо ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#якщо _CALL_ELF != 2
  інакше якщо (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  інакше
    повернути FFI_BAD_ABI;
  повернути ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned ціле nfixedargs,
			  unsigned ціле ntotalargs MAYBE_UNUSED)
{
  якщо ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#якщо _CALL_ELF != 2
  інакше якщо (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  інакше
    повернути FFI_BAD_ABI;
#якщо _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  повернути ffi_prep_cif_linux64_core (cif);
}


/* ffi_prep_args64 is called by the assembly routine once stack space
   has been allocated for the function's arguments.

   The stack layout we want looks like this:

   |   Ret addr from ffi_call_LINUX64	8bytes	|	higher addresses
   |--------------------------------------------|
   |   CR save area			8bytes	|
   |--------------------------------------------|
   |   Previous backchain pointer	8	|	stack pointer here
   |--------------------------------------------|<+ <<<	on entry to
   |   Saved r28-r31			4*8	| |	ffi_call_LINUX64
   |--------------------------------------------| |
   |   GPR registers r3-r10		8*8	| |
   |--------------------------------------------| |
   |   FPR registers f1-f13 (optional)	13*8	| |
   |--------------------------------------------| |
   |   VEC registers v2-v13 (optional)  12*16   | |
   |--------------------------------------------| |
   |   Parameter save area		        | |
   |--------------------------------------------| |
   |   TOC save area			8	| |
   |--------------------------------------------| |	stack	|
   |   Linker doubleword		8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Compiler doubleword		8	| |
   |--------------------------------------------| |	lower addresses
   |   Space for callee's LR		8	| |
   |--------------------------------------------| |
   |   CR save area			8	| |
   |--------------------------------------------| |	stack pointer here
   |   Current backchain pointer	8	|-/	during
   |--------------------------------------------|   <<<	ffi_call_LINUX64

*/

void FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *const stack)
{
  const unsigned long bytes = ecif->cif->bytes;
  const unsigned long flags = ecif->cif->flags;

  typedef union
  {
    char *c;
    unsigned long *ul;
    float *f;
    double *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_base;
  valp gpr_end;
  valp rest;
  valp next_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_base;
  unsigned ціле fparg_count;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned ціле vecarg_count;

  unsigned ціле i, words, nargs, nfixedargs;
  ffi_type **ptr;
  double double_tmp;
  union
  {
    void **v;
    char **c;
    signed char **sc;
    unsigned char **uc;
    signed short **ss;
    unsigned short **us;
    signed ціле **si;
    unsigned ціле **ui;
    unsigned long **ul;
    float **f;
    double **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (char *) stack + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - NUM_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + NUM_GPR_ARG_REGISTERS64;
#якщо _CALL_ELF == 2
  rest.ul = stack + 4 + NUM_GPR_ARG_REGISTERS64;
#інакше
  rest.ul = stack + 6 + NUM_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - NUM_FPR_ARG_REGISTERS64;
  fparg_count = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  якщо (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  інакше
    vec_base.p = gpr_base.p;
  vec_base.f128 -= NUM_VEC_ARG_REGISTERS64;
  vecarg_count = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (char *) stack & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  якщо (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (char *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#якщо _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  якщо ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  для (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned ціле elt, elnum;

      switch ((*ptr)->type)
	{
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	case FFI_TYPE_LONGDOUBLE:
          якщо ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              якщо (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              якщо (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, sizeof (float128));
              інакше
		memcpy (next_arg.f128, *p_argv.f128, sizeof (float128));
              якщо (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_count++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              break;
            }
	  якщо ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# якщо _CALL_ELF != 2
		  якщо ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      інакше
		*next_arg.d = double_tmp;
	      якщо (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      double_tmp = (*p_argv.d)[1];
	      якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# якщо _CALL_ELF != 2
		  якщо ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      інакше
		*next_arg.d = double_tmp;
	      якщо (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      break;
	    }
	  /* Fall through.  */
#endif
	case FFI_TYPE_DOUBLE:
#якщо _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#якщо _CALL_ELF != 2
	      якщо ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  інакше
	    *next_arg.d = double_tmp;
	  якщо (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  break;

	case FFI_TYPE_FLOAT:
#якщо _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#якщо _CALL_ELF != 2
	      якщо ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (float) double_tmp;
# інакше
		  next_arg.f[0] = (float) double_tmp;
# endif
		}
#endif
	    }
	  інакше
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (float) double_tmp;
# інакше
	      next_arg.f[0] = (float) double_tmp;
# endif
	    }
	  якщо (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  break;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	case FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  якщо (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  якщо (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned ціле j;
	      для (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  якщо (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  якщо (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, sizeof (float128));
		  інакше
		    memcpy (next_arg.f128, cval + j, sizeof (float128));
		  якщо (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_count++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      break;
	    }
	  якщо (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      double *cval = (double *) *p_argv.v;
	      unsigned ціле j;
	      для (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  інакше
		    *next_arg.d = double_tmp;
		  якщо (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      break;
	    }
#endif
	  якщо (elt == FFI_TYPE_FLOAT)
	    {
	      float *cval = (float *) *p_argv.v;
	      unsigned ціле j;
	      для (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  інакше
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (float) double_tmp;
# інакше
		      next_arg.f[0] = (float) double_tmp;
# endif
		    }
		  якщо (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  інакше якщо (elt == FFI_TYPE_DOUBLE)
	    {
	      double *cval = (double *) *p_argv.v;
	      unsigned ціле j;
	      для (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  інакше
		    *next_arg.d = double_tmp;
		  якщо (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  інакше
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      char *cval = (char *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned ціле j;
	      для (j = 0; j < 2; j++)
		{
		  char *half = cval + j * hsize;
		  unsigned long gprvalue;
		  switch (elt)
		    {
		    case FFI_TYPE_UINT8:
		      gprvalue = *(unsigned char *) half; break;
		    case FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed char *) half;
		      break;
		    case FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; break;
		    case FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      break;
		    case FFI_TYPE_UINT32:
		      gprvalue = *(unsigned ціле *) half; break;
		    case FFI_TYPE_INT:
		    case FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed ціле *) half;
		      break;
		    case FFI_TYPE_SINT64:
		    case FFI_TYPE_UINT64:
		    case FFI_TYPE_POINTER:
		    default:
		      gprvalue = *(unsigned long *) half; break;
		    }
		  *next_arg.ul++ = gprvalue;
		  якщо (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  break;
#endif

	case FFI_TYPE_STRUCT:
	  якщо ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      якщо (align > 16)
		align = 16;
	      якщо (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  якщо (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  якщо (elt)
	    {
#якщо _CALL_ELF == 2
	      union {
		void *v;
		float *f;
		double *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              якщо (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  do
                    {
                      якщо (vecarg_count < NUM_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, sizeof (float128));
                      інакше
		        memcpy (next_arg.f128, arg.f128++, sizeof (float128));
                      якщо (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_count++;
                    }
                  поки (--elnum != 0);
                }
              інакше
#endif
	      якщо (elt == FFI_TYPE_FLOAT)
		{
		  do
		    {
		      double_tmp = *arg.f++;
		      якщо (fparg_count < NUM_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      інакше
			*next_arg.f = (float) double_tmp;
		      якщо (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_count++;
		    }
		  поки (--elnum != 0);
		  якщо ((next_arg.p & 7) != 0)
                    якщо (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      інакше
		do
		  {
		    double_tmp = *arg.d++;
		    якщо (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    інакше
		      *next_arg.d = double_tmp;
		    якщо (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_count++;
		  }
		поки (--elnum != 0);
#інакше
	      якщо (elt == FFI_TYPE_FLOAT)
		goto do_float;
	      інакше
		goto do_double;
#endif
	    }
	  інакше
	    {
	      words = ((*ptr)->size + 7) / 8;
	      якщо (next_arg.ul >= gpr_base.ul && next_arg.ul + words > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + words * 8 - first;
		}
	      інакше
		{
		  char *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  якщо ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += words;
		  якщо (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  break;

	case FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  goto putgpr;
	case FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  goto putgpr;
	case FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  goto putgpr;
	case FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  goto putgpr;
	case FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  goto putgpr;
	case FFI_TYPE_INT:
	case FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  goto putgpr;

	case FFI_TYPE_UINT64:
	case FFI_TYPE_SINT64:
	case FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  якщо (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  break;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#якщо _CALL_ELF == 2
#define MIN_CACHE_LINE_SIZE 8

static void
flush_icache (char *wraddr, char *xaddr, ціле size)
{
  ціле i;
  для (i = 0; i < size; i += MIN_CACHE_LINE_SIZE)
    __asm__ volatile ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ volatile ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      void (*fun) (ffi_cif *, void *, void **, void *),
			      void *user_data,
			      void *codeloc)
{
  якщо (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    повернути FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  якщо (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      void (*dest)(void) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  інакше
#endif
    {
#якщо _CALL_ELF == 2
      unsigned ціле *tramp = (unsigned ціле *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(void **) &tramp[4] = (void *) ffi_closure_LINUX64;
      *(void **) &tramp[6] = codeloc;
      flush_icache ((char *) tramp, (char *) codeloc, 4 * 4);
#інакше
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      void **tramp = (void **) &closure->tramp[0];
      memcpy (&tramp[0], (void **) ffi_closure_LINUX64, sizeof (void *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (void **) ffi_closure_LINUX64 + 1, sizeof (void *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  повернути FFI_OK;
}


ціле FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    void (*fun) (ffi_cif *, void *, void **, void *),
			    void *user_data,
			    void *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  void **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + NUM_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + NUM_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * sizeof (void *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  якщо (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (void *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#якщо _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  якщо ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  поки (i < avn)
    {
      unsigned ціле elt, elnum;

      switch (arg_types[i]->type)
	{
	case FFI_TYPE_SINT8:
	case FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (char *) pst + 7;
	  pst++;
	  break;
#endif

	case FFI_TYPE_SINT16:
	case FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (char *) pst + 6;
	  pst++;
	  break;
#endif

	case FFI_TYPE_SINT32:
	case FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (char *) pst + 4;
	  pst++;
	  break;
#endif

	case FFI_TYPE_SINT64:
	case FFI_TYPE_UINT64:
	case FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  break;

	case FFI_TYPE_STRUCT:
	  якщо ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      якщо (align > 16)
		align = 16;
	      якщо (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  якщо (elt)
	    {
#якщо _CALL_ELF == 2
	      union {
		void *v;
		unsigned long *ul;
		float *f;
		double *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              якщо (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  якщо (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  інакше
                    to.v = pst;
                }
              інакше
#endif
	      якщо (pfr + elnum <= end_pfr)
		to.v = pfr;
	      інакше
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              якщо (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  do
                    {
                      якщо (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, sizeof (float128));
                      інакше
		        memcpy (to.f128, from.f128, sizeof (float128));
                      to.f128++;
                      from.f128++;
                    }
                  поки (--elnum != 0);
                }
              інакше
#endif
	      якщо (elt == FFI_TYPE_FLOAT)
		{
		  do
		    {
		      якщо (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (float) pfr->d;
			  pfr++;
			}
		      інакше
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  поки (--elnum != 0);
		}
	      інакше
		{
		  do
		    {
		      якщо (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      інакше
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  поки (--elnum != 0);
		}
#інакше
	      якщо (elt == FFI_TYPE_FLOAT)
		goto do_float;
	      інакше
		goto do_double;
#endif
	    }
	  інакше
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      якщо (arg_types[i]->size < 8)
		avalue[i] = (char *) pst + 8 - arg_types[i]->size;
	      інакше
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  break;

#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	case FFI_TYPE_LONGDOUBLE:
          якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              якщо (((unsigned long) pst & 0xF) != 0)
                ++pst;
              якщо (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              інакше
                avalue[i] = pst;
              pst += 2;
              break;
            }
          інакше якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      якщо (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      інакше
		{
		  якщо (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      break;
	    }
	  /* Fall through.  */
#endif
	case FFI_TYPE_DOUBLE:
#якщо _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  якщо (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  інакше
	    avalue[i] = pst;
	  pst++;
	  break;

	case FFI_TYPE_FLOAT:
#якщо _CALL_ELF != 2
	do_float:
#endif
	  якщо (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (float) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  інакше
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (char *) pst + 4;
#інакше
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  break;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	case FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned ціле j;
	    elt = arg_types[i]->elements[0]->type;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    якщо (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    якщо (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * sizeof (float128));
		якщо (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		для (j = 0; j < 2; j++)
		  {
		    якщо (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, sizeof (float128));
		    інакше
		      memcpy (&cval[j], pst, sizeof (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		break;
	      }
	    якщо (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		double *cval = alloca (4 * sizeof (double));
		для (j = 0; j < 4; j++)
		  {
		    якщо (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    інакше
		      cval[j] = *(double *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		break;
	      }
#endif
	    якщо (elt == FFI_TYPE_FLOAT)
	      {
		float *cval = alloca (2 * sizeof (float));
		для (j = 0; j < 2; j++)
		  {
		    якщо (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (float) pfr->d;
			pfr++;
		      }
		    інакше
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((float *) pst)[1];
#інакше
			cval[j] = ((float *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    інакше якщо (elt == FFI_TYPE_DOUBLE)
	      {
		double *cval = alloca (2 * sizeof (double));
		для (j = 0; j < 2; j++)
		  {
		    якщо (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    інакше
		      cval[j] = *(double *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    інакше
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		char *cval = alloca (2 * hsize);
		для (j = 0; j < 2; j++)
		  {
		    char *src;
#ifndef __LITTLE_ENDIAN__
		    src = (char *) pst + (8 - hsize);
#інакше
		    src = (char *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  break;
#endif

	default:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  switch (cif->rtype->type)
    {
    case FFI_TYPE_VOID:
      повернути PPC_LD_NONE;
    case FFI_TYPE_FLOAT:
      повернути PPC_LD_F32;
    case FFI_TYPE_DOUBLE:
      повернути PPC_LD_F64;
#якщо FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    case FFI_TYPE_LONGDOUBLE:
      якщо ((cif->flags & FLAG_RETURNS_VEC) != 0)
	повернути PPC64_LD_VECTOR;
      повернути PPC_LD_F128;
#endif
    case FFI_TYPE_UINT8:
      повернути PPC_LD_U8;
    case FFI_TYPE_SINT8:
      повернути PPC_LD_S8;
    case FFI_TYPE_UINT16:
      повернути PPC_LD_U16;
    case FFI_TYPE_SINT16:
      повернути PPC_LD_S16;
    case FFI_TYPE_UINT32:
      повернути PPC_LD_U32;
    case FFI_TYPE_INT:
    case FFI_TYPE_SINT32:
      повернути PPC_LD_S32;
    case FFI_TYPE_POINTER:
      повернути PPC_LD_PTR;
    case FFI_TYPE_UINT64:
    case FFI_TYPE_SINT64:
      повернути PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    case FFI_TYPE_COMPLEX:
      {
	ціле inner = cif->rtype->elements[0]->type;
#якщо FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	якщо (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    якщо ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      повернути PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	якщо (inner == FFI_TYPE_FLOAT)
	  повернути PPC64_LD_FLOAT_HOMOG;
	якщо (inner == FFI_TYPE_DOUBLE)
	  повернути PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  char *rv = rvalue;
	  unsigned long re, im;
	  switch (inner)
	    {
	    case FFI_TYPE_UINT8:
	      re = ((unsigned char *) rv)[0];
	      im = ((unsigned char *) rv)[1];
	      break;
	    case FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed char *) rv)[0];
	      im = (unsigned long) (long) ((signed char *) rv)[1];
	      break;
	    case FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      break;
	    case FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      break;
	    case FFI_TYPE_UINT32:
	      re = ((unsigned ціле *) rv)[0];
	      im = ((unsigned ціле *) rv)[1];
	      break;
	    case FFI_TYPE_INT:
	    case FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed ціле *) rv)[0];
	      im = (unsigned long) (long) ((signed ціле *) rv)[1];
	      break;
	    case FFI_TYPE_SINT64:
	    case FFI_TYPE_UINT64:
	    case FFI_TYPE_POINTER:
	    default:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      break;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  повернути PPC_LD_R3R4;
	}
      }
#endif
    case FFI_TYPE_STRUCT:
      якщо ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  якщо ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      switch (cif->rtype->size)
		{
		case 0:
		  повернути PPC_LD_NONE;
		case 1:
		  повернути PPC_LD_U8;
		case 2:
		  повернути PPC_LD_U16;
		case 3:
		  повернути PPC64_LD_STRUCT_3;
		case 4:
		  повернути PPC_LD_U32;
		case 5:
		  повернути PPC64_LD_STRUCT_5;
		case 6:
		  повернути PPC64_LD_STRUCT_6;
		case 7:
		  повернути PPC64_LD_STRUCT_7;
		case 8 ... 16:
		  повернути PPC_LD_R3R4;
		}
	      break;
	    }
	  якщо ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    повернути PPC64_LD_VECTOR_HOMOG;
	  якщо ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    повернути PPC64_LD_DOUBLE_HOMOG;
	  повернути PPC64_LD_FLOAT_HOMOG;
        }
      повернути PPC_LD_NONE;
    }
  abort();
}
#endif

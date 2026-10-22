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

#ضمن "ffi.h"
#ضمن <tramp.h>
#ضمن <stdlib.h>

#ifdef POWERPC64
#ضمن "ffi_common.h"
#ضمن "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
تعداد {
  NUM_GPR_ARG_REGISTERS64 = 8,
  NUM_FPR_ARG_REGISTERS64 = 13,
  NUM_VEC_ARG_REGISTERS64 = 12,
};
تعداد { ASM_NEEDS_REGISTERS64 = 4 };


#אם HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
فراغ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  אם ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  אחרת
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


ساكن unsigned صحيح
discover_homogeneous_aggregate (ffi_abi abi,
                                ثابت ffi_type *t,
                                unsigned صحيح *elnum)
{
  بدل (t->type)
    {
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      אם ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          החזר FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      אחרת אם ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          החזר FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      *elnum = 1;
      החזר (صحيح) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned صحيح inner_elnum = 0;
	unsigned صحيح inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	אם (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    החזר inner;
	  }
	החזר 0;
      }
#endif

    حالة FFI_TYPE_STRUCT:;
      {
	unsigned صحيح base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	כלעוד (*el)
	  {
	    unsigned صحيح el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    אם (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      החזר 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#אם _CALL_ELF == 2
	    אם (total_elnum > 8)
	      החזר 0;
#אחרת
	    אם (total_elnum > 1)
	      החזר 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	החזר base_elt;
      }

    افتراضي:
      החזר 0;
    }
}


/* Perform machine dependent cif processing */
ساكن ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_count = 0, intarg_count = 0, vecarg_count = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#אם FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    החזר FFI_BAD_ABI;
#وإلاإذا !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    החזר FFI_BAD_ABI;
#אחרת
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    החזר FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#אם _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * حجم (long);

  /* Space for the general registers.  */
  bytes += NUM_GPR_ARG_REGISTERS64 * حجم (long);
#אחרת
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * حجم (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * NUM_GPR_ARG_REGISTERS64 * حجم (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#אם _CALL_ELF == 2
homogeneous:
#endif
  بدل (rtype)
    {
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          اكسر;
        }
      אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    حالة FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    حالة FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      اكسر;

    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      اكسر;

    حالة FFI_TYPE_STRUCT:
#אם _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      אם (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          اذهب homogeneous;
        }
      אם (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          اكسر;
        }
#endif
      intarg_count++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالة FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      اكسر;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      بدل (rtype)
        {
        حالة FFI_TYPE_FLOAT:
        حالة FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          اذهب homogeneous;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        حالة FFI_TYPE_LONGDOUBLE:
          אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              اكسر;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          اذهب homogeneous;
#endif
        حالة FFI_TYPE_INT:
        حالة FFI_TYPE_SINT8:  حالة FFI_TYPE_UINT8:
        حالة FFI_TYPE_SINT16: حالة FFI_TYPE_UINT16:
        حالة FFI_TYPE_SINT32: حالة FFI_TYPE_UINT32:
        حالة FFI_TYPE_SINT64: حالة FFI_TYPE_UINT64:
        حالة FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          اكسر;
        افتراضي:
          החזר FFI_BAD_TYPEDEF;
        }
      اكسر;
#endif

    افتراضي:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      اكسر;
    }

  עבור (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned صحيح align;

      بدل ((*ptr)->type)
	{
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_count = (intarg_count + 3) & ~0x1;
              אם (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_count++;
	      intarg_count++;
	    }
	  /* Fall through.  */
#endif
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_FLOAT:
	  fparg_count++;
	  intarg_count++;
	  אם (fparg_count > NUM_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  אם ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      אם (align > 16)
		align = 16;
	      align = align / 8;
	      אם (align > 1)
		intarg_count = FFI_ALIGN (intarg_count, align);
	    }
	  intarg_count += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          אם (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count += elnum;
              אם (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  אחרת
#endif
	  אם (elt)
	    {
	      fparg_count += elnum;
	      אם (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  אחרת
	    {
	      אם (intarg_count > NUM_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  اكسر;

	حالة FFI_TYPE_POINTER:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_count++;
	  אם (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالة FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  אם (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  بدل (elt)
	    {
	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_DOUBLE:
	      fparg_count += 2;
	      intarg_count += 2;
	      אם (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    حالة FFI_TYPE_LONGDOUBLE:
	      אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_count += 2;
		  intarg_count = (intarg_count + 1) & ~0x1;
		  intarg_count += 4;
		  אם (vecarg_count > NUM_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  اكسر;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_count += 4;
	      intarg_count += 4;
	      אם (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#endif
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT8:  حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16: حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_SINT32: حالة FFI_TYPE_UINT32:
	    حالة FFI_TYPE_SINT64: حالة FFI_TYPE_UINT64:
	    حالة FFI_TYPE_POINTER:
	      intarg_count += 2;
	      اكسر;
	    افتراضي:
	      החזר FFI_BAD_TYPEDEF;
	    }
	  אם (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;
#endif

	افتراضي:
	  FFI_ASSERT (0);
	}
    }

  אם (fparg_count != 0)
    flags |= FLAG_FP_ARGUMENTS;
  אם (intarg_count > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  אם (vecarg_count != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  אם (fparg_count != 0)
    bytes += NUM_FPR_ARG_REGISTERS64 * حجم (مزدوج);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  אם (vecarg_count != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += NUM_VEC_ARG_REGISTERS64 * حجم (float128);
  }

  /* Stack space.  */
#אם _CALL_ELF == 2
  אם ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_count * حجم (long);
#אחרת
  אם (intarg_count > NUM_GPR_ARG_REGISTERS64)
    bytes += (intarg_count - NUM_GPR_ARG_REGISTERS64) * حجم (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  החזר FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  אם ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#אם _CALL_ELF != 2
  אחרת אם (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  אחרת
    החזר FFI_BAD_ABI;
  החזר ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned صحيح nfixedargs,
			  unsigned صحيح ntotalargs MAYBE_UNUSED)
{
  אם ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#אם _CALL_ELF != 2
  אחרת אם (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  אחרת
    החזר FFI_BAD_ABI;
#אם _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  החזר ffi_prep_cif_linux64_core (cif);
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

فراغ FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *ثابت stack)
{
  ثابت unsigned long bytes = ecif->cif->bytes;
  ثابت unsigned long flags = ecif->cif->flags;

  تعريفنوع اتحاد
  {
    حرف *c;
    unsigned long *ul;
    عائم *f;
    مزدوج *d;
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
  unsigned صحيح fparg_count;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned صحيح vecarg_count;

  unsigned صحيح i, words, nargs, nfixedargs;
  ffi_type **ptr;
  مزدوج double_tmp;
  اتحاد
  {
    فراغ **v;
    حرف **c;
    signed حرف **sc;
    unsigned حرف **uc;
    signed short **ss;
    unsigned short **us;
    signed صحيح **si;
    unsigned صحيح **ui;
    unsigned long **ul;
    عائم **f;
    مزدوج **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (حرف *) stack + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - NUM_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + NUM_GPR_ARG_REGISTERS64;
#אם _CALL_ELF == 2
  rest.ul = stack + 4 + NUM_GPR_ARG_REGISTERS64;
#אחרת
  rest.ul = stack + 6 + NUM_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - NUM_FPR_ARG_REGISTERS64;
  fparg_count = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  אם (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  אחרת
    vec_base.p = gpr_base.p;
  vec_base.f128 -= NUM_VEC_ARG_REGISTERS64;
  vecarg_count = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (حرف *) stack & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  אם (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#אם _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  אם ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  עבור (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned صحيح elt, elnum;

      بدل ((*ptr)->type)
	{
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          אם ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              אם (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              אם (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, حجم (float128));
              אחרת
		memcpy (next_arg.f128, *p_argv.f128, حجم (float128));
              אם (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_count++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              اكسر;
            }
	  אם ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# אם _CALL_ELF != 2
		  אם ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      אחרת
		*next_arg.d = double_tmp;
	      אם (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      double_tmp = (*p_argv.d)[1];
	      אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# אם _CALL_ELF != 2
		  אם ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      אחרת
		*next_arg.d = double_tmp;
	      אם (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
	  /* Fall through.  */
#endif
	حالة FFI_TYPE_DOUBLE:
#אם _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#אם _CALL_ELF != 2
	      אם ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  אחרת
	    *next_arg.d = double_tmp;
	  אם (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
#אם _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#אם _CALL_ELF != 2
	      אם ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (عائم) double_tmp;
# אחרת
		  next_arg.f[0] = (عائم) double_tmp;
# endif
		}
#endif
	    }
	  אחרת
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (عائم) double_tmp;
# אחרת
	      next_arg.f[0] = (عائم) double_tmp;
# endif
	    }
	  אם (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالة FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  אם (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  אם (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned صحيح j;
	      עבור (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  אם (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  אם (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, حجم (float128));
		  אחרת
		    memcpy (next_arg.f128, cval + j, حجم (float128));
		  אם (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_count++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      اكسر;
	    }
	  אם (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      مزدوج *cval = (مزدوج *) *p_argv.v;
	      unsigned صحيح j;
	      עבור (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  אחרת
		    *next_arg.d = double_tmp;
		  אם (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
#endif
	  אם (elt == FFI_TYPE_FLOAT)
	    {
	      عائم *cval = (عائم *) *p_argv.v;
	      unsigned صحيح j;
	      עבור (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  אחרת
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (عائم) double_tmp;
# אחרת
		      next_arg.f[0] = (عائم) double_tmp;
# endif
		    }
		  אם (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  אחרת אם (elt == FFI_TYPE_DOUBLE)
	    {
	      مزدوج *cval = (مزدوج *) *p_argv.v;
	      unsigned صحيح j;
	      עבור (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  אחרת
		    *next_arg.d = double_tmp;
		  אם (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  אחרת
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      حرف *cval = (حرف *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned صحيح j;
	      עבור (j = 0; j < 2; j++)
		{
		  حرف *half = cval + j * hsize;
		  unsigned long gprvalue;
		  بدل (elt)
		    {
		    حالة FFI_TYPE_UINT8:
		      gprvalue = *(unsigned حرف *) half; اكسر;
		    حالة FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed حرف *) half;
		      اكسر;
		    حالة FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; اكسر;
		    حالة FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      اكسر;
		    حالة FFI_TYPE_UINT32:
		      gprvalue = *(unsigned صحيح *) half; اكسر;
		    حالة FFI_TYPE_INT:
		    حالة FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed صحيح *) half;
		      اكسر;
		    حالة FFI_TYPE_SINT64:
		    حالة FFI_TYPE_UINT64:
		    حالة FFI_TYPE_POINTER:
		    افتراضي:
		      gprvalue = *(unsigned long *) half; اكسر;
		    }
		  *next_arg.ul++ = gprvalue;
		  אם (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  اكسر;
#endif

	حالة FFI_TYPE_STRUCT:
	  אם ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      אם (align > 16)
		align = 16;
	      אם (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  אם (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  אם (elt)
	    {
#אם _CALL_ELF == 2
	      اتحاد {
		فراغ *v;
		عائم *f;
		مزدوج *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              אם (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  עשה
                    {
                      אם (vecarg_count < NUM_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, حجم (float128));
                      אחרת
		        memcpy (next_arg.f128, arg.f128++, حجم (float128));
                      אם (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_count++;
                    }
                  כלעוד (--elnum != 0);
                }
              אחרת
#endif
	      אם (elt == FFI_TYPE_FLOAT)
		{
		  עשה
		    {
		      double_tmp = *arg.f++;
		      אם (fparg_count < NUM_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      אחרת
			*next_arg.f = (عائم) double_tmp;
		      אם (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_count++;
		    }
		  כלעוד (--elnum != 0);
		  אם ((next_arg.p & 7) != 0)
                    אם (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      אחרת
		עשה
		  {
		    double_tmp = *arg.d++;
		    אם (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    אחרת
		      *next_arg.d = double_tmp;
		    אם (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_count++;
		  }
		כלעוד (--elnum != 0);
#אחרת
	      אם (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      אחרת
		اذهب do_double;
#endif
	    }
	  אחרת
	    {
	      words = ((*ptr)->size + 7) / 8;
	      אם (next_arg.ul >= gpr_base.ul && next_arg.ul + words > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + words * 8 - first;
		}
	      אחרת
		{
		  حرف *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  אם ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += words;
		  אם (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  اكسر;

	حالة FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  اذهب putgpr;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  اذهب putgpr;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  אם (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  اكسر;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#אם _CALL_ELF == 2
#define MIN_CACHE_LINE_SIZE 8

ساكن فراغ
flush_icache (حرف *wraddr, حرف *xaddr, صحيح size)
{
  صحيح i;
  עבור (i = 0; i < size; i += MIN_CACHE_LINE_SIZE)
    __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			      فراغ *user_data,
			      فراغ *codeloc)
{
  אם (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    החזר FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  אם (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  אחרת
#endif
    {
#אם _CALL_ELF == 2
      unsigned صحيح *tramp = (unsigned صحيح *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(فراغ **) &tramp[4] = (فراغ *) ffi_closure_LINUX64;
      *(فراغ **) &tramp[6] = codeloc;
      flush_icache ((حرف *) tramp, (حرف *) codeloc, 4 * 4);
#אחרת
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      فراغ **tramp = (فراغ **) &closure->tramp[0];
      memcpy (&tramp[0], (فراغ **) ffi_closure_LINUX64, حجم (فراغ *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (فراغ **) ffi_closure_LINUX64 + 1, حجم (فراغ *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  החזר FFI_OK;
}


صحيح FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			    فراغ *user_data,
			    فراغ *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  فراغ **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + NUM_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + NUM_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * حجم (فراغ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  אם (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (فراغ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#אם _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  אם ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  כלעוד (i < avn)
    {
      unsigned صحيح elt, elnum;

      بدل (arg_types[i]->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 7;
	  pst++;
	  اكسر;
#endif

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 6;
	  pst++;
	  اكسر;
#endif

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 4;
	  pst++;
	  اكسر;
#endif

	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  אם ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      אם (align > 16)
		align = 16;
	      אם (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  אם (elt)
	    {
#אם _CALL_ELF == 2
	      اتحاد {
		فراغ *v;
		unsigned long *ul;
		عائم *f;
		مزدوج *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              אם (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  אם (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  אחרת
                    to.v = pst;
                }
              אחרת
#endif
	      אם (pfr + elnum <= end_pfr)
		to.v = pfr;
	      אחרת
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              אם (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  עשה
                    {
                      אם (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, حجم (float128));
                      אחרת
		        memcpy (to.f128, from.f128, حجم (float128));
                      to.f128++;
                      from.f128++;
                    }
                  כלעוד (--elnum != 0);
                }
              אחרת
#endif
	      אם (elt == FFI_TYPE_FLOAT)
		{
		  עשה
		    {
		      אם (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (عائم) pfr->d;
			  pfr++;
			}
		      אחרת
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  כלעוד (--elnum != 0);
		}
	      אחרת
		{
		  עשה
		    {
		      אם (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      אחרת
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  כלעוד (--elnum != 0);
		}
#אחרת
	      אם (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      אחרת
		اذهب do_double;
#endif
	    }
	  אחרת
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      אם (arg_types[i]->size < 8)
		avalue[i] = (حرف *) pst + 8 - arg_types[i]->size;
	      אחרת
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  اكسر;

#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              אם (((unsigned long) pst & 0xF) != 0)
                ++pst;
              אם (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              אחרת
                avalue[i] = pst;
              pst += 2;
              اكسر;
            }
          אחרת אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      אם (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      אחרת
		{
		  אם (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      اكسر;
	    }
	  /* Fall through.  */
#endif
	حالة FFI_TYPE_DOUBLE:
#אם _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  אם (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  אחרת
	    avalue[i] = pst;
	  pst++;
	  اكسر;

	حالة FFI_TYPE_FLOAT:
#אם _CALL_ELF != 2
	do_float:
#endif
	  אם (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (عائم) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  אחרת
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (حرف *) pst + 4;
#אחרת
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  اكسر;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالة FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned صحيح j;
	    elt = arg_types[i]->elements[0]->type;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    אם (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    אם (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * حجم (float128));
		אם (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		עבור (j = 0; j < 2; j++)
		  {
		    אם (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, حجم (float128));
		    אחרת
		      memcpy (&cval[j], pst, حجم (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		اكسر;
	      }
	    אם (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		مزدوج *cval = alloca (4 * حجم (مزدوج));
		עבור (j = 0; j < 4; j++)
		  {
		    אם (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    אחרת
		      cval[j] = *(مزدوج *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		اكسر;
	      }
#endif
	    אם (elt == FFI_TYPE_FLOAT)
	      {
		عائم *cval = alloca (2 * حجم (عائم));
		עבור (j = 0; j < 2; j++)
		  {
		    אם (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (عائم) pfr->d;
			pfr++;
		      }
		    אחרת
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((عائم *) pst)[1];
#אחרת
			cval[j] = ((عائم *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    אחרת אם (elt == FFI_TYPE_DOUBLE)
	      {
		مزدوج *cval = alloca (2 * حجم (مزدوج));
		עבור (j = 0; j < 2; j++)
		  {
		    אם (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    אחרת
		      cval[j] = *(مزدوج *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    אחרת
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		حرف *cval = alloca (2 * hsize);
		עבור (j = 0; j < 2; j++)
		  {
		    حرف *src;
#ifndef __LITTLE_ENDIAN__
		    src = (حرف *) pst + (8 - hsize);
#אחרת
		    src = (حرف *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  اكسر;
#endif

	افتراضي:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      החזר PPC_LD_NONE;
    حالة FFI_TYPE_FLOAT:
      החזר PPC_LD_F32;
    حالة FFI_TYPE_DOUBLE:
      החזר PPC_LD_F64;
#אם FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      אם ((cif->flags & FLAG_RETURNS_VEC) != 0)
	החזר PPC64_LD_VECTOR;
      החזר PPC_LD_F128;
#endif
    حالة FFI_TYPE_UINT8:
      החזר PPC_LD_U8;
    حالة FFI_TYPE_SINT8:
      החזר PPC_LD_S8;
    حالة FFI_TYPE_UINT16:
      החזר PPC_LD_U16;
    حالة FFI_TYPE_SINT16:
      החזר PPC_LD_S16;
    حالة FFI_TYPE_UINT32:
      החזר PPC_LD_U32;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      החזר PPC_LD_S32;
    حالة FFI_TYPE_POINTER:
      החזר PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      החזר PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      {
	صحيح inner = cif->rtype->elements[0]->type;
#אם FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	אם (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    אם ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      החזר PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	אם (inner == FFI_TYPE_FLOAT)
	  החזר PPC64_LD_FLOAT_HOMOG;
	אם (inner == FFI_TYPE_DOUBLE)
	  החזר PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  حرف *rv = rvalue;
	  unsigned long re, im;
	  بدل (inner)
	    {
	    حالة FFI_TYPE_UINT8:
	      re = ((unsigned حرف *) rv)[0];
	      im = ((unsigned حرف *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed حرف *) rv)[0];
	      im = (unsigned long) (long) ((signed حرف *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_UINT32:
	      re = ((unsigned صحيح *) rv)[0];
	      im = ((unsigned صحيح *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed صحيح *) rv)[0];
	      im = (unsigned long) (long) ((signed صحيح *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT64:
	    حالة FFI_TYPE_UINT64:
	    حالة FFI_TYPE_POINTER:
	    افتراضي:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      اكسر;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  החזר PPC_LD_R3R4;
	}
      }
#endif
    حالة FFI_TYPE_STRUCT:
      אם ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  אם ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      بدل (cif->rtype->size)
		{
		حالة 0:
		  החזר PPC_LD_NONE;
		حالة 1:
		  החזר PPC_LD_U8;
		حالة 2:
		  החזר PPC_LD_U16;
		حالة 3:
		  החזר PPC64_LD_STRUCT_3;
		حالة 4:
		  החזר PPC_LD_U32;
		حالة 5:
		  החזר PPC64_LD_STRUCT_5;
		حالة 6:
		  החזר PPC64_LD_STRUCT_6;
		حالة 7:
		  החזר PPC64_LD_STRUCT_7;
		حالة 8 ... 16:
		  החזר PPC_LD_R3R4;
		}
	      اكسر;
	    }
	  אם ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    החזר PPC64_LD_VECTOR_HOMOG;
	  אם ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    החזר PPC64_LD_DOUBLE_HOMOG;
	  החזר PPC64_LD_FLOAT_HOMOG;
        }
      החזר PPC_LD_NONE;
    }
  abort();
}
#endif

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
  ژمارە_GPR_ARG_REGISTERS64 = 8,
  ژمارە_FPR_ARG_REGISTERS64 = 13,
  ژمارە_VEC_ARG_REGISTERS64 = 12,
};
تعداد { ASM_NEEDS_REGISTERS64 = 4 };


#إذا HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
فراغ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  إذا ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  وإلا
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
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      إذا ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          أرجع FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      وإلا إذا ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          أرجع FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      *elnum = 1;
      أرجع (صحيح) t->type;

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
	إذا (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    أرجع inner;
	  }
	أرجع 0;
      }
#endif

    حالة FFI_TYPE_STRUCT:;
      {
	unsigned صحيح base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	طالما (*el)
	  {
	    unsigned صحيح el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    إذا (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      أرجع 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#إذا _CALL_ELF == 2
	    إذا (total_elnum > 8)
	      أرجع 0;
#وإلا
	    إذا (total_elnum > 1)
	      أرجع 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	أرجع base_elt;
      }

    افتراضي:
      أرجع 0;
    }
}


/* Perform machine dependent cif processing */
ساكن ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_ژماردن = 0, intarg_ژماردن = 0, vecarg_ژماردن = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#إذا FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    أرجع FFI_BAD_ABI;
#وإلاإذا !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    أرجع FFI_BAD_ABI;
#وإلا
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    أرجع FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#إذا _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * حجم (long);

  /* Space for the general registers.  */
  bytes += ژمارە_GPR_ARG_REGISTERS64 * حجم (long);
#وإلا
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * حجم (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * ژمارە_GPR_ARG_REGISTERS64 * حجم (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#إذا _CALL_ELF == 2
homogeneous:
#endif
  بدل (rtype)
    {
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          اكسر;
        }
      إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
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
#إذا _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      إذا (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          اذهب homogeneous;
        }
      إذا (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          اكسر;
        }
#endif
      intarg_ژماردن++;
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
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        حالة FFI_TYPE_LONGDOUBLE:
          إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
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
          أرجع FFI_BAD_TYPEDEF;
        }
      اكسر;
#endif

    افتراضي:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      اكسر;
    }

  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned صحيح align;

      بدل ((*ptr)->type)
	{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ژماردن++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ژماردن = (intarg_ژماردن + 3) & ~0x1;
              إذا (vecarg_ژماردن > ژمارە_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ژماردن++;
	      intarg_ژماردن++;
	    }
	  /* Fall through.  */
#endif
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_FLOAT:
	  fparg_ژماردن++;
	  intarg_ژماردن++;
	  إذا (fparg_ژماردن > ژمارە_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  إذا ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      إذا (align > 16)
		align = 16;
	      align = align / 8;
	      إذا (align > 1)
		intarg_ژماردن = FFI_ALIGN (intarg_ژماردن, align);
	    }
	  intarg_ژماردن += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          إذا (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ژماردن += elnum;
              إذا (vecarg_ژماردن > ژمارە_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  وإلا
#endif
	  إذا (elt)
	    {
	      fparg_ژماردن += elnum;
	      إذا (fparg_ژماردن > ژمارە_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  وإلا
	    {
	      إذا (intarg_ژماردن > ژمارە_GPR_ARG_REGISTERS64)
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
	  intarg_ژماردن++;
	  إذا (intarg_ژماردن > ژمارە_GPR_ARG_REGISTERS64)
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
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  إذا (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  بدل (elt)
	    {
	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_DOUBLE:
	      fparg_ژماردن += 2;
	      intarg_ژماردن += 2;
	      إذا (fparg_ژماردن > ژمارە_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    حالة FFI_TYPE_LONGDOUBLE:
	      إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ژماردن += 2;
		  intarg_ژماردن = (intarg_ژماردن + 1) & ~0x1;
		  intarg_ژماردن += 4;
		  إذا (vecarg_ژماردن > ژمارە_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  اكسر;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ژماردن += 4;
	      intarg_ژماردن += 4;
	      إذا (fparg_ژماردن > ژمارە_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#endif
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT8:  حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16: حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_SINT32: حالة FFI_TYPE_UINT32:
	    حالة FFI_TYPE_SINT64: حالة FFI_TYPE_UINT64:
	    حالة FFI_TYPE_POINTER:
	      intarg_ژماردن += 2;
	      اكسر;
	    افتراضي:
	      أرجع FFI_BAD_TYPEDEF;
	    }
	  إذا (intarg_ژماردن > ژمارە_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;
#endif

	افتراضي:
	  FFI_ASSERT (0);
	}
    }

  إذا (fparg_ژماردن != 0)
    flags |= FLAG_FP_ARGUMENTS;
  إذا (intarg_ژماردن > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  إذا (vecarg_ژماردن != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  إذا (fparg_ژماردن != 0)
    bytes += ژمارە_FPR_ARG_REGISTERS64 * حجم (مزدوج);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  إذا (vecarg_ژماردن != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += ژمارە_VEC_ARG_REGISTERS64 * حجم (float128);
  }

  /* Stack space.  */
#إذا _CALL_ELF == 2
  إذا ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_ژماردن * حجم (long);
#وإلا
  إذا (intarg_ژماردن > ژمارە_GPR_ARG_REGISTERS64)
    bytes += (intarg_ژماردن - ژمارە_GPR_ARG_REGISTERS64) * حجم (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  أرجع FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  إذا ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#إذا _CALL_ELF != 2
  وإلا إذا (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  وإلا
    أرجع FFI_BAD_ABI;
  أرجع ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned صحيح nfixedargs,
			  unsigned صحيح ntotalargs MAYBE_UNUSED)
{
  إذا ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#إذا _CALL_ELF != 2
  وإلا إذا (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  وإلا
    أرجع FFI_BAD_ABI;
#إذا _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  أرجع ffi_prep_cif_linux64_core (cif);
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
ffi_prep_args64 (extended_cif *ecif, unsigned long *ثابت کەڵەکە)
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
  unsigned صحيح fparg_ژماردن;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned صحيح vecarg_ژماردن;

  unsigned صحيح i, وشەکان, nargs, nfixedargs;
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

  stacktop.c = (حرف *) کەڵەکە + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - ژمارە_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + ژمارە_GPR_ARG_REGISTERS64;
#إذا _CALL_ELF == 2
  rest.ul = کەڵەکە + 4 + ژمارە_GPR_ARG_REGISTERS64;
#وإلا
  rest.ul = کەڵەکە + 6 + ژمارە_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - ژمارە_FPR_ARG_REGISTERS64;
  fparg_ژماردن = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  إذا (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  وإلا
    vec_base.p = gpr_base.p;
  vec_base.f128 -= ژمارە_VEC_ARG_REGISTERS64;
  vecarg_ژماردن = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (حرف *) کەڵەکە & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  إذا (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#إذا _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  إذا ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  لكل (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned صحيح elt, elnum;

      بدل ((*ptr)->type)
	{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              إذا (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              إذا (vecarg_ژماردن < ژمارە_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, حجم (float128));
              وإلا
		memcpy (next_arg.f128, *p_argv.f128, حجم (float128));
              إذا (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_ژماردن++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              اكسر;
            }
	  إذا ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# إذا _CALL_ELF != 2
		  إذا ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      وإلا
		*next_arg.d = double_tmp;
	      إذا (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ژماردن++;
	      double_tmp = (*p_argv.d)[1];
	      إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# إذا _CALL_ELF != 2
		  إذا ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      وإلا
		*next_arg.d = double_tmp;
	      إذا (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ژماردن++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
	  /* Fall through.  */
#endif
	حالة FFI_TYPE_DOUBLE:
#إذا _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#إذا _CALL_ELF != 2
	      إذا ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  وإلا
	    *next_arg.d = double_tmp;
	  إذا (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ژماردن++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
#إذا _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#إذا _CALL_ELF != 2
	      إذا ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (عائم) double_tmp;
# وإلا
		  next_arg.f[0] = (عائم) double_tmp;
# endif
		}
#endif
	    }
	  وإلا
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (عائم) double_tmp;
# وإلا
	      next_arg.f[0] = (عائم) double_tmp;
# endif
	    }
	  إذا (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ژماردن++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالة FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  إذا (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  إذا (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  إذا (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  إذا (vecarg_ژماردن < ژمارە_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, حجم (float128));
		  وإلا
		    memcpy (next_arg.f128, cval + j, حجم (float128));
		  إذا (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_ژماردن++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      اكسر;
	    }
	  إذا (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      مزدوج *cval = (مزدوج *) *p_argv.v;
	      unsigned صحيح j;
	      لكل (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وإلا
		    *next_arg.d = double_tmp;
		  إذا (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ژماردن++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
#endif
	  إذا (elt == FFI_TYPE_FLOAT)
	    {
	      عائم *cval = (عائم *) *p_argv.v;
	      unsigned صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وإلا
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (عائم) double_tmp;
# وإلا
		      next_arg.f[0] = (عائم) double_tmp;
# endif
		    }
		  إذا (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ژماردن++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وإلا إذا (elt == FFI_TYPE_DOUBLE)
	    {
	      مزدوج *cval = (مزدوج *) *p_argv.v;
	      unsigned صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وإلا
		    *next_arg.d = double_tmp;
		  إذا (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ژماردن++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وإلا
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      حرف *cval = (حرف *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned صحيح j;
	      لكل (j = 0; j < 2; j++)
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
		  إذا (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  اكسر;
#endif

	حالة FFI_TYPE_STRUCT:
	  إذا ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      إذا (align > 16)
		align = 16;
	      إذا (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  إذا (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  إذا (elt)
	    {
#إذا _CALL_ELF == 2
	      اتحاد {
		فراغ *v;
		عائم *f;
		مزدوج *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              إذا (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  افعل
                    {
                      إذا (vecarg_ژماردن < ژمارە_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, حجم (float128));
                      وإلا
		        memcpy (next_arg.f128, arg.f128++, حجم (float128));
                      إذا (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_ژماردن++;
                    }
                  طالما (--elnum != 0);
                }
              وإلا
#endif
	      إذا (elt == FFI_TYPE_FLOAT)
		{
		  افعل
		    {
		      double_tmp = *arg.f++;
		      إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      وإلا
			*next_arg.f = (عائم) double_tmp;
		      إذا (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_ژماردن++;
		    }
		  طالما (--elnum != 0);
		  إذا ((next_arg.p & 7) != 0)
                    إذا (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      وإلا
		افعل
		  {
		    double_tmp = *arg.d++;
		    إذا (fparg_ژماردن < ژمارە_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    وإلا
		      *next_arg.d = double_tmp;
		    إذا (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_ژماردن++;
		  }
		طالما (--elnum != 0);
#وإلا
	      إذا (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      وإلا
		اذهب do_double;
#endif
	    }
	  وإلا
	    {
	      وشەکان = ((*ptr)->size + 7) / 8;
	      إذا (next_arg.ul >= gpr_base.ul && next_arg.ul + وشەکان > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + وشەکان * 8 - first;
		}
	      وإلا
		{
		  حرف *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  إذا ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += وشەکان;
		  إذا (next_arg.ul == gpr_end.ul)
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
	  إذا (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  اكسر;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#إذا _CALL_ELF == 2
#define MIN_CACHE_هێڵ_SIZE 8

ساكن فراغ
flush_icache (حرف *wraddr, حرف *xaddr, صحيح size)
{
  صحيح i;
  لكل (i = 0; i < size; i += MIN_CACHE_هێڵ_SIZE)
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
  إذا (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    أرجع FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  وإلا
#endif
    {
#إذا _CALL_ELF == 2
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
#وإلا
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

  أرجع FFI_OK;
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
  ffi_dblfl *end_pfr = pfr + ژمارە_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + ژمارە_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * حجم (فراغ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (فراغ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#إذا _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  إذا ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  طالما (i < avn)
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
	  إذا ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      إذا (align > 16)
		align = 16;
	      إذا (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  إذا (elt)
	    {
#إذا _CALL_ELF == 2
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
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              إذا (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  إذا (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  وإلا
                    to.v = pst;
                }
              وإلا
#endif
	      إذا (pfr + elnum <= end_pfr)
		to.v = pfr;
	      وإلا
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              إذا (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  افعل
                    {
                      إذا (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, حجم (float128));
                      وإلا
		        memcpy (to.f128, from.f128, حجم (float128));
                      to.f128++;
                      from.f128++;
                    }
                  طالما (--elnum != 0);
                }
              وإلا
#endif
	      إذا (elt == FFI_TYPE_FLOAT)
		{
		  افعل
		    {
		      إذا (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (عائم) pfr->d;
			  pfr++;
			}
		      وإلا
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  طالما (--elnum != 0);
		}
	      وإلا
		{
		  افعل
		    {
		      إذا (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      وإلا
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  طالما (--elnum != 0);
		}
#وإلا
	      إذا (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      وإلا
		اذهب do_double;
#endif
	    }
	  وإلا
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      إذا (arg_types[i]->size < 8)
		avalue[i] = (حرف *) pst + 8 - arg_types[i]->size;
	      وإلا
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              إذا (((unsigned long) pst & 0xF) != 0)
                ++pst;
              إذا (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              وإلا
                avalue[i] = pst;
              pst += 2;
              اكسر;
            }
          وإلا إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      إذا (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      وإلا
		{
		  إذا (pfr < end_pfr && i < nfixedargs)
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
#إذا _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  إذا (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    avalue[i] = pst;
	  pst++;
	  اكسر;

	حالة FFI_TYPE_FLOAT:
#إذا _CALL_ELF != 2
	do_float:
#endif
	  إذا (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (عائم) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (حرف *) pst + 4;
#وإلا
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
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    إذا (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    إذا (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * حجم (float128));
		إذا (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		لكل (j = 0; j < 2; j++)
		  {
		    إذا (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, حجم (float128));
		    وإلا
		      memcpy (&cval[j], pst, حجم (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		اكسر;
	      }
	    إذا (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		مزدوج *cval = alloca (4 * حجم (مزدوج));
		لكل (j = 0; j < 4; j++)
		  {
		    إذا (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    وإلا
		      cval[j] = *(مزدوج *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		اكسر;
	      }
#endif
	    إذا (elt == FFI_TYPE_FLOAT)
	      {
		عائم *cval = alloca (2 * حجم (عائم));
		لكل (j = 0; j < 2; j++)
		  {
		    إذا (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (عائم) pfr->d;
			pfr++;
		      }
		    وإلا
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((عائم *) pst)[1];
#وإلا
			cval[j] = ((عائم *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    وإلا إذا (elt == FFI_TYPE_DOUBLE)
	      {
		مزدوج *cval = alloca (2 * حجم (مزدوج));
		لكل (j = 0; j < 2; j++)
		  {
		    إذا (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    وإلا
		      cval[j] = *(مزدوج *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    وإلا
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		حرف *cval = alloca (2 * hsize);
		لكل (j = 0; j < 2; j++)
		  {
		    حرف *src;
#ifndef __LITTLE_ENDIAN__
		    src = (حرف *) pst + (8 - hsize);
#وإلا
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
      أرجع PPC_LD_NONE;
    حالة FFI_TYPE_FLOAT:
      أرجع PPC_LD_F32;
    حالة FFI_TYPE_DOUBLE:
      أرجع PPC_LD_F64;
#إذا FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      إذا ((cif->flags & FLAG_RETURNS_VEC) != 0)
	أرجع PPC64_LD_VECTOR;
      أرجع PPC_LD_F128;
#endif
    حالة FFI_TYPE_UINT8:
      أرجع PPC_LD_U8;
    حالة FFI_TYPE_SINT8:
      أرجع PPC_LD_S8;
    حالة FFI_TYPE_UINT16:
      أرجع PPC_LD_U16;
    حالة FFI_TYPE_SINT16:
      أرجع PPC_LD_S16;
    حالة FFI_TYPE_UINT32:
      أرجع PPC_LD_U32;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      أرجع PPC_LD_S32;
    حالة FFI_TYPE_POINTER:
      أرجع PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      أرجع PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      {
	صحيح inner = cif->rtype->elements[0]->type;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	إذا (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      أرجع PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	إذا (inner == FFI_TYPE_FLOAT)
	  أرجع PPC64_LD_FLOAT_HOMOG;
	إذا (inner == FFI_TYPE_DOUBLE)
	  أرجع PPC64_LD_DOUBLE_HOMOG;
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
	  أرجع PPC_LD_R3R4;
	}
      }
#endif
    حالة FFI_TYPE_STRUCT:
      إذا ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  إذا ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      بدل (cif->rtype->size)
		{
		حالة 0:
		  أرجع PPC_LD_NONE;
		حالة 1:
		  أرجع PPC_LD_U8;
		حالة 2:
		  أرجع PPC_LD_U16;
		حالة 3:
		  أرجع PPC64_LD_STRUCT_3;
		حالة 4:
		  أرجع PPC_LD_U32;
		حالة 5:
		  أرجع PPC64_LD_STRUCT_5;
		حالة 6:
		  أرجع PPC64_LD_STRUCT_6;
		حالة 7:
		  أرجع PPC64_LD_STRUCT_7;
		حالة 8 ... 16:
		  أرجع PPC_LD_R3R4;
		}
	      اكسر;
	    }
	  إذا ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    أرجع PPC64_LD_VECTOR_HOMOG;
	  إذا ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    أرجع PPC64_LD_DOUBLE_HOMOG;
	  أرجع PPC64_LD_FLOAT_HOMOG;
        }
      أرجع PPC_LD_NONE;
    }
  abort();
}
#endif

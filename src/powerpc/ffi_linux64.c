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

#جے_تعریف_شدہ POWERPC64
#ضمن "ffi_common.h"
#ضمن "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
تعداد {
  عدد_GPR_ARG_REGISTERS64 = 8,
  عدد_FPR_ARG_REGISTERS64 = 13,
  عدد_VEC_ARG_REGISTERS64 = 12,
};
تعداد { ASM_NEEDS_REGISTERS64 = 4 };


#إذا HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
فراغ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  إذا ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.ناپ = 8;
      ffi_type_longdouble.alignment = 8;
    }
  وإلا
    {
      ffi_type_longdouble.ناپ = 16;
      ffi_type_longdouble.alignment = 16;
    }
#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.ناپ = 2 * ffi_type_longdouble.ناپ;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#جے_انت
}
#جے_انت


ساكن بنانشانی صحيح
discover_homogeneous_aggregate (ffi_abi abi,
                                ثابت ffi_type *t,
                                بنانشانی صحيح *elnum)
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
#جے_انت
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
      *elnum = 1;
      أرجع (صحيح) t->type;

#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	بنانشانی صحيح inner_elnum = 0;
	بنانشانی صحيح inner
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
#جے_انت

    حالة FFI_TYPE_STRUCT:;
      {
	بنانشانی صحيح ادھار_elt = 0, کل_elnum = 0;
	ffi_type **el = t->elements;
	طالما (*el)
	  {
	    بنانشانی صحيح el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    إذا (el_elt == 0
		|| (ادھار_elt && ادھار_elt != el_elt))
	      أرجع 0;
	    ادھار_elt = el_elt;
	    کل_elnum += el_elnum;
#إذا _CALL_ELF == 2
	    إذا (کل_elnum > 8)
	      أرجع 0;
#وإلا
	    إذا (کل_elnum > 1)
	      أرجع 0;
#جے_انت
	    el++;
	  }
	*elnum = کل_elnum;
	أرجع ادھار_elt;
      }

    افتراضي:
      أرجع 0;
    }
}


/* Perform machine dependent cif processing */
ساكن ffi_حالت
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  بنانشانی بائٹاں;
  بنانشانی i, fparg_گنتی = 0, intarg_گنتی = 0, vecarg_گنتی = 0;
  بنانشانی flags = cif->flags;
  بنانشانی elt, elnum, rtype;

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
#جے_انت

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#إذا _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  بائٹاں = (4 + ASM_NEEDS_REGISTERS64) * حجم (لما);

  /* Space for the general registers.  */
  بائٹاں += عدد_GPR_ARG_REGISTERS64 * حجم (لما);
#وإلا
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  بائٹاں = (6 + ASM_NEEDS_REGISTERS64) * حجم (لما);

  /* Space for the mandatory parm save area and general registers.  */
  بائٹاں += 2 * عدد_GPR_ARG_REGISTERS64 * حجم (لما);
#جے_انت

  /* Return value handling.  */
  rtype = cif->rtype->type;
#إذا _CALL_ELF == 2
homogeneous:
#جے_انت
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
	flags |= FLAG_RETURNS_128_بٹ;
      /* Fall through.  */
#جے_انت
    حالة FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٹ;
      /* Fall through.  */
    حالة FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      اكسر;

    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_اشارہ_گر:
      flags |= FLAG_RETURNS_64_بٹ;
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
      إذا (cif->rtype->ناپ <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          اكسر;
        }
#جے_انت
      intarg_گنتی++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالة FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      اكسر;

#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
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
#جے_انت
        حالة FFI_TYPE_INT:
        حالة FFI_TYPE_SINT8:  حالة FFI_TYPE_UINT8:
        حالة FFI_TYPE_SINT16: حالة FFI_TYPE_UINT16:
        حالة FFI_TYPE_SINT32: حالة FFI_TYPE_UINT32:
        حالة FFI_TYPE_SINT64: حالة FFI_TYPE_UINT64:
        حالة FFI_TYPE_اشارہ_گر:
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
#جے_انت

    افتراضي:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      اكسر;
    }

  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      بنانشانی صحيح align;

      بدل ((*ptr)->type)
	{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_گنتی++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_گنتی = (intarg_گنتی + 3) & ~0x1;
              إذا (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_گنتی++;
	      intarg_گنتی++;
	    }
	  /* Fall through.  */
#جے_انت
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_FLOAT:
	  fparg_گنتی++;
	  intarg_گنتی++;
	  إذا (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
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
		intarg_گنتی = FFI_ALIGN (intarg_گنتی, align);
	    }
	  intarg_گنتی += ((*ptr)->ناپ + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          إذا (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_گنتی += elnum;
              إذا (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              اكسر;
            }
	  وإلا
#جے_انت
	  إذا (elt)
	    {
	      fparg_گنتی += elnum;
	      إذا (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  وإلا
	    {
	      إذا (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  اكسر;

	حالة FFI_TYPE_اشارہ_گر:
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
	  intarg_گنتی++;
	  إذا (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;

#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
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
#جے_انت
	  بدل (elt)
	    {
	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_DOUBLE:
	      fparg_گنتی += 2;
	      intarg_گنتی += 2;
	      إذا (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    حالة FFI_TYPE_LONGDOUBLE:
	      إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_گنتی += 2;
		  intarg_گنتی = (intarg_گنتی + 1) & ~0x1;
		  intarg_گنتی += 4;
		  إذا (vecarg_گنتی > عدد_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  اكسر;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_گنتی += 4;
	      intarg_گنتی += 4;
	      إذا (fparg_گنتی > عدد_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      اكسر;
#جے_انت
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT8:  حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16: حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_SINT32: حالة FFI_TYPE_UINT32:
	    حالة FFI_TYPE_SINT64: حالة FFI_TYPE_UINT64:
	    حالة FFI_TYPE_اشارہ_گر:
	      intarg_گنتی += 2;
	      اكسر;
	    افتراضي:
	      أرجع FFI_BAD_TYPEDEF;
	    }
	  إذا (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  اكسر;
#جے_انت

	افتراضي:
	  FFI_ASSERT (0);
	}
    }

  إذا (fparg_گنتی != 0)
    flags |= FLAG_FP_ARGUMENTS;
  إذا (intarg_گنتی > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  إذا (vecarg_گنتی != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  إذا (fparg_گنتی != 0)
    بائٹاں += عدد_FPR_ARG_REGISTERS64 * حجم (مزدوج);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  إذا (vecarg_گنتی != 0) {
    بائٹاں = (بائٹاں + 15) & ~0xF;
    بائٹاں += عدد_VEC_ARG_REGISTERS64 * حجم (float128);
  }

  /* Stack space.  */
#إذا _CALL_ELF == 2
  إذا ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    بائٹاں += intarg_گنتی * حجم (لما);
#وإلا
  إذا (intarg_گنتی > عدد_GPR_ARG_REGISTERS64)
    بائٹاں += (intarg_گنتی - عدد_GPR_ARG_REGISTERS64) * حجم (لما);
#جے_انت

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائٹاں = (بائٹاں + 15) & ~0xF;

  cif->flags = flags;
  cif->بائٹاں = بائٹاں;

  أرجع FFI_OK;
}

ffi_حالت FFI_HIDDEN
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
#جے_انت
  وإلا
    أرجع FFI_BAD_ABI;
  أرجع ffi_prep_cif_linux64_core (cif);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  بنانشانی صحيح nfixedargs,
			  بنانشانی صحيح ntotalargs MAYBE_بن_ورتیا)
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
#جے_انت
  وإلا
    أرجع FFI_BAD_ABI;
#إذا _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#جے_انت
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
ffi_prep_args64 (extended_cif *ecif, بنانشانی لما *ثابت ڈھیر)
{
  ثابت بنانشانی لما بائٹاں = ecif->cif->بائٹاں;
  ثابت بنانشانی لما flags = ecif->cif->flags;

  تعريفنوع اتحاد
  {
    حرف *c;
    بنانشانی لما *ul;
    عائم *f;
    مزدوج *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_ادھار;
  valp gpr_سرا;
  valp rest;
  valp اگلا_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_ادھار;
  بنانشانی صحيح fparg_گنتی;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_ادھار;
  بنانشانی صحيح vecarg_گنتی;

  بنانشانی صحيح i, الفاظ, nargs, nfixedargs;
  ffi_type **ptr;
  مزدوج double_tmp;
  اتحاد
  {
    فراغ **v;
    حرف **c;
    نشانیوالا حرف **sc;
    بنانشانی حرف **uc;
    نشانیوالا چھوٹا **ss;
    بنانشانی چھوٹا **us;
    نشانیوالا صحيح **si;
    بنانشانی صحيح **ui;
    بنانشانی لما **ul;
    عائم **f;
    مزدوج **d;
    float128 **f128;
  } p_argv;
  بنانشانی لما gprvalue;
  بنانشانی لما align;

  stacktop.c = (حرف *) ڈھیر + بائٹاں;
  gpr_ادھار.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - عدد_GPR_ARG_REGISTERS64;
  gpr_سرا.ul = gpr_ادھار.ul + عدد_GPR_ARG_REGISTERS64;
#إذا _CALL_ELF == 2
  rest.ul = ڈھیر + 4 + عدد_GPR_ARG_REGISTERS64;
#وإلا
  rest.ul = ڈھیر + 6 + عدد_GPR_ARG_REGISTERS64;
#جے_انت
  fpr_ادھار.d = gpr_ادھار.d - عدد_FPR_ARG_REGISTERS64;
  fparg_گنتی = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  إذا (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_ادھار.p = fpr_ادھار.p & ~0xF;
  وإلا
    vec_ادھار.p = gpr_ادھار.p;
  vec_ادھار.f128 -= عدد_VEC_ARG_REGISTERS64;
  vecarg_گنتی = 0;
  اگلا_arg.ul = gpr_ادھار.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((بنانشانی لما) (حرف *) ڈھیر & 0xF) == 0);
  FFI_ASSERT (((بنانشانی لما) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((بنانشانی لما) gpr_ادھار.c & 0xF) == 0);
  FFI_ASSERT (((بنانشانی لما) gpr_سرا.c  & 0xF) == 0);
  FFI_ASSERT (((بنانشانی لما) vec_ادھار.c & 0xF) == 0);
  FFI_ASSERT ((بائٹاں & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  إذا (flags & FLAG_RETVAL_REFERENCE)
    *اگلا_arg.ul++ = (بنانشانی لما) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#إذا _CALL_ELF != 2
  nfixedargs = (بنانشانی) -1;
  إذا ((flags & FLAG_COMPAT) == 0)
#جے_انت
    nfixedargs = ecif->cif->nfixedargs;
  لكل (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      بنانشانی صحيح elt, elnum;

      بدل ((*ptr)->type)
	{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, 16);
              إذا (اگلا_arg.ul == gpr_سرا.ul)
                اگلا_arg.ul = rest.ul;
              إذا (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64 && i < nfixedargs)
		حافظہ_نقل (vec_ادھار.f128++, *p_argv.f128, حجم (float128));
              وإلا
		حافظہ_نقل (اگلا_arg.f128, *p_argv.f128, حجم (float128));
              إذا (++اگلا_arg.f128 == gpr_سرا.f128)
                اگلا_arg.f128 = rest.f128;
              vecarg_گنتی++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              اكسر;
            }
	  إذا ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ادھار.d++ = double_tmp;
# إذا _CALL_ELF != 2
		  إذا ((flags & FLAG_COMPAT) != 0)
		    *اگلا_arg.d = double_tmp;
# endif
		}
	      وإلا
		*اگلا_arg.d = double_tmp;
	      إذا (++اگلا_arg.ul == gpr_سرا.ul)
		اگلا_arg.ul = rest.ul;
	      fparg_گنتی++;
	      double_tmp = (*p_argv.d)[1];
	      إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ادھار.d++ = double_tmp;
# إذا _CALL_ELF != 2
		  إذا ((flags & FLAG_COMPAT) != 0)
		    *اگلا_arg.d = double_tmp;
# endif
		}
	      وإلا
		*اگلا_arg.d = double_tmp;
	      إذا (++اگلا_arg.ul == gpr_سرا.ul)
		اگلا_arg.ul = rest.ul;
	      fparg_گنتی++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
	  /* Fall through.  */
#جے_انت
	حالة FFI_TYPE_DOUBLE:
#إذا _CALL_ELF != 2
	do_double:
#جے_انت
	  double_tmp = **p_argv.d;
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ادھار.d++ = double_tmp;
#إذا _CALL_ELF != 2
	      إذا ((flags & FLAG_COMPAT) != 0)
		*اگلا_arg.d = double_tmp;
#جے_انت
	    }
	  وإلا
	    *اگلا_arg.d = double_tmp;
	  إذا (++اگلا_arg.ul == gpr_سرا.ul)
	    اگلا_arg.ul = rest.ul;
	  fparg_گنتی++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
#إذا _CALL_ELF != 2
	do_float:
#جے_انت
	  double_tmp = **p_argv.f;
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ادھار.d++ = double_tmp;
#إذا _CALL_ELF != 2
	      إذا ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  اگلا_arg.f[1] = (عائم) double_tmp;
# وإلا
		  اگلا_arg.f[0] = (عائم) double_tmp;
# endif
		}
#جے_انت
	    }
	  وإلا
	    {
# ifndef __LITTLE_ENDIAN__
	      اگلا_arg.f[1] = (عائم) double_tmp;
# وإلا
	      اگلا_arg.f[0] = (عائم) double_tmp;
# endif
	    }
	  إذا (++اگلا_arg.ul == gpr_سرا.ul)
	    اگلا_arg.ul = rest.ul;
	  fparg_گنتی++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
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
	      بنانشانی صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, 16);
		  إذا (اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
		  إذا (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    حافظہ_نقل (vec_ادھار.f128++, cval + j, حجم (float128));
		  وإلا
		    حافظہ_نقل (اگلا_arg.f128, cval + j, حجم (float128));
		  إذا (++اگلا_arg.f128 == gpr_سرا.f128)
		    اگلا_arg.f128 = rest.f128;
		  vecarg_گنتی++;
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
	      بنانشانی صحيح j;
	      لكل (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ادھار.d++ = double_tmp;
		  وإلا
		    *اگلا_arg.d = double_tmp;
		  إذا (++اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      اكسر;
	    }
#جے_انت
	  إذا (elt == FFI_TYPE_FLOAT)
	    {
	      عائم *cval = (عائم *) *p_argv.v;
	      بنانشانی صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ادھار.d++ = double_tmp;
		  وإلا
		    {
# ifndef __LITTLE_ENDIAN__
		      اگلا_arg.f[1] = (عائم) double_tmp;
# وإلا
		      اگلا_arg.f[0] = (عائم) double_tmp;
# endif
		    }
		  إذا (++اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وإلا إذا (elt == FFI_TYPE_DOUBLE)
	    {
	      مزدوج *cval = (مزدوج *) *p_argv.v;
	      بنانشانی صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ادھار.d++ = double_tmp;
		  وإلا
		    *اگلا_arg.d = double_tmp;
		  إذا (++اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
		  fparg_گنتی++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وإلا
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      حرف *cval = (حرف *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->ناپ;
	      بنانشانی صحيح j;
	      لكل (j = 0; j < 2; j++)
		{
		  حرف *half = cval + j * hsize;
		  بنانشانی لما gprvalue;
		  بدل (elt)
		    {
		    حالة FFI_TYPE_UINT8:
		      gprvalue = *(بنانشانی حرف *) half; اكسر;
		    حالة FFI_TYPE_SINT8:
		      gprvalue = (بنانشانی لما) (لما) *(نشانیوالا حرف *) half;
		      اكسر;
		    حالة FFI_TYPE_UINT16:
		      gprvalue = *(بنانشانی چھوٹا *) half; اكسر;
		    حالة FFI_TYPE_SINT16:
		      gprvalue = (بنانشانی لما) (لما) *(نشانیوالا چھوٹا *) half;
		      اكسر;
		    حالة FFI_TYPE_UINT32:
		      gprvalue = *(بنانشانی صحيح *) half; اكسر;
		    حالة FFI_TYPE_INT:
		    حالة FFI_TYPE_SINT32:
		      gprvalue = (بنانشانی لما) (لما) *(نشانیوالا صحيح *) half;
		      اكسر;
		    حالة FFI_TYPE_SINT64:
		    حالة FFI_TYPE_UINT64:
		    حالة FFI_TYPE_اشارہ_گر:
		    افتراضي:
		      gprvalue = *(بنانشانی لما *) half; اكسر;
		    }
		  *اگلا_arg.ul++ = gprvalue;
		  إذا (اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
		}
	    }
	  اكسر;
#جے_انت

	حالة FFI_TYPE_STRUCT:
	  إذا ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      إذا (align > 16)
		align = 16;
	      إذا (align > 1)
                {
                  اگلا_arg.p = FFI_ALIGN (اگلا_arg.p, align);
                  إذا (اگلا_arg.ul == gpr_سرا.ul)
                    اگلا_arg.ul = rest.ul;
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
                      إذا (vecarg_گنتی < عدد_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        حافظہ_نقل (vec_ادھار.f128++, arg.f128++, حجم (float128));
                      وإلا
		        حافظہ_نقل (اگلا_arg.f128, arg.f128++, حجم (float128));
                      إذا (++اگلا_arg.f128 == gpr_سرا.f128)
                        اگلا_arg.f128 = rest.f128;
                      vecarg_گنتی++;
                    }
                  طالما (--elnum != 0);
                }
              وإلا
#جے_انت
	      إذا (elt == FFI_TYPE_FLOAT)
		{
		  افعل
		    {
		      double_tmp = *arg.f++;
		      إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_ادھار.d++ = double_tmp;
		      وإلا
			*اگلا_arg.f = (عائم) double_tmp;
		      إذا (++اگلا_arg.f == gpr_سرا.f)
			اگلا_arg.f = rest.f;
		      fparg_گنتی++;
		    }
		  طالما (--elnum != 0);
		  إذا ((اگلا_arg.p & 7) != 0)
                    إذا (++اگلا_arg.f == gpr_سرا.f)
                      اگلا_arg.f = rest.f;
		}
	      وإلا
		افعل
		  {
		    double_tmp = *arg.d++;
		    إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_ادھار.d++ = double_tmp;
		    وإلا
		      *اگلا_arg.d = double_tmp;
		    إذا (++اگلا_arg.d == gpr_سرا.d)
		      اگلا_arg.d = rest.d;
		    fparg_گنتی++;
		  }
		طالما (--elnum != 0);
#وإلا
	      إذا (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      وإلا
		اذهب do_double;
#جے_انت
	    }
	  وإلا
	    {
	      الفاظ = ((*ptr)->ناپ + 7) / 8;
	      إذا (اگلا_arg.ul >= gpr_ادھار.ul && اگلا_arg.ul + الفاظ > gpr_سرا.ul)
		{
		  size_t پہلا = gpr_سرا.c - اگلا_arg.c;
		  حافظہ_نقل (اگلا_arg.c, *p_argv.c, پہلا);
		  حافظہ_نقل (rest.c, *p_argv.c + پہلا, (*ptr)->ناپ - پہلا);
		  اگلا_arg.c = rest.c + الفاظ * 8 - پہلا;
		}
	      وإلا
		{
		  حرف *where = اگلا_arg.c;

#جے_بنا_تعریف __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  إذا ((*ptr)->ناپ < 8)
		    where += 8 - (*ptr)->ناپ;
#جے_انت
		  حافظہ_نقل (where, *p_argv.c, (*ptr)->ناپ);
		  اگلا_arg.ul += الفاظ;
		  إذا (اگلا_arg.ul == gpr_سرا.ul)
		    اگلا_arg.ul = rest.ul;
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
	حالة FFI_TYPE_اشارہ_گر:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *اگلا_arg.ul++ = gprvalue;
	  إذا (اگلا_arg.ul == gpr_سرا.ul)
	    اگلا_arg.ul = rest.ul;
	  اكسر;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (اگلا_arg.ul >= gpr_ادھار.ul
		  && اگلا_arg.ul <= gpr_ادھار.ul + 4));
}


#إذا _CALL_ELF == 2
#تعریف_کرو MIN_CACHE_سطر_ناپ 8

ساكن فراغ
flush_icache (حرف *wraddr, حرف *xaddr, صحيح ناپ)
{
  صحيح i;
  لكل (i = 0; i < ناپ; i += MIN_CACHE_سطر_ناپ)
    __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + ناپ - 1), "r"(wraddr + ناپ - 1)
		    : "memory");
}
#جے_انت


ffi_حالت FFI_HIDDEN
ffi_prep_گھیرا_loc_linux64 (ffi_گھیرا *گھیرا,
			      ffi_cif *cif,
			      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			      فراغ *user_انکڑے,
			      فراغ *codeloc)
{
  إذا (cif->abi < FFI_LINUX || cif->abi >= FFI_آخری_ABI)
    أرجع FFI_BAD_ABI;

#جے_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present(گھیرا))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_گھیرا_LINUX64;
      ffi_tramp_سیٹ_parms (گھیرا->ftramp, dest, گھیرا);
    }
  وإلا
#جے_انت
    {
#إذا _CALL_ELF == 2
      بنانشانی صحيح *tramp = (بنانشانی صحيح *) &گھیرا->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(فراغ **) &tramp[4] = (فراغ *) ffi_گھیرا_LINUX64;
      *(فراغ **) &tramp[6] = codeloc;
      flush_icache ((حرف *) tramp, (حرف *) codeloc, 4 * 4);
#وإلا
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      فراغ **tramp = (فراغ **) &گھیرا->tramp[0];
      حافظہ_نقل (&tramp[0], (فراغ **) ffi_گھیرا_LINUX64, حجم (فراغ *));
      tramp[1] = codeloc;
      حافظہ_نقل (&tramp[2], (فراغ **) ffi_گھیرا_LINUX64 + 1, حجم (فراغ *));
#جے_انت
    }

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}


صحيح FFI_HIDDEN
ffi_گھیرا_helper_LINUX64 (ffi_cif *cif,
			    فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			    فراغ *user_انکڑے,
			    فراغ *rvalue,
			    بنانشانی لما *pst,
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
  بنانشانی لما i, avn, nfixedargs;
  ffi_dblfl *سرا_pfr = pfr + عدد_FPR_ARG_REGISTERS64;
  float128 *سرا_pvec = pvec + عدد_VEC_ARG_REGISTERS64;
  بنانشانی لما align;

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
  nfixedargs = (بنانشانی) -1;
  إذا ((cif->flags & FLAG_COMPAT) == 0)
#جے_انت
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  طالما (i < avn)
    {
      بنانشانی صحيح elt, elnum;

      بدل (arg_types[i]->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
#جے_بنا_تعریف __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 7;
	  pst++;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
#جے_بنا_تعریف __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 6;
	  pst++;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
#جے_بنا_تعریف __LITTLE_ENDIAN__
	  avalue[i] = (حرف *) pst + 4;
	  pst++;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_اشارہ_گر:
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
		pst = (بنانشانی لما *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  إذا (elt)
	    {
#إذا _CALL_ELF == 2
	      اتحاد {
		فراغ *v;
		بنانشانی لما *ul;
		عائم *f;
		مزدوج *d;
		float128 *f128;
		size_t p;
	      } منزل, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              إذا (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  إذا (pvec + elnum <= سرا_pvec)
                    منزل.v = pvec;
                  وإلا
                    منزل.v = pst;
                }
              وإلا
#جے_انت
	      إذا (pfr + elnum <= سرا_pfr)
		منزل.v = pfr;
	      وإلا
		منزل.v = pst;

	      avalue[i] = منزل.v;
	      from.ul = pst;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              إذا (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  افعل
                    {
                      إذا (pvec < سرا_pvec && i < nfixedargs)
		        حافظہ_نقل (منزل.f128, pvec++, حجم (float128));
                      وإلا
		        حافظہ_نقل (منزل.f128, from.f128, حجم (float128));
                      منزل.f128++;
                      from.f128++;
                    }
                  طالما (--elnum != 0);
                }
              وإلا
#جے_انت
	      إذا (elt == FFI_TYPE_FLOAT)
		{
		  افعل
		    {
		      إذا (pfr < سرا_pfr && i < nfixedargs)
			{
			  *منزل.f = (عائم) pfr->d;
			  pfr++;
			}
		      وإلا
			*منزل.f = *from.f;
		      منزل.f++;
		      from.f++;
		    }
		  طالما (--elnum != 0);
		}
	      وإلا
		{
		  افعل
		    {
		      إذا (pfr < سرا_pfr && i < nfixedargs)
			{
			  *منزل.d = pfr->d;
			  pfr++;
			}
		      وإلا
			*منزل.d = *from.d;
		      منزل.d++;
		      from.d++;
		    }
		  طالما (--elnum != 0);
		}
#وإلا
	      إذا (elt == FFI_TYPE_FLOAT)
		اذهب do_float;
	      وإلا
		اذهب do_double;
#جے_انت
	    }
	  وإلا
	    {
#جے_بنا_تعریف __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      إذا (arg_types[i]->ناپ < 8)
		avalue[i] = (حرف *) pst + 8 - arg_types[i]->ناپ;
	      وإلا
#جے_انت
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->ناپ + 7) / 8;
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
          إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              إذا (((بنانشانی لما) pst & 0xF) != 0)
                ++pst;
              إذا (pvec < سرا_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              وإلا
                avalue[i] = pst;
              pst += 2;
              اكسر;
            }
          وإلا إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      إذا (pfr + 1 < سرا_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      وإلا
		{
		  إذا (pfr < سرا_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(بنانشانی لما *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      اكسر;
	    }
	  /* Fall through.  */
#جے_انت
	حالة FFI_TYPE_DOUBLE:
#إذا _CALL_ELF != 2
	do_double:
#جے_انت
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  إذا (pfr < سرا_pfr && i < nfixedargs)
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
#جے_انت
	  إذا (pfr < سرا_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (عائم) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    {
#جے_بنا_تعریف __LITTLE_ENDIAN__
	      avalue[i] = (حرف *) pst + 4;
#وإلا
	      avalue[i] = pst;
#جے_انت
	    }
	  pst++;
	  اكسر;

#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
	حالة FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    بنانشانی صحيح j;
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
		إذا (((بنانشانی لما) pst & 0xF) != 0)
		  ++pst;
		لكل (j = 0; j < 2; j++)
		  {
		    إذا (pvec < سرا_pvec && i < nfixedargs)
		      حافظہ_نقل (&cval[j], pvec++, حجم (float128));
		    وإلا
		      حافظہ_نقل (&cval[j], pst, حجم (float128));
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
		    إذا (pfr < سرا_pfr && i < nfixedargs)
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
#جے_انت
	    إذا (elt == FFI_TYPE_FLOAT)
	      {
		عائم *cval = alloca (2 * حجم (عائم));
		لكل (j = 0; j < 2; j++)
		  {
		    إذا (pfr < سرا_pfr && i < nfixedargs)
		      {
			cval[j] = (عائم) pfr->d;
			pfr++;
		      }
		    وإلا
		      {
#جے_بنا_تعریف __LITTLE_ENDIAN__
			cval[j] = ((عائم *) pst)[1];
#وإلا
			cval[j] = ((عائم *) pst)[0];
#جے_انت
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
		    إذا (pfr < سرا_pfr && i < nfixedargs)
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
		size_t hsize = arg_types[i]->elements[0]->ناپ;
		حرف *cval = alloca (2 * hsize);
		لكل (j = 0; j < 2; j++)
		  {
		    حرف *src;
#جے_بنا_تعریف __LITTLE_ENDIAN__
		    src = (حرف *) pst + (8 - hsize);
#وإلا
		    src = (حرف *) pst;
#جے_انت
		    حافظہ_نقل (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  اكسر;
#جے_انت

	افتراضي:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_انکڑے);

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
	أرجع PPC64_LD_سمتیہ;
      أرجع PPC_LD_F128;
#جے_انت
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
    حالة FFI_TYPE_اشارہ_گر:
      أرجع PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      أرجع PPC_LD_I64;
#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
    حالة FFI_TYPE_COMPLEX:
      {
	صحيح inner = cif->rtype->elements[0]->type;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	إذا (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    إذا ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      أرجع PPC64_LD_سمتیہ_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#جے_انت
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
	  بنانشانی لما re, im;
	  بدل (inner)
	    {
	    حالة FFI_TYPE_UINT8:
	      re = ((بنانشانی حرف *) rv)[0];
	      im = ((بنانشانی حرف *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT8:
	      re = (بنانشانی لما) (لما) ((نشانیوالا حرف *) rv)[0];
	      im = (بنانشانی لما) (لما) ((نشانیوالا حرف *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_UINT16:
	      re = ((بنانشانی چھوٹا *) rv)[0];
	      im = ((بنانشانی چھوٹا *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT16:
	      re = (بنانشانی لما) (لما) ((نشانیوالا چھوٹا *) rv)[0];
	      im = (بنانشانی لما) (لما) ((نشانیوالا چھوٹا *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_UINT32:
	      re = ((بنانشانی صحيح *) rv)[0];
	      im = ((بنانشانی صحيح *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT32:
	      re = (بنانشانی لما) (لما) ((نشانیوالا صحيح *) rv)[0];
	      im = (بنانشانی لما) (لما) ((نشانیوالا صحيح *) rv)[1];
	      اكسر;
	    حالة FFI_TYPE_SINT64:
	    حالة FFI_TYPE_UINT64:
	    حالة FFI_TYPE_اشارہ_گر:
	    افتراضي:
	      re = ((بنانشانی لما *) rv)[0];
	      im = ((بنانشانی لما *) rv)[1];
	      اكسر;
	    }
	  ((بنانشانی لما *) rv)[0] = re;
	  ((بنانشانی لما *) rv)[1] = im;
	  أرجع PPC_LD_R3R4;
	}
      }
#جے_انت
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
	      بدل (cif->rtype->ناپ)
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
	    أرجع PPC64_LD_سمتیہ_HOMOG;
	  إذا ((cif->flags & FLAG_RETURNS_64_بٹ) != 0)
	    أرجع PPC64_LD_DOUBLE_HOMOG;
	  أرجع PPC64_LD_FLOAT_HOMOG;
        }
      أرجع PPC_LD_NONE;
    }
  abort();
}
#جے_انت

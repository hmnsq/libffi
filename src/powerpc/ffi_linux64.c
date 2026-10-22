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

#అన్తర్భావ "ffi.h"
#అన్తర్భావ <tramp.h>
#అన్తర్భావ <stdlib.h>

#నిర్వచితమైతే POWERPC64
#అన్తర్భావ "ffi_common.h"
#అన్తర్భావ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
గణన {
  అంకె_GPR_ARG_REGISTERS64 = 8,
  అంకె_FPR_ARG_REGISTERS64 = 13,
  అంకె_VEC_ARG_REGISTERS64 = 12,
};
గణన { ASM_NEEDS_REGISTERS64 = 4 };


#ఒకవేళ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
శూన్య FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  ఒకవేళ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.కొలత = 8;
      ffi_type_longdouble.alignment = 8;
    }
  లేకపోతే
    {
      ffi_type_longdouble.కొలత = 16;
      ffi_type_longdouble.alignment = 16;
    }
#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.కొలత = 2 * ffi_type_longdouble.కొలత;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#అయితే_ముగింపు
}
#అయితే_ముగింపు


స్థిర గుర్తులేని పూర్ణాంకం
discover_homogeneous_aggregate (ffi_abi abi,
                                స్థిరరాశి ffi_type *t,
                                గుర్తులేని పూర్ణాంకం *elnum)
{
  ఎంపిక (t->type)
    {
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      ఒకవేళ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          తిరిగి FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      లేకపోతే ఒకవేళ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          తిరిగి FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#అయితే_ముగింపు
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
      *elnum = 1;
      తిరిగి (పూర్ణాంకం) t->type;

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
    స్థితి FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	గుర్తులేని పూర్ణాంకం inner_elnum = 0;
	గుర్తులేని పూర్ణాంకం inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	ఒకవేళ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    తిరిగి inner;
	  }
	తిరిగి 0;
      }
#అయితే_ముగింపు

    స్థితి FFI_TYPE_STRUCT:;
      {
	గుర్తులేని పూర్ణాంకం పునాది_elt = 0, మొత్తం_elnum = 0;
	ffi_type **el = t->elements;
	అయితే (*el)
	  {
	    గుర్తులేని పూర్ణాంకం el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    ఒకవేళ (el_elt == 0
		|| (పునాది_elt && పునాది_elt != el_elt))
	      తిరిగి 0;
	    పునాది_elt = el_elt;
	    మొత్తం_elnum += el_elnum;
#ఒకవేళ _CALL_ELF == 2
	    ఒకవేళ (మొత్తం_elnum > 8)
	      తిరిగి 0;
#లేకపోతే
	    ఒకవేళ (మొత్తం_elnum > 1)
	      తిరిగి 0;
#అయితే_ముగింపు
	    el++;
	  }
	*elnum = మొత్తం_elnum;
	తిరిగి పునాది_elt;
      }

    అప్రమేయ:
      తిరిగి 0;
    }
}


/* Perform machine dependent cif processing */
స్థిర ffi_స్థితి
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  గుర్తులేని బైట్లు;
  గుర్తులేని i, fparg_లెక్క = 0, intarg_లెక్క = 0, vecarg_లెక్క = 0;
  గుర్తులేని flags = cif->flags;
  గుర్తులేని elt, elnum, rtype;

#ఒకవేళ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    తిరిగి FFI_BAD_ABI;
#లేదావేళ !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    తిరిగి FFI_BAD_ABI;
#లేకపోతే
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#ఒకవేళ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  బైట్లు = (4 + ASM_NEEDS_REGISTERS64) * పరిమాణం (దీర్ఘ);

  /* Space for the general registers.  */
  బైట్లు += అంకె_GPR_ARG_REGISTERS64 * పరిమాణం (దీర్ఘ);
#లేకపోతే
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  బైట్లు = (6 + ASM_NEEDS_REGISTERS64) * పరిమాణం (దీర్ఘ);

  /* Space for the mandatory parm save area and general registers.  */
  బైట్లు += 2 * అంకె_GPR_ARG_REGISTERS64 * పరిమాణం (దీర్ఘ);
#అయితే_ముగింపు

  /* Return value handling.  */
  rtype = cif->rtype->type;
#ఒకవేళ _CALL_ELF == 2
homogeneous:
#అయితే_ముగింపు
  ఎంపిక (rtype)
    {
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ఆపు;
        }
      ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128_బిట్లు;
      /* Fall through.  */
#అయితే_ముగింపు
    స్థితి FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_బిట్లు;
      /* Fall through.  */
    స్థితి FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ఆపు;

    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_సూచిక:
      flags |= FLAG_RETURNS_64_బిట్లు;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
#ఒకవేళ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      ఒకవేళ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          వెళ్ళు homogeneous;
        }
      ఒకవేళ (cif->rtype->కొలత <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ఆపు;
        }
#అయితే_ముగింపు
      intarg_లెక్క++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    స్థితి FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ఆపు;

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
    స్థితి FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      ఎంపిక (rtype)
        {
        స్థితి FFI_TYPE_FLOAT:
        స్థితి FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          వెళ్ళు homogeneous;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        స్థితి FFI_TYPE_LONGDOUBLE:
          ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ఆపు;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          వెళ్ళు homogeneous;
#అయితే_ముగింపు
        స్థితి FFI_TYPE_INT:
        స్థితి FFI_TYPE_SINT8:  స్థితి FFI_TYPE_UINT8:
        స్థితి FFI_TYPE_SINT16: స్థితి FFI_TYPE_UINT16:
        స్థితి FFI_TYPE_SINT32: స్థితి FFI_TYPE_UINT32:
        స్థితి FFI_TYPE_SINT64: స్థితి FFI_TYPE_UINT64:
        స్థితి FFI_TYPE_సూచిక:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ఆపు;
        అప్రమేయ:
          తిరిగి FFI_BAD_TYPEDEF;
        }
      ఆపు;
#అయితే_ముగింపు

    అప్రమేయ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ఆపు;
    }

  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      గుర్తులేని పూర్ణాంకం align;

      ఎంపిక ((*ptr)->type)
	{
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
          ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_లెక్క++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_లెక్క = (intarg_లెక్క + 3) & ~0x1;
              ఒకవేళ (vecarg_లెక్క > అంకె_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ఆపు;
            }
	  ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_లెక్క++;
	      intarg_లెక్క++;
	    }
	  /* Fall through.  */
#అయితే_ముగింపు
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_FLOAT:
	  fparg_లెక్క++;
	  intarg_లెక్క++;
	  ఒకవేళ (fparg_లెక్క > అంకె_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  ఒకవేళ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ఒకవేళ (align > 16)
		align = 16;
	      align = align / 8;
	      ఒకవేళ (align > 1)
		intarg_లెక్క = FFI_ALIGN (intarg_లెక్క, align);
	    }
	  intarg_లెక్క += ((*ptr)->కొలత + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_లెక్క += elnum;
              ఒకవేళ (vecarg_లెక్క > అంకె_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ఆపు;
            }
	  లేకపోతే
#అయితే_ముగింపు
	  ఒకవేళ (elt)
	    {
	      fparg_లెక్క += elnum;
	      ఒకవేళ (fparg_లెక్క > అంకె_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  లేకపోతే
	    {
	      ఒకవేళ (intarg_లెక్క > అంకె_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ఆపు;

	స్థితి FFI_TYPE_సూచిక:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_లెక్క++;
	  ఒకవేళ (intarg_లెక్క > అంకె_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ఆపు;

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
	స్థితి FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#అయితే_ముగింపు
	  ఎంపిక (elt)
	    {
	    స్థితి FFI_TYPE_FLOAT:
	    స్థితి FFI_TYPE_DOUBLE:
	      fparg_లెక్క += 2;
	      intarg_లెక్క += 2;
	      ఒకవేళ (fparg_లెక్క > అంకె_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ఆపు;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    స్థితి FFI_TYPE_LONGDOUBLE:
	      ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_లెక్క += 2;
		  intarg_లెక్క = (intarg_లెక్క + 1) & ~0x1;
		  intarg_లెక్క += 4;
		  ఒకవేళ (vecarg_లెక్క > అంకె_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ఆపు;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_లెక్క += 4;
	      intarg_లెక్క += 4;
	      ఒకవేళ (fparg_లెక్క > అంకె_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ఆపు;
#అయితే_ముగింపు
	    స్థితి FFI_TYPE_INT:
	    స్థితి FFI_TYPE_SINT8:  స్థితి FFI_TYPE_UINT8:
	    స్థితి FFI_TYPE_SINT16: స్థితి FFI_TYPE_UINT16:
	    స్థితి FFI_TYPE_SINT32: స్థితి FFI_TYPE_UINT32:
	    స్థితి FFI_TYPE_SINT64: స్థితి FFI_TYPE_UINT64:
	    స్థితి FFI_TYPE_సూచిక:
	      intarg_లెక్క += 2;
	      ఆపు;
	    అప్రమేయ:
	      తిరిగి FFI_BAD_TYPEDEF;
	    }
	  ఒకవేళ (intarg_లెక్క > అంకె_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ఆపు;
#అయితే_ముగింపు

	అప్రమేయ:
	  FFI_ASSERT (0);
	}
    }

  ఒకవేళ (fparg_లెక్క != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ఒకవేళ (intarg_లెక్క > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ఒకవేళ (vecarg_లెక్క != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  ఒకవేళ (fparg_లెక్క != 0)
    బైట్లు += అంకె_FPR_ARG_REGISTERS64 * పరిమాణం (ద్విగుణ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  ఒకవేళ (vecarg_లెక్క != 0) {
    బైట్లు = (బైట్లు + 15) & ~0xF;
    బైట్లు += అంకె_VEC_ARG_REGISTERS64 * పరిమాణం (float128);
  }

  /* Stack space.  */
#ఒకవేళ _CALL_ELF == 2
  ఒకవేళ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    బైట్లు += intarg_లెక్క * పరిమాణం (దీర్ఘ);
#లేకపోతే
  ఒకవేళ (intarg_లెక్క > అంకె_GPR_ARG_REGISTERS64)
    బైట్లు += (intarg_లెక్క - అంకె_GPR_ARG_REGISTERS64) * పరిమాణం (దీర్ఘ);
#అయితే_ముగింపు

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  బైట్లు = (బైట్లు + 15) & ~0xF;

  cif->flags = flags;
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  ఒకవేళ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#ఒకవేళ _CALL_ELF != 2
  లేకపోతే ఒకవేళ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#అయితే_ముగింపు
  లేకపోతే
    తిరిగి FFI_BAD_ABI;
  తిరిగి ffi_prep_cif_linux64_core (cif);
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  గుర్తులేని పూర్ణాంకం nfixedargs,
			  గుర్తులేని పూర్ణాంకం ntotalargs MAYBE_వాడని)
{
  ఒకవేళ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#ఒకవేళ _CALL_ELF != 2
  లేకపోతే ఒకవేళ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#అయితే_ముగింపు
  లేకపోతే
    తిరిగి FFI_BAD_ABI;
#ఒకవేళ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#అయితే_ముగింపు
  తిరిగి ffi_prep_cif_linux64_core (cif);
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

శూన్య FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, గుర్తులేని దీర్ఘ *స్థిరరాశి దొంతర)
{
  స్థిరరాశి గుర్తులేని దీర్ఘ బైట్లు = ecif->cif->బైట్లు;
  స్థిరరాశి గుర్తులేని దీర్ఘ flags = ecif->cif->flags;

  రకనిర్వచనం సంఘం
  {
    అక్షరం *c;
    గుర్తులేని దీర్ఘ *ul;
    దశాంశం *f;
    ద్విగుణ *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_పునాది;
  valp gpr_చివర;
  valp rest;
  valp తదుపరి_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_పునాది;
  గుర్తులేని పూర్ణాంకం fparg_లెక్క;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_పునాది;
  గుర్తులేని పూర్ణాంకం vecarg_లెక్క;

  గుర్తులేని పూర్ణాంకం i, పదాలు, nargs, nfixedargs;
  ffi_type **ptr;
  ద్విగుణ double_tmp;
  సంఘం
  {
    శూన్య **v;
    అక్షరం **c;
    గుర్తుగల అక్షరం **sc;
    గుర్తులేని అక్షరం **uc;
    గుర్తుగల పొట్టి **ss;
    గుర్తులేని పొట్టి **us;
    గుర్తుగల పూర్ణాంకం **si;
    గుర్తులేని పూర్ణాంకం **ui;
    గుర్తులేని దీర్ఘ **ul;
    దశాంశం **f;
    ద్విగుణ **d;
    float128 **f128;
  } p_argv;
  గుర్తులేని దీర్ఘ gprvalue;
  గుర్తులేని దీర్ఘ align;

  stacktop.c = (అక్షరం *) దొంతర + బైట్లు;
  gpr_పునాది.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - అంకె_GPR_ARG_REGISTERS64;
  gpr_చివర.ul = gpr_పునాది.ul + అంకె_GPR_ARG_REGISTERS64;
#ఒకవేళ _CALL_ELF == 2
  rest.ul = దొంతర + 4 + అంకె_GPR_ARG_REGISTERS64;
#లేకపోతే
  rest.ul = దొంతర + 6 + అంకె_GPR_ARG_REGISTERS64;
#అయితే_ముగింపు
  fpr_పునాది.d = gpr_పునాది.d - అంకె_FPR_ARG_REGISTERS64;
  fparg_లెక్క = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  ఒకవేళ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_పునాది.p = fpr_పునాది.p & ~0xF;
  లేకపోతే
    vec_పునాది.p = gpr_పునాది.p;
  vec_పునాది.f128 -= అంకె_VEC_ARG_REGISTERS64;
  vecarg_లెక్క = 0;
  తదుపరి_arg.ul = gpr_పునాది.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) (అక్షరం *) దొంతర & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) gpr_పునాది.c & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) gpr_చివర.c  & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) vec_పునాది.c & 0xF) == 0);
  FFI_ASSERT ((బైట్లు & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  ఒకవేళ (flags & FLAG_RETVAL_REFERENCE)
    *తదుపరి_arg.ul++ = (గుర్తులేని దీర్ఘ) (అక్షరం *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#ఒకవేళ _CALL_ELF != 2
  nfixedargs = (గుర్తులేని) -1;
  ఒకవేళ ((flags & FLAG_COMPAT) == 0)
#అయితే_ముగింపు
    nfixedargs = ecif->cif->nfixedargs;
  కొరకు (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      గుర్తులేని పూర్ణాంకం elt, elnum;

      ఎంపిక ((*ptr)->type)
	{
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
          ఒకవేళ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              తదుపరి_arg.p = FFI_ALIGN (తదుపరి_arg.p, 16);
              ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
                తదుపరి_arg.ul = rest.ul;
              ఒకవేళ (vecarg_లెక్క < అంకె_VEC_ARG_REGISTERS64 && i < nfixedargs)
		జ్ఞాపకం_నకలు (vec_పునాది.f128++, *p_argv.f128, పరిమాణం (float128));
              లేకపోతే
		జ్ఞాపకం_నకలు (తదుపరి_arg.f128, *p_argv.f128, పరిమాణం (float128));
              ఒకవేళ (++తదుపరి_arg.f128 == gpr_చివర.f128)
                తదుపరి_arg.f128 = rest.f128;
              vecarg_లెక్క++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ఆపు;
            }
	  ఒకవేళ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_పునాది.d++ = double_tmp;
# ఒకవేళ _CALL_ELF != 2
		  ఒకవేళ ((flags & FLAG_COMPAT) != 0)
		    *తదుపరి_arg.d = double_tmp;
# endif
		}
	      లేకపోతే
		*తదుపరి_arg.d = double_tmp;
	      ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
		తదుపరి_arg.ul = rest.ul;
	      fparg_లెక్క++;
	      double_tmp = (*p_argv.d)[1];
	      ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_పునాది.d++ = double_tmp;
# ఒకవేళ _CALL_ELF != 2
		  ఒకవేళ ((flags & FLAG_COMPAT) != 0)
		    *తదుపరి_arg.d = double_tmp;
# endif
		}
	      లేకపోతే
		*తదుపరి_arg.d = double_tmp;
	      ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
		తదుపరి_arg.ul = rest.ul;
	      fparg_లెక్క++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ఆపు;
	    }
	  /* Fall through.  */
#అయితే_ముగింపు
	స్థితి FFI_TYPE_DOUBLE:
#ఒకవేళ _CALL_ELF != 2
	do_double:
#అయితే_ముగింపు
	  double_tmp = **p_argv.d;
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_పునాది.d++ = double_tmp;
#ఒకవేళ _CALL_ELF != 2
	      ఒకవేళ ((flags & FLAG_COMPAT) != 0)
		*తదుపరి_arg.d = double_tmp;
#అయితే_ముగింపు
	    }
	  లేకపోతే
	    *తదుపరి_arg.d = double_tmp;
	  ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
	    తదుపరి_arg.ul = rest.ul;
	  fparg_లెక్క++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
#ఒకవేళ _CALL_ELF != 2
	do_float:
#అయితే_ముగింపు
	  double_tmp = **p_argv.f;
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_పునాది.d++ = double_tmp;
#ఒకవేళ _CALL_ELF != 2
	      ఒకవేళ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  తదుపరి_arg.f[1] = (దశాంశం) double_tmp;
# లేకపోతే
		  తదుపరి_arg.f[0] = (దశాంశం) double_tmp;
# endif
		}
#అయితే_ముగింపు
	    }
	  లేకపోతే
	    {
# ifndef __LITTLE_ENDIAN__
	      తదుపరి_arg.f[1] = (దశాంశం) double_tmp;
# లేకపోతే
	      తదుపరి_arg.f[0] = (దశాంశం) double_tmp;
# endif
	    }
	  ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
	    తదుపరి_arg.ul = rest.ul;
	  fparg_లెక్క++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ఆపు;

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
	స్థితి FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      గుర్తులేని పూర్ణాంకం j;
	      కొరకు (j = 0; j < 2; j++)
		{
		  తదుపరి_arg.p = FFI_ALIGN (తదుపరి_arg.p, 16);
		  ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		  ఒకవేళ (vecarg_లెక్క < అంకె_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    జ్ఞాపకం_నకలు (vec_పునాది.f128++, cval + j, పరిమాణం (float128));
		  లేకపోతే
		    జ్ఞాపకం_నకలు (తదుపరి_arg.f128, cval + j, పరిమాణం (float128));
		  ఒకవేళ (++తదుపరి_arg.f128 == gpr_చివర.f128)
		    తదుపరి_arg.f128 = rest.f128;
		  vecarg_లెక్క++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ఆపు;
	    }
	  ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ద్విగుణ *cval = (ద్విగుణ *) *p_argv.v;
	      గుర్తులేని పూర్ణాంకం j;
	      కొరకు (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_పునాది.d++ = double_tmp;
		  లేకపోతే
		    *తదుపరి_arg.d = double_tmp;
		  ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		  fparg_లెక్క++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ఆపు;
	    }
#అయితే_ముగింపు
	  ఒకవేళ (elt == FFI_TYPE_FLOAT)
	    {
	      దశాంశం *cval = (దశాంశం *) *p_argv.v;
	      గుర్తులేని పూర్ణాంకం j;
	      కొరకు (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_పునాది.d++ = double_tmp;
		  లేకపోతే
		    {
# ifndef __LITTLE_ENDIAN__
		      తదుపరి_arg.f[1] = (దశాంశం) double_tmp;
# లేకపోతే
		      తదుపరి_arg.f[0] = (దశాంశం) double_tmp;
# endif
		    }
		  ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		  fparg_లెక్క++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  లేకపోతే ఒకవేళ (elt == FFI_TYPE_DOUBLE)
	    {
	      ద్విగుణ *cval = (ద్విగుణ *) *p_argv.v;
	      గుర్తులేని పూర్ణాంకం j;
	      కొరకు (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_పునాది.d++ = double_tmp;
		  లేకపోతే
		    *తదుపరి_arg.d = double_tmp;
		  ఒకవేళ (++తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		  fparg_లెక్క++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  లేకపోతే
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      అక్షరం *cval = (అక్షరం *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->కొలత;
	      గుర్తులేని పూర్ణాంకం j;
	      కొరకు (j = 0; j < 2; j++)
		{
		  అక్షరం *half = cval + j * hsize;
		  గుర్తులేని దీర్ఘ gprvalue;
		  ఎంపిక (elt)
		    {
		    స్థితి FFI_TYPE_UINT8:
		      gprvalue = *(గుర్తులేని అక్షరం *) half; ఆపు;
		    స్థితి FFI_TYPE_SINT8:
		      gprvalue = (గుర్తులేని దీర్ఘ) (దీర్ఘ) *(గుర్తుగల అక్షరం *) half;
		      ఆపు;
		    స్థితి FFI_TYPE_UINT16:
		      gprvalue = *(గుర్తులేని పొట్టి *) half; ఆపు;
		    స్థితి FFI_TYPE_SINT16:
		      gprvalue = (గుర్తులేని దీర్ఘ) (దీర్ఘ) *(గుర్తుగల పొట్టి *) half;
		      ఆపు;
		    స్థితి FFI_TYPE_UINT32:
		      gprvalue = *(గుర్తులేని పూర్ణాంకం *) half; ఆపు;
		    స్థితి FFI_TYPE_INT:
		    స్థితి FFI_TYPE_SINT32:
		      gprvalue = (గుర్తులేని దీర్ఘ) (దీర్ఘ) *(గుర్తుగల పూర్ణాంకం *) half;
		      ఆపు;
		    స్థితి FFI_TYPE_SINT64:
		    స్థితి FFI_TYPE_UINT64:
		    స్థితి FFI_TYPE_సూచిక:
		    అప్రమేయ:
		      gprvalue = *(గుర్తులేని దీర్ఘ *) half; ఆపు;
		    }
		  *తదుపరి_arg.ul++ = gprvalue;
		  ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		}
	    }
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_STRUCT:
	  ఒకవేళ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ఒకవేళ (align > 16)
		align = 16;
	      ఒకవేళ (align > 1)
                {
                  తదుపరి_arg.p = FFI_ALIGN (తదుపరి_arg.p, align);
                  ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
                    తదుపరి_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  ఒకవేళ (elt)
	    {
#ఒకవేళ _CALL_ELF == 2
	      సంఘం {
		శూన్య *v;
		దశాంశం *f;
		ద్విగుణ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  చేయి
                    {
                      ఒకవేళ (vecarg_లెక్క < అంకె_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        జ్ఞాపకం_నకలు (vec_పునాది.f128++, arg.f128++, పరిమాణం (float128));
                      లేకపోతే
		        జ్ఞాపకం_నకలు (తదుపరి_arg.f128, arg.f128++, పరిమాణం (float128));
                      ఒకవేళ (++తదుపరి_arg.f128 == gpr_చివర.f128)
                        తదుపరి_arg.f128 = rest.f128;
                      vecarg_లెక్క++;
                    }
                  అయితే (--elnum != 0);
                }
              లేకపోతే
#అయితే_ముగింపు
	      ఒకవేళ (elt == FFI_TYPE_FLOAT)
		{
		  చేయి
		    {
		      double_tmp = *arg.f++;
		      ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_పునాది.d++ = double_tmp;
		      లేకపోతే
			*తదుపరి_arg.f = (దశాంశం) double_tmp;
		      ఒకవేళ (++తదుపరి_arg.f == gpr_చివర.f)
			తదుపరి_arg.f = rest.f;
		      fparg_లెక్క++;
		    }
		  అయితే (--elnum != 0);
		  ఒకవేళ ((తదుపరి_arg.p & 7) != 0)
                    ఒకవేళ (++తదుపరి_arg.f == gpr_చివర.f)
                      తదుపరి_arg.f = rest.f;
		}
	      లేకపోతే
		చేయి
		  {
		    double_tmp = *arg.d++;
		    ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_పునాది.d++ = double_tmp;
		    లేకపోతే
		      *తదుపరి_arg.d = double_tmp;
		    ఒకవేళ (++తదుపరి_arg.d == gpr_చివర.d)
		      తదుపరి_arg.d = rest.d;
		    fparg_లెక్క++;
		  }
		అయితే (--elnum != 0);
#లేకపోతే
	      ఒకవేళ (elt == FFI_TYPE_FLOAT)
		వెళ్ళు do_float;
	      లేకపోతే
		వెళ్ళు do_double;
#అయితే_ముగింపు
	    }
	  లేకపోతే
	    {
	      పదాలు = ((*ptr)->కొలత + 7) / 8;
	      ఒకవేళ (తదుపరి_arg.ul >= gpr_పునాది.ul && తదుపరి_arg.ul + పదాలు > gpr_చివర.ul)
		{
		  size_t మొదటి = gpr_చివర.c - తదుపరి_arg.c;
		  జ్ఞాపకం_నకలు (తదుపరి_arg.c, *p_argv.c, మొదటి);
		  జ్ఞాపకం_నకలు (rest.c, *p_argv.c + మొదటి, (*ptr)->కొలత - మొదటి);
		  తదుపరి_arg.c = rest.c + పదాలు * 8 - మొదటి;
		}
	      లేకపోతే
		{
		  అక్షరం *where = తదుపరి_arg.c;

#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  ఒకవేళ ((*ptr)->కొలత < 8)
		    where += 8 - (*ptr)->కొలత;
#అయితే_ముగింపు
		  జ్ఞాపకం_నకలు (where, *p_argv.c, (*ptr)->కొలత);
		  తదుపరి_arg.ul += పదాలు;
		  ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
		    తదుపరి_arg.ul = rest.ul;
		}
	    }
	  ఆపు;

	స్థితి FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  వెళ్ళు putgpr;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_సూచిక:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *తదుపరి_arg.ul++ = gprvalue;
	  ఒకవేళ (తదుపరి_arg.ul == gpr_చివర.ul)
	    తదుపరి_arg.ul = rest.ul;
	  ఆపు;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (తదుపరి_arg.ul >= gpr_పునాది.ul
		  && తదుపరి_arg.ul <= gpr_పునాది.ul + 4));
}


#ఒకవేళ _CALL_ELF == 2
#నిర్వచించు MIN_CACHE_పంక్తి_కొలత 8

స్థిర శూన్య
flush_icache (అక్షరం *wraddr, అక్షరం *xaddr, పూర్ణాంకం కొలత)
{
  పూర్ణాంకం i;
  కొరకు (i = 0; i < కొలత; i += MIN_CACHE_పంక్తి_కొలత)
    __asm__ అస్థిర ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ అస్థిర ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + కొలత - 1), "r"(wraddr + కొలత - 1)
		    : "memory");
}
#అయితే_ముగింపు


ffi_స్థితి FFI_HIDDEN
ffi_prep_సంవృతం_loc_linux64 (ffi_సంవృతం *సంవృతం,
			      ffi_cif *cif,
			      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
			      శూన్య *user_దత్తాంశం,
			      శూన్య *codeloc)
{
  ఒకవేళ (cif->abi < FFI_LINUX || cif->abi >= FFI_చివరి_ABI)
    తిరిగి FFI_BAD_ABI;

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      శూన్య (*dest)(శూన్య) = ffi_సంవృతం_LINUX64;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
    }
  లేకపోతే
#అయితే_ముగింపు
    {
#ఒకవేళ _CALL_ELF == 2
      గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(శూన్య **) &tramp[4] = (శూన్య *) ffi_సంవృతం_LINUX64;
      *(శూన్య **) &tramp[6] = codeloc;
      flush_icache ((అక్షరం *) tramp, (అక్షరం *) codeloc, 4 * 4);
#లేకపోతే
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      శూన్య **tramp = (శూన్య **) &సంవృతం->tramp[0];
      జ్ఞాపకం_నకలు (&tramp[0], (శూన్య **) ffi_సంవృతం_LINUX64, పరిమాణం (శూన్య *));
      tramp[1] = codeloc;
      జ్ఞాపకం_నకలు (&tramp[2], (శూన్య **) ffi_సంవృతం_LINUX64 + 1, పరిమాణం (శూన్య *));
#అయితే_ముగింపు
    }

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}


పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_helper_LINUX64 (ffi_cif *cif,
			    శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
			    శూన్య *user_దత్తాంశం,
			    శూన్య *rvalue,
			    గుర్తులేని దీర్ఘ *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  శూన్య **avalue;
  ffi_type **arg_types;
  గుర్తులేని దీర్ఘ i, avn, nfixedargs;
  ffi_dblfl *చివర_pfr = pfr + అంకె_FPR_ARG_REGISTERS64;
  float128 *చివర_pvec = pvec + అంకె_VEC_ARG_REGISTERS64;
  గుర్తులేని దీర్ఘ align;

  avalue = alloca (cif->nargs * పరిమాణం (శూన్య *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (శూన్య *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#ఒకవేళ _CALL_ELF != 2
  nfixedargs = (గుర్తులేని) -1;
  ఒకవేళ ((cif->flags & FLAG_COMPAT) == 0)
#అయితే_ముగింపు
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  అయితే (i < avn)
    {
      గుర్తులేని పూర్ణాంకం elt, elnum;

      ఎంపిక (arg_types[i]->type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	  avalue[i] = (అక్షరం *) pst + 7;
	  pst++;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	  avalue[i] = (అక్షరం *) pst + 6;
	  pst++;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	  avalue[i] = (అక్షరం *) pst + 4;
	  pst++;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = pst;
	  pst++;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  ఒకవేళ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      ఒకవేళ (align > 16)
		align = 16;
	      ఒకవేళ (align > 1)
		pst = (గుర్తులేని దీర్ఘ *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  ఒకవేళ (elt)
	    {
#ఒకవేళ _CALL_ELF == 2
	      సంఘం {
		శూన్య *v;
		గుర్తులేని దీర్ఘ *ul;
		దశాంశం *f;
		ద్విగుణ *d;
		float128 *f128;
		size_t p;
	      } గమ్యం, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ఒకవేళ (pvec + elnum <= చివర_pvec)
                    గమ్యం.v = pvec;
                  లేకపోతే
                    గమ్యం.v = pst;
                }
              లేకపోతే
#అయితే_ముగింపు
	      ఒకవేళ (pfr + elnum <= చివర_pfr)
		గమ్యం.v = pfr;
	      లేకపోతే
		గమ్యం.v = pst;

	      avalue[i] = గమ్యం.v;
	      from.ul = pst;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  చేయి
                    {
                      ఒకవేళ (pvec < చివర_pvec && i < nfixedargs)
		        జ్ఞాపకం_నకలు (గమ్యం.f128, pvec++, పరిమాణం (float128));
                      లేకపోతే
		        జ్ఞాపకం_నకలు (గమ్యం.f128, from.f128, పరిమాణం (float128));
                      గమ్యం.f128++;
                      from.f128++;
                    }
                  అయితే (--elnum != 0);
                }
              లేకపోతే
#అయితే_ముగింపు
	      ఒకవేళ (elt == FFI_TYPE_FLOAT)
		{
		  చేయి
		    {
		      ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
			{
			  *గమ్యం.f = (దశాంశం) pfr->d;
			  pfr++;
			}
		      లేకపోతే
			*గమ్యం.f = *from.f;
		      గమ్యం.f++;
		      from.f++;
		    }
		  అయితే (--elnum != 0);
		}
	      లేకపోతే
		{
		  చేయి
		    {
		      ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
			{
			  *గమ్యం.d = pfr->d;
			  pfr++;
			}
		      లేకపోతే
			*గమ్యం.d = *from.d;
		      గమ్యం.d++;
		      from.d++;
		    }
		  అయితే (--elnum != 0);
		}
#లేకపోతే
	      ఒకవేళ (elt == FFI_TYPE_FLOAT)
		వెళ్ళు do_float;
	      లేకపోతే
		వెళ్ళు do_double;
#అయితే_ముగింపు
	    }
	  లేకపోతే
	    {
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      ఒకవేళ (arg_types[i]->కొలత < 8)
		avalue[i] = (అక్షరం *) pst + 8 - arg_types[i]->కొలత;
	      లేకపోతే
#అయితే_ముగింపు
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->కొలత + 7) / 8;
	  ఆపు;

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
          ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ఒకవేళ (((గుర్తులేని దీర్ఘ) pst & 0xF) != 0)
                ++pst;
              ఒకవేళ (pvec < చివర_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              లేకపోతే
                avalue[i] = pst;
              pst += 2;
              ఆపు;
            }
          లేకపోతే ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      ఒకవేళ (pfr + 1 < చివర_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      లేకపోతే
		{
		  ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(గుర్తులేని దీర్ఘ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ఆపు;
	    }
	  /* Fall through.  */
#అయితే_ముగింపు
	స్థితి FFI_TYPE_DOUBLE:
#ఒకవేళ _CALL_ELF != 2
	do_double:
#అయితే_ముగింపు
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  లేకపోతే
	    avalue[i] = pst;
	  pst++;
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
#ఒకవేళ _CALL_ELF != 2
	do_float:
#అయితే_ముగింపు
	  ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (దశాంశం) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  లేకపోతే
	    {
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	      avalue[i] = (అక్షరం *) pst + 4;
#లేకపోతే
	      avalue[i] = pst;
#అయితే_ముగింపు
	    }
	  pst++;
	  ఆపు;

#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
	స్థితి FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    గుర్తులేని పూర్ణాంకం j;
	    elt = arg_types[i]->elements[0]->type;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * పరిమాణం (float128));
		ఒకవేళ (((గుర్తులేని దీర్ఘ) pst & 0xF) != 0)
		  ++pst;
		కొరకు (j = 0; j < 2; j++)
		  {
		    ఒకవేళ (pvec < చివర_pvec && i < nfixedargs)
		      జ్ఞాపకం_నకలు (&cval[j], pvec++, పరిమాణం (float128));
		    లేకపోతే
		      జ్ఞాపకం_నకలు (&cval[j], pst, పరిమాణం (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ఆపు;
	      }
	    ఒకవేళ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ద్విగుణ *cval = alloca (4 * పరిమాణం (ద్విగుణ));
		కొరకు (j = 0; j < 4; j++)
		  {
		    ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    లేకపోతే
		      cval[j] = *(ద్విగుణ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ఆపు;
	      }
#అయితే_ముగింపు
	    ఒకవేళ (elt == FFI_TYPE_FLOAT)
	      {
		దశాంశం *cval = alloca (2 * పరిమాణం (దశాంశం));
		కొరకు (j = 0; j < 2; j++)
		  {
		    ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
		      {
			cval[j] = (దశాంశం) pfr->d;
			pfr++;
		      }
		    లేకపోతే
		      {
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
			cval[j] = ((దశాంశం *) pst)[1];
#లేకపోతే
			cval[j] = ((దశాంశం *) pst)[0];
#అయితే_ముగింపు
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    లేకపోతే ఒకవేళ (elt == FFI_TYPE_DOUBLE)
	      {
		ద్విగుణ *cval = alloca (2 * పరిమాణం (ద్విగుణ));
		కొరకు (j = 0; j < 2; j++)
		  {
		    ఒకవేళ (pfr < చివర_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    లేకపోతే
		      cval[j] = *(ద్విగుణ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    లేకపోతే
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->కొలత;
		అక్షరం *cval = alloca (2 * hsize);
		కొరకు (j = 0; j < 2; j++)
		  {
		    అక్షరం *src;
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
		    src = (అక్షరం *) pst + (8 - hsize);
#లేకపోతే
		    src = (అక్షరం *) pst;
#అయితే_ముగింపు
		    జ్ఞాపకం_నకలు (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ఆపు;
#అయితే_ముగింపు

	అప్రమేయ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      తిరిగి PPC_LD_NONE;
    స్థితి FFI_TYPE_FLOAT:
      తిరిగి PPC_LD_F32;
    స్థితి FFI_TYPE_DOUBLE:
      తిరిగి PPC_LD_F64;
#ఒకవేళ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      ఒకవేళ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	తిరిగి PPC64_LD_సదిశ;
      తిరిగి PPC_LD_F128;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_UINT8:
      తిరిగి PPC_LD_U8;
    స్థితి FFI_TYPE_SINT8:
      తిరిగి PPC_LD_S8;
    స్థితి FFI_TYPE_UINT16:
      తిరిగి PPC_LD_U16;
    స్థితి FFI_TYPE_SINT16:
      తిరిగి PPC_LD_S16;
    స్థితి FFI_TYPE_UINT32:
      తిరిగి PPC_LD_U32;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      తిరిగి PPC_LD_S32;
    స్థితి FFI_TYPE_సూచిక:
      తిరిగి PPC_LD_PTR;
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      తిరిగి PPC_LD_I64;
#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
    స్థితి FFI_TYPE_COMPLEX:
      {
	పూర్ణాంకం inner = cif->rtype->elements[0]->type;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ఒకవేళ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    ఒకవేళ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      తిరిగి PPC64_LD_సదిశ_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#అయితే_ముగింపు
	ఒకవేళ (inner == FFI_TYPE_FLOAT)
	  తిరిగి PPC64_LD_FLOAT_HOMOG;
	ఒకవేళ (inner == FFI_TYPE_DOUBLE)
	  తిరిగి PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  అక్షరం *rv = rvalue;
	  గుర్తులేని దీర్ఘ re, im;
	  ఎంపిక (inner)
	    {
	    స్థితి FFI_TYPE_UINT8:
	      re = ((గుర్తులేని అక్షరం *) rv)[0];
	      im = ((గుర్తులేని అక్షరం *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_SINT8:
	      re = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల అక్షరం *) rv)[0];
	      im = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల అక్షరం *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_UINT16:
	      re = ((గుర్తులేని పొట్టి *) rv)[0];
	      im = ((గుర్తులేని పొట్టి *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_SINT16:
	      re = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల పొట్టి *) rv)[0];
	      im = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల పొట్టి *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_UINT32:
	      re = ((గుర్తులేని పూర్ణాంకం *) rv)[0];
	      im = ((గుర్తులేని పూర్ణాంకం *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_INT:
	    స్థితి FFI_TYPE_SINT32:
	      re = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల పూర్ణాంకం *) rv)[0];
	      im = (గుర్తులేని దీర్ఘ) (దీర్ఘ) ((గుర్తుగల పూర్ణాంకం *) rv)[1];
	      ఆపు;
	    స్థితి FFI_TYPE_SINT64:
	    స్థితి FFI_TYPE_UINT64:
	    స్థితి FFI_TYPE_సూచిక:
	    అప్రమేయ:
	      re = ((గుర్తులేని దీర్ఘ *) rv)[0];
	      im = ((గుర్తులేని దీర్ఘ *) rv)[1];
	      ఆపు;
	    }
	  ((గుర్తులేని దీర్ఘ *) rv)[0] = re;
	  ((గుర్తులేని దీర్ఘ *) rv)[1] = im;
	  తిరిగి PPC_LD_R3R4;
	}
      }
#అయితే_ముగింపు
    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  ఒకవేళ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      ఎంపిక (cif->rtype->కొలత)
		{
		స్థితి 0:
		  తిరిగి PPC_LD_NONE;
		స్థితి 1:
		  తిరిగి PPC_LD_U8;
		స్థితి 2:
		  తిరిగి PPC_LD_U16;
		స్థితి 3:
		  తిరిగి PPC64_LD_STRUCT_3;
		స్థితి 4:
		  తిరిగి PPC_LD_U32;
		స్థితి 5:
		  తిరిగి PPC64_LD_STRUCT_5;
		స్థితి 6:
		  తిరిగి PPC64_LD_STRUCT_6;
		స్థితి 7:
		  తిరిగి PPC64_LD_STRUCT_7;
		స్థితి 8 ... 16:
		  తిరిగి PPC_LD_R3R4;
		}
	      ఆపు;
	    }
	  ఒకవేళ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    తిరిగి PPC64_LD_సదిశ_HOMOG;
	  ఒకవేళ ((cif->flags & FLAG_RETURNS_64_బిట్లు) != 0)
	    తిరిగి PPC64_LD_DOUBLE_HOMOG;
	  తిరిగి PPC64_LD_FLOAT_HOMOG;
        }
      తిరిగి PPC_LD_NONE;
    }
  abort();
}
#అయితే_ముగింపు

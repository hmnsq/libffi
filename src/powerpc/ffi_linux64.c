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

#अन्तर्भाव "ffi.h"
#अन्तर्भाव <tramp.h>
#अन्तर्भाव <stdlib.h>

#ifdef POWERPC64
#अन्तर्भाव "ffi_common.h"
#अन्तर्भाव "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
गणना {
  अंक_GPR_ARG_REGISTERS64 = 8,
  अंक_FPR_ARG_REGISTERS64 = 13,
  अंक_VEC_ARG_REGISTERS64 = 12,
};
गणना { ASM_NEEDS_REGISTERS64 = 4 };


#यदि HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  यदि ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  अन्यथा
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


स्थिर अचिन्हित पूर्णाङ्क
discover_homogeneous_aggregate (ffi_abi abi,
                                ध्रुव ffi_type *t,
                                अचिन्हित पूर्णाङ्क *elnum)
{
  विकल्प (t->type)
    {
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      यदि ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          प्रत्यागम FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      अन्यथा यदि ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          प्रत्यागम FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
      *elnum = 1;
      प्रत्यागम (पूर्णाङ्क) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    पक्ष FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	अचिन्हित पूर्णाङ्क inner_elnum = 0;
	अचिन्हित पूर्णाङ्क inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	यदि (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    प्रत्यागम inner;
	  }
	प्रत्यागम 0;
      }
#endif

    पक्ष FFI_TYPE_STRUCT:;
      {
	अचिन्हित पूर्णाङ्क base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	यावत् (*el)
	  {
	    अचिन्हित पूर्णाङ्क el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    यदि (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      प्रत्यागम 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#यदि _CALL_ELF == 2
	    यदि (total_elnum > 8)
	      प्रत्यागम 0;
#अन्यथा
	    यदि (total_elnum > 1)
	      प्रत्यागम 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	प्रत्यागम base_elt;
      }

    स्वभाव:
      प्रत्यागम 0;
    }
}


/* Perform machine dependent cif processing */
स्थिर ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित i, fparg_गिनती = 0, intarg_गिनती = 0, vecarg_गिनती = 0;
  अचिन्हित flags = cif->flags;
  अचिन्हित elt, elnum, rtype;

#यदि FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    प्रत्यागम FFI_BAD_ABI;
#नोचेद्यदि !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    प्रत्यागम FFI_BAD_ABI;
#अन्यथा
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    प्रत्यागम FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#यदि _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * परिमाण (दीर्घ);

  /* Space for the general registers.  */
  bytes += अंक_GPR_ARG_REGISTERS64 * परिमाण (दीर्घ);
#अन्यथा
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * परिमाण (दीर्घ);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * अंक_GPR_ARG_REGISTERS64 * परिमाण (दीर्घ);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#यदि _CALL_ELF == 2
homogeneous:
#endif
  विकल्प (rtype)
    {
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          भङ्ग;
        }
      यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    पक्ष FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    पक्ष FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      भङ्ग;

    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
#यदि _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      यदि (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          गच्छ homogeneous;
        }
      यदि (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          भङ्ग;
        }
#endif
      intarg_गिनती++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    पक्ष FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      भङ्ग;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    पक्ष FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      विकल्प (rtype)
        {
        पक्ष FFI_TYPE_FLOAT:
        पक्ष FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          गच्छ homogeneous;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        पक्ष FFI_TYPE_LONGDOUBLE:
          यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              भङ्ग;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          गच्छ homogeneous;
#endif
        पक्ष FFI_TYPE_INT:
        पक्ष FFI_TYPE_SINT8:  पक्ष FFI_TYPE_UINT8:
        पक्ष FFI_TYPE_SINT16: पक्ष FFI_TYPE_UINT16:
        पक्ष FFI_TYPE_SINT32: पक्ष FFI_TYPE_UINT32:
        पक्ष FFI_TYPE_SINT64: पक्ष FFI_TYPE_UINT64:
        पक्ष FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          भङ्ग;
        स्वभाव:
          प्रत्यागम FFI_BAD_TYPEDEF;
        }
      भङ्ग;
#endif

    स्वभाव:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      भङ्ग;
    }

  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित पूर्णाङ्क align;

      विकल्प ((*ptr)->type)
	{
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
          यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_गिनती++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_गिनती = (intarg_गिनती + 3) & ~0x1;
              यदि (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              भङ्ग;
            }
	  यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_गिनती++;
	      intarg_गिनती++;
	    }
	  /* Fall through.  */
#endif
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_FLOAT:
	  fparg_गिनती++;
	  intarg_गिनती++;
	  यदि (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  यदि ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      यदि (align > 16)
		align = 16;
	      align = align / 8;
	      यदि (align > 1)
		intarg_गिनती = FFI_ALIGN (intarg_गिनती, align);
	    }
	  intarg_गिनती += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          यदि (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_गिनती += elnum;
              यदि (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              भङ्ग;
            }
	  अन्यथा
#endif
	  यदि (elt)
	    {
	      fparg_गिनती += elnum;
	      यदि (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  अन्यथा
	    {
	      यदि (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_POINTER:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_गिनती++;
	  यदि (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  भङ्ग;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	पक्ष FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  यदि (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  विकल्प (elt)
	    {
	    पक्ष FFI_TYPE_FLOAT:
	    पक्ष FFI_TYPE_DOUBLE:
	      fparg_गिनती += 2;
	      intarg_गिनती += 2;
	      यदि (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      भङ्ग;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    पक्ष FFI_TYPE_LONGDOUBLE:
	      यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_गिनती += 2;
		  intarg_गिनती = (intarg_गिनती + 1) & ~0x1;
		  intarg_गिनती += 4;
		  यदि (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  भङ्ग;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_गिनती += 4;
	      intarg_गिनती += 4;
	      यदि (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      भङ्ग;
#endif
	    पक्ष FFI_TYPE_INT:
	    पक्ष FFI_TYPE_SINT8:  पक्ष FFI_TYPE_UINT8:
	    पक्ष FFI_TYPE_SINT16: पक्ष FFI_TYPE_UINT16:
	    पक्ष FFI_TYPE_SINT32: पक्ष FFI_TYPE_UINT32:
	    पक्ष FFI_TYPE_SINT64: पक्ष FFI_TYPE_UINT64:
	    पक्ष FFI_TYPE_POINTER:
	      intarg_गिनती += 2;
	      भङ्ग;
	    स्वभाव:
	      प्रत्यागम FFI_BAD_TYPEDEF;
	    }
	  यदि (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  भङ्ग;
#endif

	स्वभाव:
	  FFI_ASSERT (0);
	}
    }

  यदि (fparg_गिनती != 0)
    flags |= FLAG_FP_ARGUMENTS;
  यदि (intarg_गिनती > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  यदि (vecarg_गिनती != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  यदि (fparg_गिनती != 0)
    bytes += अंक_FPR_ARG_REGISTERS64 * परिमाण (द्विगुण);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  यदि (vecarg_गिनती != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += अंक_VEC_ARG_REGISTERS64 * परिमाण (float128);
  }

  /* Stack space.  */
#यदि _CALL_ELF == 2
  यदि ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_गिनती * परिमाण (दीर्घ);
#अन्यथा
  यदि (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
    bytes += (intarg_गिनती - अंक_GPR_ARG_REGISTERS64) * परिमाण (दीर्घ);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  प्रत्यागम FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  यदि ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#यदि _CALL_ELF != 2
  अन्यथा यदि (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  अन्यथा
    प्रत्यागम FFI_BAD_ABI;
  प्रत्यागम ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  अचिन्हित पूर्णाङ्क nfixedargs,
			  अचिन्हित पूर्णाङ्क ntotalargs MAYBE_UNUSED)
{
  यदि ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#यदि _CALL_ELF != 2
  अन्यथा यदि (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  अन्यथा
    प्रत्यागम FFI_BAD_ABI;
#यदि _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  प्रत्यागम ffi_prep_cif_linux64_core (cif);
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

शून्य FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, अचिन्हित दीर्घ *ध्रुव ढेर)
{
  ध्रुव अचिन्हित दीर्घ bytes = ecif->cif->bytes;
  ध्रुव अचिन्हित दीर्घ flags = ecif->cif->flags;

  प्रकारनिर्देश सङ्घ
  {
    वर्ण *c;
    अचिन्हित दीर्घ *ul;
    भिन्न *f;
    द्विगुण *d;
    float128 *f128;
    माप_प्रकार p;
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
  अचिन्हित पूर्णाङ्क fparg_गिनती;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  अचिन्हित पूर्णाङ्क vecarg_गिनती;

  अचिन्हित पूर्णाङ्क i, शब्दगण, nargs, nfixedargs;
  ffi_type **ptr;
  द्विगुण double_tmp;
  सङ्घ
  {
    शून्य **v;
    वर्ण **c;
    चिन्हित वर्ण **sc;
    अचिन्हित वर्ण **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    चिन्हित पूर्णाङ्क **si;
    अचिन्हित पूर्णाङ्क **ui;
    अचिन्हित दीर्घ **ul;
    भिन्न **f;
    द्विगुण **d;
    float128 **f128;
  } p_argv;
  अचिन्हित दीर्घ gprvalue;
  अचिन्हित दीर्घ align;

  stacktop.c = (वर्ण *) ढेर + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - अंक_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + अंक_GPR_ARG_REGISTERS64;
#यदि _CALL_ELF == 2
  rest.ul = ढेर + 4 + अंक_GPR_ARG_REGISTERS64;
#अन्यथा
  rest.ul = ढेर + 6 + अंक_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - अंक_FPR_ARG_REGISTERS64;
  fparg_गिनती = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  यदि (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  अन्यथा
    vec_base.p = gpr_base.p;
  vec_base.f128 -= अंक_VEC_ARG_REGISTERS64;
  vecarg_गिनती = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिन्हित दीर्घ) (वर्ण *) ढेर & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  यदि (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (अचिन्हित दीर्घ) (वर्ण *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#यदि _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  यदि ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  प्रति (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      अचिन्हित पूर्णाङ्क elt, elnum;

      विकल्प ((*ptr)->type)
	{
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
          यदि ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              यदि (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              यदि (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		स_नकल (vec_base.f128++, *p_argv.f128, परिमाण (float128));
              अन्यथा
		स_नकल (next_arg.f128, *p_argv.f128, परिमाण (float128));
              यदि (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_गिनती++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              भङ्ग;
            }
	  यदि ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# यदि _CALL_ELF != 2
		  यदि ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*next_arg.d = double_tmp;
	      यदि (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_गिनती++;
	      double_tmp = (*p_argv.d)[1];
	      यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# यदि _CALL_ELF != 2
		  यदि ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*next_arg.d = double_tmp;
	      यदि (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_गिनती++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      भङ्ग;
	    }
	  /* Fall through.  */
#endif
	पक्ष FFI_TYPE_DOUBLE:
#यदि _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#यदि _CALL_ELF != 2
	      यदि ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  अन्यथा
	    *next_arg.d = double_tmp;
	  यदि (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_गिनती++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
#यदि _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#यदि _CALL_ELF != 2
	      यदि ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (भिन्न) double_tmp;
# अन्यथा
		  next_arg.f[0] = (भिन्न) double_tmp;
# endif
		}
#endif
	    }
	  अन्यथा
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (भिन्न) double_tmp;
# अन्यथा
	      next_arg.f[0] = (भिन्न) double_tmp;
# endif
	    }
	  यदि (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_गिनती++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	पक्ष FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  यदि (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  यदि (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      अचिन्हित पूर्णाङ्क j;
	      प्रति (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  यदि (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  यदि (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    स_नकल (vec_base.f128++, cval + j, परिमाण (float128));
		  अन्यथा
		    स_नकल (next_arg.f128, cval + j, परिमाण (float128));
		  यदि (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      भङ्ग;
	    }
	  यदि (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      द्विगुण *cval = (द्विगुण *) *p_argv.v;
	      अचिन्हित पूर्णाङ्क j;
	      प्रति (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *next_arg.d = double_tmp;
		  यदि (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      भङ्ग;
	    }
#endif
	  यदि (elt == FFI_TYPE_FLOAT)
	    {
	      भिन्न *cval = (भिन्न *) *p_argv.v;
	      अचिन्हित पूर्णाङ्क j;
	      प्रति (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (भिन्न) double_tmp;
# अन्यथा
		      next_arg.f[0] = (भिन्न) double_tmp;
# endif
		    }
		  यदि (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा यदि (elt == FFI_TYPE_DOUBLE)
	    {
	      द्विगुण *cval = (द्विगुण *) *p_argv.v;
	      अचिन्हित पूर्णाङ्क j;
	      प्रति (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *next_arg.d = double_tmp;
		  यदि (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      वर्ण *cval = (वर्ण *) *p_argv.v;
	      माप_प्रकार hsize = (*ptr)->elements[0]->size;
	      अचिन्हित पूर्णाङ्क j;
	      प्रति (j = 0; j < 2; j++)
		{
		  वर्ण *half = cval + j * hsize;
		  अचिन्हित दीर्घ gprvalue;
		  विकल्प (elt)
		    {
		    पक्ष FFI_TYPE_UINT8:
		      gprvalue = *(अचिन्हित वर्ण *) half; भङ्ग;
		    पक्ष FFI_TYPE_SINT8:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित वर्ण *) half;
		      भङ्ग;
		    पक्ष FFI_TYPE_UINT16:
		      gprvalue = *(अचिन्हित लघु *) half; भङ्ग;
		    पक्ष FFI_TYPE_SINT16:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित लघु *) half;
		      भङ्ग;
		    पक्ष FFI_TYPE_UINT32:
		      gprvalue = *(अचिन्हित पूर्णाङ्क *) half; भङ्ग;
		    पक्ष FFI_TYPE_INT:
		    पक्ष FFI_TYPE_SINT32:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित पूर्णाङ्क *) half;
		      भङ्ग;
		    पक्ष FFI_TYPE_SINT64:
		    पक्ष FFI_TYPE_UINT64:
		    पक्ष FFI_TYPE_POINTER:
		    स्वभाव:
		      gprvalue = *(अचिन्हित दीर्घ *) half; भङ्ग;
		    }
		  *next_arg.ul++ = gprvalue;
		  यदि (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_STRUCT:
	  यदि ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      यदि (align > 16)
		align = 16;
	      यदि (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  यदि (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  यदि (elt)
	    {
#यदि _CALL_ELF == 2
	      सङ्घ {
		शून्य *v;
		भिन्न *f;
		द्विगुण *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              यदि (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  कुरु
                    {
                      यदि (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        स_नकल (vec_base.f128++, arg.f128++, परिमाण (float128));
                      अन्यथा
		        स_नकल (next_arg.f128, arg.f128++, परिमाण (float128));
                      यदि (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_गिनती++;
                    }
                  यावत् (--elnum != 0);
                }
              अन्यथा
#endif
	      यदि (elt == FFI_TYPE_FLOAT)
		{
		  कुरु
		    {
		      double_tmp = *arg.f++;
		      यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      अन्यथा
			*next_arg.f = (भिन्न) double_tmp;
		      यदि (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_गिनती++;
		    }
		  यावत् (--elnum != 0);
		  यदि ((next_arg.p & 7) != 0)
                    यदि (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      अन्यथा
		कुरु
		  {
		    double_tmp = *arg.d++;
		    यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    अन्यथा
		      *next_arg.d = double_tmp;
		    यदि (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_गिनती++;
		  }
		यावत् (--elnum != 0);
#अन्यथा
	      यदि (elt == FFI_TYPE_FLOAT)
		गच्छ do_float;
	      अन्यथा
		गच्छ do_double;
#endif
	    }
	  अन्यथा
	    {
	      शब्दगण = ((*ptr)->size + 7) / 8;
	      यदि (next_arg.ul >= gpr_base.ul && next_arg.ul + शब्दगण > gpr_end.ul)
		{
		  माप_प्रकार first = gpr_end.c - next_arg.c;
		  स_नकल (next_arg.c, *p_argv.c, first);
		  स_नकल (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + शब्दगण * 8 - first;
		}
	      अन्यथा
		{
		  वर्ण *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  यदि ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  स_नकल (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += शब्दगण;
		  यदि (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  गच्छ putgpr;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  यदि (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  भङ्ग;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#यदि _CALL_ELF == 2
#define MIN_CACHE_पंक्ति_SIZE 8

स्थिर शून्य
flush_icache (वर्ण *wraddr, वर्ण *xaddr, पूर्णाङ्क size)
{
  पूर्णाङ्क i;
  प्रति (i = 0; i < size; i += MIN_CACHE_पंक्ति_SIZE)
    __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			      शून्य *user_data,
			      शून्य *codeloc)
{
  यदि (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    प्रत्यागम FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  यदि (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#endif
    {
#यदि _CALL_ELF == 2
      अचिन्हित पूर्णाङ्क *tramp = (अचिन्हित पूर्णाङ्क *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(शून्य **) &tramp[4] = (शून्य *) ffi_closure_LINUX64;
      *(शून्य **) &tramp[6] = codeloc;
      flush_icache ((वर्ण *) tramp, (वर्ण *) codeloc, 4 * 4);
#अन्यथा
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      शून्य **tramp = (शून्य **) &closure->tramp[0];
      स_नकल (&tramp[0], (शून्य **) ffi_closure_LINUX64, परिमाण (शून्य *));
      tramp[1] = codeloc;
      स_नकल (&tramp[2], (शून्य **) ffi_closure_LINUX64 + 1, परिमाण (शून्य *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}


पूर्णाङ्क FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			    शून्य *user_data,
			    शून्य *rvalue,
			    अचिन्हित दीर्घ *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  शून्य **avalue;
  ffi_type **arg_types;
  अचिन्हित दीर्घ i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + अंक_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + अंक_VEC_ARG_REGISTERS64;
  अचिन्हित दीर्घ align;

  avalue = alloca (cif->nargs * परिमाण (शून्य *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  यदि (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (शून्य *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#यदि _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  यदि ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  यावत् (i < avn)
    {
      अचिन्हित पूर्णाङ्क elt, elnum;

      विकल्प (arg_types[i]->type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (वर्ण *) pst + 7;
	  pst++;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (वर्ण *) pst + 6;
	  pst++;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (वर्ण *) pst + 4;
	  pst++;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  यदि ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      यदि (align > 16)
		align = 16;
	      यदि (align > 1)
		pst = (अचिन्हित दीर्घ *) FFI_ALIGN ((माप_प्रकार) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  यदि (elt)
	    {
#यदि _CALL_ELF == 2
	      सङ्घ {
		शून्य *v;
		अचिन्हित दीर्घ *ul;
		भिन्न *f;
		द्विगुण *d;
		float128 *f128;
		माप_प्रकार p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              यदि (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  यदि (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  अन्यथा
                    to.v = pst;
                }
              अन्यथा
#endif
	      यदि (pfr + elnum <= end_pfr)
		to.v = pfr;
	      अन्यथा
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              यदि (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  कुरु
                    {
                      यदि (pvec < end_pvec && i < nfixedargs)
		        स_नकल (to.f128, pvec++, परिमाण (float128));
                      अन्यथा
		        स_नकल (to.f128, from.f128, परिमाण (float128));
                      to.f128++;
                      from.f128++;
                    }
                  यावत् (--elnum != 0);
                }
              अन्यथा
#endif
	      यदि (elt == FFI_TYPE_FLOAT)
		{
		  कुरु
		    {
		      यदि (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (भिन्न) pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  यावत् (--elnum != 0);
		}
	      अन्यथा
		{
		  कुरु
		    {
		      यदि (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  यावत् (--elnum != 0);
		}
#अन्यथा
	      यदि (elt == FFI_TYPE_FLOAT)
		गच्छ do_float;
	      अन्यथा
		गच्छ do_double;
#endif
	    }
	  अन्यथा
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      यदि (arg_types[i]->size < 8)
		avalue[i] = (वर्ण *) pst + 8 - arg_types[i]->size;
	      अन्यथा
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  भङ्ग;

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
          यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              यदि (((अचिन्हित दीर्घ) pst & 0xF) != 0)
                ++pst;
              यदि (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              अन्यथा
                avalue[i] = pst;
              pst += 2;
              भङ्ग;
            }
          अन्यथा यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      यदि (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      अन्यथा
		{
		  यदि (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(अचिन्हित दीर्घ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      भङ्ग;
	    }
	  /* Fall through.  */
#endif
	पक्ष FFI_TYPE_DOUBLE:
#यदि _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  यदि (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    avalue[i] = pst;
	  pst++;
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
#यदि _CALL_ELF != 2
	do_float:
#endif
	  यदि (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (भिन्न) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (वर्ण *) pst + 4;
#अन्यथा
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  भङ्ग;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	पक्ष FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    अचिन्हित पूर्णाङ्क j;
	    elt = arg_types[i]->elements[0]->type;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    यदि (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    यदि (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * परिमाण (float128));
		यदि (((अचिन्हित दीर्घ) pst & 0xF) != 0)
		  ++pst;
		प्रति (j = 0; j < 2; j++)
		  {
		    यदि (pvec < end_pvec && i < nfixedargs)
		      स_नकल (&cval[j], pvec++, परिमाण (float128));
		    अन्यथा
		      स_नकल (&cval[j], pst, परिमाण (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		भङ्ग;
	      }
	    यदि (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		द्विगुण *cval = alloca (4 * परिमाण (द्विगुण));
		प्रति (j = 0; j < 4; j++)
		  {
		    यदि (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    अन्यथा
		      cval[j] = *(द्विगुण *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		भङ्ग;
	      }
#endif
	    यदि (elt == FFI_TYPE_FLOAT)
	      {
		भिन्न *cval = alloca (2 * परिमाण (भिन्न));
		प्रति (j = 0; j < 2; j++)
		  {
		    यदि (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (भिन्न) pfr->d;
			pfr++;
		      }
		    अन्यथा
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((भिन्न *) pst)[1];
#अन्यथा
			cval[j] = ((भिन्न *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    अन्यथा यदि (elt == FFI_TYPE_DOUBLE)
	      {
		द्विगुण *cval = alloca (2 * परिमाण (द्विगुण));
		प्रति (j = 0; j < 2; j++)
		  {
		    यदि (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    अन्यथा
		      cval[j] = *(द्विगुण *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    अन्यथा
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		माप_प्रकार hsize = arg_types[i]->elements[0]->size;
		वर्ण *cval = alloca (2 * hsize);
		प्रति (j = 0; j < 2; j++)
		  {
		    वर्ण *src;
#ifndef __LITTLE_ENDIAN__
		    src = (वर्ण *) pst + (8 - hsize);
#अन्यथा
		    src = (वर्ण *) pst;
#endif
		    स_नकल (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  भङ्ग;
#endif

	स्वभाव:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      प्रत्यागम PPC_LD_NONE;
    पक्ष FFI_TYPE_FLOAT:
      प्रत्यागम PPC_LD_F32;
    पक्ष FFI_TYPE_DOUBLE:
      प्रत्यागम PPC_LD_F64;
#यदि FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      यदि ((cif->flags & FLAG_RETURNS_VEC) != 0)
	प्रत्यागम PPC64_LD_VECTOR;
      प्रत्यागम PPC_LD_F128;
#endif
    पक्ष FFI_TYPE_UINT8:
      प्रत्यागम PPC_LD_U8;
    पक्ष FFI_TYPE_SINT8:
      प्रत्यागम PPC_LD_S8;
    पक्ष FFI_TYPE_UINT16:
      प्रत्यागम PPC_LD_U16;
    पक्ष FFI_TYPE_SINT16:
      प्रत्यागम PPC_LD_S16;
    पक्ष FFI_TYPE_UINT32:
      प्रत्यागम PPC_LD_U32;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      प्रत्यागम PPC_LD_S32;
    पक्ष FFI_TYPE_POINTER:
      प्रत्यागम PPC_LD_PTR;
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      प्रत्यागम PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    पक्ष FFI_TYPE_COMPLEX:
      {
	पूर्णाङ्क inner = cif->rtype->elements[0]->type;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	यदि (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    यदि ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      प्रत्यागम PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	यदि (inner == FFI_TYPE_FLOAT)
	  प्रत्यागम PPC64_LD_FLOAT_HOMOG;
	यदि (inner == FFI_TYPE_DOUBLE)
	  प्रत्यागम PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  वर्ण *rv = rvalue;
	  अचिन्हित दीर्घ re, im;
	  विकल्प (inner)
	    {
	    पक्ष FFI_TYPE_UINT8:
	      re = ((अचिन्हित वर्ण *) rv)[0];
	      im = ((अचिन्हित वर्ण *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_SINT8:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित वर्ण *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित वर्ण *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_UINT16:
	      re = ((अचिन्हित लघु *) rv)[0];
	      im = ((अचिन्हित लघु *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_SINT16:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_UINT32:
	      re = ((अचिन्हित पूर्णाङ्क *) rv)[0];
	      im = ((अचिन्हित पूर्णाङ्क *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_INT:
	    पक्ष FFI_TYPE_SINT32:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णाङ्क *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णाङ्क *) rv)[1];
	      भङ्ग;
	    पक्ष FFI_TYPE_SINT64:
	    पक्ष FFI_TYPE_UINT64:
	    पक्ष FFI_TYPE_POINTER:
	    स्वभाव:
	      re = ((अचिन्हित दीर्घ *) rv)[0];
	      im = ((अचिन्हित दीर्घ *) rv)[1];
	      भङ्ग;
	    }
	  ((अचिन्हित दीर्घ *) rv)[0] = re;
	  ((अचिन्हित दीर्घ *) rv)[1] = im;
	  प्रत्यागम PPC_LD_R3R4;
	}
      }
#endif
    पक्ष FFI_TYPE_STRUCT:
      यदि ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  यदि ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      विकल्प (cif->rtype->size)
		{
		पक्ष 0:
		  प्रत्यागम PPC_LD_NONE;
		पक्ष 1:
		  प्रत्यागम PPC_LD_U8;
		पक्ष 2:
		  प्रत्यागम PPC_LD_U16;
		पक्ष 3:
		  प्रत्यागम PPC64_LD_STRUCT_3;
		पक्ष 4:
		  प्रत्यागम PPC_LD_U32;
		पक्ष 5:
		  प्रत्यागम PPC64_LD_STRUCT_5;
		पक्ष 6:
		  प्रत्यागम PPC64_LD_STRUCT_6;
		पक्ष 7:
		  प्रत्यागम PPC64_LD_STRUCT_7;
		पक्ष 8 ... 16:
		  प्रत्यागम PPC_LD_R3R4;
		}
	      भङ्ग;
	    }
	  यदि ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    प्रत्यागम PPC64_LD_VECTOR_HOMOG;
	  यदि ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    प्रत्यागम PPC64_LD_DOUBLE_HOMOG;
	  प्रत्यागम PPC64_LD_FLOAT_HOMOG;
        }
      प्रत्यागम PPC_LD_NONE;
    }
  पात();
}
#endif

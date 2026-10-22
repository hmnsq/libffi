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

#समावेश "ffi.h"
#समावेश <tramp.h>
#समावेश <stdlib.h>

#अगर_घोषित POWERPC64
#समावेश "ffi_common.h"
#समावेश "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
क्रमागत {
  अंक_GPR_ARG_REGISTERS64 = 8,
  अंक_FPR_ARG_REGISTERS64 = 13,
  अंक_VEC_ARG_REGISTERS64 = 12,
};
क्रमागत { ASM_NEEDS_REGISTERS64 = 4 };


#अगर HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
व्योम FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  अगर ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.आकार = 8;
      ffi_type_longdouble.alignment = 8;
    }
  अन्यथा
    {
      ffi_type_longdouble.आकार = 16;
      ffi_type_longdouble.alignment = 16;
    }
#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.आकार = 2 * ffi_type_longdouble.आकार;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#पूर्ण_अगर
}
#पूर्ण_अगर


अटल अचिन्हित पूर्णांक
discover_homogeneous_aggregate (ffi_abi abi,
                                स्थिर ffi_type *t,
                                अचिन्हित पूर्णांक *elnum)
{
  चयन (t->type)
    {
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      अगर ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          वापस FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      अन्यथा अगर ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          वापस FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#पूर्ण_अगर
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
      *elnum = 1;
      वापस (पूर्णांक) t->type;

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
    हाल FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	अचिन्हित पूर्णांक inner_elnum = 0;
	अचिन्हित पूर्णांक inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	अगर (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    वापस inner;
	  }
	वापस 0;
      }
#पूर्ण_अगर

    हाल FFI_TYPE_STRUCT:;
      {
	अचिन्हित पूर्णांक base_elt = 0, योग_elnum = 0;
	ffi_type **el = t->elements;
	जबतक (*el)
	  {
	    अचिन्हित पूर्णांक el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    अगर (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      वापस 0;
	    base_elt = el_elt;
	    योग_elnum += el_elnum;
#अगर _CALL_ELF == 2
	    अगर (योग_elnum > 8)
	      वापस 0;
#अन्यथा
	    अगर (योग_elnum > 1)
	      वापस 0;
#पूर्ण_अगर
	    el++;
	  }
	*elnum = योग_elnum;
	वापस base_elt;
      }

    शेष:
      वापस 0;
    }
}


/* Perform machine dependent cif processing */
अटल ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित i, fparg_गिनती = 0, intarg_गिनती = 0, vecarg_गिनती = 0;
  अचिन्हित flags = cif->flags;
  अचिन्हित elt, elnum, rtype;

#अगर FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    वापस FFI_BAD_ABI;
#या_अगर !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    वापस FFI_BAD_ABI;
#अन्यथा
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    वापस FFI_BAD_ABI;
#पूर्ण_अगर

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#अगर _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * माप (दीर्घ);

  /* Space for the general registers.  */
  bytes += अंक_GPR_ARG_REGISTERS64 * माप (दीर्घ);
#अन्यथा
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * माप (दीर्घ);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * अंक_GPR_ARG_REGISTERS64 * माप (दीर्घ);
#पूर्ण_अगर

  /* Return value handling.  */
  rtype = cif->rtype->type;
#अगर _CALL_ELF == 2
homogeneous:
#पूर्ण_अगर
  चयन (rtype)
    {
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          अवरोध;
        }
      अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#पूर्ण_अगर
    हाल FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    हाल FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      अवरोध;

    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
#अगर _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      अगर (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          जाओ homogeneous;
        }
      अगर (cif->rtype->आकार <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          अवरोध;
        }
#पूर्ण_अगर
      intarg_गिनती++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    हाल FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      अवरोध;

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
    हाल FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      चयन (rtype)
        {
        हाल FFI_TYPE_FLOAT:
        हाल FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          जाओ homogeneous;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        हाल FFI_TYPE_LONGDOUBLE:
          अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              अवरोध;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          जाओ homogeneous;
#पूर्ण_अगर
        हाल FFI_TYPE_INT:
        हाल FFI_TYPE_SINT8:  हाल FFI_TYPE_UINT8:
        हाल FFI_TYPE_SINT16: हाल FFI_TYPE_UINT16:
        हाल FFI_TYPE_SINT32: हाल FFI_TYPE_UINT32:
        हाल FFI_TYPE_SINT64: हाल FFI_TYPE_UINT64:
        हाल FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          अवरोध;
        शेष:
          वापस FFI_BAD_TYPEDEF;
        }
      अवरोध;
#पूर्ण_अगर

    शेष:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      अवरोध;
    }

  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित पूर्णांक align;

      चयन ((*ptr)->type)
	{
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
          अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_गिनती++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_गिनती = (intarg_गिनती + 3) & ~0x1;
              अगर (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              अवरोध;
            }
	  अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_गिनती++;
	      intarg_गिनती++;
	    }
	  /* Fall through.  */
#पूर्ण_अगर
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_FLOAT:
	  fparg_गिनती++;
	  intarg_गिनती++;
	  अगर (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  अगर ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      अगर (align > 16)
		align = 16;
	      align = align / 8;
	      अगर (align > 1)
		intarg_गिनती = FFI_ALIGN (intarg_गिनती, align);
	    }
	  intarg_गिनती += ((*ptr)->आकार + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          अगर (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_गिनती += elnum;
              अगर (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              अवरोध;
            }
	  अन्यथा
#पूर्ण_अगर
	  अगर (elt)
	    {
	      fparg_गिनती += elnum;
	      अगर (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  अन्यथा
	    {
	      अगर (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  अवरोध;

	हाल FFI_TYPE_POINTER:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_गिनती++;
	  अगर (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  अवरोध;

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
	हाल FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  अगर (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#पूर्ण_अगर
	  चयन (elt)
	    {
	    हाल FFI_TYPE_FLOAT:
	    हाल FFI_TYPE_DOUBLE:
	      fparg_गिनती += 2;
	      intarg_गिनती += 2;
	      अगर (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      अवरोध;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    हाल FFI_TYPE_LONGDOUBLE:
	      अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_गिनती += 2;
		  intarg_गिनती = (intarg_गिनती + 1) & ~0x1;
		  intarg_गिनती += 4;
		  अगर (vecarg_गिनती > अंक_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  अवरोध;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_गिनती += 4;
	      intarg_गिनती += 4;
	      अगर (fparg_गिनती > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      अवरोध;
#पूर्ण_अगर
	    हाल FFI_TYPE_INT:
	    हाल FFI_TYPE_SINT8:  हाल FFI_TYPE_UINT8:
	    हाल FFI_TYPE_SINT16: हाल FFI_TYPE_UINT16:
	    हाल FFI_TYPE_SINT32: हाल FFI_TYPE_UINT32:
	    हाल FFI_TYPE_SINT64: हाल FFI_TYPE_UINT64:
	    हाल FFI_TYPE_POINTER:
	      intarg_गिनती += 2;
	      अवरोध;
	    शेष:
	      वापस FFI_BAD_TYPEDEF;
	    }
	  अगर (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  अवरोध;
#पूर्ण_अगर

	शेष:
	  FFI_ASSERT (0);
	}
    }

  अगर (fparg_गिनती != 0)
    flags |= FLAG_FP_ARGUMENTS;
  अगर (intarg_गिनती > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  अगर (vecarg_गिनती != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  अगर (fparg_गिनती != 0)
    bytes += अंक_FPR_ARG_REGISTERS64 * माप (द्विगुन);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  अगर (vecarg_गिनती != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += अंक_VEC_ARG_REGISTERS64 * माप (float128);
  }

  /* Stack space.  */
#अगर _CALL_ELF == 2
  अगर ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_गिनती * माप (दीर्घ);
#अन्यथा
  अगर (intarg_गिनती > अंक_GPR_ARG_REGISTERS64)
    bytes += (intarg_गिनती - अंक_GPR_ARG_REGISTERS64) * माप (दीर्घ);
#पूर्ण_अगर

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  वापस FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  अगर ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#अगर _CALL_ELF != 2
  अन्यथा अगर (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#पूर्ण_अगर
  अन्यथा
    वापस FFI_BAD_ABI;
  वापस ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  अचिन्हित पूर्णांक nfixedargs,
			  अचिन्हित पूर्णांक ntotalargs MAYBE_UNUSED)
{
  अगर ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#अगर _CALL_ELF != 2
  अन्यथा अगर (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#पूर्ण_अगर
  अन्यथा
    वापस FFI_BAD_ABI;
#अगर _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#पूर्ण_अगर
  वापस ffi_prep_cif_linux64_core (cif);
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

व्योम FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, अचिन्हित दीर्घ *स्थिर ढेर)
{
  स्थिर अचिन्हित दीर्घ bytes = ecif->cif->bytes;
  स्थिर अचिन्हित दीर्घ flags = ecif->cif->flags;

  प्रकार जोड़
  {
    अक्षर *c;
    अचिन्हित दीर्घ *ul;
    भग्न *f;
    द्विगुन *d;
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
  valp अग्र_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_base;
  अचिन्हित पूर्णांक fparg_गिनती;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  अचिन्हित पूर्णांक vecarg_गिनती;

  अचिन्हित पूर्णांक i, शब्दगण, nargs, nfixedargs;
  ffi_type **ptr;
  द्विगुन double_tmp;
  जोड़
  {
    व्योम **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    चिन्हित पूर्णांक **si;
    अचिन्हित पूर्णांक **ui;
    अचिन्हित दीर्घ **ul;
    भग्न **f;
    द्विगुन **d;
    float128 **f128;
  } p_argv;
  अचिन्हित दीर्घ gprvalue;
  अचिन्हित दीर्घ align;

  stacktop.c = (अक्षर *) ढेर + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - अंक_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + अंक_GPR_ARG_REGISTERS64;
#अगर _CALL_ELF == 2
  rest.ul = ढेर + 4 + अंक_GPR_ARG_REGISTERS64;
#अन्यथा
  rest.ul = ढेर + 6 + अंक_GPR_ARG_REGISTERS64;
#पूर्ण_अगर
  fpr_base.d = gpr_base.d - अंक_FPR_ARG_REGISTERS64;
  fparg_गिनती = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  अगर (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  अन्यथा
    vec_base.p = gpr_base.p;
  vec_base.f128 -= अंक_VEC_ARG_REGISTERS64;
  vecarg_गिनती = 0;
  अग्र_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिन्हित दीर्घ) (अक्षर *) ढेर & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  अगर (flags & FLAG_RETVAL_REFERENCE)
    *अग्र_arg.ul++ = (अचिन्हित दीर्घ) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#अगर _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  अगर ((flags & FLAG_COMPAT) == 0)
#पूर्ण_अगर
    nfixedargs = ecif->cif->nfixedargs;
  क्रम (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      अचिन्हित पूर्णांक elt, elnum;

      चयन ((*ptr)->type)
	{
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
          अगर ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              अग्र_arg.p = FFI_ALIGN (अग्र_arg.p, 16);
              अगर (अग्र_arg.ul == gpr_end.ul)
                अग्र_arg.ul = rest.ul;
              अगर (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		स_नकल (vec_base.f128++, *p_argv.f128, माप (float128));
              अन्यथा
		स_नकल (अग्र_arg.f128, *p_argv.f128, माप (float128));
              अगर (++अग्र_arg.f128 == gpr_end.f128)
                अग्र_arg.f128 = rest.f128;
              vecarg_गिनती++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              अवरोध;
            }
	  अगर ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# अगर _CALL_ELF != 2
		  अगर ((flags & FLAG_COMPAT) != 0)
		    *अग्र_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*अग्र_arg.d = double_tmp;
	      अगर (++अग्र_arg.ul == gpr_end.ul)
		अग्र_arg.ul = rest.ul;
	      fparg_गिनती++;
	      double_tmp = (*p_argv.d)[1];
	      अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# अगर _CALL_ELF != 2
		  अगर ((flags & FLAG_COMPAT) != 0)
		    *अग्र_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*अग्र_arg.d = double_tmp;
	      अगर (++अग्र_arg.ul == gpr_end.ul)
		अग्र_arg.ul = rest.ul;
	      fparg_गिनती++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      अवरोध;
	    }
	  /* Fall through.  */
#पूर्ण_अगर
	हाल FFI_TYPE_DOUBLE:
#अगर _CALL_ELF != 2
	do_double:
#पूर्ण_अगर
	  double_tmp = **p_argv.d;
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#अगर _CALL_ELF != 2
	      अगर ((flags & FLAG_COMPAT) != 0)
		*अग्र_arg.d = double_tmp;
#पूर्ण_अगर
	    }
	  अन्यथा
	    *अग्र_arg.d = double_tmp;
	  अगर (++अग्र_arg.ul == gpr_end.ul)
	    अग्र_arg.ul = rest.ul;
	  fparg_गिनती++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
#अगर _CALL_ELF != 2
	do_float:
#पूर्ण_अगर
	  double_tmp = **p_argv.f;
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#अगर _CALL_ELF != 2
	      अगर ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  अग्र_arg.f[1] = (भग्न) double_tmp;
# अन्यथा
		  अग्र_arg.f[0] = (भग्न) double_tmp;
# endif
		}
#पूर्ण_अगर
	    }
	  अन्यथा
	    {
# ifndef __LITTLE_ENDIAN__
	      अग्र_arg.f[1] = (भग्न) double_tmp;
# अन्यथा
	      अग्र_arg.f[0] = (भग्न) double_tmp;
# endif
	    }
	  अगर (++अग्र_arg.ul == gpr_end.ul)
	    अग्र_arg.ul = rest.ul;
	  fparg_गिनती++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  अवरोध;

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
	हाल FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  अगर (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  अगर (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      क्रम (j = 0; j < 2; j++)
		{
		  अग्र_arg.p = FFI_ALIGN (अग्र_arg.p, 16);
		  अगर (अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		  अगर (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    स_नकल (vec_base.f128++, cval + j, माप (float128));
		  अन्यथा
		    स_नकल (अग्र_arg.f128, cval + j, माप (float128));
		  अगर (++अग्र_arg.f128 == gpr_end.f128)
		    अग्र_arg.f128 = rest.f128;
		  vecarg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      अवरोध;
	    }
	  अगर (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      द्विगुन *cval = (द्विगुन *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      क्रम (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *अग्र_arg.d = double_tmp;
		  अगर (++अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      अवरोध;
	    }
#पूर्ण_अगर
	  अगर (elt == FFI_TYPE_FLOAT)
	    {
	      भग्न *cval = (भग्न *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      क्रम (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    {
# ifndef __LITTLE_ENDIAN__
		      अग्र_arg.f[1] = (भग्न) double_tmp;
# अन्यथा
		      अग्र_arg.f[0] = (भग्न) double_tmp;
# endif
		    }
		  अगर (++अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा अगर (elt == FFI_TYPE_DOUBLE)
	    {
	      द्विगुन *cval = (द्विगुन *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      क्रम (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *अग्र_arg.d = double_tmp;
		  अगर (++अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		  fparg_गिनती++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      अक्षर *cval = (अक्षर *) *p_argv.v;
	      माप_प्रकार hsize = (*ptr)->elements[0]->आकार;
	      अचिन्हित पूर्णांक j;
	      क्रम (j = 0; j < 2; j++)
		{
		  अक्षर *half = cval + j * hsize;
		  अचिन्हित दीर्घ gprvalue;
		  चयन (elt)
		    {
		    हाल FFI_TYPE_UINT8:
		      gprvalue = *(अचिन्हित अक्षर *) half; अवरोध;
		    हाल FFI_TYPE_SINT8:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित अक्षर *) half;
		      अवरोध;
		    हाल FFI_TYPE_UINT16:
		      gprvalue = *(अचिन्हित लघु *) half; अवरोध;
		    हाल FFI_TYPE_SINT16:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित लघु *) half;
		      अवरोध;
		    हाल FFI_TYPE_UINT32:
		      gprvalue = *(अचिन्हित पूर्णांक *) half; अवरोध;
		    हाल FFI_TYPE_INT:
		    हाल FFI_TYPE_SINT32:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित पूर्णांक *) half;
		      अवरोध;
		    हाल FFI_TYPE_SINT64:
		    हाल FFI_TYPE_UINT64:
		    हाल FFI_TYPE_POINTER:
		    शेष:
		      gprvalue = *(अचिन्हित दीर्घ *) half; अवरोध;
		    }
		  *अग्र_arg.ul++ = gprvalue;
		  अगर (अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		}
	    }
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_STRUCT:
	  अगर ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      अगर (align > 16)
		align = 16;
	      अगर (align > 1)
                {
                  अग्र_arg.p = FFI_ALIGN (अग्र_arg.p, align);
                  अगर (अग्र_arg.ul == gpr_end.ul)
                    अग्र_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  अगर (elt)
	    {
#अगर _CALL_ELF == 2
	      जोड़ {
		व्योम *v;
		भग्न *f;
		द्विगुन *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              अगर (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  करो
                    {
                      अगर (vecarg_गिनती < अंक_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        स_नकल (vec_base.f128++, arg.f128++, माप (float128));
                      अन्यथा
		        स_नकल (अग्र_arg.f128, arg.f128++, माप (float128));
                      अगर (++अग्र_arg.f128 == gpr_end.f128)
                        अग्र_arg.f128 = rest.f128;
                      vecarg_गिनती++;
                    }
                  जबतक (--elnum != 0);
                }
              अन्यथा
#पूर्ण_अगर
	      अगर (elt == FFI_TYPE_FLOAT)
		{
		  करो
		    {
		      double_tmp = *arg.f++;
		      अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      अन्यथा
			*अग्र_arg.f = (भग्न) double_tmp;
		      अगर (++अग्र_arg.f == gpr_end.f)
			अग्र_arg.f = rest.f;
		      fparg_गिनती++;
		    }
		  जबतक (--elnum != 0);
		  अगर ((अग्र_arg.p & 7) != 0)
                    अगर (++अग्र_arg.f == gpr_end.f)
                      अग्र_arg.f = rest.f;
		}
	      अन्यथा
		करो
		  {
		    double_tmp = *arg.d++;
		    अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    अन्यथा
		      *अग्र_arg.d = double_tmp;
		    अगर (++अग्र_arg.d == gpr_end.d)
		      अग्र_arg.d = rest.d;
		    fparg_गिनती++;
		  }
		जबतक (--elnum != 0);
#अन्यथा
	      अगर (elt == FFI_TYPE_FLOAT)
		जाओ do_float;
	      अन्यथा
		जाओ do_double;
#पूर्ण_अगर
	    }
	  अन्यथा
	    {
	      शब्दगण = ((*ptr)->आकार + 7) / 8;
	      अगर (अग्र_arg.ul >= gpr_base.ul && अग्र_arg.ul + शब्दगण > gpr_end.ul)
		{
		  माप_प्रकार first = gpr_end.c - अग्र_arg.c;
		  स_नकल (अग्र_arg.c, *p_argv.c, first);
		  स_नकल (rest.c, *p_argv.c + first, (*ptr)->आकार - first);
		  अग्र_arg.c = rest.c + शब्दगण * 8 - first;
		}
	      अन्यथा
		{
		  अक्षर *where = अग्र_arg.c;

#अगर_अघोषित __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  अगर ((*ptr)->आकार < 8)
		    where += 8 - (*ptr)->आकार;
#पूर्ण_अगर
		  स_नकल (where, *p_argv.c, (*ptr)->आकार);
		  अग्र_arg.ul += शब्दगण;
		  अगर (अग्र_arg.ul == gpr_end.ul)
		    अग्र_arg.ul = rest.ul;
		}
	    }
	  अवरोध;

	हाल FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  जाओ putgpr;
	हाल FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  जाओ putgpr;
	हाल FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  जाओ putgpr;
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  जाओ putgpr;

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *अग्र_arg.ul++ = gprvalue;
	  अगर (अग्र_arg.ul == gpr_end.ul)
	    अग्र_arg.ul = rest.ul;
	  अवरोध;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (अग्र_arg.ul >= gpr_base.ul
		  && अग्र_arg.ul <= gpr_base.ul + 4));
}


#अगर _CALL_ELF == 2
#घोषणा MIN_CACHE_पंक्ति_आकार 8

अटल व्योम
flush_icache (अक्षर *wraddr, अक्षर *xaddr, पूर्णांक आकार)
{
  पूर्णांक i;
  क्रम (i = 0; i < आकार; i += MIN_CACHE_पंक्ति_आकार)
    __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + आकार - 1), "r"(wraddr + आकार - 1)
		    : "memory");
}
#पूर्ण_अगर


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
			      व्योम *user_तथ्य,
			      व्योम *codeloc)
{
  अगर (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    वापस FFI_BAD_ABI;

#अगर_घोषित FFI_EXEC_STATIC_TRAMP
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      व्योम (*dest)(व्योम) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#पूर्ण_अगर
    {
#अगर _CALL_ELF == 2
      अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(व्योम **) &tramp[4] = (व्योम *) ffi_closure_LINUX64;
      *(व्योम **) &tramp[6] = codeloc;
      flush_icache ((अक्षर *) tramp, (अक्षर *) codeloc, 4 * 4);
#अन्यथा
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      व्योम **tramp = (व्योम **) &closure->tramp[0];
      स_नकल (&tramp[0], (व्योम **) ffi_closure_LINUX64, माप (व्योम *));
      tramp[1] = codeloc;
      स_नकल (&tramp[2], (व्योम **) ffi_closure_LINUX64 + 1, माप (व्योम *));
#पूर्ण_अगर
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}


पूर्णांक FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
			    व्योम *user_तथ्य,
			    व्योम *rvalue,
			    अचिन्हित दीर्घ *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  व्योम **avalue;
  ffi_type **arg_types;
  अचिन्हित दीर्घ i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + अंक_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + अंक_VEC_ARG_REGISTERS64;
  अचिन्हित दीर्घ align;

  avalue = alloca (cif->nargs * माप (व्योम *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  अगर (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (व्योम *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#अगर _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  अगर ((cif->flags & FLAG_COMPAT) == 0)
#पूर्ण_अगर
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  जबतक (i < avn)
    {
      अचिन्हित पूर्णांक elt, elnum;

      चयन (arg_types[i]->type)
	{
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
#अगर_अघोषित __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 7;
	  pst++;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
#अगर_अघोषित __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 6;
	  pst++;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
#अगर_अघोषित __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 4;
	  pst++;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  अगर ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      अगर (align > 16)
		align = 16;
	      अगर (align > 1)
		pst = (अचिन्हित दीर्घ *) FFI_ALIGN ((माप_प्रकार) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  अगर (elt)
	    {
#अगर _CALL_ELF == 2
	      जोड़ {
		व्योम *v;
		अचिन्हित दीर्घ *ul;
		भग्न *f;
		द्विगुन *d;
		float128 *f128;
		माप_प्रकार p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              अगर (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  अगर (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  अन्यथा
                    to.v = pst;
                }
              अन्यथा
#पूर्ण_अगर
	      अगर (pfr + elnum <= end_pfr)
		to.v = pfr;
	      अन्यथा
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              अगर (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  करो
                    {
                      अगर (pvec < end_pvec && i < nfixedargs)
		        स_नकल (to.f128, pvec++, माप (float128));
                      अन्यथा
		        स_नकल (to.f128, from.f128, माप (float128));
                      to.f128++;
                      from.f128++;
                    }
                  जबतक (--elnum != 0);
                }
              अन्यथा
#पूर्ण_अगर
	      अगर (elt == FFI_TYPE_FLOAT)
		{
		  करो
		    {
		      अगर (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (भग्न) pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  जबतक (--elnum != 0);
		}
	      अन्यथा
		{
		  करो
		    {
		      अगर (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  जबतक (--elnum != 0);
		}
#अन्यथा
	      अगर (elt == FFI_TYPE_FLOAT)
		जाओ do_float;
	      अन्यथा
		जाओ do_double;
#पूर्ण_अगर
	    }
	  अन्यथा
	    {
#अगर_अघोषित __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      अगर (arg_types[i]->आकार < 8)
		avalue[i] = (अक्षर *) pst + 8 - arg_types[i]->आकार;
	      अन्यथा
#पूर्ण_अगर
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->आकार + 7) / 8;
	  अवरोध;

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
          अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              अगर (((अचिन्हित दीर्घ) pst & 0xF) != 0)
                ++pst;
              अगर (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              अन्यथा
                avalue[i] = pst;
              pst += 2;
              अवरोध;
            }
          अन्यथा अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      अगर (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      अन्यथा
		{
		  अगर (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(अचिन्हित दीर्घ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      अवरोध;
	    }
	  /* Fall through.  */
#पूर्ण_अगर
	हाल FFI_TYPE_DOUBLE:
#अगर _CALL_ELF != 2
	do_double:
#पूर्ण_अगर
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  अगर (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    avalue[i] = pst;
	  pst++;
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
#अगर _CALL_ELF != 2
	do_float:
#पूर्ण_अगर
	  अगर (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (भग्न) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
#अगर_अघोषित __LITTLE_ENDIAN__
	      avalue[i] = (अक्षर *) pst + 4;
#अन्यथा
	      avalue[i] = pst;
#पूर्ण_अगर
	    }
	  pst++;
	  अवरोध;

#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
	हाल FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    अचिन्हित पूर्णांक j;
	    elt = arg_types[i]->elements[0]->type;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    अगर (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    अगर (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * माप (float128));
		अगर (((अचिन्हित दीर्घ) pst & 0xF) != 0)
		  ++pst;
		क्रम (j = 0; j < 2; j++)
		  {
		    अगर (pvec < end_pvec && i < nfixedargs)
		      स_नकल (&cval[j], pvec++, माप (float128));
		    अन्यथा
		      स_नकल (&cval[j], pst, माप (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		अवरोध;
	      }
	    अगर (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		द्विगुन *cval = alloca (4 * माप (द्विगुन));
		क्रम (j = 0; j < 4; j++)
		  {
		    अगर (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    अन्यथा
		      cval[j] = *(द्विगुन *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		अवरोध;
	      }
#पूर्ण_अगर
	    अगर (elt == FFI_TYPE_FLOAT)
	      {
		भग्न *cval = alloca (2 * माप (भग्न));
		क्रम (j = 0; j < 2; j++)
		  {
		    अगर (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (भग्न) pfr->d;
			pfr++;
		      }
		    अन्यथा
		      {
#अगर_अघोषित __LITTLE_ENDIAN__
			cval[j] = ((भग्न *) pst)[1];
#अन्यथा
			cval[j] = ((भग्न *) pst)[0];
#पूर्ण_अगर
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    अन्यथा अगर (elt == FFI_TYPE_DOUBLE)
	      {
		द्विगुन *cval = alloca (2 * माप (द्विगुन));
		क्रम (j = 0; j < 2; j++)
		  {
		    अगर (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    अन्यथा
		      cval[j] = *(द्विगुन *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    अन्यथा
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		माप_प्रकार hsize = arg_types[i]->elements[0]->आकार;
		अक्षर *cval = alloca (2 * hsize);
		क्रम (j = 0; j < 2; j++)
		  {
		    अक्षर *src;
#अगर_अघोषित __LITTLE_ENDIAN__
		    src = (अक्षर *) pst + (8 - hsize);
#अन्यथा
		    src = (अक्षर *) pst;
#पूर्ण_अगर
		    स_नकल (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  अवरोध;
#पूर्ण_अगर

	शेष:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_तथ्य);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      वापस PPC_LD_NONE;
    हाल FFI_TYPE_FLOAT:
      वापस PPC_LD_F32;
    हाल FFI_TYPE_DOUBLE:
      वापस PPC_LD_F64;
#अगर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      अगर ((cif->flags & FLAG_RETURNS_VEC) != 0)
	वापस PPC64_LD_VECTOR;
      वापस PPC_LD_F128;
#पूर्ण_अगर
    हाल FFI_TYPE_UINT8:
      वापस PPC_LD_U8;
    हाल FFI_TYPE_SINT8:
      वापस PPC_LD_S8;
    हाल FFI_TYPE_UINT16:
      वापस PPC_LD_U16;
    हाल FFI_TYPE_SINT16:
      वापस PPC_LD_S16;
    हाल FFI_TYPE_UINT32:
      वापस PPC_LD_U32;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      वापस PPC_LD_S32;
    हाल FFI_TYPE_POINTER:
      वापस PPC_LD_PTR;
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      वापस PPC_LD_I64;
#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
    हाल FFI_TYPE_COMPLEX:
      {
	पूर्णांक inner = cif->rtype->elements[0]->type;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	अगर (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    अगर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      वापस PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#पूर्ण_अगर
	अगर (inner == FFI_TYPE_FLOAT)
	  वापस PPC64_LD_FLOAT_HOMOG;
	अगर (inner == FFI_TYPE_DOUBLE)
	  वापस PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  अक्षर *rv = rvalue;
	  अचिन्हित दीर्घ re, im;
	  चयन (inner)
	    {
	    हाल FFI_TYPE_UINT8:
	      re = ((अचिन्हित अक्षर *) rv)[0];
	      im = ((अचिन्हित अक्षर *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_SINT8:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित अक्षर *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित अक्षर *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_UINT16:
	      re = ((अचिन्हित लघु *) rv)[0];
	      im = ((अचिन्हित लघु *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_SINT16:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_UINT32:
	      re = ((अचिन्हित पूर्णांक *) rv)[0];
	      im = ((अचिन्हित पूर्णांक *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_INT:
	    हाल FFI_TYPE_SINT32:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णांक *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णांक *) rv)[1];
	      अवरोध;
	    हाल FFI_TYPE_SINT64:
	    हाल FFI_TYPE_UINT64:
	    हाल FFI_TYPE_POINTER:
	    शेष:
	      re = ((अचिन्हित दीर्घ *) rv)[0];
	      im = ((अचिन्हित दीर्घ *) rv)[1];
	      अवरोध;
	    }
	  ((अचिन्हित दीर्घ *) rv)[0] = re;
	  ((अचिन्हित दीर्घ *) rv)[1] = im;
	  वापस PPC_LD_R3R4;
	}
      }
#पूर्ण_अगर
    हाल FFI_TYPE_STRUCT:
      अगर ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  अगर ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      चयन (cif->rtype->आकार)
		{
		हाल 0:
		  वापस PPC_LD_NONE;
		हाल 1:
		  वापस PPC_LD_U8;
		हाल 2:
		  वापस PPC_LD_U16;
		हाल 3:
		  वापस PPC64_LD_STRUCT_3;
		हाल 4:
		  वापस PPC_LD_U32;
		हाल 5:
		  वापस PPC64_LD_STRUCT_5;
		हाल 6:
		  वापस PPC64_LD_STRUCT_6;
		हाल 7:
		  वापस PPC64_LD_STRUCT_7;
		हाल 8 ... 16:
		  वापस PPC_LD_R3R4;
		}
	      अवरोध;
	    }
	  अगर ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    वापस PPC64_LD_VECTOR_HOMOG;
	  अगर ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    वापस PPC64_LD_DOUBLE_HOMOG;
	  वापस PPC64_LD_FLOAT_HOMOG;
        }
      वापस PPC_LD_NONE;
    }
  पात();
}
#पूर्ण_अगर

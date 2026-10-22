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


#जर HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  जर ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
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


स्थिर अचिन्हित पूर्णांक
discover_homogeneous_aggregate (ffi_abi abi,
                                स्थिरांक ffi_type *t,
                                अचिन्हित पूर्णांक *elnum)
{
  निवड (t->type)
    {
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      जर ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          परत FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      अन्यथा जर ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          परत FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    स्थिती FFI_TYPE_FLOAT:
    स्थिती FFI_TYPE_DOUBLE:
      *elnum = 1;
      परत (पूर्णांक) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    स्थिती FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	अचिन्हित पूर्णांक inner_elnum = 0;
	अचिन्हित पूर्णांक inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	जर (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    परत inner;
	  }
	परत 0;
      }
#endif

    स्थिती FFI_TYPE_STRUCT:;
      {
	अचिन्हित पूर्णांक base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	पर्यंत (*el)
	  {
	    अचिन्हित पूर्णांक el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    जर (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      परत 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#जर _CALL_ELF == 2
	    जर (total_elnum > 8)
	      परत 0;
#अन्यथा
	    जर (total_elnum > 1)
	      परत 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	परत base_elt;
      }

    पूर्वनिर्धारित:
      परत 0;
    }
}


/* Perform machine dependent cif processing */
स्थिर ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित i, fparg_मोजणी = 0, intarg_मोजणी = 0, vecarg_मोजणी = 0;
  अचिन्हित flags = cif->flags;
  अचिन्हित elt, elnum, rtype;

#जर FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    परत FFI_BAD_ABI;
#किंवाजर !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    परत FFI_BAD_ABI;
#अन्यथा
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    परत FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#जर _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * आकार (दीर्घ);

  /* Space for the general registers.  */
  bytes += अंक_GPR_ARG_REGISTERS64 * आकार (दीर्घ);
#अन्यथा
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * आकार (दीर्घ);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * अंक_GPR_ARG_REGISTERS64 * आकार (दीर्घ);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#जर _CALL_ELF == 2
homogeneous:
#endif
  निवड (rtype)
    {
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          थांबा;
        }
      जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    स्थिती FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    स्थिती FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      थांबा;

    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
#जर _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      जर (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          जा homogeneous;
        }
      जर (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          थांबा;
        }
#endif
      intarg_मोजणी++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    स्थिती FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      थांबा;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    स्थिती FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      निवड (rtype)
        {
        स्थिती FFI_TYPE_FLOAT:
        स्थिती FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          जा homogeneous;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        स्थिती FFI_TYPE_LONGDOUBLE:
          जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              थांबा;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          जा homogeneous;
#endif
        स्थिती FFI_TYPE_INT:
        स्थिती FFI_TYPE_SINT8:  स्थिती FFI_TYPE_UINT8:
        स्थिती FFI_TYPE_SINT16: स्थिती FFI_TYPE_UINT16:
        स्थिती FFI_TYPE_SINT32: स्थिती FFI_TYPE_UINT32:
        स्थिती FFI_TYPE_SINT64: स्थिती FFI_TYPE_UINT64:
        स्थिती FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          थांबा;
        पूर्वनिर्धारित:
          परत FFI_BAD_TYPEDEF;
        }
      थांबा;
#endif

    पूर्वनिर्धारित:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      थांबा;
    }

  साठी (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित पूर्णांक align;

      निवड ((*ptr)->type)
	{
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
          जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_मोजणी++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_मोजणी = (intarg_मोजणी + 3) & ~0x1;
              जर (vecarg_मोजणी > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              थांबा;
            }
	  जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_मोजणी++;
	      intarg_मोजणी++;
	    }
	  /* Fall through.  */
#endif
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_FLOAT:
	  fparg_मोजणी++;
	  intarg_मोजणी++;
	  जर (fparg_मोजणी > अंक_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  जर ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      जर (align > 16)
		align = 16;
	      align = align / 8;
	      जर (align > 1)
		intarg_मोजणी = FFI_ALIGN (intarg_मोजणी, align);
	    }
	  intarg_मोजणी += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          जर (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_मोजणी += elnum;
              जर (vecarg_मोजणी > अंक_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              थांबा;
            }
	  अन्यथा
#endif
	  जर (elt)
	    {
	      fparg_मोजणी += elnum;
	      जर (fparg_मोजणी > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  अन्यथा
	    {
	      जर (intarg_मोजणी > अंक_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  थांबा;

	स्थिती FFI_TYPE_POINTER:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_मोजणी++;
	  जर (intarg_मोजणी > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  थांबा;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	स्थिती FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  जर (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  निवड (elt)
	    {
	    स्थिती FFI_TYPE_FLOAT:
	    स्थिती FFI_TYPE_DOUBLE:
	      fparg_मोजणी += 2;
	      intarg_मोजणी += 2;
	      जर (fparg_मोजणी > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      थांबा;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    स्थिती FFI_TYPE_LONGDOUBLE:
	      जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_मोजणी += 2;
		  intarg_मोजणी = (intarg_मोजणी + 1) & ~0x1;
		  intarg_मोजणी += 4;
		  जर (vecarg_मोजणी > अंक_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  थांबा;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_मोजणी += 4;
	      intarg_मोजणी += 4;
	      जर (fparg_मोजणी > अंक_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      थांबा;
#endif
	    स्थिती FFI_TYPE_INT:
	    स्थिती FFI_TYPE_SINT8:  स्थिती FFI_TYPE_UINT8:
	    स्थिती FFI_TYPE_SINT16: स्थिती FFI_TYPE_UINT16:
	    स्थिती FFI_TYPE_SINT32: स्थिती FFI_TYPE_UINT32:
	    स्थिती FFI_TYPE_SINT64: स्थिती FFI_TYPE_UINT64:
	    स्थिती FFI_TYPE_POINTER:
	      intarg_मोजणी += 2;
	      थांबा;
	    पूर्वनिर्धारित:
	      परत FFI_BAD_TYPEDEF;
	    }
	  जर (intarg_मोजणी > अंक_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  थांबा;
#endif

	पूर्वनिर्धारित:
	  FFI_ASSERT (0);
	}
    }

  जर (fparg_मोजणी != 0)
    flags |= FLAG_FP_ARGUMENTS;
  जर (intarg_मोजणी > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  जर (vecarg_मोजणी != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  जर (fparg_मोजणी != 0)
    bytes += अंक_FPR_ARG_REGISTERS64 * आकार (द्विगुण);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  जर (vecarg_मोजणी != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += अंक_VEC_ARG_REGISTERS64 * आकार (float128);
  }

  /* Stack space.  */
#जर _CALL_ELF == 2
  जर ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_मोजणी * आकार (दीर्घ);
#अन्यथा
  जर (intarg_मोजणी > अंक_GPR_ARG_REGISTERS64)
    bytes += (intarg_मोजणी - अंक_GPR_ARG_REGISTERS64) * आकार (दीर्घ);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  परत FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  जर ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#जर _CALL_ELF != 2
  अन्यथा जर (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  अन्यथा
    परत FFI_BAD_ABI;
  परत ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  अचिन्हित पूर्णांक nfixedargs,
			  अचिन्हित पूर्णांक ntotalargs MAYBE_UNUSED)
{
  जर ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#जर _CALL_ELF != 2
  अन्यथा जर (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  अन्यथा
    परत FFI_BAD_ABI;
#जर _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  परत ffi_prep_cif_linux64_core (cif);
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
ffi_prep_args64 (extended_cif *ecif, अचिन्हित दीर्घ *स्थिरांक चळत)
{
  स्थिरांक अचिन्हित दीर्घ bytes = ecif->cif->bytes;
  स्थिरांक अचिन्हित दीर्घ flags = ecif->cif->flags;

  प्रकारव्याख्या संघ
  {
    अक्षर *c;
    अचिन्हित दीर्घ *ul;
    दशांश *f;
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
  अचिन्हित पूर्णांक fparg_मोजणी;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  अचिन्हित पूर्णांक vecarg_मोजणी;

  अचिन्हित पूर्णांक i, शब्द, nargs, nfixedargs;
  ffi_type **ptr;
  द्विगुण double_tmp;
  संघ
  {
    शून्य **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    चिन्हित पूर्णांक **si;
    अचिन्हित पूर्णांक **ui;
    अचिन्हित दीर्घ **ul;
    दशांश **f;
    द्विगुण **d;
    float128 **f128;
  } p_argv;
  अचिन्हित दीर्घ gprvalue;
  अचिन्हित दीर्घ align;

  stacktop.c = (अक्षर *) चळत + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - अंक_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + अंक_GPR_ARG_REGISTERS64;
#जर _CALL_ELF == 2
  rest.ul = चळत + 4 + अंक_GPR_ARG_REGISTERS64;
#अन्यथा
  rest.ul = चळत + 6 + अंक_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - अंक_FPR_ARG_REGISTERS64;
  fparg_मोजणी = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  जर (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  अन्यथा
    vec_base.p = gpr_base.p;
  vec_base.f128 -= अंक_VEC_ARG_REGISTERS64;
  vecarg_मोजणी = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिन्हित दीर्घ) (अक्षर *) चळत & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  जर (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (अचिन्हित दीर्घ) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#जर _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  जर ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  साठी (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      अचिन्हित पूर्णांक elt, elnum;

      निवड ((*ptr)->type)
	{
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
          जर ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              जर (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              जर (vecarg_मोजणी < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		स_नकल (vec_base.f128++, *p_argv.f128, आकार (float128));
              अन्यथा
		स_नकल (next_arg.f128, *p_argv.f128, आकार (float128));
              जर (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_मोजणी++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              थांबा;
            }
	  जर ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# जर _CALL_ELF != 2
		  जर ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*next_arg.d = double_tmp;
	      जर (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_मोजणी++;
	      double_tmp = (*p_argv.d)[1];
	      जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# जर _CALL_ELF != 2
		  जर ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      अन्यथा
		*next_arg.d = double_tmp;
	      जर (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_मोजणी++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      थांबा;
	    }
	  /* Fall through.  */
#endif
	स्थिती FFI_TYPE_DOUBLE:
#जर _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#जर _CALL_ELF != 2
	      जर ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  अन्यथा
	    *next_arg.d = double_tmp;
	  जर (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_मोजणी++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
#जर _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#जर _CALL_ELF != 2
	      जर ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (दशांश) double_tmp;
# अन्यथा
		  next_arg.f[0] = (दशांश) double_tmp;
# endif
		}
#endif
	    }
	  अन्यथा
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (दशांश) double_tmp;
# अन्यथा
	      next_arg.f[0] = (दशांश) double_tmp;
# endif
	    }
	  जर (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_मोजणी++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  थांबा;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	स्थिती FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  जर (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  जर (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      साठी (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  जर (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  जर (vecarg_मोजणी < अंक_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    स_नकल (vec_base.f128++, cval + j, आकार (float128));
		  अन्यथा
		    स_नकल (next_arg.f128, cval + j, आकार (float128));
		  जर (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_मोजणी++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      थांबा;
	    }
	  जर (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      द्विगुण *cval = (द्विगुण *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      साठी (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *next_arg.d = double_tmp;
		  जर (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_मोजणी++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      थांबा;
	    }
#endif
	  जर (elt == FFI_TYPE_FLOAT)
	    {
	      दशांश *cval = (दशांश *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      साठी (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (दशांश) double_tmp;
# अन्यथा
		      next_arg.f[0] = (दशांश) double_tmp;
# endif
		    }
		  जर (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_मोजणी++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा जर (elt == FFI_TYPE_DOUBLE)
	    {
	      द्विगुण *cval = (द्विगुण *) *p_argv.v;
	      अचिन्हित पूर्णांक j;
	      साठी (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  अन्यथा
		    *next_arg.d = double_tmp;
		  जर (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_मोजणी++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  अन्यथा
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      अक्षर *cval = (अक्षर *) *p_argv.v;
	      माप_प्रकार hsize = (*ptr)->elements[0]->size;
	      अचिन्हित पूर्णांक j;
	      साठी (j = 0; j < 2; j++)
		{
		  अक्षर *half = cval + j * hsize;
		  अचिन्हित दीर्घ gprvalue;
		  निवड (elt)
		    {
		    स्थिती FFI_TYPE_UINT8:
		      gprvalue = *(अचिन्हित अक्षर *) half; थांबा;
		    स्थिती FFI_TYPE_SINT8:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित अक्षर *) half;
		      थांबा;
		    स्थिती FFI_TYPE_UINT16:
		      gprvalue = *(अचिन्हित लघु *) half; थांबा;
		    स्थिती FFI_TYPE_SINT16:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित लघु *) half;
		      थांबा;
		    स्थिती FFI_TYPE_UINT32:
		      gprvalue = *(अचिन्हित पूर्णांक *) half; थांबा;
		    स्थिती FFI_TYPE_INT:
		    स्थिती FFI_TYPE_SINT32:
		      gprvalue = (अचिन्हित दीर्घ) (दीर्घ) *(चिन्हित पूर्णांक *) half;
		      थांबा;
		    स्थिती FFI_TYPE_SINT64:
		    स्थिती FFI_TYPE_UINT64:
		    स्थिती FFI_TYPE_POINTER:
		    पूर्वनिर्धारित:
		      gprvalue = *(अचिन्हित दीर्घ *) half; थांबा;
		    }
		  *next_arg.ul++ = gprvalue;
		  जर (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  थांबा;
#endif

	स्थिती FFI_TYPE_STRUCT:
	  जर ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      जर (align > 16)
		align = 16;
	      जर (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  जर (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  जर (elt)
	    {
#जर _CALL_ELF == 2
	      संघ {
		शून्य *v;
		दशांश *f;
		द्विगुण *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              जर (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  करा
                    {
                      जर (vecarg_मोजणी < अंक_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        स_नकल (vec_base.f128++, arg.f128++, आकार (float128));
                      अन्यथा
		        स_नकल (next_arg.f128, arg.f128++, आकार (float128));
                      जर (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_मोजणी++;
                    }
                  पर्यंत (--elnum != 0);
                }
              अन्यथा
#endif
	      जर (elt == FFI_TYPE_FLOAT)
		{
		  करा
		    {
		      double_tmp = *arg.f++;
		      जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      अन्यथा
			*next_arg.f = (दशांश) double_tmp;
		      जर (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_मोजणी++;
		    }
		  पर्यंत (--elnum != 0);
		  जर ((next_arg.p & 7) != 0)
                    जर (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      अन्यथा
		करा
		  {
		    double_tmp = *arg.d++;
		    जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    अन्यथा
		      *next_arg.d = double_tmp;
		    जर (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_मोजणी++;
		  }
		पर्यंत (--elnum != 0);
#अन्यथा
	      जर (elt == FFI_TYPE_FLOAT)
		जा do_float;
	      अन्यथा
		जा do_double;
#endif
	    }
	  अन्यथा
	    {
	      शब्द = ((*ptr)->size + 7) / 8;
	      जर (next_arg.ul >= gpr_base.ul && next_arg.ul + शब्द > gpr_end.ul)
		{
		  माप_प्रकार first = gpr_end.c - next_arg.c;
		  स_नकल (next_arg.c, *p_argv.c, first);
		  स_नकल (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + शब्द * 8 - first;
		}
	      अन्यथा
		{
		  अक्षर *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  जर ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  स_नकल (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += शब्द;
		  जर (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  थांबा;

	स्थिती FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  जा putgpr;
	स्थिती FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  जा putgpr;
	स्थिती FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  जा putgpr;
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  जा putgpr;

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  जर (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  थांबा;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#जर _CALL_ELF == 2
#define MIN_CACHE_ओळ_SIZE 8

स्थिर शून्य
flush_icache (अक्षर *wraddr, अक्षर *xaddr, पूर्णांक size)
{
  पूर्णांक i;
  साठी (i = 0; i < size; i += MIN_CACHE_ओळ_SIZE)
    __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
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
  जर (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    परत FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#endif
    {
#जर _CALL_ELF == 2
      अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(शून्य **) &tramp[4] = (शून्य *) ffi_closure_LINUX64;
      *(शून्य **) &tramp[6] = codeloc;
      flush_icache ((अक्षर *) tramp, (अक्षर *) codeloc, 4 * 4);
#अन्यथा
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      शून्य **tramp = (शून्य **) &closure->tramp[0];
      स_नकल (&tramp[0], (शून्य **) ffi_closure_LINUX64, आकार (शून्य *));
      tramp[1] = codeloc;
      स_नकल (&tramp[2], (शून्य **) ffi_closure_LINUX64 + 1, आकार (शून्य *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}


पूर्णांक FFI_HIDDEN
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

  avalue = alloca (cif->nargs * आकार (शून्य *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  जर (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (शून्य *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#जर _CALL_ELF != 2
  nfixedargs = (अचिन्हित) -1;
  जर ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  पर्यंत (i < avn)
    {
      अचिन्हित पूर्णांक elt, elnum;

      निवड (arg_types[i]->type)
	{
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 7;
	  pst++;
	  थांबा;
#endif

	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 6;
	  pst++;
	  थांबा;
#endif

	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (अक्षर *) pst + 4;
	  pst++;
	  थांबा;
#endif

	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  जर ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      जर (align > 16)
		align = 16;
	      जर (align > 1)
		pst = (अचिन्हित दीर्घ *) FFI_ALIGN ((माप_प्रकार) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  जर (elt)
	    {
#जर _CALL_ELF == 2
	      संघ {
		शून्य *v;
		अचिन्हित दीर्घ *ul;
		दशांश *f;
		द्विगुण *d;
		float128 *f128;
		माप_प्रकार p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              जर (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  जर (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  अन्यथा
                    to.v = pst;
                }
              अन्यथा
#endif
	      जर (pfr + elnum <= end_pfr)
		to.v = pfr;
	      अन्यथा
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              जर (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  करा
                    {
                      जर (pvec < end_pvec && i < nfixedargs)
		        स_नकल (to.f128, pvec++, आकार (float128));
                      अन्यथा
		        स_नकल (to.f128, from.f128, आकार (float128));
                      to.f128++;
                      from.f128++;
                    }
                  पर्यंत (--elnum != 0);
                }
              अन्यथा
#endif
	      जर (elt == FFI_TYPE_FLOAT)
		{
		  करा
		    {
		      जर (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (दशांश) pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  पर्यंत (--elnum != 0);
		}
	      अन्यथा
		{
		  करा
		    {
		      जर (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      अन्यथा
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  पर्यंत (--elnum != 0);
		}
#अन्यथा
	      जर (elt == FFI_TYPE_FLOAT)
		जा do_float;
	      अन्यथा
		जा do_double;
#endif
	    }
	  अन्यथा
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      जर (arg_types[i]->size < 8)
		avalue[i] = (अक्षर *) pst + 8 - arg_types[i]->size;
	      अन्यथा
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  थांबा;

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
          जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              जर (((अचिन्हित दीर्घ) pst & 0xF) != 0)
                ++pst;
              जर (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              अन्यथा
                avalue[i] = pst;
              pst += 2;
              थांबा;
            }
          अन्यथा जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      जर (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      अन्यथा
		{
		  जर (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(अचिन्हित दीर्घ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      थांबा;
	    }
	  /* Fall through.  */
#endif
	स्थिती FFI_TYPE_DOUBLE:
#जर _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  जर (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    avalue[i] = pst;
	  pst++;
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
#जर _CALL_ELF != 2
	do_float:
#endif
	  जर (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (दशांश) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (अक्षर *) pst + 4;
#अन्यथा
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  थांबा;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	स्थिती FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    अचिन्हित पूर्णांक j;
	    elt = arg_types[i]->elements[0]->type;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    जर (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    जर (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * आकार (float128));
		जर (((अचिन्हित दीर्घ) pst & 0xF) != 0)
		  ++pst;
		साठी (j = 0; j < 2; j++)
		  {
		    जर (pvec < end_pvec && i < nfixedargs)
		      स_नकल (&cval[j], pvec++, आकार (float128));
		    अन्यथा
		      स_नकल (&cval[j], pst, आकार (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		थांबा;
	      }
	    जर (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		द्विगुण *cval = alloca (4 * आकार (द्विगुण));
		साठी (j = 0; j < 4; j++)
		  {
		    जर (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    अन्यथा
		      cval[j] = *(द्विगुण *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		थांबा;
	      }
#endif
	    जर (elt == FFI_TYPE_FLOAT)
	      {
		दशांश *cval = alloca (2 * आकार (दशांश));
		साठी (j = 0; j < 2; j++)
		  {
		    जर (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (दशांश) pfr->d;
			pfr++;
		      }
		    अन्यथा
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((दशांश *) pst)[1];
#अन्यथा
			cval[j] = ((दशांश *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    अन्यथा जर (elt == FFI_TYPE_DOUBLE)
	      {
		द्विगुण *cval = alloca (2 * आकार (द्विगुण));
		साठी (j = 0; j < 2; j++)
		  {
		    जर (pfr < end_pfr && i < nfixedargs)
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
		अक्षर *cval = alloca (2 * hsize);
		साठी (j = 0; j < 2; j++)
		  {
		    अक्षर *src;
#ifndef __LITTLE_ENDIAN__
		    src = (अक्षर *) pst + (8 - hsize);
#अन्यथा
		    src = (अक्षर *) pst;
#endif
		    स_नकल (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  थांबा;
#endif

	पूर्वनिर्धारित:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      परत PPC_LD_NONE;
    स्थिती FFI_TYPE_FLOAT:
      परत PPC_LD_F32;
    स्थिती FFI_TYPE_DOUBLE:
      परत PPC_LD_F64;
#जर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      जर ((cif->flags & FLAG_RETURNS_VEC) != 0)
	परत PPC64_LD_VECTOR;
      परत PPC_LD_F128;
#endif
    स्थिती FFI_TYPE_UINT8:
      परत PPC_LD_U8;
    स्थिती FFI_TYPE_SINT8:
      परत PPC_LD_S8;
    स्थिती FFI_TYPE_UINT16:
      परत PPC_LD_U16;
    स्थिती FFI_TYPE_SINT16:
      परत PPC_LD_S16;
    स्थिती FFI_TYPE_UINT32:
      परत PPC_LD_U32;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      परत PPC_LD_S32;
    स्थिती FFI_TYPE_POINTER:
      परत PPC_LD_PTR;
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      परत PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    स्थिती FFI_TYPE_COMPLEX:
      {
	पूर्णांक inner = cif->rtype->elements[0]->type;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	जर (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    जर ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      परत PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	जर (inner == FFI_TYPE_FLOAT)
	  परत PPC64_LD_FLOAT_HOMOG;
	जर (inner == FFI_TYPE_DOUBLE)
	  परत PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  अक्षर *rv = rvalue;
	  अचिन्हित दीर्घ re, im;
	  निवड (inner)
	    {
	    स्थिती FFI_TYPE_UINT8:
	      re = ((अचिन्हित अक्षर *) rv)[0];
	      im = ((अचिन्हित अक्षर *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_SINT8:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित अक्षर *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित अक्षर *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_UINT16:
	      re = ((अचिन्हित लघु *) rv)[0];
	      im = ((अचिन्हित लघु *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_SINT16:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित लघु *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_UINT32:
	      re = ((अचिन्हित पूर्णांक *) rv)[0];
	      im = ((अचिन्हित पूर्णांक *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_INT:
	    स्थिती FFI_TYPE_SINT32:
	      re = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णांक *) rv)[0];
	      im = (अचिन्हित दीर्घ) (दीर्घ) ((चिन्हित पूर्णांक *) rv)[1];
	      थांबा;
	    स्थिती FFI_TYPE_SINT64:
	    स्थिती FFI_TYPE_UINT64:
	    स्थिती FFI_TYPE_POINTER:
	    पूर्वनिर्धारित:
	      re = ((अचिन्हित दीर्घ *) rv)[0];
	      im = ((अचिन्हित दीर्घ *) rv)[1];
	      थांबा;
	    }
	  ((अचिन्हित दीर्घ *) rv)[0] = re;
	  ((अचिन्हित दीर्घ *) rv)[1] = im;
	  परत PPC_LD_R3R4;
	}
      }
#endif
    स्थिती FFI_TYPE_STRUCT:
      जर ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  जर ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      निवड (cif->rtype->size)
		{
		स्थिती 0:
		  परत PPC_LD_NONE;
		स्थिती 1:
		  परत PPC_LD_U8;
		स्थिती 2:
		  परत PPC_LD_U16;
		स्थिती 3:
		  परत PPC64_LD_STRUCT_3;
		स्थिती 4:
		  परत PPC_LD_U32;
		स्थिती 5:
		  परत PPC64_LD_STRUCT_5;
		स्थिती 6:
		  परत PPC64_LD_STRUCT_6;
		स्थिती 7:
		  परत PPC64_LD_STRUCT_7;
		स्थिती 8 ... 16:
		  परत PPC_LD_R3R4;
		}
	      थांबा;
	    }
	  जर ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    परत PPC64_LD_VECTOR_HOMOG;
	  जर ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    परत PPC64_LD_DOUBLE_HOMOG;
	  परत PPC64_LD_FLOAT_HOMOG;
        }
      परत PPC_LD_NONE;
    }
  पात();
}
#endif

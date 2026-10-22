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

#অন্তর্ভাব "ffi.h"
#অন্তর্ভাব <tramp.h>
#অন্তর্ভাব <stdlib.h>

#ifdef POWERPC64
#অন্তর্ভাব "ffi_common.h"
#অন্তর্ভাব "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
গণনা {
  অঙ্ক_GPR_ARG_REGISTERS64 = 8,
  অঙ্ক_FPR_ARG_REGISTERS64 = 13,
  অঙ্ক_VEC_ARG_REGISTERS64 = 12,
};
গণনা { ASM_NEEDS_REGISTERS64 = 4 };


#যদি HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
শূন্য FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  যদি ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  অন্যথা
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


স্থির unsigned পূর্ণাঙ্ক
discover_homogeneous_aggregate (ffi_abi abi,
                                ধ্রুব ffi_type *t,
                                unsigned পূর্ণাঙ্ক *elnum)
{
  বিকল্প (t->type)
    {
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    পক্ষ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      যদি ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          প্রত্যাগম FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      অন্যথা যদি ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          প্রত্যাগম FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    পক্ষ FFI_TYPE_FLOAT:
    পক্ষ FFI_TYPE_DOUBLE:
      *elnum = 1;
      প্রত্যাগম (পূর্ণাঙ্ক) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    পক্ষ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned পূর্ণাঙ্ক inner_elnum = 0;
	unsigned পূর্ণাঙ্ক inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	যদি (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    প্রত্যাগম inner;
	  }
	প্রত্যাগম 0;
      }
#endif

    পক্ষ FFI_TYPE_STRUCT:;
      {
	unsigned পূর্ণাঙ্ক base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	যাবত্ (*el)
	  {
	    unsigned পূর্ণাঙ্ক el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    যদি (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      প্রত্যাগম 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#যদি _CALL_ELF == 2
	    যদি (total_elnum > 8)
	      প্রত্যাগম 0;
#অন্যথা
	    যদি (total_elnum > 1)
	      প্রত্যাগম 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	প্রত্যাগম base_elt;
      }

    স্বভাব:
      প্রত্যাগম 0;
    }
}


/* Perform machine dependent cif processing */
স্থির ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_গুনতি = 0, intarg_গুনতি = 0, vecarg_গুনতি = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#যদি FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    প্রত্যাগম FFI_BAD_ABI;
#নোচেদ্যদি !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    প্রত্যাগম FFI_BAD_ABI;
#অন্যথা
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    প্রত্যাগম FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#যদি _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * পরিমাণ (long);

  /* Space for the general registers.  */
  bytes += অঙ্ক_GPR_ARG_REGISTERS64 * পরিমাণ (long);
#অন্যথা
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * পরিমাণ (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * অঙ্ক_GPR_ARG_REGISTERS64 * পরিমাণ (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#যদি _CALL_ELF == 2
homogeneous:
#endif
  বিকল্প (rtype)
    {
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    পক্ষ FFI_TYPE_LONGDOUBLE:
      যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ভঙ্গ;
        }
      যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    পক্ষ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    পক্ষ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ভঙ্গ;

    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
#যদি _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      যদি (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          গচ্ছ homogeneous;
        }
      যদি (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ভঙ্গ;
        }
#endif
      intarg_গুনতি++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    পক্ষ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ভঙ্গ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    পক্ষ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      বিকল্প (rtype)
        {
        পক্ষ FFI_TYPE_FLOAT:
        পক্ষ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          গচ্ছ homogeneous;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        পক্ষ FFI_TYPE_LONGDOUBLE:
          যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ভঙ্গ;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          গচ্ছ homogeneous;
#endif
        পক্ষ FFI_TYPE_INT:
        পক্ষ FFI_TYPE_SINT8:  পক্ষ FFI_TYPE_UINT8:
        পক্ষ FFI_TYPE_SINT16: পক্ষ FFI_TYPE_UINT16:
        পক্ষ FFI_TYPE_SINT32: পক্ষ FFI_TYPE_UINT32:
        পক্ষ FFI_TYPE_SINT64: পক্ষ FFI_TYPE_UINT64:
        পক্ষ FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ভঙ্গ;
        স্বভাব:
          প্রত্যাগম FFI_BAD_TYPEDEF;
        }
      ভঙ্গ;
#endif

    স্বভাব:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ভঙ্গ;
    }

  প্রতি (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned পূর্ণাঙ্ক align;

      বিকল্প ((*ptr)->type)
	{
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	পক্ষ FFI_TYPE_LONGDOUBLE:
          যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_গুনতি++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_গুনতি = (intarg_গুনতি + 3) & ~0x1;
              যদি (vecarg_গুনতি > অঙ্ক_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ভঙ্গ;
            }
	  যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_গুনতি++;
	      intarg_গুনতি++;
	    }
	  /* Fall through.  */
#endif
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_FLOAT:
	  fparg_গুনতি++;
	  intarg_গুনতি++;
	  যদি (fparg_গুনতি > অঙ্ক_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_STRUCT:
	  যদি ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      যদি (align > 16)
		align = 16;
	      align = align / 8;
	      যদি (align > 1)
		intarg_গুনতি = FFI_ALIGN (intarg_গুনতি, align);
	    }
	  intarg_গুনতি += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          যদি (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_গুনতি += elnum;
              যদি (vecarg_গুনতি > অঙ্ক_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ভঙ্গ;
            }
	  অন্যথা
#endif
	  যদি (elt)
	    {
	      fparg_গুনতি += elnum;
	      যদি (fparg_গুনতি > অঙ্ক_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  অন্যথা
	    {
	      যদি (intarg_গুনতি > অঙ্ক_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ভঙ্গ;

	পক্ষ FFI_TYPE_POINTER:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT16:
	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT8:
	পক্ষ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_গুনতি++;
	  যদি (intarg_গুনতি > অঙ্ক_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ভঙ্গ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	পক্ষ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  যদি (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  বিকল্প (elt)
	    {
	    পক্ষ FFI_TYPE_FLOAT:
	    পক্ষ FFI_TYPE_DOUBLE:
	      fparg_গুনতি += 2;
	      intarg_গুনতি += 2;
	      যদি (fparg_গুনতি > অঙ্ক_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ভঙ্গ;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    পক্ষ FFI_TYPE_LONGDOUBLE:
	      যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_গুনতি += 2;
		  intarg_গুনতি = (intarg_গুনতি + 1) & ~0x1;
		  intarg_গুনতি += 4;
		  যদি (vecarg_গুনতি > অঙ্ক_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ভঙ্গ;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_গুনতি += 4;
	      intarg_গুনতি += 4;
	      যদি (fparg_গুনতি > অঙ্ক_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ভঙ্গ;
#endif
	    পক্ষ FFI_TYPE_INT:
	    পক্ষ FFI_TYPE_SINT8:  পক্ষ FFI_TYPE_UINT8:
	    পক্ষ FFI_TYPE_SINT16: পক্ষ FFI_TYPE_UINT16:
	    পক্ষ FFI_TYPE_SINT32: পক্ষ FFI_TYPE_UINT32:
	    পক্ষ FFI_TYPE_SINT64: পক্ষ FFI_TYPE_UINT64:
	    পক্ষ FFI_TYPE_POINTER:
	      intarg_গুনতি += 2;
	      ভঙ্গ;
	    স্বভাব:
	      প্রত্যাগম FFI_BAD_TYPEDEF;
	    }
	  যদি (intarg_গুনতি > অঙ্ক_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ভঙ্গ;
#endif

	স্বভাব:
	  FFI_ASSERT (0);
	}
    }

  যদি (fparg_গুনতি != 0)
    flags |= FLAG_FP_ARGUMENTS;
  যদি (intarg_গুনতি > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  যদি (vecarg_গুনতি != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  যদি (fparg_গুনতি != 0)
    bytes += অঙ্ক_FPR_ARG_REGISTERS64 * পরিমাণ (দ্বিগুণ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  যদি (vecarg_গুনতি != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += অঙ্ক_VEC_ARG_REGISTERS64 * পরিমাণ (float128);
  }

  /* Stack space.  */
#যদি _CALL_ELF == 2
  যদি ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_গুনতি * পরিমাণ (long);
#অন্যথা
  যদি (intarg_গুনতি > অঙ্ক_GPR_ARG_REGISTERS64)
    bytes += (intarg_গুনতি - অঙ্ক_GPR_ARG_REGISTERS64) * পরিমাণ (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  প্রত্যাগম FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  যদি ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#যদি _CALL_ELF != 2
  অন্যথা যদি (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  অন্যথা
    প্রত্যাগম FFI_BAD_ABI;
  প্রত্যাগম ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned পূর্ণাঙ্ক nfixedargs,
			  unsigned পূর্ণাঙ্ক ntotalargs MAYBE_UNUSED)
{
  যদি ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#যদি _CALL_ELF != 2
  অন্যথা যদি (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  অন্যথা
    প্রত্যাগম FFI_BAD_ABI;
#যদি _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  প্রত্যাগম ffi_prep_cif_linux64_core (cif);
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

শূন্য FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *ধ্রুব স্তূপ)
{
  ধ্রুব unsigned long bytes = ecif->cif->bytes;
  ধ্রুব unsigned long flags = ecif->cif->flags;

  প্রকারনির্দেশ সঙ্ঘ
  {
    বর্ণ *c;
    unsigned long *ul;
    ভিন্ন *f;
    দ্বিগুণ *d;
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
  unsigned পূর্ণাঙ্ক fparg_গুনতি;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned পূর্ণাঙ্ক vecarg_গুনতি;

  unsigned পূর্ণাঙ্ক i, শব্দগুলি, nargs, nfixedargs;
  ffi_type **ptr;
  দ্বিগুণ double_tmp;
  সঙ্ঘ
  {
    শূন্য **v;
    বর্ণ **c;
    signed বর্ণ **sc;
    unsigned বর্ণ **uc;
    signed short **ss;
    unsigned short **us;
    signed পূর্ণাঙ্ক **si;
    unsigned পূর্ণাঙ্ক **ui;
    unsigned long **ul;
    ভিন্ন **f;
    দ্বিগুণ **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (বর্ণ *) স্তূপ + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - অঙ্ক_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + অঙ্ক_GPR_ARG_REGISTERS64;
#যদি _CALL_ELF == 2
  rest.ul = স্তূপ + 4 + অঙ্ক_GPR_ARG_REGISTERS64;
#অন্যথা
  rest.ul = স্তূপ + 6 + অঙ্ক_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - অঙ্ক_FPR_ARG_REGISTERS64;
  fparg_গুনতি = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  যদি (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  অন্যথা
    vec_base.p = gpr_base.p;
  vec_base.f128 -= অঙ্ক_VEC_ARG_REGISTERS64;
  vecarg_গুনতি = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (বর্ণ *) স্তূপ & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  যদি (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (বর্ণ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#যদি _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  যদি ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  প্রতি (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned পূর্ণাঙ্ক elt, elnum;

      বিকল্প ((*ptr)->type)
	{
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	পক্ষ FFI_TYPE_LONGDOUBLE:
          যদি ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              যদি (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              যদি (vecarg_গুনতি < অঙ্ক_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, পরিমাণ (float128));
              অন্যথা
		memcpy (next_arg.f128, *p_argv.f128, পরিমাণ (float128));
              যদি (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_গুনতি++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ভঙ্গ;
            }
	  যদি ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# যদি _CALL_ELF != 2
		  যদি ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      অন্যথা
		*next_arg.d = double_tmp;
	      যদি (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_গুনতি++;
	      double_tmp = (*p_argv.d)[1];
	      যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# যদি _CALL_ELF != 2
		  যদি ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      অন্যথা
		*next_arg.d = double_tmp;
	      যদি (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_গুনতি++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ভঙ্গ;
	    }
	  /* Fall through.  */
#endif
	পক্ষ FFI_TYPE_DOUBLE:
#যদি _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#যদি _CALL_ELF != 2
	      যদি ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  অন্যথা
	    *next_arg.d = double_tmp;
	  যদি (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_গুনতি++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
#যদি _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#যদি _CALL_ELF != 2
	      যদি ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (ভিন্ন) double_tmp;
# অন্যথা
		  next_arg.f[0] = (ভিন্ন) double_tmp;
# endif
		}
#endif
	    }
	  অন্যথা
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (ভিন্ন) double_tmp;
# অন্যথা
	      next_arg.f[0] = (ভিন্ন) double_tmp;
# endif
	    }
	  যদি (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_গুনতি++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ভঙ্গ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	পক্ষ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  যদি (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  যদি (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned পূর্ণাঙ্ক j;
	      প্রতি (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  যদি (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  যদি (vecarg_গুনতি < অঙ্ক_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, পরিমাণ (float128));
		  অন্যথা
		    memcpy (next_arg.f128, cval + j, পরিমাণ (float128));
		  যদি (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_গুনতি++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ভঙ্গ;
	    }
	  যদি (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      দ্বিগুণ *cval = (দ্বিগুণ *) *p_argv.v;
	      unsigned পূর্ণাঙ্ক j;
	      প্রতি (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  অন্যথা
		    *next_arg.d = double_tmp;
		  যদি (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_গুনতি++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ভঙ্গ;
	    }
#endif
	  যদি (elt == FFI_TYPE_FLOAT)
	    {
	      ভিন্ন *cval = (ভিন্ন *) *p_argv.v;
	      unsigned পূর্ণাঙ্ক j;
	      প্রতি (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  অন্যথা
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (ভিন্ন) double_tmp;
# অন্যথা
		      next_arg.f[0] = (ভিন্ন) double_tmp;
# endif
		    }
		  যদি (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_গুনতি++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  অন্যথা যদি (elt == FFI_TYPE_DOUBLE)
	    {
	      দ্বিগুণ *cval = (দ্বিগুণ *) *p_argv.v;
	      unsigned পূর্ণাঙ্ক j;
	      প্রতি (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  অন্যথা
		    *next_arg.d = double_tmp;
		  যদি (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_গুনতি++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  অন্যথা
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      বর্ণ *cval = (বর্ণ *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned পূর্ণাঙ্ক j;
	      প্রতি (j = 0; j < 2; j++)
		{
		  বর্ণ *half = cval + j * hsize;
		  unsigned long gprvalue;
		  বিকল্প (elt)
		    {
		    পক্ষ FFI_TYPE_UINT8:
		      gprvalue = *(unsigned বর্ণ *) half; ভঙ্গ;
		    পক্ষ FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed বর্ণ *) half;
		      ভঙ্গ;
		    পক্ষ FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; ভঙ্গ;
		    পক্ষ FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      ভঙ্গ;
		    পক্ষ FFI_TYPE_UINT32:
		      gprvalue = *(unsigned পূর্ণাঙ্ক *) half; ভঙ্গ;
		    পক্ষ FFI_TYPE_INT:
		    পক্ষ FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed পূর্ণাঙ্ক *) half;
		      ভঙ্গ;
		    পক্ষ FFI_TYPE_SINT64:
		    পক্ষ FFI_TYPE_UINT64:
		    পক্ষ FFI_TYPE_POINTER:
		    স্বভাব:
		      gprvalue = *(unsigned long *) half; ভঙ্গ;
		    }
		  *next_arg.ul++ = gprvalue;
		  যদি (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_STRUCT:
	  যদি ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      যদি (align > 16)
		align = 16;
	      যদি (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  যদি (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  যদি (elt)
	    {
#যদি _CALL_ELF == 2
	      সঙ্ঘ {
		শূন্য *v;
		ভিন্ন *f;
		দ্বিগুণ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              যদি (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  কুরু
                    {
                      যদি (vecarg_গুনতি < অঙ্ক_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, পরিমাণ (float128));
                      অন্যথা
		        memcpy (next_arg.f128, arg.f128++, পরিমাণ (float128));
                      যদি (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_গুনতি++;
                    }
                  যাবত্ (--elnum != 0);
                }
              অন্যথা
#endif
	      যদি (elt == FFI_TYPE_FLOAT)
		{
		  কুরু
		    {
		      double_tmp = *arg.f++;
		      যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      অন্যথা
			*next_arg.f = (ভিন্ন) double_tmp;
		      যদি (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_গুনতি++;
		    }
		  যাবত্ (--elnum != 0);
		  যদি ((next_arg.p & 7) != 0)
                    যদি (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      অন্যথা
		কুরু
		  {
		    double_tmp = *arg.d++;
		    যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    অন্যথা
		      *next_arg.d = double_tmp;
		    যদি (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_গুনতি++;
		  }
		যাবত্ (--elnum != 0);
#অন্যথা
	      যদি (elt == FFI_TYPE_FLOAT)
		গচ্ছ do_float;
	      অন্যথা
		গচ্ছ do_double;
#endif
	    }
	  অন্যথা
	    {
	      শব্দগুলি = ((*ptr)->size + 7) / 8;
	      যদি (next_arg.ul >= gpr_base.ul && next_arg.ul + শব্দগুলি > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + শব্দগুলি * 8 - first;
		}
	      অন্যথা
		{
		  বর্ণ *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  যদি ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += শব্দগুলি;
		  যদি (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  গচ্ছ putgpr;

	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  যদি (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  ভঙ্গ;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#যদি _CALL_ELF == 2
#define MIN_CACHE_সারি_SIZE 8

স্থির শূন্য
flush_icache (বর্ণ *wraddr, বর্ণ *xaddr, পূর্ণাঙ্ক size)
{
  পূর্ণাঙ্ক i;
  প্রতি (i = 0; i < size; i += MIN_CACHE_সারি_SIZE)
    __asm__ চঞ্চল ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ চঞ্চল ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
			      শূন্য *user_data,
			      শূন্য *codeloc)
{
  যদি (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    প্রত্যাগম FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      শূন্য (*dest)(শূন্য) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  অন্যথা
#endif
    {
#যদি _CALL_ELF == 2
      unsigned পূর্ণাঙ্ক *tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(শূন্য **) &tramp[4] = (শূন্য *) ffi_closure_LINUX64;
      *(শূন্য **) &tramp[6] = codeloc;
      flush_icache ((বর্ণ *) tramp, (বর্ণ *) codeloc, 4 * 4);
#অন্যথা
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      শূন্য **tramp = (শূন্য **) &closure->tramp[0];
      memcpy (&tramp[0], (শূন্য **) ffi_closure_LINUX64, পরিমাণ (শূন্য *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (শূন্য **) ffi_closure_LINUX64 + 1, পরিমাণ (শূন্য *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  প্রত্যাগম FFI_OK;
}


পূর্ণাঙ্ক FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
			    শূন্য *user_data,
			    শূন্য *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  শূন্য **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + অঙ্ক_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + অঙ্ক_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * পরিমাণ (শূন্য *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  যদি (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (শূন্য *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#যদি _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  যদি ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  যাবত্ (i < avn)
    {
      unsigned পূর্ণাঙ্ক elt, elnum;

      বিকল্প (arg_types[i]->type)
	{
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (বর্ণ *) pst + 7;
	  pst++;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (বর্ণ *) pst + 6;
	  pst++;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (বর্ণ *) pst + 4;
	  pst++;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_STRUCT:
	  যদি ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      যদি (align > 16)
		align = 16;
	      যদি (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  যদি (elt)
	    {
#যদি _CALL_ELF == 2
	      সঙ্ঘ {
		শূন্য *v;
		unsigned long *ul;
		ভিন্ন *f;
		দ্বিগুণ *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              যদি (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  যদি (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  অন্যথা
                    to.v = pst;
                }
              অন্যথা
#endif
	      যদি (pfr + elnum <= end_pfr)
		to.v = pfr;
	      অন্যথা
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              যদি (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  কুরু
                    {
                      যদি (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, পরিমাণ (float128));
                      অন্যথা
		        memcpy (to.f128, from.f128, পরিমাণ (float128));
                      to.f128++;
                      from.f128++;
                    }
                  যাবত্ (--elnum != 0);
                }
              অন্যথা
#endif
	      যদি (elt == FFI_TYPE_FLOAT)
		{
		  কুরু
		    {
		      যদি (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (ভিন্ন) pfr->d;
			  pfr++;
			}
		      অন্যথা
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  যাবত্ (--elnum != 0);
		}
	      অন্যথা
		{
		  কুরু
		    {
		      যদি (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      অন্যথা
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  যাবত্ (--elnum != 0);
		}
#অন্যথা
	      যদি (elt == FFI_TYPE_FLOAT)
		গচ্ছ do_float;
	      অন্যথা
		গচ্ছ do_double;
#endif
	    }
	  অন্যথা
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      যদি (arg_types[i]->size < 8)
		avalue[i] = (বর্ণ *) pst + 8 - arg_types[i]->size;
	      অন্যথা
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  ভঙ্গ;

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	পক্ষ FFI_TYPE_LONGDOUBLE:
          যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              যদি (((unsigned long) pst & 0xF) != 0)
                ++pst;
              যদি (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              অন্যথা
                avalue[i] = pst;
              pst += 2;
              ভঙ্গ;
            }
          অন্যথা যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      যদি (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      অন্যথা
		{
		  যদি (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ভঙ্গ;
	    }
	  /* Fall through.  */
#endif
	পক্ষ FFI_TYPE_DOUBLE:
#যদি _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  যদি (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
	    avalue[i] = pst;
	  pst++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_FLOAT:
#যদি _CALL_ELF != 2
	do_float:
#endif
	  যদি (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ভিন্ন) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (বর্ণ *) pst + 4;
#অন্যথা
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  ভঙ্গ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	পক্ষ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned পূর্ণাঙ্ক j;
	    elt = arg_types[i]->elements[0]->type;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    যদি (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    যদি (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * পরিমাণ (float128));
		যদি (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		প্রতি (j = 0; j < 2; j++)
		  {
		    যদি (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, পরিমাণ (float128));
		    অন্যথা
		      memcpy (&cval[j], pst, পরিমাণ (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ভঙ্গ;
	      }
	    যদি (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		দ্বিগুণ *cval = alloca (4 * পরিমাণ (দ্বিগুণ));
		প্রতি (j = 0; j < 4; j++)
		  {
		    যদি (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    অন্যথা
		      cval[j] = *(দ্বিগুণ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ভঙ্গ;
	      }
#endif
	    যদি (elt == FFI_TYPE_FLOAT)
	      {
		ভিন্ন *cval = alloca (2 * পরিমাণ (ভিন্ন));
		প্রতি (j = 0; j < 2; j++)
		  {
		    যদি (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (ভিন্ন) pfr->d;
			pfr++;
		      }
		    অন্যথা
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((ভিন্ন *) pst)[1];
#অন্যথা
			cval[j] = ((ভিন্ন *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    অন্যথা যদি (elt == FFI_TYPE_DOUBLE)
	      {
		দ্বিগুণ *cval = alloca (2 * পরিমাণ (দ্বিগুণ));
		প্রতি (j = 0; j < 2; j++)
		  {
		    যদি (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    অন্যথা
		      cval[j] = *(দ্বিগুণ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    অন্যথা
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		বর্ণ *cval = alloca (2 * hsize);
		প্রতি (j = 0; j < 2; j++)
		  {
		    বর্ণ *src;
#ifndef __LITTLE_ENDIAN__
		    src = (বর্ণ *) pst + (8 - hsize);
#অন্যথা
		    src = (বর্ণ *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ভঙ্গ;
#endif

	স্বভাব:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  বিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      প্রত্যাগম PPC_LD_NONE;
    পক্ষ FFI_TYPE_FLOAT:
      প্রত্যাগম PPC_LD_F32;
    পক্ষ FFI_TYPE_DOUBLE:
      প্রত্যাগম PPC_LD_F64;
#যদি FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    পক্ষ FFI_TYPE_LONGDOUBLE:
      যদি ((cif->flags & FLAG_RETURNS_VEC) != 0)
	প্রত্যাগম PPC64_LD_VECTOR;
      প্রত্যাগম PPC_LD_F128;
#endif
    পক্ষ FFI_TYPE_UINT8:
      প্রত্যাগম PPC_LD_U8;
    পক্ষ FFI_TYPE_SINT8:
      প্রত্যাগম PPC_LD_S8;
    পক্ষ FFI_TYPE_UINT16:
      প্রত্যাগম PPC_LD_U16;
    পক্ষ FFI_TYPE_SINT16:
      প্রত্যাগম PPC_LD_S16;
    পক্ষ FFI_TYPE_UINT32:
      প্রত্যাগম PPC_LD_U32;
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      প্রত্যাগম PPC_LD_S32;
    পক্ষ FFI_TYPE_POINTER:
      প্রত্যাগম PPC_LD_PTR;
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      প্রত্যাগম PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    পক্ষ FFI_TYPE_COMPLEX:
      {
	পূর্ণাঙ্ক inner = cif->rtype->elements[0]->type;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	যদি (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    যদি ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      প্রত্যাগম PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	যদি (inner == FFI_TYPE_FLOAT)
	  প্রত্যাগম PPC64_LD_FLOAT_HOMOG;
	যদি (inner == FFI_TYPE_DOUBLE)
	  প্রত্যাগম PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  বর্ণ *rv = rvalue;
	  unsigned long re, im;
	  বিকল্প (inner)
	    {
	    পক্ষ FFI_TYPE_UINT8:
	      re = ((unsigned বর্ণ *) rv)[0];
	      im = ((unsigned বর্ণ *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed বর্ণ *) rv)[0];
	      im = (unsigned long) (long) ((signed বর্ণ *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_UINT32:
	      re = ((unsigned পূর্ণাঙ্ক *) rv)[0];
	      im = ((unsigned পূর্ণাঙ্ক *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_INT:
	    পক্ষ FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed পূর্ণাঙ্ক *) rv)[0];
	      im = (unsigned long) (long) ((signed পূর্ণাঙ্ক *) rv)[1];
	      ভঙ্গ;
	    পক্ষ FFI_TYPE_SINT64:
	    পক্ষ FFI_TYPE_UINT64:
	    পক্ষ FFI_TYPE_POINTER:
	    স্বভাব:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      ভঙ্গ;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  প্রত্যাগম PPC_LD_R3R4;
	}
      }
#endif
    পক্ষ FFI_TYPE_STRUCT:
      যদি ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  যদি ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      বিকল্প (cif->rtype->size)
		{
		পক্ষ 0:
		  প্রত্যাগম PPC_LD_NONE;
		পক্ষ 1:
		  প্রত্যাগম PPC_LD_U8;
		পক্ষ 2:
		  প্রত্যাগম PPC_LD_U16;
		পক্ষ 3:
		  প্রত্যাগম PPC64_LD_STRUCT_3;
		পক্ষ 4:
		  প্রত্যাগম PPC_LD_U32;
		পক্ষ 5:
		  প্রত্যাগম PPC64_LD_STRUCT_5;
		পক্ষ 6:
		  প্রত্যাগম PPC64_LD_STRUCT_6;
		পক্ষ 7:
		  প্রত্যাগম PPC64_LD_STRUCT_7;
		পক্ষ 8 ... 16:
		  প্রত্যাগম PPC_LD_R3R4;
		}
	      ভঙ্গ;
	    }
	  যদি ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    প্রত্যাগম PPC64_LD_VECTOR_HOMOG;
	  যদি ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    প্রত্যাগম PPC64_LD_DOUBLE_HOMOG;
	  প্রত্যাগম PPC64_LD_FLOAT_HOMOG;
        }
      প্রত্যাগম PPC_LD_NONE;
    }
  abort();
}
#endif

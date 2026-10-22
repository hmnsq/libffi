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

#അന്തര്ഭാവ "ffi.h"
#അന്തര്ഭാവ <tramp.h>
#അന്തര്ഭാവ <stdlib.h>

#ifdef POWERPC64
#അന്തര്ഭാവ "ffi_common.h"
#അന്തര്ഭാവ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
എണ്ണൽ {
  അക്കം_GPR_ARG_REGISTERS64 = 8,
  അക്കം_FPR_ARG_REGISTERS64 = 13,
  അക്കം_VEC_ARG_REGISTERS64 = 12,
};
എണ്ണൽ { ASM_NEEDS_REGISTERS64 = 4 };


#എങ്കിൽ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ശൂന്യം FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  എങ്കിൽ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  അല്ലെങ്കിൽ
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


സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ
discover_homogeneous_aggregate (ffi_abi abi,
                                സ്ഥിരം ffi_type *t,
                                unsigned പൂർണ്ണസംഖ്യ *elnum)
{
  തിരഞ്ഞെടുപ്പ് (t->type)
    {
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      എങ്കിൽ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          തിരികെ FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      അല്ലെങ്കിൽ എങ്കിൽ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          തിരികെ FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      *elnum = 1;
      തിരികെ (പൂർണ്ണസംഖ്യ) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    അവസ്ഥ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned പൂർണ്ണസംഖ്യ inner_elnum = 0;
	unsigned പൂർണ്ണസംഖ്യ inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	എങ്കിൽ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    തിരികെ inner;
	  }
	തിരികെ 0;
      }
#endif

    അവസ്ഥ FFI_TYPE_STRUCT:;
      {
	unsigned പൂർണ്ണസംഖ്യ base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	ആകുമ്പോൾ (*el)
	  {
	    unsigned പൂർണ്ണസംഖ്യ el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    എങ്കിൽ (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      തിരികെ 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#എങ്കിൽ _CALL_ELF == 2
	    എങ്കിൽ (total_elnum > 8)
	      തിരികെ 0;
#അല്ലെങ്കിൽ
	    എങ്കിൽ (total_elnum > 1)
	      തിരികെ 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	തിരികെ base_elt;
      }

    സ്ഥിരസ്ഥിതി:
      തിരികെ 0;
    }
}


/* Perform machine dependent cif processing */
സ്ഥിതം ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_എണ്ണം = 0, intarg_എണ്ണം = 0, vecarg_എണ്ണം = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    തിരികെ FFI_BAD_ABI;
#അല്ലെങ്കിലെങ്കിൽ !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    തിരികെ FFI_BAD_ABI;
#അല്ലെങ്കിൽ
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    തിരികെ FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#എങ്കിൽ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * വലുപ്പം (long);

  /* Space for the general registers.  */
  bytes += അക്കം_GPR_ARG_REGISTERS64 * വലുപ്പം (long);
#അല്ലെങ്കിൽ
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * വലുപ്പം (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * അക്കം_GPR_ARG_REGISTERS64 * വലുപ്പം (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#എങ്കിൽ _CALL_ELF == 2
homogeneous:
#endif
  തിരഞ്ഞെടുപ്പ് (rtype)
    {
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          നിർത്തുക;
        }
      എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
#എങ്കിൽ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      എങ്കിൽ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          പോകുക homogeneous;
        }
      എങ്കിൽ (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          നിർത്തുക;
        }
#endif
      intarg_എണ്ണം++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      നിർത്തുക;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    അവസ്ഥ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      തിരഞ്ഞെടുപ്പ് (rtype)
        {
        അവസ്ഥ FFI_TYPE_FLOAT:
        അവസ്ഥ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          പോകുക homogeneous;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        അവസ്ഥ FFI_TYPE_LONGDOUBLE:
          എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              നിർത്തുക;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          പോകുക homogeneous;
#endif
        അവസ്ഥ FFI_TYPE_INT:
        അവസ്ഥ FFI_TYPE_SINT8:  അവസ്ഥ FFI_TYPE_UINT8:
        അവസ്ഥ FFI_TYPE_SINT16: അവസ്ഥ FFI_TYPE_UINT16:
        അവസ്ഥ FFI_TYPE_SINT32: അവസ്ഥ FFI_TYPE_UINT32:
        അവസ്ഥ FFI_TYPE_SINT64: അവസ്ഥ FFI_TYPE_UINT64:
        അവസ്ഥ FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          നിർത്തുക;
        സ്ഥിരസ്ഥിതി:
          തിരികെ FFI_BAD_TYPEDEF;
        }
      നിർത്തുക;
#endif

    സ്ഥിരസ്ഥിതി:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      നിർത്തുക;
    }

  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned പൂർണ്ണസംഖ്യ align;

      തിരഞ്ഞെടുപ്പ് ((*ptr)->type)
	{
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
          എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_എണ്ണം++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_എണ്ണം = (intarg_എണ്ണം + 3) & ~0x1;
              എങ്കിൽ (vecarg_എണ്ണം > അക്കം_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              നിർത്തുക;
            }
	  എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_എണ്ണം++;
	      intarg_എണ്ണം++;
	    }
	  /* Fall through.  */
#endif
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_FLOAT:
	  fparg_എണ്ണം++;
	  intarg_എണ്ണം++;
	  എങ്കിൽ (fparg_എണ്ണം > അക്കം_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  എങ്കിൽ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      എങ്കിൽ (align > 16)
		align = 16;
	      align = align / 8;
	      എങ്കിൽ (align > 1)
		intarg_എണ്ണം = FFI_ALIGN (intarg_എണ്ണം, align);
	    }
	  intarg_എണ്ണം += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_എണ്ണം += elnum;
              എങ്കിൽ (vecarg_എണ്ണം > അക്കം_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              നിർത്തുക;
            }
	  അല്ലെങ്കിൽ
#endif
	  എങ്കിൽ (elt)
	    {
	      fparg_എണ്ണം += elnum;
	      എങ്കിൽ (fparg_എണ്ണം > അക്കം_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  അല്ലെങ്കിൽ
	    {
	      എങ്കിൽ (intarg_എണ്ണം > അക്കം_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_POINTER:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_എണ്ണം++;
	  എങ്കിൽ (intarg_എണ്ണം > അക്കം_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  നിർത്തുക;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  തിരഞ്ഞെടുപ്പ് (elt)
	    {
	    അവസ്ഥ FFI_TYPE_FLOAT:
	    അവസ്ഥ FFI_TYPE_DOUBLE:
	      fparg_എണ്ണം += 2;
	      intarg_എണ്ണം += 2;
	      എങ്കിൽ (fparg_എണ്ണം > അക്കം_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      നിർത്തുക;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	      എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_എണ്ണം += 2;
		  intarg_എണ്ണം = (intarg_എണ്ണം + 1) & ~0x1;
		  intarg_എണ്ണം += 4;
		  എങ്കിൽ (vecarg_എണ്ണം > അക്കം_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  നിർത്തുക;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_എണ്ണം += 4;
	      intarg_എണ്ണം += 4;
	      എങ്കിൽ (fparg_എണ്ണം > അക്കം_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      നിർത്തുക;
#endif
	    അവസ്ഥ FFI_TYPE_INT:
	    അവസ്ഥ FFI_TYPE_SINT8:  അവസ്ഥ FFI_TYPE_UINT8:
	    അവസ്ഥ FFI_TYPE_SINT16: അവസ്ഥ FFI_TYPE_UINT16:
	    അവസ്ഥ FFI_TYPE_SINT32: അവസ്ഥ FFI_TYPE_UINT32:
	    അവസ്ഥ FFI_TYPE_SINT64: അവസ്ഥ FFI_TYPE_UINT64:
	    അവസ്ഥ FFI_TYPE_POINTER:
	      intarg_എണ്ണം += 2;
	      നിർത്തുക;
	    സ്ഥിരസ്ഥിതി:
	      തിരികെ FFI_BAD_TYPEDEF;
	    }
	  എങ്കിൽ (intarg_എണ്ണം > അക്കം_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  നിർത്തുക;
#endif

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT (0);
	}
    }

  എങ്കിൽ (fparg_എണ്ണം != 0)
    flags |= FLAG_FP_ARGUMENTS;
  എങ്കിൽ (intarg_എണ്ണം > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  എങ്കിൽ (vecarg_എണ്ണം != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  എങ്കിൽ (fparg_എണ്ണം != 0)
    bytes += അക്കം_FPR_ARG_REGISTERS64 * വലുപ്പം (ഇരട്ട);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  എങ്കിൽ (vecarg_എണ്ണം != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += അക്കം_VEC_ARG_REGISTERS64 * വലുപ്പം (float128);
  }

  /* Stack space.  */
#എങ്കിൽ _CALL_ELF == 2
  എങ്കിൽ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_എണ്ണം * വലുപ്പം (long);
#അല്ലെങ്കിൽ
  എങ്കിൽ (intarg_എണ്ണം > അക്കം_GPR_ARG_REGISTERS64)
    bytes += (intarg_എണ്ണം - അക്കം_GPR_ARG_REGISTERS64) * വലുപ്പം (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  തിരികെ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  എങ്കിൽ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#എങ്കിൽ _CALL_ELF != 2
  അല്ലെങ്കിൽ എങ്കിൽ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  അല്ലെങ്കിൽ
    തിരികെ FFI_BAD_ABI;
  തിരികെ ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned പൂർണ്ണസംഖ്യ nfixedargs,
			  unsigned പൂർണ്ണസംഖ്യ ntotalargs MAYBE_UNUSED)
{
  എങ്കിൽ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#എങ്കിൽ _CALL_ELF != 2
  അല്ലെങ്കിൽ എങ്കിൽ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  അല്ലെങ്കിൽ
    തിരികെ FFI_BAD_ABI;
#എങ്കിൽ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  തിരികെ ffi_prep_cif_linux64_core (cif);
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

ശൂന്യം FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *സ്ഥിരം അട്ടി)
{
  സ്ഥിരം unsigned long bytes = ecif->cif->bytes;
  സ്ഥിരം unsigned long flags = ecif->cif->flags;

  തരനിർവചനം സംഘം
  {
    അക്ഷരം *c;
    unsigned long *ul;
    ദശാംശം *f;
    ഇരട്ട *d;
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
  unsigned പൂർണ്ണസംഖ്യ fparg_എണ്ണം;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned പൂർണ്ണസംഖ്യ vecarg_എണ്ണം;

  unsigned പൂർണ്ണസംഖ്യ i, വാക്കുകൾ, nargs, nfixedargs;
  ffi_type **ptr;
  ഇരട്ട double_tmp;
  സംഘം
  {
    ശൂന്യം **v;
    അക്ഷരം **c;
    signed അക്ഷരം **sc;
    unsigned അക്ഷരം **uc;
    signed short **ss;
    unsigned short **us;
    signed പൂർണ്ണസംഖ്യ **si;
    unsigned പൂർണ്ണസംഖ്യ **ui;
    unsigned long **ul;
    ദശാംശം **f;
    ഇരട്ട **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (അക്ഷരം *) അട്ടി + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - അക്കം_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + അക്കം_GPR_ARG_REGISTERS64;
#എങ്കിൽ _CALL_ELF == 2
  rest.ul = അട്ടി + 4 + അക്കം_GPR_ARG_REGISTERS64;
#അല്ലെങ്കിൽ
  rest.ul = അട്ടി + 6 + അക്കം_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - അക്കം_FPR_ARG_REGISTERS64;
  fparg_എണ്ണം = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  എങ്കിൽ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  അല്ലെങ്കിൽ
    vec_base.p = gpr_base.p;
  vec_base.f128 -= അക്കം_VEC_ARG_REGISTERS64;
  vecarg_എണ്ണം = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (അക്ഷരം *) അട്ടി & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  എങ്കിൽ (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (അക്ഷരം *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#എങ്കിൽ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  എങ്കിൽ ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  വേണ്ടി (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned പൂർണ്ണസംഖ്യ elt, elnum;

      തിരഞ്ഞെടുപ്പ് ((*ptr)->type)
	{
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
          എങ്കിൽ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              എങ്കിൽ (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              എങ്കിൽ (vecarg_എണ്ണം < അക്കം_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, വലുപ്പം (float128));
              അല്ലെങ്കിൽ
		memcpy (next_arg.f128, *p_argv.f128, വലുപ്പം (float128));
              എങ്കിൽ (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_എണ്ണം++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              നിർത്തുക;
            }
	  എങ്കിൽ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# എങ്കിൽ _CALL_ELF != 2
		  എങ്കിൽ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      അല്ലെങ്കിൽ
		*next_arg.d = double_tmp;
	      എങ്കിൽ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_എണ്ണം++;
	      double_tmp = (*p_argv.d)[1];
	      എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# എങ്കിൽ _CALL_ELF != 2
		  എങ്കിൽ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      അല്ലെങ്കിൽ
		*next_arg.d = double_tmp;
	      എങ്കിൽ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_എണ്ണം++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      നിർത്തുക;
	    }
	  /* Fall through.  */
#endif
	അവസ്ഥ FFI_TYPE_DOUBLE:
#എങ്കിൽ _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#എങ്കിൽ _CALL_ELF != 2
	      എങ്കിൽ ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  അല്ലെങ്കിൽ
	    *next_arg.d = double_tmp;
	  എങ്കിൽ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_എണ്ണം++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
#എങ്കിൽ _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#എങ്കിൽ _CALL_ELF != 2
	      എങ്കിൽ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (ദശാംശം) double_tmp;
# അല്ലെങ്കിൽ
		  next_arg.f[0] = (ദശാംശം) double_tmp;
# endif
		}
#endif
	    }
	  അല്ലെങ്കിൽ
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (ദശാംശം) double_tmp;
# അല്ലെങ്കിൽ
	      next_arg.f[0] = (ദശാംശം) double_tmp;
# endif
	    }
	  എങ്കിൽ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_എണ്ണം++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned പൂർണ്ണസംഖ്യ j;
	      വേണ്ടി (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  എങ്കിൽ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  എങ്കിൽ (vecarg_എണ്ണം < അക്കം_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, വലുപ്പം (float128));
		  അല്ലെങ്കിൽ
		    memcpy (next_arg.f128, cval + j, വലുപ്പം (float128));
		  എങ്കിൽ (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_എണ്ണം++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      നിർത്തുക;
	    }
	  എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ഇരട്ട *cval = (ഇരട്ട *) *p_argv.v;
	      unsigned പൂർണ്ണസംഖ്യ j;
	      വേണ്ടി (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  അല്ലെങ്കിൽ
		    *next_arg.d = double_tmp;
		  എങ്കിൽ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_എണ്ണം++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      നിർത്തുക;
	    }
#endif
	  എങ്കിൽ (elt == FFI_TYPE_FLOAT)
	    {
	      ദശാംശം *cval = (ദശാംശം *) *p_argv.v;
	      unsigned പൂർണ്ണസംഖ്യ j;
	      വേണ്ടി (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  അല്ലെങ്കിൽ
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (ദശാംശം) double_tmp;
# അല്ലെങ്കിൽ
		      next_arg.f[0] = (ദശാംശം) double_tmp;
# endif
		    }
		  എങ്കിൽ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_എണ്ണം++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (elt == FFI_TYPE_DOUBLE)
	    {
	      ഇരട്ട *cval = (ഇരട്ട *) *p_argv.v;
	      unsigned പൂർണ്ണസംഖ്യ j;
	      വേണ്ടി (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  അല്ലെങ്കിൽ
		    *next_arg.d = double_tmp;
		  എങ്കിൽ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_എണ്ണം++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  അല്ലെങ്കിൽ
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      അക്ഷരം *cval = (അക്ഷരം *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned പൂർണ്ണസംഖ്യ j;
	      വേണ്ടി (j = 0; j < 2; j++)
		{
		  അക്ഷരം *half = cval + j * hsize;
		  unsigned long gprvalue;
		  തിരഞ്ഞെടുപ്പ് (elt)
		    {
		    അവസ്ഥ FFI_TYPE_UINT8:
		      gprvalue = *(unsigned അക്ഷരം *) half; നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed അക്ഷരം *) half;
		      നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_UINT32:
		      gprvalue = *(unsigned പൂർണ്ണസംഖ്യ *) half; നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_INT:
		    അവസ്ഥ FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed പൂർണ്ണസംഖ്യ *) half;
		      നിർത്തുക;
		    അവസ്ഥ FFI_TYPE_SINT64:
		    അവസ്ഥ FFI_TYPE_UINT64:
		    അവസ്ഥ FFI_TYPE_POINTER:
		    സ്ഥിരസ്ഥിതി:
		      gprvalue = *(unsigned long *) half; നിർത്തുക;
		    }
		  *next_arg.ul++ = gprvalue;
		  എങ്കിൽ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_STRUCT:
	  എങ്കിൽ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      എങ്കിൽ (align > 16)
		align = 16;
	      എങ്കിൽ (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  എങ്കിൽ (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  എങ്കിൽ (elt)
	    {
#എങ്കിൽ _CALL_ELF == 2
	      സംഘം {
		ശൂന്യം *v;
		ദശാംശം *f;
		ഇരട്ട *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ചെയ്യുക
                    {
                      എങ്കിൽ (vecarg_എണ്ണം < അക്കം_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, വലുപ്പം (float128));
                      അല്ലെങ്കിൽ
		        memcpy (next_arg.f128, arg.f128++, വലുപ്പം (float128));
                      എങ്കിൽ (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_എണ്ണം++;
                    }
                  ആകുമ്പോൾ (--elnum != 0);
                }
              അല്ലെങ്കിൽ
#endif
	      എങ്കിൽ (elt == FFI_TYPE_FLOAT)
		{
		  ചെയ്യുക
		    {
		      double_tmp = *arg.f++;
		      എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      അല്ലെങ്കിൽ
			*next_arg.f = (ദശാംശം) double_tmp;
		      എങ്കിൽ (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_എണ്ണം++;
		    }
		  ആകുമ്പോൾ (--elnum != 0);
		  എങ്കിൽ ((next_arg.p & 7) != 0)
                    എങ്കിൽ (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      അല്ലെങ്കിൽ
		ചെയ്യുക
		  {
		    double_tmp = *arg.d++;
		    എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    അല്ലെങ്കിൽ
		      *next_arg.d = double_tmp;
		    എങ്കിൽ (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_എണ്ണം++;
		  }
		ആകുമ്പോൾ (--elnum != 0);
#അല്ലെങ്കിൽ
	      എങ്കിൽ (elt == FFI_TYPE_FLOAT)
		പോകുക do_float;
	      അല്ലെങ്കിൽ
		പോകുക do_double;
#endif
	    }
	  അല്ലെങ്കിൽ
	    {
	      വാക്കുകൾ = ((*ptr)->size + 7) / 8;
	      എങ്കിൽ (next_arg.ul >= gpr_base.ul && next_arg.ul + വാക്കുകൾ > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + വാക്കുകൾ * 8 - first;
		}
	      അല്ലെങ്കിൽ
		{
		  അക്ഷരം *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  എങ്കിൽ ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += വാക്കുകൾ;
		  എങ്കിൽ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  പോകുക putgpr;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  എങ്കിൽ (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  നിർത്തുക;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#എങ്കിൽ _CALL_ELF == 2
#define MIN_CACHE_വരി_SIZE 8

സ്ഥിതം ശൂന്യം
flush_icache (അക്ഷരം *wraddr, അക്ഷരം *xaddr, പൂർണ്ണസംഖ്യ size)
{
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < size; i += MIN_CACHE_വരി_SIZE)
    __asm__ അസ്ഥിര ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ അസ്ഥിര ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
			      ശൂന്യം *user_data,
			      ശൂന്യം *codeloc)
{
  എങ്കിൽ (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    തിരികെ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ശൂന്യം (*dest)(ശൂന്യം) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  അല്ലെങ്കിൽ
#endif
    {
#എങ്കിൽ _CALL_ELF == 2
      unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(ശൂന്യം **) &tramp[4] = (ശൂന്യം *) ffi_closure_LINUX64;
      *(ശൂന്യം **) &tramp[6] = codeloc;
      flush_icache ((അക്ഷരം *) tramp, (അക്ഷരം *) codeloc, 4 * 4);
#അല്ലെങ്കിൽ
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      ശൂന്യം **tramp = (ശൂന്യം **) &closure->tramp[0];
      memcpy (&tramp[0], (ശൂന്യം **) ffi_closure_LINUX64, വലുപ്പം (ശൂന്യം *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (ശൂന്യം **) ffi_closure_LINUX64 + 1, വലുപ്പം (ശൂന്യം *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}


പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
			    ശൂന്യം *user_data,
			    ശൂന്യം *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  ശൂന്യം **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + അക്കം_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + അക്കം_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (ശൂന്യം *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#എങ്കിൽ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  എങ്കിൽ ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ആകുമ്പോൾ (i < avn)
    {
      unsigned പൂർണ്ണസംഖ്യ elt, elnum;

      തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (അക്ഷരം *) pst + 7;
	  pst++;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (അക്ഷരം *) pst + 6;
	  pst++;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (അക്ഷരം *) pst + 4;
	  pst++;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  എങ്കിൽ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      എങ്കിൽ (align > 16)
		align = 16;
	      എങ്കിൽ (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  എങ്കിൽ (elt)
	    {
#എങ്കിൽ _CALL_ELF == 2
	      സംഘം {
		ശൂന്യം *v;
		unsigned long *ul;
		ദശാംശം *f;
		ഇരട്ട *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  എങ്കിൽ (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  അല്ലെങ്കിൽ
                    to.v = pst;
                }
              അല്ലെങ്കിൽ
#endif
	      എങ്കിൽ (pfr + elnum <= end_pfr)
		to.v = pfr;
	      അല്ലെങ്കിൽ
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ചെയ്യുക
                    {
                      എങ്കിൽ (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, വലുപ്പം (float128));
                      അല്ലെങ്കിൽ
		        memcpy (to.f128, from.f128, വലുപ്പം (float128));
                      to.f128++;
                      from.f128++;
                    }
                  ആകുമ്പോൾ (--elnum != 0);
                }
              അല്ലെങ്കിൽ
#endif
	      എങ്കിൽ (elt == FFI_TYPE_FLOAT)
		{
		  ചെയ്യുക
		    {
		      എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (ദശാംശം) pfr->d;
			  pfr++;
			}
		      അല്ലെങ്കിൽ
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  ആകുമ്പോൾ (--elnum != 0);
		}
	      അല്ലെങ്കിൽ
		{
		  ചെയ്യുക
		    {
		      എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      അല്ലെങ്കിൽ
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  ആകുമ്പോൾ (--elnum != 0);
		}
#അല്ലെങ്കിൽ
	      എങ്കിൽ (elt == FFI_TYPE_FLOAT)
		പോകുക do_float;
	      അല്ലെങ്കിൽ
		പോകുക do_double;
#endif
	    }
	  അല്ലെങ്കിൽ
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      എങ്കിൽ (arg_types[i]->size < 8)
		avalue[i] = (അക്ഷരം *) pst + 8 - arg_types[i]->size;
	      അല്ലെങ്കിൽ
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  നിർത്തുക;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
          എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              എങ്കിൽ (((unsigned long) pst & 0xF) != 0)
                ++pst;
              എങ്കിൽ (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              അല്ലെങ്കിൽ
                avalue[i] = pst;
              pst += 2;
              നിർത്തുക;
            }
          അല്ലെങ്കിൽ എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      എങ്കിൽ (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      അല്ലെങ്കിൽ
		{
		  എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      നിർത്തുക;
	    }
	  /* Fall through.  */
#endif
	അവസ്ഥ FFI_TYPE_DOUBLE:
#എങ്കിൽ _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
	    avalue[i] = pst;
	  pst++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
#എങ്കിൽ _CALL_ELF != 2
	do_float:
#endif
	  എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ദശാംശം) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (അക്ഷരം *) pst + 4;
#അല്ലെങ്കിൽ
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  നിർത്തുക;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned പൂർണ്ണസംഖ്യ j;
	    elt = arg_types[i]->elements[0]->type;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * വലുപ്പം (float128));
		എങ്കിൽ (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		വേണ്ടി (j = 0; j < 2; j++)
		  {
		    എങ്കിൽ (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, വലുപ്പം (float128));
		    അല്ലെങ്കിൽ
		      memcpy (&cval[j], pst, വലുപ്പം (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		നിർത്തുക;
	      }
	    എങ്കിൽ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ഇരട്ട *cval = alloca (4 * വലുപ്പം (ഇരട്ട));
		വേണ്ടി (j = 0; j < 4; j++)
		  {
		    എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    അല്ലെങ്കിൽ
		      cval[j] = *(ഇരട്ട *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		നിർത്തുക;
	      }
#endif
	    എങ്കിൽ (elt == FFI_TYPE_FLOAT)
	      {
		ദശാംശം *cval = alloca (2 * വലുപ്പം (ദശാംശം));
		വേണ്ടി (j = 0; j < 2; j++)
		  {
		    എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (ദശാംശം) pfr->d;
			pfr++;
		      }
		    അല്ലെങ്കിൽ
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((ദശാംശം *) pst)[1];
#അല്ലെങ്കിൽ
			cval[j] = ((ദശാംശം *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    അല്ലെങ്കിൽ എങ്കിൽ (elt == FFI_TYPE_DOUBLE)
	      {
		ഇരട്ട *cval = alloca (2 * വലുപ്പം (ഇരട്ട));
		വേണ്ടി (j = 0; j < 2; j++)
		  {
		    എങ്കിൽ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    അല്ലെങ്കിൽ
		      cval[j] = *(ഇരട്ട *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    അല്ലെങ്കിൽ
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		അക്ഷരം *cval = alloca (2 * hsize);
		വേണ്ടി (j = 0; j < 2; j++)
		  {
		    അക്ഷരം *src;
#ifndef __LITTLE_ENDIAN__
		    src = (അക്ഷരം *) pst + (8 - hsize);
#അല്ലെങ്കിൽ
		    src = (അക്ഷരം *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  നിർത്തുക;
#endif

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      തിരികെ PPC_LD_NONE;
    അവസ്ഥ FFI_TYPE_FLOAT:
      തിരികെ PPC_LD_F32;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      തിരികെ PPC_LD_F64;
#എങ്കിൽ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      എങ്കിൽ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	തിരികെ PPC64_LD_VECTOR;
      തിരികെ PPC_LD_F128;
#endif
    അവസ്ഥ FFI_TYPE_UINT8:
      തിരികെ PPC_LD_U8;
    അവസ്ഥ FFI_TYPE_SINT8:
      തിരികെ PPC_LD_S8;
    അവസ്ഥ FFI_TYPE_UINT16:
      തിരികെ PPC_LD_U16;
    അവസ്ഥ FFI_TYPE_SINT16:
      തിരികെ PPC_LD_S16;
    അവസ്ഥ FFI_TYPE_UINT32:
      തിരികെ PPC_LD_U32;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      തിരികെ PPC_LD_S32;
    അവസ്ഥ FFI_TYPE_POINTER:
      തിരികെ PPC_LD_PTR;
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      തിരികെ PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    അവസ്ഥ FFI_TYPE_COMPLEX:
      {
	പൂർണ്ണസംഖ്യ inner = cif->rtype->elements[0]->type;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	എങ്കിൽ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    എങ്കിൽ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      തിരികെ PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	എങ്കിൽ (inner == FFI_TYPE_FLOAT)
	  തിരികെ PPC64_LD_FLOAT_HOMOG;
	എങ്കിൽ (inner == FFI_TYPE_DOUBLE)
	  തിരികെ PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  അക്ഷരം *rv = rvalue;
	  unsigned long re, im;
	  തിരഞ്ഞെടുപ്പ് (inner)
	    {
	    അവസ്ഥ FFI_TYPE_UINT8:
	      re = ((unsigned അക്ഷരം *) rv)[0];
	      im = ((unsigned അക്ഷരം *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed അക്ഷരം *) rv)[0];
	      im = (unsigned long) (long) ((signed അക്ഷരം *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_UINT32:
	      re = ((unsigned പൂർണ്ണസംഖ്യ *) rv)[0];
	      im = ((unsigned പൂർണ്ണസംഖ്യ *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_INT:
	    അവസ്ഥ FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed പൂർണ്ണസംഖ്യ *) rv)[0];
	      im = (unsigned long) (long) ((signed പൂർണ്ണസംഖ്യ *) rv)[1];
	      നിർത്തുക;
	    അവസ്ഥ FFI_TYPE_SINT64:
	    അവസ്ഥ FFI_TYPE_UINT64:
	    അവസ്ഥ FFI_TYPE_POINTER:
	    സ്ഥിരസ്ഥിതി:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      നിർത്തുക;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  തിരികെ PPC_LD_R3R4;
	}
      }
#endif
    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  എങ്കിൽ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      തിരഞ്ഞെടുപ്പ് (cif->rtype->size)
		{
		അവസ്ഥ 0:
		  തിരികെ PPC_LD_NONE;
		അവസ്ഥ 1:
		  തിരികെ PPC_LD_U8;
		അവസ്ഥ 2:
		  തിരികെ PPC_LD_U16;
		അവസ്ഥ 3:
		  തിരികെ PPC64_LD_STRUCT_3;
		അവസ്ഥ 4:
		  തിരികെ PPC_LD_U32;
		അവസ്ഥ 5:
		  തിരികെ PPC64_LD_STRUCT_5;
		അവസ്ഥ 6:
		  തിരികെ PPC64_LD_STRUCT_6;
		അവസ്ഥ 7:
		  തിരികെ PPC64_LD_STRUCT_7;
		അവസ്ഥ 8 ... 16:
		  തിരികെ PPC_LD_R3R4;
		}
	      നിർത്തുക;
	    }
	  എങ്കിൽ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    തിരികെ PPC64_LD_VECTOR_HOMOG;
	  എങ്കിൽ ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    തിരികെ PPC64_LD_DOUBLE_HOMOG;
	  തിരികെ PPC64_LD_FLOAT_HOMOG;
        }
      തിരികെ PPC_LD_NONE;
    }
  abort();
}
#endif

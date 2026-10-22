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

#ಅನ್ತರ್ಭಾವ "ffi.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ifdef POWERPC64
#ಅನ್ತರ್ಭಾವ "ffi_common.h"
#ಅನ್ತರ್ಭಾವ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ಗಣನಾ {
  ಅಂಕೆ_GPR_ARG_REGISTERS64 = 8,
  ಅಂಕೆ_FPR_ARG_REGISTERS64 = 13,
  ಅಂಕೆ_VEC_ARG_REGISTERS64 = 12,
};
ಗಣನಾ { ASM_NEEDS_REGISTERS64 = 4 };


#ಯದಿ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  ಯದಿ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ಅನ್ಯಥಾ
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


ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ
discover_homogeneous_aggregate (ffi_abi abi,
                                ಧ್ರುವ ffi_type *t,
                                unsigned ಪೂರ್ಣಾಙ್ಕ *elnum)
{
  ವಿಕಲ್ಪ (t->type)
    {
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      ಯದಿ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      ಅನ್ಯಥಾ ಯದಿ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    ಪಕ್ಷ FFI_TYPE_FLOAT:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      *elnum = 1;
      ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned ಪೂರ್ಣಾಙ್ಕ inner_elnum = 0;
	unsigned ಪೂರ್ಣಾಙ್ಕ inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	ಯದಿ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    ಪ್ರತ್ಯಾಗಮ inner;
	  }
	ಪ್ರತ್ಯಾಗಮ 0;
      }
#endif

    ಪಕ್ಷ FFI_TYPE_STRUCT:;
      {
	unsigned ಪೂರ್ಣಾಙ್ಕ base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	ಯಾವತ್ (*el)
	  {
	    unsigned ಪೂರ್ಣಾಙ್ಕ el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    ಯದಿ (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      ಪ್ರತ್ಯಾಗಮ 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#ಯದಿ _CALL_ELF == 2
	    ಯದಿ (total_elnum > 8)
	      ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	    ಯದಿ (total_elnum > 1)
	      ಪ್ರತ್ಯಾಗಮ 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	ಪ್ರತ್ಯಾಗಮ base_elt;
      }

    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ 0;
    }
}


/* Perform machine dependent cif processing */
ಸ್ಥಿರ ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_ಎಣಿಕೆ = 0, intarg_ಎಣಿಕೆ = 0, vecarg_ಎಣಿಕೆ = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#ಯದಿ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#ನೋಚೇದ್ಯದಿ !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#ಅನ್ಯಥಾ
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#ಯದಿ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * ಪರಿಮಾಣ (long);

  /* Space for the general registers.  */
  bytes += ಅಂಕೆ_GPR_ARG_REGISTERS64 * ಪರಿಮಾಣ (long);
#ಅನ್ಯಥಾ
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * ಪರಿಮಾಣ (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * ಅಂಕೆ_GPR_ARG_REGISTERS64 * ಪರಿಮಾಣ (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#ಯದಿ _CALL_ELF == 2
homogeneous:
#endif
  ವಿಕಲ್ಪ (rtype)
    {
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ಭಙ್ಗ;
        }
      ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
#ಯದಿ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      ಯದಿ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          ಗಚ್ಛ homogeneous;
        }
      ಯದಿ (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ಭಙ್ಗ;
        }
#endif
      intarg_ಎಣಿಕೆ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ಭಙ್ಗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      ವಿಕಲ್ಪ (rtype)
        {
        ಪಕ್ಷ FFI_TYPE_FLOAT:
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          ಗಚ್ಛ homogeneous;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
          ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ಭಙ್ಗ;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          ಗಚ್ಛ homogeneous;
#endif
        ಪಕ್ಷ FFI_TYPE_INT:
        ಪಕ್ಷ FFI_TYPE_SINT8:  ಪಕ್ಷ FFI_TYPE_UINT8:
        ಪಕ್ಷ FFI_TYPE_SINT16: ಪಕ್ಷ FFI_TYPE_UINT16:
        ಪಕ್ಷ FFI_TYPE_SINT32: ಪಕ್ಷ FFI_TYPE_UINT32:
        ಪಕ್ಷ FFI_TYPE_SINT64: ಪಕ್ಷ FFI_TYPE_UINT64:
        ಪಕ್ಷ FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ಭಙ್ಗ;
        ಸ್ವಭಾವ:
          ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
        }
      ಭಙ್ಗ;
#endif

    ಸ್ವಭಾವ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ಭಙ್ಗ;
    }

  ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned ಪೂರ್ಣಾಙ್ಕ align;

      ವಿಕಲ್ಪ ((*ptr)->type)
	{
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
          ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ಎಣಿಕೆ++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ಎಣಿಕೆ = (intarg_ಎಣಿಕೆ + 3) & ~0x1;
              ಯದಿ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ಭಙ್ಗ;
            }
	  ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ಎಣಿಕೆ++;
	      intarg_ಎಣಿಕೆ++;
	    }
	  /* Fall through.  */
#endif
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  fparg_ಎಣಿಕೆ++;
	  intarg_ಎಣಿಕೆ++;
	  ಯದಿ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  ಯದಿ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ಯದಿ (align > 16)
		align = 16;
	      align = align / 8;
	      ಯದಿ (align > 1)
		intarg_ಎಣಿಕೆ = FFI_ALIGN (intarg_ಎಣಿಕೆ, align);
	    }
	  intarg_ಎಣಿಕೆ += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          ಯದಿ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ಎಣಿಕೆ += elnum;
              ಯದಿ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ಭಙ್ಗ;
            }
	  ಅನ್ಯಥಾ
#endif
	  ಯದಿ (elt)
	    {
	      fparg_ಎಣಿಕೆ += elnum;
	      ಯದಿ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ಅನ್ಯಥಾ
	    {
	      ಯದಿ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_POINTER:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ಎಣಿಕೆ++;
	  ಯದಿ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ಭಙ್ಗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ಯದಿ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  ವಿಕಲ್ಪ (elt)
	    {
	    ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಪಕ್ಷ FFI_TYPE_DOUBLE:
	      fparg_ಎಣಿಕೆ += 2;
	      intarg_ಎಣಿಕೆ += 2;
	      ಯದಿ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ಭಙ್ಗ;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	      ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ಎಣಿಕೆ += 2;
		  intarg_ಎಣಿಕೆ = (intarg_ಎಣಿಕೆ + 1) & ~0x1;
		  intarg_ಎಣಿಕೆ += 4;
		  ಯದಿ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ಭಙ್ಗ;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ಎಣಿಕೆ += 4;
	      intarg_ಎಣಿಕೆ += 4;
	      ಯದಿ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ಭಙ್ಗ;
#endif
	    ಪಕ್ಷ FFI_TYPE_INT:
	    ಪಕ್ಷ FFI_TYPE_SINT8:  ಪಕ್ಷ FFI_TYPE_UINT8:
	    ಪಕ್ಷ FFI_TYPE_SINT16: ಪಕ್ಷ FFI_TYPE_UINT16:
	    ಪಕ್ಷ FFI_TYPE_SINT32: ಪಕ್ಷ FFI_TYPE_UINT32:
	    ಪಕ್ಷ FFI_TYPE_SINT64: ಪಕ್ಷ FFI_TYPE_UINT64:
	    ಪಕ್ಷ FFI_TYPE_POINTER:
	      intarg_ಎಣಿಕೆ += 2;
	      ಭಙ್ಗ;
	    ಸ್ವಭಾವ:
	      ಪ್ರತ್ಯಾಗಮ FFI_BAD_TYPEDEF;
	    }
	  ಯದಿ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ಭಙ್ಗ;
#endif

	ಸ್ವಭಾವ:
	  FFI_ASSERT (0);
	}
    }

  ಯದಿ (fparg_ಎಣಿಕೆ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ಯದಿ (intarg_ಎಣಿಕೆ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ಯದಿ (vecarg_ಎಣಿಕೆ != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  ಯದಿ (fparg_ಎಣಿಕೆ != 0)
    bytes += ಅಂಕೆ_FPR_ARG_REGISTERS64 * ಪರಿಮಾಣ (ದ್ವಿಗುಣ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  ಯದಿ (vecarg_ಎಣಿಕೆ != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += ಅಂಕೆ_VEC_ARG_REGISTERS64 * ಪರಿಮಾಣ (float128);
  }

  /* Stack space.  */
#ಯದಿ _CALL_ELF == 2
  ಯದಿ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_ಎಣಿಕೆ * ಪರಿಮಾಣ (long);
#ಅನ್ಯಥಾ
  ಯದಿ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
    bytes += (intarg_ಎಣಿಕೆ - ಅಂಕೆ_GPR_ARG_REGISTERS64) * ಪರಿಮಾಣ (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  ಯದಿ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#ಯದಿ _CALL_ELF != 2
  ಅನ್ಯಥಾ ಯದಿ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
			  unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs MAYBE_UNUSED)
{
  ಯದಿ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#ಯದಿ _CALL_ELF != 2
  ಅನ್ಯಥಾ ಯದಿ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#ಯದಿ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_linux64_core (cif);
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

ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *ಧ್ರುವ ರಾಶಿ)
{
  ಧ್ರುವ unsigned long bytes = ecif->cif->bytes;
  ಧ್ರುವ unsigned long flags = ecif->cif->flags;

  ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ
  {
    ವರ್ಣ *c;
    unsigned long *ul;
    ಭಿನ್ನ *f;
    ದ್ವಿಗುಣ *d;
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
  unsigned ಪೂರ್ಣಾಙ್ಕ fparg_ಎಣಿಕೆ;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned ಪೂರ್ಣಾಙ್ಕ vecarg_ಎಣಿಕೆ;

  unsigned ಪೂರ್ಣಾಙ್ಕ i, ಪದಗಳು, nargs, nfixedargs;
  ffi_type **ptr;
  ದ್ವಿಗುಣ double_tmp;
  ಸಙ್ಘ
  {
    ಶೂನ್ಯ **v;
    ವರ್ಣ **c;
    signed ವರ್ಣ **sc;
    unsigned ವರ್ಣ **uc;
    signed short **ss;
    unsigned short **us;
    signed ಪೂರ್ಣಾಙ್ಕ **si;
    unsigned ಪೂರ್ಣಾಙ್ಕ **ui;
    unsigned long **ul;
    ಭಿನ್ನ **f;
    ದ್ವಿಗುಣ **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (ವರ್ಣ *) ರಾಶಿ + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - ಅಂಕೆ_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#ಯದಿ _CALL_ELF == 2
  rest.ul = ರಾಶಿ + 4 + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#ಅನ್ಯಥಾ
  rest.ul = ರಾಶಿ + 6 + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - ಅಂಕೆ_FPR_ARG_REGISTERS64;
  fparg_ಎಣಿಕೆ = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  ಯದಿ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  ಅನ್ಯಥಾ
    vec_base.p = gpr_base.p;
  vec_base.f128 -= ಅಂಕೆ_VEC_ARG_REGISTERS64;
  vecarg_ಎಣಿಕೆ = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (ವರ್ಣ *) ರಾಶಿ & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  ಯದಿ (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (ವರ್ಣ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#ಯದಿ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  ಯದಿ ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  ಪ್ರತಿ (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned ಪೂರ್ಣಾಙ್ಕ elt, elnum;

      ವಿಕಲ್ಪ ((*ptr)->type)
	{
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
          ಯದಿ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              ಯದಿ (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              ಯದಿ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, ಪರಿಮಾಣ (float128));
              ಅನ್ಯಥಾ
		memcpy (next_arg.f128, *p_argv.f128, ಪರಿಮಾಣ (float128));
              ಯದಿ (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_ಎಣಿಕೆ++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ಭಙ್ಗ;
            }
	  ಯದಿ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# ಯದಿ _CALL_ELF != 2
		  ಯದಿ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      ಅನ್ಯಥಾ
		*next_arg.d = double_tmp;
	      ಯದಿ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ಎಣಿಕೆ++;
	      double_tmp = (*p_argv.d)[1];
	      ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# ಯದಿ _CALL_ELF != 2
		  ಯದಿ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      ಅನ್ಯಥಾ
		*next_arg.d = double_tmp;
	      ಯದಿ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ಎಣಿಕೆ++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ಭಙ್ಗ;
	    }
	  /* Fall through.  */
#endif
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
#ಯದಿ _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#ಯದಿ _CALL_ELF != 2
	      ಯದಿ ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  ಅನ್ಯಥಾ
	    *next_arg.d = double_tmp;
	  ಯದಿ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
#ಯದಿ _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#ಯದಿ _CALL_ELF != 2
	      ಯದಿ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (ಭಿನ್ನ) double_tmp;
# ಅನ್ಯಥಾ
		  next_arg.f[0] = (ಭಿನ್ನ) double_tmp;
# endif
		}
#endif
	    }
	  ಅನ್ಯಥಾ
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (ಭಿನ್ನ) double_tmp;
# ಅನ್ಯಥಾ
	      next_arg.f[0] = (ಭಿನ್ನ) double_tmp;
# endif
	    }
	  ಯದಿ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ಭಙ್ಗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ಯದಿ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  ಯದಿ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned ಪೂರ್ಣಾಙ್ಕ j;
	      ಪ್ರತಿ (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  ಯದಿ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  ಯದಿ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, ಪರಿಮಾಣ (float128));
		  ಅನ್ಯಥಾ
		    memcpy (next_arg.f128, cval + j, ಪರಿಮಾಣ (float128));
		  ಯದಿ (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ಭಙ್ಗ;
	    }
	  ಯದಿ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ದ್ವಿಗುಣ *cval = (ದ್ವಿಗುಣ *) *p_argv.v;
	      unsigned ಪೂರ್ಣಾಙ್ಕ j;
	      ಪ್ರತಿ (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ಅನ್ಯಥಾ
		    *next_arg.d = double_tmp;
		  ಯದಿ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ಭಙ್ಗ;
	    }
#endif
	  ಯದಿ (elt == FFI_TYPE_FLOAT)
	    {
	      ಭಿನ್ನ *cval = (ಭಿನ್ನ *) *p_argv.v;
	      unsigned ಪೂರ್ಣಾಙ್ಕ j;
	      ಪ್ರತಿ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ಅನ್ಯಥಾ
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (ಭಿನ್ನ) double_tmp;
# ಅನ್ಯಥಾ
		      next_arg.f[0] = (ಭಿನ್ನ) double_tmp;
# endif
		    }
		  ಯದಿ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ಅನ್ಯಥಾ ಯದಿ (elt == FFI_TYPE_DOUBLE)
	    {
	      ದ್ವಿಗುಣ *cval = (ದ್ವಿಗುಣ *) *p_argv.v;
	      unsigned ಪೂರ್ಣಾಙ್ಕ j;
	      ಪ್ರತಿ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ಅನ್ಯಥಾ
		    *next_arg.d = double_tmp;
		  ಯದಿ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ಅನ್ಯಥಾ
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      ವರ್ಣ *cval = (ವರ್ಣ *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned ಪೂರ್ಣಾಙ್ಕ j;
	      ಪ್ರತಿ (j = 0; j < 2; j++)
		{
		  ವರ್ಣ *half = cval + j * hsize;
		  unsigned long gprvalue;
		  ವಿಕಲ್ಪ (elt)
		    {
		    ಪಕ್ಷ FFI_TYPE_UINT8:
		      gprvalue = *(unsigned ವರ್ಣ *) half; ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed ವರ್ಣ *) half;
		      ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_UINT32:
		      gprvalue = *(unsigned ಪೂರ್ಣಾಙ್ಕ *) half; ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_INT:
		    ಪಕ್ಷ FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed ಪೂರ್ಣಾಙ್ಕ *) half;
		      ಭಙ್ಗ;
		    ಪಕ್ಷ FFI_TYPE_SINT64:
		    ಪಕ್ಷ FFI_TYPE_UINT64:
		    ಪಕ್ಷ FFI_TYPE_POINTER:
		    ಸ್ವಭಾವ:
		      gprvalue = *(unsigned long *) half; ಭಙ್ಗ;
		    }
		  *next_arg.ul++ = gprvalue;
		  ಯದಿ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  ಯದಿ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ಯದಿ (align > 16)
		align = 16;
	      ಯದಿ (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  ಯದಿ (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  ಯದಿ (elt)
	    {
#ಯದಿ _CALL_ELF == 2
	      ಸಙ್ಘ {
		ಶೂನ್ಯ *v;
		ಭಿನ್ನ *f;
		ದ್ವಿಗುಣ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಯದಿ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಕುರು
                    {
                      ಯದಿ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, ಪರಿಮಾಣ (float128));
                      ಅನ್ಯಥಾ
		        memcpy (next_arg.f128, arg.f128++, ಪರಿಮಾಣ (float128));
                      ಯದಿ (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_ಎಣಿಕೆ++;
                    }
                  ಯಾವತ್ (--elnum != 0);
                }
              ಅನ್ಯಥಾ
#endif
	      ಯದಿ (elt == FFI_TYPE_FLOAT)
		{
		  ಕುರು
		    {
		      double_tmp = *arg.f++;
		      ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      ಅನ್ಯಥಾ
			*next_arg.f = (ಭಿನ್ನ) double_tmp;
		      ಯದಿ (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_ಎಣಿಕೆ++;
		    }
		  ಯಾವತ್ (--elnum != 0);
		  ಯದಿ ((next_arg.p & 7) != 0)
                    ಯದಿ (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      ಅನ್ಯಥಾ
		ಕುರು
		  {
		    double_tmp = *arg.d++;
		    ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    ಅನ್ಯಥಾ
		      *next_arg.d = double_tmp;
		    ಯದಿ (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_ಎಣಿಕೆ++;
		  }
		ಯಾವತ್ (--elnum != 0);
#ಅನ್ಯಥಾ
	      ಯದಿ (elt == FFI_TYPE_FLOAT)
		ಗಚ್ಛ do_float;
	      ಅನ್ಯಥಾ
		ಗಚ್ಛ do_double;
#endif
	    }
	  ಅನ್ಯಥಾ
	    {
	      ಪದಗಳು = ((*ptr)->size + 7) / 8;
	      ಯದಿ (next_arg.ul >= gpr_base.ul && next_arg.ul + ಪದಗಳು > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + ಪದಗಳು * 8 - first;
		}
	      ಅನ್ಯಥಾ
		{
		  ವರ್ಣ *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  ಯದಿ ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += ಪದಗಳು;
		  ಯದಿ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  ಗಚ್ಛ putgpr;

	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  ಯದಿ (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  ಭಙ್ಗ;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#ಯದಿ _CALL_ELF == 2
#define MIN_CACHE_ಸಾಲು_SIZE 8

ಸ್ಥಿರ ಶೂನ್ಯ
flush_icache (ವರ್ಣ *wraddr, ವರ್ಣ *xaddr, ಪೂರ್ಣಾಙ್ಕ size)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < size; i += MIN_CACHE_ಸಾಲು_SIZE)
    __asm__ ಚಞ್ಚಲ ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ಚಞ್ಚಲ ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			      ಶೂನ್ಯ *user_data,
			      ಶೂನ್ಯ *codeloc)
{
  ಯದಿ (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ಅನ್ಯಥಾ
#endif
    {
#ಯದಿ _CALL_ELF == 2
      unsigned ಪೂರ್ಣಾಙ್ಕ *tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(ಶೂನ್ಯ **) &tramp[4] = (ಶೂನ್ಯ *) ffi_closure_LINUX64;
      *(ಶೂನ್ಯ **) &tramp[6] = codeloc;
      flush_icache ((ವರ್ಣ *) tramp, (ವರ್ಣ *) codeloc, 4 * 4);
#ಅನ್ಯಥಾ
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      ಶೂನ್ಯ **tramp = (ಶೂನ್ಯ **) &closure->tramp[0];
      memcpy (&tramp[0], (ಶೂನ್ಯ **) ffi_closure_LINUX64, ಪರಿಮಾಣ (ಶೂನ್ಯ *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (ಶೂನ್ಯ **) ffi_closure_LINUX64 + 1, ಪರಿಮಾಣ (ಶೂನ್ಯ *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}


ಪೂರ್ಣಾಙ್ಕ FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			    ಶೂನ್ಯ *user_data,
			    ಶೂನ್ಯ *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  ಶೂನ್ಯ **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + ಅಂಕೆ_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + ಅಂಕೆ_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (ಶೂನ್ಯ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#ಯದಿ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  ಯದಿ ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಯಾವತ್ (i < avn)
    {
      unsigned ಪೂರ್ಣಾಙ್ಕ elt, elnum;

      ವಿಕಲ್ಪ (arg_types[i]->type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ವರ್ಣ *) pst + 7;
	  pst++;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ವರ್ಣ *) pst + 6;
	  pst++;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ವರ್ಣ *) pst + 4;
	  pst++;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  ಯದಿ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      ಯದಿ (align > 16)
		align = 16;
	      ಯದಿ (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  ಯದಿ (elt)
	    {
#ಯದಿ _CALL_ELF == 2
	      ಸಙ್ಘ {
		ಶೂನ್ಯ *v;
		unsigned long *ul;
		ಭಿನ್ನ *f;
		ದ್ವಿಗುಣ *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಯದಿ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಯದಿ (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  ಅನ್ಯಥಾ
                    to.v = pst;
                }
              ಅನ್ಯಥಾ
#endif
	      ಯದಿ (pfr + elnum <= end_pfr)
		to.v = pfr;
	      ಅನ್ಯಥಾ
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಯದಿ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಕುರು
                    {
                      ಯದಿ (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, ಪರಿಮಾಣ (float128));
                      ಅನ್ಯಥಾ
		        memcpy (to.f128, from.f128, ಪರಿಮಾಣ (float128));
                      to.f128++;
                      from.f128++;
                    }
                  ಯಾವತ್ (--elnum != 0);
                }
              ಅನ್ಯಥಾ
#endif
	      ಯದಿ (elt == FFI_TYPE_FLOAT)
		{
		  ಕುರು
		    {
		      ಯದಿ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (ಭಿನ್ನ) pfr->d;
			  pfr++;
			}
		      ಅನ್ಯಥಾ
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  ಯಾವತ್ (--elnum != 0);
		}
	      ಅನ್ಯಥಾ
		{
		  ಕುರು
		    {
		      ಯದಿ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      ಅನ್ಯಥಾ
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  ಯಾವತ್ (--elnum != 0);
		}
#ಅನ್ಯಥಾ
	      ಯದಿ (elt == FFI_TYPE_FLOAT)
		ಗಚ್ಛ do_float;
	      ಅನ್ಯಥಾ
		ಗಚ್ಛ do_double;
#endif
	    }
	  ಅನ್ಯಥಾ
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      ಯದಿ (arg_types[i]->size < 8)
		avalue[i] = (ವರ್ಣ *) pst + 8 - arg_types[i]->size;
	      ಅನ್ಯಥಾ
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  ಭಙ್ಗ;

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
          ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ಯದಿ (((unsigned long) pst & 0xF) != 0)
                ++pst;
              ಯದಿ (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              ಅನ್ಯಥಾ
                avalue[i] = pst;
              pst += 2;
              ಭಙ್ಗ;
            }
          ಅನ್ಯಥಾ ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      ಯದಿ (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      ಅನ್ಯಥಾ
		{
		  ಯದಿ (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ಭಙ್ಗ;
	    }
	  /* Fall through.  */
#endif
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
#ಯದಿ _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  ಯದಿ (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
	    avalue[i] = pst;
	  pst++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
#ಯದಿ _CALL_ELF != 2
	do_float:
#endif
	  ಯದಿ (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ಭಿನ್ನ) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (ವರ್ಣ *) pst + 4;
#ಅನ್ಯಥಾ
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  ಭಙ್ಗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned ಪೂರ್ಣಾಙ್ಕ j;
	    elt = arg_types[i]->elements[0]->type;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    ಯದಿ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    ಯದಿ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * ಪರಿಮಾಣ (float128));
		ಯದಿ (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		ಪ್ರತಿ (j = 0; j < 2; j++)
		  {
		    ಯದಿ (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, ಪರಿಮಾಣ (float128));
		    ಅನ್ಯಥಾ
		      memcpy (&cval[j], pst, ಪರಿಮಾಣ (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ಭಙ್ಗ;
	      }
	    ಯದಿ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ದ್ವಿಗುಣ *cval = alloca (4 * ಪರಿಮಾಣ (ದ್ವಿಗುಣ));
		ಪ್ರತಿ (j = 0; j < 4; j++)
		  {
		    ಯದಿ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ಅನ್ಯಥಾ
		      cval[j] = *(ದ್ವಿಗುಣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ಭಙ್ಗ;
	      }
#endif
	    ಯದಿ (elt == FFI_TYPE_FLOAT)
	      {
		ಭಿನ್ನ *cval = alloca (2 * ಪರಿಮಾಣ (ಭಿನ್ನ));
		ಪ್ರತಿ (j = 0; j < 2; j++)
		  {
		    ಯದಿ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (ಭಿನ್ನ) pfr->d;
			pfr++;
		      }
		    ಅನ್ಯಥಾ
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((ಭಿನ್ನ *) pst)[1];
#ಅನ್ಯಥಾ
			cval[j] = ((ಭಿನ್ನ *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ಅನ್ಯಥಾ ಯದಿ (elt == FFI_TYPE_DOUBLE)
	      {
		ದ್ವಿಗುಣ *cval = alloca (2 * ಪರಿಮಾಣ (ದ್ವಿಗುಣ));
		ಪ್ರತಿ (j = 0; j < 2; j++)
		  {
		    ಯದಿ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ಅನ್ಯಥಾ
		      cval[j] = *(ದ್ವಿಗುಣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ಅನ್ಯಥಾ
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		ವರ್ಣ *cval = alloca (2 * hsize);
		ಪ್ರತಿ (j = 0; j < 2; j++)
		  {
		    ವರ್ಣ *src;
#ifndef __LITTLE_ENDIAN__
		    src = (ವರ್ಣ *) pst + (8 - hsize);
#ಅನ್ಯಥಾ
		    src = (ವರ್ಣ *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ಭಙ್ಗ;
#endif

	ಸ್ವಭಾವ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_NONE;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F32;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F64;
#ಯದಿ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      ಯದಿ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	ಪ್ರತ್ಯಾಗಮ PPC64_LD_VECTOR;
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F128;
#endif
    ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U8;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S8;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U16;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S16;
    ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U32;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S32;
    ಪಕ್ಷ FFI_TYPE_POINTER:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_PTR;
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      {
	ಪೂರ್ಣಾಙ್ಕ inner = cif->rtype->elements[0]->type;
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಯದಿ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    ಯದಿ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      ಪ್ರತ್ಯಾಗಮ PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	ಯದಿ (inner == FFI_TYPE_FLOAT)
	  ಪ್ರತ್ಯಾಗಮ PPC64_LD_FLOAT_HOMOG;
	ಯದಿ (inner == FFI_TYPE_DOUBLE)
	  ಪ್ರತ್ಯಾಗಮ PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  ವರ್ಣ *rv = rvalue;
	  unsigned long re, im;
	  ವಿಕಲ್ಪ (inner)
	    {
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      re = ((unsigned ವರ್ಣ *) rv)[0];
	      im = ((unsigned ವರ್ಣ *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed ವರ್ಣ *) rv)[0];
	      im = (unsigned long) (long) ((signed ವರ್ಣ *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_UINT32:
	      re = ((unsigned ಪೂರ್ಣಾಙ್ಕ *) rv)[0];
	      im = ((unsigned ಪೂರ್ಣಾಙ್ಕ *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_INT:
	    ಪಕ್ಷ FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed ಪೂರ್ಣಾಙ್ಕ *) rv)[0];
	      im = (unsigned long) (long) ((signed ಪೂರ್ಣಾಙ್ಕ *) rv)[1];
	      ಭಙ್ಗ;
	    ಪಕ್ಷ FFI_TYPE_SINT64:
	    ಪಕ್ಷ FFI_TYPE_UINT64:
	    ಪಕ್ಷ FFI_TYPE_POINTER:
	    ಸ್ವಭಾವ:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      ಭಙ್ಗ;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  ಪ್ರತ್ಯಾಗಮ PPC_LD_R3R4;
	}
      }
#endif
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  ಯದಿ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      ವಿಕಲ್ಪ (cif->rtype->size)
		{
		ಪಕ್ಷ 0:
		  ಪ್ರತ್ಯಾಗಮ PPC_LD_NONE;
		ಪಕ್ಷ 1:
		  ಪ್ರತ್ಯಾಗಮ PPC_LD_U8;
		ಪಕ್ಷ 2:
		  ಪ್ರತ್ಯಾಗಮ PPC_LD_U16;
		ಪಕ್ಷ 3:
		  ಪ್ರತ್ಯಾಗಮ PPC64_LD_STRUCT_3;
		ಪಕ್ಷ 4:
		  ಪ್ರತ್ಯಾಗಮ PPC_LD_U32;
		ಪಕ್ಷ 5:
		  ಪ್ರತ್ಯಾಗಮ PPC64_LD_STRUCT_5;
		ಪಕ್ಷ 6:
		  ಪ್ರತ್ಯಾಗಮ PPC64_LD_STRUCT_6;
		ಪಕ್ಷ 7:
		  ಪ್ರತ್ಯಾಗಮ PPC64_LD_STRUCT_7;
		ಪಕ್ಷ 8 ... 16:
		  ಪ್ರತ್ಯಾಗಮ PPC_LD_R3R4;
		}
	      ಭಙ್ಗ;
	    }
	  ಯದಿ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    ಪ್ರತ್ಯಾಗಮ PPC64_LD_VECTOR_HOMOG;
	  ಯದಿ ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    ಪ್ರತ್ಯಾಗಮ PPC64_LD_DOUBLE_HOMOG;
	  ಪ್ರತ್ಯಾಗಮ PPC64_LD_FLOAT_HOMOG;
        }
      ಪ್ರತ್ಯಾಗಮ PPC_LD_NONE;
    }
  abort();
}
#endif

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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
#ಅನ್ತರ್ಭಾವ "ffi_common.h"
#ಅನ್ತರ್ಭಾವ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ಎಣಿಕೆ {
  ಅಂಕೆ_GPR_ARG_REGISTERS64 = 8,
  ಅಂಕೆ_FPR_ARG_REGISTERS64 = 13,
  ಅಂಕೆ_VEC_ARG_REGISTERS64 = 12,
};
ಎಣಿಕೆ { ASM_NEEDS_REGISTERS64 = 4 };


#ಆದರೆ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  ಆದರೆ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.ಗಾತ್ರ_2 = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ffi_type_longdouble.ಗಾತ್ರ_2 = 16;
      ffi_type_longdouble.alignment = 16;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.ಗಾತ್ರ_2 = 2 * ffi_type_longdouble.ಗಾತ್ರ_2;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ


ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ
discover_homogeneous_aggregate (ffi_abi abi,
                                ಸ್ಥಿರಾಂಕ ffi_type *t,
                                ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *elnum)
{
  ಆಯ್ಕೆ (t->type)
    {
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      ಆದರೆ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          ಹಿಂತಿರುಗಿ FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          ಹಿಂತಿರುಗಿ FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      *elnum = 1;
      ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ) t->type;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ inner_elnum = 0;
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	ಆದರೆ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    ಹಿಂತಿರುಗಿ inner;
	  }
	ಹಿಂತಿರುಗಿ 0;
      }
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:;
      {
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ತಳ_elt = 0, ಒಟ್ಟು_elnum = 0;
	ffi_type **el = t->elements;
	ಆಗ (*el)
	  {
	    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    ಆದರೆ (el_elt == 0
		|| (ತಳ_elt && ತಳ_elt != el_elt))
	      ಹಿಂತಿರುಗಿ 0;
	    ತಳ_elt = el_elt;
	    ಒಟ್ಟು_elnum += el_elnum;
#ಆದರೆ _CALL_ELF == 2
	    ಆದರೆ (ಒಟ್ಟು_elnum > 8)
	      ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	    ಆದರೆ (ಒಟ್ಟು_elnum > 1)
	      ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
	    el++;
	  }
	*elnum = ಒಟ್ಟು_elnum;
	ಹಿಂತಿರುಗಿ ತಳ_elt;
      }

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ 0;
    }
}


/* Perform machine dependent cif processing */
ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು;
  ಚಿಹ್ನೆರಹಿತ i, fparg_ಎಣಿಕೆ = 0, intarg_ಎಣಿಕೆ = 0, vecarg_ಎಣಿಕೆ = 0;
  ಚಿಹ್ನೆರಹಿತ flags = cif->flags;
  ಚಿಹ್ನೆರಹಿತ elt, elnum, rtype;

#ಆದರೆ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಅಥವಾಆದರೆ !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಇಲ್ಲದಿದ್ದರೆ
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#ಆದರೆ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  ಬೈಟುಗಳು = (4 + ASM_NEEDS_REGISTERS64) * ಗಾತ್ರ (ದೀರ್ಘ);

  /* Space for the general registers.  */
  ಬೈಟುಗಳು += ಅಂಕೆ_GPR_ARG_REGISTERS64 * ಗಾತ್ರ (ದೀರ್ಘ);
#ಇಲ್ಲದಿದ್ದರೆ
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  ಬೈಟುಗಳು = (6 + ASM_NEEDS_REGISTERS64) * ಗಾತ್ರ (ದೀರ್ಘ);

  /* Space for the mandatory parm save area and general registers.  */
  ಬೈಟುಗಳು += 2 * ಅಂಕೆ_GPR_ARG_REGISTERS64 * ಗಾತ್ರ (ದೀರ್ಘ);
#ಆದರೆ_ಅಂತ್ಯ

  /* Return value handling.  */
  rtype = cif->rtype->type;
#ಆದರೆ _CALL_ELF == 2
homogeneous:
#ಆದರೆ_ಅಂತ್ಯ
  ಆಯ್ಕೆ (rtype)
    {
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ನಿಲ್ಲಿಸು;
        }
      ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ಆದರೆ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      ಆದರೆ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          ಹೋಗು homogeneous;
        }
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ನಿಲ್ಲಿಸು;
        }
#ಆದರೆ_ಅಂತ್ಯ
      intarg_ಎಣಿಕೆ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      ಆಯ್ಕೆ (rtype)
        {
        ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          ಹೋಗು homogeneous;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
          ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ನಿಲ್ಲಿಸು;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          ಹೋಗು homogeneous;
#ಆದರೆ_ಅಂತ್ಯ
        ಸ್ಥಿತಿ FFI_TYPE_INT:
        ಸ್ಥಿತಿ FFI_TYPE_SINT8:  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
        ಸ್ಥಿತಿ FFI_TYPE_SINT16: ಸ್ಥಿತಿ FFI_TYPE_UINT16:
        ಸ್ಥಿತಿ FFI_TYPE_SINT32: ಸ್ಥಿತಿ FFI_TYPE_UINT32:
        ಸ್ಥಿತಿ FFI_TYPE_SINT64: ಸ್ಥಿತಿ FFI_TYPE_UINT64:
        ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ನಿಲ್ಲಿಸು;
        ಪೂರ್ವನಿಯೋಜಿತ:
          ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
        }
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಪೂರ್ವನಿಯೋಜಿತ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ನಿಲ್ಲಿಸು;
    }

  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ align;

      ಆಯ್ಕೆ ((*ptr)->type)
	{
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
          ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ಎಣಿಕೆ++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ಎಣಿಕೆ = (intarg_ಎಣಿಕೆ + 3) & ~0x1;
              ಆದರೆ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ನಿಲ್ಲಿಸು;
            }
	  ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ಎಣಿಕೆ++;
	      intarg_ಎಣಿಕೆ++;
	    }
	  /* Fall through.  */
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  fparg_ಎಣಿಕೆ++;
	  intarg_ಎಣಿಕೆ++;
	  ಆದರೆ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಆದರೆ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ಆದರೆ (align > 16)
		align = 16;
	      align = align / 8;
	      ಆದರೆ (align > 1)
		intarg_ಎಣಿಕೆ = FFI_ALIGN (intarg_ಎಣಿಕೆ, align);
	    }
	  intarg_ಎಣಿಕೆ += ((*ptr)->ಗಾತ್ರ_2 + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ಎಣಿಕೆ += elnum;
              ಆದರೆ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ನಿಲ್ಲಿಸು;
            }
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	  ಆದರೆ (elt)
	    {
	      fparg_ಎಣಿಕೆ += elnum;
	      ಆದರೆ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಆದರೆ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ಎಣಿಕೆ++;
	  ಆದರೆ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#ಆದರೆ_ಅಂತ್ಯ
	  ಆಯ್ಕೆ (elt)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	      fparg_ಎಣಿಕೆ += 2;
	      intarg_ಎಣಿಕೆ += 2;
	      ಆದರೆ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ನಿಲ್ಲಿಸು;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	      ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ಎಣಿಕೆ += 2;
		  intarg_ಎಣಿಕೆ = (intarg_ಎಣಿಕೆ + 1) & ~0x1;
		  intarg_ಎಣಿಕೆ += 4;
		  ಆದರೆ (vecarg_ಎಣಿಕೆ > ಅಂಕೆ_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ನಿಲ್ಲಿಸು;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ಎಣಿಕೆ += 4;
	      intarg_ಎಣಿಕೆ += 4;
	      ಆದರೆ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	    ಸ್ಥಿತಿ FFI_TYPE_INT:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:  ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16: ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT32: ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT64: ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	      intarg_ಎಣಿಕೆ += 2;
	      ನಿಲ್ಲಿಸು;
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
	    }
	  ಆದರೆ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT (0);
	}
    }

  ಆದರೆ (fparg_ಎಣಿಕೆ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ಆದರೆ (intarg_ಎಣಿಕೆ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ಆದರೆ (vecarg_ಎಣಿಕೆ != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  ಆದರೆ (fparg_ಎಣಿಕೆ != 0)
    ಬೈಟುಗಳು += ಅಂಕೆ_FPR_ARG_REGISTERS64 * ಗಾತ್ರ (ದ್ವಿಗುಣ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  ಆದರೆ (vecarg_ಎಣಿಕೆ != 0) {
    ಬೈಟುಗಳು = (ಬೈಟುಗಳು + 15) & ~0xF;
    ಬೈಟುಗಳು += ಅಂಕೆ_VEC_ARG_REGISTERS64 * ಗಾತ್ರ (float128);
  }

  /* Stack space.  */
#ಆದರೆ _CALL_ELF == 2
  ಆದರೆ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    ಬೈಟುಗಳು += intarg_ಎಣಿಕೆ * ಗಾತ್ರ (ದೀರ್ಘ);
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (intarg_ಎಣಿಕೆ > ಅಂಕೆ_GPR_ARG_REGISTERS64)
    ಬೈಟುಗಳು += (intarg_ಎಣಿಕೆ - ಅಂಕೆ_GPR_ARG_REGISTERS64) * ಗಾತ್ರ (ದೀರ್ಘ);
#ಆದರೆ_ಅಂತ್ಯ

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ಬೈಟುಗಳು = (ಬೈಟುಗಳು + 15) & ~0xF;

  cif->flags = flags;
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  ಆದರೆ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#ಆದರೆ _CALL_ELF != 2
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  ಹಿಂತಿರುಗಿ ffi_prep_cif_linux64_core (cif);
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs MAYBE_ಬಳಸದ)
{
  ಆದರೆ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#ಆದರೆ _CALL_ELF != 2
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ ffi_prep_cif_linux64_core (cif);
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
ffi_prep_args64 (extended_cif *ecif, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಸ್ಥಿರಾಂಕ ರಾಶಿ)
{
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಬೈಟುಗಳು = ecif->cif->ಬೈಟುಗಳು;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ flags = ecif->cif->flags;

  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ
  {
    ಅಕ್ಷರ *c;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ul;
    ದಶಮಾಂಶ *f;
    ದ್ವಿಗುಣ *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_ತಳ;
  valp gpr_ಕೊನೆ;
  valp rest;
  valp ಮುಂದಿನ_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_ತಳ;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ fparg_ಎಣಿಕೆ;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_ತಳ;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ vecarg_ಎಣಿಕೆ;

  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i, ಪದಗಳು, nargs, nfixedargs;
  ffi_type **ptr;
  ದ್ವಿಗುಣ double_tmp;
  ಒಕ್ಕೂಟ
  {
    ಶೂನ್ಯ **v;
    ಅಕ್ಷರ **c;
    ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ **sc;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **uc;
    ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ **ss;
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ **us;
    ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ **si;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ **ui;
    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ **ul;
    ದಶಮಾಂಶ **f;
    ದ್ವಿಗುಣ **d;
    float128 **f128;
  } p_argv;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ gprvalue;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ align;

  stacktop.c = (ಅಕ್ಷರ *) ರಾಶಿ + ಬೈಟುಗಳು;
  gpr_ತಳ.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - ಅಂಕೆ_GPR_ARG_REGISTERS64;
  gpr_ಕೊನೆ.ul = gpr_ತಳ.ul + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#ಆದರೆ _CALL_ELF == 2
  rest.ul = ರಾಶಿ + 4 + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#ಇಲ್ಲದಿದ್ದರೆ
  rest.ul = ರಾಶಿ + 6 + ಅಂಕೆ_GPR_ARG_REGISTERS64;
#ಆದರೆ_ಅಂತ್ಯ
  fpr_ತಳ.d = gpr_ತಳ.d - ಅಂಕೆ_FPR_ARG_REGISTERS64;
  fparg_ಎಣಿಕೆ = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  ಆದರೆ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_ತಳ.p = fpr_ತಳ.p & ~0xF;
  ಇಲ್ಲದಿದ್ದರೆ
    vec_ತಳ.p = gpr_ತಳ.p;
  vec_ತಳ.f128 -= ಅಂಕೆ_VEC_ARG_REGISTERS64;
  vecarg_ಎಣಿಕೆ = 0;
  ಮುಂದಿನ_arg.ul = gpr_ತಳ.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ಅಕ್ಷರ *) ರಾಶಿ & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) gpr_ತಳ.c & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) gpr_ಕೊನೆ.c  & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) vec_ತಳ.c & 0xF) == 0);
  FFI_ASSERT ((ಬೈಟುಗಳು & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  ಆದರೆ (flags & FLAG_RETVAL_REFERENCE)
    *ಮುಂದಿನ_arg.ul++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ಅಕ್ಷರ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#ಆದರೆ _CALL_ELF != 2
  nfixedargs = (ಚಿಹ್ನೆರಹಿತ) -1;
  ಆದರೆ ((flags & FLAG_COMPAT) == 0)
#ಆದರೆ_ಅಂತ್ಯ
    nfixedargs = ecif->cif->nfixedargs;
  ಗಾಗಿ (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ elt, elnum;

      ಆಯ್ಕೆ ((*ptr)->type)
	{
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
          ಆದರೆ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ಮುಂದಿನ_arg.p = FFI_ALIGN (ಮುಂದಿನ_arg.p, 16);
              ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
                ಮುಂದಿನ_arg.ul = rest.ul;
              ಆದರೆ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		ಸ್ಮರಣೆ_ನಕಲು (vec_ತಳ.f128++, *p_argv.f128, ಗಾತ್ರ (float128));
              ಇಲ್ಲದಿದ್ದರೆ
		ಸ್ಮರಣೆ_ನಕಲು (ಮುಂದಿನ_arg.f128, *p_argv.f128, ಗಾತ್ರ (float128));
              ಆದರೆ (++ಮುಂದಿನ_arg.f128 == gpr_ಕೊನೆ.f128)
                ಮುಂದಿನ_arg.f128 = rest.f128;
              vecarg_ಎಣಿಕೆ++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ನಿಲ್ಲಿಸು;
            }
	  ಆದರೆ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ತಳ.d++ = double_tmp;
# ಆದರೆ _CALL_ELF != 2
		  ಆದರೆ ((flags & FLAG_COMPAT) != 0)
		    *ಮುಂದಿನ_arg.d = double_tmp;
# endif
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		*ಮುಂದಿನ_arg.d = double_tmp;
	      ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		ಮುಂದಿನ_arg.ul = rest.ul;
	      fparg_ಎಣಿಕೆ++;
	      double_tmp = (*p_argv.d)[1];
	      ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ತಳ.d++ = double_tmp;
# ಆದರೆ _CALL_ELF != 2
		  ಆದರೆ ((flags & FLAG_COMPAT) != 0)
		    *ಮುಂದಿನ_arg.d = double_tmp;
# endif
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		*ಮುಂದಿನ_arg.d = double_tmp;
	      ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		ಮುಂದಿನ_arg.ul = rest.ul;
	      fparg_ಎಣಿಕೆ++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ನಿಲ್ಲಿಸು;
	    }
	  /* Fall through.  */
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
#ಆದರೆ _CALL_ELF != 2
	do_double:
#ಆದರೆ_ಅಂತ್ಯ
	  double_tmp = **p_argv.d;
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ತಳ.d++ = double_tmp;
#ಆದರೆ _CALL_ELF != 2
	      ಆದರೆ ((flags & FLAG_COMPAT) != 0)
		*ಮುಂದಿನ_arg.d = double_tmp;
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    *ಮುಂದಿನ_arg.d = double_tmp;
	  ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
	    ಮುಂದಿನ_arg.ul = rest.ul;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
#ಆದರೆ _CALL_ELF != 2
	do_float:
#ಆದರೆ_ಅಂತ್ಯ
	  double_tmp = **p_argv.f;
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ತಳ.d++ = double_tmp;
#ಆದರೆ _CALL_ELF != 2
	      ಆದರೆ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  ಮುಂದಿನ_arg.f[1] = (ದಶಮಾಂಶ) double_tmp;
# ಇಲ್ಲದಿದ್ದರೆ
		  ಮುಂದಿನ_arg.f[0] = (ದಶಮಾಂಶ) double_tmp;
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
# ifndef __LITTLE_ENDIAN__
	      ಮುಂದಿನ_arg.f[1] = (ದಶಮಾಂಶ) double_tmp;
# ಇಲ್ಲದಿದ್ದರೆ
	      ಮುಂದಿನ_arg.f[0] = (ದಶಮಾಂಶ) double_tmp;
# endif
	    }
	  ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
	    ಮುಂದಿನ_arg.ul = rest.ul;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	      ಗಾಗಿ (j = 0; j < 2; j++)
		{
		  ಮುಂದಿನ_arg.p = FFI_ALIGN (ಮುಂದಿನ_arg.p, 16);
		  ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		  ಆದರೆ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    ಸ್ಮರಣೆ_ನಕಲು (vec_ತಳ.f128++, cval + j, ಗಾತ್ರ (float128));
		  ಇಲ್ಲದಿದ್ದರೆ
		    ಸ್ಮರಣೆ_ನಕಲು (ಮುಂದಿನ_arg.f128, cval + j, ಗಾತ್ರ (float128));
		  ಆದರೆ (++ಮುಂದಿನ_arg.f128 == gpr_ಕೊನೆ.f128)
		    ಮುಂದಿನ_arg.f128 = rest.f128;
		  vecarg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ನಿಲ್ಲಿಸು;
	    }
	  ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ದ್ವಿಗುಣ *cval = (ದ್ವಿಗುಣ *) *p_argv.v;
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	      ಗಾಗಿ (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ತಳ.d++ = double_tmp;
		  ಇಲ್ಲದಿದ್ದರೆ
		    *ಮುಂದಿನ_arg.d = double_tmp;
		  ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ನಿಲ್ಲಿಸು;
	    }
#ಆದರೆ_ಅಂತ್ಯ
	  ಆದರೆ (elt == FFI_TYPE_FLOAT)
	    {
	      ದಶಮಾಂಶ *cval = (ದಶಮಾಂಶ *) *p_argv.v;
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	      ಗಾಗಿ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ತಳ.d++ = double_tmp;
		  ಇಲ್ಲದಿದ್ದರೆ
		    {
# ifndef __LITTLE_ENDIAN__
		      ಮುಂದಿನ_arg.f[1] = (ದಶಮಾಂಶ) double_tmp;
# ಇಲ್ಲದಿದ್ದರೆ
		      ಮುಂದಿನ_arg.f[0] = (ದಶಮಾಂಶ) double_tmp;
# endif
		    }
		  ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (elt == FFI_TYPE_DOUBLE)
	    {
	      ದ್ವಿಗುಣ *cval = (ದ್ವಿಗುಣ *) *p_argv.v;
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	      ಗಾಗಿ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ತಳ.d++ = double_tmp;
		  ಇಲ್ಲದಿದ್ದರೆ
		    *ಮುಂದಿನ_arg.d = double_tmp;
		  ಆದರೆ (++ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		  fparg_ಎಣಿಕೆ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      ಅಕ್ಷರ *cval = (ಅಕ್ಷರ *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->ಗಾತ್ರ_2;
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	      ಗಾಗಿ (j = 0; j < 2; j++)
		{
		  ಅಕ್ಷರ *half = cval + j * hsize;
		  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ gprvalue;
		  ಆಯ್ಕೆ (elt)
		    {
		    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
		      gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) half; ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		      gprvalue = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) *(ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ *) half;
		      ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
		      gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) half; ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		      gprvalue = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) *(ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ *) half;
		      ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
		      gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) half; ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_INT:
		    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
		      gprvalue = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) half;
		      ನಿಲ್ಲಿಸು;
		    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
		    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
		    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
		    ಪೂರ್ವನಿಯೋಜಿತ:
		      gprvalue = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) half; ನಿಲ್ಲಿಸು;
		    }
		  *ಮುಂದಿನ_arg.ul++ = gprvalue;
		  ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		}
	    }
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಆದರೆ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ಆದರೆ (align > 16)
		align = 16;
	      ಆದರೆ (align > 1)
                {
                  ಮುಂದಿನ_arg.p = FFI_ALIGN (ಮುಂದಿನ_arg.p, align);
                  ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
                    ಮುಂದಿನ_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  ಆದರೆ (elt)
	    {
#ಆದರೆ _CALL_ELF == 2
	      ಒಕ್ಕೂಟ {
		ಶೂನ್ಯ *v;
		ದಶಮಾಂಶ *f;
		ದ್ವಿಗುಣ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಮಾಡು
                    {
                      ಆದರೆ (vecarg_ಎಣಿಕೆ < ಅಂಕೆ_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        ಸ್ಮರಣೆ_ನಕಲು (vec_ತಳ.f128++, arg.f128++, ಗಾತ್ರ (float128));
                      ಇಲ್ಲದಿದ್ದರೆ
		        ಸ್ಮರಣೆ_ನಕಲು (ಮುಂದಿನ_arg.f128, arg.f128++, ಗಾತ್ರ (float128));
                      ಆದರೆ (++ಮುಂದಿನ_arg.f128 == gpr_ಕೊನೆ.f128)
                        ಮುಂದಿನ_arg.f128 = rest.f128;
                      vecarg_ಎಣಿಕೆ++;
                    }
                  ಆಗ (--elnum != 0);
                }
              ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	      ಆದರೆ (elt == FFI_TYPE_FLOAT)
		{
		  ಮಾಡು
		    {
		      double_tmp = *arg.f++;
		      ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_ತಳ.d++ = double_tmp;
		      ಇಲ್ಲದಿದ್ದರೆ
			*ಮುಂದಿನ_arg.f = (ದಶಮಾಂಶ) double_tmp;
		      ಆದರೆ (++ಮುಂದಿನ_arg.f == gpr_ಕೊನೆ.f)
			ಮುಂದಿನ_arg.f = rest.f;
		      fparg_ಎಣಿಕೆ++;
		    }
		  ಆಗ (--elnum != 0);
		  ಆದರೆ ((ಮುಂದಿನ_arg.p & 7) != 0)
                    ಆದರೆ (++ಮುಂದಿನ_arg.f == gpr_ಕೊನೆ.f)
                      ಮುಂದಿನ_arg.f = rest.f;
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		ಮಾಡು
		  {
		    double_tmp = *arg.d++;
		    ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_ತಳ.d++ = double_tmp;
		    ಇಲ್ಲದಿದ್ದರೆ
		      *ಮುಂದಿನ_arg.d = double_tmp;
		    ಆದರೆ (++ಮುಂದಿನ_arg.d == gpr_ಕೊನೆ.d)
		      ಮುಂದಿನ_arg.d = rest.d;
		    fparg_ಎಣಿಕೆ++;
		  }
		ಆಗ (--elnum != 0);
#ಇಲ್ಲದಿದ್ದರೆ
	      ಆದರೆ (elt == FFI_TYPE_FLOAT)
		ಹೋಗು do_float;
	      ಇಲ್ಲದಿದ್ದರೆ
		ಹೋಗು do_double;
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಪದಗಳು = ((*ptr)->ಗಾತ್ರ_2 + 7) / 8;
	      ಆದರೆ (ಮುಂದಿನ_arg.ul >= gpr_ತಳ.ul && ಮುಂದಿನ_arg.ul + ಪದಗಳು > gpr_ಕೊನೆ.ul)
		{
		  size_t ಮೊದಲ = gpr_ಕೊನೆ.c - ಮುಂದಿನ_arg.c;
		  ಸ್ಮರಣೆ_ನಕಲು (ಮುಂದಿನ_arg.c, *p_argv.c, ಮೊದಲ);
		  ಸ್ಮರಣೆ_ನಕಲು (rest.c, *p_argv.c + ಮೊದಲ, (*ptr)->ಗಾತ್ರ_2 - ಮೊದಲ);
		  ಮುಂದಿನ_arg.c = rest.c + ಪದಗಳು * 8 - ಮೊದಲ;
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		{
		  ಅಕ್ಷರ *where = ಮುಂದಿನ_arg.c;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  ಆದರೆ ((*ptr)->ಗಾತ್ರ_2 < 8)
		    where += 8 - (*ptr)->ಗಾತ್ರ_2;
#ಆದರೆ_ಅಂತ್ಯ
		  ಸ್ಮರಣೆ_ನಕಲು (where, *p_argv.c, (*ptr)->ಗಾತ್ರ_2);
		  ಮುಂದಿನ_arg.ul += ಪದಗಳು;
		  ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
		    ಮುಂದಿನ_arg.ul = rest.ul;
		}
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  ಹೋಗು putgpr;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *ಮುಂದಿನ_arg.ul++ = gprvalue;
	  ಆದರೆ (ಮುಂದಿನ_arg.ul == gpr_ಕೊನೆ.ul)
	    ಮುಂದಿನ_arg.ul = rest.ul;
	  ನಿಲ್ಲಿಸು;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (ಮುಂದಿನ_arg.ul >= gpr_ತಳ.ul
		  && ಮುಂದಿನ_arg.ul <= gpr_ತಳ.ul + 4));
}


#ಆದರೆ _CALL_ELF == 2
#ವ್ಯಾಖ್ಯಾನಿಸು MIN_CACHE_ಸಾಲು_ಗಾತ್ರ 8

ಸ್ಥಿರ ಶೂನ್ಯ
flush_icache (ಅಕ್ಷರ *wraddr, ಅಕ್ಷರ *xaddr, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_2; i += MIN_CACHE_ಸಾಲು_ಗಾತ್ರ)
    __asm__ ಅಸ್ಥಿರ ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ಅಸ್ಥಿರ ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + ಗಾತ್ರ_2 - 1), "r"(wraddr + ಗಾತ್ರ_2 - 1)
		    : "memory");
}
#ಆದರೆ_ಅಂತ್ಯ


ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_ಆವರಣ_loc_linux64 (ffi_ಆವರಣ *ಆವರಣ,
			      ffi_cif *cif,
			      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			      ಶೂನ್ಯ *codeloc)
{
  ಆದರೆ (cif->abi < FFI_LINUX || cif->abi >= FFI_ಕೊನೆಯ_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_EXEC_STATIC_TRAMP
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ) = ffi_ಆವರಣ_LINUX64;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, dest, ಆವರಣ);
    }
  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
#ಆದರೆ _CALL_ELF == 2
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(ಶೂನ್ಯ **) &tramp[4] = (ಶೂನ್ಯ *) ffi_ಆವರಣ_LINUX64;
      *(ಶೂನ್ಯ **) &tramp[6] = codeloc;
      flush_icache ((ಅಕ್ಷರ *) tramp, (ಅಕ್ಷರ *) codeloc, 4 * 4);
#ಇಲ್ಲದಿದ್ದರೆ
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      ಶೂನ್ಯ **tramp = (ಶೂನ್ಯ **) &ಆವರಣ->tramp[0];
      ಸ್ಮರಣೆ_ನಕಲು (&tramp[0], (ಶೂನ್ಯ **) ffi_ಆವರಣ_LINUX64, ಗಾತ್ರ (ಶೂನ್ಯ *));
      tramp[1] = codeloc;
      ಸ್ಮರಣೆ_ನಕಲು (&tramp[2], (ಶೂನ್ಯ **) ffi_ಆವರಣ_LINUX64 + 1, ಗಾತ್ರ (ಶೂನ್ಯ *));
#ಆದರೆ_ಅಂತ್ಯ
    }

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}


ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_helper_LINUX64 (ffi_cif *cif,
			    ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			    ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			    ಶೂನ್ಯ *rvalue,
			    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pst,
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
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ i, avn, nfixedargs;
  ffi_dblfl *ಕೊನೆ_pfr = pfr + ಅಂಕೆ_FPR_ARG_REGISTERS64;
  float128 *ಕೊನೆ_pvec = pvec + ಅಂಕೆ_VEC_ARG_REGISTERS64;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ align;

  avalue = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (ಶೂನ್ಯ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#ಆದರೆ _CALL_ELF != 2
  nfixedargs = (ಚಿಹ್ನೆರಹಿತ) -1;
  ಆದರೆ ((cif->flags & FLAG_COMPAT) == 0)
#ಆದರೆ_ಅಂತ್ಯ
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಆಗ (i < avn)
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ elt, elnum;

      ಆಯ್ಕೆ (arg_types[i]->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	  avalue[i] = (ಅಕ್ಷರ *) pst + 7;
	  pst++;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	  avalue[i] = (ಅಕ್ಷರ *) pst + 6;
	  pst++;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	  avalue[i] = (ಅಕ್ಷರ *) pst + 4;
	  pst++;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = pst;
	  pst++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಆದರೆ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      ಆದರೆ (align > 16)
		align = 16;
	      ಆದರೆ (align > 1)
		pst = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  ಆದರೆ (elt)
	    {
#ಆದರೆ _CALL_ELF == 2
	      ಒಕ್ಕೂಟ {
		ಶೂನ್ಯ *v;
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ul;
		ದಶಮಾಂಶ *f;
		ದ್ವಿಗುಣ *d;
		float128 *f128;
		size_t p;
	      } ಗಮ್ಯ, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಆದರೆ (pvec + elnum <= ಕೊನೆ_pvec)
                    ಗಮ್ಯ.v = pvec;
                  ಇಲ್ಲದಿದ್ದರೆ
                    ಗಮ್ಯ.v = pst;
                }
              ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	      ಆದರೆ (pfr + elnum <= ಕೊನೆ_pfr)
		ಗಮ್ಯ.v = pfr;
	      ಇಲ್ಲದಿದ್ದರೆ
		ಗಮ್ಯ.v = pst;

	      avalue[i] = ಗಮ್ಯ.v;
	      from.ul = pst;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ಮಾಡು
                    {
                      ಆದರೆ (pvec < ಕೊನೆ_pvec && i < nfixedargs)
		        ಸ್ಮರಣೆ_ನಕಲು (ಗಮ್ಯ.f128, pvec++, ಗಾತ್ರ (float128));
                      ಇಲ್ಲದಿದ್ದರೆ
		        ಸ್ಮರಣೆ_ನಕಲು (ಗಮ್ಯ.f128, from.f128, ಗಾತ್ರ (float128));
                      ಗಮ್ಯ.f128++;
                      from.f128++;
                    }
                  ಆಗ (--elnum != 0);
                }
              ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	      ಆದರೆ (elt == FFI_TYPE_FLOAT)
		{
		  ಮಾಡು
		    {
		      ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
			{
			  *ಗಮ್ಯ.f = (ದಶಮಾಂಶ) pfr->d;
			  pfr++;
			}
		      ಇಲ್ಲದಿದ್ದರೆ
			*ಗಮ್ಯ.f = *from.f;
		      ಗಮ್ಯ.f++;
		      from.f++;
		    }
		  ಆಗ (--elnum != 0);
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		{
		  ಮಾಡು
		    {
		      ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
			{
			  *ಗಮ್ಯ.d = pfr->d;
			  pfr++;
			}
		      ಇಲ್ಲದಿದ್ದರೆ
			*ಗಮ್ಯ.d = *from.d;
		      ಗಮ್ಯ.d++;
		      from.d++;
		    }
		  ಆಗ (--elnum != 0);
		}
#ಇಲ್ಲದಿದ್ದರೆ
	      ಆದರೆ (elt == FFI_TYPE_FLOAT)
		ಹೋಗು do_float;
	      ಇಲ್ಲದಿದ್ದರೆ
		ಹೋಗು do_double;
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      ಆದರೆ (arg_types[i]->ಗಾತ್ರ_2 < 8)
		avalue[i] = (ಅಕ್ಷರ *) pst + 8 - arg_types[i]->ಗಾತ್ರ_2;
	      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->ಗಾತ್ರ_2 + 7) / 8;
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
          ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ಆದರೆ (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) pst & 0xF) != 0)
                ++pst;
              ಆದರೆ (pvec < ಕೊನೆ_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              ಇಲ್ಲದಿದ್ದರೆ
                avalue[i] = pst;
              pst += 2;
              ನಿಲ್ಲಿಸು;
            }
          ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      ಆದರೆ (pfr + 1 < ಕೊನೆ_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		{
		  ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ನಿಲ್ಲಿಸು;
	    }
	  /* Fall through.  */
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
#ಆದರೆ _CALL_ELF != 2
	do_double:
#ಆದರೆ_ಅಂತ್ಯ
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = pst;
	  pst++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
#ಆದರೆ _CALL_ELF != 2
	do_float:
#ಆದರೆ_ಅಂತ್ಯ
	  ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ದಶಮಾಂಶ) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	      avalue[i] = (ಅಕ್ಷರ *) pst + 4;
#ಇಲ್ಲದಿದ್ದರೆ
	      avalue[i] = pst;
#ಆದರೆ_ಅಂತ್ಯ
	    }
	  pst++;
	  ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ j;
	    elt = arg_types[i]->elements[0]->type;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * ಗಾತ್ರ (float128));
		ಆದರೆ (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) pst & 0xF) != 0)
		  ++pst;
		ಗಾಗಿ (j = 0; j < 2; j++)
		  {
		    ಆದರೆ (pvec < ಕೊನೆ_pvec && i < nfixedargs)
		      ಸ್ಮರಣೆ_ನಕಲು (&cval[j], pvec++, ಗಾತ್ರ (float128));
		    ಇಲ್ಲದಿದ್ದರೆ
		      ಸ್ಮರಣೆ_ನಕಲು (&cval[j], pst, ಗಾತ್ರ (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ನಿಲ್ಲಿಸು;
	      }
	    ಆದರೆ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ದ್ವಿಗುಣ *cval = alloca (4 * ಗಾತ್ರ (ದ್ವಿಗುಣ));
		ಗಾಗಿ (j = 0; j < 4; j++)
		  {
		    ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ಇಲ್ಲದಿದ್ದರೆ
		      cval[j] = *(ದ್ವಿಗುಣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ನಿಲ್ಲಿಸು;
	      }
#ಆದರೆ_ಅಂತ್ಯ
	    ಆದರೆ (elt == FFI_TYPE_FLOAT)
	      {
		ದಶಮಾಂಶ *cval = alloca (2 * ಗಾತ್ರ (ದಶಮಾಂಶ));
		ಗಾಗಿ (j = 0; j < 2; j++)
		  {
		    ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
		      {
			cval[j] = (ದಶಮಾಂಶ) pfr->d;
			pfr++;
		      }
		    ಇಲ್ಲದಿದ್ದರೆ
		      {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
			cval[j] = ((ದಶಮಾಂಶ *) pst)[1];
#ಇಲ್ಲದಿದ್ದರೆ
			cval[j] = ((ದಶಮಾಂಶ *) pst)[0];
#ಆದರೆ_ಅಂತ್ಯ
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (elt == FFI_TYPE_DOUBLE)
	      {
		ದ್ವಿಗುಣ *cval = alloca (2 * ಗಾತ್ರ (ದ್ವಿಗುಣ));
		ಗಾಗಿ (j = 0; j < 2; j++)
		  {
		    ಆದರೆ (pfr < ಕೊನೆ_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ಇಲ್ಲದಿದ್ದರೆ
		      cval[j] = *(ದ್ವಿಗುಣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->ಗಾತ್ರ_2;
		ಅಕ್ಷರ *cval = alloca (2 * hsize);
		ಗಾಗಿ (j = 0; j < 2; j++)
		  {
		    ಅಕ್ಷರ *src;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
		    src = (ಅಕ್ಷರ *) pst + (8 - hsize);
#ಇಲ್ಲದಿದ್ದರೆ
		    src = (ಅಕ್ಷರ *) pst;
#ಆದರೆ_ಅಂತ್ಯ
		    ಸ್ಮರಣೆ_ನಕಲು (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಹಿಂತಿರುಗಿ PPC_LD_F32;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ಹಿಂತಿರುಗಿ PPC_LD_F64;
#ಆದರೆ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ಆದರೆ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	ಹಿಂತಿರುಗಿ PPC64_LD_ಸದಿಶ;
      ಹಿಂತಿರುಗಿ PPC_LD_F128;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_U8;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_S8;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_U16;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_S16;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_U32;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_S32;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಹಿಂತಿರುಗಿ PPC_LD_PTR;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ಹಿಂತಿರುಗಿ PPC_LD_I64;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      {
	ಪೂರ್ಣಾಂಕ inner = cif->rtype->elements[0]->type;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಆದರೆ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    ಆದರೆ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      ಹಿಂತಿರುಗಿ PPC64_LD_ಸದಿಶ_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (inner == FFI_TYPE_FLOAT)
	  ಹಿಂತಿರುಗಿ PPC64_LD_FLOAT_HOMOG;
	ಆದರೆ (inner == FFI_TYPE_DOUBLE)
	  ಹಿಂತಿರುಗಿ PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  ಅಕ್ಷರ *rv = rvalue;
	  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ re, im;
	  ಆಯ್ಕೆ (inner)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	      re = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) rv)[0];
	      im = ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	      re = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ *) rv)[0];
	      im = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	      re = ((ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) rv)[0];
	      im = ((ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	      re = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ *) rv)[0];
	      im = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	      re = ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) rv)[0];
	      im = ((ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_INT:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	      re = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) rv)[0];
	      im = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ದೀರ್ಘ) ((ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      re = ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rv)[0];
	      im = ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rv)[1];
	      ನಿಲ್ಲಿಸು;
	    }
	  ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rv)[0] = re;
	  ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) rv)[1] = im;
	  ಹಿಂತಿರುಗಿ PPC_LD_R3R4;
	}
      }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  ಆದರೆ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      ಆಯ್ಕೆ (cif->rtype->ಗಾತ್ರ_2)
		{
		ಸ್ಥಿತಿ 0:
		  ಹಿಂತಿರುಗಿ PPC_LD_NONE;
		ಸ್ಥಿತಿ 1:
		  ಹಿಂತಿರುಗಿ PPC_LD_U8;
		ಸ್ಥಿತಿ 2:
		  ಹಿಂತಿರುಗಿ PPC_LD_U16;
		ಸ್ಥಿತಿ 3:
		  ಹಿಂತಿರುಗಿ PPC64_LD_STRUCT_3;
		ಸ್ಥಿತಿ 4:
		  ಹಿಂತಿರುಗಿ PPC_LD_U32;
		ಸ್ಥಿತಿ 5:
		  ಹಿಂತಿರುಗಿ PPC64_LD_STRUCT_5;
		ಸ್ಥಿತಿ 6:
		  ಹಿಂತಿರುಗಿ PPC64_LD_STRUCT_6;
		ಸ್ಥಿತಿ 7:
		  ಹಿಂತಿರುಗಿ PPC64_LD_STRUCT_7;
		ಸ್ಥಿತಿ 8 ... 16:
		  ಹಿಂತಿರುಗಿ PPC_LD_R3R4;
		}
	      ನಿಲ್ಲಿಸು;
	    }
	  ಆದರೆ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    ಹಿಂತಿರುಗಿ PPC64_LD_ಸದಿಶ_HOMOG;
	  ಆದರೆ ((cif->flags & FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು) != 0)
	    ಹಿಂತಿರುಗಿ PPC64_LD_DOUBLE_HOMOG;
	  ಹಿಂತಿರುಗಿ PPC64_LD_FLOAT_HOMOG;
        }
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    }
  abort();
}
#ಆದರೆ_ಅಂತ್ಯ

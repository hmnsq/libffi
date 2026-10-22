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

#ਅਨ੍ਤਰ੍ਭਾਵ "ffi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_common.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ਗਿਣਤੀ {
  ਅੰਕ_GPR_ARG_REGISTERS64 = 8,
  ਅੰਕ_FPR_ARG_REGISTERS64 = 13,
  ਅੰਕ_VEC_ARG_REGISTERS64 = 12,
};
ਗਿਣਤੀ { ASM_NEEDS_REGISTERS64 = 4 };


#ਜੇ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ਖਾਲੀ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  ਜੇ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.ਮਾਪ = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ਨਹੀਂਤਾਂ
    {
      ffi_type_longdouble.ਮਾਪ = 16;
      ffi_type_longdouble.alignment = 16;
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
  /* _Complex long double is two halves of whatever long double just became.
     Its size comes from sizeof(_Complex long double) at build time, which is
     only right for the ABI libffi itself was built for, so keep it in step.
     Leaving it behind makes the type disagree with its own element: a caller
     that asks for the other long double width would get a return value copied
     at cif->rtype->size (wrong by 16 bytes either way) and structs containing
     the type laid out to the build-time size.  */
  ffi_type_complex_longdouble.ਮਾਪ = 2 * ffi_type_longdouble.ਮਾਪ;
  ffi_type_complex_longdouble.alignment = ffi_type_longdouble.alignment;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ


ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
discover_homogeneous_aggregate (ffi_abi abi,
                                ਅਚਲ ffi_type *t,
                                ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *elnum)
{
  ਚੋਣ (t->type)
    {
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      ਜੇ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          ਵਾਪਸ FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      ਨਹੀਂਤਾਂ ਜੇ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          ਵਾਪਸ FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      *elnum = 1;
      ਵਾਪਸ (ਪੂਰਨ) t->type;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ inner_elnum = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	ਜੇ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    ਵਾਪਸ inner;
	  }
	ਵਾਪਸ 0;
      }
#ਜੇ_ਅੰਤ

    ਹਾਲਤ FFI_TYPE_STRUCT:;
      {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਅਧਾਰ_elt = 0, ਕੁੱਲ_elnum = 0;
	ffi_type **el = t->elements;
	ਜਦੋਂ (*el)
	  {
	    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    ਜੇ (el_elt == 0
		|| (ਅਧਾਰ_elt && ਅਧਾਰ_elt != el_elt))
	      ਵਾਪਸ 0;
	    ਅਧਾਰ_elt = el_elt;
	    ਕੁੱਲ_elnum += el_elnum;
#ਜੇ _CALL_ELF == 2
	    ਜੇ (ਕੁੱਲ_elnum > 8)
	      ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	    ਜੇ (ਕੁੱਲ_elnum > 1)
	      ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
	    el++;
	  }
	*elnum = ਕੁੱਲ_elnum;
	ਵਾਪਸ ਅਧਾਰ_elt;
      }

    ਮੂਲ:
      ਵਾਪਸ 0;
    }
}


/* Perform machine dependent cif processing */
ਸਥਿਰ ffi_ਸਥਿਤੀ
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i, fparg_ਗਿਣਤੀ = 0, intarg_ਗਿਣਤੀ = 0, vecarg_ਗਿਣਤੀ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = cif->flags;
  ਬਿਨਾਨਿਸ਼ਾਨੀ elt, elnum, rtype;

#ਜੇ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ਵਾਪਸ FFI_BAD_ABI;
#ਜਾਂਜੇ !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ਵਾਪਸ FFI_BAD_ABI;
#ਨਹੀਂਤਾਂ
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#ਜੇ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  ਬਾਈਟਾਂ = (4 + ASM_NEEDS_REGISTERS64) * ਆਕਾਰ (ਲੰਮਾ);

  /* Space for the general registers.  */
  ਬਾਈਟਾਂ += ਅੰਕ_GPR_ARG_REGISTERS64 * ਆਕਾਰ (ਲੰਮਾ);
#ਨਹੀਂਤਾਂ
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  ਬਾਈਟਾਂ = (6 + ASM_NEEDS_REGISTERS64) * ਆਕਾਰ (ਲੰਮਾ);

  /* Space for the mandatory parm save area and general registers.  */
  ਬਾਈਟਾਂ += 2 * ਅੰਕ_GPR_ARG_REGISTERS64 * ਆਕਾਰ (ਲੰਮਾ);
#ਜੇ_ਅੰਤ

  /* Return value handling.  */
  rtype = cif->rtype->type;
#ਜੇ _CALL_ELF == 2
homogeneous:
#ਜੇ_ਅੰਤ
  ਚੋਣ (rtype)
    {
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ਰੋਕੋ;
        }
      ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128_ਬਿੱਟ;
      /* Fall through.  */
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      ਜੇ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          ਜਾਓ homogeneous;
        }
      ਜੇ (cif->rtype->ਮਾਪ <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ਰੋਕੋ;
        }
#ਜੇ_ਅੰਤ
      intarg_ਗਿਣਤੀ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      ਚੋਣ (rtype)
        {
        ਹਾਲਤ FFI_TYPE_FLOAT:
        ਹਾਲਤ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          ਜਾਓ homogeneous;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
          ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ਰੋਕੋ;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          ਜਾਓ homogeneous;
#ਜੇ_ਅੰਤ
        ਹਾਲਤ FFI_TYPE_INT:
        ਹਾਲਤ FFI_TYPE_SINT8:  ਹਾਲਤ FFI_TYPE_UINT8:
        ਹਾਲਤ FFI_TYPE_SINT16: ਹਾਲਤ FFI_TYPE_UINT16:
        ਹਾਲਤ FFI_TYPE_SINT32: ਹਾਲਤ FFI_TYPE_UINT32:
        ਹਾਲਤ FFI_TYPE_SINT64: ਹਾਲਤ FFI_TYPE_UINT64:
        ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ਰੋਕੋ;
        ਮੂਲ:
          ਵਾਪਸ FFI_BAD_TYPEDEF;
        }
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਮੂਲ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ਰੋਕੋ;
    }

  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ align;

      ਚੋਣ ((*ptr)->type)
	{
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
          ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ਗਿਣਤੀ++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ਗਿਣਤੀ = (intarg_ਗਿਣਤੀ + 3) & ~0x1;
              ਜੇ (vecarg_ਗਿਣਤੀ > ਅੰਕ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ਰੋਕੋ;
            }
	  ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ਗਿਣਤੀ++;
	      intarg_ਗਿਣਤੀ++;
	    }
	  /* Fall through.  */
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  fparg_ਗਿਣਤੀ++;
	  intarg_ਗਿਣਤੀ++;
	  ਜੇ (fparg_ਗਿਣਤੀ > ਅੰਕ_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਜੇ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ਜੇ (align > 16)
		align = 16;
	      align = align / 8;
	      ਜੇ (align > 1)
		intarg_ਗਿਣਤੀ = FFI_ALIGN (intarg_ਗਿਣਤੀ, align);
	    }
	  intarg_ਗਿਣਤੀ += ((*ptr)->ਮਾਪ + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          ਜੇ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ਗਿਣਤੀ += elnum;
              ਜੇ (vecarg_ਗਿਣਤੀ > ਅੰਕ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ਰੋਕੋ;
            }
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	  ਜੇ (elt)
	    {
	      fparg_ਗਿਣਤੀ += elnum;
	      ਜੇ (fparg_ਗਿਣਤੀ > ਅੰਕ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਜੇ (intarg_ਗਿਣਤੀ > ਅੰਕ_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ਗਿਣਤੀ++;
	  ਜੇ (intarg_ਗਿਣਤੀ > ਅੰਕ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ਜੇ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#ਜੇ_ਅੰਤ
	  ਚੋਣ (elt)
	    {
	    ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਹਾਲਤ FFI_TYPE_DOUBLE:
	      fparg_ਗਿਣਤੀ += 2;
	      intarg_ਗਿਣਤੀ += 2;
	      ਜੇ (fparg_ਗਿਣਤੀ > ਅੰਕ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ਰੋਕੋ;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	      ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ਗਿਣਤੀ += 2;
		  intarg_ਗਿਣਤੀ = (intarg_ਗਿਣਤੀ + 1) & ~0x1;
		  intarg_ਗਿਣਤੀ += 4;
		  ਜੇ (vecarg_ਗਿਣਤੀ > ਅੰਕ_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ਰੋਕੋ;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ਗਿਣਤੀ += 4;
	      intarg_ਗਿਣਤੀ += 4;
	      ਜੇ (fparg_ਗਿਣਤੀ > ਅੰਕ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	    ਹਾਲਤ FFI_TYPE_INT:
	    ਹਾਲਤ FFI_TYPE_SINT8:  ਹਾਲਤ FFI_TYPE_UINT8:
	    ਹਾਲਤ FFI_TYPE_SINT16: ਹਾਲਤ FFI_TYPE_UINT16:
	    ਹਾਲਤ FFI_TYPE_SINT32: ਹਾਲਤ FFI_TYPE_UINT32:
	    ਹਾਲਤ FFI_TYPE_SINT64: ਹਾਲਤ FFI_TYPE_UINT64:
	    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	      intarg_ਗਿਣਤੀ += 2;
	      ਰੋਕੋ;
	    ਮੂਲ:
	      ਵਾਪਸ FFI_BAD_TYPEDEF;
	    }
	  ਜੇ (intarg_ਗਿਣਤੀ > ਅੰਕ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਮੂਲ:
	  FFI_ASSERT (0);
	}
    }

  ਜੇ (fparg_ਗਿਣਤੀ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ਜੇ (intarg_ਗਿਣਤੀ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ਜੇ (vecarg_ਗਿਣਤੀ != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  ਜੇ (fparg_ਗਿਣਤੀ != 0)
    ਬਾਈਟਾਂ += ਅੰਕ_FPR_ARG_REGISTERS64 * ਆਕਾਰ (ਦੁੱਗਣਾ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  ਜੇ (vecarg_ਗਿਣਤੀ != 0) {
    ਬਾਈਟਾਂ = (ਬਾਈਟਾਂ + 15) & ~0xF;
    ਬਾਈਟਾਂ += ਅੰਕ_VEC_ARG_REGISTERS64 * ਆਕਾਰ (float128);
  }

  /* Stack space.  */
#ਜੇ _CALL_ELF == 2
  ਜੇ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    ਬਾਈਟਾਂ += intarg_ਗਿਣਤੀ * ਆਕਾਰ (ਲੰਮਾ);
#ਨਹੀਂਤਾਂ
  ਜੇ (intarg_ਗਿਣਤੀ > ਅੰਕ_GPR_ARG_REGISTERS64)
    ਬਾਈਟਾਂ += (intarg_ਗਿਣਤੀ - ਅੰਕ_GPR_ARG_REGISTERS64) * ਆਕਾਰ (ਲੰਮਾ);
#ਜੇ_ਅੰਤ

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ਬਾਈਟਾਂ = (ਬਾਈਟਾਂ + 15) & ~0xF;

  cif->flags = flags;
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  ਜੇ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#ਜੇ _CALL_ELF != 2
  ਨਹੀਂਤਾਂ ਜੇ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_BAD_ABI;
  ਵਾਪਸ ffi_prep_cif_linux64_core (cif);
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs MAYBE_ਅਣਵਰਤਿਆ)
{
  ਜੇ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#ਜੇ _CALL_ELF != 2
  ਨਹੀਂਤਾਂ ਜੇ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ ffi_prep_cif_linux64_core (cif);
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

ਖਾਲੀ FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਚਲ ਢੇਰ)
{
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਬਾਈਟਾਂ = ecif->cif->ਬਾਈਟਾਂ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ flags = ecif->cif->flags;

  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ
  {
    ਅੱਖਰ *c;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ul;
    ਦਸ਼ਮਲਵ *f;
    ਦੁੱਗਣਾ *d;
    float128 *f128;
    size_t p;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'next_arg' points at the space for gpr3, and grows upwards as
     we use GPR registers, then continues at rest.  */
  valp gpr_ਅਧਾਰ;
  valp gpr_ਸਿਰਾ;
  valp rest;
  valp ਅਗਲਾ_arg;

  /* 'fpr_base' points at the space for f1, and grows upwards as
     we use FPR registers.  */
  valp fpr_ਅਧਾਰ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ fparg_ਗਿਣਤੀ;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_ਅਧਾਰ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ vecarg_ਗਿਣਤੀ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i, ਸ਼ਬਦ, nargs, nfixedargs;
  ffi_type **ptr;
  ਦੁੱਗਣਾ double_tmp;
  ਸੰਘ
  {
    ਖਾਲੀ **v;
    ਅੱਖਰ **c;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ **sc;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **uc;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ **ss;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ **us;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ **si;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ **ui;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ **ul;
    ਦਸ਼ਮਲਵ **f;
    ਦੁੱਗਣਾ **d;
    float128 **f128;
  } p_argv;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ gprvalue;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ align;

  stacktop.c = (ਅੱਖਰ *) ਢੇਰ + ਬਾਈਟਾਂ;
  gpr_ਅਧਾਰ.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - ਅੰਕ_GPR_ARG_REGISTERS64;
  gpr_ਸਿਰਾ.ul = gpr_ਅਧਾਰ.ul + ਅੰਕ_GPR_ARG_REGISTERS64;
#ਜੇ _CALL_ELF == 2
  rest.ul = ਢੇਰ + 4 + ਅੰਕ_GPR_ARG_REGISTERS64;
#ਨਹੀਂਤਾਂ
  rest.ul = ਢੇਰ + 6 + ਅੰਕ_GPR_ARG_REGISTERS64;
#ਜੇ_ਅੰਤ
  fpr_ਅਧਾਰ.d = gpr_ਅਧਾਰ.d - ਅੰਕ_FPR_ARG_REGISTERS64;
  fparg_ਗਿਣਤੀ = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  ਜੇ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_ਅਧਾਰ.p = fpr_ਅਧਾਰ.p & ~0xF;
  ਨਹੀਂਤਾਂ
    vec_ਅਧਾਰ.p = gpr_ਅਧਾਰ.p;
  vec_ਅਧਾਰ.f128 -= ਅੰਕ_VEC_ARG_REGISTERS64;
  vecarg_ਗਿਣਤੀ = 0;
  ਅਗਲਾ_arg.ul = gpr_ਅਧਾਰ.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਅੱਖਰ *) ਢੇਰ & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) gpr_ਅਧਾਰ.c & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) gpr_ਸਿਰਾ.c  & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) vec_ਅਧਾਰ.c & 0xF) == 0);
  FFI_ASSERT ((ਬਾਈਟਾਂ & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  ਜੇ (flags & FLAG_RETVAL_REFERENCE)
    *ਅਗਲਾ_arg.ul++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਅੱਖਰ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#ਜੇ _CALL_ELF != 2
  nfixedargs = (ਬਿਨਾਨਿਸ਼ਾਨੀ) -1;
  ਜੇ ((flags & FLAG_COMPAT) == 0)
#ਜੇ_ਅੰਤ
    nfixedargs = ecif->cif->nfixedargs;
  ਲਈ (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ elt, elnum;

      ਚੋਣ ((*ptr)->type)
	{
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
          ਜੇ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ਅਗਲਾ_arg.p = FFI_ALIGN (ਅਗਲਾ_arg.p, 16);
              ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
                ਅਗਲਾ_arg.ul = rest.ul;
              ਜੇ (vecarg_ਗਿਣਤੀ < ਅੰਕ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (vec_ਅਧਾਰ.f128++, *p_argv.f128, ਆਕਾਰ (float128));
              ਨਹੀਂਤਾਂ
		ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਅਗਲਾ_arg.f128, *p_argv.f128, ਆਕਾਰ (float128));
              ਜੇ (++ਅਗਲਾ_arg.f128 == gpr_ਸਿਰਾ.f128)
                ਅਗਲਾ_arg.f128 = rest.f128;
              vecarg_ਗਿਣਤੀ++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ਰੋਕੋ;
            }
	  ਜੇ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ਅਧਾਰ.d++ = double_tmp;
# ਜੇ _CALL_ELF != 2
		  ਜੇ ((flags & FLAG_COMPAT) != 0)
		    *ਅਗਲਾ_arg.d = double_tmp;
# endif
		}
	      ਨਹੀਂਤਾਂ
		*ਅਗਲਾ_arg.d = double_tmp;
	      ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		ਅਗਲਾ_arg.ul = rest.ul;
	      fparg_ਗਿਣਤੀ++;
	      double_tmp = (*p_argv.d)[1];
	      ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_ਅਧਾਰ.d++ = double_tmp;
# ਜੇ _CALL_ELF != 2
		  ਜੇ ((flags & FLAG_COMPAT) != 0)
		    *ਅਗਲਾ_arg.d = double_tmp;
# endif
		}
	      ਨਹੀਂਤਾਂ
		*ਅਗਲਾ_arg.d = double_tmp;
	      ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		ਅਗਲਾ_arg.ul = rest.ul;
	      fparg_ਗਿਣਤੀ++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ਰੋਕੋ;
	    }
	  /* Fall through.  */
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_DOUBLE:
#ਜੇ _CALL_ELF != 2
	do_double:
#ਜੇ_ਅੰਤ
	  double_tmp = **p_argv.d;
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ਅਧਾਰ.d++ = double_tmp;
#ਜੇ _CALL_ELF != 2
	      ਜੇ ((flags & FLAG_COMPAT) != 0)
		*ਅਗਲਾ_arg.d = double_tmp;
#ਜੇ_ਅੰਤ
	    }
	  ਨਹੀਂਤਾਂ
	    *ਅਗਲਾ_arg.d = double_tmp;
	  ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
	    ਅਗਲਾ_arg.ul = rest.ul;
	  fparg_ਗਿਣਤੀ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
#ਜੇ _CALL_ELF != 2
	do_float:
#ਜੇ_ਅੰਤ
	  double_tmp = **p_argv.f;
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_ਅਧਾਰ.d++ = double_tmp;
#ਜੇ _CALL_ELF != 2
	      ਜੇ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  ਅਗਲਾ_arg.f[1] = (ਦਸ਼ਮਲਵ) double_tmp;
# ਨਹੀਂਤਾਂ
		  ਅਗਲਾ_arg.f[0] = (ਦਸ਼ਮਲਵ) double_tmp;
# endif
		}
#ਜੇ_ਅੰਤ
	    }
	  ਨਹੀਂਤਾਂ
	    {
# ifndef __LITTLE_ENDIAN__
	      ਅਗਲਾ_arg.f[1] = (ਦਸ਼ਮਲਵ) double_tmp;
# ਨਹੀਂਤਾਂ
	      ਅਗਲਾ_arg.f[0] = (ਦਸ਼ਮਲਵ) double_tmp;
# endif
	    }
	  ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
	    ਅਗਲਾ_arg.ul = rest.ul;
	  fparg_ਗਿਣਤੀ++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ਜੇ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  ਜੇ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	      ਲਈ (j = 0; j < 2; j++)
		{
		  ਅਗਲਾ_arg.p = FFI_ALIGN (ਅਗਲਾ_arg.p, 16);
		  ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		  ਜੇ (vecarg_ਗਿਣਤੀ < ਅੰਕ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (vec_ਅਧਾਰ.f128++, cval + j, ਆਕਾਰ (float128));
		  ਨਹੀਂਤਾਂ
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਅਗਲਾ_arg.f128, cval + j, ਆਕਾਰ (float128));
		  ਜੇ (++ਅਗਲਾ_arg.f128 == gpr_ਸਿਰਾ.f128)
		    ਅਗਲਾ_arg.f128 = rest.f128;
		  vecarg_ਗਿਣਤੀ++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ਰੋਕੋ;
	    }
	  ਜੇ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ਦੁੱਗਣਾ *cval = (ਦੁੱਗਣਾ *) *p_argv.v;
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	      ਲਈ (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ਅਧਾਰ.d++ = double_tmp;
		  ਨਹੀਂਤਾਂ
		    *ਅਗਲਾ_arg.d = double_tmp;
		  ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		  fparg_ਗਿਣਤੀ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ਰੋਕੋ;
	    }
#ਜੇ_ਅੰਤ
	  ਜੇ (elt == FFI_TYPE_FLOAT)
	    {
	      ਦਸ਼ਮਲਵ *cval = (ਦਸ਼ਮਲਵ *) *p_argv.v;
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	      ਲਈ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ਅਧਾਰ.d++ = double_tmp;
		  ਨਹੀਂਤਾਂ
		    {
# ifndef __LITTLE_ENDIAN__
		      ਅਗਲਾ_arg.f[1] = (ਦਸ਼ਮਲਵ) double_tmp;
# ਨਹੀਂਤਾਂ
		      ਅਗਲਾ_arg.f[0] = (ਦਸ਼ਮਲਵ) double_tmp;
# endif
		    }
		  ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		  fparg_ਗਿਣਤੀ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (elt == FFI_TYPE_DOUBLE)
	    {
	      ਦੁੱਗਣਾ *cval = (ਦੁੱਗਣਾ *) *p_argv.v;
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	      ਲਈ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_ਅਧਾਰ.d++ = double_tmp;
		  ਨਹੀਂਤਾਂ
		    *ਅਗਲਾ_arg.d = double_tmp;
		  ਜੇ (++ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		  fparg_ਗਿਣਤੀ++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      ਅੱਖਰ *cval = (ਅੱਖਰ *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->ਮਾਪ;
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	      ਲਈ (j = 0; j < 2; j++)
		{
		  ਅੱਖਰ *half = cval + j * hsize;
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ gprvalue;
		  ਚੋਣ (elt)
		    {
		    ਹਾਲਤ FFI_TYPE_UINT8:
		      gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) half; ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_SINT8:
		      gprvalue = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ *) half;
		      ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_UINT16:
		      gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) half; ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_SINT16:
		      gprvalue = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ *) half;
		      ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_UINT32:
		      gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) half; ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_INT:
		    ਹਾਲਤ FFI_TYPE_SINT32:
		      gprvalue = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) half;
		      ਰੋਕੋ;
		    ਹਾਲਤ FFI_TYPE_SINT64:
		    ਹਾਲਤ FFI_TYPE_UINT64:
		    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
		    ਮੂਲ:
		      gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) half; ਰੋਕੋ;
		    }
		  *ਅਗਲਾ_arg.ul++ = gprvalue;
		  ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		}
	    }
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਜੇ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ਜੇ (align > 16)
		align = 16;
	      ਜੇ (align > 1)
                {
                  ਅਗਲਾ_arg.p = FFI_ALIGN (ਅਗਲਾ_arg.p, align);
                  ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
                    ਅਗਲਾ_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  ਜੇ (elt)
	    {
#ਜੇ _CALL_ELF == 2
	      ਸੰਘ {
		ਖਾਲੀ *v;
		ਦਸ਼ਮਲਵ *f;
		ਦੁੱਗਣਾ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ਜੇ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ਕਰੋ
                    {
                      ਜੇ (vecarg_ਗਿਣਤੀ < ਅੰਕ_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        ਸਮ੍ਰਿਤੀ_ਨਕਲ (vec_ਅਧਾਰ.f128++, arg.f128++, ਆਕਾਰ (float128));
                      ਨਹੀਂਤਾਂ
		        ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਅਗਲਾ_arg.f128, arg.f128++, ਆਕਾਰ (float128));
                      ਜੇ (++ਅਗਲਾ_arg.f128 == gpr_ਸਿਰਾ.f128)
                        ਅਗਲਾ_arg.f128 = rest.f128;
                      vecarg_ਗਿਣਤੀ++;
                    }
                  ਜਦੋਂ (--elnum != 0);
                }
              ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	      ਜੇ (elt == FFI_TYPE_FLOAT)
		{
		  ਕਰੋ
		    {
		      double_tmp = *arg.f++;
		      ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_ਅਧਾਰ.d++ = double_tmp;
		      ਨਹੀਂਤਾਂ
			*ਅਗਲਾ_arg.f = (ਦਸ਼ਮਲਵ) double_tmp;
		      ਜੇ (++ਅਗਲਾ_arg.f == gpr_ਸਿਰਾ.f)
			ਅਗਲਾ_arg.f = rest.f;
		      fparg_ਗਿਣਤੀ++;
		    }
		  ਜਦੋਂ (--elnum != 0);
		  ਜੇ ((ਅਗਲਾ_arg.p & 7) != 0)
                    ਜੇ (++ਅਗਲਾ_arg.f == gpr_ਸਿਰਾ.f)
                      ਅਗਲਾ_arg.f = rest.f;
		}
	      ਨਹੀਂਤਾਂ
		ਕਰੋ
		  {
		    double_tmp = *arg.d++;
		    ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_ਅਧਾਰ.d++ = double_tmp;
		    ਨਹੀਂਤਾਂ
		      *ਅਗਲਾ_arg.d = double_tmp;
		    ਜੇ (++ਅਗਲਾ_arg.d == gpr_ਸਿਰਾ.d)
		      ਅਗਲਾ_arg.d = rest.d;
		    fparg_ਗਿਣਤੀ++;
		  }
		ਜਦੋਂ (--elnum != 0);
#ਨਹੀਂਤਾਂ
	      ਜੇ (elt == FFI_TYPE_FLOAT)
		ਜਾਓ do_float;
	      ਨਹੀਂਤਾਂ
		ਜਾਓ do_double;
#ਜੇ_ਅੰਤ
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਸ਼ਬਦ = ((*ptr)->ਮਾਪ + 7) / 8;
	      ਜੇ (ਅਗਲਾ_arg.ul >= gpr_ਅਧਾਰ.ul && ਅਗਲਾ_arg.ul + ਸ਼ਬਦ > gpr_ਸਿਰਾ.ul)
		{
		  size_t ਪਹਿਲਾ = gpr_ਸਿਰਾ.c - ਅਗਲਾ_arg.c;
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਅਗਲਾ_arg.c, *p_argv.c, ਪਹਿਲਾ);
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (rest.c, *p_argv.c + ਪਹਿਲਾ, (*ptr)->ਮਾਪ - ਪਹਿਲਾ);
		  ਅਗਲਾ_arg.c = rest.c + ਸ਼ਬਦ * 8 - ਪਹਿਲਾ;
		}
	      ਨਹੀਂਤਾਂ
		{
		  ਅੱਖਰ *where = ਅਗਲਾ_arg.c;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  ਜੇ ((*ptr)->ਮਾਪ < 8)
		    where += 8 - (*ptr)->ਮਾਪ;
#ਜੇ_ਅੰਤ
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (where, *p_argv.c, (*ptr)->ਮਾਪ);
		  ਅਗਲਾ_arg.ul += ਸ਼ਬਦ;
		  ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
		    ਅਗਲਾ_arg.ul = rest.ul;
		}
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  ਜਾਓ putgpr;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *ਅਗਲਾ_arg.ul++ = gprvalue;
	  ਜੇ (ਅਗਲਾ_arg.ul == gpr_ਸਿਰਾ.ul)
	    ਅਗਲਾ_arg.ul = rest.ul;
	  ਰੋਕੋ;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (ਅਗਲਾ_arg.ul >= gpr_ਅਧਾਰ.ul
		  && ਅਗਲਾ_arg.ul <= gpr_ਅਧਾਰ.ul + 4));
}


#ਜੇ _CALL_ELF == 2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_CACHE_ਸਤਰ_ਮਾਪ 8

ਸਥਿਰ ਖਾਲੀ
flush_icache (ਅੱਖਰ *wraddr, ਅੱਖਰ *xaddr, ਪੂਰਨ ਮਾਪ)
{
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < ਮਾਪ; i += MIN_CACHE_ਸਤਰ_ਮਾਪ)
    __asm__ ਅਸਥਿਰ ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ਅਸਥਿਰ ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + ਮਾਪ - 1), "r"(wraddr + ਮਾਪ - 1)
		    : "memory");
}
#ਜੇ_ਅੰਤ


ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_ਘੇਰਾ_loc_linux64 (ffi_ਘੇਰਾ *ਘੇਰਾ,
			      ffi_cif *cif,
			      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
			      ਖਾਲੀ *user_ਅੰਕੜੇ,
			      ਖਾਲੀ *codeloc)
{
  ਜੇ (cif->abi < FFI_LINUX || cif->abi >= FFI_ਆਖਰੀ_ABI)
    ਵਾਪਸ FFI_BAD_ABI;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਖਾਲੀ (*dest)(ਖਾਲੀ) = ffi_ਘੇਰਾ_LINUX64;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
    }
  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
#ਜੇ _CALL_ELF == 2
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(ਖਾਲੀ **) &tramp[4] = (ਖਾਲੀ *) ffi_ਘੇਰਾ_LINUX64;
      *(ਖਾਲੀ **) &tramp[6] = codeloc;
      flush_icache ((ਅੱਖਰ *) tramp, (ਅੱਖਰ *) codeloc, 4 * 4);
#ਨਹੀਂਤਾਂ
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      ਖਾਲੀ **tramp = (ਖਾਲੀ **) &ਘੇਰਾ->tramp[0];
      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&tramp[0], (ਖਾਲੀ **) ffi_ਘੇਰਾ_LINUX64, ਆਕਾਰ (ਖਾਲੀ *));
      tramp[1] = codeloc;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&tramp[2], (ਖਾਲੀ **) ffi_ਘੇਰਾ_LINUX64 + 1, ਆਕਾਰ (ਖਾਲੀ *));
#ਜੇ_ਅੰਤ
    }

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}


ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_helper_LINUX64 (ffi_cif *cif,
			    ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
			    ਖਾਲੀ *user_ਅੰਕੜੇ,
			    ਖਾਲੀ *rvalue,
			    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  ਖਾਲੀ **avalue;
  ffi_type **arg_types;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ i, avn, nfixedargs;
  ffi_dblfl *ਸਿਰਾ_pfr = pfr + ਅੰਕ_FPR_ARG_REGISTERS64;
  float128 *ਸਿਰਾ_pvec = pvec + ਅੰਕ_VEC_ARG_REGISTERS64;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ align;

  avalue = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (ਖਾਲੀ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#ਜੇ _CALL_ELF != 2
  nfixedargs = (ਬਿਨਾਨਿਸ਼ਾਨੀ) -1;
  ਜੇ ((cif->flags & FLAG_COMPAT) == 0)
#ਜੇ_ਅੰਤ
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਜਦੋਂ (i < avn)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ elt, elnum;

      ਚੋਣ (arg_types[i]->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	  avalue[i] = (ਅੱਖਰ *) pst + 7;
	  pst++;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	  avalue[i] = (ਅੱਖਰ *) pst + 6;
	  pst++;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	  avalue[i] = (ਅੱਖਰ *) pst + 4;
	  pst++;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = pst;
	  pst++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਜੇ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      ਜੇ (align > 16)
		align = 16;
	      ਜੇ (align > 1)
		pst = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  ਜੇ (elt)
	    {
#ਜੇ _CALL_ELF == 2
	      ਸੰਘ {
		ਖਾਲੀ *v;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ul;
		ਦਸ਼ਮਲਵ *f;
		ਦੁੱਗਣਾ *d;
		float128 *f128;
		size_t p;
	      } ਮੰਜ਼ਿਲ, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ਜੇ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ਜੇ (pvec + elnum <= ਸਿਰਾ_pvec)
                    ਮੰਜ਼ਿਲ.v = pvec;
                  ਨਹੀਂਤਾਂ
                    ਮੰਜ਼ਿਲ.v = pst;
                }
              ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	      ਜੇ (pfr + elnum <= ਸਿਰਾ_pfr)
		ਮੰਜ਼ਿਲ.v = pfr;
	      ਨਹੀਂਤਾਂ
		ਮੰਜ਼ਿਲ.v = pst;

	      avalue[i] = ਮੰਜ਼ਿਲ.v;
	      from.ul = pst;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ਜੇ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ਕਰੋ
                    {
                      ਜੇ (pvec < ਸਿਰਾ_pvec && i < nfixedargs)
		        ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਮੰਜ਼ਿਲ.f128, pvec++, ਆਕਾਰ (float128));
                      ਨਹੀਂਤਾਂ
		        ਸਮ੍ਰਿਤੀ_ਨਕਲ (ਮੰਜ਼ਿਲ.f128, from.f128, ਆਕਾਰ (float128));
                      ਮੰਜ਼ਿਲ.f128++;
                      from.f128++;
                    }
                  ਜਦੋਂ (--elnum != 0);
                }
              ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	      ਜੇ (elt == FFI_TYPE_FLOAT)
		{
		  ਕਰੋ
		    {
		      ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
			{
			  *ਮੰਜ਼ਿਲ.f = (ਦਸ਼ਮਲਵ) pfr->d;
			  pfr++;
			}
		      ਨਹੀਂਤਾਂ
			*ਮੰਜ਼ਿਲ.f = *from.f;
		      ਮੰਜ਼ਿਲ.f++;
		      from.f++;
		    }
		  ਜਦੋਂ (--elnum != 0);
		}
	      ਨਹੀਂਤਾਂ
		{
		  ਕਰੋ
		    {
		      ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
			{
			  *ਮੰਜ਼ਿਲ.d = pfr->d;
			  pfr++;
			}
		      ਨਹੀਂਤਾਂ
			*ਮੰਜ਼ਿਲ.d = *from.d;
		      ਮੰਜ਼ਿਲ.d++;
		      from.d++;
		    }
		  ਜਦੋਂ (--elnum != 0);
		}
#ਨਹੀਂਤਾਂ
	      ਜੇ (elt == FFI_TYPE_FLOAT)
		ਜਾਓ do_float;
	      ਨਹੀਂਤਾਂ
		ਜਾਓ do_double;
#ਜੇ_ਅੰਤ
	    }
	  ਨਹੀਂਤਾਂ
	    {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      ਜੇ (arg_types[i]->ਮਾਪ < 8)
		avalue[i] = (ਅੱਖਰ *) pst + 8 - arg_types[i]->ਮਾਪ;
	      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->ਮਾਪ + 7) / 8;
	  ਰੋਕੋ;

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
          ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ਜੇ (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) pst & 0xF) != 0)
                ++pst;
              ਜੇ (pvec < ਸਿਰਾ_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              ਨਹੀਂਤਾਂ
                avalue[i] = pst;
              pst += 2;
              ਰੋਕੋ;
            }
          ਨਹੀਂਤਾਂ ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      ਜੇ (pfr + 1 < ਸਿਰਾ_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      ਨਹੀਂਤਾਂ
		{
		  ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ਰੋਕੋ;
	    }
	  /* Fall through.  */
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_DOUBLE:
#ਜੇ _CALL_ELF != 2
	do_double:
#ਜੇ_ਅੰਤ
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
	    avalue[i] = pst;
	  pst++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
#ਜੇ _CALL_ELF != 2
	do_float:
#ਜੇ_ਅੰਤ
	  ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ਦਸ਼ਮਲਵ) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
	    {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	      avalue[i] = (ਅੱਖਰ *) pst + 4;
#ਨਹੀਂਤਾਂ
	      avalue[i] = pst;
#ਜੇ_ਅੰਤ
	    }
	  pst++;
	  ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ j;
	    elt = arg_types[i]->elements[0]->type;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    ਜੇ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    ਜੇ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * ਆਕਾਰ (float128));
		ਜੇ (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) pst & 0xF) != 0)
		  ++pst;
		ਲਈ (j = 0; j < 2; j++)
		  {
		    ਜੇ (pvec < ਸਿਰਾ_pvec && i < nfixedargs)
		      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&cval[j], pvec++, ਆਕਾਰ (float128));
		    ਨਹੀਂਤਾਂ
		      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&cval[j], pst, ਆਕਾਰ (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ਰੋਕੋ;
	      }
	    ਜੇ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ਦੁੱਗਣਾ *cval = alloca (4 * ਆਕਾਰ (ਦੁੱਗਣਾ));
		ਲਈ (j = 0; j < 4; j++)
		  {
		    ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ਨਹੀਂਤਾਂ
		      cval[j] = *(ਦੁੱਗਣਾ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ਰੋਕੋ;
	      }
#ਜੇ_ਅੰਤ
	    ਜੇ (elt == FFI_TYPE_FLOAT)
	      {
		ਦਸ਼ਮਲਵ *cval = alloca (2 * ਆਕਾਰ (ਦਸ਼ਮਲਵ));
		ਲਈ (j = 0; j < 2; j++)
		  {
		    ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
		      {
			cval[j] = (ਦਸ਼ਮਲਵ) pfr->d;
			pfr++;
		      }
		    ਨਹੀਂਤਾਂ
		      {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
			cval[j] = ((ਦਸ਼ਮਲਵ *) pst)[1];
#ਨਹੀਂਤਾਂ
			cval[j] = ((ਦਸ਼ਮਲਵ *) pst)[0];
#ਜੇ_ਅੰਤ
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ਨਹੀਂਤਾਂ ਜੇ (elt == FFI_TYPE_DOUBLE)
	      {
		ਦੁੱਗਣਾ *cval = alloca (2 * ਆਕਾਰ (ਦੁੱਗਣਾ));
		ਲਈ (j = 0; j < 2; j++)
		  {
		    ਜੇ (pfr < ਸਿਰਾ_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ਨਹੀਂਤਾਂ
		      cval[j] = *(ਦੁੱਗਣਾ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ਨਹੀਂਤਾਂ
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->ਮਾਪ;
		ਅੱਖਰ *cval = alloca (2 * hsize);
		ਲਈ (j = 0; j < 2; j++)
		  {
		    ਅੱਖਰ *src;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
		    src = (ਅੱਖਰ *) pst + (8 - hsize);
#ਨਹੀਂਤਾਂ
		    src = (ਅੱਖਰ *) pst;
#ਜੇ_ਅੰਤ
		    ਸਮ੍ਰਿਤੀ_ਨਕਲ (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਮੂਲ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      ਵਾਪਸ PPC_LD_NONE;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਵਾਪਸ PPC_LD_F32;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਵਾਪਸ PPC_LD_F64;
#ਜੇ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ਜੇ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	ਵਾਪਸ PPC64_LD_ਸਦਿਸ਼;
      ਵਾਪਸ PPC_LD_F128;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਵਾਪਸ PPC_LD_U8;
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਵਾਪਸ PPC_LD_S8;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਵਾਪਸ PPC_LD_U16;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਵਾਪਸ PPC_LD_S16;
    ਹਾਲਤ FFI_TYPE_UINT32:
      ਵਾਪਸ PPC_LD_U32;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      ਵਾਪਸ PPC_LD_S32;
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਵਾਪਸ PPC_LD_PTR;
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      ਵਾਪਸ PPC_LD_I64;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      {
	ਪੂਰਨ inner = cif->rtype->elements[0]->type;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਜੇ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    ਜੇ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      ਵਾਪਸ PPC64_LD_ਸਦਿਸ਼_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#ਜੇ_ਅੰਤ
	ਜੇ (inner == FFI_TYPE_FLOAT)
	  ਵਾਪਸ PPC64_LD_FLOAT_HOMOG;
	ਜੇ (inner == FFI_TYPE_DOUBLE)
	  ਵਾਪਸ PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  ਅੱਖਰ *rv = rvalue;
	  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ re, im;
	  ਚੋਣ (inner)
	    {
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      re = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) rv)[0];
	      im = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      re = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ *) rv)[0];
	      im = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      re = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) rv)[0];
	      im = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_SINT16:
	      re = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ *) rv)[0];
	      im = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_UINT32:
	      re = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) rv)[0];
	      im = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_INT:
	    ਹਾਲਤ FFI_TYPE_SINT32:
	      re = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) rv)[0];
	      im = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਲੰਮਾ) ((ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) rv)[1];
	      ਰੋਕੋ;
	    ਹਾਲਤ FFI_TYPE_SINT64:
	    ਹਾਲਤ FFI_TYPE_UINT64:
	    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	    ਮੂਲ:
	      re = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rv)[0];
	      im = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rv)[1];
	      ਰੋਕੋ;
	    }
	  ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rv)[0] = re;
	  ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rv)[1] = im;
	  ਵਾਪਸ PPC_LD_R3R4;
	}
      }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  ਜੇ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      ਚੋਣ (cif->rtype->ਮਾਪ)
		{
		ਹਾਲਤ 0:
		  ਵਾਪਸ PPC_LD_NONE;
		ਹਾਲਤ 1:
		  ਵਾਪਸ PPC_LD_U8;
		ਹਾਲਤ 2:
		  ਵਾਪਸ PPC_LD_U16;
		ਹਾਲਤ 3:
		  ਵਾਪਸ PPC64_LD_STRUCT_3;
		ਹਾਲਤ 4:
		  ਵਾਪਸ PPC_LD_U32;
		ਹਾਲਤ 5:
		  ਵਾਪਸ PPC64_LD_STRUCT_5;
		ਹਾਲਤ 6:
		  ਵਾਪਸ PPC64_LD_STRUCT_6;
		ਹਾਲਤ 7:
		  ਵਾਪਸ PPC64_LD_STRUCT_7;
		ਹਾਲਤ 8 ... 16:
		  ਵਾਪਸ PPC_LD_R3R4;
		}
	      ਰੋਕੋ;
	    }
	  ਜੇ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    ਵਾਪਸ PPC64_LD_ਸਦਿਸ਼_HOMOG;
	  ਜੇ ((cif->flags & FLAG_RETURNS_64_ਬਿੱਟ) != 0)
	    ਵਾਪਸ PPC64_LD_DOUBLE_HOMOG;
	  ਵਾਪਸ PPC64_LD_FLOAT_HOMOG;
        }
      ਵਾਪਸ PPC_LD_NONE;
    }
  abort();
}
#ਜੇ_ਅੰਤ

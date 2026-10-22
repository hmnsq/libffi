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

#ଅନ୍ତର୍ଭାଵ "ffi.h"
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ifdef POWERPC64
#ଅନ୍ତର୍ଭାଵ "ffi_common.h"
#ଅନ୍ତର୍ଭାଵ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ଗଣନା {
  ଅଙ୍କ_GPR_ARG_REGISTERS64 = 8,
  ଅଙ୍କ_FPR_ARG_REGISTERS64 = 13,
  ଅଙ୍କ_VEC_ARG_REGISTERS64 = 12,
};
ଗଣନା { ASM_NEEDS_REGISTERS64 = 4 };


#ଯଦି HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ଶୂନ୍ୟ FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  ଯଦି ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ଅନ୍ୟଥା
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


ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
discover_homogeneous_aggregate (ffi_abi abi,
                                ଅଚଳ ffi_type *t,
                                unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *elnum)
{
  ବାଛ (t->type)
    {
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      ଯଦି ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          ଫେରନ୍ତୁ FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      ଅନ୍ୟଥା ଯଦି ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          ଫେରନ୍ତୁ FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    ସ୍ଥିତି FFI_TYPE_FLOAT:
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      *elnum = 1;
      ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା inner_elnum = 0;
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	ଯଦି (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    ଫେରନ୍ତୁ inner;
	  }
	ଫେରନ୍ତୁ 0;
      }
#endif

    ସ୍ଥିତି FFI_TYPE_STRUCT:;
      {
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	ଯେତେବେଳେ (*el)
	  {
	    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    ଯଦି (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      ଫେରନ୍ତୁ 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#ଯଦି _CALL_ELF == 2
	    ଯଦି (total_elnum > 8)
	      ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	    ଯଦି (total_elnum > 1)
	      ଫେରନ୍ତୁ 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	ଫେରନ୍ତୁ base_elt;
      }

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
    }
}


/* Perform machine dependent cif processing */
ସ୍ଥିର ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_ଗଣନା = 0, intarg_ଗଣନା = 0, vecarg_ଗଣନା = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#ଯଦି FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#ନଚେତଯଦି !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#ଅନ୍ୟଥା
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#ଯଦି _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * ଆକାର (long);

  /* Space for the general registers.  */
  bytes += ଅଙ୍କ_GPR_ARG_REGISTERS64 * ଆକାର (long);
#ଅନ୍ୟଥା
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * ଆକାର (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * ଅଙ୍କ_GPR_ARG_REGISTERS64 * ଆକାର (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#ଯଦି _CALL_ELF == 2
homogeneous:
#endif
  ବାଛ (rtype)
    {
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          ଭାଙ୍ଗ;
        }
      ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
#ଯଦି _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      ଯଦି (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          ଯାଅ homogeneous;
        }
      ଯଦି (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          ଭାଙ୍ଗ;
        }
#endif
      intarg_ଗଣନା++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ଭାଙ୍ଗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      ବାଛ (rtype)
        {
        ସ୍ଥିତି FFI_TYPE_FLOAT:
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          ଯାଅ homogeneous;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
          ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              ଭାଙ୍ଗ;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          ଯାଅ homogeneous;
#endif
        ସ୍ଥିତି FFI_TYPE_INT:
        ସ୍ଥିତି FFI_TYPE_SINT8:  ସ୍ଥିତି FFI_TYPE_UINT8:
        ସ୍ଥିତି FFI_TYPE_SINT16: ସ୍ଥିତି FFI_TYPE_UINT16:
        ସ୍ଥିତି FFI_TYPE_SINT32: ସ୍ଥିତି FFI_TYPE_UINT32:
        ସ୍ଥିତି FFI_TYPE_SINT64: ସ୍ଥିତି FFI_TYPE_UINT64:
        ସ୍ଥିତି FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          ଭାଙ୍ଗ;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
        }
      ଭାଙ୍ଗ;
#endif

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ଭାଙ୍ଗ;
    }

  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା align;

      ବାଛ ((*ptr)->type)
	{
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
          ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ଗଣନା++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ଗଣନା = (intarg_ଗଣନା + 3) & ~0x1;
              ଯଦି (vecarg_ଗଣନା > ଅଙ୍କ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ଭାଙ୍ଗ;
            }
	  ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ଗଣନା++;
	      intarg_ଗଣନା++;
	    }
	  /* Fall through.  */
#endif
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  fparg_ଗଣନା++;
	  intarg_ଗଣନା++;
	  ଯଦି (fparg_ଗଣନା > ଅଙ୍କ_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  ଯଦି ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ଯଦି (align > 16)
		align = 16;
	      align = align / 8;
	      ଯଦି (align > 1)
		intarg_ଗଣନା = FFI_ALIGN (intarg_ଗଣନା, align);
	    }
	  intarg_ଗଣନା += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          ଯଦି (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ଗଣନା += elnum;
              ଯଦି (vecarg_ଗଣନା > ଅଙ୍କ_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              ଭାଙ୍ଗ;
            }
	  ଅନ୍ୟଥା
#endif
	  ଯଦି (elt)
	    {
	      fparg_ଗଣନା += elnum;
	      ଯଦି (fparg_ଗଣନା > ଅଙ୍କ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ଅନ୍ୟଥା
	    {
	      ଯଦି (intarg_ଗଣନା > ଅଙ୍କ_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_POINTER:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ଗଣନା++;
	  ଯଦି (intarg_ଗଣନା > ଅଙ୍କ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ଭାଙ୍ଗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ଯଦି (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  ବାଛ (elt)
	    {
	    ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ସ୍ଥିତି FFI_TYPE_DOUBLE:
	      fparg_ଗଣନା += 2;
	      intarg_ଗଣନା += 2;
	      ଯଦି (fparg_ଗଣନା > ଅଙ୍କ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ଭାଙ୍ଗ;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	      ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ଗଣନା += 2;
		  intarg_ଗଣନା = (intarg_ଗଣନା + 1) & ~0x1;
		  intarg_ଗଣନା += 4;
		  ଯଦି (vecarg_ଗଣନା > ଅଙ୍କ_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  ଭାଙ୍ଗ;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ଗଣନା += 4;
	      intarg_ଗଣନା += 4;
	      ଯଦି (fparg_ଗଣନା > ଅଙ୍କ_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      ଭାଙ୍ଗ;
#endif
	    ସ୍ଥିତି FFI_TYPE_INT:
	    ସ୍ଥିତି FFI_TYPE_SINT8:  ସ୍ଥିତି FFI_TYPE_UINT8:
	    ସ୍ଥିତି FFI_TYPE_SINT16: ସ୍ଥିତି FFI_TYPE_UINT16:
	    ସ୍ଥିତି FFI_TYPE_SINT32: ସ୍ଥିତି FFI_TYPE_UINT32:
	    ସ୍ଥିତି FFI_TYPE_SINT64: ସ୍ଥିତି FFI_TYPE_UINT64:
	    ସ୍ଥିତି FFI_TYPE_POINTER:
	      intarg_ଗଣନା += 2;
	      ଭାଙ୍ଗ;
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      ଫେରନ୍ତୁ FFI_BAD_TYPEDEF;
	    }
	  ଯଦି (intarg_ଗଣନା > ଅଙ୍କ_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  ଭାଙ୍ଗ;
#endif

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT (0);
	}
    }

  ଯଦି (fparg_ଗଣନା != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ଯଦି (intarg_ଗଣନା > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ଯଦି (vecarg_ଗଣନା != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  ଯଦି (fparg_ଗଣନା != 0)
    bytes += ଅଙ୍କ_FPR_ARG_REGISTERS64 * ଆକାର (ଦ୍ୱିଗୁଣ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  ଯଦି (vecarg_ଗଣନା != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += ଅଙ୍କ_VEC_ARG_REGISTERS64 * ଆକାର (float128);
  }

  /* Stack space.  */
#ଯଦି _CALL_ELF == 2
  ଯଦି ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_ଗଣନା * ଆକାର (long);
#ଅନ୍ୟଥା
  ଯଦି (intarg_ଗଣନା > ଅଙ୍କ_GPR_ARG_REGISTERS64)
    bytes += (intarg_ଗଣନା - ଅଙ୍କ_GPR_ARG_REGISTERS64) * ଆକାର (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  ଯଦି ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#ଯଦି _CALL_ELF != 2
  ଅନ୍ୟଥା ଯଦି (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ FFI_BAD_ABI;
  ଫେରନ୍ତୁ ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
			  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs MAYBE_UNUSED)
{
  ଯଦି ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#ଯଦି _CALL_ELF != 2
  ଅନ୍ୟଥା ଯଦି (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#ଯଦି _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  ଫେରନ୍ତୁ ffi_prep_cif_linux64_core (cif);
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

ଶୂନ୍ୟ FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *ଅଚଳ ଗଦା)
{
  ଅଚଳ unsigned long bytes = ecif->cif->bytes;
  ଅଚଳ unsigned long flags = ecif->cif->flags;

  ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ
  {
    ଅକ୍ଷର *c;
    unsigned long *ul;
    ଦଶମିକ *f;
    ଦ୍ୱିଗୁଣ *d;
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
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା fparg_ଗଣନା;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା vecarg_ଗଣନା;

  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ଶବ୍ଦଗୁଡ଼ିକ, nargs, nfixedargs;
  ffi_type **ptr;
  ଦ୍ୱିଗୁଣ double_tmp;
  ସଂଘ
  {
    ଶୂନ୍ୟ **v;
    ଅକ୍ଷର **c;
    signed ଅକ୍ଷର **sc;
    unsigned ଅକ୍ଷର **uc;
    signed short **ss;
    unsigned short **us;
    signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା **si;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା **ui;
    unsigned long **ul;
    ଦଶମିକ **f;
    ଦ୍ୱିଗୁଣ **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (ଅକ୍ଷର *) ଗଦା + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - ଅଙ୍କ_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + ଅଙ୍କ_GPR_ARG_REGISTERS64;
#ଯଦି _CALL_ELF == 2
  rest.ul = ଗଦା + 4 + ଅଙ୍କ_GPR_ARG_REGISTERS64;
#ଅନ୍ୟଥା
  rest.ul = ଗଦା + 6 + ଅଙ୍କ_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - ଅଙ୍କ_FPR_ARG_REGISTERS64;
  fparg_ଗଣନା = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  ଯଦି (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  ଅନ୍ୟଥା
    vec_base.p = gpr_base.p;
  vec_base.f128 -= ଅଙ୍କ_VEC_ARG_REGISTERS64;
  vecarg_ଗଣନା = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (ଅକ୍ଷର *) ଗଦା & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  ଯଦି (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (ଅକ୍ଷର *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#ଯଦି _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  ଯଦି ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  ପାଇଁ (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା elt, elnum;

      ବାଛ ((*ptr)->type)
	{
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
          ଯଦି ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              ଯଦି (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              ଯଦି (vecarg_ଗଣନା < ଅଙ୍କ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, ଆକାର (float128));
              ଅନ୍ୟଥା
		memcpy (next_arg.f128, *p_argv.f128, ଆକାର (float128));
              ଯଦି (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_ଗଣନା++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              ଭାଙ୍ଗ;
            }
	  ଯଦି ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# ଯଦି _CALL_ELF != 2
		  ଯଦି ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      ଅନ୍ୟଥା
		*next_arg.d = double_tmp;
	      ଯଦି (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ଗଣନା++;
	      double_tmp = (*p_argv.d)[1];
	      ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# ଯଦି _CALL_ELF != 2
		  ଯଦି ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      ଅନ୍ୟଥା
		*next_arg.d = double_tmp;
	      ଯଦି (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ଗଣନା++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ଭାଙ୍ଗ;
	    }
	  /* Fall through.  */
#endif
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
#ଯଦି _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#ଯଦି _CALL_ELF != 2
	      ଯଦି ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  ଅନ୍ୟଥା
	    *next_arg.d = double_tmp;
	  ଯଦି (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ଗଣନା++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
#ଯଦି _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#ଯଦି _CALL_ELF != 2
	      ଯଦି ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (ଦଶମିକ) double_tmp;
# ଅନ୍ୟଥା
		  next_arg.f[0] = (ଦଶମିକ) double_tmp;
# endif
		}
#endif
	    }
	  ଅନ୍ୟଥା
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (ଦଶମିକ) double_tmp;
# ଅନ୍ୟଥା
	      next_arg.f[0] = (ଦଶମିକ) double_tmp;
# endif
	    }
	  ଯଦି (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ଗଣନା++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  ଯଦି (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  ଯଦି (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	      ପାଇଁ (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  ଯଦି (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  ଯଦି (vecarg_ଗଣନା < ଅଙ୍କ_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, ଆକାର (float128));
		  ଅନ୍ୟଥା
		    memcpy (next_arg.f128, cval + j, ଆକାର (float128));
		  ଯଦି (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_ଗଣନା++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      ଭାଙ୍ଗ;
	    }
	  ଯଦି (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      ଦ୍ୱିଗୁଣ *cval = (ଦ୍ୱିଗୁଣ *) *p_argv.v;
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	      ପାଇଁ (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ଅନ୍ୟଥା
		    *next_arg.d = double_tmp;
		  ଯଦି (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ଗଣନା++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      ଭାଙ୍ଗ;
	    }
#endif
	  ଯଦି (elt == FFI_TYPE_FLOAT)
	    {
	      ଦଶମିକ *cval = (ଦଶମିକ *) *p_argv.v;
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	      ପାଇଁ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ଅନ୍ୟଥା
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (ଦଶମିକ) double_tmp;
# ଅନ୍ୟଥା
		      next_arg.f[0] = (ଦଶମିକ) double_tmp;
# endif
		    }
		  ଯଦି (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ଗଣନା++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ଅନ୍ୟଥା ଯଦି (elt == FFI_TYPE_DOUBLE)
	    {
	      ଦ୍ୱିଗୁଣ *cval = (ଦ୍ୱିଗୁଣ *) *p_argv.v;
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	      ପାଇଁ (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  ଅନ୍ୟଥା
		    *next_arg.d = double_tmp;
		  ଯଦି (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ଗଣନା++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  ଅନ୍ୟଥା
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      ଅକ୍ଷର *cval = (ଅକ୍ଷର *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	      ପାଇଁ (j = 0; j < 2; j++)
		{
		  ଅକ୍ଷର *half = cval + j * hsize;
		  unsigned long gprvalue;
		  ବାଛ (elt)
		    {
		    ସ୍ଥିତି FFI_TYPE_UINT8:
		      gprvalue = *(unsigned ଅକ୍ଷର *) half; ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed ଅକ୍ଷର *) half;
		      ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_UINT32:
		      gprvalue = *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) half; ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_INT:
		    ସ୍ଥିତି FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) half;
		      ଭାଙ୍ଗ;
		    ସ୍ଥିତି FFI_TYPE_SINT64:
		    ସ୍ଥିତି FFI_TYPE_UINT64:
		    ସ୍ଥିତି FFI_TYPE_POINTER:
		    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		      gprvalue = *(unsigned long *) half; ଭାଙ୍ଗ;
		    }
		  *next_arg.ul++ = gprvalue;
		  ଯଦି (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  ଯଦି ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      ଯଦି (align > 16)
		align = 16;
	      ଯଦି (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  ଯଦି (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  ଯଦି (elt)
	    {
#ଯଦି _CALL_ELF == 2
	      ସଂଘ {
		ଶୂନ୍ୟ *v;
		ଦଶମିକ *f;
		ଦ୍ୱିଗୁଣ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ଯଦି (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  କର
                    {
                      ଯଦି (vecarg_ଗଣନା < ଅଙ୍କ_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, ଆକାର (float128));
                      ଅନ୍ୟଥା
		        memcpy (next_arg.f128, arg.f128++, ଆକାର (float128));
                      ଯଦି (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_ଗଣନା++;
                    }
                  ଯେତେବେଳେ (--elnum != 0);
                }
              ଅନ୍ୟଥା
#endif
	      ଯଦି (elt == FFI_TYPE_FLOAT)
		{
		  କର
		    {
		      double_tmp = *arg.f++;
		      ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      ଅନ୍ୟଥା
			*next_arg.f = (ଦଶମିକ) double_tmp;
		      ଯଦି (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_ଗଣନା++;
		    }
		  ଯେତେବେଳେ (--elnum != 0);
		  ଯଦି ((next_arg.p & 7) != 0)
                    ଯଦି (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      ଅନ୍ୟଥା
		କର
		  {
		    double_tmp = *arg.d++;
		    ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    ଅନ୍ୟଥା
		      *next_arg.d = double_tmp;
		    ଯଦି (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_ଗଣନା++;
		  }
		ଯେତେବେଳେ (--elnum != 0);
#ଅନ୍ୟଥା
	      ଯଦି (elt == FFI_TYPE_FLOAT)
		ଯାଅ do_float;
	      ଅନ୍ୟଥା
		ଯାଅ do_double;
#endif
	    }
	  ଅନ୍ୟଥା
	    {
	      ଶବ୍ଦଗୁଡ଼ିକ = ((*ptr)->size + 7) / 8;
	      ଯଦି (next_arg.ul >= gpr_base.ul && next_arg.ul + ଶବ୍ଦଗୁଡ଼ିକ > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + ଶବ୍ଦଗୁଡ଼ିକ * 8 - first;
		}
	      ଅନ୍ୟଥା
		{
		  ଅକ୍ଷର *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  ଯଦି ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += ଶବ୍ଦଗୁଡ଼ିକ;
		  ଯଦି (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  ଯାଅ putgpr;

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  ଯଦି (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  ଭାଙ୍ଗ;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#ଯଦି _CALL_ELF == 2
#define MIN_CACHE_ଧାଡ଼ି_SIZE 8

ସ୍ଥିର ଶୂନ୍ୟ
flush_icache (ଅକ୍ଷର *wraddr, ଅକ୍ଷର *xaddr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < size; i += MIN_CACHE_ଧାଡ଼ି_SIZE)
    __asm__ ଅସ୍ଥିର ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ଅସ୍ଥିର ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
			      ଶୂନ୍ୟ *user_data,
			      ଶୂନ୍ୟ *codeloc)
{
  ଯଦି (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ଅନ୍ୟଥା
#endif
    {
#ଯଦି _CALL_ELF == 2
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(ଶୂନ୍ୟ **) &tramp[4] = (ଶୂନ୍ୟ *) ffi_closure_LINUX64;
      *(ଶୂନ୍ୟ **) &tramp[6] = codeloc;
      flush_icache ((ଅକ୍ଷର *) tramp, (ଅକ୍ଷର *) codeloc, 4 * 4);
#ଅନ୍ୟଥା
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      ଶୂନ୍ୟ **tramp = (ଶୂନ୍ୟ **) &closure->tramp[0];
      memcpy (&tramp[0], (ଶୂନ୍ୟ **) ffi_closure_LINUX64, ଆକାର (ଶୂନ୍ୟ *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (ଶୂନ୍ୟ **) ffi_closure_LINUX64 + 1, ଆକାର (ଶୂନ୍ୟ *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}


ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
			    ଶୂନ୍ୟ *user_data,
			    ଶୂନ୍ୟ *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  ଶୂନ୍ୟ **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + ଅଙ୍କ_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + ଅଙ୍କ_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (ଶୂନ୍ୟ *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#ଯଦି _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  ଯଦି ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ଯେତେବେଳେ (i < avn)
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା elt, elnum;

      ବାଛ (arg_types[i]->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ଅକ୍ଷର *) pst + 7;
	  pst++;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ଅକ୍ଷର *) pst + 6;
	  pst++;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (ଅକ୍ଷର *) pst + 4;
	  pst++;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  ଯଦି ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      ଯଦି (align > 16)
		align = 16;
	      ଯଦି (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  ଯଦି (elt)
	    {
#ଯଦି _CALL_ELF == 2
	      ସଂଘ {
		ଶୂନ୍ୟ *v;
		unsigned long *ul;
		ଦଶମିକ *f;
		ଦ୍ୱିଗୁଣ *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ଯଦି (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  ଯଦି (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  ଅନ୍ୟଥା
                    to.v = pst;
                }
              ଅନ୍ୟଥା
#endif
	      ଯଦି (pfr + elnum <= end_pfr)
		to.v = pfr;
	      ଅନ୍ୟଥା
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              ଯଦି (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  କର
                    {
                      ଯଦି (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, ଆକାର (float128));
                      ଅନ୍ୟଥା
		        memcpy (to.f128, from.f128, ଆକାର (float128));
                      to.f128++;
                      from.f128++;
                    }
                  ଯେତେବେଳେ (--elnum != 0);
                }
              ଅନ୍ୟଥା
#endif
	      ଯଦି (elt == FFI_TYPE_FLOAT)
		{
		  କର
		    {
		      ଯଦି (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (ଦଶମିକ) pfr->d;
			  pfr++;
			}
		      ଅନ୍ୟଥା
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  ଯେତେବେଳେ (--elnum != 0);
		}
	      ଅନ୍ୟଥା
		{
		  କର
		    {
		      ଯଦି (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      ଅନ୍ୟଥା
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  ଯେତେବେଳେ (--elnum != 0);
		}
#ଅନ୍ୟଥା
	      ଯଦି (elt == FFI_TYPE_FLOAT)
		ଯାଅ do_float;
	      ଅନ୍ୟଥା
		ଯାଅ do_double;
#endif
	    }
	  ଅନ୍ୟଥା
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      ଯଦି (arg_types[i]->size < 8)
		avalue[i] = (ଅକ୍ଷର *) pst + 8 - arg_types[i]->size;
	      ଅନ୍ୟଥା
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  ଭାଙ୍ଗ;

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
          ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              ଯଦି (((unsigned long) pst & 0xF) != 0)
                ++pst;
              ଯଦି (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              ଅନ୍ୟଥା
                avalue[i] = pst;
              pst += 2;
              ଭାଙ୍ଗ;
            }
          ଅନ୍ୟଥା ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      ଯଦି (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      ଅନ୍ୟଥା
		{
		  ଯଦି (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      ଭାଙ୍ଗ;
	    }
	  /* Fall through.  */
#endif
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
#ଯଦି _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  ଯଦି (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
	    avalue[i] = pst;
	  pst++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
#ଯଦି _CALL_ELF != 2
	do_float:
#endif
	  ଯଦି (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (ଦଶମିକ) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (ଅକ୍ଷର *) pst + 4;
#ଅନ୍ୟଥା
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  ଭାଙ୍ଗ;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
	    elt = arg_types[i]->elements[0]->type;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    ଯଦି (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    ଯଦି (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * ଆକାର (float128));
		ଯଦି (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		ପାଇଁ (j = 0; j < 2; j++)
		  {
		    ଯଦି (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, ଆକାର (float128));
		    ଅନ୍ୟଥା
		      memcpy (&cval[j], pst, ଆକାର (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		ଭାଙ୍ଗ;
	      }
	    ଯଦି (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		ଦ୍ୱିଗୁଣ *cval = alloca (4 * ଆକାର (ଦ୍ୱିଗୁଣ));
		ପାଇଁ (j = 0; j < 4; j++)
		  {
		    ଯଦି (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ଅନ୍ୟଥା
		      cval[j] = *(ଦ୍ୱିଗୁଣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		ଭାଙ୍ଗ;
	      }
#endif
	    ଯଦି (elt == FFI_TYPE_FLOAT)
	      {
		ଦଶମିକ *cval = alloca (2 * ଆକାର (ଦଶମିକ));
		ପାଇଁ (j = 0; j < 2; j++)
		  {
		    ଯଦି (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (ଦଶମିକ) pfr->d;
			pfr++;
		      }
		    ଅନ୍ୟଥା
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((ଦଶମିକ *) pst)[1];
#ଅନ୍ୟଥା
			cval[j] = ((ଦଶମିକ *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ଅନ୍ୟଥା ଯଦି (elt == FFI_TYPE_DOUBLE)
	      {
		ଦ୍ୱିଗୁଣ *cval = alloca (2 * ଆକାର (ଦ୍ୱିଗୁଣ));
		ପାଇଁ (j = 0; j < 2; j++)
		  {
		    ଯଦି (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    ଅନ୍ୟଥା
		      cval[j] = *(ଦ୍ୱିଗୁଣ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    ଅନ୍ୟଥା
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		ଅକ୍ଷର *cval = alloca (2 * hsize);
		ପାଇଁ (j = 0; j < 2; j++)
		  {
		    ଅକ୍ଷର *src;
#ifndef __LITTLE_ENDIAN__
		    src = (ଅକ୍ଷର *) pst + (8 - hsize);
#ଅନ୍ୟଥା
		    src = (ଅକ୍ଷର *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  ଭାଙ୍ଗ;
#endif

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      ଫେରନ୍ତୁ PPC_LD_NONE;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ଫେରନ୍ତୁ PPC_LD_F32;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ଫେରନ୍ତୁ PPC_LD_F64;
#ଯଦି FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      ଯଦି ((cif->flags & FLAG_RETURNS_VEC) != 0)
	ଫେରନ୍ତୁ PPC64_LD_VECTOR;
      ଫେରନ୍ତୁ PPC_LD_F128;
#endif
    ସ୍ଥିତି FFI_TYPE_UINT8:
      ଫେରନ୍ତୁ PPC_LD_U8;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      ଫେରନ୍ତୁ PPC_LD_S8;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      ଫେରନ୍ତୁ PPC_LD_U16;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      ଫେରନ୍ତୁ PPC_LD_S16;
    ସ୍ଥିତି FFI_TYPE_UINT32:
      ଫେରନ୍ତୁ PPC_LD_U32;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
      ଫେରନ୍ତୁ PPC_LD_S32;
    ସ୍ଥିତି FFI_TYPE_POINTER:
      ଫେରନ୍ତୁ PPC_LD_PTR;
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      ଫେରନ୍ତୁ PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା inner = cif->rtype->elements[0]->type;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ଯଦି (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    ଯଦି ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      ଫେରନ୍ତୁ PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	ଯଦି (inner == FFI_TYPE_FLOAT)
	  ଫେରନ୍ତୁ PPC64_LD_FLOAT_HOMOG;
	ଯଦି (inner == FFI_TYPE_DOUBLE)
	  ଫେରନ୍ତୁ PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  ଅକ୍ଷର *rv = rvalue;
	  unsigned long re, im;
	  ବାଛ (inner)
	    {
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      re = ((unsigned ଅକ୍ଷର *) rv)[0];
	      im = ((unsigned ଅକ୍ଷର *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed ଅକ୍ଷର *) rv)[0];
	      im = (unsigned long) (long) ((signed ଅକ୍ଷର *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_UINT32:
	      re = ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) rv)[0];
	      im = ((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_INT:
	    ସ୍ଥିତି FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) rv)[0];
	      im = (unsigned long) (long) ((signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) rv)[1];
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି FFI_TYPE_SINT64:
	    ସ୍ଥିତି FFI_TYPE_UINT64:
	    ସ୍ଥିତି FFI_TYPE_POINTER:
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      ଭାଙ୍ଗ;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  ଫେରନ୍ତୁ PPC_LD_R3R4;
	}
      }
#endif
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  ଯଦି ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      ବାଛ (cif->rtype->size)
		{
		ସ୍ଥିତି 0:
		  ଫେରନ୍ତୁ PPC_LD_NONE;
		ସ୍ଥିତି 1:
		  ଫେରନ୍ତୁ PPC_LD_U8;
		ସ୍ଥିତି 2:
		  ଫେରନ୍ତୁ PPC_LD_U16;
		ସ୍ଥିତି 3:
		  ଫେରନ୍ତୁ PPC64_LD_STRUCT_3;
		ସ୍ଥିତି 4:
		  ଫେରନ୍ତୁ PPC_LD_U32;
		ସ୍ଥିତି 5:
		  ଫେରନ୍ତୁ PPC64_LD_STRUCT_5;
		ସ୍ଥିତି 6:
		  ଫେରନ୍ତୁ PPC64_LD_STRUCT_6;
		ସ୍ଥିତି 7:
		  ଫେରନ୍ତୁ PPC64_LD_STRUCT_7;
		ସ୍ଥିତି 8 ... 16:
		  ଫେରନ୍ତୁ PPC_LD_R3R4;
		}
	      ଭାଙ୍ଗ;
	    }
	  ଯଦି ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    ଫେରନ୍ତୁ PPC64_LD_VECTOR_HOMOG;
	  ଯଦି ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    ଫେରନ୍ତୁ PPC64_LD_DOUBLE_HOMOG;
	  ଫେରନ୍ତୁ PPC64_LD_FLOAT_HOMOG;
        }
      ଫେରନ୍ତୁ PPC_LD_NONE;
    }
  abort();
}
#endif

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

#அந்தர்பாவ "ffi.h"
#அந்தர்பாவ <tramp.h>
#அந்தர்பாவ <stdlib.h>

#ifdef POWERPC64
#அந்தர்பாவ "ffi_common.h"
#அந்தர்பாவ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
எண்ணிக்கை {
  இலக்கம்_GPR_ARG_REGISTERS64 = 8,
  இலக்கம்_FPR_ARG_REGISTERS64 = 13,
  இலக்கம்_VEC_ARG_REGISTERS64 = 12,
};
எண்ணிக்கை { ASM_NEEDS_REGISTERS64 = 4 };


#எனில் HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
வெறுமை FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  எனில் ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  இல்லையெனில்
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


நிலையான unsigned முழு
discover_homogeneous_aggregate (ffi_abi abi,
                                மாறிலி ffi_type *t,
                                unsigned முழு *elnum)
{
  தேர்வு (t->type)
    {
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      எனில் ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          திரும்பு FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      இல்லையெனில் எனில் ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          திரும்பு FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    நிலை FFI_TYPE_FLOAT:
    நிலை FFI_TYPE_DOUBLE:
      *elnum = 1;
      திரும்பு (முழு) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    நிலை FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned முழு inner_elnum = 0;
	unsigned முழு inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	எனில் (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    திரும்பு inner;
	  }
	திரும்பு 0;
      }
#endif

    நிலை FFI_TYPE_STRUCT:;
      {
	unsigned முழு base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	போது (*el)
	  {
	    unsigned முழு el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    எனில் (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      திரும்பு 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#எனில் _CALL_ELF == 2
	    எனில் (total_elnum > 8)
	      திரும்பு 0;
#இல்லையெனில்
	    எனில் (total_elnum > 1)
	      திரும்பு 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	திரும்பு base_elt;
      }

    இயல்பு:
      திரும்பு 0;
    }
}


/* Perform machine dependent cif processing */
நிலையான ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_மொத்தம் = 0, intarg_மொத்தம் = 0, vecarg_மொத்தம் = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#எனில் FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    திரும்பு FFI_BAD_ABI;
#அல்லதெனில் !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    திரும்பு FFI_BAD_ABI;
#இல்லையெனில்
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    திரும்பு FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#எனில் _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * அளவு (long);

  /* Space for the general registers.  */
  bytes += இலக்கம்_GPR_ARG_REGISTERS64 * அளவு (long);
#இல்லையெனில்
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * அளவு (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * இலக்கம்_GPR_ARG_REGISTERS64 * அளவு (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#எனில் _CALL_ELF == 2
homogeneous:
#endif
  தேர்வு (rtype)
    {
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
      எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          நிறுத்து;
        }
      எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    நிலை FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    நிலை FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      நிறுத்து;

    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
#எனில் _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      எனில் (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          செல் homogeneous;
        }
      எனில் (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          நிறுத்து;
        }
#endif
      intarg_மொத்தம்++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    நிலை FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      நிறுத்து;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    நிலை FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      தேர்வு (rtype)
        {
        நிலை FFI_TYPE_FLOAT:
        நிலை FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          செல் homogeneous;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        நிலை FFI_TYPE_LONGDOUBLE:
          எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              நிறுத்து;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          செல் homogeneous;
#endif
        நிலை FFI_TYPE_INT:
        நிலை FFI_TYPE_SINT8:  நிலை FFI_TYPE_UINT8:
        நிலை FFI_TYPE_SINT16: நிலை FFI_TYPE_UINT16:
        நிலை FFI_TYPE_SINT32: நிலை FFI_TYPE_UINT32:
        நிலை FFI_TYPE_SINT64: நிலை FFI_TYPE_UINT64:
        நிலை FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          நிறுத்து;
        இயல்பு:
          திரும்பு FFI_BAD_TYPEDEF;
        }
      நிறுத்து;
#endif

    இயல்பு:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      நிறுத்து;
    }

  க்கு (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned முழு align;

      தேர்வு ((*ptr)->type)
	{
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
          எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_மொத்தம்++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_மொத்தம் = (intarg_மொத்தம் + 3) & ~0x1;
              எனில் (vecarg_மொத்தம் > இலக்கம்_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              நிறுத்து;
            }
	  எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_மொத்தம்++;
	      intarg_மொத்தம்++;
	    }
	  /* Fall through.  */
#endif
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_FLOAT:
	  fparg_மொத்தம்++;
	  intarg_மொத்தம்++;
	  எனில் (fparg_மொத்தம் > இலக்கம்_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  எனில் ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      எனில் (align > 16)
		align = 16;
	      align = align / 8;
	      எனில் (align > 1)
		intarg_மொத்தம் = FFI_ALIGN (intarg_மொத்தம், align);
	    }
	  intarg_மொத்தம் += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          எனில் (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_மொத்தம் += elnum;
              எனில் (vecarg_மொத்தம் > இலக்கம்_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              நிறுத்து;
            }
	  இல்லையெனில்
#endif
	  எனில் (elt)
	    {
	      fparg_மொத்தம் += elnum;
	      எனில் (fparg_மொத்தம் > இலக்கம்_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  இல்லையெனில்
	    {
	      எனில் (intarg_மொத்தம் > இலக்கம்_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  நிறுத்து;

	நிலை FFI_TYPE_POINTER:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_மொத்தம்++;
	  எனில் (intarg_மொத்தம் > இலக்கம்_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  நிறுத்து;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	நிலை FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  எனில் (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  தேர்வு (elt)
	    {
	    நிலை FFI_TYPE_FLOAT:
	    நிலை FFI_TYPE_DOUBLE:
	      fparg_மொத்தம் += 2;
	      intarg_மொத்தம் += 2;
	      எனில் (fparg_மொத்தம் > இலக்கம்_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      நிறுத்து;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    நிலை FFI_TYPE_LONGDOUBLE:
	      எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_மொத்தம் += 2;
		  intarg_மொத்தம் = (intarg_மொத்தம் + 1) & ~0x1;
		  intarg_மொத்தம் += 4;
		  எனில் (vecarg_மொத்தம் > இலக்கம்_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  நிறுத்து;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_மொத்தம் += 4;
	      intarg_மொத்தம் += 4;
	      எனில் (fparg_மொத்தம் > இலக்கம்_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      நிறுத்து;
#endif
	    நிலை FFI_TYPE_INT:
	    நிலை FFI_TYPE_SINT8:  நிலை FFI_TYPE_UINT8:
	    நிலை FFI_TYPE_SINT16: நிலை FFI_TYPE_UINT16:
	    நிலை FFI_TYPE_SINT32: நிலை FFI_TYPE_UINT32:
	    நிலை FFI_TYPE_SINT64: நிலை FFI_TYPE_UINT64:
	    நிலை FFI_TYPE_POINTER:
	      intarg_மொத்தம் += 2;
	      நிறுத்து;
	    இயல்பு:
	      திரும்பு FFI_BAD_TYPEDEF;
	    }
	  எனில் (intarg_மொத்தம் > இலக்கம்_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  நிறுத்து;
#endif

	இயல்பு:
	  FFI_ASSERT (0);
	}
    }

  எனில் (fparg_மொத்தம் != 0)
    flags |= FLAG_FP_ARGUMENTS;
  எனில் (intarg_மொத்தம் > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  எனில் (vecarg_மொத்தம் != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  எனில் (fparg_மொத்தம் != 0)
    bytes += இலக்கம்_FPR_ARG_REGISTERS64 * அளவு (இரட்டை);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  எனில் (vecarg_மொத்தம் != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += இலக்கம்_VEC_ARG_REGISTERS64 * அளவு (float128);
  }

  /* Stack space.  */
#எனில் _CALL_ELF == 2
  எனில் ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_மொத்தம் * அளவு (long);
#இல்லையெனில்
  எனில் (intarg_மொத்தம் > இலக்கம்_GPR_ARG_REGISTERS64)
    bytes += (intarg_மொத்தம் - இலக்கம்_GPR_ARG_REGISTERS64) * அளவு (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  திரும்பு FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  எனில் ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#எனில் _CALL_ELF != 2
  இல்லையெனில் எனில் (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  இல்லையெனில்
    திரும்பு FFI_BAD_ABI;
  திரும்பு ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned முழு nfixedargs,
			  unsigned முழு ntotalargs MAYBE_UNUSED)
{
  எனில் ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#எனில் _CALL_ELF != 2
  இல்லையெனில் எனில் (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  இல்லையெனில்
    திரும்பு FFI_BAD_ABI;
#எனில் _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  திரும்பு ffi_prep_cif_linux64_core (cif);
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

வெறுமை FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *மாறிலி அடுக்கு)
{
  மாறிலி unsigned long bytes = ecif->cif->bytes;
  மாறிலி unsigned long flags = ecif->cif->flags;

  வகைவரை ஒன்றிப்பு
  {
    எழுத்து *c;
    unsigned long *ul;
    தசம *f;
    இரட்டை *d;
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
  unsigned முழு fparg_மொத்தம்;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned முழு vecarg_மொத்தம்;

  unsigned முழு i, சொற்கள், nargs, nfixedargs;
  ffi_type **ptr;
  இரட்டை double_tmp;
  ஒன்றிப்பு
  {
    வெறுமை **v;
    எழுத்து **c;
    signed எழுத்து **sc;
    unsigned எழுத்து **uc;
    signed short **ss;
    unsigned short **us;
    signed முழு **si;
    unsigned முழு **ui;
    unsigned long **ul;
    தசம **f;
    இரட்டை **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (எழுத்து *) அடுக்கு + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - இலக்கம்_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + இலக்கம்_GPR_ARG_REGISTERS64;
#எனில் _CALL_ELF == 2
  rest.ul = அடுக்கு + 4 + இலக்கம்_GPR_ARG_REGISTERS64;
#இல்லையெனில்
  rest.ul = அடுக்கு + 6 + இலக்கம்_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - இலக்கம்_FPR_ARG_REGISTERS64;
  fparg_மொத்தம் = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  எனில் (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  இல்லையெனில்
    vec_base.p = gpr_base.p;
  vec_base.f128 -= இலக்கம்_VEC_ARG_REGISTERS64;
  vecarg_மொத்தம் = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (எழுத்து *) அடுக்கு & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  எனில் (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (எழுத்து *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#எனில் _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  எனில் ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  க்கு (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned முழு elt, elnum;

      தேர்வு ((*ptr)->type)
	{
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
          எனில் ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              எனில் (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              எனில் (vecarg_மொத்தம் < இலக்கம்_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, அளவு (float128));
              இல்லையெனில்
		memcpy (next_arg.f128, *p_argv.f128, அளவு (float128));
              எனில் (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_மொத்தம்++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              நிறுத்து;
            }
	  எனில் ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# எனில் _CALL_ELF != 2
		  எனில் ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      இல்லையெனில்
		*next_arg.d = double_tmp;
	      எனில் (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_மொத்தம்++;
	      double_tmp = (*p_argv.d)[1];
	      எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# எனில் _CALL_ELF != 2
		  எனில் ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      இல்லையெனில்
		*next_arg.d = double_tmp;
	      எனில் (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_மொத்தம்++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      நிறுத்து;
	    }
	  /* Fall through.  */
#endif
	நிலை FFI_TYPE_DOUBLE:
#எனில் _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#எனில் _CALL_ELF != 2
	      எனில் ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  இல்லையெனில்
	    *next_arg.d = double_tmp;
	  எனில் (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_மொத்தம்++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
#எனில் _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#எனில் _CALL_ELF != 2
	      எனில் ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (தசம) double_tmp;
# இல்லையெனில்
		  next_arg.f[0] = (தசம) double_tmp;
# endif
		}
#endif
	    }
	  இல்லையெனில்
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (தசம) double_tmp;
# இல்லையெனில்
	      next_arg.f[0] = (தசம) double_tmp;
# endif
	    }
	  எனில் (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_மொத்தம்++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  நிறுத்து;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	நிலை FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  எனில் (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  எனில் (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned முழு j;
	      க்கு (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  எனில் (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  எனில் (vecarg_மொத்தம் < இலக்கம்_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, அளவு (float128));
		  இல்லையெனில்
		    memcpy (next_arg.f128, cval + j, அளவு (float128));
		  எனில் (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_மொத்தம்++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      நிறுத்து;
	    }
	  எனில் (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      இரட்டை *cval = (இரட்டை *) *p_argv.v;
	      unsigned முழு j;
	      க்கு (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  இல்லையெனில்
		    *next_arg.d = double_tmp;
		  எனில் (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_மொத்தம்++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      நிறுத்து;
	    }
#endif
	  எனில் (elt == FFI_TYPE_FLOAT)
	    {
	      தசம *cval = (தசம *) *p_argv.v;
	      unsigned முழு j;
	      க்கு (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  இல்லையெனில்
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (தசம) double_tmp;
# இல்லையெனில்
		      next_arg.f[0] = (தசம) double_tmp;
# endif
		    }
		  எனில் (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_மொத்தம்++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  இல்லையெனில் எனில் (elt == FFI_TYPE_DOUBLE)
	    {
	      இரட்டை *cval = (இரட்டை *) *p_argv.v;
	      unsigned முழு j;
	      க்கு (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  இல்லையெனில்
		    *next_arg.d = double_tmp;
		  எனில் (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_மொத்தம்++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  இல்லையெனில்
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      எழுத்து *cval = (எழுத்து *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned முழு j;
	      க்கு (j = 0; j < 2; j++)
		{
		  எழுத்து *half = cval + j * hsize;
		  unsigned long gprvalue;
		  தேர்வு (elt)
		    {
		    நிலை FFI_TYPE_UINT8:
		      gprvalue = *(unsigned எழுத்து *) half; நிறுத்து;
		    நிலை FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed எழுத்து *) half;
		      நிறுத்து;
		    நிலை FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; நிறுத்து;
		    நிலை FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      நிறுத்து;
		    நிலை FFI_TYPE_UINT32:
		      gprvalue = *(unsigned முழு *) half; நிறுத்து;
		    நிலை FFI_TYPE_INT:
		    நிலை FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed முழு *) half;
		      நிறுத்து;
		    நிலை FFI_TYPE_SINT64:
		    நிலை FFI_TYPE_UINT64:
		    நிலை FFI_TYPE_POINTER:
		    இயல்பு:
		      gprvalue = *(unsigned long *) half; நிறுத்து;
		    }
		  *next_arg.ul++ = gprvalue;
		  எனில் (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_STRUCT:
	  எனில் ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      எனில் (align > 16)
		align = 16;
	      எனில் (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  எனில் (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  எனில் (elt)
	    {
#எனில் _CALL_ELF == 2
	      ஒன்றிப்பு {
		வெறுமை *v;
		தசம *f;
		இரட்டை *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              எனில் (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  செய்
                    {
                      எனில் (vecarg_மொத்தம் < இலக்கம்_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, அளவு (float128));
                      இல்லையெனில்
		        memcpy (next_arg.f128, arg.f128++, அளவு (float128));
                      எனில் (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_மொத்தம்++;
                    }
                  போது (--elnum != 0);
                }
              இல்லையெனில்
#endif
	      எனில் (elt == FFI_TYPE_FLOAT)
		{
		  செய்
		    {
		      double_tmp = *arg.f++;
		      எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      இல்லையெனில்
			*next_arg.f = (தசம) double_tmp;
		      எனில் (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_மொத்தம்++;
		    }
		  போது (--elnum != 0);
		  எனில் ((next_arg.p & 7) != 0)
                    எனில் (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      இல்லையெனில்
		செய்
		  {
		    double_tmp = *arg.d++;
		    எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    இல்லையெனில்
		      *next_arg.d = double_tmp;
		    எனில் (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_மொத்தம்++;
		  }
		போது (--elnum != 0);
#இல்லையெனில்
	      எனில் (elt == FFI_TYPE_FLOAT)
		செல் do_float;
	      இல்லையெனில்
		செல் do_double;
#endif
	    }
	  இல்லையெனில்
	    {
	      சொற்கள் = ((*ptr)->size + 7) / 8;
	      எனில் (next_arg.ul >= gpr_base.ul && next_arg.ul + சொற்கள் > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + சொற்கள் * 8 - first;
		}
	      இல்லையெனில்
		{
		  எழுத்து *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  எனில் ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += சொற்கள்;
		  எனில் (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  நிறுத்து;

	நிலை FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  செல் putgpr;
	நிலை FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  செல் putgpr;
	நிலை FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  செல் putgpr;
	நிலை FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  செல் putgpr;
	நிலை FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  செல் putgpr;
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  செல் putgpr;

	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  எனில் (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  நிறுத்து;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#எனில் _CALL_ELF == 2
#define MIN_CACHE_வரி_SIZE 8

நிலையான வெறுமை
flush_icache (எழுத்து *wraddr, எழுத்து *xaddr, முழு size)
{
  முழு i;
  க்கு (i = 0; i < size; i += MIN_CACHE_வரி_SIZE)
    __asm__ நிலையற்ற ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ நிலையற்ற ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
			      வெறுமை *user_data,
			      வெறுமை *codeloc)
{
  எனில் (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    திரும்பு FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      வெறுமை (*dest)(வெறுமை) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  இல்லையெனில்
#endif
    {
#எனில் _CALL_ELF == 2
      unsigned முழு *tramp = (unsigned முழு *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(வெறுமை **) &tramp[4] = (வெறுமை *) ffi_closure_LINUX64;
      *(வெறுமை **) &tramp[6] = codeloc;
      flush_icache ((எழுத்து *) tramp, (எழுத்து *) codeloc, 4 * 4);
#இல்லையெனில்
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      வெறுமை **tramp = (வெறுமை **) &closure->tramp[0];
      memcpy (&tramp[0], (வெறுமை **) ffi_closure_LINUX64, அளவு (வெறுமை *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (வெறுமை **) ffi_closure_LINUX64 + 1, அளவு (வெறுமை *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}


முழு FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
			    வெறுமை *user_data,
			    வெறுமை *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  வெறுமை **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + இலக்கம்_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + இலக்கம்_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * அளவு (வெறுமை *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  எனில் (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (வெறுமை *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#எனில் _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  எனில் ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  போது (i < avn)
    {
      unsigned முழு elt, elnum;

      தேர்வு (arg_types[i]->type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (எழுத்து *) pst + 7;
	  pst++;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (எழுத்து *) pst + 6;
	  pst++;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (எழுத்து *) pst + 4;
	  pst++;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  எனில் ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      எனில் (align > 16)
		align = 16;
	      எனில் (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  எனில் (elt)
	    {
#எனில் _CALL_ELF == 2
	      ஒன்றிப்பு {
		வெறுமை *v;
		unsigned long *ul;
		தசம *f;
		இரட்டை *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              எனில் (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  எனில் (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  இல்லையெனில்
                    to.v = pst;
                }
              இல்லையெனில்
#endif
	      எனில் (pfr + elnum <= end_pfr)
		to.v = pfr;
	      இல்லையெனில்
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              எனில் (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  செய்
                    {
                      எனில் (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, அளவு (float128));
                      இல்லையெனில்
		        memcpy (to.f128, from.f128, அளவு (float128));
                      to.f128++;
                      from.f128++;
                    }
                  போது (--elnum != 0);
                }
              இல்லையெனில்
#endif
	      எனில் (elt == FFI_TYPE_FLOAT)
		{
		  செய்
		    {
		      எனில் (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (தசம) pfr->d;
			  pfr++;
			}
		      இல்லையெனில்
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  போது (--elnum != 0);
		}
	      இல்லையெனில்
		{
		  செய்
		    {
		      எனில் (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      இல்லையெனில்
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  போது (--elnum != 0);
		}
#இல்லையெனில்
	      எனில் (elt == FFI_TYPE_FLOAT)
		செல் do_float;
	      இல்லையெனில்
		செல் do_double;
#endif
	    }
	  இல்லையெனில்
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      எனில் (arg_types[i]->size < 8)
		avalue[i] = (எழுத்து *) pst + 8 - arg_types[i]->size;
	      இல்லையெனில்
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  நிறுத்து;

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
          எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              எனில் (((unsigned long) pst & 0xF) != 0)
                ++pst;
              எனில் (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              இல்லையெனில்
                avalue[i] = pst;
              pst += 2;
              நிறுத்து;
            }
          இல்லையெனில் எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      எனில் (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      இல்லையெனில்
		{
		  எனில் (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      நிறுத்து;
	    }
	  /* Fall through.  */
#endif
	நிலை FFI_TYPE_DOUBLE:
#எனில் _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  எனில் (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  இல்லையெனில்
	    avalue[i] = pst;
	  pst++;
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
#எனில் _CALL_ELF != 2
	do_float:
#endif
	  எனில் (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (தசம) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  இல்லையெனில்
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (எழுத்து *) pst + 4;
#இல்லையெனில்
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  நிறுத்து;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	நிலை FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned முழு j;
	    elt = arg_types[i]->elements[0]->type;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    எனில் (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    எனில் (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * அளவு (float128));
		எனில் (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		க்கு (j = 0; j < 2; j++)
		  {
		    எனில் (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, அளவு (float128));
		    இல்லையெனில்
		      memcpy (&cval[j], pst, அளவு (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		நிறுத்து;
	      }
	    எனில் (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		இரட்டை *cval = alloca (4 * அளவு (இரட்டை));
		க்கு (j = 0; j < 4; j++)
		  {
		    எனில் (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    இல்லையெனில்
		      cval[j] = *(இரட்டை *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		நிறுத்து;
	      }
#endif
	    எனில் (elt == FFI_TYPE_FLOAT)
	      {
		தசம *cval = alloca (2 * அளவு (தசம));
		க்கு (j = 0; j < 2; j++)
		  {
		    எனில் (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (தசம) pfr->d;
			pfr++;
		      }
		    இல்லையெனில்
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((தசம *) pst)[1];
#இல்லையெனில்
			cval[j] = ((தசம *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    இல்லையெனில் எனில் (elt == FFI_TYPE_DOUBLE)
	      {
		இரட்டை *cval = alloca (2 * அளவு (இரட்டை));
		க்கு (j = 0; j < 2; j++)
		  {
		    எனில் (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    இல்லையெனில்
		      cval[j] = *(இரட்டை *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    இல்லையெனில்
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		எழுத்து *cval = alloca (2 * hsize);
		க்கு (j = 0; j < 2; j++)
		  {
		    எழுத்து *src;
#ifndef __LITTLE_ENDIAN__
		    src = (எழுத்து *) pst + (8 - hsize);
#இல்லையெனில்
		    src = (எழுத்து *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  நிறுத்து;
#endif

	இயல்பு:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      திரும்பு PPC_LD_NONE;
    நிலை FFI_TYPE_FLOAT:
      திரும்பு PPC_LD_F32;
    நிலை FFI_TYPE_DOUBLE:
      திரும்பு PPC_LD_F64;
#எனில் FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
      எனில் ((cif->flags & FLAG_RETURNS_VEC) != 0)
	திரும்பு PPC64_LD_VECTOR;
      திரும்பு PPC_LD_F128;
#endif
    நிலை FFI_TYPE_UINT8:
      திரும்பு PPC_LD_U8;
    நிலை FFI_TYPE_SINT8:
      திரும்பு PPC_LD_S8;
    நிலை FFI_TYPE_UINT16:
      திரும்பு PPC_LD_U16;
    நிலை FFI_TYPE_SINT16:
      திரும்பு PPC_LD_S16;
    நிலை FFI_TYPE_UINT32:
      திரும்பு PPC_LD_U32;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      திரும்பு PPC_LD_S32;
    நிலை FFI_TYPE_POINTER:
      திரும்பு PPC_LD_PTR;
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
      திரும்பு PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    நிலை FFI_TYPE_COMPLEX:
      {
	முழு inner = cif->rtype->elements[0]->type;
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	எனில் (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    எனில் ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      திரும்பு PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	எனில் (inner == FFI_TYPE_FLOAT)
	  திரும்பு PPC64_LD_FLOAT_HOMOG;
	எனில் (inner == FFI_TYPE_DOUBLE)
	  திரும்பு PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  எழுத்து *rv = rvalue;
	  unsigned long re, im;
	  தேர்வு (inner)
	    {
	    நிலை FFI_TYPE_UINT8:
	      re = ((unsigned எழுத்து *) rv)[0];
	      im = ((unsigned எழுத்து *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed எழுத்து *) rv)[0];
	      im = (unsigned long) (long) ((signed எழுத்து *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_UINT32:
	      re = ((unsigned முழு *) rv)[0];
	      im = ((unsigned முழு *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_INT:
	    நிலை FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed முழு *) rv)[0];
	      im = (unsigned long) (long) ((signed முழு *) rv)[1];
	      நிறுத்து;
	    நிலை FFI_TYPE_SINT64:
	    நிலை FFI_TYPE_UINT64:
	    நிலை FFI_TYPE_POINTER:
	    இயல்பு:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      நிறுத்து;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  திரும்பு PPC_LD_R3R4;
	}
      }
#endif
    நிலை FFI_TYPE_STRUCT:
      எனில் ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  எனில் ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      தேர்வு (cif->rtype->size)
		{
		நிலை 0:
		  திரும்பு PPC_LD_NONE;
		நிலை 1:
		  திரும்பு PPC_LD_U8;
		நிலை 2:
		  திரும்பு PPC_LD_U16;
		நிலை 3:
		  திரும்பு PPC64_LD_STRUCT_3;
		நிலை 4:
		  திரும்பு PPC_LD_U32;
		நிலை 5:
		  திரும்பு PPC64_LD_STRUCT_5;
		நிலை 6:
		  திரும்பு PPC64_LD_STRUCT_6;
		நிலை 7:
		  திரும்பு PPC64_LD_STRUCT_7;
		நிலை 8 ... 16:
		  திரும்பு PPC_LD_R3R4;
		}
	      நிறுத்து;
	    }
	  எனில் ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    திரும்பு PPC64_LD_VECTOR_HOMOG;
	  எனில் ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    திரும்பு PPC64_LD_DOUBLE_HOMOG;
	  திரும்பு PPC64_LD_FLOAT_HOMOG;
        }
      திரும்பு PPC_LD_NONE;
    }
  abort();
}
#endif

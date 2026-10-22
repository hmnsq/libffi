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

#અન્તર્ભાવ "ffi.h"
#અન્તર્ભાવ <tramp.h>
#અન્તર્ભાવ <stdlib.h>

#ifdef POWERPC64
#અન્તર્ભાવ "ffi_common.h"
#અન્તર્ભાવ "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ગણના {
  અંક_GPR_ARG_REGISTERS64 = 8,
  અંક_FPR_ARG_REGISTERS64 = 13,
  અંક_VEC_ARG_REGISTERS64 = 12,
};
ગણના { ASM_NEEDS_REGISTERS64 = 4 };


#જો HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
શૂન્ય FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  જો ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  નહીંતર
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


સ્થિર unsigned પૂર્ણાંક
discover_homogeneous_aggregate (ffi_abi abi,
                                અચલ ffi_type *t,
                                unsigned પૂર્ણાંક *elnum)
{
  પસંદગી (t->type)
    {
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      જો ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          પાછા FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      નહીંતર જો ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          પાછા FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    સ્થિતિ FFI_TYPE_FLOAT:
    સ્થિતિ FFI_TYPE_DOUBLE:
      *elnum = 1;
      પાછા (પૂર્ણાંક) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    સ્થિતિ FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned પૂર્ણાંક inner_elnum = 0;
	unsigned પૂર્ણાંક inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	જો (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    પાછા inner;
	  }
	પાછા 0;
      }
#endif

    સ્થિતિ FFI_TYPE_STRUCT:;
      {
	unsigned પૂર્ણાંક base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	જ્યારે (*el)
	  {
	    unsigned પૂર્ણાંક el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    જો (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      પાછા 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#જો _CALL_ELF == 2
	    જો (total_elnum > 8)
	      પાછા 0;
#નહીંતર
	    જો (total_elnum > 1)
	      પાછા 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	પાછા base_elt;
      }

    મૂળભૂત:
      પાછા 0;
    }
}


/* Perform machine dependent cif processing */
સ્થિર ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_ગણતરી = 0, intarg_ગણતરી = 0, vecarg_ગણતરી = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#જો FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    પાછા FFI_BAD_ABI;
#અથવાજો !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    પાછા FFI_BAD_ABI;
#નહીંતર
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    પાછા FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#જો _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * કદ (long);

  /* Space for the general registers.  */
  bytes += અંક_GPR_ARG_REGISTERS64 * કદ (long);
#નહીંતર
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * કદ (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * અંક_GPR_ARG_REGISTERS64 * કદ (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#જો _CALL_ELF == 2
homogeneous:
#endif
  પસંદગી (rtype)
    {
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          તોડો;
        }
      જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    સ્થિતિ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      તોડો;

    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
#જો _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      જો (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          જાઓ homogeneous;
        }
      જો (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          તોડો;
        }
#endif
      intarg_ગણતરી++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    સ્થિતિ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      તોડો;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    સ્થિતિ FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      પસંદગી (rtype)
        {
        સ્થિતિ FFI_TYPE_FLOAT:
        સ્થિતિ FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          જાઓ homogeneous;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        સ્થિતિ FFI_TYPE_LONGDOUBLE:
          જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              તોડો;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          જાઓ homogeneous;
#endif
        સ્થિતિ FFI_TYPE_INT:
        સ્થિતિ FFI_TYPE_SINT8:  સ્થિતિ FFI_TYPE_UINT8:
        સ્થિતિ FFI_TYPE_SINT16: સ્થિતિ FFI_TYPE_UINT16:
        સ્થિતિ FFI_TYPE_SINT32: સ્થિતિ FFI_TYPE_UINT32:
        સ્થિતિ FFI_TYPE_SINT64: સ્થિતિ FFI_TYPE_UINT64:
        સ્થિતિ FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          તોડો;
        મૂળભૂત:
          પાછા FFI_BAD_TYPEDEF;
        }
      તોડો;
#endif

    મૂળભૂત:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      તોડો;
    }

  માટે (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned પૂર્ણાંક align;

      પસંદગી ((*ptr)->type)
	{
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
          જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ગણતરી++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_ગણતરી = (intarg_ગણતરી + 3) & ~0x1;
              જો (vecarg_ગણતરી > અંક_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              તોડો;
            }
	  જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_ગણતરી++;
	      intarg_ગણતરી++;
	    }
	  /* Fall through.  */
#endif
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_FLOAT:
	  fparg_ગણતરી++;
	  intarg_ગણતરી++;
	  જો (fparg_ગણતરી > અંક_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  જો ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      જો (align > 16)
		align = 16;
	      align = align / 8;
	      જો (align > 1)
		intarg_ગણતરી = FFI_ALIGN (intarg_ગણતરી, align);
	    }
	  intarg_ગણતરી += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          જો (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_ગણતરી += elnum;
              જો (vecarg_ગણતરી > અંક_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              તોડો;
            }
	  નહીંતર
#endif
	  જો (elt)
	    {
	      fparg_ગણતરી += elnum;
	      જો (fparg_ગણતરી > અંક_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  નહીંતર
	    {
	      જો (intarg_ગણતરી > અંક_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  તોડો;

	સ્થિતિ FFI_TYPE_POINTER:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ગણતરી++;
	  જો (intarg_ગણતરી > અંક_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  તોડો;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  જો (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  પસંદગી (elt)
	    {
	    સ્થિતિ FFI_TYPE_FLOAT:
	    સ્થિતિ FFI_TYPE_DOUBLE:
	      fparg_ગણતરી += 2;
	      intarg_ગણતરી += 2;
	      જો (fparg_ગણતરી > અંક_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      તોડો;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    સ્થિતિ FFI_TYPE_LONGDOUBLE:
	      જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_ગણતરી += 2;
		  intarg_ગણતરી = (intarg_ગણતરી + 1) & ~0x1;
		  intarg_ગણતરી += 4;
		  જો (vecarg_ગણતરી > અંક_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  તોડો;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_ગણતરી += 4;
	      intarg_ગણતરી += 4;
	      જો (fparg_ગણતરી > અંક_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      તોડો;
#endif
	    સ્થિતિ FFI_TYPE_INT:
	    સ્થિતિ FFI_TYPE_SINT8:  સ્થિતિ FFI_TYPE_UINT8:
	    સ્થિતિ FFI_TYPE_SINT16: સ્થિતિ FFI_TYPE_UINT16:
	    સ્થિતિ FFI_TYPE_SINT32: સ્થિતિ FFI_TYPE_UINT32:
	    સ્થિતિ FFI_TYPE_SINT64: સ્થિતિ FFI_TYPE_UINT64:
	    સ્થિતિ FFI_TYPE_POINTER:
	      intarg_ગણતરી += 2;
	      તોડો;
	    મૂળભૂત:
	      પાછા FFI_BAD_TYPEDEF;
	    }
	  જો (intarg_ગણતરી > અંક_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  તોડો;
#endif

	મૂળભૂત:
	  FFI_ASSERT (0);
	}
    }

  જો (fparg_ગણતરી != 0)
    flags |= FLAG_FP_ARGUMENTS;
  જો (intarg_ગણતરી > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  જો (vecarg_ગણતરી != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  જો (fparg_ગણતરી != 0)
    bytes += અંક_FPR_ARG_REGISTERS64 * કદ (દ્વિગુણ);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  જો (vecarg_ગણતરી != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += અંક_VEC_ARG_REGISTERS64 * કદ (float128);
  }

  /* Stack space.  */
#જો _CALL_ELF == 2
  જો ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_ગણતરી * કદ (long);
#નહીંતર
  જો (intarg_ગણતરી > અંક_GPR_ARG_REGISTERS64)
    bytes += (intarg_ગણતરી - અંક_GPR_ARG_REGISTERS64) * કદ (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  પાછા FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  જો ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#જો _CALL_ELF != 2
  નહીંતર જો (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  નહીંતર
    પાછા FFI_BAD_ABI;
  પાછા ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned પૂર્ણાંક nfixedargs,
			  unsigned પૂર્ણાંક ntotalargs MAYBE_UNUSED)
{
  જો ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#જો _CALL_ELF != 2
  નહીંતર જો (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  નહીંતર
    પાછા FFI_BAD_ABI;
#જો _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  પાછા ffi_prep_cif_linux64_core (cif);
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

શૂન્ય FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *અચલ ઢગલો)
{
  અચલ unsigned long bytes = ecif->cif->bytes;
  અચલ unsigned long flags = ecif->cif->flags;

  પ્રકારવ્યાખ્યા સંઘ
  {
    અક્ષર *c;
    unsigned long *ul;
    દશાંશ *f;
    દ્વિગુણ *d;
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
  unsigned પૂર્ણાંક fparg_ગણતરી;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned પૂર્ણાંક vecarg_ગણતરી;

  unsigned પૂર્ણાંક i, શબ્દો, nargs, nfixedargs;
  ffi_type **ptr;
  દ્વિગુણ double_tmp;
  સંઘ
  {
    શૂન્ય **v;
    અક્ષર **c;
    signed અક્ષર **sc;
    unsigned અક્ષર **uc;
    signed short **ss;
    unsigned short **us;
    signed પૂર્ણાંક **si;
    unsigned પૂર્ણાંક **ui;
    unsigned long **ul;
    દશાંશ **f;
    દ્વિગુણ **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (અક્ષર *) ઢગલો + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - અંક_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + અંક_GPR_ARG_REGISTERS64;
#જો _CALL_ELF == 2
  rest.ul = ઢગલો + 4 + અંક_GPR_ARG_REGISTERS64;
#નહીંતર
  rest.ul = ઢગલો + 6 + અંક_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - અંક_FPR_ARG_REGISTERS64;
  fparg_ગણતરી = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  જો (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  નહીંતર
    vec_base.p = gpr_base.p;
  vec_base.f128 -= અંક_VEC_ARG_REGISTERS64;
  vecarg_ગણતરી = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (અક્ષર *) ઢગલો & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  જો (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (અક્ષર *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#જો _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  જો ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  માટે (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned પૂર્ણાંક elt, elnum;

      પસંદગી ((*ptr)->type)
	{
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
          જો ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              જો (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              જો (vecarg_ગણતરી < અંક_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, કદ (float128));
              નહીંતર
		memcpy (next_arg.f128, *p_argv.f128, કદ (float128));
              જો (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_ગણતરી++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              તોડો;
            }
	  જો ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# જો _CALL_ELF != 2
		  જો ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      નહીંતર
		*next_arg.d = double_tmp;
	      જો (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ગણતરી++;
	      double_tmp = (*p_argv.d)[1];
	      જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# જો _CALL_ELF != 2
		  જો ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      નહીંતર
		*next_arg.d = double_tmp;
	      જો (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_ગણતરી++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      તોડો;
	    }
	  /* Fall through.  */
#endif
	સ્થિતિ FFI_TYPE_DOUBLE:
#જો _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#જો _CALL_ELF != 2
	      જો ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  નહીંતર
	    *next_arg.d = double_tmp;
	  જો (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ગણતરી++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
#જો _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#જો _CALL_ELF != 2
	      જો ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (દશાંશ) double_tmp;
# નહીંતર
		  next_arg.f[0] = (દશાંશ) double_tmp;
# endif
		}
#endif
	    }
	  નહીંતર
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (દશાંશ) double_tmp;
# નહીંતર
	      next_arg.f[0] = (દશાંશ) double_tmp;
# endif
	    }
	  જો (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_ગણતરી++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  તોડો;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	સ્થિતિ FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  જો (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  જો (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned પૂર્ણાંક j;
	      માટે (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  જો (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  જો (vecarg_ગણતરી < અંક_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, કદ (float128));
		  નહીંતર
		    memcpy (next_arg.f128, cval + j, કદ (float128));
		  જો (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_ગણતરી++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      તોડો;
	    }
	  જો (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      દ્વિગુણ *cval = (દ્વિગુણ *) *p_argv.v;
	      unsigned પૂર્ણાંક j;
	      માટે (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  નહીંતર
		    *next_arg.d = double_tmp;
		  જો (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ગણતરી++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      તોડો;
	    }
#endif
	  જો (elt == FFI_TYPE_FLOAT)
	    {
	      દશાંશ *cval = (દશાંશ *) *p_argv.v;
	      unsigned પૂર્ણાંક j;
	      માટે (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  નહીંતર
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (દશાંશ) double_tmp;
# નહીંતર
		      next_arg.f[0] = (દશાંશ) double_tmp;
# endif
		    }
		  જો (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ગણતરી++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  નહીંતર જો (elt == FFI_TYPE_DOUBLE)
	    {
	      દ્વિગુણ *cval = (દ્વિગુણ *) *p_argv.v;
	      unsigned પૂર્ણાંક j;
	      માટે (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  નહીંતર
		    *next_arg.d = double_tmp;
		  જો (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_ગણતરી++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  નહીંતર
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      અક્ષર *cval = (અક્ષર *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned પૂર્ણાંક j;
	      માટે (j = 0; j < 2; j++)
		{
		  અક્ષર *half = cval + j * hsize;
		  unsigned long gprvalue;
		  પસંદગી (elt)
		    {
		    સ્થિતિ FFI_TYPE_UINT8:
		      gprvalue = *(unsigned અક્ષર *) half; તોડો;
		    સ્થિતિ FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed અક્ષર *) half;
		      તોડો;
		    સ્થિતિ FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; તોડો;
		    સ્થિતિ FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      તોડો;
		    સ્થિતિ FFI_TYPE_UINT32:
		      gprvalue = *(unsigned પૂર્ણાંક *) half; તોડો;
		    સ્થિતિ FFI_TYPE_INT:
		    સ્થિતિ FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed પૂર્ણાંક *) half;
		      તોડો;
		    સ્થિતિ FFI_TYPE_SINT64:
		    સ્થિતિ FFI_TYPE_UINT64:
		    સ્થિતિ FFI_TYPE_POINTER:
		    મૂળભૂત:
		      gprvalue = *(unsigned long *) half; તોડો;
		    }
		  *next_arg.ul++ = gprvalue;
		  જો (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_STRUCT:
	  જો ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      જો (align > 16)
		align = 16;
	      જો (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  જો (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  જો (elt)
	    {
#જો _CALL_ELF == 2
	      સંઘ {
		શૂન્ય *v;
		દશાંશ *f;
		દ્વિગુણ *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              જો (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  કરો
                    {
                      જો (vecarg_ગણતરી < અંક_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, કદ (float128));
                      નહીંતર
		        memcpy (next_arg.f128, arg.f128++, કદ (float128));
                      જો (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_ગણતરી++;
                    }
                  જ્યારે (--elnum != 0);
                }
              નહીંતર
#endif
	      જો (elt == FFI_TYPE_FLOAT)
		{
		  કરો
		    {
		      double_tmp = *arg.f++;
		      જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      નહીંતર
			*next_arg.f = (દશાંશ) double_tmp;
		      જો (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_ગણતરી++;
		    }
		  જ્યારે (--elnum != 0);
		  જો ((next_arg.p & 7) != 0)
                    જો (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      નહીંતર
		કરો
		  {
		    double_tmp = *arg.d++;
		    જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    નહીંતર
		      *next_arg.d = double_tmp;
		    જો (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_ગણતરી++;
		  }
		જ્યારે (--elnum != 0);
#નહીંતર
	      જો (elt == FFI_TYPE_FLOAT)
		જાઓ do_float;
	      નહીંતર
		જાઓ do_double;
#endif
	    }
	  નહીંતર
	    {
	      શબ્દો = ((*ptr)->size + 7) / 8;
	      જો (next_arg.ul >= gpr_base.ul && next_arg.ul + શબ્દો > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + શબ્દો * 8 - first;
		}
	      નહીંતર
		{
		  અક્ષર *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  જો ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += શબ્દો;
		  જો (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  જાઓ putgpr;

	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  જો (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  તોડો;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#જો _CALL_ELF == 2
#define MIN_CACHE_લીટી_SIZE 8

સ્થિર શૂન્ય
flush_icache (અક્ષર *wraddr, અક્ષર *xaddr, પૂર્ણાંક size)
{
  પૂર્ણાંક i;
  માટે (i = 0; i < size; i += MIN_CACHE_લીટી_SIZE)
    __asm__ અસ્થિર ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ અસ્થિર ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
			      શૂન્ય *user_data,
			      શૂન્ય *codeloc)
{
  જો (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    પાછા FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  જો (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      શૂન્ય (*dest)(શૂન્ય) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  નહીંતર
#endif
    {
#જો _CALL_ELF == 2
      unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(શૂન્ય **) &tramp[4] = (શૂન્ય *) ffi_closure_LINUX64;
      *(શૂન્ય **) &tramp[6] = codeloc;
      flush_icache ((અક્ષર *) tramp, (અક્ષર *) codeloc, 4 * 4);
#નહીંતર
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      શૂન્ય **tramp = (શૂન્ય **) &closure->tramp[0];
      memcpy (&tramp[0], (શૂન્ય **) ffi_closure_LINUX64, કદ (શૂન્ય *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (શૂન્ય **) ffi_closure_LINUX64 + 1, કદ (શૂન્ય *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}


પૂર્ણાંક FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    શૂન્ય (*fun) (ffi_cif *, શૂન્ય *, શૂન્ય **, શૂન્ય *),
			    શૂન્ય *user_data,
			    શૂન્ય *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  શૂન્ય **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + અંક_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + અંક_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * કદ (શૂન્ય *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  જો (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (શૂન્ય *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#જો _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  જો ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  જ્યારે (i < avn)
    {
      unsigned પૂર્ણાંક elt, elnum;

      પસંદગી (arg_types[i]->type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (અક્ષર *) pst + 7;
	  pst++;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (અક્ષર *) pst + 6;
	  pst++;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (અક્ષર *) pst + 4;
	  pst++;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  જો ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      જો (align > 16)
		align = 16;
	      જો (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  જો (elt)
	    {
#જો _CALL_ELF == 2
	      સંઘ {
		શૂન્ય *v;
		unsigned long *ul;
		દશાંશ *f;
		દ્વિગુણ *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              જો (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  જો (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  નહીંતર
                    to.v = pst;
                }
              નહીંતર
#endif
	      જો (pfr + elnum <= end_pfr)
		to.v = pfr;
	      નહીંતર
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              જો (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  કરો
                    {
                      જો (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, કદ (float128));
                      નહીંતર
		        memcpy (to.f128, from.f128, કદ (float128));
                      to.f128++;
                      from.f128++;
                    }
                  જ્યારે (--elnum != 0);
                }
              નહીંતર
#endif
	      જો (elt == FFI_TYPE_FLOAT)
		{
		  કરો
		    {
		      જો (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (દશાંશ) pfr->d;
			  pfr++;
			}
		      નહીંતર
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  જ્યારે (--elnum != 0);
		}
	      નહીંતર
		{
		  કરો
		    {
		      જો (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      નહીંતર
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  જ્યારે (--elnum != 0);
		}
#નહીંતર
	      જો (elt == FFI_TYPE_FLOAT)
		જાઓ do_float;
	      નહીંતર
		જાઓ do_double;
#endif
	    }
	  નહીંતર
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      જો (arg_types[i]->size < 8)
		avalue[i] = (અક્ષર *) pst + 8 - arg_types[i]->size;
	      નહીંતર
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  તોડો;

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
          જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              જો (((unsigned long) pst & 0xF) != 0)
                ++pst;
              જો (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              નહીંતર
                avalue[i] = pst;
              pst += 2;
              તોડો;
            }
          નહીંતર જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      જો (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      નહીંતર
		{
		  જો (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      તોડો;
	    }
	  /* Fall through.  */
#endif
	સ્થિતિ FFI_TYPE_DOUBLE:
#જો _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  જો (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  નહીંતર
	    avalue[i] = pst;
	  pst++;
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
#જો _CALL_ELF != 2
	do_float:
#endif
	  જો (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (દશાંશ) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  નહીંતર
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (અક્ષર *) pst + 4;
#નહીંતર
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  તોડો;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned પૂર્ણાંક j;
	    elt = arg_types[i]->elements[0]->type;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    જો (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    જો (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * કદ (float128));
		જો (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		માટે (j = 0; j < 2; j++)
		  {
		    જો (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, કદ (float128));
		    નહીંતર
		      memcpy (&cval[j], pst, કદ (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		તોડો;
	      }
	    જો (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		દ્વિગુણ *cval = alloca (4 * કદ (દ્વિગુણ));
		માટે (j = 0; j < 4; j++)
		  {
		    જો (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    નહીંતર
		      cval[j] = *(દ્વિગુણ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		તોડો;
	      }
#endif
	    જો (elt == FFI_TYPE_FLOAT)
	      {
		દશાંશ *cval = alloca (2 * કદ (દશાંશ));
		માટે (j = 0; j < 2; j++)
		  {
		    જો (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (દશાંશ) pfr->d;
			pfr++;
		      }
		    નહીંતર
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((દશાંશ *) pst)[1];
#નહીંતર
			cval[j] = ((દશાંશ *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    નહીંતર જો (elt == FFI_TYPE_DOUBLE)
	      {
		દ્વિગુણ *cval = alloca (2 * કદ (દ્વિગુણ));
		માટે (j = 0; j < 2; j++)
		  {
		    જો (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    નહીંતર
		      cval[j] = *(દ્વિગુણ *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    નહીંતર
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		અક્ષર *cval = alloca (2 * hsize);
		માટે (j = 0; j < 2; j++)
		  {
		    અક્ષર *src;
#ifndef __LITTLE_ENDIAN__
		    src = (અક્ષર *) pst + (8 - hsize);
#નહીંતર
		    src = (અક્ષર *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  તોડો;
#endif

	મૂળભૂત:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      પાછા PPC_LD_NONE;
    સ્થિતિ FFI_TYPE_FLOAT:
      પાછા PPC_LD_F32;
    સ્થિતિ FFI_TYPE_DOUBLE:
      પાછા PPC_LD_F64;
#જો FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      જો ((cif->flags & FLAG_RETURNS_VEC) != 0)
	પાછા PPC64_LD_VECTOR;
      પાછા PPC_LD_F128;
#endif
    સ્થિતિ FFI_TYPE_UINT8:
      પાછા PPC_LD_U8;
    સ્થિતિ FFI_TYPE_SINT8:
      પાછા PPC_LD_S8;
    સ્થિતિ FFI_TYPE_UINT16:
      પાછા PPC_LD_U16;
    સ્થિતિ FFI_TYPE_SINT16:
      પાછા PPC_LD_S16;
    સ્થિતિ FFI_TYPE_UINT32:
      પાછા PPC_LD_U32;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      પાછા PPC_LD_S32;
    સ્થિતિ FFI_TYPE_POINTER:
      પાછા PPC_LD_PTR;
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
      પાછા PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    સ્થિતિ FFI_TYPE_COMPLEX:
      {
	પૂર્ણાંક inner = cif->rtype->elements[0]->type;
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	જો (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    જો ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      પાછા PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	જો (inner == FFI_TYPE_FLOAT)
	  પાછા PPC64_LD_FLOAT_HOMOG;
	જો (inner == FFI_TYPE_DOUBLE)
	  પાછા PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  અક્ષર *rv = rvalue;
	  unsigned long re, im;
	  પસંદગી (inner)
	    {
	    સ્થિતિ FFI_TYPE_UINT8:
	      re = ((unsigned અક્ષર *) rv)[0];
	      im = ((unsigned અક્ષર *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed અક્ષર *) rv)[0];
	      im = (unsigned long) (long) ((signed અક્ષર *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_UINT32:
	      re = ((unsigned પૂર્ણાંક *) rv)[0];
	      im = ((unsigned પૂર્ણાંક *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_INT:
	    સ્થિતિ FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed પૂર્ણાંક *) rv)[0];
	      im = (unsigned long) (long) ((signed પૂર્ણાંક *) rv)[1];
	      તોડો;
	    સ્થિતિ FFI_TYPE_SINT64:
	    સ્થિતિ FFI_TYPE_UINT64:
	    સ્થિતિ FFI_TYPE_POINTER:
	    મૂળભૂત:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      તોડો;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  પાછા PPC_LD_R3R4;
	}
      }
#endif
    સ્થિતિ FFI_TYPE_STRUCT:
      જો ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  જો ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      પસંદગી (cif->rtype->size)
		{
		સ્થિતિ 0:
		  પાછા PPC_LD_NONE;
		સ્થિતિ 1:
		  પાછા PPC_LD_U8;
		સ્થિતિ 2:
		  પાછા PPC_LD_U16;
		સ્થિતિ 3:
		  પાછા PPC64_LD_STRUCT_3;
		સ્થિતિ 4:
		  પાછા PPC_LD_U32;
		સ્થિતિ 5:
		  પાછા PPC64_LD_STRUCT_5;
		સ્થિતિ 6:
		  પાછા PPC64_LD_STRUCT_6;
		સ્થિતિ 7:
		  પાછા PPC64_LD_STRUCT_7;
		સ્થિતિ 8 ... 16:
		  પાછા PPC_LD_R3R4;
		}
	      તોડો;
	    }
	  જો ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    પાછા PPC64_LD_VECTOR_HOMOG;
	  જો ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    પાછા PPC64_LD_DOUBLE_HOMOG;
	  પાછા PPC64_LD_FLOAT_HOMOG;
        }
      પાછા PPC_LD_NONE;
    }
  abort();
}
#endif

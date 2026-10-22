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

#شامل "ffi.h"
#شامل <tramp.h>
#شامل <stdlib.h>

#ifdef POWERPC64
#شامل "ffi_common.h"
#شامل "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
شمارش {
  رقم_GPR_ARG_REGISTERS64 = 8,
  رقم_FPR_ARG_REGISTERS64 = 13,
  رقم_VEC_ARG_REGISTERS64 = 12,
};
شمارش { ASM_NEEDS_REGISTERS64 = 4 };


#اگر HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
تهی FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  اگر ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  وگرنه
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


ایستا unsigned صحیح
discover_homogeneous_aggregate (ffi_abi abi,
                                ثابت ffi_type *t,
                                unsigned صحیح *elnum)
{
  انتخاب (t->type)
    {
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      اگر ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          بازگشت FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      وگرنه اگر ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          بازگشت FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
      *elnum = 1;
      بازگشت (صحیح) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned صحیح inner_elnum = 0;
	unsigned صحیح inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	اگر (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    بازگشت inner;
	  }
	بازگشت 0;
      }
#endif

    حالت FFI_TYPE_STRUCT:;
      {
	unsigned صحیح base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	وقتی (*el)
	  {
	    unsigned صحیح el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    اگر (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      بازگشت 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#اگر _CALL_ELF == 2
	    اگر (total_elnum > 8)
	      بازگشت 0;
#وگرنه
	    اگر (total_elnum > 1)
	      بازگشت 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	بازگشت base_elt;
      }

    پیشفرض:
      بازگشت 0;
    }
}


/* Perform machine dependent cif processing */
ایستا ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_شمار = 0, intarg_شمار = 0, vecarg_شمار = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#اگر FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    بازگشت FFI_BAD_ABI;
#وگرنهاگر !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    بازگشت FFI_BAD_ABI;
#وگرنه
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    بازگشت FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#اگر _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * اندازه (long);

  /* Space for the general registers.  */
  bytes += رقم_GPR_ARG_REGISTERS64 * اندازه (long);
#وگرنه
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * اندازه (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * رقم_GPR_ARG_REGISTERS64 * اندازه (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#اگر _CALL_ELF == 2
homogeneous:
#endif
  انتخاب (rtype)
    {
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          بشکن;
        }
      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    حالت FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    حالت FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      بشکن;

    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      بشکن;

    حالت FFI_TYPE_STRUCT:
#اگر _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      اگر (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          برو homogeneous;
        }
      اگر (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          بشکن;
        }
#endif
      intarg_شمار++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالت FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      بشکن;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      انتخاب (rtype)
        {
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          برو homogeneous;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              بشکن;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          برو homogeneous;
#endif
        حالت FFI_TYPE_INT:
        حالت FFI_TYPE_SINT8:  حالت FFI_TYPE_UINT8:
        حالت FFI_TYPE_SINT16: حالت FFI_TYPE_UINT16:
        حالت FFI_TYPE_SINT32: حالت FFI_TYPE_UINT32:
        حالت FFI_TYPE_SINT64: حالت FFI_TYPE_UINT64:
        حالت FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          بشکن;
        پیشفرض:
          بازگشت FFI_BAD_TYPEDEF;
        }
      بشکن;
#endif

    پیشفرض:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      بشکن;
    }

  برای (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned صحیح align;

      انتخاب ((*ptr)->type)
	{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_شمار++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_شمار = (intarg_شمار + 3) & ~0x1;
              اگر (vecarg_شمار > رقم_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              بشکن;
            }
	  اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_شمار++;
	      intarg_شمار++;
	    }
	  /* Fall through.  */
#endif
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_FLOAT:
	  fparg_شمار++;
	  intarg_شمار++;
	  اگر (fparg_شمار > رقم_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  اگر ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      اگر (align > 16)
		align = 16;
	      align = align / 8;
	      اگر (align > 1)
		intarg_شمار = FFI_ALIGN (intarg_شمار, align);
	    }
	  intarg_شمار += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          اگر (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_شمار += elnum;
              اگر (vecarg_شمار > رقم_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              بشکن;
            }
	  وگرنه
#endif
	  اگر (elt)
	    {
	      fparg_شمار += elnum;
	      اگر (fparg_شمار > رقم_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  وگرنه
	    {
	      اگر (intarg_شمار > رقم_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  بشکن;

	حالت FFI_TYPE_POINTER:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_شمار++;
	  اگر (intarg_شمار > رقم_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  بشکن;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  انتخاب (elt)
	    {
	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_DOUBLE:
	      fparg_شمار += 2;
	      intarg_شمار += 2;
	      اگر (fparg_شمار > رقم_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      بشکن;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    حالت FFI_TYPE_LONGDOUBLE:
	      اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_شمار += 2;
		  intarg_شمار = (intarg_شمار + 1) & ~0x1;
		  intarg_شمار += 4;
		  اگر (vecarg_شمار > رقم_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  بشکن;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_شمار += 4;
	      intarg_شمار += 4;
	      اگر (fparg_شمار > رقم_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      بشکن;
#endif
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT8:  حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16: حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_SINT32: حالت FFI_TYPE_UINT32:
	    حالت FFI_TYPE_SINT64: حالت FFI_TYPE_UINT64:
	    حالت FFI_TYPE_POINTER:
	      intarg_شمار += 2;
	      بشکن;
	    پیشفرض:
	      بازگشت FFI_BAD_TYPEDEF;
	    }
	  اگر (intarg_شمار > رقم_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  بشکن;
#endif

	پیشفرض:
	  FFI_ASSERT (0);
	}
    }

  اگر (fparg_شمار != 0)
    flags |= FLAG_FP_ARGUMENTS;
  اگر (intarg_شمار > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  اگر (vecarg_شمار != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  اگر (fparg_شمار != 0)
    bytes += رقم_FPR_ARG_REGISTERS64 * اندازه (دوگانه);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  اگر (vecarg_شمار != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += رقم_VEC_ARG_REGISTERS64 * اندازه (float128);
  }

  /* Stack space.  */
#اگر _CALL_ELF == 2
  اگر ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_شمار * اندازه (long);
#وگرنه
  اگر (intarg_شمار > رقم_GPR_ARG_REGISTERS64)
    bytes += (intarg_شمار - رقم_GPR_ARG_REGISTERS64) * اندازه (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  بازگشت FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  اگر ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#اگر _CALL_ELF != 2
  وگرنه اگر (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  وگرنه
    بازگشت FFI_BAD_ABI;
  بازگشت ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned صحیح nfixedargs,
			  unsigned صحیح ntotalargs MAYBE_UNUSED)
{
  اگر ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#اگر _CALL_ELF != 2
  وگرنه اگر (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  وگرنه
    بازگشت FFI_BAD_ABI;
#اگر _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  بازگشت ffi_prep_cif_linux64_core (cif);
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

تهی FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *ثابت پشته)
{
  ثابت unsigned long bytes = ecif->cif->bytes;
  ثابت unsigned long flags = ecif->cif->flags;

  تعریفنوع اجتماع
  {
    نویسه *c;
    unsigned long *ul;
    اعشاری *f;
    دوگانه *d;
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
  unsigned صحیح fparg_شمار;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned صحیح vecarg_شمار;

  unsigned صحیح i, واژگان, nargs, nfixedargs;
  ffi_type **ptr;
  دوگانه double_tmp;
  اجتماع
  {
    تهی **v;
    نویسه **c;
    signed نویسه **sc;
    unsigned نویسه **uc;
    signed short **ss;
    unsigned short **us;
    signed صحیح **si;
    unsigned صحیح **ui;
    unsigned long **ul;
    اعشاری **f;
    دوگانه **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (نویسه *) پشته + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - رقم_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + رقم_GPR_ARG_REGISTERS64;
#اگر _CALL_ELF == 2
  rest.ul = پشته + 4 + رقم_GPR_ARG_REGISTERS64;
#وگرنه
  rest.ul = پشته + 6 + رقم_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - رقم_FPR_ARG_REGISTERS64;
  fparg_شمار = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  اگر (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  وگرنه
    vec_base.p = gpr_base.p;
  vec_base.f128 -= رقم_VEC_ARG_REGISTERS64;
  vecarg_شمار = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (نویسه *) پشته & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  اگر (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (نویسه *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#اگر _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  اگر ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  برای (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned صحیح elt, elnum;

      انتخاب ((*ptr)->type)
	{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              اگر (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              اگر (vecarg_شمار < رقم_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, اندازه (float128));
              وگرنه
		memcpy (next_arg.f128, *p_argv.f128, اندازه (float128));
              اگر (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_شمار++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              بشکن;
            }
	  اگر ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# اگر _CALL_ELF != 2
		  اگر ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      وگرنه
		*next_arg.d = double_tmp;
	      اگر (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_شمار++;
	      double_tmp = (*p_argv.d)[1];
	      اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# اگر _CALL_ELF != 2
		  اگر ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      وگرنه
		*next_arg.d = double_tmp;
	      اگر (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_شمار++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      بشکن;
	    }
	  /* Fall through.  */
#endif
	حالت FFI_TYPE_DOUBLE:
#اگر _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#اگر _CALL_ELF != 2
	      اگر ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  وگرنه
	    *next_arg.d = double_tmp;
	  اگر (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_شمار++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  بشکن;

	حالت FFI_TYPE_FLOAT:
#اگر _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#اگر _CALL_ELF != 2
	      اگر ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (اعشاری) double_tmp;
# وگرنه
		  next_arg.f[0] = (اعشاری) double_tmp;
# endif
		}
#endif
	    }
	  وگرنه
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (اعشاری) double_tmp;
# وگرنه
	      next_arg.f[0] = (اعشاری) double_tmp;
# endif
	    }
	  اگر (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_شمار++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  بشکن;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  اگر (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned صحیح j;
	      برای (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  اگر (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  اگر (vecarg_شمار < رقم_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, اندازه (float128));
		  وگرنه
		    memcpy (next_arg.f128, cval + j, اندازه (float128));
		  اگر (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_شمار++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      بشکن;
	    }
	  اگر (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      دوگانه *cval = (دوگانه *) *p_argv.v;
	      unsigned صحیح j;
	      برای (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وگرنه
		    *next_arg.d = double_tmp;
		  اگر (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_شمار++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      بشکن;
	    }
#endif
	  اگر (elt == FFI_TYPE_FLOAT)
	    {
	      اعشاری *cval = (اعشاری *) *p_argv.v;
	      unsigned صحیح j;
	      برای (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وگرنه
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (اعشاری) double_tmp;
# وگرنه
		      next_arg.f[0] = (اعشاری) double_tmp;
# endif
		    }
		  اگر (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_شمار++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وگرنه اگر (elt == FFI_TYPE_DOUBLE)
	    {
	      دوگانه *cval = (دوگانه *) *p_argv.v;
	      unsigned صحیح j;
	      برای (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  وگرنه
		    *next_arg.d = double_tmp;
		  اگر (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_شمار++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  وگرنه
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      نویسه *cval = (نویسه *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned صحیح j;
	      برای (j = 0; j < 2; j++)
		{
		  نویسه *half = cval + j * hsize;
		  unsigned long gprvalue;
		  انتخاب (elt)
		    {
		    حالت FFI_TYPE_UINT8:
		      gprvalue = *(unsigned نویسه *) half; بشکن;
		    حالت FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed نویسه *) half;
		      بشکن;
		    حالت FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; بشکن;
		    حالت FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      بشکن;
		    حالت FFI_TYPE_UINT32:
		      gprvalue = *(unsigned صحیح *) half; بشکن;
		    حالت FFI_TYPE_INT:
		    حالت FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed صحیح *) half;
		      بشکن;
		    حالت FFI_TYPE_SINT64:
		    حالت FFI_TYPE_UINT64:
		    حالت FFI_TYPE_POINTER:
		    پیشفرض:
		      gprvalue = *(unsigned long *) half; بشکن;
		    }
		  *next_arg.ul++ = gprvalue;
		  اگر (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  بشکن;
#endif

	حالت FFI_TYPE_STRUCT:
	  اگر ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      اگر (align > 16)
		align = 16;
	      اگر (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  اگر (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  اگر (elt)
	    {
#اگر _CALL_ELF == 2
	      اجتماع {
		تهی *v;
		اعشاری *f;
		دوگانه *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  انجامبده
                    {
                      اگر (vecarg_شمار < رقم_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, اندازه (float128));
                      وگرنه
		        memcpy (next_arg.f128, arg.f128++, اندازه (float128));
                      اگر (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_شمار++;
                    }
                  وقتی (--elnum != 0);
                }
              وگرنه
#endif
	      اگر (elt == FFI_TYPE_FLOAT)
		{
		  انجامبده
		    {
		      double_tmp = *arg.f++;
		      اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      وگرنه
			*next_arg.f = (اعشاری) double_tmp;
		      اگر (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_شمار++;
		    }
		  وقتی (--elnum != 0);
		  اگر ((next_arg.p & 7) != 0)
                    اگر (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      وگرنه
		انجامبده
		  {
		    double_tmp = *arg.d++;
		    اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    وگرنه
		      *next_arg.d = double_tmp;
		    اگر (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_شمار++;
		  }
		وقتی (--elnum != 0);
#وگرنه
	      اگر (elt == FFI_TYPE_FLOAT)
		برو do_float;
	      وگرنه
		برو do_double;
#endif
	    }
	  وگرنه
	    {
	      واژگان = ((*ptr)->size + 7) / 8;
	      اگر (next_arg.ul >= gpr_base.ul && next_arg.ul + واژگان > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + واژگان * 8 - first;
		}
	      وگرنه
		{
		  نویسه *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  اگر ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += واژگان;
		  اگر (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  بشکن;

	حالت FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  برو putgpr;
	حالت FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  برو putgpr;
	حالت FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  برو putgpr;
	حالت FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  برو putgpr;
	حالت FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  برو putgpr;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  برو putgpr;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  اگر (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  بشکن;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#اگر _CALL_ELF == 2
#define MIN_CACHE_سطر_SIZE 8

ایستا تهی
flush_icache (نویسه *wraddr, نویسه *xaddr, صحیح size)
{
  صحیح i;
  برای (i = 0; i < size; i += MIN_CACHE_سطر_SIZE)
    __asm__ فرار ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ فرار ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
			      تهی *user_data,
			      تهی *codeloc)
{
  اگر (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    بازگشت FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  اگر (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      تهی (*dest)(تهی) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  وگرنه
#endif
    {
#اگر _CALL_ELF == 2
      unsigned صحیح *tramp = (unsigned صحیح *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(تهی **) &tramp[4] = (تهی *) ffi_closure_LINUX64;
      *(تهی **) &tramp[6] = codeloc;
      flush_icache ((نویسه *) tramp, (نویسه *) codeloc, 4 * 4);
#وگرنه
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      تهی **tramp = (تهی **) &closure->tramp[0];
      memcpy (&tramp[0], (تهی **) ffi_closure_LINUX64, اندازه (تهی *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (تهی **) ffi_closure_LINUX64 + 1, اندازه (تهی *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}


صحیح FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
			    تهی *user_data,
			    تهی *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  تهی **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + رقم_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + رقم_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * اندازه (تهی *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (تهی *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#اگر _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  اگر ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  وقتی (i < avn)
    {
      unsigned صحیح elt, elnum;

      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (نویسه *) pst + 7;
	  pst++;
	  بشکن;
#endif

	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (نویسه *) pst + 6;
	  pst++;
	  بشکن;
#endif

	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (نویسه *) pst + 4;
	  pst++;
	  بشکن;
#endif

	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  اگر ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      اگر (align > 16)
		align = 16;
	      اگر (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  اگر (elt)
	    {
#اگر _CALL_ELF == 2
	      اجتماع {
		تهی *v;
		unsigned long *ul;
		اعشاری *f;
		دوگانه *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  اگر (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  وگرنه
                    to.v = pst;
                }
              وگرنه
#endif
	      اگر (pfr + elnum <= end_pfr)
		to.v = pfr;
	      وگرنه
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              اگر (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  انجامبده
                    {
                      اگر (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, اندازه (float128));
                      وگرنه
		        memcpy (to.f128, from.f128, اندازه (float128));
                      to.f128++;
                      from.f128++;
                    }
                  وقتی (--elnum != 0);
                }
              وگرنه
#endif
	      اگر (elt == FFI_TYPE_FLOAT)
		{
		  انجامبده
		    {
		      اگر (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (اعشاری) pfr->d;
			  pfr++;
			}
		      وگرنه
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  وقتی (--elnum != 0);
		}
	      وگرنه
		{
		  انجامبده
		    {
		      اگر (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      وگرنه
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  وقتی (--elnum != 0);
		}
#وگرنه
	      اگر (elt == FFI_TYPE_FLOAT)
		برو do_float;
	      وگرنه
		برو do_double;
#endif
	    }
	  وگرنه
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      اگر (arg_types[i]->size < 8)
		avalue[i] = (نویسه *) pst + 8 - arg_types[i]->size;
	      وگرنه
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  بشکن;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
          اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              اگر (((unsigned long) pst & 0xF) != 0)
                ++pst;
              اگر (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              وگرنه
                avalue[i] = pst;
              pst += 2;
              بشکن;
            }
          وگرنه اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      اگر (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      وگرنه
		{
		  اگر (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      بشکن;
	    }
	  /* Fall through.  */
#endif
	حالت FFI_TYPE_DOUBLE:
#اگر _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  اگر (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وگرنه
	    avalue[i] = pst;
	  pst++;
	  بشکن;

	حالت FFI_TYPE_FLOAT:
#اگر _CALL_ELF != 2
	do_float:
#endif
	  اگر (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (اعشاری) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وگرنه
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (نویسه *) pst + 4;
#وگرنه
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  بشکن;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	حالت FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned صحیح j;
	    elt = arg_types[i]->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    اگر (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    اگر (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * اندازه (float128));
		اگر (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		برای (j = 0; j < 2; j++)
		  {
		    اگر (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, اندازه (float128));
		    وگرنه
		      memcpy (&cval[j], pst, اندازه (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		بشکن;
	      }
	    اگر (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		دوگانه *cval = alloca (4 * اندازه (دوگانه));
		برای (j = 0; j < 4; j++)
		  {
		    اگر (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    وگرنه
		      cval[j] = *(دوگانه *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		بشکن;
	      }
#endif
	    اگر (elt == FFI_TYPE_FLOAT)
	      {
		اعشاری *cval = alloca (2 * اندازه (اعشاری));
		برای (j = 0; j < 2; j++)
		  {
		    اگر (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (اعشاری) pfr->d;
			pfr++;
		      }
		    وگرنه
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((اعشاری *) pst)[1];
#وگرنه
			cval[j] = ((اعشاری *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    وگرنه اگر (elt == FFI_TYPE_DOUBLE)
	      {
		دوگانه *cval = alloca (2 * اندازه (دوگانه));
		برای (j = 0; j < 2; j++)
		  {
		    اگر (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    وگرنه
		      cval[j] = *(دوگانه *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    وگرنه
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		نویسه *cval = alloca (2 * hsize);
		برای (j = 0; j < 2; j++)
		  {
		    نویسه *src;
#ifndef __LITTLE_ENDIAN__
		    src = (نویسه *) pst + (8 - hsize);
#وگرنه
		    src = (نویسه *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  بشکن;
#endif

	پیشفرض:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      بازگشت PPC_LD_NONE;
    حالت FFI_TYPE_FLOAT:
      بازگشت PPC_LD_F32;
    حالت FFI_TYPE_DOUBLE:
      بازگشت PPC_LD_F64;
#اگر FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      اگر ((cif->flags & FLAG_RETURNS_VEC) != 0)
	بازگشت PPC64_LD_VECTOR;
      بازگشت PPC_LD_F128;
#endif
    حالت FFI_TYPE_UINT8:
      بازگشت PPC_LD_U8;
    حالت FFI_TYPE_SINT8:
      بازگشت PPC_LD_S8;
    حالت FFI_TYPE_UINT16:
      بازگشت PPC_LD_U16;
    حالت FFI_TYPE_SINT16:
      بازگشت PPC_LD_S16;
    حالت FFI_TYPE_UINT32:
      بازگشت PPC_LD_U32;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      بازگشت PPC_LD_S32;
    حالت FFI_TYPE_POINTER:
      بازگشت PPC_LD_PTR;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      بازگشت PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    حالت FFI_TYPE_COMPLEX:
      {
	صحیح inner = cif->rtype->elements[0]->type;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	اگر (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    اگر ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      بازگشت PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	اگر (inner == FFI_TYPE_FLOAT)
	  بازگشت PPC64_LD_FLOAT_HOMOG;
	اگر (inner == FFI_TYPE_DOUBLE)
	  بازگشت PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  نویسه *rv = rvalue;
	  unsigned long re, im;
	  انتخاب (inner)
	    {
	    حالت FFI_TYPE_UINT8:
	      re = ((unsigned نویسه *) rv)[0];
	      im = ((unsigned نویسه *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed نویسه *) rv)[0];
	      im = (unsigned long) (long) ((signed نویسه *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_UINT32:
	      re = ((unsigned صحیح *) rv)[0];
	      im = ((unsigned صحیح *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed صحیح *) rv)[0];
	      im = (unsigned long) (long) ((signed صحیح *) rv)[1];
	      بشکن;
	    حالت FFI_TYPE_SINT64:
	    حالت FFI_TYPE_UINT64:
	    حالت FFI_TYPE_POINTER:
	    پیشفرض:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      بشکن;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  بازگشت PPC_LD_R3R4;
	}
      }
#endif
    حالت FFI_TYPE_STRUCT:
      اگر ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  اگر ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      انتخاب (cif->rtype->size)
		{
		حالت 0:
		  بازگشت PPC_LD_NONE;
		حالت 1:
		  بازگشت PPC_LD_U8;
		حالت 2:
		  بازگشت PPC_LD_U16;
		حالت 3:
		  بازگشت PPC64_LD_STRUCT_3;
		حالت 4:
		  بازگشت PPC_LD_U32;
		حالت 5:
		  بازگشت PPC64_LD_STRUCT_5;
		حالت 6:
		  بازگشت PPC64_LD_STRUCT_6;
		حالت 7:
		  بازگشت PPC64_LD_STRUCT_7;
		حالت 8 ... 16:
		  بازگشت PPC_LD_R3R4;
		}
	      بشکن;
	    }
	  اگر ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    بازگشت PPC64_LD_VECTOR_HOMOG;
	  اگر ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    بازگشت PPC64_LD_DOUBLE_HOMOG;
	  بازگشت PPC64_LD_FLOAT_HOMOG;
        }
      بازگشت PPC_LD_NONE;
    }
  abort();
}
#endif

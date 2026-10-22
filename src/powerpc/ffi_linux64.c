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

#include "ffi.h"
#include <tramp.h>
#include <stdlib.h>

#ifdef POWERPC64
#include "ffi_common.h"
#include "ffi_powerpc.h"


/* About the LINUX64 ABI.  */
ga_nanaa {
  NUM_GPR_ARG_REGISTERS64 = 8,
  NUM_FPR_ARG_REGISTERS64 = 13,
  NUM_VEC_ARG_REGISTERS64 = 12,
};
ga_nanaa { ASM_NEEDS_REGISTERS64 = 4 };


#အကယ်၍ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
void FFI_HIDDEN
ffi_prep_types_linux64 (ffi_abi abi)
{
  အကယ်၍ ((abi & (FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128)) == FFI_LINUX)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  မဟုတ်ပါက
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


static unsigned ကိန်းပြည့်
discover_homogeneous_aggregate (ffi_abi abi,
                                const ffi_type *t,
                                unsigned ကိန်းပြည့် *elnum)
{
  switch (t->type)
    {
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    pak_sa FFI_TYPE_LONGDOUBLE:
      /* 64-bit long doubles are equivalent to doubles. */
      အကယ်၍ ((abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
        {
          *elnum = 1;
          ပြန်ပေး FFI_TYPE_DOUBLE;
        }
      /* IBM extended precision values use unaligned pairs
         of FPRs, but according to the ABI must be considered
         distinct from doubles. They are also limited to a
         maximum of four members in a homogeneous aggregate. */
      မဟုတ်ပါက အကယ်၍ ((abi & FFI_LINUX_LONG_DOUBLE_IEEE128) == 0)
        {
          *elnum = 2;
          ပြန်ပေး FFI_TYPE_LONGDOUBLE;
        }
      /* Fall through. */
#endif
    pak_sa FFI_TYPE_FLOAT:
    pak_sa FFI_TYPE_DOUBLE:
      *elnum = 1;
      ပြန်ပေး (ကိန်းပြည့်) t->type;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    pak_sa FFI_TYPE_COMPLEX:
      /* Count complex of an FP base as two elements of that base, so a
	 struct containing complex members is recognised as an HFA.  This
	 only affects FFI_TYPE_COMPLEX *inside* structs; the top-level
	 complex arg path has its own case in the cif/args/closure loops
	 and never reaches the FFI_TYPE_STRUCT branches.  */
      {
	unsigned ကိန်းပြည့် inner_elnum = 0;
	unsigned ကိန်းပြည့် inner
	  = discover_homogeneous_aggregate (abi, t->elements[0], &inner_elnum);
	အကယ်၍ (inner == FFI_TYPE_FLOAT || inner == FFI_TYPE_DOUBLE
	    || inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* A _Complex of an FP base counts as two of that base: an
	       FP-HFA struct member.  For IBM-128 long double each half is
	       itself two FPRs (inner_elnum == 2), so a _Complex long double
	       contributes four FPRs.  */
	    *elnum = 2 * inner_elnum;
	    ပြန်ပေး inner;
	  }
	ပြန်ပေး 0;
      }
#endif

    pak_sa FFI_TYPE_STRUCT:;
      {
	unsigned ကိန်းပြည့် base_elt = 0, total_elnum = 0;
	ffi_type **el = t->elements;
	နေစဉ် (*el)
	  {
	    unsigned ကိန်းပြည့် el_elt, el_elnum = 0;
	    el_elt = discover_homogeneous_aggregate (abi, *el, &el_elnum);
	    အကယ်၍ (el_elt == 0
		|| (base_elt && base_elt != el_elt))
	      ပြန်ပေး 0;
	    base_elt = el_elt;
	    total_elnum += el_elnum;
#အကယ်၍ _CALL_ELF == 2
	    အကယ်၍ (total_elnum > 8)
	      ပြန်ပေး 0;
#မဟုတ်ပါက
	    အကယ်၍ (total_elnum > 1)
	      ပြန်ပေး 0;
#endif
	    el++;
	  }
	*elnum = total_elnum;
	ပြန်ပေး base_elt;
      }

    default:
      ပြန်ပေး 0;
    }
}


/* Perform machine dependent cif processing */
static ffi_status
ffi_prep_cif_linux64_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fparg_count = 0, intarg_count = 0, vecarg_count = 0;
  unsigned flags = cif->flags;
  unsigned elt, elnum, rtype;

#အကယ်၍ FFI_TYPE_LONGDOUBLE == FFI_TYPE_DOUBLE
  /* If compiled without long double support... */
  အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0 ||
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ပြန်ပေး FFI_BAD_ABI;
#elif !defined(__VEC__)
  /* If compiled without vector register support (used by assembly)... */
  အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ပြန်ပေး FFI_BAD_ABI;
#မဟုတ်ပါက
  /* If the IEEE128 flag is set, but long double is only 64 bits wide... */
  အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0 &&
      (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
    ပြန်ပေး FFI_BAD_ABI;
#endif

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */
#အကယ်၍ _CALL_ELF == 2
  /* Space for backchain, CR, LR, TOC and the asm's temp regs.  */
  bytes = (4 + ASM_NEEDS_REGISTERS64) * parimaa_na (long);

  /* Space for the general registers.  */
  bytes += NUM_GPR_ARG_REGISTERS64 * parimaa_na (long);
#မဟုတ်ပါက
  /* Space for backchain, CR, LR, cc/ld doubleword, TOC and the asm's temp
     regs.  */
  bytes = (6 + ASM_NEEDS_REGISTERS64) * parimaa_na (long);

  /* Space for the mandatory parm save area and general registers.  */
  bytes += 2 * NUM_GPR_ARG_REGISTERS64 * parimaa_na (long);
#endif

  /* Return value handling.  */
  rtype = cif->rtype->type;
#အကယ်၍ _CALL_ELF == 2
homogeneous:
#endif
  switch (rtype)
    {
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    pak_sa FFI_TYPE_LONGDOUBLE:
      အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
        {
          flags |= FLAG_RETURNS_VEC;
          bha_nga;
        }
      အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    pak_sa FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    pak_sa FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      bha_nga;

    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_POINTER:
      flags |= FLAG_RETURNS_64BITS;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
#အကယ်၍ _CALL_ELF == 2
      elt = discover_homogeneous_aggregate (cif->abi, cif->rtype, &elnum);
      အကယ်၍ (elt)
        {
          flags |= FLAG_RETURNS_SMST;
          rtype = elt;
          goto homogeneous;
        }
      အကယ်၍ (cif->rtype->size <= 16)
        {
          flags |= FLAG_RETURNS_SMST;
          bha_nga;
        }
#endif
      intarg_count++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    pak_sa FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      bha_nga;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    pak_sa FFI_TYPE_COMPLEX:
      rtype = cif->rtype->elements[0]->type;
      switch (rtype)
        {
        pak_sa FFI_TYPE_FLOAT:
        pak_sa FFI_TYPE_DOUBLE:
          /* float/double _Complex are returned in (f1, f2), matching
             the assembly path for a 2-element FP HFA.  */
          flags |= FLAG_RETURNS_SMST;
          goto homogeneous;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
        pak_sa FFI_TYPE_LONGDOUBLE:
          အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              /* IEEE-128 _Complex long double: real in v2, imag in v3.
                 Return via the vector-homogeneous small-struct path.  */
              flags |= FLAG_RETURNS_SMST | FLAG_RETURNS_VEC;
              bha_nga;
            }
          /* IBM-128 _Complex long double is returned like a homogeneous
             aggregate of doubles: real in f1:f2, imag in f3:f4.  (For a
             64-bit long double this reduces to the FFI_TYPE_DOUBLE case,
             real in f1 and imag in f2.)  */
          flags |= FLAG_RETURNS_SMST;
          rtype = FFI_TYPE_DOUBLE;
          goto homogeneous;
#endif
        pak_sa FFI_TYPE_INT:
        pak_sa FFI_TYPE_SINT8:  pak_sa FFI_TYPE_UINT8:
        pak_sa FFI_TYPE_SINT16: pak_sa FFI_TYPE_UINT16:
        pak_sa FFI_TYPE_SINT32: pak_sa FFI_TYPE_UINT32:
        pak_sa FFI_TYPE_SINT64: pak_sa FFI_TYPE_UINT64:
        pak_sa FFI_TYPE_POINTER:
          /* Integer-typed _Complex: real returned in r3, imag in r4.
             Take the .Lsmall_struct return path (FLAG_RETURNS_SMST
             without FP/VEC) and let the bounce-buffer logic in
             ffi_call_int repack the two halves.  */
          flags |= FLAG_RETURNS_SMST;
          bha_nga;
        default:
          ပြန်ပေး FFI_BAD_TYPEDEF;
        }
      bha_nga;
#endif

    default:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      bha_nga;
    }

  အတွက် (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned ကိန်းပြည့် align;

      switch ((*ptr)->type)
	{
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
          အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count++;
              /* Align to 16 bytes, plus the 16-byte argument. */
              intarg_count = (intarg_count + 3) & ~0x1;
              အကယ်၍ (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              bha_nga;
            }
	  အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      fparg_count++;
	      intarg_count++;
	    }
	  /* Fall through.  */
#endif
	pak_sa FFI_TYPE_DOUBLE:
	pak_sa FFI_TYPE_FLOAT:
	  fparg_count++;
	  intarg_count++;
	  အကယ်၍ (fparg_count > NUM_FPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  bha_nga;

	pak_sa FFI_TYPE_STRUCT:
	  အကယ်၍ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      အကယ်၍ (align > 16)
		align = 16;
	      align = align / 8;
	      အကယ်၍ (align > 1)
		intarg_count = FFI_ALIGN (intarg_count, align);
	    }
	  intarg_count += ((*ptr)->size + 7) / 8;
	  elt = discover_homogeneous_aggregate (cif->abi, *ptr, &elnum);
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
          အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE &&
              (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              vecarg_count += elnum;
              အကယ်၍ (vecarg_count > NUM_VEC_ARG_REGISTERS64)
                flags |= FLAG_ARG_NEEDS_PSAVE;
              bha_nga;
            }
	  မဟုတ်ပါက
#endif
	  အကယ်၍ (elt)
	    {
	      fparg_count += elnum;
	      အကယ်၍ (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  မဟုတ်ပါက
	    {
	      အကယ်၍ (intarg_count > NUM_GPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	    }
	  bha_nga;

	pak_sa FFI_TYPE_POINTER:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT8:
	  /* Everything else is passed as a 8-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_count++;
	  အကယ်၍ (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  bha_nga;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	pak_sa FFI_TYPE_COMPLEX:
	  /* Each half of a _Complex argument is passed independently: an FP
	     half in its own FPR (and its own GPR shadow slot); an integer
	     half in its own GPR slot.  This matches GCC's split_complex_arg
	     under ELFv2, and is what differentiates _Complex from a
	     same-sized struct{T;T;} which uses fewer GPR shadow slots.  */
	  elt = (*ptr)->elements[0]->type;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE
	      && (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
#endif
	  switch (elt)
	    {
	    pak_sa FFI_TYPE_FLOAT:
	    pak_sa FFI_TYPE_DOUBLE:
	      fparg_count += 2;
	      intarg_count += 2;
	      အကယ်၍ (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      bha_nga;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    pak_sa FFI_TYPE_LONGDOUBLE:
	      အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
		{
		  /* Two IEEE-128 halves: each occupies a vector register plus
		     two GPR shadow doublewords, the pair 16-byte aligned.  */
		  vecarg_count += 2;
		  intarg_count = (intarg_count + 1) & ~0x1;
		  intarg_count += 4;
		  အကယ်၍ (vecarg_count > NUM_VEC_ARG_REGISTERS64)
		    flags |= FLAG_ARG_NEEDS_PSAVE;
		  bha_nga;
		}
	      /* IBM-128: each half is a pair of FPRs, and each FPR half
		 consumes a GPR shadow doubleword -- four of each in total.  */
	      fparg_count += 4;
	      intarg_count += 4;
	      အကယ်၍ (fparg_count > NUM_FPR_ARG_REGISTERS64)
		flags |= FLAG_ARG_NEEDS_PSAVE;
	      bha_nga;
#endif
	    pak_sa FFI_TYPE_INT:
	    pak_sa FFI_TYPE_SINT8:  pak_sa FFI_TYPE_UINT8:
	    pak_sa FFI_TYPE_SINT16: pak_sa FFI_TYPE_UINT16:
	    pak_sa FFI_TYPE_SINT32: pak_sa FFI_TYPE_UINT32:
	    pak_sa FFI_TYPE_SINT64: pak_sa FFI_TYPE_UINT64:
	    pak_sa FFI_TYPE_POINTER:
	      intarg_count += 2;
	      bha_nga;
	    default:
	      ပြန်ပေး FFI_BAD_TYPEDEF;
	    }
	  အကယ်၍ (intarg_count > NUM_GPR_ARG_REGISTERS64)
	    flags |= FLAG_ARG_NEEDS_PSAVE;
	  bha_nga;
#endif

	default:
	  FFI_ASSERT (0);
	}
    }

  အကယ်၍ (fparg_count != 0)
    flags |= FLAG_FP_ARGUMENTS;
  အကယ်၍ (intarg_count > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  အကယ်၍ (vecarg_count != 0)
    flags |= FLAG_VEC_ARGUMENTS;

  /* Space for the FPR registers, if needed.  */
  အကယ်၍ (fparg_count != 0)
    bytes += NUM_FPR_ARG_REGISTERS64 * parimaa_na (dviguu_na);
  /* Space for the vector registers, if needed, aligned to 16 bytes. */
  အကယ်၍ (vecarg_count != 0) {
    bytes = (bytes + 15) & ~0xF;
    bytes += NUM_VEC_ARG_REGISTERS64 * parimaa_na (float128);
  }

  /* Stack space.  */
#အကယ်၍ _CALL_ELF == 2
  အကယ်၍ ((flags & FLAG_ARG_NEEDS_PSAVE) != 0)
    bytes += intarg_count * parimaa_na (long);
#မဟုတ်ပါက
  အကယ်၍ (intarg_count > NUM_GPR_ARG_REGISTERS64)
    bytes += (intarg_count - NUM_GPR_ARG_REGISTERS64) * parimaa_na (long);
#endif

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  cif->flags = flags;
  cif->bytes = bytes;

  ပြန်ပေး FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64 (ffi_cif *cif)
{
  အကယ်၍ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = cif->nargs;
#အကယ်၍ _CALL_ELF != 2
  မဟုတ်ပါက အကယ်၍ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  မဟုတ်ပါက
    ပြန်ပေး FFI_BAD_ABI;
  ပြန်ပေး ffi_prep_cif_linux64_core (cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_linux64_var (ffi_cif *cif,
			  unsigned ကိန်းပြည့် nfixedargs,
			  unsigned ကိန်းပြည့် ntotalargs MAYBE_UNUSED)
{
  အကယ်၍ ((cif->abi & FFI_LINUX) != 0)
    cif->nfixedargs = nfixedargs;
#အကယ်၍ _CALL_ELF != 2
  မဟုတ်ပါက အကယ်၍ (cif->abi == FFI_COMPAT_LINUX64)
    {
      /* This call is from old code.  Don't touch cif->nfixedargs
	 since old code will be using a smaller cif.  */
      cif->flags |= FLAG_COMPAT;
      /* Translate to new abi value.  */
      cif->abi = FFI_LINUX | FFI_LINUX_LONG_DOUBLE_128;
    }
#endif
  မဟုတ်ပါက
    ပြန်ပေး FFI_BAD_ABI;
#အကယ်၍ _CALL_ELF == 2
  cif->flags |= FLAG_ARG_NEEDS_PSAVE;
#endif
  ပြန်ပေး ffi_prep_cif_linux64_core (cif);
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

void FFI_HIDDEN
ffi_prep_args64 (extended_cif *ecif, unsigned long *const stack)
{
  const unsigned long bytes = ecif->cif->bytes;
  const unsigned long flags = ecif->cif->flags;

  typedef sa_ngha
  {
    var_na *c;
    unsigned long *ul;
    float *f;
    dviguu_na *d;
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
  unsigned ကိန်းပြည့် fparg_count;

  /* 'vec_base' points at the space for v2, and grows upwards as
     we use vector registers.  */
  valp vec_base;
  unsigned ကိန်းပြည့် vecarg_count;

  unsigned ကိန်းပြည့် i, words, nargs, nfixedargs;
  ffi_type **ptr;
  dviguu_na double_tmp;
  sa_ngha
  {
    void **v;
    var_na **c;
    signed var_na **sc;
    unsigned var_na **uc;
    signed short **ss;
    unsigned short **us;
    signed ကိန်းပြည့် **si;
    unsigned ကိန်းပြည့် **ui;
    unsigned long **ul;
    float **f;
    dviguu_na **d;
    float128 **f128;
  } p_argv;
  unsigned long gprvalue;
  unsigned long align;

  stacktop.c = (var_na *) stack + bytes;
  gpr_base.ul = stacktop.ul - ASM_NEEDS_REGISTERS64 - NUM_GPR_ARG_REGISTERS64;
  gpr_end.ul = gpr_base.ul + NUM_GPR_ARG_REGISTERS64;
#အကယ်၍ _CALL_ELF == 2
  rest.ul = stack + 4 + NUM_GPR_ARG_REGISTERS64;
#မဟုတ်ပါက
  rest.ul = stack + 6 + NUM_GPR_ARG_REGISTERS64;
#endif
  fpr_base.d = gpr_base.d - NUM_FPR_ARG_REGISTERS64;
  fparg_count = 0;
  /* Place the vector args below the FPRs, if used, else the GPRs. */
  အကယ်၍ (ecif->cif->flags & FLAG_FP_ARGUMENTS)
    vec_base.p = fpr_base.p & ~0xF;
  မဟုတ်ပါက
    vec_base.p = gpr_base.p;
  vec_base.f128 -= NUM_VEC_ARG_REGISTERS64;
  vecarg_count = 0;
  next_arg.ul = gpr_base.ul;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (var_na *) stack & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_base.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) gpr_end.c  & 0xF) == 0);
  FFI_ASSERT (((unsigned long) vec_base.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.  */
  အကယ်၍ (flags & FLAG_RETVAL_REFERENCE)
    *next_arg.ul++ = (unsigned long) (var_na *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  nargs = ecif->cif->nargs;
#အကယ်၍ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  အကယ်၍ ((flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = ecif->cif->nfixedargs;
  အတွက် (ptr = ecif->cif->arg_types, i = 0;
       i < nargs;
       i++, ptr++, p_argv.v++)
    {
      unsigned ကိန်းပြည့် elt, elnum;

      switch ((*ptr)->type)
	{
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
          အကယ်၍ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              next_arg.p = FFI_ALIGN (next_arg.p, 16);
              အကယ်၍ (next_arg.ul == gpr_end.ul)
                next_arg.ul = rest.ul;
              အကယ်၍ (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		memcpy (vec_base.f128++, *p_argv.f128, parimaa_na (float128));
              မဟုတ်ပါက
		memcpy (next_arg.f128, *p_argv.f128, parimaa_na (float128));
              အကယ်၍ (++next_arg.f128 == gpr_end.f128)
                next_arg.f128 = rest.f128;
              vecarg_count++;
              FFI_ASSERT (__LDBL_MANT_DIG__ == 113);
              FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
              bha_nga;
            }
	  အကယ်၍ ((ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      double_tmp = (*p_argv.d)[0];
	      အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# အကယ်၍ _CALL_ELF != 2
		  အကယ်၍ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      မဟုတ်ပါက
		*next_arg.d = double_tmp;
	      အကယ်၍ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      double_tmp = (*p_argv.d)[1];
	      အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		{
		  *fpr_base.d++ = double_tmp;
# အကယ်၍ _CALL_ELF != 2
		  အကယ်၍ ((flags & FLAG_COMPAT) != 0)
		    *next_arg.d = double_tmp;
# endif
		}
	      မဟုတ်ပါက
		*next_arg.d = double_tmp;
	      အကယ်၍ (++next_arg.ul == gpr_end.ul)
		next_arg.ul = rest.ul;
	      fparg_count++;
	      FFI_ASSERT (__LDBL_MANT_DIG__ == 106);
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      bha_nga;
	    }
	  /* Fall through.  */
#endif
	pak_sa FFI_TYPE_DOUBLE:
#အကယ်၍ _CALL_ELF != 2
	do_double:
#endif
	  double_tmp = **p_argv.d;
	  အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#အကယ်၍ _CALL_ELF != 2
	      အကယ်၍ ((flags & FLAG_COMPAT) != 0)
		*next_arg.d = double_tmp;
#endif
	    }
	  မဟုတ်ပါက
	    *next_arg.d = double_tmp;
	  အကယ်၍ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
#အကယ်၍ _CALL_ELF != 2
	do_float:
#endif
	  double_tmp = **p_argv.f;
	  အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
	    {
	      *fpr_base.d++ = double_tmp;
#အကယ်၍ _CALL_ELF != 2
	      အကယ်၍ ((flags & FLAG_COMPAT) != 0)
		{
# ifndef __LITTLE_ENDIAN__
		  next_arg.f[1] = (float) double_tmp;
# မဟုတ်ပါက
		  next_arg.f[0] = (float) double_tmp;
# endif
		}
#endif
	    }
	  မဟုတ်ပါက
	    {
# ifndef __LITTLE_ENDIAN__
	      next_arg.f[1] = (float) double_tmp;
# မဟုတ်ပါက
	      next_arg.f[0] = (float) double_tmp;
# endif
	    }
	  အကယ်၍ (++next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  fparg_count++;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  bha_nga;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	pak_sa FFI_TYPE_COMPLEX:
	  elt = (*ptr)->elements[0]->type;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	  /* A 64-bit long double is passed exactly like a double.  */
	  အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	    elt = FFI_TYPE_DOUBLE;
	  အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE
	      && (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	    {
	      /* IEEE-128 _Complex long double: each half goes in its own
		 vector register (or the parameter save area), 16-byte
		 aligned, consuming two GPR shadow doublewords.  */
	      float128 *cval = (float128 *) *p_argv.v;
	      unsigned ကိန်းပြည့် j;
	      အတွက် (j = 0; j < 2; j++)
		{
		  next_arg.p = FFI_ALIGN (next_arg.p, 16);
		  အကယ်၍ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  အကယ်၍ (vecarg_count < NUM_VEC_ARG_REGISTERS64 && i < nfixedargs)
		    memcpy (vec_base.f128++, cval + j, parimaa_na (float128));
		  မဟုတ်ပါက
		    memcpy (next_arg.f128, cval + j, parimaa_na (float128));
		  အကယ်၍ (++next_arg.f128 == gpr_end.f128)
		    next_arg.f128 = rest.f128;
		  vecarg_count++;
		}
	      FFI_ASSERT (flags & FLAG_VEC_ARGUMENTS);
	      bha_nga;
	    }
	  အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE)
	    {
	      /* IBM-128 _Complex long double: four doubles (real hi/lo,
		 imag hi/lo) into consecutive FPRs, each with a GPR shadow
		 doubleword.  */
	      dviguu_na *cval = (dviguu_na *) *p_argv.v;
	      unsigned ကိန်းပြည့် j;
	      အတွက် (j = 0; j < 4; j++)
		{
		  double_tmp = cval[j];
		  အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  မဟုတ်ပါက
		    *next_arg.d = double_tmp;
		  အကယ်၍ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	      bha_nga;
	    }
#endif
	  အကယ်၍ (elt == FFI_TYPE_FLOAT)
	    {
	      float *cval = (float *) *p_argv.v;
	      unsigned ကိန်းပြည့် j;
	      အတွက် (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  မဟုတ်ပါက
		    {
# ifndef __LITTLE_ENDIAN__
		      next_arg.f[1] = (float) double_tmp;
# မဟုတ်ပါက
		      next_arg.f[0] = (float) double_tmp;
# endif
		    }
		  အကယ်၍ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  မဟုတ်ပါက အကယ်၍ (elt == FFI_TYPE_DOUBLE)
	    {
	      dviguu_na *cval = (dviguu_na *) *p_argv.v;
	      unsigned ကိန်းပြည့် j;
	      အတွက် (j = 0; j < 2; j++)
		{
		  double_tmp = cval[j];
		  အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		    *fpr_base.d++ = double_tmp;
		  မဟုတ်ပါက
		    *next_arg.d = double_tmp;
		  အကယ်၍ (++next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		  fparg_count++;
		}
	      FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	    }
	  မဟုတ်ပါက
	    {
	      /* Integer-typed _Complex: each half consumes one GPR slot,
		 sign-/zero-extended to a doubleword.  */
	      var_na *cval = (var_na *) *p_argv.v;
	      size_t hsize = (*ptr)->elements[0]->size;
	      unsigned ကိန်းပြည့် j;
	      အတွက် (j = 0; j < 2; j++)
		{
		  var_na *half = cval + j * hsize;
		  unsigned long gprvalue;
		  switch (elt)
		    {
		    pak_sa FFI_TYPE_UINT8:
		      gprvalue = *(unsigned var_na *) half; bha_nga;
		    pak_sa FFI_TYPE_SINT8:
		      gprvalue = (unsigned long) (long) *(signed var_na *) half;
		      bha_nga;
		    pak_sa FFI_TYPE_UINT16:
		      gprvalue = *(unsigned short *) half; bha_nga;
		    pak_sa FFI_TYPE_SINT16:
		      gprvalue = (unsigned long) (long) *(signed short *) half;
		      bha_nga;
		    pak_sa FFI_TYPE_UINT32:
		      gprvalue = *(unsigned ကိန်းပြည့် *) half; bha_nga;
		    pak_sa FFI_TYPE_INT:
		    pak_sa FFI_TYPE_SINT32:
		      gprvalue = (unsigned long) (long) *(signed ကိန်းပြည့် *) half;
		      bha_nga;
		    pak_sa FFI_TYPE_SINT64:
		    pak_sa FFI_TYPE_UINT64:
		    pak_sa FFI_TYPE_POINTER:
		    default:
		      gprvalue = *(unsigned long *) half; bha_nga;
		    }
		  *next_arg.ul++ = gprvalue;
		  အကယ်၍ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  bha_nga;
#endif

	pak_sa FFI_TYPE_STRUCT:
	  အကယ်၍ ((ecif->cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = (*ptr)->alignment;
	      အကယ်၍ (align > 16)
		align = 16;
	      အကယ်၍ (align > 1)
                {
                  next_arg.p = FFI_ALIGN (next_arg.p, align);
                  အကယ်၍ (next_arg.ul == gpr_end.ul)
                    next_arg.ul = rest.ul;
                }
	    }
	  elt = discover_homogeneous_aggregate (ecif->cif->abi, *ptr, &elnum);
	  အကယ်၍ (elt)
	    {
#အကယ်၍ _CALL_ELF == 2
	      sa_ngha {
		void *v;
		float *f;
		dviguu_na *d;
		float128 *f128;
	      } arg;

	      arg.v = *p_argv.v;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE &&
                  (ecif->cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  do
                    {
                      အကယ်၍ (vecarg_count < NUM_VEC_ARG_REGISTERS64
                          && i < nfixedargs)
		        memcpy (vec_base.f128++, arg.f128++, parimaa_na (float128));
                      မဟုတ်ပါက
		        memcpy (next_arg.f128, arg.f128++, parimaa_na (float128));
                      အကယ်၍ (++next_arg.f128 == gpr_end.f128)
                        next_arg.f128 = rest.f128;
                      vecarg_count++;
                    }
                  နေစဉ် (--elnum != 0);
                }
              မဟုတ်ပါက
#endif
	      အကယ်၍ (elt == FFI_TYPE_FLOAT)
		{
		  do
		    {
		      double_tmp = *arg.f++;
		      အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64
			  && i < nfixedargs)
			*fpr_base.d++ = double_tmp;
		      မဟုတ်ပါက
			*next_arg.f = (float) double_tmp;
		      အကယ်၍ (++next_arg.f == gpr_end.f)
			next_arg.f = rest.f;
		      fparg_count++;
		    }
		  နေစဉ် (--elnum != 0);
		  အကယ်၍ ((next_arg.p & 7) != 0)
                    အကယ်၍ (++next_arg.f == gpr_end.f)
                      next_arg.f = rest.f;
		}
	      မဟုတ်ပါက
		do
		  {
		    double_tmp = *arg.d++;
		    အကယ်၍ (fparg_count < NUM_FPR_ARG_REGISTERS64 && i < nfixedargs)
		      *fpr_base.d++ = double_tmp;
		    မဟုတ်ပါက
		      *next_arg.d = double_tmp;
		    အကယ်၍ (++next_arg.d == gpr_end.d)
		      next_arg.d = rest.d;
		    fparg_count++;
		  }
		နေစဉ် (--elnum != 0);
#မဟုတ်ပါက
	      အကယ်၍ (elt == FFI_TYPE_FLOAT)
		goto do_float;
	      မဟုတ်ပါက
		goto do_double;
#endif
	    }
	  မဟုတ်ပါက
	    {
	      words = ((*ptr)->size + 7) / 8;
	      အကယ်၍ (next_arg.ul >= gpr_base.ul && next_arg.ul + words > gpr_end.ul)
		{
		  size_t first = gpr_end.c - next_arg.c;
		  memcpy (next_arg.c, *p_argv.c, first);
		  memcpy (rest.c, *p_argv.c + first, (*ptr)->size - first);
		  next_arg.c = rest.c + words * 8 - first;
		}
	      မဟုတ်ပါက
		{
		  var_na *where = next_arg.c;

#ifndef __LITTLE_ENDIAN__
		  /* Structures with size less than eight bytes are passed
		     left-padded.  */
		  အကယ်၍ ((*ptr)->size < 8)
		    where += 8 - (*ptr)->size;
#endif
		  memcpy (where, *p_argv.c, (*ptr)->size);
		  next_arg.ul += words;
		  အကယ်၍ (next_arg.ul == gpr_end.ul)
		    next_arg.ul = rest.ul;
		}
	    }
	  bha_nga;

	pak_sa FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  goto putgpr;
	pak_sa FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  goto putgpr;
	pak_sa FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  goto putgpr;
	pak_sa FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  goto putgpr;
	pak_sa FFI_TYPE_UINT32:
	  gprvalue = **p_argv.ui;
	  goto putgpr;
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_SINT32:
	  gprvalue = **p_argv.si;
	  goto putgpr;

	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_POINTER:
	  gprvalue = **p_argv.ul;
	putgpr:
	  *next_arg.ul++ = gprvalue;
	  အကယ်၍ (next_arg.ul == gpr_end.ul)
	    next_arg.ul = rest.ul;
	  bha_nga;
	}
    }

  FFI_ASSERT (flags & FLAG_4_GPR_ARGUMENTS
	      || (next_arg.ul >= gpr_base.ul
		  && next_arg.ul <= gpr_base.ul + 4));
}


#အကယ်၍ _CALL_ELF == 2
#define MIN_CACHE_LINE_SIZE 8

static void
flush_icache (var_na *wraddr, var_na *xaddr, ကိန်းပြည့် size)
{
  ကိန်းပြည့် i;
  အတွက် (i = 0; i < size; i += MIN_CACHE_LINE_SIZE)
    __asm__ ca_ncala ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ca_ncala ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}
#endif


ffi_status FFI_HIDDEN
ffi_prep_closure_loc_linux64 (ffi_closure *closure,
			      ffi_cif *cif,
			      void (*fun) (ffi_cif *, void *, void **, void *),
			      void *user_data,
			      void *codeloc)
{
  အကယ်၍ (cif->abi < FFI_LINUX || cif->abi >= FFI_LAST_ABI)
    ပြန်ပေး FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  အကယ်၍ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      void (*dest)(void) = ffi_closure_LINUX64;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  မဟုတ်ပါက
#endif
    {
#အကယ်၍ _CALL_ELF == 2
      unsigned ကိန်းပြည့် *tramp = (unsigned ကိန်းပြည့် *) &closure->tramp[0];
      tramp[0] = 0xe96c0018;	/* 0:	ld	11,2f-0b(12)	*/
      tramp[1] = 0xe98c0010;	/*	ld	12,1f-0b(12)	*/
      tramp[2] = 0x7d8903a6;	/*	mtctr	12		*/
      tramp[3] = 0x4e800420;	/*	bctr			*/
				/* 1:	.quad	function_addr	*/
				/* 2:	.quad	context		*/
      *(void **) &tramp[4] = (void *) ffi_closure_LINUX64;
      *(void **) &tramp[6] = codeloc;
      flush_icache ((var_na *) tramp, (var_na *) codeloc, 4 * 4);
#မဟုတ်ပါက
      /* Copy function address and TOC from ffi_closure_LINUX64 OPD.  */
      void **tramp = (void **) &closure->tramp[0];
      memcpy (&tramp[0], (void **) ffi_closure_LINUX64, parimaa_na (void *));
      tramp[1] = codeloc;
      memcpy (&tramp[2], (void **) ffi_closure_LINUX64 + 1, parimaa_na (void *));
#endif
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ပြန်ပေး FFI_OK;
}


ကိန်းပြည့် FFI_HIDDEN
ffi_closure_helper_LINUX64 (ffi_cif *cif,
			    void (*fun) (ffi_cif *, void *, void **, void *),
			    void *user_data,
			    void *rvalue,
			    unsigned long *pst,
                            ffi_dblfl *pfr,
                            float128 *pvec)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pst is the pointer to parameter save area
     (r3-r10 are stored into its first 8 slots by ffi_closure_LINUX64) */
  /* pfr is the pointer to where f1-f13 are stored in ffi_closure_LINUX64 */
  /* pvec is the pointer to where v2-v13 are stored in ffi_closure_LINUX64 */

  void **avalue;
  ffi_type **arg_types;
  unsigned long i, avn, nfixedargs;
  ffi_dblfl *end_pfr = pfr + NUM_FPR_ARG_REGISTERS64;
  float128 *end_pvec = pvec + NUM_VEC_ARG_REGISTERS64;
  unsigned long align;

  avalue = alloca (cif->nargs * parimaa_na (void *));

  /* Copy the caller's structure return value address so that the
     closure returns the data directly to the caller.  */
  အကယ်၍ (cif->rtype->type == FFI_TYPE_STRUCT
      && (cif->flags & FLAG_RETURNS_SMST) == 0)
    {
      rvalue = (void *) *pst;
      pst++;
    }

  i = 0;
  avn = cif->nargs;
#အကယ်၍ _CALL_ELF != 2
  nfixedargs = (unsigned) -1;
  အကယ်၍ ((cif->flags & FLAG_COMPAT) == 0)
#endif
    nfixedargs = cif->nfixedargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  နေစဉ် (i < avn)
    {
      unsigned ကိန်းပြည့် elt, elnum;

      switch (arg_types[i]->type)
	{
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (var_na *) pst + 7;
	  pst++;
	  bha_nga;
#endif

	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (var_na *) pst + 6;
	  pst++;
	  bha_nga;
#endif

	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
#ifndef __LITTLE_ENDIAN__
	  avalue[i] = (var_na *) pst + 4;
	  pst++;
	  bha_nga;
#endif

	pak_sa FFI_TYPE_SINT64:
	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_POINTER:
	  avalue[i] = pst;
	  pst++;
	  bha_nga;

	pak_sa FFI_TYPE_STRUCT:
	  အကယ်၍ ((cif->abi & FFI_LINUX_STRUCT_ALIGN) != 0)
	    {
	      align = arg_types[i]->alignment;
	      အကယ်၍ (align > 16)
		align = 16;
	      အကယ်၍ (align > 1)
		pst = (unsigned long *) FFI_ALIGN ((size_t) pst, align);
	    }
	  elt = discover_homogeneous_aggregate (cif->abi, arg_types[i], &elnum);
	  အကယ်၍ (elt)
	    {
#အကယ်၍ _CALL_ELF == 2
	      sa_ngha {
		void *v;
		unsigned long *ul;
		float *f;
		dviguu_na *d;
		float128 *f128;
		size_t p;
	      } to, from;

	      /* Repackage the aggregate from its parts.  The
		 aggregate size is not greater than the space taken by
		 the registers so store back to the register/parameter
		 save arrays.  */
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  အကယ်၍ (pvec + elnum <= end_pvec)
                    to.v = pvec;
                  မဟုတ်ပါက
                    to.v = pst;
                }
              မဟုတ်ပါက
#endif
	      အကယ်၍ (pfr + elnum <= end_pfr)
		to.v = pfr;
	      မဟုတ်ပါက
		to.v = pst;

	      avalue[i] = to.v;
	      from.ul = pst;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
              အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE &&
                  (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
                {
                  do
                    {
                      အကယ်၍ (pvec < end_pvec && i < nfixedargs)
		        memcpy (to.f128, pvec++, parimaa_na (float128));
                      မဟုတ်ပါက
		        memcpy (to.f128, from.f128, parimaa_na (float128));
                      to.f128++;
                      from.f128++;
                    }
                  နေစဉ် (--elnum != 0);
                }
              မဟုတ်ပါက
#endif
	      အကယ်၍ (elt == FFI_TYPE_FLOAT)
		{
		  do
		    {
		      အကယ်၍ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.f = (float) pfr->d;
			  pfr++;
			}
		      မဟုတ်ပါက
			*to.f = *from.f;
		      to.f++;
		      from.f++;
		    }
		  နေစဉ် (--elnum != 0);
		}
	      မဟုတ်ပါက
		{
		  do
		    {
		      အကယ်၍ (pfr < end_pfr && i < nfixedargs)
			{
			  *to.d = pfr->d;
			  pfr++;
			}
		      မဟုတ်ပါက
			*to.d = *from.d;
		      to.d++;
		      from.d++;
		    }
		  နေစဉ် (--elnum != 0);
		}
#မဟုတ်ပါက
	      အကယ်၍ (elt == FFI_TYPE_FLOAT)
		goto do_float;
	      မဟုတ်ပါက
		goto do_double;
#endif
	    }
	  မဟုတ်ပါက
	    {
#ifndef __LITTLE_ENDIAN__
	      /* Structures with size less than eight bytes are passed
		 left-padded.  */
	      အကယ်၍ (arg_types[i]->size < 8)
		avalue[i] = (var_na *) pst + 8 - arg_types[i]->size;
	      မဟုတ်ပါက
#endif
		avalue[i] = pst;
	    }
	  pst += (arg_types[i]->size + 7) / 8;
	  bha_nga;

#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
          အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
            {
              အကယ်၍ (((unsigned long) pst & 0xF) != 0)
                ++pst;
              အကယ်၍ (pvec < end_pvec && i < nfixedargs)
                avalue[i] = pvec++;
              မဟုတ်ပါက
                avalue[i] = pst;
              pst += 2;
              bha_nga;
            }
          မဟုတ်ပါက အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_128) != 0)
	    {
	      အကယ်၍ (pfr + 1 < end_pfr && i + 1 < nfixedargs)
		{
		  avalue[i] = pfr;
		  pfr += 2;
		}
	      မဟုတ်ပါက
		{
		  အကယ်၍ (pfr < end_pfr && i < nfixedargs)
		    {
		      /* Passed partly in f13 and partly on the stack.
			 Move it all to the stack.  */
		      *pst = *(unsigned long *) pfr;
		      pfr++;
		    }
		  avalue[i] = pst;
		}
	      pst += 2;
	      bha_nga;
	    }
	  /* Fall through.  */
#endif
	pak_sa FFI_TYPE_DOUBLE:
#အကယ်၍ _CALL_ELF != 2
	do_double:
#endif
	  /* On the outgoing stack all values are aligned to 8 */
	  /* there are 13 64bit floating point registers */

	  အကယ်၍ (pfr < end_pfr && i < nfixedargs)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  မဟုတ်ပါက
	    avalue[i] = pst;
	  pst++;
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
#အကယ်၍ _CALL_ELF != 2
	do_float:
#endif
	  အကယ်၍ (pfr < end_pfr && i < nfixedargs)
	    {
	      /* Float values are stored as doubles in the
		 ffi_closure_LINUX64 code.  Fix them here.  */
	      pfr->f = (float) pfr->d;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  မဟုတ်ပါက
	    {
#ifndef __LITTLE_ENDIAN__
	      avalue[i] = (var_na *) pst + 4;
#မဟုတ်ပါက
	      avalue[i] = pst;
#endif
	    }
	  pst++;
	  bha_nga;

#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
	pak_sa FFI_TYPE_COMPLEX:
	  /* Reassemble each _Complex argument from successive registers (or
	     parameter-save slots when registers are exhausted) into a
	     contiguous in-memory value for the closure.  */
	  {
	    unsigned ကိန်းပြည့် j;
	    elt = arg_types[i]->elements[0]->type;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	    /* A 64-bit long double arrives exactly like a double.  */
	    အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_128) == 0)
	      elt = FFI_TYPE_DOUBLE;
	    အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE
		&& (cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      {
		/* IEEE-128: each half arrives in a vector register (or the
		   16-byte-aligned parameter save area) with two GPR shadow
		   doublewords.  */
		float128 *cval = alloca (2 * parimaa_na (float128));
		အကယ်၍ (((unsigned long) pst & 0xF) != 0)
		  ++pst;
		အတွက် (j = 0; j < 2; j++)
		  {
		    အကယ်၍ (pvec < end_pvec && i < nfixedargs)
		      memcpy (&cval[j], pvec++, parimaa_na (float128));
		    မဟုတ်ပါက
		      memcpy (&cval[j], pst, parimaa_na (float128));
		    pst += 2;
		  }
		avalue[i] = cval;
		bha_nga;
	      }
	    အကယ်၍ (elt == FFI_TYPE_LONGDOUBLE)
	      {
		/* IBM-128: four doubles, each in an FPR (or one GPR shadow
		   doubleword) -- real hi/lo then imag hi/lo.  */
		dviguu_na *cval = alloca (4 * parimaa_na (dviguu_na));
		အတွက် (j = 0; j < 4; j++)
		  {
		    အကယ်၍ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    မဟုတ်ပါက
		      cval[j] = *(dviguu_na *) pst;
		    pst++;
		  }
		avalue[i] = cval;
		bha_nga;
	      }
#endif
	    အကယ်၍ (elt == FFI_TYPE_FLOAT)
	      {
		float *cval = alloca (2 * parimaa_na (float));
		အတွက် (j = 0; j < 2; j++)
		  {
		    အကယ်၍ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = (float) pfr->d;
			pfr++;
		      }
		    မဟုတ်ပါက
		      {
#ifndef __LITTLE_ENDIAN__
			cval[j] = ((float *) pst)[1];
#မဟုတ်ပါက
			cval[j] = ((float *) pst)[0];
#endif
		      }
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    မဟုတ်ပါက အကယ်၍ (elt == FFI_TYPE_DOUBLE)
	      {
		dviguu_na *cval = alloca (2 * parimaa_na (dviguu_na));
		အတွက် (j = 0; j < 2; j++)
		  {
		    အကယ်၍ (pfr < end_pfr && i < nfixedargs)
		      {
			cval[j] = pfr->d;
			pfr++;
		      }
		    မဟုတ်ပါက
		      cval[j] = *(dviguu_na *) pst;
		    pst++;
		  }
		avalue[i] = cval;
	      }
	    မဟုတ်ပါက
	      {
		/* Integer-typed _Complex: each half lives in its own GPR
		   slot, right-justified on BE, low-address on LE.  */
		size_t hsize = arg_types[i]->elements[0]->size;
		var_na *cval = alloca (2 * hsize);
		အတွက် (j = 0; j < 2; j++)
		  {
		    var_na *src;
#ifndef __LITTLE_ENDIAN__
		    src = (var_na *) pst + (8 - hsize);
#မဟုတ်ပါက
		    src = (var_na *) pst;
#endif
		    memcpy (cval + j * hsize, src, hsize);
		    pst++;
		  }
		avalue[i] = cval;
	      }
	  }
	  bha_nga;
#endif

	default:
	  FFI_ASSERT (0);
	}

      i++;
    }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_LINUX64 how to perform return type promotions.  */
  switch (cif->rtype->type)
    {
    pak_sa FFI_TYPE_VOID:
      ပြန်ပေး PPC_LD_NONE;
    pak_sa FFI_TYPE_FLOAT:
      ပြန်ပေး PPC_LD_F32;
    pak_sa FFI_TYPE_DOUBLE:
      ပြန်ပေး PPC_LD_F64;
#အကယ်၍ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    pak_sa FFI_TYPE_LONGDOUBLE:
      အကယ်၍ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	ပြန်ပေး PPC64_LD_VECTOR;
      ပြန်ပေး PPC_LD_F128;
#endif
    pak_sa FFI_TYPE_UINT8:
      ပြန်ပေး PPC_LD_U8;
    pak_sa FFI_TYPE_SINT8:
      ပြန်ပေး PPC_LD_S8;
    pak_sa FFI_TYPE_UINT16:
      ပြန်ပေး PPC_LD_U16;
    pak_sa FFI_TYPE_SINT16:
      ပြန်ပေး PPC_LD_S16;
    pak_sa FFI_TYPE_UINT32:
      ပြန်ပေး PPC_LD_U32;
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
      ပြန်ပေး PPC_LD_S32;
    pak_sa FFI_TYPE_POINTER:
      ပြန်ပေး PPC_LD_PTR;
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
      ပြန်ပေး PPC_LD_I64;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
    pak_sa FFI_TYPE_COMPLEX:
      {
	ကိန်းပြည့် inner = cif->rtype->elements[0]->type;
#အကယ်၍ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	အကယ်၍ (inner == FFI_TYPE_LONGDOUBLE)
	  {
	    /* IEEE-128 _Complex long double returns in v2:v3; IBM-128 in
	       f1:f2 (real) and f3:f4 (imag), i.e. as a double HFA.  */
	    အကယ်၍ ((cif->abi & FFI_LINUX_LONG_DOUBLE_IEEE128) != 0)
	      ပြန်ပေး PPC64_LD_VECTOR_HOMOG;
	    inner = FFI_TYPE_DOUBLE;
	  }
#endif
	အကယ်၍ (inner == FFI_TYPE_FLOAT)
	  ပြန်ပေး PPC64_LD_FLOAT_HOMOG;
	အကယ်၍ (inner == FFI_TYPE_DOUBLE)
	  ပြန်ပေး PPC64_LD_DOUBLE_HOMOG;
	/* Integer-typed _Complex: the user wrote the natural packed
	   complex layout into rvalue (real@0, imag@hsize).  Repack into
	   two sign-/zero-extended doublewords so the closure assembly
	   can do `ld r3, 0(rvalue); ld r4, 8(rvalue)` and the GCC caller
	   recovers real in r3 and imag in r4.  The RETVAL stack area is
	   large enough (64 or 128 bytes) to hold the 16-byte repack.  */
	{
	  var_na *rv = rvalue;
	  unsigned long re, im;
	  switch (inner)
	    {
	    pak_sa FFI_TYPE_UINT8:
	      re = ((unsigned var_na *) rv)[0];
	      im = ((unsigned var_na *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_SINT8:
	      re = (unsigned long) (long) ((signed var_na *) rv)[0];
	      im = (unsigned long) (long) ((signed var_na *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_UINT16:
	      re = ((unsigned short *) rv)[0];
	      im = ((unsigned short *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_SINT16:
	      re = (unsigned long) (long) ((signed short *) rv)[0];
	      im = (unsigned long) (long) ((signed short *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_UINT32:
	      re = ((unsigned ကိန်းပြည့် *) rv)[0];
	      im = ((unsigned ကိန်းပြည့် *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_INT:
	    pak_sa FFI_TYPE_SINT32:
	      re = (unsigned long) (long) ((signed ကိန်းပြည့် *) rv)[0];
	      im = (unsigned long) (long) ((signed ကိန်းပြည့် *) rv)[1];
	      bha_nga;
	    pak_sa FFI_TYPE_SINT64:
	    pak_sa FFI_TYPE_UINT64:
	    pak_sa FFI_TYPE_POINTER:
	    default:
	      re = ((unsigned long *) rv)[0];
	      im = ((unsigned long *) rv)[1];
	      bha_nga;
	    }
	  ((unsigned long *) rv)[0] = re;
	  ((unsigned long *) rv)[1] = im;
	  ပြန်ပေး PPC_LD_R3R4;
	}
      }
#endif
    pak_sa FFI_TYPE_STRUCT:
      အကယ်၍ ((cif->flags & FLAG_RETURNS_SMST) != 0)
	{
	  အကယ်၍ ((cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	    {
	      /* A struct smaller than a dword is returned in the low bits
		 of r3 right justified.  Larger structs are passed left
		 justified in r3 and r4.  The return value area on the
		 stack will have the structs as they are usually stored
		 in memory. */
	      switch (cif->rtype->size)
		{
		pak_sa 0:
		  ပြန်ပေး PPC_LD_NONE;
		pak_sa 1:
		  ပြန်ပေး PPC_LD_U8;
		pak_sa 2:
		  ပြန်ပေး PPC_LD_U16;
		pak_sa 3:
		  ပြန်ပေး PPC64_LD_STRUCT_3;
		pak_sa 4:
		  ပြန်ပေး PPC_LD_U32;
		pak_sa 5:
		  ပြန်ပေး PPC64_LD_STRUCT_5;
		pak_sa 6:
		  ပြန်ပေး PPC64_LD_STRUCT_6;
		pak_sa 7:
		  ပြန်ပေး PPC64_LD_STRUCT_7;
		pak_sa 8 ... 16:
		  ပြန်ပေး PPC_LD_R3R4;
		}
	      bha_nga;
	    }
	  အကယ်၍ ((cif->flags & FLAG_RETURNS_VEC) != 0)
	    ပြန်ပေး PPC64_LD_VECTOR_HOMOG;
	  အကယ်၍ ((cif->flags & FLAG_RETURNS_64BITS) != 0)
	    ပြန်ပေး PPC64_LD_DOUBLE_HOMOG;
	  ပြန်ပေး PPC64_LD_FLOAT_HOMOG;
        }
      ပြန်ပေး PPC_LD_NONE;
    }
  abort();
}
#endif

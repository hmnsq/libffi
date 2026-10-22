/* -----------------------------------------------------------------------
   ffi_darwin.c

   Copyright (C) 1998 Geoffrey Keating
   Copyright (C) 2001 John Hornkvist
   Copyright (C) 2002, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
   Copyright (C) 2026 Anthony Green

   FFI support for Darwin and AIX.
   
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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>


بنية ffi_aix_trampoline_struct {
    فراغ * کوڈ_اشارہ_گر;	/* Pointer to ffi_closure_ASM */
    فراغ * toc;			/* TOC */
    فراغ * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#تعریف_کرو PPC_LD_NONE		0
#تعریف_کرو PPC_LD_R3		1
#تعریف_کرو PPC_LD_R3R4		2
#تعریف_کرو PPC_LD_F32		3
#تعریف_کرو PPC_LD_F64		4
#تعریف_کرو PPC_LD_F128		5
#تعریف_کرو PPC_LD_U8		6
#تعریف_کرو PPC_LD_S8		7
#تعریف_کرو PPC_LD_U16		8
#تعریف_کرو PPC_LD_S16		9

#جے_بنا_تعریف POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#وإلا
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#جے_انت

خارجي فراغ ffi_گھیرا_ASM (فراغ);

#إذا defined (FFI_GO_CLOSURES)
خارجي فراغ ffi_go_گھیرا_ASM (فراغ);
#جے_انت

تعداد {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128_بٹ	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64_بٹ	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
تعداد {
  عدد_GPR_ARG_REGISTERS = 8,
  عدد_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

تعداد { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments.
   
   m32/m64

   The stack layout we want looks like this:

   |   Return address from ffi_call_DARWIN      |	higher addresses
   |--------------------------------------------|
   |   Previous backchain pointer	4/8	|	stack pointer here
   |--------------------------------------------|<+ <<<	on entry to
   |   ASM_NEEDS_REGISTERS=r28-r31   4*(4/8)	| |	ffi_call_DARWIN
   |--------------------------------------------| |
   |   When we have any FP activity... the	| |
   |   FPRs occupy NUM_FPR_ARG_REGISTERS slots	| |
   |   here fp13 .. fp1 from high to low addr.	| |
   ~						~ ~
   |   Parameters      (at least 8*4/8=32/64)	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |	stack	|
   |   Reserved                       2*4/8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |	lower addresses
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |     stack pointer here
   |   Current backchain pointer	4/8	|-/	during
   |--------------------------------------------|   <<<	ffi_call_DARWIN

   */

#إذا defined(POWERPC_DARWIN64)
ساكن فراغ
darwin64_pass_struct_by_قدر 
  (ffi_type *, حرف *, بنانشانی, بنانشانی *, مزدوج **, بنانشانی لما **);
#جے_انت

/* This depends on GPR_SIZE = sizeof (unsigned long) */

فراغ
ffi_prep_دلیلاں (extended_cif *ecif, بنانشانی لما *ثابت ڈھیر)
{
  ثابت بنانشانی بائٹاں = ecif->cif->بائٹاں;
  ثابت بنانشانی flags = ecif->cif->flags;
  ثابت بنانشانی nargs = ecif->cif->nargs;
#إذا !defined(POWERPC_DARWIN64) 
  ثابت ffi_abi abi = ecif->cif->abi;
#جے_انت

  /* 'stacktop' points at the previous backchain pointer.  */
  بنانشانی لما *ثابت stacktop = ڈھیر + (بائٹاں / حجم(بنانشانی لما));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  مزدوج *fpr_ادھار = (مزدوج *) (stacktop - ASM_NEEDS_REGISTERS) - عدد_FPR_ARG_REGISTERS;
  صحيح gp_گنتی = 0, fparg_گنتی = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  بنانشانی لما *اگلا_arg = ڈھیر + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  صحيح i;
  مزدوج double_tmp;
  فراغ **p_argv = ecif->avalue;
  بنانشانی لما gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#إذا !defined(POWERPC_DARWIN64) 
  حرف *dest_cpy;
#جے_انت
  بنانشانی ناپ_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((بنانشانی) (حرف *) ڈھیر & 0xF) == 0);
  FFI_ASSERT(((بنانشانی) (حرف *) stacktop & 0xF) == 0);
  FFI_ASSERT((بائٹاں & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  إذا (flags & FLAG_RETVAL_REFERENCE)
    *اگلا_arg++ = (بنانشانی لما) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  لكل (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      بدل ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	حالة FFI_TYPE_FLOAT:
	  double_tmp = *(عائم *) *p_argv;
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_ادھار++ = double_tmp;
#إذا defined(POWERPC_DARWIN)
	  *(عائم *)اگلا_arg = *(عائم *) *p_argv;
#وإلا
	  *(مزدوج *)اگلا_arg = double_tmp;
#جے_انت
	  اگلا_arg++;
	  gp_گنتی++;
	  fparg_گنتی++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  double_tmp = *(مزدوج *) *p_argv;
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_ادھار++ = double_tmp;
	  *(مزدوج *)اگلا_arg = double_tmp;
#جے_تعریف_شدہ POWERPC64
	  اگلا_arg++;
	  gp_گنتی++;
#وإلا
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#جے_انت
	  fparg_گنتی++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالة FFI_TYPE_LONGDOUBLE:
#  إذا defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *(لما مزدوج *) fpr_ادھار++ = *(لما مزدوج *) *p_argv;
	  وإلا
	    *(لما مزدوج *) اگلا_arg = *(لما مزدوج *) *p_argv;
	  اگلا_arg += 2;
	  fparg_گنتی += 2;
#  وإلا
	  double_tmp = ((مزدوج *) *p_argv)[0];
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_ادھار++ = double_tmp;
	  *(مزدوج *) اگلا_arg = double_tmp;
#    إذا defined(POWERPC_DARWIN64)
	  اگلا_arg++;
	  gp_گنتی++;
#    وإلا
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#    endif
	  fparg_گنتی++;
	  double_tmp = ((مزدوج *) *p_argv)[1];
	  إذا (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_ادھار++ = double_tmp;
	  *(مزدوج *) اگلا_arg = double_tmp;
#    إذا defined(POWERPC_DARWIN64)
	  اگلا_arg++;
	  gp_گنتی++;
#    وإلا
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#    endif
	  fparg_گنتی++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;
#جے_انت
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#جے_تعریف_شدہ POWERPC64
	  gprvalue = *(لما لما *) *p_argv;
	  اذهب putgpr;
#وإلا
	  *(لما لما *) اگلا_arg = *(لما لما *) *p_argv;
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#جے_انت
	  اكسر;
	حالة FFI_TYPE_اشارہ_گر:
	  gprvalue = *(بنانشانی لما *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT8:
	  gprvalue = *(بنانشانی حرف *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT8:
	  gprvalue = *(نشانیوالا حرف *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT16:
	  gprvalue = *(بنانشانی چھوٹا *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT16:
	  gprvalue = *(نشانیوالا چھوٹا *) *p_argv;
	  اذهب putgpr;

	حالة FFI_TYPE_STRUCT:
	  ناپ_al = (*ptr)->ناپ;
#إذا defined(POWERPC_DARWIN64)
	  اگلا_arg = (بنانشانی لما *)FFI_ALIGN((حرف *)اگلا_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_قدر (*ptr, (حرف *) *p_argv, 
					 (بنانشانی) ناپ_al,
					 (بنانشانی صحيح *) &fparg_گنتی,
					 &fpr_ادھار, &اگلا_arg);
#وإلا
	  dest_cpy = (حرف *) اگلا_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  إذا ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    ناپ_al = FFI_ALIGN((*ptr)->ناپ, 8);

#  إذا defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  حافظہ_نقل ((حرف *) dest_cpy, (حرف *) *p_argv, ناپ_al);
	  اگلا_arg += (ناپ_al + 7) / 8;
#  وإلا
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  إذا (ناپ_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - ناپ_al;

	  حافظہ_نقل((حرف *) dest_cpy, (حرف *) *p_argv, ناپ_al);
	  اگلا_arg += (ناپ_al + 3) / 4;
#  endif
#جے_انت
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  gprvalue = *(نشانیوالا صحيح *) *p_argv;
	  اذهب putgpr;

	حالة FFI_TYPE_UINT32:
	  gprvalue = *(بنانشانی صحيح *) *p_argv;
	putgpr:
	  *اگلا_arg++ = gprvalue;
	  gp_گنتی++;
	  اكسر;
	افتراضي:
	  اكسر;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#إذا defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ساكن فراغ
darwin64_جانچ_struct_for_floats (ffi_type *s, بنانشانی *nfpr)
{
  صحيح i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      ffi_type *p = s->elements[i];
      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    darwin64_جانچ_struct_for_floats (p, nfpr);
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    اكسر;
	  حالة FFI_TYPE_DOUBLE:
	  حالة FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    اكسر;
	  افتراضي:
	    اكسر;    
	}
    }
}

ساكن صحيح
darwin64_struct_ناپ_exceeds_gprs_p (ffi_type *s, حرف *src, بنانشانی *nfpr)
{
  بنانشانی struct_وتھ=0, i;

  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *شے_ادھار;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_وتھ = FFI_ALIGN(struct_وتھ, p->alignment);

      شے_ادھار = src + struct_وتھ;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    إذا (darwin64_struct_ناپ_exceeds_gprs_p (p, شے_ادھار, nfpr))
	      أرجع 1;
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    شے_ادھار += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    اكسر;
	  افتراضي:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    إذا ((بنانشانی لما)شے_ادھار >= 8*8) 
	      أرجع 1;
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_وتھ += p->ناپ;
    }
  أرجع 0;
}

/* Can this struct be returned by value?  */
صحيح 
darwin64_struct_ret_by_قدر_p (ffi_type *s)
{
  بنانشانی nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  إذا (s->ناپ > 168)
    أرجع 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_جانچ_struct_for_floats (s, &nfp);
  إذا (nfp > 13)
    أرجع 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  إذا (s->ناپ <= 64)
    أرجع 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  إذا (darwin64_struct_ناپ_exceeds_gprs_p (s, عدم, &nfp))
    أرجع 0;
  
  أرجع 1;
}

فراغ
darwin64_pass_struct_floats (ffi_type *s, حرف *src, 
			     بنانشانی *nfpr, مزدوج **fprs)
{
  صحيح i;
  مزدوج *fpr_ادھار = *fprs;
  بنانشانی struct_وتھ = 0;

  /* We don't assume anything about the alignment of the source.  */
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *شے_ادھار;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_وتھ = FFI_ALIGN(struct_وتھ, p->alignment);
      شے_ادھار = src + struct_وتھ;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, شے_ادھار, nfpr,
					   &fpr_ادھار);
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_ادھار++ = *(مزدوج *)شے_ادھار;
	    (*nfpr) += 1;
	    شے_ادھار += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_ادھار++ = *(مزدوج *)شے_ادھار;
	    (*nfpr) += 1;
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_ادھار++ = (مزدوج) *(عائم *)شے_ادھار;
	    (*nfpr) += 1;
	    اكسر;
	  افتراضي:
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_وتھ += p->ناپ;
    }
  /* Update the scores.  */
  *fprs = fpr_ادھار;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ساكن فراغ
darwin64_pass_struct_by_قدر (ffi_type *s, حرف *src, بنانشانی ناپ,
			       بنانشانی *nfpr, مزدوج **fprs, بنانشانی لما **arg)
{
  بنانشانی لما *اگلا_arg = *arg;
  حرف *dest_cpy = (حرف *)اگلا_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  إذا (!ناپ)
    أرجع;

  /* First... special cases.  */
  إذا (ناپ < 3
      || (ناپ == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *اگلا_arg = 0UL; 
      dest_cpy += 8 - ناپ;
      حافظہ_نقل ((حرف *) dest_cpy, src, ناپ);
      اگلا_arg++;
    }
  وإلا إذا (ناپ == 16)
    {
      حافظہ_نقل ((حرف *) dest_cpy, src, ناپ);
      اگلا_arg += 2;
    }
  وإلا
    {
      /* now the general case, we consider embedded floats.  */
      حافظہ_نقل ((حرف *) dest_cpy, src, ناپ);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      اگلا_arg += (ناپ+7)/8;
    }
    
  *arg = اگلا_arg;
}

مزدوج *
darwin64_struct_floats_منزل_mem (ffi_type *s, حرف *dest, مزدوج *fprs, بنانشانی *nf)
{
  صحيح i;
  بنانشانی struct_وتھ = 0;

  /* We don't assume anything about the alignment of the source.  */
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *شے_ادھار;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_وتھ = FFI_ALIGN(struct_وتھ, p->alignment);
      شے_ادھار = dest + struct_وتھ;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_منزل_mem (p, شے_ادھار, fprs, nf);
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(مزدوج *)شے_ادھار = *fprs++ ;
		(*nf) += 1;
	      }
	    شے_ادھار += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(مزدوج *)شے_ادھار = *fprs++ ;
		(*nf) += 1;
	      }
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(عائم *)شے_ادھار = (عائم) *fprs++ ;
		(*nf) += 1;
	      }
	    اكسر;
	  افتراضي:
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_وتھ += p->ناپ;
    }
  أرجع fprs;
}

#جے_انت

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ساكن فراغ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  صحيح i;

  إذا (s->type != FFI_TYPE_STRUCT)
    أرجع;

  s->ناپ = 0;
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      ffi_type *p;
      صحيح align;
      
      p = s->elements[i];
      إذا (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#إذا defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#وإلا
      /* Natural alignment for the first item... */
      إذا (i == 0)
	align = p->alignment;
      وإلا إذا (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      وإلا
	/* .. or align is 4.  */
	align = 4;
#جے_انت
      /* Pad, if necessary, before adding the current item.  */
      s->ناپ = FFI_ALIGN(s->ناپ, align) + p->ناپ;
    }
  
  s->ناپ = FFI_ALIGN(s->ناپ, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  إذا (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ساكن صحيح
aix_adjust_aggregate_sizes (ffi_type *s, صحيح outer_most_type_or_پہلا_member)
{
  صحيح i, nested_پہلا_member=0, final_align, rc=0;

  إذا (s->type != FFI_TYPE_STRUCT)
    أرجع 0;

  s->ناپ = 0;
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      ffi_type p;
      صحيح align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      إذا (i == 0)
        nested_پہلا_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_پہلا_member);
      وإلا
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      إذا (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->ناپ = FFI_ALIGN(s->ناپ, align) + p.ناپ;
    }

  final_align=s->alignment;
  إذا ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_پہلا_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      إذا (outer_most_type_or_پہلا_member)
        s->alignment=final_align;
  }

  s->ناپ = FFI_ALIGN(s->ناپ, final_align);
  أرجع rc;
}

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  بنانشانی i;
  ffi_type **ptr;
  بنانشانی بائٹاں;
  بنانشانی fparg_گنتی = 0, intarg_گنتی = 0;
  بنانشانی flags = 0;
  بنانشانی ناپ_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  إذا (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      لكل (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  إذا (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      لكل (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  بائٹاں = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * حجم(بنانشانی لما);

  /* Return value handling.  
    The rules m32 are as follows:
     - 32-bit (or less) integer values are returned in gpr3;
     - structures of size <= 4 bytes also returned in gpr3;
     - 64-bit integer values [??? and structures between 5 and 8 bytes] are
       returned in gpr3 and gpr4;
     - Single/double FP values are returned in fpr1;
     - Long double FP (if not equivalent to double) values are returned in
       fpr1 and fpr2;
     m64:
     - 64-bit or smaller integral values are returned in GPR3
     - Single/double FP values are returned in fpr1;
     - Long double FP values are returned in fpr1 and fpr2;
     m64 Structures:
     - If the structure could be accommodated in registers were it to be the
       first argument to a routine, then it is returned in those registers.
     m32/m64 structures otherwise:
     - Larger structures values are allocated space and a pointer is passed
       as the first argument.  */
  بدل (cif->rtype->type)
    {

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_بٹ;
      flags |= FLAG_RETURNS_FP;
      اكسر;
#جے_انت

    حالة FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٹ;
      /* Fall through.  */
    حالة FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      اكسر;

    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
#جے_تعریف_شدہ POWERPC64
    حالة FFI_TYPE_اشارہ_گر:
#جے_انت
      flags |= FLAG_RETURNS_64_بٹ;
      اكسر;

    حالة FFI_TYPE_STRUCT:
#إذا defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	إذا (darwin64_struct_ret_by_قدر_p (cif->rtype))
	  {
	    بنانشانی nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    إذا (cif->rtype->ناپ != 16)
	      darwin64_جانچ_struct_for_floats (cif->rtype, &nfpr) ;
	    وإلا
	      flags |= FLAG_RETURNS_128_بٹ;
	    /* Will be 0 for 16byte struct.  */
	    إذا (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	وإلا /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_گنتی++;
	  }
      }
#وإلاإذا defined(DARWIN_PPC)
      إذا (cif->rtype->ناپ <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      وإلا /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_گنتی++;
	}
#وإلا /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_گنتی++;
#جے_انت
      اكسر;
    حالة FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      اكسر;

    افتراضي:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      اكسر;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      بنانشانی align_الفاظ;
      بدل ((*ptr)->type)
	{
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	  fparg_گنتی++;
#إذا !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  إذا (fparg_گنتی > عدد_FPR_ARG_REGISTERS
	      && (intarg_گنتی & 0x01) != 0)
	    intarg_گنتی++;
#جے_انت
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
	  fparg_گنتی += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  إذا (fparg_گنتی >= عدد_FPR_ARG_REGISTERS)
#إذا defined (POWERPC64)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, 2);
#وإلا
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, 4);
#جے_انت
	  اكسر;
#جے_انت

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#إذا defined(POWERPC64)
	  intarg_گنتی++;
#وإلا
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  إذا (intarg_گنتی == عدد_GPR_ARG_REGISTERS-1
	      || (intarg_گنتی >= عدد_GPR_ARG_REGISTERS 
	          && (intarg_گنتی & 0x01) != 0))
	    intarg_گنتی++;
	  intarg_گنتی += 2;
#جے_انت
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  ناپ_al = (*ptr)->ناپ;
#إذا defined(POWERPC_DARWIN64)
	  align_الفاظ = (*ptr)->alignment >> 3;
	  إذا (align_الفاظ)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, align_الفاظ);
	  /* Base size of the struct.  */
	  intarg_گنتی += (ناپ_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  إذا (ناپ_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_جانچ_struct_for_floats (*ptr, &fparg_گنتی) ;
#وإلا
	  align_الفاظ = (*ptr)->alignment >> 2;
	  إذا (align_الفاظ)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, align_الفاظ);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_گنتی += (ناپ_al + 7) / 8;
#  وإلا
	  intarg_گنتی += (ناپ_al + 3) / 4;
#  endif
#جے_انت
	  اكسر;

	افتراضي:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_گنتی++;
	  اكسر;
	}
    }

  إذا (fparg_گنتی != 0)
    flags |= FLAG_FP_ARGUMENTS;

#إذا defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  إذا (fparg_گنتی != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    بائٹاں += عدد_FPR_ARG_REGISTERS * حجم(مزدوج);
#وإلا
  /* Space for the FPR registers, if needed.  */
  إذا (fparg_گنتی != 0)
    بائٹاں += عدد_FPR_ARG_REGISTERS * حجم(مزدوج);
#جے_انت

  /* Stack space.  */
#جے_تعریف_شدہ POWERPC64
  إذا ((intarg_گنتی + fparg_گنتی) > عدد_GPR_ARG_REGISTERS)
    بائٹاں += (intarg_گنتی + fparg_گنتی) * حجم(لما);
#وإلا
  إذا ((intarg_گنتی + 2 * fparg_گنتی) > عدد_GPR_ARG_REGISTERS)
    بائٹاں += (intarg_گنتی + 2 * fparg_گنتی) * حجم(لما);
#جے_انت
  وإلا
    بائٹاں += عدد_GPR_ARG_REGISTERS * حجم(لما);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائٹاں = FFI_ALIGN(بائٹاں, 16) ;

  cif->flags = flags;
  cif->بائٹاں = بائٹاں;

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_AIX(extended_cif *, لما, بنانشانی, بنانشانی *,
			 فراغ (*fn)(فراغ), فراغ (*fn2)(فراغ));

#إذا defined (FFI_GO_CLOSURES)
خارجي فراغ ffi_call_go_AIX(extended_cif *, لما, بنانشانی, بنانشانی *,
			    فراغ (*fn)(فراغ), فراغ (*fn2)(فراغ), فراغ *گھیرا);
#جے_انت

خارجي فراغ ffi_call_DARWIN(extended_cif *, لما, بنانشانی, بنانشانی *,
			    فراغ (*fn)(فراغ), فراغ (*fn2)(فراغ), ffi_type*);

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  إذا ((rvalue == عدم) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ناپ);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_AIX:
      ffi_call_AIX(&ecif, -(لما)cif->بائٹاں, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_دلیلاں));
      اكسر;
    حالة FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(لما)cif->بائٹاں, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_دلیلاں), cif->rtype);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

#إذا defined (FFI_GO_CLOSURES)
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	     فراغ *گھیرا)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  إذا ((rvalue == عدم) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ناپ);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_AIX:
      ffi_call_go_AIX(&ecif, -(لما)cif->بائٹاں, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_دلیلاں), گھیرا);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}
#جے_انت

ساكن فراغ flush_icache(حرف *);
ساكن فراغ flush_range(حرف *, صحيح);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

تعريفنوع بنية aix_fd_struct {
  فراغ *کوڈ_اشارہ_گر;
  فراغ *toc;
} aix_fd;

/* here I'd like to add the stack frame layout we use in darwin_closure.S
   and aix_closure.S

   m32/m64

   The stack layout looks like this:

   |   Additional params...			| |     Higher address
   ~						~ ~
   |   Parameters      (at least 8*4/8=32/64)	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |
   |   Reserved                       2*4/8	| |
   |--------------------------------------------| |
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |
   |   Current backchain pointer	4/8	|-/ Parent's frame.
   |--------------------------------------------| <+ <<< on entry to ffi_closure_ASM
   |   Result Bytes			16	| |
   |--------------------------------------------| |
   ~   padding to 16-byte alignment		~ ~
   |--------------------------------------------| |
   |   NUM_FPR_ARG_REGISTERS slots		| |
   |   here fp13 .. fp1		       13*8	| |
   |--------------------------------------------| |
   |   R3..R10			  8*4/8=32/64	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |	stack	|
   |   Reserved [compiler,binder]     2*4/8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |	lower addresses
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |     stack pointer here
   |   Current backchain pointer	4/8	|-/	during
   |--------------------------------------------|   <<<	ffi_closure_ASM.

*/

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  بنانشانی صحيح *tramp;
  بنية ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  بدل (cif->abi)
    {
      حالة FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (بنانشانی صحيح *) &گھیرا->tramp[0];
#إذا defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((بنانشانی لما *)&tramp[2]) = (بنانشانی لما) ffi_گھیرا_ASM; /* function  */
	*((بنانشانی لما *)&tramp[4]) = (بنانشانی لما) codeloc; /* context  */
#وإلا
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (بنانشانی لما) ffi_گھیرا_ASM; /* function  */
	tramp[3] = (بنانشانی لما) codeloc; /* context  */
#جے_انت
	گھیرا->cif = cif;
	گھیرا->fun = fun;
	گھیرا->user_انکڑے = user_انکڑے;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_ناپ);

	اكسر;

    حالة FFI_AIX:

      tramp_aix = (بنية ffi_aix_trampoline_struct *) (گھیرا->tramp);
      fd = (aix_fd *)(فراغ *)ffi_گھیرا_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->کوڈ_اشارہ_گر = fd->کوڈ_اشارہ_گر;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      گھیرا->cif = cif;
      گھیرا->fun = fun;
      گھیرا->user_انکڑے = user_انکڑے;
      اكسر;

    افتراضي:
      أرجع FFI_BAD_ABI;
      اكسر;
    }
  أرجع FFI_OK;
}

#إذا defined (FFI_GO_CLOSURES)
ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا* گھیرا,
		     ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  بدل (cif->abi)
    {
      حالة FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        گھیرا->tramp = (فراغ *)ffi_go_گھیرا_ASM;
        گھیرا->cif = cif;
        گھیرا->fun = fun;
        أرجع FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      افتراضي:
        أرجع FFI_BAD_ABI;
        اكسر;
    }
  أرجع FFI_OK;
}
#جے_انت

ساكن فراغ
flush_icache(حرف *addr)
{
#جے_بنا_تعریف _AIX
  __asm__ متطاير (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#جے_انت
}

ساكن فراغ
flush_range(حرف * addr1, صحيح ناپ)
{
#تعریف_کرو MIN_سطر_ناپ 32
  صحيح i;
  لكل (i = 0; i < ناپ; i += MIN_سطر_ناپ)
    flush_icache(addr1+i);
  flush_icache(addr1+ناپ-1);
}

تعريفنوع اتحاد
{
  عائم f;
  مزدوج d;
} ffi_dblfl;

صحيح
ffi_گھیرا_helper_DARWIN (ffi_گھیرا *, فراغ *,
			   بنانشانی لما *, ffi_dblfl *);

#إذا defined (FFI_GO_CLOSURES)
صحيح
ffi_go_گھیرا_helper_DARWIN (ffi_go_گھیرا*, فراغ *,
			      بنانشانی لما *, ffi_dblfl *);
#جے_انت

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ساكن صحيح
ffi_گھیرا_helper_common (ffi_cif* cif,
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_انکڑے, فراغ *rvalue,
			   بنانشانی لما *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  تعريفنوع مزدوج ldbits[2];

  اتحاد ldu
  {
    ldbits lb;
    لما مزدوج ld;
  };

  فراغ **          avalue;
  ffi_type **      arg_types;
  لما             i, avn;
  ffi_dblfl *      سرا_pfr = pfr + عدد_FPR_ARG_REGISTERS;
  بنانشانی         ناپ_al;
  صحيح              struct_ret_by_قدر = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  بنانشانی لما *  pgr0 = pgr;
#إذا defined(POWERPC_DARWIN64)
  بنانشانی 	   fpsused = 0;
#جے_انت

  avalue = alloca (cif->nargs * حجم(فراغ *));

  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#إذا defined(POWERPC_DARWIN64)
      إذا (!darwin64_struct_ret_by_قدر_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (فراغ *) *pgr;
	  pgr++;
	}
      وإلا
	struct_ret_by_قدر = 1;
#وإلاإذا defined(DARWIN_PPC)
      إذا (cif->rtype->ناپ > 4)
	{
	  rvalue = (فراغ *) *pgr;
	  pgr++;
	}
      وإلا
	struct_ret_by_قدر = 1;
#وإلا /* assume we return by ref.  */
      rvalue = (فراغ *) *pgr;
      pgr++;
#جے_انت
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  طالما (i < avn)
    {
      بدل (arg_types[i]->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
#إذا  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 7;
#وإلا
	  avalue[i] = (حرف *) pgr + 3;
#جے_انت
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
#إذا  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 6;
#وإلا
	  avalue[i] = (حرف *) pgr + 2;
#جے_انت
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
#إذا  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 4;
#وإلا
	حالة FFI_TYPE_اشارہ_گر:
	  avalue[i] = pgr;
#جے_انت
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  ناپ_al = arg_types[i]->ناپ;
#إذا defined(POWERPC_DARWIN64)
	  pgr = (بنانشانی لما *)FFI_ALIGN((حرف *)pgr, arg_types[i]->alignment);
	  إذا (ناپ_al < 3 || ناپ_al == 4)
	    {
	      avalue[i] = ((حرف *)pgr)+8-ناپ_al;
	      إذا (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < عدد_FPR_ARG_REGISTERS)
		{
		  *(عائم *)pgr = (عائم) *(مزدوج *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  وإلا 
	    {
	      إذا (ناپ_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_منزل_mem (arg_types[i], (حرف *)pgr,
						   (مزدوج *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (ناپ_al + 7) / 8;
#وإلا
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  إذا (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    ناپ_al = FFI_ALIGN(arg_types[i]->ناپ, 8);
#  إذا defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (ناپ_al + 7) / 8;
#  وإلا
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  إذا (ناپ_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (حرف*) pgr + 4 - ناپ_al;
	  وإلا
	    avalue[i] = pgr;
	  pgr += (ناپ_al + 3) / 4;
#  endif
#جے_انت
	  اكسر;

	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
#إذا  defined(POWERPC64)
	حالة FFI_TYPE_اشارہ_گر:
	  avalue[i] = pgr;
	  pgr++;
	  اكسر;
#وإلا
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  اكسر;
#جے_انت

	حالة FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  إذا (pfr < سرا_pfr)
	    {
	      مزدوج عارضی = pfr->d;
	      pfr->f = (عائم) عارضی;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  إذا (pfr < سرا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    {
	      avalue[i] = pgr;
	    }
#جے_تعریف_شدہ POWERPC64
	  pgr++;
#وإلا
	  pgr += 2;
#جے_انت
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالة FFI_TYPE_LONGDOUBLE:
#جے_تعریف_شدہ POWERPC64
	  إذا (pfr + 1 < سرا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  وإلا
	    {
	      إذا (pfr < سرا_pfr)
		{
		  *pgr = *(بنانشانی لما *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#وإلا  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  إذا (pfr + 1 < سرا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  وإلا إذا (pfr + 1 == سرا_pfr)
	    {
	      اتحاد ldu عارضی_ld;
	      حافظہ_نقل (&عارضی_ld.lb[0], pfr, حجم(ldbits));
	      حافظہ_نقل (&عارضی_ld.lb[1], pgr + 2, حجم(ldbits));
	      avalue[i] = &عارضی_ld.ld;
	      pfr++;
	    }
	  وإلا
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#جے_انت  /* POWERPC64 */
	  اكسر;
#جے_انت
	افتراضي:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_انکڑے);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      أرجع PPC_LD_NONE;
    حالة FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      إذا (struct_ret_by_قدر)
	{
	  *pgr0 = (بنانشانی لما) cif->rtype;
	  أرجع PPC_LD_STRUCT;
	}
      أرجع PPC_LD_NONE;
    حالة FFI_TYPE_FLOAT:
      أرجع PPC_LD_F32;
    حالة FFI_TYPE_DOUBLE:
      أرجع PPC_LD_F64;
#إذا FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      أرجع PPC_LD_F128;
#جے_انت
    حالة FFI_TYPE_UINT8:
      أرجع PPC_LD_U8;
    حالة FFI_TYPE_SINT8:
      أرجع PPC_LD_S8;
    حالة FFI_TYPE_UINT16:
      أرجع PPC_LD_U16;
    حالة FFI_TYPE_SINT16:
      أرجع PPC_LD_S16;
    حالة FFI_TYPE_UINT32:
      أرجع PPC_LD_U32;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      أرجع PPC_LD_S32;
    حالة FFI_TYPE_اشارہ_گر:
      أرجع PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      أرجع PPC_LD_I64;
    افتراضي:
      abort();
    }
}

صحيح
ffi_گھیرا_helper_DARWIN (ffi_گھیرا *گھیرا, فراغ *rvalue,
			   بنانشانی لما *pgr, ffi_dblfl *pfr)
{
  أرجع ffi_گھیرا_helper_common (گھیرا->cif, گھیرا->fun,
				    گھیرا->user_انکڑے, rvalue, pgr, pfr);
}

#إذا defined (FFI_GO_CLOSURES)
صحيح
ffi_go_گھیرا_helper_DARWIN (ffi_go_گھیرا *گھیرا, فراغ *rvalue,
			      بنانشانی لما *pgr, ffi_dblfl *pfr)
{
  أرجع ffi_گھیرا_helper_common (گھیرا->cif, گھیرا->fun,
				    گھیرا, rvalue, pgr, pfr);
}
#جے_انت

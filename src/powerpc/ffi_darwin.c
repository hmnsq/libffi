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
    فراغ * کد_اشاره_گر;	/* Pointer to ffi_closure_ASM */
    فراغ * toc;			/* TOC */
    فراغ * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#تعریف_کن PPC_LD_NONE		0
#تعریف_کن PPC_LD_R3		1
#تعریف_کن PPC_LD_R3R4		2
#تعریف_کن PPC_LD_F32		3
#تعریف_کن PPC_LD_F64		4
#تعریف_کن PPC_LD_F128		5
#تعریف_کن PPC_LD_U8		6
#تعریف_کن PPC_LD_S8		7
#تعریف_کن PPC_LD_U16		8
#تعریف_کن PPC_LD_S16		9

#اگر_تعریف_نشده POWERPC64
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
#پایان_اگر

خارجي فراغ ffi_بستگی_ASM (فراغ);

#إذا defined (FFI_GO_CLOSURES)
خارجي فراغ ffi_go_بستگی_ASM (فراغ);
#پایان_اگر

تعداد {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128_بیتها	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64_بیتها	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
تعداد {
  رقم_GPR_ARG_REGISTERS = 8,
  رقم_FPR_ARG_REGISTERS = 13,
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
darwin64_pass_struct_by_مقدار 
  (ffi_type *, حرف *, بیعلامت, بیعلامت *, مزدوج **, بیعلامت دراز **);
#پایان_اگر

/* This depends on GPR_SIZE = sizeof (unsigned long) */

فراغ
ffi_prep_آرگومانها (extended_cif *ecif, بیعلامت دراز *ثابت پشته)
{
  ثابت بیعلامت بایتها = ecif->cif->بایتها;
  ثابت بیعلامت flags = ecif->cif->flags;
  ثابت بیعلامت nargs = ecif->cif->nargs;
#إذا !defined(POWERPC_DARWIN64) 
  ثابت ffi_abi abi = ecif->cif->abi;
#پایان_اگر

  /* 'stacktop' points at the previous backchain pointer.  */
  بیعلامت دراز *ثابت stacktop = پشته + (بایتها / حجم(بیعلامت دراز));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  مزدوج *fpr_پایه = (مزدوج *) (stacktop - ASM_NEEDS_REGISTERS) - رقم_FPR_ARG_REGISTERS;
  صحيح gp_شمار = 0, fparg_شمار = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  بیعلامت دراز *بعدی_arg = پشته + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  صحيح i;
  مزدوج double_tmp;
  فراغ **p_argv = ecif->avalue;
  بیعلامت دراز gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#إذا !defined(POWERPC_DARWIN64) 
  حرف *dest_cpy;
#پایان_اگر
  بیعلامت اندازه_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((بیعلامت) (حرف *) پشته & 0xF) == 0);
  FFI_ASSERT(((بیعلامت) (حرف *) stacktop & 0xF) == 0);
  FFI_ASSERT((بایتها & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  إذا (flags & FLAG_RETVAL_REFERENCE)
    *بعدی_arg++ = (بیعلامت دراز) (حرف *) ecif->rvalue;

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
	  إذا (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_پایه++ = double_tmp;
#إذا defined(POWERPC_DARWIN)
	  *(عائم *)بعدی_arg = *(عائم *) *p_argv;
#وإلا
	  *(مزدوج *)بعدی_arg = double_tmp;
#پایان_اگر
	  بعدی_arg++;
	  gp_شمار++;
	  fparg_شمار++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  double_tmp = *(مزدوج *) *p_argv;
	  إذا (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_پایه++ = double_tmp;
	  *(مزدوج *)بعدی_arg = double_tmp;
#اگر_تعریف_شده POWERPC64
	  بعدی_arg++;
	  gp_شمار++;
#وإلا
	  بعدی_arg += 2;
	  gp_شمار += 2;
#پایان_اگر
	  fparg_شمار++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالة FFI_TYPE_LONGDOUBLE:
#  إذا defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  إذا (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *(دراز مزدوج *) fpr_پایه++ = *(دراز مزدوج *) *p_argv;
	  وإلا
	    *(دراز مزدوج *) بعدی_arg = *(دراز مزدوج *) *p_argv;
	  بعدی_arg += 2;
	  fparg_شمار += 2;
#  وإلا
	  double_tmp = ((مزدوج *) *p_argv)[0];
	  إذا (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_پایه++ = double_tmp;
	  *(مزدوج *) بعدی_arg = double_tmp;
#    إذا defined(POWERPC_DARWIN64)
	  بعدی_arg++;
	  gp_شمار++;
#    وإلا
	  بعدی_arg += 2;
	  gp_شمار += 2;
#    endif
	  fparg_شمار++;
	  double_tmp = ((مزدوج *) *p_argv)[1];
	  إذا (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_پایه++ = double_tmp;
	  *(مزدوج *) بعدی_arg = double_tmp;
#    إذا defined(POWERPC_DARWIN64)
	  بعدی_arg++;
	  gp_شمار++;
#    وإلا
	  بعدی_arg += 2;
	  gp_شمار += 2;
#    endif
	  fparg_شمار++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  اكسر;
#پایان_اگر
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#اگر_تعریف_شده POWERPC64
	  gprvalue = *(دراز دراز *) *p_argv;
	  اذهب putgpr;
#وإلا
	  *(دراز دراز *) بعدی_arg = *(دراز دراز *) *p_argv;
	  بعدی_arg += 2;
	  gp_شمار += 2;
#پایان_اگر
	  اكسر;
	حالة FFI_TYPE_اشاره_گر:
	  gprvalue = *(بیعلامت دراز *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT8:
	  gprvalue = *(بیعلامت حرف *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT8:
	  gprvalue = *(علامتدار حرف *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT16:
	  gprvalue = *(بیعلامت کوتاه *) *p_argv;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT16:
	  gprvalue = *(علامتدار کوتاه *) *p_argv;
	  اذهب putgpr;

	حالة FFI_TYPE_STRUCT:
	  اندازه_al = (*ptr)->اندازه;
#إذا defined(POWERPC_DARWIN64)
	  بعدی_arg = (بیعلامت دراز *)FFI_ALIGN((حرف *)بعدی_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_مقدار (*ptr, (حرف *) *p_argv, 
					 (بیعلامت) اندازه_al,
					 (بیعلامت صحيح *) &fparg_شمار,
					 &fpr_پایه, &بعدی_arg);
#وإلا
	  dest_cpy = (حرف *) بعدی_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  إذا ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    اندازه_al = FFI_ALIGN((*ptr)->اندازه, 8);

#  إذا defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  کپی_حافظه ((حرف *) dest_cpy, (حرف *) *p_argv, اندازه_al);
	  بعدی_arg += (اندازه_al + 7) / 8;
#  وإلا
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  إذا (اندازه_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - اندازه_al;

	  کپی_حافظه((حرف *) dest_cpy, (حرف *) *p_argv, اندازه_al);
	  بعدی_arg += (اندازه_al + 3) / 4;
#  endif
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  gprvalue = *(علامتدار صحيح *) *p_argv;
	  اذهب putgpr;

	حالة FFI_TYPE_UINT32:
	  gprvalue = *(بیعلامت صحيح *) *p_argv;
	putgpr:
	  *بعدی_arg++ = gprvalue;
	  gp_شمار++;
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
darwin64_پیمایش_struct_for_floats (ffi_type *s, بیعلامت *nfpr)
{
  صحيح i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      ffi_type *p = s->elements[i];
      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    darwin64_پیمایش_struct_for_floats (p, nfpr);
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
darwin64_struct_اندازه_exceeds_gprs_p (ffi_type *s, حرف *src, بیعلامت *nfpr)
{
  بیعلامت struct_جابه_جایی=0, i;

  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *قلم_پایه;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_جابه_جایی = FFI_ALIGN(struct_جابه_جایی, p->alignment);

      قلم_پایه = src + struct_جابه_جایی;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    إذا (darwin64_struct_اندازه_exceeds_gprs_p (p, قلم_پایه, nfpr))
	      أرجع 1;
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    قلم_پایه += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      أرجع 1;
	    (*nfpr) += 1;
	    اكسر;
	  افتراضي:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    إذا ((بیعلامت دراز)قلم_پایه >= 8*8) 
	      أرجع 1;
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_جابه_جایی += p->اندازه;
    }
  أرجع 0;
}

/* Can this struct be returned by value?  */
صحيح 
darwin64_struct_ret_by_مقدار_p (ffi_type *s)
{
  بیعلامت nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  إذا (s->اندازه > 168)
    أرجع 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_پیمایش_struct_for_floats (s, &nfp);
  إذا (nfp > 13)
    أرجع 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  إذا (s->اندازه <= 64)
    أرجع 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  إذا (darwin64_struct_اندازه_exceeds_gprs_p (s, عدم, &nfp))
    أرجع 0;
  
  أرجع 1;
}

فراغ
darwin64_pass_struct_floats (ffi_type *s, حرف *src, 
			     بیعلامت *nfpr, مزدوج **fprs)
{
  صحيح i;
  مزدوج *fpr_پایه = *fprs;
  بیعلامت struct_جابه_جایی = 0;

  /* We don't assume anything about the alignment of the source.  */
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *قلم_پایه;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_جابه_جایی = FFI_ALIGN(struct_جابه_جایی, p->alignment);
      قلم_پایه = src + struct_جابه_جایی;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, قلم_پایه, nfpr,
					   &fpr_پایه);
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_پایه++ = *(مزدوج *)قلم_پایه;
	    (*nfpr) += 1;
	    قلم_پایه += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_پایه++ = *(مزدوج *)قلم_پایه;
	    (*nfpr) += 1;
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_پایه++ = (مزدوج) *(عائم *)قلم_پایه;
	    (*nfpr) += 1;
	    اكسر;
	  افتراضي:
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_جابه_جایی += p->اندازه;
    }
  /* Update the scores.  */
  *fprs = fpr_پایه;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ساكن فراغ
darwin64_pass_struct_by_مقدار (ffi_type *s, حرف *src, بیعلامت اندازه,
			       بیعلامت *nfpr, مزدوج **fprs, بیعلامت دراز **arg)
{
  بیعلامت دراز *بعدی_arg = *arg;
  حرف *dest_cpy = (حرف *)بعدی_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  إذا (!اندازه)
    أرجع;

  /* First... special cases.  */
  إذا (اندازه < 3
      || (اندازه == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *بعدی_arg = 0UL; 
      dest_cpy += 8 - اندازه;
      کپی_حافظه ((حرف *) dest_cpy, src, اندازه);
      بعدی_arg++;
    }
  وإلا إذا (اندازه == 16)
    {
      کپی_حافظه ((حرف *) dest_cpy, src, اندازه);
      بعدی_arg += 2;
    }
  وإلا
    {
      /* now the general case, we consider embedded floats.  */
      کپی_حافظه ((حرف *) dest_cpy, src, اندازه);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      بعدی_arg += (اندازه+7)/8;
    }
    
  *arg = بعدی_arg;
}

مزدوج *
darwin64_struct_floats_مقصد_mem (ffi_type *s, حرف *dest, مزدوج *fprs, بیعلامت *nf)
{
  صحيح i;
  بیعلامت struct_جابه_جایی = 0;

  /* We don't assume anything about the alignment of the source.  */
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      حرف *قلم_پایه;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      إذا (i > 0)
        struct_جابه_جایی = FFI_ALIGN(struct_جابه_جایی, p->alignment);
      قلم_پایه = dest + struct_جابه_جایی;

      بدل (p->type)
	{
	  حالة FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_مقصد_mem (p, قلم_پایه, fprs, nf);
	    اكسر;
	  حالة FFI_TYPE_LONGDOUBLE:
	    إذا (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(مزدوج *)قلم_پایه = *fprs++ ;
		(*nf) += 1;
	      }
	    قلم_پایه += 8;
	  /* FALL THROUGH */
	  حالة FFI_TYPE_DOUBLE:
	    إذا (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(مزدوج *)قلم_پایه = *fprs++ ;
		(*nf) += 1;
	      }
	    اكسر;
	  حالة FFI_TYPE_FLOAT:
	    إذا (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(عائم *)قلم_پایه = (عائم) *fprs++ ;
		(*nf) += 1;
	      }
	    اكسر;
	  افتراضي:
	    اكسر;    
	}
      /* now count the size of what we just used.  */
      struct_جابه_جایی += p->اندازه;
    }
  أرجع fprs;
}

#پایان_اگر

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ساكن فراغ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  صحيح i;

  إذا (s->type != FFI_TYPE_STRUCT)
    أرجع;

  s->اندازه = 0;
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
#پایان_اگر
      /* Pad, if necessary, before adding the current item.  */
      s->اندازه = FFI_ALIGN(s->اندازه, align) + p->اندازه;
    }
  
  s->اندازه = FFI_ALIGN(s->اندازه, s->alignment);
  
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
aix_adjust_aggregate_sizes (ffi_type *s, صحيح outer_most_type_or_اولی_member)
{
  صحيح i, nested_اولی_member=0, final_align, rc=0;

  إذا (s->type != FFI_TYPE_STRUCT)
    أرجع 0;

  s->اندازه = 0;
  لكل (i = 0; s->elements[i] != عدم; i++)
    {
      ffi_type p;
      صحيح align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      إذا (i == 0)
        nested_اولی_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_اولی_member);
      وإلا
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      إذا (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->اندازه = FFI_ALIGN(s->اندازه, align) + p.اندازه;
    }

  final_align=s->alignment;
  إذا ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_اولی_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      إذا (outer_most_type_or_اولی_member)
        s->alignment=final_align;
  }

  s->اندازه = FFI_ALIGN(s->اندازه, final_align);
  أرجع rc;
}

/* Perform machine dependent cif processing.  */
ffi_وضعیت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  بیعلامت i;
  ffi_type **ptr;
  بیعلامت بایتها;
  بیعلامت fparg_شمار = 0, intarg_شمار = 0;
  بیعلامت flags = 0;
  بیعلامت اندازه_al = 0;

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

  بایتها = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * حجم(بیعلامت دراز);

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
      flags |= FLAG_RETURNS_128_بیتها;
      flags |= FLAG_RETURNS_FP;
      اكسر;
#پایان_اگر

    حالة FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بیتها;
      /* Fall through.  */
    حالة FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      اكسر;

    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
#اگر_تعریف_شده POWERPC64
    حالة FFI_TYPE_اشاره_گر:
#پایان_اگر
      flags |= FLAG_RETURNS_64_بیتها;
      اكسر;

    حالة FFI_TYPE_STRUCT:
#إذا defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	إذا (darwin64_struct_ret_by_مقدار_p (cif->rtype))
	  {
	    بیعلامت nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    إذا (cif->rtype->اندازه != 16)
	      darwin64_پیمایش_struct_for_floats (cif->rtype, &nfpr) ;
	    وإلا
	      flags |= FLAG_RETURNS_128_بیتها;
	    /* Will be 0 for 16byte struct.  */
	    إذا (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	وإلا /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_شمار++;
	  }
      }
#وإلاإذا defined(DARWIN_PPC)
      إذا (cif->rtype->اندازه <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      وإلا /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_شمار++;
	}
#وإلا /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_شمار++;
#پایان_اگر
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
      بیعلامت align_واژگان;
      بدل ((*ptr)->type)
	{
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	  fparg_شمار++;
#إذا !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  إذا (fparg_شمار > رقم_FPR_ARG_REGISTERS
	      && (intarg_شمار & 0x01) != 0)
	    intarg_شمار++;
#پایان_اگر
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
	  fparg_شمار += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  إذا (fparg_شمار >= رقم_FPR_ARG_REGISTERS)
#إذا defined (POWERPC64)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, 2);
#وإلا
	    intarg_شمار = FFI_ALIGN(intarg_شمار, 4);
#پایان_اگر
	  اكسر;
#پایان_اگر

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#إذا defined(POWERPC64)
	  intarg_شمار++;
#وإلا
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  إذا (intarg_شمار == رقم_GPR_ARG_REGISTERS-1
	      || (intarg_شمار >= رقم_GPR_ARG_REGISTERS 
	          && (intarg_شمار & 0x01) != 0))
	    intarg_شمار++;
	  intarg_شمار += 2;
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  اندازه_al = (*ptr)->اندازه;
#إذا defined(POWERPC_DARWIN64)
	  align_واژگان = (*ptr)->alignment >> 3;
	  إذا (align_واژگان)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, align_واژگان);
	  /* Base size of the struct.  */
	  intarg_شمار += (اندازه_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  إذا (اندازه_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_پیمایش_struct_for_floats (*ptr, &fparg_شمار) ;
#وإلا
	  align_واژگان = (*ptr)->alignment >> 2;
	  إذا (align_واژگان)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, align_واژگان);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_شمار += (اندازه_al + 7) / 8;
#  وإلا
	  intarg_شمار += (اندازه_al + 3) / 4;
#  endif
#پایان_اگر
	  اكسر;

	افتراضي:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_شمار++;
	  اكسر;
	}
    }

  إذا (fparg_شمار != 0)
    flags |= FLAG_FP_ARGUMENTS;

#إذا defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  إذا (fparg_شمار != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    بایتها += رقم_FPR_ARG_REGISTERS * حجم(مزدوج);
#وإلا
  /* Space for the FPR registers, if needed.  */
  إذا (fparg_شمار != 0)
    بایتها += رقم_FPR_ARG_REGISTERS * حجم(مزدوج);
#پایان_اگر

  /* Stack space.  */
#اگر_تعریف_شده POWERPC64
  إذا ((intarg_شمار + fparg_شمار) > رقم_GPR_ARG_REGISTERS)
    بایتها += (intarg_شمار + fparg_شمار) * حجم(دراز);
#وإلا
  إذا ((intarg_شمار + 2 * fparg_شمار) > رقم_GPR_ARG_REGISTERS)
    بایتها += (intarg_شمار + 2 * fparg_شمار) * حجم(دراز);
#پایان_اگر
  وإلا
    بایتها += رقم_GPR_ARG_REGISTERS * حجم(دراز);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بایتها = FFI_ALIGN(بایتها, 16) ;

  cif->flags = flags;
  cif->بایتها = بایتها;

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_AIX(extended_cif *, دراز, بیعلامت, بیعلامت *,
			 فراغ (*fn)(فراغ), فراغ (*fn2)(فراغ));

#إذا defined (FFI_GO_CLOSURES)
خارجي فراغ ffi_call_go_AIX(extended_cif *, دراز, بیعلامت, بیعلامت *,
			    فراغ (*fn)(فراغ), فراغ (*fn2)(فراغ), فراغ *بستگی);
#پایان_اگر

خارجي فراغ ffi_call_DARWIN(extended_cif *, دراز, بیعلامت, بیعلامت *,
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
      ecif.rvalue = alloca (cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_AIX:
      ffi_call_AIX(&ecif, -(دراز)cif->بایتها, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_آرگومانها));
      اكسر;
    حالة FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(دراز)cif->بایتها, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_آرگومانها), cif->rtype);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

#إذا defined (FFI_GO_CLOSURES)
فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	     فراغ *بستگی)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  إذا ((rvalue == عدم) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_AIX:
      ffi_call_go_AIX(&ecif, -(دراز)cif->بایتها, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_آرگومانها), بستگی);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}
#پایان_اگر

ساكن فراغ flush_icache(حرف *);
ساكن فراغ flush_range(حرف *, صحيح);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

تعريفنوع بنية aix_fd_struct {
  فراغ *کد_اشاره_گر;
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

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_داده,
		      فراغ *codeloc)
{
  بیعلامت صحيح *tramp;
  بنية ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  بدل (cif->abi)
    {
      حالة FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (بیعلامت صحيح *) &بستگی->tramp[0];
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

	*((بیعلامت دراز *)&tramp[2]) = (بیعلامت دراز) ffi_بستگی_ASM; /* function  */
	*((بیعلامت دراز *)&tramp[4]) = (بیعلامت دراز) codeloc; /* context  */
#وإلا
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (بیعلامت دراز) ffi_بستگی_ASM; /* function  */
	tramp[3] = (بیعلامت دراز) codeloc; /* context  */
#پایان_اگر
	بستگی->cif = cif;
	بستگی->fun = fun;
	بستگی->user_داده = user_داده;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_اندازه);

	اكسر;

    حالة FFI_AIX:

      tramp_aix = (بنية ffi_aix_trampoline_struct *) (بستگی->tramp);
      fd = (aix_fd *)(فراغ *)ffi_بستگی_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->کد_اشاره_گر = fd->کد_اشاره_گر;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      بستگی->cif = cif;
      بستگی->fun = fun;
      بستگی->user_داده = user_داده;
      اكسر;

    افتراضي:
      أرجع FFI_BAD_ABI;
      اكسر;
    }
  أرجع FFI_OK;
}

#إذا defined (FFI_GO_CLOSURES)
ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی* بستگی,
		     ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  بدل (cif->abi)
    {
      حالة FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        بستگی->tramp = (فراغ *)ffi_go_بستگی_ASM;
        بستگی->cif = cif;
        بستگی->fun = fun;
        أرجع FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      افتراضي:
        أرجع FFI_BAD_ABI;
        اكسر;
    }
  أرجع FFI_OK;
}
#پایان_اگر

ساكن فراغ
flush_icache(حرف *addr)
{
#اگر_تعریف_نشده _AIX
  __asm__ متطاير (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#پایان_اگر
}

ساكن فراغ
flush_range(حرف * addr1, صحيح اندازه)
{
#تعریف_کن MIN_سطر_اندازه 32
  صحيح i;
  لكل (i = 0; i < اندازه; i += MIN_سطر_اندازه)
    flush_icache(addr1+i);
  flush_icache(addr1+اندازه-1);
}

تعريفنوع اتحاد
{
  عائم f;
  مزدوج d;
} ffi_dblfl;

صحيح
ffi_بستگی_helper_DARWIN (ffi_بستگی *, فراغ *,
			   بیعلامت دراز *, ffi_dblfl *);

#إذا defined (FFI_GO_CLOSURES)
صحيح
ffi_go_بستگی_helper_DARWIN (ffi_go_بستگی*, فراغ *,
			      بیعلامت دراز *, ffi_dblfl *);
#پایان_اگر

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ساكن صحيح
ffi_بستگی_helper_common (ffi_cif* cif,
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_داده, فراغ *rvalue,
			   بیعلامت دراز *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  تعريفنوع مزدوج ldbits[2];

  اتحاد ldu
  {
    ldbits lb;
    دراز مزدوج ld;
  };

  فراغ **          avalue;
  ffi_type **      arg_types;
  دراز             i, avn;
  ffi_dblfl *      انتها_pfr = pfr + رقم_FPR_ARG_REGISTERS;
  بیعلامت         اندازه_al;
  صحيح              struct_ret_by_مقدار = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  بیعلامت دراز *  pgr0 = pgr;
#إذا defined(POWERPC_DARWIN64)
  بیعلامت 	   fpsused = 0;
#پایان_اگر

  avalue = alloca (cif->nargs * حجم(فراغ *));

  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#إذا defined(POWERPC_DARWIN64)
      إذا (!darwin64_struct_ret_by_مقدار_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (فراغ *) *pgr;
	  pgr++;
	}
      وإلا
	struct_ret_by_مقدار = 1;
#وإلاإذا defined(DARWIN_PPC)
      إذا (cif->rtype->اندازه > 4)
	{
	  rvalue = (فراغ *) *pgr;
	  pgr++;
	}
      وإلا
	struct_ret_by_مقدار = 1;
#وإلا /* assume we return by ref.  */
      rvalue = (فراغ *) *pgr;
      pgr++;
#پایان_اگر
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
#پایان_اگر
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
#إذا  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 6;
#وإلا
	  avalue[i] = (حرف *) pgr + 2;
#پایان_اگر
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
#إذا  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 4;
#وإلا
	حالة FFI_TYPE_اشاره_گر:
	  avalue[i] = pgr;
#پایان_اگر
	  pgr++;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  اندازه_al = arg_types[i]->اندازه;
#إذا defined(POWERPC_DARWIN64)
	  pgr = (بیعلامت دراز *)FFI_ALIGN((حرف *)pgr, arg_types[i]->alignment);
	  إذا (اندازه_al < 3 || اندازه_al == 4)
	    {
	      avalue[i] = ((حرف *)pgr)+8-اندازه_al;
	      إذا (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < رقم_FPR_ARG_REGISTERS)
		{
		  *(عائم *)pgr = (عائم) *(مزدوج *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  وإلا 
	    {
	      إذا (اندازه_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_مقصد_mem (arg_types[i], (حرف *)pgr,
						   (مزدوج *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (اندازه_al + 7) / 8;
#وإلا
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  إذا (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    اندازه_al = FFI_ALIGN(arg_types[i]->اندازه, 8);
#  إذا defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (اندازه_al + 7) / 8;
#  وإلا
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  إذا (اندازه_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (حرف*) pgr + 4 - اندازه_al;
	  وإلا
	    avalue[i] = pgr;
	  pgr += (اندازه_al + 3) / 4;
#  endif
#پایان_اگر
	  اكسر;

	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
#إذا  defined(POWERPC64)
	حالة FFI_TYPE_اشاره_گر:
	  avalue[i] = pgr;
	  pgr++;
	  اكسر;
#وإلا
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  اكسر;
#پایان_اگر

	حالة FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  إذا (pfr < انتها_pfr)
	    {
	      مزدوج موقت = pfr->d;
	      pfr->f = (عائم) موقت;
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
	  إذا (pfr < انتها_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وإلا
	    {
	      avalue[i] = pgr;
	    }
#اگر_تعریف_شده POWERPC64
	  pgr++;
#وإلا
	  pgr += 2;
#پایان_اگر
	  اكسر;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالة FFI_TYPE_LONGDOUBLE:
#اگر_تعریف_شده POWERPC64
	  إذا (pfr + 1 < انتها_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  وإلا
	    {
	      إذا (pfr < انتها_pfr)
		{
		  *pgr = *(بیعلامت دراز *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#وإلا  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  إذا (pfr + 1 < انتها_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  وإلا إذا (pfr + 1 == انتها_pfr)
	    {
	      اتحاد ldu موقت_ld;
	      کپی_حافظه (&موقت_ld.lb[0], pfr, حجم(ldbits));
	      کپی_حافظه (&موقت_ld.lb[1], pgr + 2, حجم(ldbits));
	      avalue[i] = &موقت_ld.ld;
	      pfr++;
	    }
	  وإلا
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#پایان_اگر  /* POWERPC64 */
	  اكسر;
#پایان_اگر
	افتراضي:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_داده);

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
      إذا (struct_ret_by_مقدار)
	{
	  *pgr0 = (بیعلامت دراز) cif->rtype;
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
#پایان_اگر
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
    حالة FFI_TYPE_اشاره_گر:
      أرجع PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      أرجع PPC_LD_I64;
    افتراضي:
      abort();
    }
}

صحيح
ffi_بستگی_helper_DARWIN (ffi_بستگی *بستگی, فراغ *rvalue,
			   بیعلامت دراز *pgr, ffi_dblfl *pfr)
{
  أرجع ffi_بستگی_helper_common (بستگی->cif, بستگی->fun,
				    بستگی->user_داده, rvalue, pgr, pfr);
}

#إذا defined (FFI_GO_CLOSURES)
صحيح
ffi_go_بستگی_helper_DARWIN (ffi_go_بستگی *بستگی, فراغ *rvalue,
			      بیعلامت دراز *pgr, ffi_dblfl *pfr)
{
  أرجع ffi_بستگی_helper_common (بستگی->cif, بستگی->fun,
				    بستگی, rvalue, pgr, pfr);
}
#پایان_اگر

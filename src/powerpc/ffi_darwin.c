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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>


ساخت ffi_aix_trampoline_struct {
    خالی * رمز_اشارہ_گر;	/* Pointer to ffi_closure_ASM */
    خالی * toc;			/* TOC */
    خالی * static_chain;	/* Pointer to closure */
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

#اگر_غیر_تعریف_شدہ POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#ورنہ
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#اگر_ختم

خارجی خالی ffi_احاطہ_ASM (خالی);

#اگر defined (FFI_GO_CLOSURES)
خارجی خالی ffi_go_احاطہ_ASM (خالی);
#اگر_ختم

شمار {
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
شمار {
  عدد_GPR_ARG_REGISTERS = 8,
  عدد_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

شمار { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#اگر defined(POWERPC_DARWIN64)
ساکن خالی
darwin64_pass_struct_by_قدر 
  (ffi_type *, حرف *, غیرعلامتی, غیرعلامتی *, دوہرا **, غیرعلامتی طویل **);
#اگر_ختم

/* This depends on GPR_SIZE = sizeof (unsigned long) */

خالی
ffi_prep_دلائل (extended_cif *ecif, غیرعلامتی طویل *ثابت ڈھیر)
{
  ثابت غیرعلامتی بائٹیں = ecif->cif->بائٹیں;
  ثابت غیرعلامتی flags = ecif->cif->flags;
  ثابت غیرعلامتی nargs = ecif->cif->nargs;
#اگر !defined(POWERPC_DARWIN64) 
  ثابت ffi_abi abi = ecif->cif->abi;
#اگر_ختم

  /* 'stacktop' points at the previous backchain pointer.  */
  غیرعلامتی طویل *ثابت stacktop = ڈھیر + (بائٹیں / حجم(غیرعلامتی طویل));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  دوہرا *fpr_بنیاد = (دوہرا *) (stacktop - ASM_NEEDS_REGISTERS) - عدد_FPR_ARG_REGISTERS;
  صحیح gp_گنتی = 0, fparg_گنتی = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  غیرعلامتی طویل *اگلا_arg = ڈھیر + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  صحیح i;
  دوہرا double_tmp;
  خالی **p_argv = ecif->avalue;
  غیرعلامتی طویل gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#اگر !defined(POWERPC_DARWIN64) 
  حرف *dest_cpy;
#اگر_ختم
  غیرعلامتی سائز_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((غیرعلامتی) (حرف *) ڈھیر & 0xF) == 0);
  FFI_ASSERT(((غیرعلامتی) (حرف *) stacktop & 0xF) == 0);
  FFI_ASSERT((بائٹیں & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  اگر (flags & FLAG_RETVAL_REFERENCE)
    *اگلا_arg++ = (غیرعلامتی طویل) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  برائے (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      انتخاب ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	حالت FFI_TYPE_FLOAT:
	  double_tmp = *(اعشاری *) *p_argv;
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_بنیاد++ = double_tmp;
#اگر defined(POWERPC_DARWIN)
	  *(اعشاری *)اگلا_arg = *(اعشاری *) *p_argv;
#ورنہ
	  *(دوہرا *)اگلا_arg = double_tmp;
#اگر_ختم
	  اگلا_arg++;
	  gp_گنتی++;
	  fparg_گنتی++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  double_tmp = *(دوہرا *) *p_argv;
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_بنیاد++ = double_tmp;
	  *(دوہرا *)اگلا_arg = double_tmp;
#اگر_تعریف_شدہ POWERPC64
	  اگلا_arg++;
	  gp_گنتی++;
#ورنہ
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#اگر_ختم
	  fparg_گنتی++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  توڑو;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالت FFI_TYPE_LONGDOUBLE:
#  اگر defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *(طویل دوہرا *) fpr_بنیاد++ = *(طویل دوہرا *) *p_argv;
	  ورنہ
	    *(طویل دوہرا *) اگلا_arg = *(طویل دوہرا *) *p_argv;
	  اگلا_arg += 2;
	  fparg_گنتی += 2;
#  ورنہ
	  double_tmp = ((دوہرا *) *p_argv)[0];
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_بنیاد++ = double_tmp;
	  *(دوہرا *) اگلا_arg = double_tmp;
#    اگر defined(POWERPC_DARWIN64)
	  اگلا_arg++;
	  gp_گنتی++;
#    ورنہ
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#    endif
	  fparg_گنتی++;
	  double_tmp = ((دوہرا *) *p_argv)[1];
	  اگر (fparg_گنتی < عدد_FPR_ARG_REGISTERS)
	    *fpr_بنیاد++ = double_tmp;
	  *(دوہرا *) اگلا_arg = double_tmp;
#    اگر defined(POWERPC_DARWIN64)
	  اگلا_arg++;
	  gp_گنتی++;
#    ورنہ
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#    endif
	  fparg_گنتی++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  توڑو;
#اگر_ختم
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#اگر_تعریف_شدہ POWERPC64
	  gprvalue = *(طویل طویل *) *p_argv;
	  جاؤ putgpr;
#ورنہ
	  *(طویل طویل *) اگلا_arg = *(طویل طویل *) *p_argv;
	  اگلا_arg += 2;
	  gp_گنتی += 2;
#اگر_ختم
	  توڑو;
	حالت FFI_TYPE_اشارہ_گر:
	  gprvalue = *(غیرعلامتی طویل *) *p_argv;
	  جاؤ putgpr;
	حالت FFI_TYPE_UINT8:
	  gprvalue = *(غیرعلامتی حرف *) *p_argv;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT8:
	  gprvalue = *(علامتی حرف *) *p_argv;
	  جاؤ putgpr;
	حالت FFI_TYPE_UINT16:
	  gprvalue = *(غیرعلامتی مختصر *) *p_argv;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT16:
	  gprvalue = *(علامتی مختصر *) *p_argv;
	  جاؤ putgpr;

	حالت FFI_TYPE_STRUCT:
	  سائز_al = (*ptr)->سائز;
#اگر defined(POWERPC_DARWIN64)
	  اگلا_arg = (غیرعلامتی طویل *)FFI_ALIGN((حرف *)اگلا_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_قدر (*ptr, (حرف *) *p_argv, 
					 (غیرعلامتی) سائز_al,
					 (غیرعلامتی صحیح *) &fparg_گنتی,
					 &fpr_بنیاد, &اگلا_arg);
#ورنہ
	  dest_cpy = (حرف *) اگلا_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  اگر ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    سائز_al = FFI_ALIGN((*ptr)->سائز, 8);

#  اگر defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  حافظہ_نقل ((حرف *) dest_cpy, (حرف *) *p_argv, سائز_al);
	  اگلا_arg += (سائز_al + 7) / 8;
#  ورنہ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  اگر (سائز_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - سائز_al;

	  حافظہ_نقل((حرف *) dest_cpy, (حرف *) *p_argv, سائز_al);
	  اگلا_arg += (سائز_al + 3) / 4;
#  endif
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  gprvalue = *(علامتی صحیح *) *p_argv;
	  جاؤ putgpr;

	حالت FFI_TYPE_UINT32:
	  gprvalue = *(غیرعلامتی صحیح *) *p_argv;
	putgpr:
	  *اگلا_arg++ = gprvalue;
	  gp_گنتی++;
	  توڑو;
	طےشدہ:
	  توڑو;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#اگر defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ساکن خالی
darwin64_جائزہ_struct_for_floats (ffi_type *s, غیرعلامتی *nfpr)
{
  صحیح i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      ffi_type *p = s->elements[i];
      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    darwin64_جائزہ_struct_for_floats (p, nfpr);
	    توڑو;
	  حالت FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    توڑو;
	  حالت FFI_TYPE_DOUBLE:
	  حالت FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    توڑو;
	  طےشدہ:
	    توڑو;    
	}
    }
}

ساکن صحیح
darwin64_struct_سائز_exceeds_gprs_p (ffi_type *s, حرف *src, غیرعلامتی *nfpr)
{
  غیرعلامتی struct_انحراف=0, i;

  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      حرف *شے_بنیاد;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_انحراف = FFI_ALIGN(struct_انحراف, p->alignment);

      شے_بنیاد = src + struct_انحراف;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    اگر (darwin64_struct_سائز_exceeds_gprs_p (p, شے_بنیاد, nfpr))
	      واپس 1;
	    توڑو;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      واپس 1;
	    (*nfpr) += 1;
	    شے_بنیاد += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      واپس 1;
	    (*nfpr) += 1;
	    توڑو;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nfpr >= عدد_FPR_ARG_REGISTERS)
	      واپس 1;
	    (*nfpr) += 1;
	    توڑو;
	  طےشدہ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    اگر ((غیرعلامتی طویل)شے_بنیاد >= 8*8) 
	      واپس 1;
	    توڑو;    
	}
      /* now count the size of what we just used.  */
      struct_انحراف += p->سائز;
    }
  واپس 0;
}

/* Can this struct be returned by value?  */
صحیح 
darwin64_struct_ret_by_قدر_p (ffi_type *s)
{
  غیرعلامتی nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  اگر (s->سائز > 168)
    واپس 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_جائزہ_struct_for_floats (s, &nfp);
  اگر (nfp > 13)
    واپس 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  اگر (s->سائز <= 64)
    واپس 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  اگر (darwin64_struct_سائز_exceeds_gprs_p (s, تہی, &nfp))
    واپس 0;
  
  واپس 1;
}

خالی
darwin64_pass_struct_floats (ffi_type *s, حرف *src, 
			     غیرعلامتی *nfpr, دوہرا **fprs)
{
  صحیح i;
  دوہرا *fpr_بنیاد = *fprs;
  غیرعلامتی struct_انحراف = 0;

  /* We don't assume anything about the alignment of the source.  */
  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      حرف *شے_بنیاد;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_انحراف = FFI_ALIGN(struct_انحراف, p->alignment);
      شے_بنیاد = src + struct_انحراف;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, شے_بنیاد, nfpr,
					   &fpr_بنیاد);
	    توڑو;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_بنیاد++ = *(دوہرا *)شے_بنیاد;
	    (*nfpr) += 1;
	    شے_بنیاد += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_بنیاد++ = *(دوہرا *)شے_بنیاد;
	    (*nfpr) += 1;
	    توڑو;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nfpr < عدد_FPR_ARG_REGISTERS)
	      *fpr_بنیاد++ = (دوہرا) *(اعشاری *)شے_بنیاد;
	    (*nfpr) += 1;
	    توڑو;
	  طےشدہ:
	    توڑو;    
	}
      /* now count the size of what we just used.  */
      struct_انحراف += p->سائز;
    }
  /* Update the scores.  */
  *fprs = fpr_بنیاد;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ساکن خالی
darwin64_pass_struct_by_قدر (ffi_type *s, حرف *src, غیرعلامتی سائز,
			       غیرعلامتی *nfpr, دوہرا **fprs, غیرعلامتی طویل **arg)
{
  غیرعلامتی طویل *اگلا_arg = *arg;
  حرف *dest_cpy = (حرف *)اگلا_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  اگر (!سائز)
    واپس;

  /* First... special cases.  */
  اگر (سائز < 3
      || (سائز == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *اگلا_arg = 0UL; 
      dest_cpy += 8 - سائز;
      حافظہ_نقل ((حرف *) dest_cpy, src, سائز);
      اگلا_arg++;
    }
  ورنہ اگر (سائز == 16)
    {
      حافظہ_نقل ((حرف *) dest_cpy, src, سائز);
      اگلا_arg += 2;
    }
  ورنہ
    {
      /* now the general case, we consider embedded floats.  */
      حافظہ_نقل ((حرف *) dest_cpy, src, سائز);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      اگلا_arg += (سائز+7)/8;
    }
    
  *arg = اگلا_arg;
}

دوہرا *
darwin64_struct_floats_مقصد_mem (ffi_type *s, حرف *dest, دوہرا *fprs, غیرعلامتی *nf)
{
  صحیح i;
  غیرعلامتی struct_انحراف = 0;

  /* We don't assume anything about the alignment of the source.  */
  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      حرف *شے_بنیاد;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_انحراف = FFI_ALIGN(struct_انحراف, p->alignment);
      شے_بنیاد = dest + struct_انحراف;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_مقصد_mem (p, شے_بنیاد, fprs, nf);
	    توڑو;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(دوہرا *)شے_بنیاد = *fprs++ ;
		(*nf) += 1;
	      }
	    شے_بنیاد += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(دوہرا *)شے_بنیاد = *fprs++ ;
		(*nf) += 1;
	      }
	    توڑو;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nf < عدد_FPR_ARG_REGISTERS)
	      {
		*(اعشاری *)شے_بنیاد = (اعشاری) *fprs++ ;
		(*nf) += 1;
	      }
	    توڑو;
	  طےشدہ:
	    توڑو;    
	}
      /* now count the size of what we just used.  */
      struct_انحراف += p->سائز;
    }
  واپس fprs;
}

#اگر_ختم

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ساکن خالی
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  صحیح i;

  اگر (s->type != FFI_TYPE_STRUCT)
    واپس;

  s->سائز = 0;
  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      ffi_type *p;
      صحیح align;
      
      p = s->elements[i];
      اگر (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#اگر defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#ورنہ
      /* Natural alignment for the first item... */
      اگر (i == 0)
	align = p->alignment;
      ورنہ اگر (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      ورنہ
	/* .. or align is 4.  */
	align = 4;
#اگر_ختم
      /* Pad, if necessary, before adding the current item.  */
      s->سائز = FFI_ALIGN(s->سائز, align) + p->سائز;
    }
  
  s->سائز = FFI_ALIGN(s->سائز, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  اگر (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ساکن صحیح
aix_adjust_aggregate_sizes (ffi_type *s, صحیح outer_most_type_or_پہلا_member)
{
  صحیح i, nested_پہلا_member=0, final_align, rc=0;

  اگر (s->type != FFI_TYPE_STRUCT)
    واپس 0;

  s->سائز = 0;
  برائے (i = 0; s->elements[i] != تہی; i++)
    {
      ffi_type p;
      صحیح align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      اگر (i == 0)
        nested_پہلا_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_پہلا_member);
      ورنہ
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      اگر (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->سائز = FFI_ALIGN(s->سائز, align) + p.سائز;
    }

  final_align=s->alignment;
  اگر ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_پہلا_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      اگر (outer_most_type_or_پہلا_member)
        s->alignment=final_align;
  }

  s->سائز = FFI_ALIGN(s->سائز, final_align);
  واپس rc;
}

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  غیرعلامتی i;
  ffi_type **ptr;
  غیرعلامتی بائٹیں;
  غیرعلامتی fparg_گنتی = 0, intarg_گنتی = 0;
  غیرعلامتی flags = 0;
  غیرعلامتی سائز_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  اگر (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      برائے (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  اگر (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      برائے (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  بائٹیں = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * حجم(غیرعلامتی طویل);

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
  انتخاب (cif->rtype->type)
    {

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_بٹ;
      flags |= FLAG_RETURNS_FP;
      توڑو;
#اگر_ختم

    حالت FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٹ;
      /* Fall through.  */
    حالت FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      توڑو;

    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
#اگر_تعریف_شدہ POWERPC64
    حالت FFI_TYPE_اشارہ_گر:
#اگر_ختم
      flags |= FLAG_RETURNS_64_بٹ;
      توڑو;

    حالت FFI_TYPE_STRUCT:
#اگر defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	اگر (darwin64_struct_ret_by_قدر_p (cif->rtype))
	  {
	    غیرعلامتی nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    اگر (cif->rtype->سائز != 16)
	      darwin64_جائزہ_struct_for_floats (cif->rtype, &nfpr) ;
	    ورنہ
	      flags |= FLAG_RETURNS_128_بٹ;
	    /* Will be 0 for 16byte struct.  */
	    اگر (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	ورنہ /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_گنتی++;
	  }
      }
#وگرنہ_اگر defined(DARWIN_PPC)
      اگر (cif->rtype->سائز <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      ورنہ /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_گنتی++;
	}
#ورنہ /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_گنتی++;
#اگر_ختم
      توڑو;
    حالت FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      توڑو;

    طےشدہ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      توڑو;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      غیرعلامتی align_الفاظ;
      انتخاب ((*ptr)->type)
	{
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	  fparg_گنتی++;
#اگر !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  اگر (fparg_گنتی > عدد_FPR_ARG_REGISTERS
	      && (intarg_گنتی & 0x01) != 0)
	    intarg_گنتی++;
#اگر_ختم
	  توڑو;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
	  fparg_گنتی += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  اگر (fparg_گنتی >= عدد_FPR_ARG_REGISTERS)
#اگر defined (POWERPC64)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, 2);
#ورنہ
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, 4);
#اگر_ختم
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#اگر defined(POWERPC64)
	  intarg_گنتی++;
#ورنہ
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  اگر (intarg_گنتی == عدد_GPR_ARG_REGISTERS-1
	      || (intarg_گنتی >= عدد_GPR_ARG_REGISTERS 
	          && (intarg_گنتی & 0x01) != 0))
	    intarg_گنتی++;
	  intarg_گنتی += 2;
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  سائز_al = (*ptr)->سائز;
#اگر defined(POWERPC_DARWIN64)
	  align_الفاظ = (*ptr)->alignment >> 3;
	  اگر (align_الفاظ)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, align_الفاظ);
	  /* Base size of the struct.  */
	  intarg_گنتی += (سائز_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  اگر (سائز_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_جائزہ_struct_for_floats (*ptr, &fparg_گنتی) ;
#ورنہ
	  align_الفاظ = (*ptr)->alignment >> 2;
	  اگر (align_الفاظ)
	    intarg_گنتی = FFI_ALIGN(intarg_گنتی, align_الفاظ);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_گنتی += (سائز_al + 7) / 8;
#  ورنہ
	  intarg_گنتی += (سائز_al + 3) / 4;
#  endif
#اگر_ختم
	  توڑو;

	طےشدہ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_گنتی++;
	  توڑو;
	}
    }

  اگر (fparg_گنتی != 0)
    flags |= FLAG_FP_ARGUMENTS;

#اگر defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  اگر (fparg_گنتی != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    بائٹیں += عدد_FPR_ARG_REGISTERS * حجم(دوہرا);
#ورنہ
  /* Space for the FPR registers, if needed.  */
  اگر (fparg_گنتی != 0)
    بائٹیں += عدد_FPR_ARG_REGISTERS * حجم(دوہرا);
#اگر_ختم

  /* Stack space.  */
#اگر_تعریف_شدہ POWERPC64
  اگر ((intarg_گنتی + fparg_گنتی) > عدد_GPR_ARG_REGISTERS)
    بائٹیں += (intarg_گنتی + fparg_گنتی) * حجم(طویل);
#ورنہ
  اگر ((intarg_گنتی + 2 * fparg_گنتی) > عدد_GPR_ARG_REGISTERS)
    بائٹیں += (intarg_گنتی + 2 * fparg_گنتی) * حجم(طویل);
#اگر_ختم
  ورنہ
    بائٹیں += عدد_GPR_ARG_REGISTERS * حجم(طویل);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائٹیں = FFI_ALIGN(بائٹیں, 16) ;

  cif->flags = flags;
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

خارجی خالی ffi_call_AIX(extended_cif *, طویل, غیرعلامتی, غیرعلامتی *,
			 خالی (*fn)(خالی), خالی (*fn2)(خالی));

#اگر defined (FFI_GO_CLOSURES)
خارجی خالی ffi_call_go_AIX(extended_cif *, طویل, غیرعلامتی, غیرعلامتی *,
			    خالی (*fn)(خالی), خالی (*fn2)(خالی), خالی *احاطہ);
#اگر_ختم

خارجی خالی ffi_call_DARWIN(extended_cif *, طویل, غیرعلامتی, غیرعلامتی *,
			    خالی (*fn)(خالی), خالی (*fn2)(خالی), ffi_type*);

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر ((rvalue == تہی) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_AIX:
      ffi_call_AIX(&ecif, -(طویل)cif->بائٹیں, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_دلائل));
      توڑو;
    حالت FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(طویل)cif->بائٹیں, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_دلائل), cif->rtype);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

#اگر defined (FFI_GO_CLOSURES)
خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue,
	     خالی *احاطہ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر ((rvalue == تہی) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_AIX:
      ffi_call_go_AIX(&ecif, -(طویل)cif->بائٹیں, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_دلائل), احاطہ);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}
#اگر_ختم

ساکن خالی flush_icache(حرف *);
ساکن خالی flush_range(حرف *, صحیح);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

قسمتعریف ساخت aix_fd_struct {
  خالی *رمز_اشارہ_گر;
  خالی *toc;
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
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp;
  ساخت ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  انتخاب (cif->abi)
    {
      حالت FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
#اگر defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((غیرعلامتی طویل *)&tramp[2]) = (غیرعلامتی طویل) ffi_احاطہ_ASM; /* function  */
	*((غیرعلامتی طویل *)&tramp[4]) = (غیرعلامتی طویل) codeloc; /* context  */
#ورنہ
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (غیرعلامتی طویل) ffi_احاطہ_ASM; /* function  */
	tramp[3] = (غیرعلامتی طویل) codeloc; /* context  */
#اگر_ختم
	احاطہ->cif = cif;
	احاطہ->fun = fun;
	احاطہ->user_ڈیٹا = user_ڈیٹا;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_سائز);

	توڑو;

    حالت FFI_AIX:

      tramp_aix = (ساخت ffi_aix_trampoline_struct *) (احاطہ->tramp);
      fd = (aix_fd *)(خالی *)ffi_احاطہ_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->رمز_اشارہ_گر = fd->رمز_اشارہ_گر;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      احاطہ->cif = cif;
      احاطہ->fun = fun;
      احاطہ->user_ڈیٹا = user_ڈیٹا;
      توڑو;

    طےشدہ:
      واپس FFI_BAD_ABI;
      توڑو;
    }
  واپس FFI_OK;
}

#اگر defined (FFI_GO_CLOSURES)
ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ,
		     ffi_cif* cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
  انتخاب (cif->abi)
    {
      حالت FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        احاطہ->tramp = (خالی *)ffi_go_احاطہ_ASM;
        احاطہ->cif = cif;
        احاطہ->fun = fun;
        واپس FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      طےشدہ:
        واپس FFI_BAD_ABI;
        توڑو;
    }
  واپس FFI_OK;
}
#اگر_ختم

ساکن خالی
flush_icache(حرف *addr)
{
#اگر_غیر_تعریف_شدہ _AIX
  __asm__ متلون (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#اگر_ختم
}

ساکن خالی
flush_range(حرف * addr1, صحیح سائز)
{
#تعریف_کرو MIN_سطر_سائز 32
  صحیح i;
  برائے (i = 0; i < سائز; i += MIN_سطر_سائز)
    flush_icache(addr1+i);
  flush_icache(addr1+سائز-1);
}

قسمتعریف اتحاد
{
  اعشاری f;
  دوہرا d;
} ffi_dblfl;

صحیح
ffi_احاطہ_helper_DARWIN (ffi_احاطہ *, خالی *,
			   غیرعلامتی طویل *, ffi_dblfl *);

#اگر defined (FFI_GO_CLOSURES)
صحیح
ffi_go_احاطہ_helper_DARWIN (ffi_go_احاطہ*, خالی *,
			      غیرعلامتی طویل *, ffi_dblfl *);
#اگر_ختم

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ساکن صحیح
ffi_احاطہ_helper_common (ffi_cif* cif,
			   خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			   خالی *user_ڈیٹا, خالی *rvalue,
			   غیرعلامتی طویل *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  قسمتعریف دوہرا ldbits[2];

  اتحاد ldu
  {
    ldbits lb;
    طویل دوہرا ld;
  };

  خالی **          avalue;
  ffi_type **      arg_types;
  طویل             i, avn;
  ffi_dblfl *      انتہا_pfr = pfr + عدد_FPR_ARG_REGISTERS;
  غیرعلامتی         سائز_al;
  صحیح              struct_ret_by_قدر = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  غیرعلامتی طویل *  pgr0 = pgr;
#اگر defined(POWERPC_DARWIN64)
  غیرعلامتی 	   fpsused = 0;
#اگر_ختم

  avalue = alloca (cif->nargs * حجم(خالی *));

  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#اگر defined(POWERPC_DARWIN64)
      اگر (!darwin64_struct_ret_by_قدر_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (خالی *) *pgr;
	  pgr++;
	}
      ورنہ
	struct_ret_by_قدر = 1;
#وگرنہ_اگر defined(DARWIN_PPC)
      اگر (cif->rtype->سائز > 4)
	{
	  rvalue = (خالی *) *pgr;
	  pgr++;
	}
      ورنہ
	struct_ret_by_قدر = 1;
#ورنہ /* assume we return by ref.  */
      rvalue = (خالی *) *pgr;
      pgr++;
#اگر_ختم
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  جب (i < avn)
    {
      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
#اگر  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 7;
#ورنہ
	  avalue[i] = (حرف *) pgr + 3;
#اگر_ختم
	  pgr++;
	  توڑو;

	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
#اگر  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 6;
#ورنہ
	  avalue[i] = (حرف *) pgr + 2;
#اگر_ختم
	  pgr++;
	  توڑو;

	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
#اگر  defined(POWERPC64)
	  avalue[i] = (حرف *) pgr + 4;
#ورنہ
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = pgr;
#اگر_ختم
	  pgr++;
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  سائز_al = arg_types[i]->سائز;
#اگر defined(POWERPC_DARWIN64)
	  pgr = (غیرعلامتی طویل *)FFI_ALIGN((حرف *)pgr, arg_types[i]->alignment);
	  اگر (سائز_al < 3 || سائز_al == 4)
	    {
	      avalue[i] = ((حرف *)pgr)+8-سائز_al;
	      اگر (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < عدد_FPR_ARG_REGISTERS)
		{
		  *(اعشاری *)pgr = (اعشاری) *(دوہرا *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  ورنہ 
	    {
	      اگر (سائز_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_مقصد_mem (arg_types[i], (حرف *)pgr,
						   (دوہرا *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (سائز_al + 7) / 8;
#ورنہ
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  اگر (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    سائز_al = FFI_ALIGN(arg_types[i]->سائز, 8);
#  اگر defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (سائز_al + 7) / 8;
#  ورنہ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  اگر (سائز_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (حرف*) pgr + 4 - سائز_al;
	  ورنہ
	    avalue[i] = pgr;
	  pgr += (سائز_al + 3) / 4;
#  endif
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
#اگر  defined(POWERPC64)
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = pgr;
	  pgr++;
	  توڑو;
#ورنہ
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr < انتہا_pfr)
	    {
	      دوہرا عارضی = pfr->d;
	      pfr->f = (اعشاری) عارضی;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ورنہ
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr < انتہا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ورنہ
	    {
	      avalue[i] = pgr;
	    }
#اگر_تعریف_شدہ POWERPC64
	  pgr++;
#ورنہ
	  pgr += 2;
#اگر_ختم
	  توڑو;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالت FFI_TYPE_LONGDOUBLE:
#اگر_تعریف_شدہ POWERPC64
	  اگر (pfr + 1 < انتہا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  ورنہ
	    {
	      اگر (pfr < انتہا_pfr)
		{
		  *pgr = *(غیرعلامتی طویل *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#ورنہ  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr + 1 < انتہا_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  ورنہ اگر (pfr + 1 == انتہا_pfr)
	    {
	      اتحاد ldu عارضی_ld;
	      حافظہ_نقل (&عارضی_ld.lb[0], pfr, حجم(ldbits));
	      حافظہ_نقل (&عارضی_ld.lb[1], pgr + 2, حجم(ldbits));
	      avalue[i] = &عارضی_ld.ld;
	      pfr++;
	    }
	  ورنہ
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#اگر_ختم  /* POWERPC64 */
	  توڑو;
#اگر_ختم
	طےشدہ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      واپس PPC_LD_NONE;
    حالت FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      اگر (struct_ret_by_قدر)
	{
	  *pgr0 = (غیرعلامتی طویل) cif->rtype;
	  واپس PPC_LD_STRUCT;
	}
      واپس PPC_LD_NONE;
    حالت FFI_TYPE_FLOAT:
      واپس PPC_LD_F32;
    حالت FFI_TYPE_DOUBLE:
      واپس PPC_LD_F64;
#اگر FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      واپس PPC_LD_F128;
#اگر_ختم
    حالت FFI_TYPE_UINT8:
      واپس PPC_LD_U8;
    حالت FFI_TYPE_SINT8:
      واپس PPC_LD_S8;
    حالت FFI_TYPE_UINT16:
      واپس PPC_LD_U16;
    حالت FFI_TYPE_SINT16:
      واپس PPC_LD_S16;
    حالت FFI_TYPE_UINT32:
      واپس PPC_LD_U32;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      واپس PPC_LD_S32;
    حالت FFI_TYPE_اشارہ_گر:
      واپس PPC_LD_PTR;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      واپس PPC_LD_I64;
    طےشدہ:
      abort();
    }
}

صحیح
ffi_احاطہ_helper_DARWIN (ffi_احاطہ *احاطہ, خالی *rvalue,
			   غیرعلامتی طویل *pgr, ffi_dblfl *pfr)
{
  واپس ffi_احاطہ_helper_common (احاطہ->cif, احاطہ->fun,
				    احاطہ->user_ڈیٹا, rvalue, pgr, pfr);
}

#اگر defined (FFI_GO_CLOSURES)
صحیح
ffi_go_احاطہ_helper_DARWIN (ffi_go_احاطہ *احاطہ, خالی *rvalue,
			      غیرعلامتی طویل *pgr, ffi_dblfl *pfr)
{
  واپس ffi_احاطہ_helper_common (احاطہ->cif, احاطہ->fun,
				    احاطہ, rvalue, pgr, pfr);
}
#اگر_ختم

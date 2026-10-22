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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>


నిర్మాణం ffi_aix_trampoline_struct {
    శూన్య * సంకేతం_సూచిక;	/* Pointer to ffi_closure_ASM */
    శూన్య * toc;			/* TOC */
    శూన్య * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#నిర్వచించు PPC_LD_NONE		0
#నిర్వచించు PPC_LD_R3		1
#నిర్వచించు PPC_LD_R3R4		2
#నిర్వచించు PPC_LD_F32		3
#నిర్వచించు PPC_LD_F64		4
#నిర్వచించు PPC_LD_F128		5
#నిర్వచించు PPC_LD_U8		6
#నిర్వచించు PPC_LD_S8		7
#నిర్వచించు PPC_LD_U16		8
#నిర్వచించు PPC_LD_S16		9

#నిర్వచితం_కాకపోతే POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#లేకపోతే
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#అయితే_ముగింపు

బాహ్య శూన్య ffi_సంవృతం_ASM (శూన్య);

#ఒకవేళ defined (FFI_GO_CLOSURES)
బాహ్య శూన్య ffi_go_సంవృతం_ASM (శూన్య);
#అయితే_ముగింపు

గణన {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128_బిట్లు	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64_బిట్లు	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
గణన {
  అంకె_GPR_ARG_REGISTERS = 8,
  అంకె_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

గణన { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#ఒకవేళ defined(POWERPC_DARWIN64)
స్థిర శూన్య
darwin64_pass_struct_by_విలువ 
  (ffi_type *, అక్షరం *, గుర్తులేని, గుర్తులేని *, ద్విగుణ **, గుర్తులేని దీర్ఘ **);
#అయితే_ముగింపు

/* This depends on GPR_SIZE = sizeof (unsigned long) */

శూన్య
ffi_prep_వాదనలు (extended_cif *ecif, గుర్తులేని దీర్ఘ *స్థిరరాశి దొంతర)
{
  స్థిరరాశి గుర్తులేని బైట్లు = ecif->cif->బైట్లు;
  స్థిరరాశి గుర్తులేని flags = ecif->cif->flags;
  స్థిరరాశి గుర్తులేని nargs = ecif->cif->nargs;
#ఒకవేళ !defined(POWERPC_DARWIN64) 
  స్థిరరాశి ffi_abi abi = ecif->cif->abi;
#అయితే_ముగింపు

  /* 'stacktop' points at the previous backchain pointer.  */
  గుర్తులేని దీర్ఘ *స్థిరరాశి stacktop = దొంతర + (బైట్లు / పరిమాణం(గుర్తులేని దీర్ఘ));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ద్విగుణ *fpr_పునాది = (ద్విగుణ *) (stacktop - ASM_NEEDS_REGISTERS) - అంకె_FPR_ARG_REGISTERS;
  పూర్ణాంకం gp_లెక్క = 0, fparg_లెక్క = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  గుర్తులేని దీర్ఘ *తదుపరి_arg = దొంతర + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  పూర్ణాంకం i;
  ద్విగుణ double_tmp;
  శూన్య **p_argv = ecif->avalue;
  గుర్తులేని దీర్ఘ gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#ఒకవేళ !defined(POWERPC_DARWIN64) 
  అక్షరం *dest_cpy;
#అయితే_ముగింపు
  గుర్తులేని కొలత_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((గుర్తులేని) (అక్షరం *) దొంతర & 0xF) == 0);
  FFI_ASSERT(((గుర్తులేని) (అక్షరం *) stacktop & 0xF) == 0);
  FFI_ASSERT((బైట్లు & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  ఒకవేళ (flags & FLAG_RETVAL_REFERENCE)
    *తదుపరి_arg++ = (గుర్తులేని దీర్ఘ) (అక్షరం *) ecif->rvalue;

  /* Now for the arguments.  */
  కొరకు (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ఎంపిక ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	స్థితి FFI_TYPE_FLOAT:
	  double_tmp = *(దశాంశం *) *p_argv;
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS)
	    *fpr_పునాది++ = double_tmp;
#ఒకవేళ defined(POWERPC_DARWIN)
	  *(దశాంశం *)తదుపరి_arg = *(దశాంశం *) *p_argv;
#లేకపోతే
	  *(ద్విగుణ *)తదుపరి_arg = double_tmp;
#అయితే_ముగింపు
	  తదుపరి_arg++;
	  gp_లెక్క++;
	  fparg_లెక్క++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  double_tmp = *(ద్విగుణ *) *p_argv;
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS)
	    *fpr_పునాది++ = double_tmp;
	  *(ద్విగుణ *)తదుపరి_arg = double_tmp;
#నిర్వచితమైతే POWERPC64
	  తదుపరి_arg++;
	  gp_లెక్క++;
#లేకపోతే
	  తదుపరి_arg += 2;
	  gp_లెక్క += 2;
#అయితే_ముగింపు
	  fparg_లెక్క++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ఆపు;

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	స్థితి FFI_TYPE_LONGDOUBLE:
#  ఒకవేళ defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS)
	    *(దీర్ఘ ద్విగుణ *) fpr_పునాది++ = *(దీర్ఘ ద్విగుణ *) *p_argv;
	  లేకపోతే
	    *(దీర్ఘ ద్విగుణ *) తదుపరి_arg = *(దీర్ఘ ద్విగుణ *) *p_argv;
	  తదుపరి_arg += 2;
	  fparg_లెక్క += 2;
#  లేకపోతే
	  double_tmp = ((ద్విగుణ *) *p_argv)[0];
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS)
	    *fpr_పునాది++ = double_tmp;
	  *(ద్విగుణ *) తదుపరి_arg = double_tmp;
#    ఒకవేళ defined(POWERPC_DARWIN64)
	  తదుపరి_arg++;
	  gp_లెక్క++;
#    లేకపోతే
	  తదుపరి_arg += 2;
	  gp_లెక్క += 2;
#    endif
	  fparg_లెక్క++;
	  double_tmp = ((ద్విగుణ *) *p_argv)[1];
	  ఒకవేళ (fparg_లెక్క < అంకె_FPR_ARG_REGISTERS)
	    *fpr_పునాది++ = double_tmp;
	  *(ద్విగుణ *) తదుపరి_arg = double_tmp;
#    ఒకవేళ defined(POWERPC_DARWIN64)
	  తదుపరి_arg++;
	  gp_లెక్క++;
#    లేకపోతే
	  తదుపరి_arg += 2;
	  gp_లెక్క += 2;
#    endif
	  fparg_లెక్క++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ఆపు;
#అయితే_ముగింపు
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
#నిర్వచితమైతే POWERPC64
	  gprvalue = *(దీర్ఘ దీర్ఘ *) *p_argv;
	  వెళ్ళు putgpr;
#లేకపోతే
	  *(దీర్ఘ దీర్ఘ *) తదుపరి_arg = *(దీర్ఘ దీర్ఘ *) *p_argv;
	  తదుపరి_arg += 2;
	  gp_లెక్క += 2;
#అయితే_ముగింపు
	  ఆపు;
	స్థితి FFI_TYPE_సూచిక:
	  gprvalue = *(గుర్తులేని దీర్ఘ *) *p_argv;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_UINT8:
	  gprvalue = *(గుర్తులేని అక్షరం *) *p_argv;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT8:
	  gprvalue = *(గుర్తుగల అక్షరం *) *p_argv;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_UINT16:
	  gprvalue = *(గుర్తులేని పొట్టి *) *p_argv;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT16:
	  gprvalue = *(గుర్తుగల పొట్టి *) *p_argv;
	  వెళ్ళు putgpr;

	స్థితి FFI_TYPE_STRUCT:
	  కొలత_al = (*ptr)->కొలత;
#ఒకవేళ defined(POWERPC_DARWIN64)
	  తదుపరి_arg = (గుర్తులేని దీర్ఘ *)FFI_ALIGN((అక్షరం *)తదుపరి_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_విలువ (*ptr, (అక్షరం *) *p_argv, 
					 (గుర్తులేని) కొలత_al,
					 (గుర్తులేని పూర్ణాంకం *) &fparg_లెక్క,
					 &fpr_పునాది, &తదుపరి_arg);
#లేకపోతే
	  dest_cpy = (అక్షరం *) తదుపరి_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  ఒకవేళ ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    కొలత_al = FFI_ALIGN((*ptr)->కొలత, 8);

#  ఒకవేళ defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  జ్ఞాపకం_నకలు ((అక్షరం *) dest_cpy, (అక్షరం *) *p_argv, కొలత_al);
	  తదుపరి_arg += (కొలత_al + 7) / 8;
#  లేకపోతే
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  ఒకవేళ (కొలత_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - కొలత_al;

	  జ్ఞాపకం_నకలు((అక్షరం *) dest_cpy, (అక్షరం *) *p_argv, కొలత_al);
	  తదుపరి_arg += (కొలత_al + 3) / 4;
#  endif
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	  gprvalue = *(గుర్తుగల పూర్ణాంకం *) *p_argv;
	  వెళ్ళు putgpr;

	స్థితి FFI_TYPE_UINT32:
	  gprvalue = *(గుర్తులేని పూర్ణాంకం *) *p_argv;
	putgpr:
	  *తదుపరి_arg++ = gprvalue;
	  gp_లెక్క++;
	  ఆపు;
	అప్రమేయ:
	  ఆపు;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#ఒకవేళ defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
స్థిర శూన్య
darwin64_పరిశీలన_struct_for_floats (ffi_type *s, గుర్తులేని *nfpr)
{
  పూర్ణాంకం i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      ffi_type *p = s->elements[i];
      ఎంపిక (p->type)
	{
	  స్థితి FFI_TYPE_STRUCT:
	    darwin64_పరిశీలన_struct_for_floats (p, nfpr);
	    ఆపు;
	  స్థితి FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ఆపు;
	  స్థితి FFI_TYPE_DOUBLE:
	  స్థితి FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ఆపు;
	  అప్రమేయ:
	    ఆపు;    
	}
    }
}

స్థిర పూర్ణాంకం
darwin64_struct_కొలత_exceeds_gprs_p (ffi_type *s, అక్షరం *src, గుర్తులేని *nfpr)
{
  గుర్తులేని struct_స్థానభ్రంశం=0, i;

  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      అక్షరం *అంశం_పునాది;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ఒకవేళ (i > 0)
        struct_స్థానభ్రంశం = FFI_ALIGN(struct_స్థానభ్రంశం, p->alignment);

      అంశం_పునాది = src + struct_స్థానభ్రంశం;

      ఎంపిక (p->type)
	{
	  స్థితి FFI_TYPE_STRUCT:
	    ఒకవేళ (darwin64_struct_కొలత_exceeds_gprs_p (p, అంశం_పునాది, nfpr))
	      తిరిగి 1;
	    ఆపు;
	  స్థితి FFI_TYPE_LONGDOUBLE:
	    ఒకవేళ (*nfpr >= అంకె_FPR_ARG_REGISTERS)
	      తిరిగి 1;
	    (*nfpr) += 1;
	    అంశం_పునాది += 8;
	  /* FALL THROUGH */
	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (*nfpr >= అంకె_FPR_ARG_REGISTERS)
	      తిరిగి 1;
	    (*nfpr) += 1;
	    ఆపు;
	  స్థితి FFI_TYPE_FLOAT:
	    ఒకవేళ (*nfpr >= అంకె_FPR_ARG_REGISTERS)
	      తిరిగి 1;
	    (*nfpr) += 1;
	    ఆపు;
	  అప్రమేయ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    ఒకవేళ ((గుర్తులేని దీర్ఘ)అంశం_పునాది >= 8*8) 
	      తిరిగి 1;
	    ఆపు;    
	}
      /* now count the size of what we just used.  */
      struct_స్థానభ్రంశం += p->కొలత;
    }
  తిరిగి 0;
}

/* Can this struct be returned by value?  */
పూర్ణాంకం 
darwin64_struct_ret_by_విలువ_p (ffi_type *s)
{
  గుర్తులేని nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  ఒకవేళ (s->కొలత > 168)
    తిరిగి 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_పరిశీలన_struct_for_floats (s, &nfp);
  ఒకవేళ (nfp > 13)
    తిరిగి 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  ఒకవేళ (s->కొలత <= 64)
    తిరిగి 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  ఒకవేళ (darwin64_struct_కొలత_exceeds_gprs_p (s, ఖాళీ, &nfp))
    తిరిగి 0;
  
  తిరిగి 1;
}

శూన్య
darwin64_pass_struct_floats (ffi_type *s, అక్షరం *src, 
			     గుర్తులేని *nfpr, ద్విగుణ **fprs)
{
  పూర్ణాంకం i;
  ద్విగుణ *fpr_పునాది = *fprs;
  గుర్తులేని struct_స్థానభ్రంశం = 0;

  /* We don't assume anything about the alignment of the source.  */
  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      అక్షరం *అంశం_పునాది;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ఒకవేళ (i > 0)
        struct_స్థానభ్రంశం = FFI_ALIGN(struct_స్థానభ్రంశం, p->alignment);
      అంశం_పునాది = src + struct_స్థానభ్రంశం;

      ఎంపిక (p->type)
	{
	  స్థితి FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, అంశం_పునాది, nfpr,
					   &fpr_పునాది);
	    ఆపు;
	  స్థితి FFI_TYPE_LONGDOUBLE:
	    ఒకవేళ (*nfpr < అంకె_FPR_ARG_REGISTERS)
	      *fpr_పునాది++ = *(ద్విగుణ *)అంశం_పునాది;
	    (*nfpr) += 1;
	    అంశం_పునాది += 8;
	  /* FALL THROUGH */
	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (*nfpr < అంకె_FPR_ARG_REGISTERS)
	      *fpr_పునాది++ = *(ద్విగుణ *)అంశం_పునాది;
	    (*nfpr) += 1;
	    ఆపు;
	  స్థితి FFI_TYPE_FLOAT:
	    ఒకవేళ (*nfpr < అంకె_FPR_ARG_REGISTERS)
	      *fpr_పునాది++ = (ద్విగుణ) *(దశాంశం *)అంశం_పునాది;
	    (*nfpr) += 1;
	    ఆపు;
	  అప్రమేయ:
	    ఆపు;    
	}
      /* now count the size of what we just used.  */
      struct_స్థానభ్రంశం += p->కొలత;
    }
  /* Update the scores.  */
  *fprs = fpr_పునాది;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
స్థిర శూన్య
darwin64_pass_struct_by_విలువ (ffi_type *s, అక్షరం *src, గుర్తులేని కొలత,
			       గుర్తులేని *nfpr, ద్విగుణ **fprs, గుర్తులేని దీర్ఘ **arg)
{
  గుర్తులేని దీర్ఘ *తదుపరి_arg = *arg;
  అక్షరం *dest_cpy = (అక్షరం *)తదుపరి_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ఒకవేళ (!కొలత)
    తిరిగి;

  /* First... special cases.  */
  ఒకవేళ (కొలత < 3
      || (కొలత == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *తదుపరి_arg = 0UL; 
      dest_cpy += 8 - కొలత;
      జ్ఞాపకం_నకలు ((అక్షరం *) dest_cpy, src, కొలత);
      తదుపరి_arg++;
    }
  లేకపోతే ఒకవేళ (కొలత == 16)
    {
      జ్ఞాపకం_నకలు ((అక్షరం *) dest_cpy, src, కొలత);
      తదుపరి_arg += 2;
    }
  లేకపోతే
    {
      /* now the general case, we consider embedded floats.  */
      జ్ఞాపకం_నకలు ((అక్షరం *) dest_cpy, src, కొలత);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      తదుపరి_arg += (కొలత+7)/8;
    }
    
  *arg = తదుపరి_arg;
}

ద్విగుణ *
darwin64_struct_floats_గమ్యం_mem (ffi_type *s, అక్షరం *dest, ద్విగుణ *fprs, గుర్తులేని *nf)
{
  పూర్ణాంకం i;
  గుర్తులేని struct_స్థానభ్రంశం = 0;

  /* We don't assume anything about the alignment of the source.  */
  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      అక్షరం *అంశం_పునాది;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ఒకవేళ (i > 0)
        struct_స్థానభ్రంశం = FFI_ALIGN(struct_స్థానభ్రంశం, p->alignment);
      అంశం_పునాది = dest + struct_స్థానభ్రంశం;

      ఎంపిక (p->type)
	{
	  స్థితి FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_గమ్యం_mem (p, అంశం_పునాది, fprs, nf);
	    ఆపు;
	  స్థితి FFI_TYPE_LONGDOUBLE:
	    ఒకవేళ (*nf < అంకె_FPR_ARG_REGISTERS)
	      {
		*(ద్విగుణ *)అంశం_పునాది = *fprs++ ;
		(*nf) += 1;
	      }
	    అంశం_పునాది += 8;
	  /* FALL THROUGH */
	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (*nf < అంకె_FPR_ARG_REGISTERS)
	      {
		*(ద్విగుణ *)అంశం_పునాది = *fprs++ ;
		(*nf) += 1;
	      }
	    ఆపు;
	  స్థితి FFI_TYPE_FLOAT:
	    ఒకవేళ (*nf < అంకె_FPR_ARG_REGISTERS)
	      {
		*(దశాంశం *)అంశం_పునాది = (దశాంశం) *fprs++ ;
		(*nf) += 1;
	      }
	    ఆపు;
	  అప్రమేయ:
	    ఆపు;    
	}
      /* now count the size of what we just used.  */
      struct_స్థానభ్రంశం += p->కొలత;
    }
  తిరిగి fprs;
}

#అయితే_ముగింపు

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

స్థిర శూన్య
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  పూర్ణాంకం i;

  ఒకవేళ (s->type != FFI_TYPE_STRUCT)
    తిరిగి;

  s->కొలత = 0;
  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      ffi_type *p;
      పూర్ణాంకం align;
      
      p = s->elements[i];
      ఒకవేళ (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#ఒకవేళ defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#లేకపోతే
      /* Natural alignment for the first item... */
      ఒకవేళ (i == 0)
	align = p->alignment;
      లేకపోతే ఒకవేళ (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      లేకపోతే
	/* .. or align is 4.  */
	align = 4;
#అయితే_ముగింపు
      /* Pad, if necessary, before adding the current item.  */
      s->కొలత = FFI_ALIGN(s->కొలత, align) + p->కొలత;
    }
  
  s->కొలత = FFI_ALIGN(s->కొలత, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  ఒకవేళ (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

స్థిర పూర్ణాంకం
aix_adjust_aggregate_sizes (ffi_type *s, పూర్ణాంకం outer_most_type_or_మొదటి_member)
{
  పూర్ణాంకం i, nested_మొదటి_member=0, final_align, rc=0;

  ఒకవేళ (s->type != FFI_TYPE_STRUCT)
    తిరిగి 0;

  s->కొలత = 0;
  కొరకు (i = 0; s->elements[i] != ఖాళీ; i++)
    {
      ffi_type p;
      పూర్ణాంకం align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      ఒకవేళ (i == 0)
        nested_మొదటి_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_మొదటి_member);
      లేకపోతే
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      ఒకవేళ (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->కొలత = FFI_ALIGN(s->కొలత, align) + p.కొలత;
    }

  final_align=s->alignment;
  ఒకవేళ ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_మొదటి_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      ఒకవేళ (outer_most_type_or_మొదటి_member)
        s->alignment=final_align;
  }

  s->కొలత = FFI_ALIGN(s->కొలత, final_align);
  తిరిగి rc;
}

/* Perform machine dependent cif processing.  */
ffi_స్థితి
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  గుర్తులేని i;
  ffi_type **ptr;
  గుర్తులేని బైట్లు;
  గుర్తులేని fparg_లెక్క = 0, intarg_లెక్క = 0;
  గుర్తులేని flags = 0;
  గుర్తులేని కొలత_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  ఒకవేళ (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      కొరకు (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  ఒకవేళ (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      కొరకు (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  బైట్లు = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * పరిమాణం(గుర్తులేని దీర్ఘ);

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
  ఎంపిక (cif->rtype->type)
    {

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_బిట్లు;
      flags |= FLAG_RETURNS_FP;
      ఆపు;
#అయితే_ముగింపు

    స్థితి FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_బిట్లు;
      /* Fall through.  */
    స్థితి FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ఆపు;

    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
#నిర్వచితమైతే POWERPC64
    స్థితి FFI_TYPE_సూచిక:
#అయితే_ముగింపు
      flags |= FLAG_RETURNS_64_బిట్లు;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
#ఒకవేళ defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	ఒకవేళ (darwin64_struct_ret_by_విలువ_p (cif->rtype))
	  {
	    గుర్తులేని nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    ఒకవేళ (cif->rtype->కొలత != 16)
	      darwin64_పరిశీలన_struct_for_floats (cif->rtype, &nfpr) ;
	    లేకపోతే
	      flags |= FLAG_RETURNS_128_బిట్లు;
	    /* Will be 0 for 16byte struct.  */
	    ఒకవేళ (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	లేకపోతే /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_లెక్క++;
	  }
      }
#లేదావేళ defined(DARWIN_PPC)
      ఒకవేళ (cif->rtype->కొలత <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      లేకపోతే /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_లెక్క++;
	}
#లేకపోతే /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_లెక్క++;
#అయితే_ముగింపు
      ఆపు;
    స్థితి FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ఆపు;

    అప్రమేయ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ఆపు;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      గుర్తులేని align_పదాలు;
      ఎంపిక ((*ptr)->type)
	{
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	  fparg_లెక్క++;
#ఒకవేళ !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  ఒకవేళ (fparg_లెక్క > అంకె_FPR_ARG_REGISTERS
	      && (intarg_లెక్క & 0x01) != 0)
	    intarg_లెక్క++;
#అయితే_ముగింపు
	  ఆపు;

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
	  fparg_లెక్క += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  ఒకవేళ (fparg_లెక్క >= అంకె_FPR_ARG_REGISTERS)
#ఒకవేళ defined (POWERPC64)
	    intarg_లెక్క = FFI_ALIGN(intarg_లెక్క, 2);
#లేకపోతే
	    intarg_లెక్క = FFI_ALIGN(intarg_లెక్క, 4);
#అయితే_ముగింపు
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
#ఒకవేళ defined(POWERPC64)
	  intarg_లెక్క++;
#లేకపోతే
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  ఒకవేళ (intarg_లెక్క == అంకె_GPR_ARG_REGISTERS-1
	      || (intarg_లెక్క >= అంకె_GPR_ARG_REGISTERS 
	          && (intarg_లెక్క & 0x01) != 0))
	    intarg_లెక్క++;
	  intarg_లెక్క += 2;
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  కొలత_al = (*ptr)->కొలత;
#ఒకవేళ defined(POWERPC_DARWIN64)
	  align_పదాలు = (*ptr)->alignment >> 3;
	  ఒకవేళ (align_పదాలు)
	    intarg_లెక్క = FFI_ALIGN(intarg_లెక్క, align_పదాలు);
	  /* Base size of the struct.  */
	  intarg_లెక్క += (కొలత_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  ఒకవేళ (కొలత_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_పరిశీలన_struct_for_floats (*ptr, &fparg_లెక్క) ;
#లేకపోతే
	  align_పదాలు = (*ptr)->alignment >> 2;
	  ఒకవేళ (align_పదాలు)
	    intarg_లెక్క = FFI_ALIGN(intarg_లెక్క, align_పదాలు);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_లెక్క += (కొలత_al + 7) / 8;
#  లేకపోతే
	  intarg_లెక్క += (కొలత_al + 3) / 4;
#  endif
#అయితే_ముగింపు
	  ఆపు;

	అప్రమేయ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_లెక్క++;
	  ఆపు;
	}
    }

  ఒకవేళ (fparg_లెక్క != 0)
    flags |= FLAG_FP_ARGUMENTS;

#ఒకవేళ defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  ఒకవేళ (fparg_లెక్క != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    బైట్లు += అంకె_FPR_ARG_REGISTERS * పరిమాణం(ద్విగుణ);
#లేకపోతే
  /* Space for the FPR registers, if needed.  */
  ఒకవేళ (fparg_లెక్క != 0)
    బైట్లు += అంకె_FPR_ARG_REGISTERS * పరిమాణం(ద్విగుణ);
#అయితే_ముగింపు

  /* Stack space.  */
#నిర్వచితమైతే POWERPC64
  ఒకవేళ ((intarg_లెక్క + fparg_లెక్క) > అంకె_GPR_ARG_REGISTERS)
    బైట్లు += (intarg_లెక్క + fparg_లెక్క) * పరిమాణం(దీర్ఘ);
#లేకపోతే
  ఒకవేళ ((intarg_లెక్క + 2 * fparg_లెక్క) > అంకె_GPR_ARG_REGISTERS)
    బైట్లు += (intarg_లెక్క + 2 * fparg_లెక్క) * పరిమాణం(దీర్ఘ);
#అయితే_ముగింపు
  లేకపోతే
    బైట్లు += అంకె_GPR_ARG_REGISTERS * పరిమాణం(దీర్ఘ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  బైట్లు = FFI_ALIGN(బైట్లు, 16) ;

  cif->flags = flags;
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_AIX(extended_cif *, దీర్ఘ, గుర్తులేని, గుర్తులేని *,
			 శూన్య (*fn)(శూన్య), శూన్య (*fn2)(శూన్య));

#ఒకవేళ defined (FFI_GO_CLOSURES)
బాహ్య శూన్య ffi_call_go_AIX(extended_cif *, దీర్ఘ, గుర్తులేని, గుర్తులేని *,
			    శూన్య (*fn)(శూన్య), శూన్య (*fn2)(శూన్య), శూన్య *సంవృతం);
#అయితే_ముగింపు

బాహ్య శూన్య ffi_call_DARWIN(extended_cif *, దీర్ఘ, గుర్తులేని, గుర్తులేని *,
			    శూన్య (*fn)(శూన్య), శూన్య (*fn2)(శూన్య), ffi_type*);

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ఒకవేళ ((rvalue == ఖాళీ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_AIX:
      ffi_call_AIX(&ecif, -(దీర్ఘ)cif->బైట్లు, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_వాదనలు));
      ఆపు;
    స్థితి FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(దీర్ఘ)cif->బైట్లు, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_వాదనలు), cif->rtype);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}

#ఒకవేళ defined (FFI_GO_CLOSURES)
శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	     శూన్య *సంవృతం)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ఒకవేళ ((rvalue == ఖాళీ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_AIX:
      ffi_call_go_AIX(&ecif, -(దీర్ఘ)cif->బైట్లు, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_వాదనలు), సంవృతం);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}
#అయితే_ముగింపు

స్థిర శూన్య flush_icache(అక్షరం *);
స్థిర శూన్య flush_range(అక్షరం *, పూర్ణాంకం);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

రకనిర్వచనం నిర్మాణం aix_fd_struct {
  శూన్య *సంకేతం_సూచిక;
  శూన్య *toc;
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

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp;
  నిర్మాణం ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ఎంపిక (cif->abi)
    {
      స్థితి FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
#ఒకవేళ defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((గుర్తులేని దీర్ఘ *)&tramp[2]) = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_ASM; /* function  */
	*((గుర్తులేని దీర్ఘ *)&tramp[4]) = (గుర్తులేని దీర్ఘ) codeloc; /* context  */
#లేకపోతే
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_ASM; /* function  */
	tramp[3] = (గుర్తులేని దీర్ఘ) codeloc; /* context  */
#అయితే_ముగింపు
	సంవృతం->cif = cif;
	సంవృతం->fun = fun;
	సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_కొలత);

	ఆపు;

    స్థితి FFI_AIX:

      tramp_aix = (నిర్మాణం ffi_aix_trampoline_struct *) (సంవృతం->tramp);
      fd = (aix_fd *)(శూన్య *)ffi_సంవృతం_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->సంకేతం_సూచిక = fd->సంకేతం_సూచిక;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      సంవృతం->cif = cif;
      సంవృతం->fun = fun;
      సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
      ఆపు;

    అప్రమేయ:
      తిరిగి FFI_BAD_ABI;
      ఆపు;
    }
  తిరిగి FFI_OK;
}

#ఒకవేళ defined (FFI_GO_CLOSURES)
ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం,
		     ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
  ఎంపిక (cif->abi)
    {
      స్థితి FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        సంవృతం->tramp = (శూన్య *)ffi_go_సంవృతం_ASM;
        సంవృతం->cif = cif;
        సంవృతం->fun = fun;
        తిరిగి FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      అప్రమేయ:
        తిరిగి FFI_BAD_ABI;
        ఆపు;
    }
  తిరిగి FFI_OK;
}
#అయితే_ముగింపు

స్థిర శూన్య
flush_icache(అక్షరం *addr)
{
#నిర్వచితం_కాకపోతే _AIX
  __asm__ అస్థిర (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#అయితే_ముగింపు
}

స్థిర శూన్య
flush_range(అక్షరం * addr1, పూర్ణాంకం కొలత)
{
#నిర్వచించు MIN_పంక్తి_కొలత 32
  పూర్ణాంకం i;
  కొరకు (i = 0; i < కొలత; i += MIN_పంక్తి_కొలత)
    flush_icache(addr1+i);
  flush_icache(addr1+కొలత-1);
}

రకనిర్వచనం సంఘం
{
  దశాంశం f;
  ద్విగుణ d;
} ffi_dblfl;

పూర్ణాంకం
ffi_సంవృతం_helper_DARWIN (ffi_సంవృతం *, శూన్య *,
			   గుర్తులేని దీర్ఘ *, ffi_dblfl *);

#ఒకవేళ defined (FFI_GO_CLOSURES)
పూర్ణాంకం
ffi_go_సంవృతం_helper_DARWIN (ffi_go_సంవృతం*, శూన్య *,
			      గుర్తులేని దీర్ఘ *, ffi_dblfl *);
#అయితే_ముగింపు

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

స్థిర పూర్ణాంకం
ffi_సంవృతం_helper_common (ffi_cif* cif,
			   శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			   శూన్య *user_దత్తాంశం, శూన్య *rvalue,
			   గుర్తులేని దీర్ఘ *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  రకనిర్వచనం ద్విగుణ ldbits[2];

  సంఘం ldu
  {
    ldbits lb;
    దీర్ఘ ద్విగుణ ld;
  };

  శూన్య **          avalue;
  ffi_type **      arg_types;
  దీర్ఘ             i, avn;
  ffi_dblfl *      చివర_pfr = pfr + అంకె_FPR_ARG_REGISTERS;
  గుర్తులేని         కొలత_al;
  పూర్ణాంకం              struct_ret_by_విలువ = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  గుర్తులేని దీర్ఘ *  pgr0 = pgr;
#ఒకవేళ defined(POWERPC_DARWIN64)
  గుర్తులేని 	   fpsused = 0;
#అయితే_ముగింపు

  avalue = alloca (cif->nargs * పరిమాణం(శూన్య *));

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#ఒకవేళ defined(POWERPC_DARWIN64)
      ఒకవేళ (!darwin64_struct_ret_by_విలువ_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (శూన్య *) *pgr;
	  pgr++;
	}
      లేకపోతే
	struct_ret_by_విలువ = 1;
#లేదావేళ defined(DARWIN_PPC)
      ఒకవేళ (cif->rtype->కొలత > 4)
	{
	  rvalue = (శూన్య *) *pgr;
	  pgr++;
	}
      లేకపోతే
	struct_ret_by_విలువ = 1;
#లేకపోతే /* assume we return by ref.  */
      rvalue = (శూన్య *) *pgr;
      pgr++;
#అయితే_ముగింపు
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  అయితే (i < avn)
    {
      ఎంపిక (arg_types[i]->type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
#ఒకవేళ  defined(POWERPC64)
	  avalue[i] = (అక్షరం *) pgr + 7;
#లేకపోతే
	  avalue[i] = (అక్షరం *) pgr + 3;
#అయితే_ముగింపు
	  pgr++;
	  ఆపు;

	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
#ఒకవేళ  defined(POWERPC64)
	  avalue[i] = (అక్షరం *) pgr + 6;
#లేకపోతే
	  avalue[i] = (అక్షరం *) pgr + 2;
#అయితే_ముగింపు
	  pgr++;
	  ఆపు;

	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
#ఒకవేళ  defined(POWERPC64)
	  avalue[i] = (అక్షరం *) pgr + 4;
#లేకపోతే
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = pgr;
#అయితే_ముగింపు
	  pgr++;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  కొలత_al = arg_types[i]->కొలత;
#ఒకవేళ defined(POWERPC_DARWIN64)
	  pgr = (గుర్తులేని దీర్ఘ *)FFI_ALIGN((అక్షరం *)pgr, arg_types[i]->alignment);
	  ఒకవేళ (కొలత_al < 3 || కొలత_al == 4)
	    {
	      avalue[i] = ((అక్షరం *)pgr)+8-కొలత_al;
	      ఒకవేళ (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < అంకె_FPR_ARG_REGISTERS)
		{
		  *(దశాంశం *)pgr = (దశాంశం) *(ద్విగుణ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  లేకపోతే 
	    {
	      ఒకవేళ (కొలత_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_గమ్యం_mem (arg_types[i], (అక్షరం *)pgr,
						   (ద్విగుణ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (కొలత_al + 7) / 8;
#లేకపోతే
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  ఒకవేళ (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    కొలత_al = FFI_ALIGN(arg_types[i]->కొలత, 8);
#  ఒకవేళ defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (కొలత_al + 7) / 8;
#  లేకపోతే
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  ఒకవేళ (కొలత_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (అక్షరం*) pgr + 4 - కొలత_al;
	  లేకపోతే
	    avalue[i] = pgr;
	  pgr += (కొలత_al + 3) / 4;
#  endif
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
#ఒకవేళ  defined(POWERPC64)
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = pgr;
	  pgr++;
	  ఆపు;
#లేకపోతే
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  ఒకవేళ (pfr < చివర_pfr)
	    {
	      ద్విగుణ తాత్కాలిక = pfr->d;
	      pfr->f = (దశాంశం) తాత్కాలిక;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  లేకపోతే
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  ఒకవేళ (pfr < చివర_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  లేకపోతే
	    {
	      avalue[i] = pgr;
	    }
#నిర్వచితమైతే POWERPC64
	  pgr++;
#లేకపోతే
	  pgr += 2;
#అయితే_ముగింపు
	  ఆపు;

#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	స్థితి FFI_TYPE_LONGDOUBLE:
#నిర్వచితమైతే POWERPC64
	  ఒకవేళ (pfr + 1 < చివర_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  లేకపోతే
	    {
	      ఒకవేళ (pfr < చివర_pfr)
		{
		  *pgr = *(గుర్తులేని దీర్ఘ *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#లేకపోతే  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  ఒకవేళ (pfr + 1 < చివర_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  లేకపోతే ఒకవేళ (pfr + 1 == చివర_pfr)
	    {
	      సంఘం ldu తాత్కాలిక_ld;
	      జ్ఞాపకం_నకలు (&తాత్కాలిక_ld.lb[0], pfr, పరిమాణం(ldbits));
	      జ్ఞాపకం_నకలు (&తాత్కాలిక_ld.lb[1], pgr + 2, పరిమాణం(ldbits));
	      avalue[i] = &తాత్కాలిక_ld.ld;
	      pfr++;
	    }
	  లేకపోతే
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#అయితే_ముగింపు  /* POWERPC64 */
	  ఆపు;
#అయితే_ముగింపు
	అప్రమేయ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      తిరిగి PPC_LD_NONE;
    స్థితి FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      ఒకవేళ (struct_ret_by_విలువ)
	{
	  *pgr0 = (గుర్తులేని దీర్ఘ) cif->rtype;
	  తిరిగి PPC_LD_STRUCT;
	}
      తిరిగి PPC_LD_NONE;
    స్థితి FFI_TYPE_FLOAT:
      తిరిగి PPC_LD_F32;
    స్థితి FFI_TYPE_DOUBLE:
      తిరిగి PPC_LD_F64;
#ఒకవేళ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      తిరిగి PPC_LD_F128;
#అయితే_ముగింపు
    స్థితి FFI_TYPE_UINT8:
      తిరిగి PPC_LD_U8;
    స్థితి FFI_TYPE_SINT8:
      తిరిగి PPC_LD_S8;
    స్థితి FFI_TYPE_UINT16:
      తిరిగి PPC_LD_U16;
    స్థితి FFI_TYPE_SINT16:
      తిరిగి PPC_LD_S16;
    స్థితి FFI_TYPE_UINT32:
      తిరిగి PPC_LD_U32;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
      తిరిగి PPC_LD_S32;
    స్థితి FFI_TYPE_సూచిక:
      తిరిగి PPC_LD_PTR;
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      తిరిగి PPC_LD_I64;
    అప్రమేయ:
      abort();
    }
}

పూర్ణాంకం
ffi_సంవృతం_helper_DARWIN (ffi_సంవృతం *సంవృతం, శూన్య *rvalue,
			   గుర్తులేని దీర్ఘ *pgr, ffi_dblfl *pfr)
{
  తిరిగి ffi_సంవృతం_helper_common (సంవృతం->cif, సంవృతం->fun,
				    సంవృతం->user_దత్తాంశం, rvalue, pgr, pfr);
}

#ఒకవేళ defined (FFI_GO_CLOSURES)
పూర్ణాంకం
ffi_go_సంవృతం_helper_DARWIN (ffi_go_సంవృతం *సంవృతం, శూన్య *rvalue,
			      గుర్తులేని దీర్ఘ *pgr, ffi_dblfl *pfr)
{
  తిరిగి ffi_సంవృతం_helper_common (సంవృతం->cif, సంవృతం->fun,
				    సంవృతం, rvalue, pgr, pfr);
}
#అయితే_ముగింపు

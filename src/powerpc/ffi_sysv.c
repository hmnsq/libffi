/* -----------------------------------------------------------------------
   ffi_sysv.c - Copyright (C) 2013 IBM
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

#అన్తర్భావ "ffi.h"
#అన్తర్భావ <tramp.h>
#అన్తర్భావ <stdlib.h>

#నిర్వచితం_కాకపోతే POWERPC64
#అన్తర్భావ "ffi_common.h"
#అన్తర్భావ "ffi_powerpc.h"


/* About the SYSV ABI.  */
#నిర్వచించు ASM_NEEDS_REGISTERS 6
#నిర్వచించు అంకె_GPR_ARG_REGISTERS 8
#నిర్వచించు అంకె_FPR_ARG_REGISTERS 8


#ఒకవేళ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
శూన్య FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  ఒకవేళ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.కొలత = 8;
      ffi_type_longdouble.alignment = 8;
    }
  లేకపోతే
    {
      ffi_type_longdouble.కొలత = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#అయితే_ముగింపు

/* Transform long double, double and float to other types as per abi.  */
స్థిర పూర్ణాంకం
translate_float (పూర్ణాంకం abi, పూర్ణాంకం type)
{
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  ఒకవేళ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#అయితే_ముగింపు
  ఒకవేళ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      ఒకవేళ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      లేకపోతే ఒకవేళ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      లేకపోతే ఒకవేళ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  లేకపోతే ఒకవేళ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      ఒకవేళ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#అయితే_ముగింపు
    }
  తిరిగి type;
}

/* Perform machine dependent cif processing */
స్థిర ffi_స్థితి
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  గుర్తులేని బైట్లు;
  గుర్తులేని i, fpr_లెక్క = 0, gpr_లెక్క = 0, దొంతర_లెక్క = 0;
  గుర్తులేని flags = cif->flags;
  గుర్తులేని struct_copy_కొలత = 0;
  గుర్తులేని type = cif->rtype->type;
  గుర్తులేని కొలత = cif->rtype->కొలత;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  బైట్లు = (2 + ASM_NEEDS_REGISTERS) * పరిమాణం (పూర్ణాంకం);

  /* Space for the GPR registers.  */
  బైట్లు += అంకె_GPR_ARG_REGISTERS * పరిమాణం (పూర్ణాంకం);

  /* Return value handling.  The rules for SYSV are as follows:
     - 32-bit (or less) integer values are returned in gpr3;
     - Structures of size <= 4 bytes also returned in gpr3;
     - 64-bit integer values and structures between 5 and 8 bytes are returned
     in gpr3 and gpr4;
     - Larger structures are allocated space and a pointer is passed as
     the first argument.
     - Single/double FP values are returned in fpr1;
     - long doubles (if not equivalent to double) are returned in
     fpr1,fpr2 for Linux and as for large structs for SysV.  */

  type = translate_float (cif->abi, type);

  ఎంపిక (type)
    {
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    స్థితి FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_బిట్లు;
      /* Fall through.  */
#అయితే_ముగింపు
    స్థితి FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_బిట్లు;
      /* Fall through.  */
    స్థితి FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#నిర్వచితమైతే __NO_FPRS__
      తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
      ఆపు;

    స్థితి FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_బిట్లు;
      /* Fall through.  */
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_బిట్లు;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      ఒకవేళ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && కొలత <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  ఆపు;
	}
      gpr_లెక్క++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    స్థితి FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ఆపు;

    అప్రమేయ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ఆపు;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      గుర్తులేని పొట్టి typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      ఎంపిక (typenum)
	{
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
	  ఒకవేళ (fpr_లెక్క >= అంకె_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_లెక్క = అంకె_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      దొంతర_లెక్క += దొంతర_లెక్క & 1;
	      దొంతర_లెక్క += 4;
	    }
	  లేకపోతే
	    fpr_లెక్క += 2;
#నిర్వచితమైతే __NO_FPRS__
	  తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
	  ఆపు;
#అయితే_ముగింపు

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (fpr_లెక్క >= అంకె_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      దొంతర_లెక్క += దొంతర_లెక్క & 1;
	      దొంతర_లెక్క += 2;
	    }
	  లేకపోతే
	    fpr_లెక్క += 1;
#నిర్వచితమైతే __NO_FPRS__
	  తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  ఒకవేళ (fpr_లెక్క >= అంకె_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    దొంతర_లెక్క += 1;
	  లేకపోతే
	    fpr_లెక్క += 1;
#నిర్వచితమైతే __NO_FPRS__
	  తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
	  ఆపు;

	స్థితి FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  ఒకవేళ (gpr_లెక్క >= అంకె_GPR_ARG_REGISTERS - 3)
	    gpr_లెక్క = అంకె_GPR_ARG_REGISTERS;
	  ఒకవేళ (gpr_లెక్క >= అంకె_GPR_ARG_REGISTERS)
	    దొంతర_లెక్క += 4;
	  లేకపోతే
	    gpr_లెక్క += 4;
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_లెక్క += gpr_లెక్క & 1;
	  ఒకవేళ (gpr_లెక్క >= అంకె_GPR_ARG_REGISTERS)
	    {
	      దొంతర_లెక్క += దొంతర_లెక్క & 1;
	      దొంతర_లెక్క += 2;
	    }
	  లేకపోతే
	    gpr_లెక్క += 2;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_కొలత += ((*ptr)->కొలత + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	స్థితి FFI_TYPE_సూచిక:
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  ఒకవేళ (gpr_లెక్క >= అంకె_GPR_ARG_REGISTERS)
	    దొంతర_లెక్క += 1;
	  లేకపోతే
	    gpr_లెక్క += 1;
	  ఆపు;

	అప్రమేయ:
	  FFI_ASSERT (0);
	}
    }

  ఒకవేళ (fpr_లెక్క != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ఒకవేళ (gpr_లెక్క > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ఒకవేళ (struct_copy_కొలత != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  ఒకవేళ (fpr_లెక్క != 0)
    బైట్లు += అంకె_FPR_ARG_REGISTERS * పరిమాణం (ద్విగుణ);

  /* Stack space.  */
  బైట్లు += దొంతర_లెక్క * పరిమాణం (పూర్ణాంకం);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  బైట్లు = (బైట్లు + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  బైట్లు += struct_copy_కొలత;

  cif->flags = flags;
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  ఒకవేళ ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      ఎంపిక (cif->abi)
	{
	అప్రమేయ:
	  తిరిగి FFI_BAD_ABI;

	స్థితి FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  ఆపు;

	స్థితి FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  ఆపు;

	స్థితి FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ఆపు;

	స్థితి FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ఆపు;
	}
    }
  తిరిగి ffi_prep_cif_sysv_core (cif);
}

/* ffi_prep_args_SYSV is called by the assembly routine once stack space
   has been allocated for the function's arguments.

   The stack layout we want looks like this:

   |   Return address from ffi_call_SYSV 4bytes	|	higher addresses
   |--------------------------------------------|
   |   Previous backchain pointer	4	|       stack pointer here
   |--------------------------------------------|<+ <<<	on entry to
   |   Saved r28-r31			4*4	| |	ffi_call_SYSV
   |--------------------------------------------| |
   |   GPR registers r3-r10		8*4	| |	ffi_call_SYSV
   |--------------------------------------------| |
   |   FPR registers f1-f8 (optional)	8*8	| |
   |--------------------------------------------| |	stack	|
   |   Space for copied structures		| |	grows	|
   |--------------------------------------------| |	down    V
   |   Parameters that didn't fit in registers  | |
   |--------------------------------------------| |	lower addresses
   |   Space for callee's LR		4	| |
   |--------------------------------------------| |	stack pointer here
   |   Current backchain pointer	4	|-/	during
   |--------------------------------------------|   <<<	ffi_call_SYSV

*/

శూన్య FFI_HIDDEN
ffi_prep_వాదనలు_SYSV (extended_cif *ecif, గుర్తులేని *స్థిరరాశి దొంతర)
{
  స్థిరరాశి గుర్తులేని బైట్లు = ecif->cif->బైట్లు;
  స్థిరరాశి గుర్తులేని flags = ecif->cif->flags;

  రకనిర్వచనం సంఘం
  {
    అక్షరం *c;
    గుర్తులేని *u;
    దీర్ఘ దీర్ఘ *ll;
    దశాంశం *f;
    ద్విగుణ *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_పునాది;
  valp gpr_చివర;

#నిర్వచితం_కాకపోతే __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_పునాది;
  valp fpr_చివర;
#అయితే_ముగింపు

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp తదుపరి_arg;

  పూర్ణాంకం i;
  ffi_type **ptr;
#నిర్వచితం_కాకపోతే __NO_FPRS__
  ద్విగుణ double_tmp;
#అయితే_ముగింపు
  సంఘం
  {
    శూన్య **v;
    అక్షరం **c;
    గుర్తుగల అక్షరం **sc;
    గుర్తులేని అక్షరం **uc;
    గుర్తుగల పొట్టి **ss;
    గుర్తులేని పొట్టి **us;
    గుర్తులేని పూర్ణాంకం **ui;
    దీర్ఘ దీర్ఘ **ll;
    దశాంశం **f;
    ద్విగుణ **d;
  } p_argv;
  size_t struct_copy_కొలత;
  గుర్తులేని gprvalue;

  stacktop.c = (అక్షరం *) దొంతర + బైట్లు;
  gpr_చివర.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_పునాది.u = gpr_చివర.u - అంకె_GPR_ARG_REGISTERS;
#నిర్వచితం_కాకపోతే __NO_FPRS__
  fpr_చివర.d = gpr_పునాది.d;
  fpr_పునాది.d = fpr_చివర.d - అంకె_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_పునాది.c : gpr_పునాది.c);
#లేకపోతే
  copy_space.c = gpr_పునాది.c;
#అయితే_ముగింపు
  తదుపరి_arg.u = దొంతర + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) (అక్షరం *) దొంతర & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((గుర్తులేని దీర్ఘ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((బైట్లు & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= తదుపరి_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  ఒకవేళ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_పునాది.u++ = (గుర్తులేని) (అక్షరం *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  కొరకు (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      గుర్తులేని పూర్ణాంకం typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      ఎంపిక (typenum)
	{
#నిర్వచితం_కాకపోతే __NO_FPRS__
# ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	స్థితి FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  ఒకవేళ (fpr_పునాది.d >= fpr_చివర.d - 1)
	    {
	      fpr_పునాది.d = fpr_చివర.d;
	      ఒకవేళ (((తదుపరి_arg.u - దొంతర) & 1) != 0)
		తదుపరి_arg.u += 1;
	      *తదుపరి_arg.d = double_tmp;
	      తదుపరి_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *తదుపరి_arg.d = double_tmp;
	      తదుపరి_arg.u += 2;
	    }
	  లేకపోతే
	    {
	      *fpr_పునాది.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_పునాది.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ఆపు;
# endif
	స్థితి FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  ఒకవేళ (fpr_పునాది.d >= fpr_చివర.d)
	    {
	      ఒకవేళ (((తదుపరి_arg.u - దొంతర) & 1) != 0)
		తదుపరి_arg.u += 1;
	      *తదుపరి_arg.d = double_tmp;
	      తదుపరి_arg.u += 2;
	    }
	  లేకపోతే
	    *fpr_పునాది.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  ఒకవేళ (fpr_పునాది.d >= fpr_చివర.d)
	    {
	      *తదుపరి_arg.f = (దశాంశం) double_tmp;
	      తదుపరి_arg.u += 1;
	    }
	  లేకపోతే
	    *fpr_పునాది.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ఆపు;
#అయితే_ముగింపు /* have FPRs */

	స్థితి FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  ఒకవేళ (gpr_పునాది.u >= gpr_చివర.u - 3)
	    {
	      గుర్తులేని పూర్ణాంకం ii;
	      gpr_పునాది.u = gpr_చివర.u;
	      కొరకు (ii = 0; ii < 4; ii++)
		{
		  గుర్తులేని పూర్ణాంకం int_tmp = (*p_argv.ui)[ii];
		  *తదుపరి_arg.u++ = int_tmp;
		}
	    }
	  లేకపోతే
	    {
	      గుర్తులేని పూర్ణాంకం ii;
	      కొరకు (ii = 0; ii < 4; ii++)
		{
		  గుర్తులేని పూర్ణాంకం int_tmp = (*p_argv.ui)[ii];
		  *gpr_పునాది.u++ = int_tmp;
		}
	    }
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  ఒకవేళ (gpr_పునాది.u >= gpr_చివర.u - 1)
	    {
	      gpr_పునాది.u = gpr_చివర.u;
	      ఒకవేళ (((తదుపరి_arg.u - దొంతర) & 1) != 0)
		తదుపరి_arg.u++;
	      *తదుపరి_arg.ll = **p_argv.ll;
	      తదుపరి_arg.u += 2;
	    }
	  లేకపోతే
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      ఒకవేళ (((gpr_చివర.u - gpr_పునాది.u) & 1) != 0)
		gpr_పునాది.u++;
	      *gpr_పునాది.ll++ = **p_argv.ll;
	    }
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  struct_copy_కొలత = ((*ptr)->కొలత + 15) & ~0xF;
	  copy_space.c -= struct_copy_కొలత;
	  జ్ఞాపకం_నకలు (copy_space.c, *p_argv.c, (*ptr)->కొలత);

	  gprvalue = (గుర్తులేని దీర్ఘ) copy_space.c;

	  FFI_ASSERT (copy_space.c > తదుపరి_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  వెళ్ళు putgpr;

	స్థితి FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  వెళ్ళు putgpr;
	స్థితి FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  వెళ్ళు putgpr;

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_సూచిక:

	  gprvalue = **p_argv.ui;

	putgpr:
	  ఒకవేళ (gpr_పునాది.u >= gpr_చివర.u)
	    *తదుపరి_arg.u++ = gprvalue;
	  లేకపోతే
	    *gpr_పునాది.u++ = gprvalue;
	  ఆపు;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= తదుపరి_arg.c);
  FFI_ASSERT (gpr_పునాది.u <= gpr_చివర.u);
#నిర్వచితం_కాకపోతే __NO_FPRS__
  FFI_ASSERT (fpr_పునాది.u <= fpr_చివర.u);
#అయితే_ముగింపు
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_చివర.u - gpr_పునాది.u < 4));
}

#నిర్వచించు MIN_CACHE_పంక్తి_కొలత 8

స్థిర శూన్య
flush_icache (అక్షరం *wraddr, అక్షరం *xaddr, పూర్ణాంకం కొలత)
{
  పూర్ణాంకం i;
  కొరకు (i = 0; i < కొలత; i += MIN_CACHE_పంక్తి_కొలత)
    __asm__ అస్థిర ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ అస్థిర ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + కొలత - 1), "r"(wraddr + కొలత - 1)
		    : "memory");
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_సంవృతం_loc_sysv (ffi_సంవృతం *సంవృతం,
			   ffi_cif *cif,
			   శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
			   శూన్య *user_దత్తాంశం,
			   శూన్య *codeloc)
{
  ఒకవేళ (cif->abi < FFI_SYSV || cif->abi >= FFI_చివరి_ABI)
    తిరిగి FFI_BAD_ABI;

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
  ఒకవేళ (ffi_tramp_is_present(సంవృతం))
    {
      /* Initialize the static trampoline's parameters. */
      శూన్య (*dest)(శూన్య) = ffi_సంవృతం_SYSV;
      ffi_tramp_సమితి_parms (సంవృతం->ftramp, dest, సంవృతం);
    }
  లేకపోతే
#అయితే_ముగింపు
    {
      గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(శూన్య **) &tramp[8] = (శూన్య *) ffi_సంవృతం_SYSV; /* function */
      *(శూన్య **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((అక్షరం *)tramp, (అక్షరం *)codeloc, 8 * 4);
    }

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  తిరిగి FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

పూర్ణాంకం
ffi_సంవృతం_helper_SYSV (ffi_cif *cif,
			 శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
			 శూన్య *user_దత్తాంశం,
			 శూన్య *rvalue,
			 గుర్తులేని దీర్ఘ *pgr,
			 ffi_dblfl *pfr,
			 గుర్తులేని దీర్ఘ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  శూన్య **          avalue;
  ffi_type **      arg_types;
  దీర్ఘ             i, avn;
#నిర్వచితం_కాకపోతే __NO_FPRS__
  దీర్ఘ             nf = 0;   /* number of floating registers already used */
#అయితే_ముగింపు
  దీర్ఘ             ng = 0;   /* number of general registers already used */

  గుర్తులేని       కొలత     = cif->rtype->కొలత;
  గుర్తులేని పొట్టి rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * పరిమాణం (శూన్య *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  ఒకవేళ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && కొలత <= 8))
    {
      rvalue = (శూన్య *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  అయితే (i < avn) {
    గుర్తులేని పొట్టి typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    ఎంపిక (typenum)
      {
#నిర్వచితం_కాకపోతే __NO_FPRS__
      స్థితి FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	ఒకవేళ (nf < అంకె_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    ద్విగుణ తాత్కాలిక = pfr->d;
	    pfr->f = (దశాంశం) తాత్కాలిక;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	లేకపోతే
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	ఆపు;

      స్థితి FFI_TYPE_DOUBLE:
	ఒకవేళ (nf < అంకె_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	లేకపోతే
	  {
	    ఒకవేళ (((దీర్ఘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	ఆపు;

# ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      స్థితి FFI_TYPE_LONGDOUBLE:
	ఒకవేళ (nf < అంకె_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	లేకపోతే
	  {
	    ఒకవేళ (((దీర్ఘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	ఆపు;
# endif
#అయితే_ముగింపు

      స్థితి FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	లేకపోతే
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	ఆపు;

      స్థితి FFI_TYPE_SINT8:
      స్థితి FFI_TYPE_UINT8:
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (అక్షరం *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	లేకపోతే
	  {
	    avalue[i] = (అక్షరం *) pst + 3;
	    pst++;
	  }
	ఆపు;
#అయితే_ముగింపు

      స్థితి FFI_TYPE_SINT16:
      స్థితి FFI_TYPE_UINT16:
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (అక్షరం *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	లేకపోతే
	  {
	    avalue[i] = (అక్షరం *) pst + 2;
	    pst++;
	  }
	ఆపు;
#అయితే_ముగింపు

      స్థితి FFI_TYPE_SINT32:
      స్థితి FFI_TYPE_UINT32:
      స్థితి FFI_TYPE_సూచిక:
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	లేకపోతే
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	ఆపు;

      స్థితి FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (శూన్య *) *pgr;
	    ng++;
	    pgr++;
	  }
	లేకపోతే
	  {
	    avalue[i] = (శూన్య *) *pst;
	    pst++;
	  }
	ఆపు;

      స్థితి FFI_TYPE_SINT64:
      స్థితి FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	ఒకవేళ (ng < అంకె_GPR_ARG_REGISTERS - 1)
	  {
	    ఒకవేళ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	లేకపోతే
	  {
	    ఒకవేళ (((దీర్ఘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = అంకె_GPR_ARG_REGISTERS;
	  }
	ఆపు;

      అప్రమేయ:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  ఎంపిక (rtypenum)
    {
    స్థితి FFI_TYPE_VOID:
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
    స్థితి FFI_TYPE_UINT128:
      తిరిగి PPC32_LD_R3R6;
    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ (cif->abi & FFI_SYSV_STRUCT_RET)
	ఎంపిక (కొలత)
	  {
	  స్థితి 1:
	    తిరిగి PPC_LD_U8;
	  స్థితి 2:
	    తిరిగి PPC_LD_U16;
	  స్థితి 3:
	    తిరిగి PPC32_SYSV_LD_STRUCT_3;
	  స్థితి 4:
	    తిరిగి PPC_LD_U32;
	  స్థితి 5:
	    తిరిగి PPC32_SYSV_LD_STRUCT_5;
	  స్థితి 6:
	    తిరిగి PPC32_SYSV_LD_STRUCT_6;
	  స్థితి 7:
	    తిరిగి PPC32_SYSV_LD_STRUCT_7;
	  స్థితి 8:
	    తిరిగి PPC_LD_I64;
	  }
      తిరిగి PPC_LD_NONE;
    అప్రమేయ:
      abort();
    }
}
#అయితే_ముగింపు

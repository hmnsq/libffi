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

#شامل "ffi.h"
#شامل <tramp.h>
#شامل <stdlib.h>

#اگر_غیر_تعریف_شدہ POWERPC64
#شامل "ffi_common.h"
#شامل "ffi_powerpc.h"


/* About the SYSV ABI.  */
#تعریف_کرو ASM_NEEDS_REGISTERS 6
#تعریف_کرو عدد_GPR_ARG_REGISTERS 8
#تعریف_کرو عدد_FPR_ARG_REGISTERS 8


#اگر HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
خالی FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  اگر ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.سائز = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ورنہ
    {
      ffi_type_longdouble.سائز = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#اگر_ختم

/* Transform long double, double and float to other types as per abi.  */
ساکن صحیح
translate_float (صحیح abi, صحیح type)
{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  اگر (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#اگر_ختم
  اگر ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      اگر (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      ورنہ اگر (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ورنہ اگر (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  ورنہ اگر ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      اگر (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#اگر_ختم
    }
  واپس type;
}

/* Perform machine dependent cif processing */
ساکن ffi_حالت
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  غیرعلامتی بائٹیں;
  غیرعلامتی i, fpr_گنتی = 0, gpr_گنتی = 0, ڈھیر_گنتی = 0;
  غیرعلامتی flags = cif->flags;
  غیرعلامتی struct_copy_سائز = 0;
  غیرعلامتی type = cif->rtype->type;
  غیرعلامتی سائز = cif->rtype->سائز;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  بائٹیں = (2 + ASM_NEEDS_REGISTERS) * حجم (صحیح);

  /* Space for the GPR registers.  */
  بائٹیں += عدد_GPR_ARG_REGISTERS * حجم (صحیح);

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

  انتخاب (type)
    {
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_بٹ;
      /* Fall through.  */
#اگر_ختم
    حالت FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٹ;
      /* Fall through.  */
    حالت FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#اگر_تعریف_شدہ __NO_FPRS__
      واپس FFI_BAD_ABI;
#اگر_ختم
      توڑو;

    حالت FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_بٹ;
      /* Fall through.  */
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_بٹ;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      اگر ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && سائز <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  توڑو;
	}
      gpr_گنتی++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالت FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      توڑو;

    طےشدہ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      توڑو;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      غیرعلامتی مختصر typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      انتخاب (typenum)
	{
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
	  اگر (fpr_گنتی >= عدد_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_گنتی = عدد_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ڈھیر_گنتی += ڈھیر_گنتی & 1;
	      ڈھیر_گنتی += 4;
	    }
	  ورنہ
	    fpr_گنتی += 2;
#اگر_تعریف_شدہ __NO_FPRS__
	  واپس FFI_BAD_ABI;
#اگر_ختم
	  توڑو;
#اگر_ختم

	حالت FFI_TYPE_DOUBLE:
	  اگر (fpr_گنتی >= عدد_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ڈھیر_گنتی += ڈھیر_گنتی & 1;
	      ڈھیر_گنتی += 2;
	    }
	  ورنہ
	    fpr_گنتی += 1;
#اگر_تعریف_شدہ __NO_FPRS__
	  واپس FFI_BAD_ABI;
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  اگر (fpr_گنتی >= عدد_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ڈھیر_گنتی += 1;
	  ورنہ
	    fpr_گنتی += 1;
#اگر_تعریف_شدہ __NO_FPRS__
	  واپس FFI_BAD_ABI;
#اگر_ختم
	  توڑو;

	حالت FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  اگر (gpr_گنتی >= عدد_GPR_ARG_REGISTERS - 3)
	    gpr_گنتی = عدد_GPR_ARG_REGISTERS;
	  اگر (gpr_گنتی >= عدد_GPR_ARG_REGISTERS)
	    ڈھیر_گنتی += 4;
	  ورنہ
	    gpr_گنتی += 4;
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_گنتی += gpr_گنتی & 1;
	  اگر (gpr_گنتی >= عدد_GPR_ARG_REGISTERS)
	    {
	      ڈھیر_گنتی += ڈھیر_گنتی & 1;
	      ڈھیر_گنتی += 2;
	    }
	  ورنہ
	    gpr_گنتی += 2;
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_سائز += ((*ptr)->سائز + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	حالت FFI_TYPE_اشارہ_گر:
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  اگر (gpr_گنتی >= عدد_GPR_ARG_REGISTERS)
	    ڈھیر_گنتی += 1;
	  ورنہ
	    gpr_گنتی += 1;
	  توڑو;

	طےشدہ:
	  FFI_ASSERT (0);
	}
    }

  اگر (fpr_گنتی != 0)
    flags |= FLAG_FP_ARGUMENTS;
  اگر (gpr_گنتی > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  اگر (struct_copy_سائز != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  اگر (fpr_گنتی != 0)
    بائٹیں += عدد_FPR_ARG_REGISTERS * حجم (دوہرا);

  /* Stack space.  */
  بائٹیں += ڈھیر_گنتی * حجم (صحیح);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائٹیں = (بائٹیں + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  بائٹیں += struct_copy_سائز;

  cif->flags = flags;
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  اگر ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      انتخاب (cif->abi)
	{
	طےشدہ:
	  واپس FFI_BAD_ABI;

	حالت FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  توڑو;

	حالت FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  توڑو;

	حالت FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  توڑو;

	حالت FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  توڑو;
	}
    }
  واپس ffi_prep_cif_sysv_core (cif);
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

خالی FFI_HIDDEN
ffi_prep_دلائل_SYSV (extended_cif *ecif, غیرعلامتی *ثابت ڈھیر)
{
  ثابت غیرعلامتی بائٹیں = ecif->cif->بائٹیں;
  ثابت غیرعلامتی flags = ecif->cif->flags;

  قسمتعریف اتحاد
  {
    حرف *c;
    غیرعلامتی *u;
    طویل طویل *ll;
    اعشاری *f;
    دوہرا *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_بنیاد;
  valp gpr_انتہا;

#اگر_غیر_تعریف_شدہ __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_بنیاد;
  valp fpr_انتہا;
#اگر_ختم

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp اگلا_arg;

  صحیح i;
  ffi_type **ptr;
#اگر_غیر_تعریف_شدہ __NO_FPRS__
  دوہرا double_tmp;
#اگر_ختم
  اتحاد
  {
    خالی **v;
    حرف **c;
    علامتی حرف **sc;
    غیرعلامتی حرف **uc;
    علامتی مختصر **ss;
    غیرعلامتی مختصر **us;
    غیرعلامتی صحیح **ui;
    طویل طویل **ll;
    اعشاری **f;
    دوہرا **d;
  } p_argv;
  size_t struct_copy_سائز;
  غیرعلامتی gprvalue;

  stacktop.c = (حرف *) ڈھیر + بائٹیں;
  gpr_انتہا.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_بنیاد.u = gpr_انتہا.u - عدد_GPR_ARG_REGISTERS;
#اگر_غیر_تعریف_شدہ __NO_FPRS__
  fpr_انتہا.d = gpr_بنیاد.d;
  fpr_بنیاد.d = fpr_انتہا.d - عدد_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_بنیاد.c : gpr_بنیاد.c);
#ورنہ
  copy_space.c = gpr_بنیاد.c;
#اگر_ختم
  اگلا_arg.u = ڈھیر + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((غیرعلامتی طویل) (حرف *) ڈھیر & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((غیرعلامتی طویل) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((بائٹیں & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= اگلا_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  اگر (flags & FLAG_RETVAL_REFERENCE)
    *gpr_بنیاد.u++ = (غیرعلامتی) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  برائے (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      غیرعلامتی صحیح typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      انتخاب (typenum)
	{
#اگر_غیر_تعریف_شدہ __NO_FPRS__
# اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  اگر (fpr_بنیاد.d >= fpr_انتہا.d - 1)
	    {
	      fpr_بنیاد.d = fpr_انتہا.d;
	      اگر (((اگلا_arg.u - ڈھیر) & 1) != 0)
		اگلا_arg.u += 1;
	      *اگلا_arg.d = double_tmp;
	      اگلا_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *اگلا_arg.d = double_tmp;
	      اگلا_arg.u += 2;
	    }
	  ورنہ
	    {
	      *fpr_بنیاد.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_بنیاد.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  توڑو;
# endif
	حالت FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  اگر (fpr_بنیاد.d >= fpr_انتہا.d)
	    {
	      اگر (((اگلا_arg.u - ڈھیر) & 1) != 0)
		اگلا_arg.u += 1;
	      *اگلا_arg.d = double_tmp;
	      اگلا_arg.u += 2;
	    }
	  ورنہ
	    *fpr_بنیاد.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  اگر (fpr_بنیاد.d >= fpr_انتہا.d)
	    {
	      *اگلا_arg.f = (اعشاری) double_tmp;
	      اگلا_arg.u += 1;
	    }
	  ورنہ
	    *fpr_بنیاد.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  توڑو;
#اگر_ختم /* have FPRs */

	حالت FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  اگر (gpr_بنیاد.u >= gpr_انتہا.u - 3)
	    {
	      غیرعلامتی صحیح ii;
	      gpr_بنیاد.u = gpr_انتہا.u;
	      برائے (ii = 0; ii < 4; ii++)
		{
		  غیرعلامتی صحیح int_tmp = (*p_argv.ui)[ii];
		  *اگلا_arg.u++ = int_tmp;
		}
	    }
	  ورنہ
	    {
	      غیرعلامتی صحیح ii;
	      برائے (ii = 0; ii < 4; ii++)
		{
		  غیرعلامتی صحیح int_tmp = (*p_argv.ui)[ii];
		  *gpr_بنیاد.u++ = int_tmp;
		}
	    }
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  اگر (gpr_بنیاد.u >= gpr_انتہا.u - 1)
	    {
	      gpr_بنیاد.u = gpr_انتہا.u;
	      اگر (((اگلا_arg.u - ڈھیر) & 1) != 0)
		اگلا_arg.u++;
	      *اگلا_arg.ll = **p_argv.ll;
	      اگلا_arg.u += 2;
	    }
	  ورنہ
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      اگر (((gpr_انتہا.u - gpr_بنیاد.u) & 1) != 0)
		gpr_بنیاد.u++;
	      *gpr_بنیاد.ll++ = **p_argv.ll;
	    }
	  توڑو;

	حالت FFI_TYPE_STRUCT:
	  struct_copy_سائز = ((*ptr)->سائز + 15) & ~0xF;
	  copy_space.c -= struct_copy_سائز;
	  حافظہ_نقل (copy_space.c, *p_argv.c, (*ptr)->سائز);

	  gprvalue = (غیرعلامتی طویل) copy_space.c;

	  FFI_ASSERT (copy_space.c > اگلا_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  جاؤ putgpr;

	حالت FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  جاؤ putgpr;
	حالت FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  جاؤ putgpr;
	حالت FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  جاؤ putgpr;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_اشارہ_گر:

	  gprvalue = **p_argv.ui;

	putgpr:
	  اگر (gpr_بنیاد.u >= gpr_انتہا.u)
	    *اگلا_arg.u++ = gprvalue;
	  ورنہ
	    *gpr_بنیاد.u++ = gprvalue;
	  توڑو;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= اگلا_arg.c);
  FFI_ASSERT (gpr_بنیاد.u <= gpr_انتہا.u);
#اگر_غیر_تعریف_شدہ __NO_FPRS__
  FFI_ASSERT (fpr_بنیاد.u <= fpr_انتہا.u);
#اگر_ختم
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_انتہا.u - gpr_بنیاد.u < 4));
}

#تعریف_کرو MIN_CACHE_سطر_سائز 8

ساکن خالی
flush_icache (حرف *wraddr, حرف *xaddr, صحیح سائز)
{
  صحیح i;
  برائے (i = 0; i < سائز; i += MIN_CACHE_سطر_سائز)
    __asm__ متلون ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ متلون ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + سائز - 1), "r"(wraddr + سائز - 1)
		    : "memory");
}

ffi_حالت FFI_HIDDEN
ffi_prep_احاطہ_loc_sysv (ffi_احاطہ *احاطہ,
			   ffi_cif *cif,
			   خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
			   خالی *user_ڈیٹا,
			   خالی *codeloc)
{
  اگر (cif->abi < FFI_SYSV || cif->abi >= FFI_آخری_ABI)
    واپس FFI_BAD_ABI;

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
  اگر (ffi_tramp_is_present(احاطہ))
    {
      /* Initialize the static trampoline's parameters. */
      خالی (*dest)(خالی) = ffi_احاطہ_SYSV;
      ffi_tramp_سیٹ_parms (احاطہ->ftramp, dest, احاطہ);
    }
  ورنہ
#اگر_ختم
    {
      غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(خالی **) &tramp[8] = (خالی *) ffi_احاطہ_SYSV; /* function */
      *(خالی **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((حرف *)tramp, (حرف *)codeloc, 8 * 4);
    }

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

صحیح
ffi_احاطہ_helper_SYSV (ffi_cif *cif,
			 خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
			 خالی *user_ڈیٹا,
			 خالی *rvalue,
			 غیرعلامتی طویل *pgr,
			 ffi_dblfl *pfr,
			 غیرعلامتی طویل *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  خالی **          avalue;
  ffi_type **      arg_types;
  طویل             i, avn;
#اگر_غیر_تعریف_شدہ __NO_FPRS__
  طویل             nf = 0;   /* number of floating registers already used */
#اگر_ختم
  طویل             ng = 0;   /* number of general registers already used */

  غیرعلامتی       سائز     = cif->rtype->سائز;
  غیرعلامتی مختصر rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * حجم (خالی *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  اگر (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && سائز <= 8))
    {
      rvalue = (خالی *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  جب (i < avn) {
    غیرعلامتی مختصر typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    انتخاب (typenum)
      {
#اگر_غیر_تعریف_شدہ __NO_FPRS__
      حالت FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	اگر (nf < عدد_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    دوہرا عارضی = pfr->d;
	    pfr->f = (اعشاری) عارضی;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ورنہ
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	توڑو;

      حالت FFI_TYPE_DOUBLE:
	اگر (nf < عدد_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ورنہ
	  {
	    اگر (((طویل) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	توڑو;

# اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالت FFI_TYPE_LONGDOUBLE:
	اگر (nf < عدد_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	ورنہ
	  {
	    اگر (((طویل) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	توڑو;
# endif
#اگر_ختم

      حالت FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	اگر (ng < عدد_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	ورنہ
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	توڑو;

      حالت FFI_TYPE_SINT8:
      حالت FFI_TYPE_UINT8:
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	اگر (ng < عدد_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (حرف *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	ورنہ
	  {
	    avalue[i] = (حرف *) pst + 3;
	    pst++;
	  }
	توڑو;
#اگر_ختم

      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_UINT16:
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	اگر (ng < عدد_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (حرف *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	ورنہ
	  {
	    avalue[i] = (حرف *) pst + 2;
	    pst++;
	  }
	توڑو;
#اگر_ختم

      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_اشارہ_گر:
	اگر (ng < عدد_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	ورنہ
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	توڑو;

      حالت FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	اگر (ng < عدد_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (خالی *) *pgr;
	    ng++;
	    pgr++;
	  }
	ورنہ
	  {
	    avalue[i] = (خالی *) *pst;
	    pst++;
	  }
	توڑو;

      حالت FFI_TYPE_SINT64:
      حالت FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	اگر (ng < عدد_GPR_ARG_REGISTERS - 1)
	  {
	    اگر (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	ورنہ
	  {
	    اگر (((طویل) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = عدد_GPR_ARG_REGISTERS;
	  }
	توڑو;

      طےشدہ:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  انتخاب (rtypenum)
    {
    حالت FFI_TYPE_VOID:
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
    حالت FFI_TYPE_UINT128:
      واپس PPC32_LD_R3R6;
    حالت FFI_TYPE_STRUCT:
      اگر (cif->abi & FFI_SYSV_STRUCT_RET)
	انتخاب (سائز)
	  {
	  حالت 1:
	    واپس PPC_LD_U8;
	  حالت 2:
	    واپس PPC_LD_U16;
	  حالت 3:
	    واپس PPC32_SYSV_LD_STRUCT_3;
	  حالت 4:
	    واپس PPC_LD_U32;
	  حالت 5:
	    واپس PPC32_SYSV_LD_STRUCT_5;
	  حالت 6:
	    واپس PPC32_SYSV_LD_STRUCT_6;
	  حالت 7:
	    واپس PPC32_SYSV_LD_STRUCT_7;
	  حالت 8:
	    واپس PPC_LD_I64;
	  }
      واپس PPC_LD_NONE;
    طےشدہ:
      abort();
    }
}
#اگر_ختم

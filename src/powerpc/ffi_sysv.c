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

#ضمن "ffi.h"
#ضمن <tramp.h>
#ضمن <stdlib.h>

#جيڪڏهن_وصف_نه_ٿيل POWERPC64
#ضمن "ffi_common.h"
#ضمن "ffi_powerpc.h"


/* About the SYSV ABI.  */
#وصف_ڪريو ASM_NEEDS_REGISTERS 6
#وصف_ڪريو انگ_GPR_ARG_REGISTERS 8
#وصف_ڪريو انگ_FPR_ARG_REGISTERS 8


#إذا HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
فراغ FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  إذا ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.سائيز = 8;
      ffi_type_longdouble.alignment = 8;
    }
  وإلا
    {
      ffi_type_longdouble.سائيز = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#جيڪڏهن_پورو

/* Transform long double, double and float to other types as per abi.  */
ساكن صحيح
translate_float (صحيح abi, صحيح type)
{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  إذا (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#جيڪڏهن_پورو
  إذا ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      إذا (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      وإلا إذا (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      وإلا إذا (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  وإلا إذا ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      إذا (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#جيڪڏهن_پورو
    }
  أرجع type;
}

/* Perform machine dependent cif processing */
ساكن ffi_حالت
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  بنانشان بائيٽون;
  بنانشان i, fpr_ڳڻپ = 0, gpr_ڳڻپ = 0, ڍير_ڳڻپ = 0;
  بنانشان flags = cif->flags;
  بنانشان struct_copy_سائيز = 0;
  بنانشان type = cif->rtype->type;
  بنانشان سائيز = cif->rtype->سائيز;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  بائيٽون = (2 + ASM_NEEDS_REGISTERS) * حجم (صحيح);

  /* Space for the GPR registers.  */
  بائيٽون += انگ_GPR_ARG_REGISTERS * حجم (صحيح);

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

  بدل (type)
    {
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_بٽ;
      /* Fall through.  */
#جيڪڏهن_پورو
    حالة FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_بٽ;
      /* Fall through.  */
    حالة FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#جيڪڏهن_وصف_ٿيل __NO_FPRS__
      أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
      اكسر;

    حالة FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_بٽ;
      /* Fall through.  */
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_بٽ;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      إذا ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && سائيز <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  اكسر;
	}
      gpr_ڳڻپ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    حالة FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      اكسر;

    افتراضي:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      اكسر;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      بنانشان ننڍو typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      بدل (typenum)
	{
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
	  إذا (fpr_ڳڻپ >= انگ_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_ڳڻپ = انگ_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ڍير_ڳڻپ += ڍير_ڳڻپ & 1;
	      ڍير_ڳڻپ += 4;
	    }
	  وإلا
	    fpr_ڳڻپ += 2;
#جيڪڏهن_وصف_ٿيل __NO_FPRS__
	  أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
	  اكسر;
#جيڪڏهن_پورو

	حالة FFI_TYPE_DOUBLE:
	  إذا (fpr_ڳڻپ >= انگ_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ڍير_ڳڻپ += ڍير_ڳڻپ & 1;
	      ڍير_ڳڻپ += 2;
	    }
	  وإلا
	    fpr_ڳڻپ += 1;
#جيڪڏهن_وصف_ٿيل __NO_FPRS__
	  أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  إذا (fpr_ڳڻپ >= انگ_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ڍير_ڳڻپ += 1;
	  وإلا
	    fpr_ڳڻپ += 1;
#جيڪڏهن_وصف_ٿيل __NO_FPRS__
	  أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
	  اكسر;

	حالة FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  إذا (gpr_ڳڻپ >= انگ_GPR_ARG_REGISTERS - 3)
	    gpr_ڳڻپ = انگ_GPR_ARG_REGISTERS;
	  إذا (gpr_ڳڻپ >= انگ_GPR_ARG_REGISTERS)
	    ڍير_ڳڻپ += 4;
	  وإلا
	    gpr_ڳڻپ += 4;
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_ڳڻپ += gpr_ڳڻپ & 1;
	  إذا (gpr_ڳڻپ >= انگ_GPR_ARG_REGISTERS)
	    {
	      ڍير_ڳڻپ += ڍير_ڳڻپ & 1;
	      ڍير_ڳڻپ += 2;
	    }
	  وإلا
	    gpr_ڳڻپ += 2;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_سائيز += ((*ptr)->سائيز + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	حالة FFI_TYPE_اشارو:
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  إذا (gpr_ڳڻپ >= انگ_GPR_ARG_REGISTERS)
	    ڍير_ڳڻپ += 1;
	  وإلا
	    gpr_ڳڻپ += 1;
	  اكسر;

	افتراضي:
	  FFI_ASSERT (0);
	}
    }

  إذا (fpr_ڳڻپ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  إذا (gpr_ڳڻپ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  إذا (struct_copy_سائيز != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  إذا (fpr_ڳڻپ != 0)
    بائيٽون += انگ_FPR_ARG_REGISTERS * حجم (مزدوج);

  /* Stack space.  */
  بائيٽون += ڍير_ڳڻپ * حجم (صحيح);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  بائيٽون = (بائيٽون + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  بائيٽون += struct_copy_سائيز;

  cif->flags = flags;
  cif->بائيٽون = بائيٽون;

  أرجع FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  إذا ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      بدل (cif->abi)
	{
	افتراضي:
	  أرجع FFI_BAD_ABI;

	حالة FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  اكسر;

	حالة FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  اكسر;

	حالة FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  اكسر;

	حالة FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  اكسر;
	}
    }
  أرجع ffi_prep_cif_sysv_core (cif);
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

فراغ FFI_HIDDEN
ffi_prep_دليلون_SYSV (extended_cif *ecif, بنانشان *ثابت ڍير)
{
  ثابت بنانشان بائيٽون = ecif->cif->بائيٽون;
  ثابت بنانشان flags = ecif->cif->flags;

  تعريفنوع اتحاد
  {
    حرف *c;
    بنانشان *u;
    ڊگهو ڊگهو *ll;
    عائم *f;
    مزدوج *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_بنياد;
  valp gpr_پڇاڙي;

#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_بنياد;
  valp fpr_پڇاڙي;
#جيڪڏهن_پورو

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp ايندڙ_arg;

  صحيح i;
  ffi_type **ptr;
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
  مزدوج double_tmp;
#جيڪڏهن_پورو
  اتحاد
  {
    فراغ **v;
    حرف **c;
    نشانوارو حرف **sc;
    بنانشان حرف **uc;
    نشانوارو ننڍو **ss;
    بنانشان ننڍو **us;
    بنانشان صحيح **ui;
    ڊگهو ڊگهو **ll;
    عائم **f;
    مزدوج **d;
  } p_argv;
  size_t struct_copy_سائيز;
  بنانشان gprvalue;

  stacktop.c = (حرف *) ڍير + بائيٽون;
  gpr_پڇاڙي.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_بنياد.u = gpr_پڇاڙي.u - انگ_GPR_ARG_REGISTERS;
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
  fpr_پڇاڙي.d = gpr_بنياد.d;
  fpr_بنياد.d = fpr_پڇاڙي.d - انگ_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_بنياد.c : gpr_بنياد.c);
#وإلا
  copy_space.c = gpr_بنياد.c;
#جيڪڏهن_پورو
  ايندڙ_arg.u = ڍير + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((بنانشان ڊگهو) (حرف *) ڍير & 0xF) == 0);
  FFI_ASSERT (((بنانشان ڊگهو) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((بنانشان ڊگهو) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((بائيٽون & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= ايندڙ_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  إذا (flags & FLAG_RETVAL_REFERENCE)
    *gpr_بنياد.u++ = (بنانشان) (حرف *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  لكل (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      بنانشان صحيح typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      بدل (typenum)
	{
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
# إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  إذا (fpr_بنياد.d >= fpr_پڇاڙي.d - 1)
	    {
	      fpr_بنياد.d = fpr_پڇاڙي.d;
	      إذا (((ايندڙ_arg.u - ڍير) & 1) != 0)
		ايندڙ_arg.u += 1;
	      *ايندڙ_arg.d = double_tmp;
	      ايندڙ_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *ايندڙ_arg.d = double_tmp;
	      ايندڙ_arg.u += 2;
	    }
	  وإلا
	    {
	      *fpr_بنياد.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_بنياد.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;
# endif
	حالة FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  إذا (fpr_بنياد.d >= fpr_پڇاڙي.d)
	    {
	      إذا (((ايندڙ_arg.u - ڍير) & 1) != 0)
		ايندڙ_arg.u += 1;
	      *ايندڙ_arg.d = double_tmp;
	      ايندڙ_arg.u += 2;
	    }
	  وإلا
	    *fpr_بنياد.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  إذا (fpr_بنياد.d >= fpr_پڇاڙي.d)
	    {
	      *ايندڙ_arg.f = (عائم) double_tmp;
	      ايندڙ_arg.u += 1;
	    }
	  وإلا
	    *fpr_بنياد.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  اكسر;
#جيڪڏهن_پورو /* have FPRs */

	حالة FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  إذا (gpr_بنياد.u >= gpr_پڇاڙي.u - 3)
	    {
	      بنانشان صحيح ii;
	      gpr_بنياد.u = gpr_پڇاڙي.u;
	      لكل (ii = 0; ii < 4; ii++)
		{
		  بنانشان صحيح int_tmp = (*p_argv.ui)[ii];
		  *ايندڙ_arg.u++ = int_tmp;
		}
	    }
	  وإلا
	    {
	      بنانشان صحيح ii;
	      لكل (ii = 0; ii < 4; ii++)
		{
		  بنانشان صحيح int_tmp = (*p_argv.ui)[ii];
		  *gpr_بنياد.u++ = int_tmp;
		}
	    }
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	  إذا (gpr_بنياد.u >= gpr_پڇاڙي.u - 1)
	    {
	      gpr_بنياد.u = gpr_پڇاڙي.u;
	      إذا (((ايندڙ_arg.u - ڍير) & 1) != 0)
		ايندڙ_arg.u++;
	      *ايندڙ_arg.ll = **p_argv.ll;
	      ايندڙ_arg.u += 2;
	    }
	  وإلا
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      إذا (((gpr_پڇاڙي.u - gpr_بنياد.u) & 1) != 0)
		gpr_بنياد.u++;
	      *gpr_بنياد.ll++ = **p_argv.ll;
	    }
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  struct_copy_سائيز = ((*ptr)->سائيز + 15) & ~0xF;
	  copy_space.c -= struct_copy_سائيز;
	  يادگيري_نقل (copy_space.c, *p_argv.c, (*ptr)->سائيز);

	  gprvalue = (بنانشان ڊگهو) copy_space.c;

	  FFI_ASSERT (copy_space.c > ايندڙ_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  اذهب putgpr;

	حالة FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  اذهب putgpr;
	حالة FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  اذهب putgpr;
	حالة FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  اذهب putgpr;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_اشارو:

	  gprvalue = **p_argv.ui;

	putgpr:
	  إذا (gpr_بنياد.u >= gpr_پڇاڙي.u)
	    *ايندڙ_arg.u++ = gprvalue;
	  وإلا
	    *gpr_بنياد.u++ = gprvalue;
	  اكسر;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= ايندڙ_arg.c);
  FFI_ASSERT (gpr_بنياد.u <= gpr_پڇاڙي.u);
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
  FFI_ASSERT (fpr_بنياد.u <= fpr_پڇاڙي.u);
#جيڪڏهن_پورو
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_پڇاڙي.u - gpr_بنياد.u < 4));
}

#وصف_ڪريو MIN_CACHE_سٽ_سائيز 8

ساكن فراغ
flush_icache (حرف *wraddr, حرف *xaddr, صحيح سائيز)
{
  صحيح i;
  لكل (i = 0; i < سائيز; i += MIN_CACHE_سٽ_سائيز)
    __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ متطاير ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + سائيز - 1), "r"(wraddr + سائيز - 1)
		    : "memory");
}

ffi_حالت FFI_HIDDEN
ffi_prep_گهيرو_loc_sysv (ffi_گهيرو *گهيرو,
			   ffi_cif *cif,
			   فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			   فراغ *user_معلومات,
			   فراغ *codeloc)
{
  إذا (cif->abi < FFI_SYSV || cif->abi >= FFI_آخري_ABI)
    أرجع FFI_BAD_ABI;

#جيڪڏهن_وصف_ٿيل FFI_EXEC_STATIC_TRAMP
  إذا (ffi_tramp_is_present(گهيرو))
    {
      /* Initialize the static trampoline's parameters. */
      فراغ (*dest)(فراغ) = ffi_گهيرو_SYSV;
      ffi_tramp_سيٽ_parms (گهيرو->ftramp, dest, گهيرو);
    }
  وإلا
#جيڪڏهن_پورو
    {
      بنانشان صحيح *tramp = (بنانشان صحيح *) &گهيرو->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(فراغ **) &tramp[8] = (فراغ *) ffi_گهيرو_SYSV; /* function */
      *(فراغ **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((حرف *)tramp, (حرف *)codeloc, 8 * 4);
    }

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

صحيح
ffi_گهيرو_helper_SYSV (ffi_cif *cif,
			 فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
			 فراغ *user_معلومات,
			 فراغ *rvalue,
			 بنانشان ڊگهو *pgr,
			 ffi_dblfl *pfr,
			 بنانشان ڊگهو *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  فراغ **          avalue;
  ffi_type **      arg_types;
  ڊگهو             i, avn;
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
  ڊگهو             nf = 0;   /* number of floating registers already used */
#جيڪڏهن_پورو
  ڊگهو             ng = 0;   /* number of general registers already used */

  بنانشان       سائيز     = cif->rtype->سائيز;
  بنانشان ننڍو rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * حجم (فراغ *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  إذا (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && سائيز <= 8))
    {
      rvalue = (فراغ *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  طالما (i < avn) {
    بنانشان ننڍو typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    بدل (typenum)
      {
#جيڪڏهن_وصف_نه_ٿيل __NO_FPRS__
      حالة FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	إذا (nf < انگ_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    مزدوج عارضي = pfr->d;
	    pfr->f = (عائم) عارضي;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	وإلا
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	اكسر;

      حالة FFI_TYPE_DOUBLE:
	إذا (nf < انگ_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	وإلا
	  {
	    إذا (((ڊگهو) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	اكسر;

# إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالة FFI_TYPE_LONGDOUBLE:
	إذا (nf < انگ_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	وإلا
	  {
	    إذا (((ڊگهو) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	اكسر;
# endif
#جيڪڏهن_پورو

      حالة FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	إذا (ng < انگ_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	وإلا
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	اكسر;

      حالة FFI_TYPE_SINT8:
      حالة FFI_TYPE_UINT8:
#جيڪڏهن_وصف_نه_ٿيل __LITTLE_ENDIAN__
	إذا (ng < انگ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (حرف *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	وإلا
	  {
	    avalue[i] = (حرف *) pst + 3;
	    pst++;
	  }
	اكسر;
#جيڪڏهن_پورو

      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_UINT16:
#جيڪڏهن_وصف_نه_ٿيل __LITTLE_ENDIAN__
	إذا (ng < انگ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (حرف *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	وإلا
	  {
	    avalue[i] = (حرف *) pst + 2;
	    pst++;
	  }
	اكسر;
#جيڪڏهن_پورو

      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_اشارو:
	إذا (ng < انگ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	وإلا
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	اكسر;

      حالة FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	إذا (ng < انگ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (فراغ *) *pgr;
	    ng++;
	    pgr++;
	  }
	وإلا
	  {
	    avalue[i] = (فراغ *) *pst;
	    pst++;
	  }
	اكسر;

      حالة FFI_TYPE_SINT64:
      حالة FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	إذا (ng < انگ_GPR_ARG_REGISTERS - 1)
	  {
	    إذا (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	وإلا
	  {
	    إذا (((ڊگهو) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = انگ_GPR_ARG_REGISTERS;
	  }
	اكسر;

      افتراضي:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_معلومات);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  بدل (rtypenum)
    {
    حالة FFI_TYPE_VOID:
      أرجع PPC_LD_NONE;
    حالة FFI_TYPE_FLOAT:
      أرجع PPC_LD_F32;
    حالة FFI_TYPE_DOUBLE:
      أرجع PPC_LD_F64;
#إذا FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالة FFI_TYPE_LONGDOUBLE:
      أرجع PPC_LD_F128;
#جيڪڏهن_پورو
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
    حالة FFI_TYPE_اشارو:
      أرجع PPC_LD_PTR;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      أرجع PPC_LD_I64;
    حالة FFI_TYPE_UINT128:
      أرجع PPC32_LD_R3R6;
    حالة FFI_TYPE_STRUCT:
      إذا (cif->abi & FFI_SYSV_STRUCT_RET)
	بدل (سائيز)
	  {
	  حالة 1:
	    أرجع PPC_LD_U8;
	  حالة 2:
	    أرجع PPC_LD_U16;
	  حالة 3:
	    أرجع PPC32_SYSV_LD_STRUCT_3;
	  حالة 4:
	    أرجع PPC_LD_U32;
	  حالة 5:
	    أرجع PPC32_SYSV_LD_STRUCT_5;
	  حالة 6:
	    أرجع PPC32_SYSV_LD_STRUCT_6;
	  حالة 7:
	    أرجع PPC32_SYSV_LD_STRUCT_7;
	  حالة 8:
	    أرجع PPC_LD_I64;
	  }
      أرجع PPC_LD_NONE;
    افتراضي:
      abort();
    }
}
#جيڪڏهن_پورو

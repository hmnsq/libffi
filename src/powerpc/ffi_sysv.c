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

#अन्तर्भाव "ffi.h"
#अन्तर्भाव <tramp.h>
#अन्तर्भाव <stdlib.h>

#यदि_अपरिभाषितम् POWERPC64
#अन्तर्भाव "ffi_common.h"
#अन्तर्भाव "ffi_powerpc.h"


/* About the SYSV ABI.  */
#परिभाषय ASM_NEEDS_REGISTERS 6
#परिभाषय अङ्कः_GPR_ARG_REGISTERS 8
#परिभाषय अङ्कः_FPR_ARG_REGISTERS 8


#यदि HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  यदि ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.आकार = 8;
      ffi_type_longdouble.alignment = 8;
    }
  अन्यथा
    {
      ffi_type_longdouble.आकार = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#यदि_अन्तः

/* Transform long double, double and float to other types as per abi.  */
स्थिर पूर्णाङ्क
translate_float (पूर्णाङ्क abi, पूर्णाङ्क type)
{
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  यदि (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#यदि_अन्तः
  यदि ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      यदि (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      अन्यथा यदि (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      अन्यथा यदि (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  अन्यथा यदि ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      यदि (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#यदि_अन्तः
    }
  प्रत्यागम type;
}

/* Perform machine dependent cif processing */
स्थिर ffi_स्थितिः
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिह्नित अष्टकानि;
  अचिह्नित i, fpr_गणनम् = 0, gpr_गणनम् = 0, राशिः_गणनम् = 0;
  अचिह्नित flags = cif->flags;
  अचिह्नित struct_copy_आकार = 0;
  अचिह्नित type = cif->rtype->type;
  अचिह्नित आकार = cif->rtype->आकार;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  अष्टकानि = (2 + ASM_NEEDS_REGISTERS) * परिमाण (पूर्णाङ्क);

  /* Space for the GPR registers.  */
  अष्टकानि += अङ्कः_GPR_ARG_REGISTERS * परिमाण (पूर्णाङ्क);

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

  विकल्प (type)
    {
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_द्वयङ्काः;
      /* Fall through.  */
#यदि_अन्तः
    पक्ष FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_द्वयङ्काः;
      /* Fall through.  */
    पक्ष FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#यदि_परिभाषितम् __NO_FPRS__
      प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
      भङ्ग;

    पक्ष FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_द्वयङ्काः;
      /* Fall through.  */
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_द्वयङ्काः;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      यदि ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && आकार <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  भङ्ग;
	}
      gpr_गणनम्++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    पक्ष FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      भङ्ग;

    स्वभाव:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      भङ्ग;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिह्नित ह्रस्व typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      विकल्प (typenum)
	{
#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
	  यदि (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_गणनम् = अङ्कः_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 4;
	    }
	  अन्यथा
	    fpr_गणनम् += 2;
#यदि_परिभाषितम् __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
	  भङ्ग;
#यदि_अन्तः

	पक्ष FFI_TYPE_DOUBLE:
	  यदि (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 2;
	    }
	  अन्यथा
	    fpr_गणनम् += 1;
#यदि_परिभाषितम् __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  यदि (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    राशिः_गणनम् += 1;
	  अन्यथा
	    fpr_गणनम् += 1;
#यदि_परिभाषितम् __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
	  भङ्ग;

	पक्ष FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  यदि (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS - 3)
	    gpr_गणनम् = अङ्कः_GPR_ARG_REGISTERS;
	  यदि (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    राशिः_गणनम् += 4;
	  अन्यथा
	    gpr_गणनम् += 4;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_गणनम् += gpr_गणनम् & 1;
	  यदि (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    {
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 2;
	    }
	  अन्यथा
	    gpr_गणनम् += 2;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_आकार += ((*ptr)->आकार + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	पक्ष FFI_TYPE_POINTER:
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  यदि (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    राशिः_गणनम् += 1;
	  अन्यथा
	    gpr_गणनम् += 1;
	  भङ्ग;

	स्वभाव:
	  FFI_ASSERT (0);
	}
    }

  यदि (fpr_गणनम् != 0)
    flags |= FLAG_FP_ARGUMENTS;
  यदि (gpr_गणनम् > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  यदि (struct_copy_आकार != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  यदि (fpr_गणनम् != 0)
    अष्टकानि += अङ्कः_FPR_ARG_REGISTERS * परिमाण (द्विगुण);

  /* Stack space.  */
  अष्टकानि += राशिः_गणनम् * परिमाण (पूर्णाङ्क);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  अष्टकानि = (अष्टकानि + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  अष्टकानि += struct_copy_आकार;

  cif->flags = flags;
  cif->अष्टकानि = अष्टकानि;

  प्रत्यागम FFI_OK;
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  यदि ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      विकल्प (cif->abi)
	{
	स्वभाव:
	  प्रत्यागम FFI_BAD_ABI;

	पक्ष FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  भङ्ग;

	पक्ष FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  भङ्ग;

	पक्ष FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  भङ्ग;

	पक्ष FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  भङ्ग;
	}
    }
  प्रत्यागम ffi_prep_cif_sysv_core (cif);
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

शून्य FFI_HIDDEN
ffi_prep_तर्काः_SYSV (extended_cif *ecif, अचिह्नित *ध्रुव राशिः)
{
  ध्रुव अचिह्नित अष्टकानि = ecif->cif->अष्टकानि;
  ध्रुव अचिह्नित flags = ecif->cif->flags;

  प्रकारनिर्देश सङ्घ
  {
    वर्ण *c;
    अचिह्नित *u;
    दीर्घ दीर्घ *ll;
    भिन्न *f;
    द्विगुण *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_आधारः;
  valp gpr_अन्तः;

#यदि_अपरिभाषितम् __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_आधारः;
  valp fpr_अन्तः;
#यदि_अन्तः

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp अग्र_arg;

  पूर्णाङ्क i;
  ffi_type **ptr;
#यदि_अपरिभाषितम् __NO_FPRS__
  द्विगुण double_tmp;
#यदि_अन्तः
  सङ्घ
  {
    शून्य **v;
    वर्ण **c;
    चिह्नित वर्ण **sc;
    अचिह्नित वर्ण **uc;
    चिह्नित ह्रस्व **ss;
    अचिह्नित ह्रस्व **us;
    अचिह्नित पूर्णाङ्क **ui;
    दीर्घ दीर्घ **ll;
    भिन्न **f;
    द्विगुण **d;
  } p_argv;
  size_t struct_copy_आकार;
  अचिह्नित gprvalue;

  stacktop.c = (वर्ण *) राशिः + अष्टकानि;
  gpr_अन्तः.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_आधारः.u = gpr_अन्तः.u - अङ्कः_GPR_ARG_REGISTERS;
#यदि_अपरिभाषितम् __NO_FPRS__
  fpr_अन्तः.d = gpr_आधारः.d;
  fpr_आधारः.d = fpr_अन्तः.d - अङ्कः_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_आधारः.c : gpr_आधारः.c);
#अन्यथा
  copy_space.c = gpr_आधारः.c;
#यदि_अन्तः
  अग्र_arg.u = राशिः + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिह्नित दीर्घ) (वर्ण *) राशिः & 0xF) == 0);
  FFI_ASSERT (((अचिह्नित दीर्घ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((अचिह्नित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((अष्टकानि & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= अग्र_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  यदि (flags & FLAG_RETVAL_REFERENCE)
    *gpr_आधारः.u++ = (अचिह्नित) (वर्ण *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  प्रति (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      अचिह्नित पूर्णाङ्क typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      विकल्प (typenum)
	{
#यदि_अपरिभाषितम् __NO_FPRS__
# यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  यदि (fpr_आधारः.d >= fpr_अन्तः.d - 1)
	    {
	      fpr_आधारः.d = fpr_अन्तः.d;
	      यदि (((अग्र_arg.u - राशिः) & 1) != 0)
		अग्र_arg.u += 1;
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	    }
	  अन्यथा
	    {
	      *fpr_आधारः.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_आधारः.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;
# endif
	पक्ष FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  यदि (fpr_आधारः.d >= fpr_अन्तः.d)
	    {
	      यदि (((अग्र_arg.u - राशिः) & 1) != 0)
		अग्र_arg.u += 1;
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	    }
	  अन्यथा
	    *fpr_आधारः.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  यदि (fpr_आधारः.d >= fpr_अन्तः.d)
	    {
	      *अग्र_arg.f = (भिन्न) double_tmp;
	      अग्र_arg.u += 1;
	    }
	  अन्यथा
	    *fpr_आधारः.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;
#यदि_अन्तः /* have FPRs */

	पक्ष FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  यदि (gpr_आधारः.u >= gpr_अन्तः.u - 3)
	    {
	      अचिह्नित पूर्णाङ्क ii;
	      gpr_आधारः.u = gpr_अन्तः.u;
	      प्रति (ii = 0; ii < 4; ii++)
		{
		  अचिह्नित पूर्णाङ्क int_tmp = (*p_argv.ui)[ii];
		  *अग्र_arg.u++ = int_tmp;
		}
	    }
	  अन्यथा
	    {
	      अचिह्नित पूर्णाङ्क ii;
	      प्रति (ii = 0; ii < 4; ii++)
		{
		  अचिह्नित पूर्णाङ्क int_tmp = (*p_argv.ui)[ii];
		  *gpr_आधारः.u++ = int_tmp;
		}
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	  यदि (gpr_आधारः.u >= gpr_अन्तः.u - 1)
	    {
	      gpr_आधारः.u = gpr_अन्तः.u;
	      यदि (((अग्र_arg.u - राशिः) & 1) != 0)
		अग्र_arg.u++;
	      *अग्र_arg.ll = **p_argv.ll;
	      अग्र_arg.u += 2;
	    }
	  अन्यथा
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      यदि (((gpr_अन्तः.u - gpr_आधारः.u) & 1) != 0)
		gpr_आधारः.u++;
	      *gpr_आधारः.ll++ = **p_argv.ll;
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  struct_copy_आकार = ((*ptr)->आकार + 15) & ~0xF;
	  copy_space.c -= struct_copy_आकार;
	  स्मृतिं_प्रतिलिख (copy_space.c, *p_argv.c, (*ptr)->आकार);

	  gprvalue = (अचिह्नित दीर्घ) copy_space.c;

	  FFI_ASSERT (copy_space.c > अग्र_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  गच्छ putgpr;

	पक्ष FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  गच्छ putgpr;

	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  यदि (gpr_आधारः.u >= gpr_अन्तः.u)
	    *अग्र_arg.u++ = gprvalue;
	  अन्यथा
	    *gpr_आधारः.u++ = gprvalue;
	  भङ्ग;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= अग्र_arg.c);
  FFI_ASSERT (gpr_आधारः.u <= gpr_अन्तः.u);
#यदि_अपरिभाषितम् __NO_FPRS__
  FFI_ASSERT (fpr_आधारः.u <= fpr_अन्तः.u);
#यदि_अन्तः
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_अन्तः.u - gpr_आधारः.u < 4));
}

#परिभाषय MIN_CACHE_पङ्क्तिः_आकार 8

स्थिर शून्य
flush_icache (वर्ण *wraddr, वर्ण *xaddr, पूर्णाङ्क आकार)
{
  पूर्णाङ्क i;
  प्रति (i = 0; i < आकार; i += MIN_CACHE_पङ्क्तिः_आकार)
    __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + आकार - 1), "r"(wraddr + आकार - 1)
		    : "memory");
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_संवृतिः_loc_sysv (ffi_संवृतिः *संवृतिः,
			   ffi_cif *cif,
			   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			   शून्य *user_दत्त,
			   शून्य *codeloc)
{
  यदि (cif->abi < FFI_SYSV || cif->abi >= FFI_चरमः_ABI)
    प्रत्यागम FFI_BAD_ABI;

#यदि_परिभाषितम् FFI_EXEC_STATIC_TRAMP
  यदि (ffi_tramp_is_present(संवृतिः))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_संवृतिः_SYSV;
      ffi_tramp_समुच्चयः_parms (संवृतिः->ftramp, dest, संवृतिः);
    }
  अन्यथा
#यदि_अन्तः
    {
      अचिह्नित पूर्णाङ्क *tramp = (अचिह्नित पूर्णाङ्क *) &संवृतिः->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(शून्य **) &tramp[8] = (शून्य *) ffi_संवृतिः_SYSV; /* function */
      *(शून्य **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((वर्ण *)tramp, (वर्ण *)codeloc, 8 * 4);
    }

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

  प्रत्यागम FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

पूर्णाङ्क
ffi_संवृतिः_helper_SYSV (ffi_cif *cif,
			 शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			 शून्य *user_दत्त,
			 शून्य *rvalue,
			 अचिह्नित दीर्घ *pgr,
			 ffi_dblfl *pfr,
			 अचिह्नित दीर्घ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  शून्य **          avalue;
  ffi_type **      arg_types;
  दीर्घ             i, avn;
#यदि_अपरिभाषितम् __NO_FPRS__
  दीर्घ             nf = 0;   /* number of floating registers already used */
#यदि_अन्तः
  दीर्घ             ng = 0;   /* number of general registers already used */

  अचिह्नित       आकार     = cif->rtype->आकार;
  अचिह्नित ह्रस्व rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * परिमाण (शून्य *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  यदि (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && आकार <= 8))
    {
      rvalue = (शून्य *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  यावत् (i < avn) {
    अचिह्नित ह्रस्व typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    विकल्प (typenum)
      {
#यदि_अपरिभाषितम् __NO_FPRS__
      पक्ष FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	यदि (nf < अङ्कः_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    द्विगुण अस्थायी = pfr->d;
	    pfr->f = (भिन्न) अस्थायी;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_DOUBLE:
	यदि (nf < अङ्कः_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    यदि (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	भङ्ग;

# यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      पक्ष FFI_TYPE_LONGDOUBLE:
	यदि (nf < अङ्कः_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	अन्यथा
	  {
	    यदि (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	भङ्ग;
# endif
#यदि_अन्तः

      पक्ष FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	अन्यथा
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_SINT8:
      पक्ष FFI_TYPE_UINT8:
#यदि_अपरिभाषितम् __LITTLE_ENDIAN__
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (वर्ण *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (वर्ण *) pst + 3;
	    pst++;
	  }
	भङ्ग;
#यदि_अन्तः

      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_UINT16:
#यदि_अपरिभाषितम् __LITTLE_ENDIAN__
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (वर्ण *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (वर्ण *) pst + 2;
	    pst++;
	  }
	भङ्ग;
#यदि_अन्तः

      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_POINTER:
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (शून्य *) *pgr;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (शून्य *) *pst;
	    pst++;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_SINT64:
      पक्ष FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	यदि (ng < अङ्कः_GPR_ARG_REGISTERS - 1)
	  {
	    यदि (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	अन्यथा
	  {
	    यदि (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = अङ्कः_GPR_ARG_REGISTERS;
	  }
	भङ्ग;

      स्वभाव:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_दत्त);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  विकल्प (rtypenum)
    {
    पक्ष FFI_TYPE_VOID:
      प्रत्यागम PPC_LD_NONE;
    पक्ष FFI_TYPE_FLOAT:
      प्रत्यागम PPC_LD_F32;
    पक्ष FFI_TYPE_DOUBLE:
      प्रत्यागम PPC_LD_F64;
#यदि FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      प्रत्यागम PPC_LD_F128;
#यदि_अन्तः
    पक्ष FFI_TYPE_UINT8:
      प्रत्यागम PPC_LD_U8;
    पक्ष FFI_TYPE_SINT8:
      प्रत्यागम PPC_LD_S8;
    पक्ष FFI_TYPE_UINT16:
      प्रत्यागम PPC_LD_U16;
    पक्ष FFI_TYPE_SINT16:
      प्रत्यागम PPC_LD_S16;
    पक्ष FFI_TYPE_UINT32:
      प्रत्यागम PPC_LD_U32;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      प्रत्यागम PPC_LD_S32;
    पक्ष FFI_TYPE_POINTER:
      प्रत्यागम PPC_LD_PTR;
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      प्रत्यागम PPC_LD_I64;
    पक्ष FFI_TYPE_UINT128:
      प्रत्यागम PPC32_LD_R3R6;
    पक्ष FFI_TYPE_STRUCT:
      यदि (cif->abi & FFI_SYSV_STRUCT_RET)
	विकल्प (आकार)
	  {
	  पक्ष 1:
	    प्रत्यागम PPC_LD_U8;
	  पक्ष 2:
	    प्रत्यागम PPC_LD_U16;
	  पक्ष 3:
	    प्रत्यागम PPC32_SYSV_LD_STRUCT_3;
	  पक्ष 4:
	    प्रत्यागम PPC_LD_U32;
	  पक्ष 5:
	    प्रत्यागम PPC32_SYSV_LD_STRUCT_5;
	  पक्ष 6:
	    प्रत्यागम PPC32_SYSV_LD_STRUCT_6;
	  पक्ष 7:
	    प्रत्यागम PPC32_SYSV_LD_STRUCT_7;
	  पक्ष 8:
	    प्रत्यागम PPC_LD_I64;
	  }
      प्रत्यागम PPC_LD_NONE;
    स्वभाव:
      abort();
    }
}
#यदि_अन्तः

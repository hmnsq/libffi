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

#समावेश "ffi.h"
#समावेश <tramp.h>
#समावेश <stdlib.h>

#अगर_अघोषित POWERPC64
#समावेश "ffi_common.h"
#समावेश "ffi_powerpc.h"


/* About the SYSV ABI.  */
#घोषणा ASM_NEEDS_REGISTERS 6
#घोषणा अंक_GPR_ARG_REGISTERS 8
#घोषणा अंक_FPR_ARG_REGISTERS 8


#अगर HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
व्योम FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  अगर ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
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
#पूर्ण_अगर

/* Transform long double, double and float to other types as per abi.  */
अटल पूर्णांक
translate_float (पूर्णांक abi, पूर्णांक type)
{
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  अगर (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#पूर्ण_अगर
  अगर ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      अगर (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      अन्यथा अगर (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      अन्यथा अगर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  अन्यथा अगर ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      अगर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#पूर्ण_अगर
    }
  वापस type;
}

/* Perform machine dependent cif processing */
अटल ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित i, fpr_गिनती = 0, gpr_गिनती = 0, ढेर_गिनती = 0;
  अचिन्हित flags = cif->flags;
  अचिन्हित struct_copy_आकार = 0;
  अचिन्हित type = cif->rtype->type;
  अचिन्हित आकार = cif->rtype->आकार;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * माप (पूर्णांक);

  /* Space for the GPR registers.  */
  bytes += अंक_GPR_ARG_REGISTERS * माप (पूर्णांक);

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

  चयन (type)
    {
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#पूर्ण_अगर
    हाल FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    हाल FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#अगर_घोषित __NO_FPRS__
      वापस FFI_BAD_ABI;
#पूर्ण_अगर
      अवरोध;

    हाल FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      अगर ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && आकार <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  अवरोध;
	}
      gpr_गिनती++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    हाल FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      अवरोध;

    शेष:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      अवरोध;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित लघु typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      चयन (typenum)
	{
#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
	  अगर (fpr_गिनती >= अंक_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_गिनती = अंक_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ढेर_गिनती += ढेर_गिनती & 1;
	      ढेर_गिनती += 4;
	    }
	  अन्यथा
	    fpr_गिनती += 2;
#अगर_घोषित __NO_FPRS__
	  वापस FFI_BAD_ABI;
#पूर्ण_अगर
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_DOUBLE:
	  अगर (fpr_गिनती >= अंक_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ढेर_गिनती += ढेर_गिनती & 1;
	      ढेर_गिनती += 2;
	    }
	  अन्यथा
	    fpr_गिनती += 1;
#अगर_घोषित __NO_FPRS__
	  वापस FFI_BAD_ABI;
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  अगर (fpr_गिनती >= अंक_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ढेर_गिनती += 1;
	  अन्यथा
	    fpr_गिनती += 1;
#अगर_घोषित __NO_FPRS__
	  वापस FFI_BAD_ABI;
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  अगर (gpr_गिनती >= अंक_GPR_ARG_REGISTERS - 3)
	    gpr_गिनती = अंक_GPR_ARG_REGISTERS;
	  अगर (gpr_गिनती >= अंक_GPR_ARG_REGISTERS)
	    ढेर_गिनती += 4;
	  अन्यथा
	    gpr_गिनती += 4;
	  अवरोध;

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_गिनती += gpr_गिनती & 1;
	  अगर (gpr_गिनती >= अंक_GPR_ARG_REGISTERS)
	    {
	      ढेर_गिनती += ढेर_गिनती & 1;
	      ढेर_गिनती += 2;
	    }
	  अन्यथा
	    gpr_गिनती += 2;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_आकार += ((*ptr)->आकार + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	हाल FFI_TYPE_POINTER:
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  अगर (gpr_गिनती >= अंक_GPR_ARG_REGISTERS)
	    ढेर_गिनती += 1;
	  अन्यथा
	    gpr_गिनती += 1;
	  अवरोध;

	शेष:
	  FFI_ASSERT (0);
	}
    }

  अगर (fpr_गिनती != 0)
    flags |= FLAG_FP_ARGUMENTS;
  अगर (gpr_गिनती > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  अगर (struct_copy_आकार != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  अगर (fpr_गिनती != 0)
    bytes += अंक_FPR_ARG_REGISTERS * माप (द्विगुन);

  /* Stack space.  */
  bytes += ढेर_गिनती * माप (पूर्णांक);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_आकार;

  cif->flags = flags;
  cif->bytes = bytes;

  वापस FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  अगर ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      चयन (cif->abi)
	{
	शेष:
	  वापस FFI_BAD_ABI;

	हाल FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  अवरोध;

	हाल FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  अवरोध;

	हाल FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  अवरोध;

	हाल FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  अवरोध;
	}
    }
  वापस ffi_prep_cif_sysv_core (cif);
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

व्योम FFI_HIDDEN
ffi_prep_args_SYSV (extended_cif *ecif, अचिन्हित *स्थिर ढेर)
{
  स्थिर अचिन्हित bytes = ecif->cif->bytes;
  स्थिर अचिन्हित flags = ecif->cif->flags;

  प्रकार जोड़
  {
    अक्षर *c;
    अचिन्हित *u;
    दीर्घ दीर्घ *ll;
    भग्न *f;
    द्विगुन *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_base;
  valp gpr_end;

#अगर_अघोषित __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_base;
  valp fpr_end;
#पूर्ण_अगर

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp अग्र_arg;

  पूर्णांक i;
  ffi_type **ptr;
#अगर_अघोषित __NO_FPRS__
  द्विगुन double_tmp;
#पूर्ण_अगर
  जोड़
  {
    व्योम **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    अचिन्हित पूर्णांक **ui;
    दीर्घ दीर्घ **ll;
    भग्न **f;
    द्विगुन **d;
  } p_argv;
  माप_प्रकार struct_copy_आकार;
  अचिन्हित gprvalue;

  stacktop.c = (अक्षर *) ढेर + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - अंक_GPR_ARG_REGISTERS;
#अगर_अघोषित __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - अंक_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#अन्यथा
  copy_space.c = gpr_base.c;
#पूर्ण_अगर
  अग्र_arg.u = ढेर + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिन्हित दीर्घ) (अक्षर *) ढेर & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= अग्र_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  अगर (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (अचिन्हित) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  क्रम (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      अचिन्हित पूर्णांक typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      चयन (typenum)
	{
#अगर_अघोषित __NO_FPRS__
# अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  अगर (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      अगर (((अग्र_arg.u - ढेर) & 1) != 0)
		अग्र_arg.u += 1;
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	    }
	  अन्यथा
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  अवरोध;
# endif
	हाल FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  अगर (fpr_base.d >= fpr_end.d)
	    {
	      अगर (((अग्र_arg.u - ढेर) & 1) != 0)
		अग्र_arg.u += 1;
	      *अग्र_arg.d = double_tmp;
	      अग्र_arg.u += 2;
	    }
	  अन्यथा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  अगर (fpr_base.d >= fpr_end.d)
	    {
	      *अग्र_arg.f = (भग्न) double_tmp;
	      अग्र_arg.u += 1;
	    }
	  अन्यथा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  अवरोध;
#पूर्ण_अगर /* have FPRs */

	हाल FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  अगर (gpr_base.u >= gpr_end.u - 3)
	    {
	      अचिन्हित पूर्णांक ii;
	      gpr_base.u = gpr_end.u;
	      क्रम (ii = 0; ii < 4; ii++)
		{
		  अचिन्हित पूर्णांक int_tmp = (*p_argv.ui)[ii];
		  *अग्र_arg.u++ = int_tmp;
		}
	    }
	  अन्यथा
	    {
	      अचिन्हित पूर्णांक ii;
	      क्रम (ii = 0; ii < 4; ii++)
		{
		  अचिन्हित पूर्णांक int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  अवरोध;

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
	  अगर (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      अगर (((अग्र_arg.u - ढेर) & 1) != 0)
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
	      अगर (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  struct_copy_आकार = ((*ptr)->आकार + 15) & ~0xF;
	  copy_space.c -= struct_copy_आकार;
	  स_नकल (copy_space.c, *p_argv.c, (*ptr)->आकार);

	  gprvalue = (अचिन्हित दीर्घ) copy_space.c;

	  FFI_ASSERT (copy_space.c > अग्र_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  जाओ putgpr;

	हाल FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  जाओ putgpr;
	हाल FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  जाओ putgpr;

	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  अगर (gpr_base.u >= gpr_end.u)
	    *अग्र_arg.u++ = gprvalue;
	  अन्यथा
	    *gpr_base.u++ = gprvalue;
	  अवरोध;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= अग्र_arg.c);
  FFI_ASSERT (gpr_base.u <= gpr_end.u);
#अगर_अघोषित __NO_FPRS__
  FFI_ASSERT (fpr_base.u <= fpr_end.u);
#पूर्ण_अगर
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_end.u - gpr_base.u < 4));
}

#घोषणा MIN_CACHE_पंक्ति_आकार 8

अटल व्योम
flush_icache (अक्षर *wraddr, अक्षर *xaddr, पूर्णांक आकार)
{
  पूर्णांक i;
  क्रम (i = 0; i < आकार; i += MIN_CACHE_पंक्ति_आकार)
    __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + आकार - 1), "r"(wraddr + आकार - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
			   व्योम *user_तथ्य,
			   व्योम *codeloc)
{
  अगर (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    वापस FFI_BAD_ABI;

#अगर_घोषित FFI_EXEC_STATIC_TRAMP
  अगर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      व्योम (*dest)(व्योम) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#पूर्ण_अगर
    {
      अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(व्योम **) &tramp[8] = (व्योम *) ffi_closure_SYSV; /* function */
      *(व्योम **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((अक्षर *)tramp, (अक्षर *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

पूर्णांक
ffi_closure_helper_SYSV (ffi_cif *cif,
			 व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
			 व्योम *user_तथ्य,
			 व्योम *rvalue,
			 अचिन्हित दीर्घ *pgr,
			 ffi_dblfl *pfr,
			 अचिन्हित दीर्घ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  व्योम **          avalue;
  ffi_type **      arg_types;
  दीर्घ             i, avn;
#अगर_अघोषित __NO_FPRS__
  दीर्घ             nf = 0;   /* number of floating registers already used */
#पूर्ण_अगर
  दीर्घ             ng = 0;   /* number of general registers already used */

  अचिन्हित       आकार     = cif->rtype->आकार;
  अचिन्हित लघु rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * माप (व्योम *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  अगर (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && आकार <= 8))
    {
      rvalue = (व्योम *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  जबतक (i < avn) {
    अचिन्हित लघु typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    चयन (typenum)
      {
#अगर_अघोषित __NO_FPRS__
      हाल FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	अगर (nf < अंक_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    द्विगुन अस्थायी = pfr->d;
	    pfr->f = (भग्न) अस्थायी;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	अवरोध;

      हाल FFI_TYPE_DOUBLE:
	अगर (nf < अंक_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    अगर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	अवरोध;

# अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      हाल FFI_TYPE_LONGDOUBLE:
	अगर (nf < अंक_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	अन्यथा
	  {
	    अगर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	अवरोध;
# endif
#पूर्ण_अगर

      हाल FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	अगर (ng < अंक_GPR_ARG_REGISTERS - 3)
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
	अवरोध;

      हाल FFI_TYPE_SINT8:
      हाल FFI_TYPE_UINT8:
#अगर_अघोषित __LITTLE_ENDIAN__
	अगर (ng < अंक_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (अक्षर *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (अक्षर *) pst + 3;
	    pst++;
	  }
	अवरोध;
#पूर्ण_अगर

      हाल FFI_TYPE_SINT16:
      हाल FFI_TYPE_UINT16:
#अगर_अघोषित __LITTLE_ENDIAN__
	अगर (ng < अंक_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (अक्षर *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (अक्षर *) pst + 2;
	    pst++;
	  }
	अवरोध;
#पूर्ण_अगर

      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_POINTER:
	अगर (ng < अंक_GPR_ARG_REGISTERS)
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
	अवरोध;

      हाल FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	अगर (ng < अंक_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (व्योम *) *pgr;
	    ng++;
	    pgr++;
	  }
	अन्यथा
	  {
	    avalue[i] = (व्योम *) *pst;
	    pst++;
	  }
	अवरोध;

      हाल FFI_TYPE_SINT64:
      हाल FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	अगर (ng < अंक_GPR_ARG_REGISTERS - 1)
	  {
	    अगर (ng & 1)
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
	    अगर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = अंक_GPR_ARG_REGISTERS;
	  }
	अवरोध;

      शेष:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_तथ्य);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  चयन (rtypenum)
    {
    हाल FFI_TYPE_VOID:
      वापस PPC_LD_NONE;
    हाल FFI_TYPE_FLOAT:
      वापस PPC_LD_F32;
    हाल FFI_TYPE_DOUBLE:
      वापस PPC_LD_F64;
#अगर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      वापस PPC_LD_F128;
#पूर्ण_अगर
    हाल FFI_TYPE_UINT8:
      वापस PPC_LD_U8;
    हाल FFI_TYPE_SINT8:
      वापस PPC_LD_S8;
    हाल FFI_TYPE_UINT16:
      वापस PPC_LD_U16;
    हाल FFI_TYPE_SINT16:
      वापस PPC_LD_S16;
    हाल FFI_TYPE_UINT32:
      वापस PPC_LD_U32;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      वापस PPC_LD_S32;
    हाल FFI_TYPE_POINTER:
      वापस PPC_LD_PTR;
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      वापस PPC_LD_I64;
    हाल FFI_TYPE_UINT128:
      वापस PPC32_LD_R3R6;
    हाल FFI_TYPE_STRUCT:
      अगर (cif->abi & FFI_SYSV_STRUCT_RET)
	चयन (आकार)
	  {
	  हाल 1:
	    वापस PPC_LD_U8;
	  हाल 2:
	    वापस PPC_LD_U16;
	  हाल 3:
	    वापस PPC32_SYSV_LD_STRUCT_3;
	  हाल 4:
	    वापस PPC_LD_U32;
	  हाल 5:
	    वापस PPC32_SYSV_LD_STRUCT_5;
	  हाल 6:
	    वापस PPC32_SYSV_LD_STRUCT_6;
	  हाल 7:
	    वापस PPC32_SYSV_LD_STRUCT_7;
	  हाल 8:
	    वापस PPC_LD_I64;
	  }
      वापस PPC_LD_NONE;
    शेष:
      पात();
    }
}
#पूर्ण_अगर

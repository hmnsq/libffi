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

#ଅନ୍ତର୍ଭାଵ "ffi.h"
#ଅନ୍ତର୍ଭାଵ <tramp.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>

#ifndef POWERPC64
#ଅନ୍ତର୍ଭାଵ "ffi_common.h"
#ଅନ୍ତର୍ଭାଵ "ffi_powerpc.h"


/* About the SYSV ABI.  */
#define ASM_NEEDS_REGISTERS 6
#define ଅଙ୍କ_GPR_ARG_REGISTERS 8
#define ଅଙ୍କ_FPR_ARG_REGISTERS 8


#ଯଦି HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ଶୂନ୍ୟ FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  ଯଦି ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ଅନ୍ୟଥା
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
translate_float (ପୂର୍ଣ୍ଣସଂଖ୍ୟା abi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  ଯଦି (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  ଯଦି ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      ଯଦି (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      ଅନ୍ୟଥା ଯଦି (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ଅନ୍ୟଥା ଯଦି (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  ଅନ୍ୟଥା ଯଦି ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      ଯଦି (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  ଫେରନ୍ତୁ type;
}

/* Perform machine dependent cif processing */
ସ୍ଥିର ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fpr_ଗଣନା = 0, gpr_ଗଣନା = 0, ଗଦା_ଗଣନା = 0;
  unsigned flags = cif->flags;
  unsigned struct_copy_size = 0;
  unsigned type = cif->rtype->type;
  unsigned size = cif->rtype->size;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

  /* Space for the GPR registers.  */
  bytes += ଅଙ୍କ_GPR_ARG_REGISTERS * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

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

  ବାଛ (type)
    {
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      ଯଦି ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  ଭାଙ୍ଗ;
	}
      gpr_ଗଣନା++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ଭାଙ୍ଗ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned short typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      ବାଛ (typenum)
	{
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  ଯଦି (fpr_ଗଣନା >= ଅଙ୍କ_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_ଗଣନା = ଅଙ୍କ_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ଗଦା_ଗଣନା += ଗଦା_ଗଣନା & 1;
	      ଗଦା_ଗଣନା += 4;
	    }
	  ଅନ୍ୟଥା
	    fpr_ଗଣନା += 2;
#ifdef __NO_FPRS__
	  ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (fpr_ଗଣନା >= ଅଙ୍କ_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ଗଦା_ଗଣନା += ଗଦା_ଗଣନା & 1;
	      ଗଦା_ଗଣନା += 2;
	    }
	  ଅନ୍ୟଥା
	    fpr_ଗଣନା += 1;
#ifdef __NO_FPRS__
	  ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ଯଦି (fpr_ଗଣନା >= ଅଙ୍କ_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ଗଦା_ଗଣନା += 1;
	  ଅନ୍ୟଥା
	    fpr_ଗଣନା += 1;
#ifdef __NO_FPRS__
	  ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  ଯଦି (gpr_ଗଣନା >= ଅଙ୍କ_GPR_ARG_REGISTERS - 3)
	    gpr_ଗଣନା = ଅଙ୍କ_GPR_ARG_REGISTERS;
	  ଯଦି (gpr_ଗଣନା >= ଅଙ୍କ_GPR_ARG_REGISTERS)
	    ଗଦା_ଗଣନା += 4;
	  ଅନ୍ୟଥା
	    gpr_ଗଣନା += 4;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_ଗଣନା += gpr_ଗଣନା & 1;
	  ଯଦି (gpr_ଗଣନା >= ଅଙ୍କ_GPR_ARG_REGISTERS)
	    {
	      ଗଦା_ଗଣନା += ଗଦା_ଗଣନା & 1;
	      ଗଦା_ଗଣନା += 2;
	    }
	  ଅନ୍ୟଥା
	    gpr_ଗଣନା += 2;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_size += ((*ptr)->size + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	ସ୍ଥିତି FFI_TYPE_POINTER:
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  ଯଦି (gpr_ଗଣନା >= ଅଙ୍କ_GPR_ARG_REGISTERS)
	    ଗଦା_ଗଣନା += 1;
	  ଅନ୍ୟଥା
	    gpr_ଗଣନା += 1;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT (0);
	}
    }

  ଯଦି (fpr_ଗଣନା != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ଯଦି (gpr_ଗଣନା > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ଯଦି (struct_copy_size != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  ଯଦି (fpr_ଗଣନା != 0)
    bytes += ଅଙ୍କ_FPR_ARG_REGISTERS * ଆକାର (ଦ୍ୱିଗୁଣ);

  /* Stack space.  */
  bytes += ଗଦା_ଗଣନା * ଆକାର (ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_size;

  cif->flags = flags;
  cif->bytes = bytes;

  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  ଯଦି ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      ବାଛ (cif->abi)
	{
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଫେରନ୍ତୁ FFI_BAD_ABI;

	ସ୍ଥିତି FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ଭାଙ୍ଗ;
	}
    }
  ଫେରନ୍ତୁ ffi_prep_cif_sysv_core (cif);
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

ଶୂନ୍ୟ FFI_HIDDEN
ffi_prep_args_SYSV (extended_cif *ecif, unsigned *ଅଚଳ ଗଦା)
{
  ଅଚଳ unsigned bytes = ecif->cif->bytes;
  ଅଚଳ unsigned flags = ecif->cif->flags;

  ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ
  {
    ଅକ୍ଷର *c;
    unsigned *u;
    long long *ll;
    ଦଶମିକ *f;
    ଦ୍ୱିଗୁଣ *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_base;
  valp gpr_end;

#ifndef __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_base;
  valp fpr_end;
#endif

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp next_arg;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  ଦ୍ୱିଗୁଣ double_tmp;
#endif
  ସଂଘ
  {
    ଶୂନ୍ୟ **v;
    ଅକ୍ଷର **c;
    signed ଅକ୍ଷର **sc;
    unsigned ଅକ୍ଷର **uc;
    signed short **ss;
    unsigned short **us;
    unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା **ui;
    long long **ll;
    ଦଶମିକ **f;
    ଦ୍ୱିଗୁଣ **d;
  } p_argv;
  size_t struct_copy_size;
  unsigned gprvalue;

  stacktop.c = (ଅକ୍ଷର *) ଗଦା + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - ଅଙ୍କ_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - ଅଙ୍କ_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#ଅନ୍ୟଥା
  copy_space.c = gpr_base.c;
#endif
  next_arg.u = ଗଦା + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (ଅକ୍ଷର *) ଗଦା & 0xF) == 0);
  FFI_ASSERT (((unsigned long) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= next_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  ଯଦି (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (unsigned) (ଅକ୍ଷର *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  ପାଇଁ (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      ବାଛ (typenum)
	{
#ifndef __NO_FPRS__
# ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  ଯଦି (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      ଯଦି (((next_arg.u - ଗଦା) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  ଅନ୍ୟଥା
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;
# endif
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  ଯଦି (fpr_base.d >= fpr_end.d)
	    {
	      ଯଦି (((next_arg.u - ଗଦା) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  ଅନ୍ୟଥା
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  ଯଦି (fpr_base.d >= fpr_end.d)
	    {
	      *next_arg.f = (ଦଶମିକ) double_tmp;
	      next_arg.u += 1;
	    }
	  ଅନ୍ୟଥା
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;
#endif /* have FPRs */

	ସ୍ଥିତି FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  ଯଦି (gpr_base.u >= gpr_end.u - 3)
	    {
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
	      gpr_base.u = gpr_end.u;
	      ପାଇଁ (ii = 0; ii < 4; ii++)
		{
		  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା int_tmp = (*p_argv.ui)[ii];
		  *next_arg.u++ = int_tmp;
		}
	    }
	  ଅନ୍ୟଥା
	    {
	      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ii;
	      ପାଇଁ (ii = 0; ii < 4; ii++)
		{
		  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	  ଯଦି (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      ଯଦି (((next_arg.u - ଗଦା) & 1) != 0)
		next_arg.u++;
	      *next_arg.ll = **p_argv.ll;
	      next_arg.u += 2;
	    }
	  ଅନ୍ୟଥା
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      ଯଦି (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  struct_copy_size = ((*ptr)->size + 15) & ~0xF;
	  copy_space.c -= struct_copy_size;
	  memcpy (copy_space.c, *p_argv.c, (*ptr)->size);

	  gprvalue = (unsigned long) copy_space.c;

	  FFI_ASSERT (copy_space.c > next_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  ଯାଅ putgpr;

	ସ୍ଥିତି FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ଯାଅ putgpr;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  ଯଦି (gpr_base.u >= gpr_end.u)
	    *next_arg.u++ = gprvalue;
	  ଅନ୍ୟଥା
	    *gpr_base.u++ = gprvalue;
	  ଭାଙ୍ଗ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= next_arg.c);
  FFI_ASSERT (gpr_base.u <= gpr_end.u);
#ifndef __NO_FPRS__
  FFI_ASSERT (fpr_base.u <= fpr_end.u);
#endif
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_end.u - gpr_base.u < 4));
}

#define MIN_CACHE_ଧାଡ଼ି_SIZE 8

ସ୍ଥିର ଶୂନ୍ୟ
flush_icache (ଅକ୍ଷର *wraddr, ଅକ୍ଷର *xaddr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < size; i += MIN_CACHE_ଧାଡ଼ି_SIZE)
    __asm__ ଅସ୍ଥିର ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ଅସ୍ଥିର ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
			   ଶୂନ୍ୟ *user_data,
			   ଶୂନ୍ୟ *codeloc)
{
  ଯଦି (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ଯଦି (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ଶୂନ୍ୟ (*dest)(ଶୂନ୍ୟ) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ଅନ୍ୟଥା
#endif
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(ଶୂନ୍ୟ **) &tramp[8] = (ଶୂନ୍ୟ *) ffi_closure_SYSV; /* function */
      *(ଶୂନ୍ୟ **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((ଅକ୍ଷର *)tramp, (ଅକ୍ଷର *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ଫେରନ୍ତୁ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_SYSV (ffi_cif *cif,
			 ଶୂନ୍ୟ (*fun) (ffi_cif *, ଶୂନ୍ୟ *, ଶୂନ୍ୟ **, ଶୂନ୍ୟ *),
			 ଶୂନ୍ୟ *user_data,
			 ଶୂନ୍ୟ *rvalue,
			 unsigned long *pgr,
			 ffi_dblfl *pfr,
			 unsigned long *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  ଶୂନ୍ୟ **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
#ifndef __NO_FPRS__
  long             nf = 0;   /* number of floating registers already used */
#endif
  long             ng = 0;   /* number of general registers already used */

  unsigned       size     = cif->rtype->size;
  unsigned short rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  ଯଦି (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8))
    {
      rvalue = (ଶୂନ୍ୟ *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ଯେତେବେଳେ (i < avn) {
    unsigned short typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    ବାଛ (typenum)
      {
#ifndef __NO_FPRS__
      ସ୍ଥିତି FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	ଯଦି (nf < ଅଙ୍କ_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    ଦ୍ୱିଗୁଣ temp = pfr->d;
	    pfr->f = (ଦଶମିକ) temp;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ଯଦି (nf < ଅଙ୍କ_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ଅନ୍ୟଥା
	  {
	    ଯଦି (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	ଭାଙ୍ଗ;

# ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ଯଦି (nf < ଅଙ୍କ_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	ଅନ୍ୟଥା
	  {
	    ଯଦି (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	ଭାଙ୍ଗ;
# endif
#endif

      ସ୍ଥିତି FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_SINT8:
      ସ୍ଥିତି FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ଅକ୍ଷର *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = (ଅକ୍ଷର *) pst + 3;
	    pst++;
	  }
	ଭାଙ୍ଗ;
#endif

      ସ୍ଥିତି FFI_TYPE_SINT16:
      ସ୍ଥିତି FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ଅକ୍ଷର *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = (ଅକ୍ଷର *) pst + 2;
	    pst++;
	  }
	ଭାଙ୍ଗ;
#endif

      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_POINTER:
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ଶୂନ୍ୟ *) *pgr;
	    ng++;
	    pgr++;
	  }
	ଅନ୍ୟଥା
	  {
	    avalue[i] = (ଶୂନ୍ୟ *) *pst;
	    pst++;
	  }
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_SINT64:
      ସ୍ଥିତି FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	ଯଦି (ng < ଅଙ୍କ_GPR_ARG_REGISTERS - 1)
	  {
	    ଯଦି (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	ଅନ୍ୟଥା
	  {
	    ଯଦି (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = ଅଙ୍କ_GPR_ARG_REGISTERS;
	  }
	ଭାଙ୍ଗ;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  ବାଛ (rtypenum)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      ଫେରନ୍ତୁ PPC_LD_NONE;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      ଫେରନ୍ତୁ PPC_LD_F32;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      ଫେରନ୍ତୁ PPC_LD_F64;
#ଯଦି FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      ଫେରନ୍ତୁ PPC_LD_F128;
#endif
    ସ୍ଥିତି FFI_TYPE_UINT8:
      ଫେରନ୍ତୁ PPC_LD_U8;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      ଫେରନ୍ତୁ PPC_LD_S8;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      ଫେରନ୍ତୁ PPC_LD_U16;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      ଫେରନ୍ତୁ PPC_LD_S16;
    ସ୍ଥିତି FFI_TYPE_UINT32:
      ଫେରନ୍ତୁ PPC_LD_U32;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
      ଫେରନ୍ତୁ PPC_LD_S32;
    ସ୍ଥିତି FFI_TYPE_POINTER:
      ଫେରନ୍ତୁ PPC_LD_PTR;
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
      ଫେରନ୍ତୁ PPC_LD_I64;
    ସ୍ଥିତି FFI_TYPE_UINT128:
      ଫେରନ୍ତୁ PPC32_LD_R3R6;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->abi & FFI_SYSV_STRUCT_RET)
	ବାଛ (size)
	  {
	  ସ୍ଥିତି 1:
	    ଫେରନ୍ତୁ PPC_LD_U8;
	  ସ୍ଥିତି 2:
	    ଫେରନ୍ତୁ PPC_LD_U16;
	  ସ୍ଥିତି 3:
	    ଫେରନ୍ତୁ PPC32_SYSV_LD_STRUCT_3;
	  ସ୍ଥିତି 4:
	    ଫେରନ୍ତୁ PPC_LD_U32;
	  ସ୍ଥିତି 5:
	    ଫେରନ୍ତୁ PPC32_SYSV_LD_STRUCT_5;
	  ସ୍ଥିତି 6:
	    ଫେରନ୍ତୁ PPC32_SYSV_LD_STRUCT_6;
	  ସ୍ଥିତି 7:
	    ଫେରନ୍ତୁ PPC32_SYSV_LD_STRUCT_7;
	  ସ୍ଥିତି 8:
	    ଫେରନ୍ତୁ PPC_LD_I64;
	  }
      ଫେରନ୍ତୁ PPC_LD_NONE;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }
}
#endif

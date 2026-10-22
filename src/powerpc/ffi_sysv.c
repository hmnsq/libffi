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

#ਅਨ੍ਤਰ੍ਭਾਵ "ffi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ POWERPC64
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_common.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_powerpc.h"


/* About the SYSV ABI.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ASM_NEEDS_REGISTERS 6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_GPR_ARG_REGISTERS 8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅੰਕ_FPR_ARG_REGISTERS 8


#ਜੇ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ਖਾਲੀ FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  ਜੇ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.ਮਾਪ = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ਨਹੀਂਤਾਂ
    {
      ffi_type_longdouble.ਮਾਪ = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#ਜੇ_ਅੰਤ

/* Transform long double, double and float to other types as per abi.  */
ਸਥਿਰ ਪੂਰਨ
translate_float (ਪੂਰਨ abi, ਪੂਰਨ type)
{
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  ਜੇ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#ਜੇ_ਅੰਤ
  ਜੇ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      ਜੇ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      ਨਹੀਂਤਾਂ ਜੇ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ਨਹੀਂਤਾਂ ਜੇ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  ਨਹੀਂਤਾਂ ਜੇ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      ਜੇ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#ਜੇ_ਅੰਤ
    }
  ਵਾਪਸ type;
}

/* Perform machine dependent cif processing */
ਸਥਿਰ ffi_ਸਥਿਤੀ
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ i, fpr_ਗਿਣਤੀ = 0, gpr_ਗਿਣਤੀ = 0, ਢੇਰ_ਗਿਣਤੀ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = cif->flags;
  ਬਿਨਾਨਿਸ਼ਾਨੀ struct_copy_ਮਾਪ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ type = cif->rtype->type;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = cif->rtype->ਮਾਪ;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  ਬਾਈਟਾਂ = (2 + ASM_NEEDS_REGISTERS) * ਆਕਾਰ (ਪੂਰਨ);

  /* Space for the GPR registers.  */
  ਬਾਈਟਾਂ += ਅੰਕ_GPR_ARG_REGISTERS * ਆਕਾਰ (ਪੂਰਨ);

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

  ਚੋਣ (type)
    {
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_ਬਿੱਟ;
      /* Fall through.  */
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
      ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_ਬਿੱਟ;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      ਜੇ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && ਮਾਪ <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  ਰੋਕੋ;
	}
      gpr_ਗਿਣਤੀ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ਰੋਕੋ;

    ਮੂਲ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ਰੋਕੋ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      ਚੋਣ (typenum)
	{
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  ਜੇ (fpr_ਗਿਣਤੀ >= ਅੰਕ_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_ਗਿਣਤੀ = ਅੰਕ_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ਢੇਰ_ਗਿਣਤੀ += ਢੇਰ_ਗਿਣਤੀ & 1;
	      ਢੇਰ_ਗਿਣਤੀ += 4;
	    }
	  ਨਹੀਂਤਾਂ
	    fpr_ਗਿਣਤੀ += 2;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
	  ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (fpr_ਗਿਣਤੀ >= ਅੰਕ_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ਢੇਰ_ਗਿਣਤੀ += ਢੇਰ_ਗਿਣਤੀ & 1;
	      ਢੇਰ_ਗਿਣਤੀ += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    fpr_ਗਿਣਤੀ += 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
	  ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਜੇ (fpr_ਗਿਣਤੀ >= ਅੰਕ_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ਢੇਰ_ਗਿਣਤੀ += 1;
	  ਨਹੀਂਤਾਂ
	    fpr_ਗਿਣਤੀ += 1;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
	  ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  ਜੇ (gpr_ਗਿਣਤੀ >= ਅੰਕ_GPR_ARG_REGISTERS - 3)
	    gpr_ਗਿਣਤੀ = ਅੰਕ_GPR_ARG_REGISTERS;
	  ਜੇ (gpr_ਗਿਣਤੀ >= ਅੰਕ_GPR_ARG_REGISTERS)
	    ਢੇਰ_ਗਿਣਤੀ += 4;
	  ਨਹੀਂਤਾਂ
	    gpr_ਗਿਣਤੀ += 4;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_ਗਿਣਤੀ += gpr_ਗਿਣਤੀ & 1;
	  ਜੇ (gpr_ਗਿਣਤੀ >= ਅੰਕ_GPR_ARG_REGISTERS)
	    {
	      ਢੇਰ_ਗਿਣਤੀ += ਢੇਰ_ਗਿਣਤੀ & 1;
	      ਢੇਰ_ਗਿਣਤੀ += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    gpr_ਗਿਣਤੀ += 2;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_ਮਾਪ += ((*ptr)->ਮਾਪ + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  ਜੇ (gpr_ਗਿਣਤੀ >= ਅੰਕ_GPR_ARG_REGISTERS)
	    ਢੇਰ_ਗਿਣਤੀ += 1;
	  ਨਹੀਂਤਾਂ
	    gpr_ਗਿਣਤੀ += 1;
	  ਰੋਕੋ;

	ਮੂਲ:
	  FFI_ASSERT (0);
	}
    }

  ਜੇ (fpr_ਗਿਣਤੀ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ਜੇ (gpr_ਗਿਣਤੀ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ਜੇ (struct_copy_ਮਾਪ != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  ਜੇ (fpr_ਗਿਣਤੀ != 0)
    ਬਾਈਟਾਂ += ਅੰਕ_FPR_ARG_REGISTERS * ਆਕਾਰ (ਦੁੱਗਣਾ);

  /* Stack space.  */
  ਬਾਈਟਾਂ += ਢੇਰ_ਗਿਣਤੀ * ਆਕਾਰ (ਪੂਰਨ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ਬਾਈਟਾਂ = (ਬਾਈਟਾਂ + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  ਬਾਈਟਾਂ += struct_copy_ਮਾਪ;

  cif->flags = flags;
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  ਜੇ ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      ਚੋਣ (cif->abi)
	{
	ਮੂਲ:
	  ਵਾਪਸ FFI_BAD_ABI;

	ਹਾਲਤ FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ਰੋਕੋ;
	}
    }
  ਵਾਪਸ ffi_prep_cif_sysv_core (cif);
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

ਖਾਲੀ FFI_HIDDEN
ffi_prep_ਦਲੀਲਾਂ_SYSV (extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ *ਅਚਲ ਢੇਰ)
{
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ = ecif->cif->ਬਾਈਟਾਂ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ flags = ecif->cif->flags;

  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ
  {
    ਅੱਖਰ *c;
    ਬਿਨਾਨਿਸ਼ਾਨੀ *u;
    ਲੰਮਾ ਲੰਮਾ *ll;
    ਦਸ਼ਮਲਵ *f;
    ਦੁੱਗਣਾ *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_ਅਧਾਰ;
  valp gpr_ਸਿਰਾ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_ਅਧਾਰ;
  valp fpr_ਸਿਰਾ;
#ਜੇ_ਅੰਤ

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp ਅਗਲਾ_arg;

  ਪੂਰਨ i;
  ffi_type **ptr;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
  ਦੁੱਗਣਾ double_tmp;
#ਜੇ_ਅੰਤ
  ਸੰਘ
  {
    ਖਾਲੀ **v;
    ਅੱਖਰ **c;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ **sc;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ **uc;
    ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ **ss;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ **us;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ **ui;
    ਲੰਮਾ ਲੰਮਾ **ll;
    ਦਸ਼ਮਲਵ **f;
    ਦੁੱਗਣਾ **d;
  } p_argv;
  size_t struct_copy_ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ gprvalue;

  stacktop.c = (ਅੱਖਰ *) ਢੇਰ + ਬਾਈਟਾਂ;
  gpr_ਸਿਰਾ.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_ਅਧਾਰ.u = gpr_ਸਿਰਾ.u - ਅੰਕ_GPR_ARG_REGISTERS;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
  fpr_ਸਿਰਾ.d = gpr_ਅਧਾਰ.d;
  fpr_ਅਧਾਰ.d = fpr_ਸਿਰਾ.d - ਅੰਕ_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_ਅਧਾਰ.c : gpr_ਅਧਾਰ.c);
#ਨਹੀਂਤਾਂ
  copy_space.c = gpr_ਅਧਾਰ.c;
#ਜੇ_ਅੰਤ
  ਅਗਲਾ_arg.u = ਢੇਰ + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਅੱਖਰ *) ਢੇਰ & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((ਬਾਈਟਾਂ & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= ਅਗਲਾ_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  ਜੇ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_ਅਧਾਰ.u++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ) (ਅੱਖਰ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  ਲਈ (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      ਚੋਣ (typenum)
	{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
# ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  ਜੇ (fpr_ਅਧਾਰ.d >= fpr_ਸਿਰਾ.d - 1)
	    {
	      fpr_ਅਧਾਰ.d = fpr_ਸਿਰਾ.d;
	      ਜੇ (((ਅਗਲਾ_arg.u - ਢੇਰ) & 1) != 0)
		ਅਗਲਾ_arg.u += 1;
	      *ਅਗਲਾ_arg.d = double_tmp;
	      ਅਗਲਾ_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *ਅਗਲਾ_arg.d = double_tmp;
	      ਅਗਲਾ_arg.u += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      *fpr_ਅਧਾਰ.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_ਅਧਾਰ.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;
# endif
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  ਜੇ (fpr_ਅਧਾਰ.d >= fpr_ਸਿਰਾ.d)
	    {
	      ਜੇ (((ਅਗਲਾ_arg.u - ਢੇਰ) & 1) != 0)
		ਅਗਲਾ_arg.u += 1;
	      *ਅਗਲਾ_arg.d = double_tmp;
	      ਅਗਲਾ_arg.u += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    *fpr_ਅਧਾਰ.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  ਜੇ (fpr_ਅਧਾਰ.d >= fpr_ਸਿਰਾ.d)
	    {
	      *ਅਗਲਾ_arg.f = (ਦਸ਼ਮਲਵ) double_tmp;
	      ਅਗਲਾ_arg.u += 1;
	    }
	  ਨਹੀਂਤਾਂ
	    *fpr_ਅਧਾਰ.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* have FPRs */

	ਹਾਲਤ FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  ਜੇ (gpr_ਅਧਾਰ.u >= gpr_ਸਿਰਾ.u - 3)
	    {
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ii;
	      gpr_ਅਧਾਰ.u = gpr_ਸਿਰਾ.u;
	      ਲਈ (ii = 0; ii < 4; ii++)
		{
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ int_tmp = (*p_argv.ui)[ii];
		  *ਅਗਲਾ_arg.u++ = int_tmp;
		}
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ii;
	      ਲਈ (ii = 0; ii < 4; ii++)
		{
		  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ int_tmp = (*p_argv.ui)[ii];
		  *gpr_ਅਧਾਰ.u++ = int_tmp;
		}
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  ਜੇ (gpr_ਅਧਾਰ.u >= gpr_ਸਿਰਾ.u - 1)
	    {
	      gpr_ਅਧਾਰ.u = gpr_ਸਿਰਾ.u;
	      ਜੇ (((ਅਗਲਾ_arg.u - ਢੇਰ) & 1) != 0)
		ਅਗਲਾ_arg.u++;
	      *ਅਗਲਾ_arg.ll = **p_argv.ll;
	      ਅਗਲਾ_arg.u += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      ਜੇ (((gpr_ਸਿਰਾ.u - gpr_ਅਧਾਰ.u) & 1) != 0)
		gpr_ਅਧਾਰ.u++;
	      *gpr_ਅਧਾਰ.ll++ = **p_argv.ll;
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  struct_copy_ਮਾਪ = ((*ptr)->ਮਾਪ + 15) & ~0xF;
	  copy_space.c -= struct_copy_ਮਾਪ;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (copy_space.c, *p_argv.c, (*ptr)->ਮਾਪ);

	  gprvalue = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) copy_space.c;

	  FFI_ASSERT (copy_space.c > ਅਗਲਾ_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  ਜਾਓ putgpr;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ਜਾਓ putgpr;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:

	  gprvalue = **p_argv.ui;

	putgpr:
	  ਜੇ (gpr_ਅਧਾਰ.u >= gpr_ਸਿਰਾ.u)
	    *ਅਗਲਾ_arg.u++ = gprvalue;
	  ਨਹੀਂਤਾਂ
	    *gpr_ਅਧਾਰ.u++ = gprvalue;
	  ਰੋਕੋ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= ਅਗਲਾ_arg.c);
  FFI_ASSERT (gpr_ਅਧਾਰ.u <= gpr_ਸਿਰਾ.u);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
  FFI_ASSERT (fpr_ਅਧਾਰ.u <= fpr_ਸਿਰਾ.u);
#ਜੇ_ਅੰਤ
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_ਸਿਰਾ.u - gpr_ਅਧਾਰ.u < 4));
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_CACHE_ਸਤਰ_ਮਾਪ 8

ਸਥਿਰ ਖਾਲੀ
flush_icache (ਅੱਖਰ *wraddr, ਅੱਖਰ *xaddr, ਪੂਰਨ ਮਾਪ)
{
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < ਮਾਪ; i += MIN_CACHE_ਸਤਰ_ਮਾਪ)
    __asm__ ਅਸਥਿਰ ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ਅਸਥਿਰ ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + ਮਾਪ - 1), "r"(wraddr + ਮਾਪ - 1)
		    : "memory");
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_ਘੇਰਾ_loc_sysv (ffi_ਘੇਰਾ *ਘੇਰਾ,
			   ffi_cif *cif,
			   ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
			   ਖਾਲੀ *user_ਅੰਕੜੇ,
			   ਖਾਲੀ *codeloc)
{
  ਜੇ (cif->abi < FFI_SYSV || cif->abi >= FFI_ਆਖਰੀ_ABI)
    ਵਾਪਸ FFI_BAD_ABI;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      ਖਾਲੀ (*dest)(ਖਾਲੀ) = ffi_ਘੇਰਾ_SYSV;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
    }
  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(ਖਾਲੀ **) &tramp[8] = (ਖਾਲੀ *) ffi_ਘੇਰਾ_SYSV; /* function */
      *(ਖਾਲੀ **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((ਅੱਖਰ *)tramp, (ਅੱਖਰ *)codeloc, 8 * 4);
    }

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

ਪੂਰਨ
ffi_ਘੇਰਾ_helper_SYSV (ffi_cif *cif,
			 ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
			 ਖਾਲੀ *user_ਅੰਕੜੇ,
			 ਖਾਲੀ *rvalue,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pgr,
			 ffi_dblfl *pfr,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  ਖਾਲੀ **          avalue;
  ffi_type **      arg_types;
  ਲੰਮਾ             i, avn;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
  ਲੰਮਾ             nf = 0;   /* number of floating registers already used */
#ਜੇ_ਅੰਤ
  ਲੰਮਾ             ng = 0;   /* number of general registers already used */

  ਬਿਨਾਨਿਸ਼ਾਨੀ       ਮਾਪ     = cif->rtype->ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  ਜੇ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && ਮਾਪ <= 8))
    {
      rvalue = (ਖਾਲੀ *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਜਦੋਂ (i < avn) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    ਚੋਣ (typenum)
      {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __NO_FPRS__
      ਹਾਲਤ FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	ਜੇ (nf < ਅੰਕ_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    ਦੁੱਗਣਾ ਆਰਜ਼ੀ = pfr->d;
	    pfr->f = (ਦਸ਼ਮਲਵ) ਆਰਜ਼ੀ;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਜੇ (nf < ਅੰਕ_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    ਜੇ (((ਲੰਮਾ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	ਰੋਕੋ;

# ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਜੇ (nf < ਅੰਕ_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    ਜੇ (((ਲੰਮਾ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	ਰੋਕੋ;
# endif
#ਜੇ_ਅੰਤ

      ਹਾਲਤ FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_SINT8:
      ਹਾਲਤ FFI_TYPE_UINT8:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ਅੱਖਰ *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = (ਅੱਖਰ *) pst + 3;
	    pst++;
	  }
	ਰੋਕੋ;
#ਜੇ_ਅੰਤ

      ਹਾਲਤ FFI_TYPE_SINT16:
      ਹਾਲਤ FFI_TYPE_UINT16:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ਅੱਖਰ *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = (ਅੱਖਰ *) pst + 2;
	    pst++;
	  }
	ਰੋਕੋ;
#ਜੇ_ਅੰਤ

      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ਖਾਲੀ *) *pgr;
	    ng++;
	    pgr++;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    avalue[i] = (ਖਾਲੀ *) *pst;
	    pst++;
	  }
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_SINT64:
      ਹਾਲਤ FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	ਜੇ (ng < ਅੰਕ_GPR_ARG_REGISTERS - 1)
	  {
	    ਜੇ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	ਨਹੀਂਤਾਂ
	  {
	    ਜੇ (((ਲੰਮਾ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = ਅੰਕ_GPR_ARG_REGISTERS;
	  }
	ਰੋਕੋ;

      ਮੂਲ:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  ਚੋਣ (rtypenum)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      ਵਾਪਸ PPC_LD_NONE;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਵਾਪਸ PPC_LD_F32;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਵਾਪਸ PPC_LD_F64;
#ਜੇ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ਵਾਪਸ PPC_LD_F128;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਵਾਪਸ PPC_LD_U8;
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਵਾਪਸ PPC_LD_S8;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਵਾਪਸ PPC_LD_U16;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਵਾਪਸ PPC_LD_S16;
    ਹਾਲਤ FFI_TYPE_UINT32:
      ਵਾਪਸ PPC_LD_U32;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      ਵਾਪਸ PPC_LD_S32;
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਵਾਪਸ PPC_LD_PTR;
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      ਵਾਪਸ PPC_LD_I64;
    ਹਾਲਤ FFI_TYPE_UINT128:
      ਵਾਪਸ PPC32_LD_R3R6;
    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->abi & FFI_SYSV_STRUCT_RET)
	ਚੋਣ (ਮਾਪ)
	  {
	  ਹਾਲਤ 1:
	    ਵਾਪਸ PPC_LD_U8;
	  ਹਾਲਤ 2:
	    ਵਾਪਸ PPC_LD_U16;
	  ਹਾਲਤ 3:
	    ਵਾਪਸ PPC32_SYSV_LD_STRUCT_3;
	  ਹਾਲਤ 4:
	    ਵਾਪਸ PPC_LD_U32;
	  ਹਾਲਤ 5:
	    ਵਾਪਸ PPC32_SYSV_LD_STRUCT_5;
	  ਹਾਲਤ 6:
	    ਵਾਪਸ PPC32_SYSV_LD_STRUCT_6;
	  ਹਾਲਤ 7:
	    ਵਾਪਸ PPC32_SYSV_LD_STRUCT_7;
	  ਹਾਲਤ 8:
	    ਵਾਪਸ PPC_LD_I64;
	  }
      ਵਾਪਸ PPC_LD_NONE;
    ਮੂਲ:
      abort();
    }
}
#ਜੇ_ਅੰਤ

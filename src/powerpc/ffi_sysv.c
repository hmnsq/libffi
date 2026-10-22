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

#അന്തര്ഭാവ "ffi.h"
#അന്തര്ഭാവ <tramp.h>
#അന്തര്ഭാവ <stdlib.h>

#ifndef POWERPC64
#അന്തര്ഭാവ "ffi_common.h"
#അന്തര്ഭാവ "ffi_powerpc.h"


/* About the SYSV ABI.  */
#define ASM_NEEDS_REGISTERS 6
#define അക്കം_GPR_ARG_REGISTERS 8
#define അക്കം_FPR_ARG_REGISTERS 8


#എങ്കിൽ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ശൂന്യം FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  എങ്കിൽ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  അല്ലെങ്കിൽ
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
translate_float (പൂർണ്ണസംഖ്യ abi, പൂർണ്ണസംഖ്യ type)
{
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  എങ്കിൽ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  എങ്കിൽ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      എങ്കിൽ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      അല്ലെങ്കിൽ എങ്കിൽ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      അല്ലെങ്കിൽ എങ്കിൽ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  അല്ലെങ്കിൽ എങ്കിൽ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      എങ്കിൽ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  തിരികെ type;
}

/* Perform machine dependent cif processing */
സ്ഥിതം ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fpr_എണ്ണം = 0, gpr_എണ്ണം = 0, അട്ടി_എണ്ണം = 0;
  unsigned flags = cif->flags;
  unsigned struct_copy_size = 0;
  unsigned type = cif->rtype->type;
  unsigned size = cif->rtype->size;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * വലുപ്പം (പൂർണ്ണസംഖ്യ);

  /* Space for the GPR registers.  */
  bytes += അക്കം_GPR_ARG_REGISTERS * വലുപ്പം (പൂർണ്ണസംഖ്യ);

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

  തിരഞ്ഞെടുപ്പ് (type)
    {
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      തിരികെ FFI_BAD_ABI;
#endif
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      എങ്കിൽ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  നിർത്തുക;
	}
      gpr_എണ്ണം++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      നിർത്തുക;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned short typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      തിരഞ്ഞെടുപ്പ് (typenum)
	{
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  എങ്കിൽ (fpr_എണ്ണം >= അക്കം_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_എണ്ണം = അക്കം_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      അട്ടി_എണ്ണം += അട്ടി_എണ്ണം & 1;
	      അട്ടി_എണ്ണം += 4;
	    }
	  അല്ലെങ്കിൽ
	    fpr_എണ്ണം += 2;
#ifdef __NO_FPRS__
	  തിരികെ FFI_BAD_ABI;
#endif
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ (fpr_എണ്ണം >= അക്കം_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      അട്ടി_എണ്ണം += അട്ടി_എണ്ണം & 1;
	      അട്ടി_എണ്ണം += 2;
	    }
	  അല്ലെങ്കിൽ
	    fpr_എണ്ണം += 1;
#ifdef __NO_FPRS__
	  തിരികെ FFI_BAD_ABI;
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  എങ്കിൽ (fpr_എണ്ണം >= അക്കം_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    അട്ടി_എണ്ണം += 1;
	  അല്ലെങ്കിൽ
	    fpr_എണ്ണം += 1;
#ifdef __NO_FPRS__
	  തിരികെ FFI_BAD_ABI;
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  എങ്കിൽ (gpr_എണ്ണം >= അക്കം_GPR_ARG_REGISTERS - 3)
	    gpr_എണ്ണം = അക്കം_GPR_ARG_REGISTERS;
	  എങ്കിൽ (gpr_എണ്ണം >= അക്കം_GPR_ARG_REGISTERS)
	    അട്ടി_എണ്ണം += 4;
	  അല്ലെങ്കിൽ
	    gpr_എണ്ണം += 4;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_എണ്ണം += gpr_എണ്ണം & 1;
	  എങ്കിൽ (gpr_എണ്ണം >= അക്കം_GPR_ARG_REGISTERS)
	    {
	      അട്ടി_എണ്ണം += അട്ടി_എണ്ണം & 1;
	      അട്ടി_എണ്ണം += 2;
	    }
	  അല്ലെങ്കിൽ
	    gpr_എണ്ണം += 2;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_size += ((*ptr)->size + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	അവസ്ഥ FFI_TYPE_POINTER:
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  എങ്കിൽ (gpr_എണ്ണം >= അക്കം_GPR_ARG_REGISTERS)
	    അട്ടി_എണ്ണം += 1;
	  അല്ലെങ്കിൽ
	    gpr_എണ്ണം += 1;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT (0);
	}
    }

  എങ്കിൽ (fpr_എണ്ണം != 0)
    flags |= FLAG_FP_ARGUMENTS;
  എങ്കിൽ (gpr_എണ്ണം > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  എങ്കിൽ (struct_copy_size != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  എങ്കിൽ (fpr_എണ്ണം != 0)
    bytes += അക്കം_FPR_ARG_REGISTERS * വലുപ്പം (ഇരട്ട);

  /* Stack space.  */
  bytes += അട്ടി_എണ്ണം * വലുപ്പം (പൂർണ്ണസംഖ്യ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_size;

  cif->flags = flags;
  cif->bytes = bytes;

  തിരികെ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  എങ്കിൽ ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      തിരഞ്ഞെടുപ്പ് (cif->abi)
	{
	സ്ഥിരസ്ഥിതി:
	  തിരികെ FFI_BAD_ABI;

	അവസ്ഥ FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  നിർത്തുക;

	അവസ്ഥ FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  നിർത്തുക;

	അവസ്ഥ FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  നിർത്തുക;

	അവസ്ഥ FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  നിർത്തുക;
	}
    }
  തിരികെ ffi_prep_cif_sysv_core (cif);
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

ശൂന്യം FFI_HIDDEN
ffi_prep_args_SYSV (extended_cif *ecif, unsigned *സ്ഥിരം അട്ടി)
{
  സ്ഥിരം unsigned bytes = ecif->cif->bytes;
  സ്ഥിരം unsigned flags = ecif->cif->flags;

  തരനിർവചനം സംഘം
  {
    അക്ഷരം *c;
    unsigned *u;
    long long *ll;
    ദശാംശം *f;
    ഇരട്ട *d;
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

  പൂർണ്ണസംഖ്യ i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  ഇരട്ട double_tmp;
#endif
  സംഘം
  {
    ശൂന്യം **v;
    അക്ഷരം **c;
    signed അക്ഷരം **sc;
    unsigned അക്ഷരം **uc;
    signed short **ss;
    unsigned short **us;
    unsigned പൂർണ്ണസംഖ്യ **ui;
    long long **ll;
    ദശാംശം **f;
    ഇരട്ട **d;
  } p_argv;
  size_t struct_copy_size;
  unsigned gprvalue;

  stacktop.c = (അക്ഷരം *) അട്ടി + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - അക്കം_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - അക്കം_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#അല്ലെങ്കിൽ
  copy_space.c = gpr_base.c;
#endif
  next_arg.u = അട്ടി + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (അക്ഷരം *) അട്ടി & 0xF) == 0);
  FFI_ASSERT (((unsigned long) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= next_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  എങ്കിൽ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (unsigned) (അക്ഷരം *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  വേണ്ടി (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      unsigned പൂർണ്ണസംഖ്യ typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      തിരഞ്ഞെടുപ്പ് (typenum)
	{
#ifndef __NO_FPRS__
# എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  എങ്കിൽ (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      എങ്കിൽ (((next_arg.u - അട്ടി) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  അല്ലെങ്കിൽ
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;
# endif
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  എങ്കിൽ (fpr_base.d >= fpr_end.d)
	    {
	      എങ്കിൽ (((next_arg.u - അട്ടി) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  അല്ലെങ്കിൽ
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  എങ്കിൽ (fpr_base.d >= fpr_end.d)
	    {
	      *next_arg.f = (ദശാംശം) double_tmp;
	      next_arg.u += 1;
	    }
	  അല്ലെങ്കിൽ
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;
#endif /* have FPRs */

	അവസ്ഥ FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  എങ്കിൽ (gpr_base.u >= gpr_end.u - 3)
	    {
	      unsigned പൂർണ്ണസംഖ്യ ii;
	      gpr_base.u = gpr_end.u;
	      വേണ്ടി (ii = 0; ii < 4; ii++)
		{
		  unsigned പൂർണ്ണസംഖ്യ int_tmp = (*p_argv.ui)[ii];
		  *next_arg.u++ = int_tmp;
		}
	    }
	  അല്ലെങ്കിൽ
	    {
	      unsigned പൂർണ്ണസംഖ്യ ii;
	      വേണ്ടി (ii = 0; ii < 4; ii++)
		{
		  unsigned പൂർണ്ണസംഖ്യ int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	  എങ്കിൽ (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      എങ്കിൽ (((next_arg.u - അട്ടി) & 1) != 0)
		next_arg.u++;
	      *next_arg.ll = **p_argv.ll;
	      next_arg.u += 2;
	    }
	  അല്ലെങ്കിൽ
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      എങ്കിൽ (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  struct_copy_size = ((*ptr)->size + 15) & ~0xF;
	  copy_space.c -= struct_copy_size;
	  memcpy (copy_space.c, *p_argv.c, (*ptr)->size);

	  gprvalue = (unsigned long) copy_space.c;

	  FFI_ASSERT (copy_space.c > next_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  പോകുക putgpr;

	അവസ്ഥ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  പോകുക putgpr;

	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  എങ്കിൽ (gpr_base.u >= gpr_end.u)
	    *next_arg.u++ = gprvalue;
	  അല്ലെങ്കിൽ
	    *gpr_base.u++ = gprvalue;
	  നിർത്തുക;
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

#define MIN_CACHE_വരി_SIZE 8

സ്ഥിതം ശൂന്യം
flush_icache (അക്ഷരം *wraddr, അക്ഷരം *xaddr, പൂർണ്ണസംഖ്യ size)
{
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < size; i += MIN_CACHE_വരി_SIZE)
    __asm__ അസ്ഥിര ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ അസ്ഥിര ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
			   ശൂന്യം *user_data,
			   ശൂന്യം *codeloc)
{
  എങ്കിൽ (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    തിരികെ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  എങ്കിൽ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      ശൂന്യം (*dest)(ശൂന്യം) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  അല്ലെങ്കിൽ
#endif
    {
      unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(ശൂന്യം **) &tramp[8] = (ശൂന്യം *) ffi_closure_SYSV; /* function */
      *(ശൂന്യം **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((അക്ഷരം *)tramp, (അക്ഷരം *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  തിരികെ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

പൂർണ്ണസംഖ്യ
ffi_closure_helper_SYSV (ffi_cif *cif,
			 ശൂന്യം (*fun) (ffi_cif *, ശൂന്യം *, ശൂന്യം **, ശൂന്യം *),
			 ശൂന്യം *user_data,
			 ശൂന്യം *rvalue,
			 unsigned long *pgr,
			 ffi_dblfl *pfr,
			 unsigned long *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  ശൂന്യം **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
#ifndef __NO_FPRS__
  long             nf = 0;   /* number of floating registers already used */
#endif
  long             ng = 0;   /* number of general registers already used */

  unsigned       size     = cif->rtype->size;
  unsigned short rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  എങ്കിൽ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8))
    {
      rvalue = (ശൂന്യം *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ആകുമ്പോൾ (i < avn) {
    unsigned short typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    തിരഞ്ഞെടുപ്പ് (typenum)
      {
#ifndef __NO_FPRS__
      അവസ്ഥ FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	എങ്കിൽ (nf < അക്കം_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    ഇരട്ട temp = pfr->d;
	    pfr->f = (ദശാംശം) temp;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_DOUBLE:
	എങ്കിൽ (nf < അക്കം_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    എങ്കിൽ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	നിർത്തുക;

# എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	എങ്കിൽ (nf < അക്കം_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	അല്ലെങ്കിൽ
	  {
	    എങ്കിൽ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	നിർത്തുക;
# endif
#endif

      അവസ്ഥ FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_SINT8:
      അവസ്ഥ FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (അക്ഷരം *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = (അക്ഷരം *) pst + 3;
	    pst++;
	  }
	നിർത്തുക;
#endif

      അവസ്ഥ FFI_TYPE_SINT16:
      അവസ്ഥ FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (അക്ഷരം *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = (അക്ഷരം *) pst + 2;
	    pst++;
	  }
	നിർത്തുക;
#endif

      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_POINTER:
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ശൂന്യം *) *pgr;
	    ng++;
	    pgr++;
	  }
	അല്ലെങ്കിൽ
	  {
	    avalue[i] = (ശൂന്യം *) *pst;
	    pst++;
	  }
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_SINT64:
      അവസ്ഥ FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	എങ്കിൽ (ng < അക്കം_GPR_ARG_REGISTERS - 1)
	  {
	    എങ്കിൽ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	അല്ലെങ്കിൽ
	  {
	    എങ്കിൽ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = അക്കം_GPR_ARG_REGISTERS;
	  }
	നിർത്തുക;

      സ്ഥിരസ്ഥിതി:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  തിരഞ്ഞെടുപ്പ് (rtypenum)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      തിരികെ PPC_LD_NONE;
    അവസ്ഥ FFI_TYPE_FLOAT:
      തിരികെ PPC_LD_F32;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      തിരികെ PPC_LD_F64;
#എങ്കിൽ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      തിരികെ PPC_LD_F128;
#endif
    അവസ്ഥ FFI_TYPE_UINT8:
      തിരികെ PPC_LD_U8;
    അവസ്ഥ FFI_TYPE_SINT8:
      തിരികെ PPC_LD_S8;
    അവസ്ഥ FFI_TYPE_UINT16:
      തിരികെ PPC_LD_U16;
    അവസ്ഥ FFI_TYPE_SINT16:
      തിരികെ PPC_LD_S16;
    അവസ്ഥ FFI_TYPE_UINT32:
      തിരികെ PPC_LD_U32;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      തിരികെ PPC_LD_S32;
    അവസ്ഥ FFI_TYPE_POINTER:
      തിരികെ PPC_LD_PTR;
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      തിരികെ PPC_LD_I64;
    അവസ്ഥ FFI_TYPE_UINT128:
      തിരികെ PPC32_LD_R3R6;
    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->abi & FFI_SYSV_STRUCT_RET)
	തിരഞ്ഞെടുപ്പ് (size)
	  {
	  അവസ്ഥ 1:
	    തിരികെ PPC_LD_U8;
	  അവസ്ഥ 2:
	    തിരികെ PPC_LD_U16;
	  അവസ്ഥ 3:
	    തിരികെ PPC32_SYSV_LD_STRUCT_3;
	  അവസ്ഥ 4:
	    തിരികെ PPC_LD_U32;
	  അവസ്ഥ 5:
	    തിരികെ PPC32_SYSV_LD_STRUCT_5;
	  അവസ്ഥ 6:
	    തിരികെ PPC32_SYSV_LD_STRUCT_6;
	  അവസ്ഥ 7:
	    തിരികെ PPC32_SYSV_LD_STRUCT_7;
	  അവസ്ഥ 8:
	    തിരികെ PPC_LD_I64;
	  }
      തിരികെ PPC_LD_NONE;
    സ്ഥിരസ്ഥിതി:
      abort();
    }
}
#endif

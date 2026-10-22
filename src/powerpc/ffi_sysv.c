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

#ಅನ್ತರ್ಭಾವ "ffi.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ POWERPC64
#ಅನ್ತರ್ಭಾವ "ffi_common.h"
#ಅನ್ತರ್ಭಾವ "ffi_powerpc.h"


/* About the SYSV ABI.  */
#ವ್ಯಾಖ್ಯಾನಿಸು ASM_NEEDS_REGISTERS 6
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_GPR_ARG_REGISTERS 8
#ವ್ಯಾಖ್ಯಾನಿಸು ಅಂಕೆ_FPR_ARG_REGISTERS 8


#ಆದರೆ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  ಆದರೆ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.ಗಾತ್ರ_2 = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ffi_type_longdouble.ಗಾತ್ರ_2 = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#ಆದರೆ_ಅಂತ್ಯ

/* Transform long double, double and float to other types as per abi.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
translate_float (ಪೂರ್ಣಾಂಕ abi, ಪೂರ್ಣಾಂಕ type)
{
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  ಆದರೆ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      ಆದರೆ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      ಆದರೆ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#ಆದರೆ_ಅಂತ್ಯ
    }
  ಹಿಂತಿರುಗಿ type;
}

/* Perform machine dependent cif processing */
ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು;
  ಚಿಹ್ನೆರಹಿತ i, fpr_ಎಣಿಕೆ = 0, gpr_ಎಣಿಕೆ = 0, ರಾಶಿ_ಎಣಿಕೆ = 0;
  ಚಿಹ್ನೆರಹಿತ flags = cif->flags;
  ಚಿಹ್ನೆರಹಿತ struct_copy_ಗಾತ್ರ = 0;
  ಚಿಹ್ನೆರಹಿತ type = cif->rtype->type;
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2 = cif->rtype->ಗಾತ್ರ_2;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  ಬೈಟುಗಳು = (2 + ASM_NEEDS_REGISTERS) * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);

  /* Space for the GPR registers.  */
  ಬೈಟುಗಳು += ಅಂಕೆ_GPR_ARG_REGISTERS * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);

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

  ಆಯ್ಕೆ (type)
    {
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NO_FPRS__
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      ಆದರೆ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && ಗಾತ್ರ_2 <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  ನಿಲ್ಲಿಸು;
	}
      gpr_ಎಣಿಕೆ++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ನಿಲ್ಲಿಸು;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      ಆಯ್ಕೆ (typenum)
	{
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  ಆದರೆ (fpr_ಎಣಿಕೆ >= ಅಂಕೆ_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_ಎಣಿಕೆ = ಅಂಕೆ_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      ರಾಶಿ_ಎಣಿಕೆ += ರಾಶಿ_ಎಣಿಕೆ & 1;
	      ರಾಶಿ_ಎಣಿಕೆ += 4;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    fpr_ಎಣಿಕೆ += 2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NO_FPRS__
	  ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (fpr_ಎಣಿಕೆ >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      ರಾಶಿ_ಎಣಿಕೆ += ರಾಶಿ_ಎಣಿಕೆ & 1;
	      ರಾಶಿ_ಎಣಿಕೆ += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    fpr_ಎಣಿಕೆ += 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NO_FPRS__
	  ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಆದರೆ (fpr_ಎಣಿಕೆ >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    ರಾಶಿ_ಎಣಿಕೆ += 1;
	  ಇಲ್ಲದಿದ್ದರೆ
	    fpr_ಎಣಿಕೆ += 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __NO_FPRS__
	  ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  ಆದರೆ (gpr_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS - 3)
	    gpr_ಎಣಿಕೆ = ಅಂಕೆ_GPR_ARG_REGISTERS;
	  ಆದರೆ (gpr_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS)
	    ರಾಶಿ_ಎಣಿಕೆ += 4;
	  ಇಲ್ಲದಿದ್ದರೆ
	    gpr_ಎಣಿಕೆ += 4;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_ಎಣಿಕೆ += gpr_ಎಣಿಕೆ & 1;
	  ಆದರೆ (gpr_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS)
	    {
	      ರಾಶಿ_ಎಣಿಕೆ += ರಾಶಿ_ಎಣಿಕೆ & 1;
	      ರಾಶಿ_ಎಣಿಕೆ += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    gpr_ಎಣಿಕೆ += 2;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_ಗಾತ್ರ += ((*ptr)->ಗಾತ್ರ_2 + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  ಆದರೆ (gpr_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS)
	    ರಾಶಿ_ಎಣಿಕೆ += 1;
	  ಇಲ್ಲದಿದ್ದರೆ
	    gpr_ಎಣಿಕೆ += 1;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT (0);
	}
    }

  ಆದರೆ (fpr_ಎಣಿಕೆ != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ಆದರೆ (gpr_ಎಣಿಕೆ > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ಆದರೆ (struct_copy_ಗಾತ್ರ != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  ಆದರೆ (fpr_ಎಣಿಕೆ != 0)
    ಬೈಟುಗಳು += ಅಂಕೆ_FPR_ARG_REGISTERS * ಗಾತ್ರ (ದ್ವಿಗುಣ);

  /* Stack space.  */
  ಬೈಟುಗಳು += ರಾಶಿ_ಎಣಿಕೆ * ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ಬೈಟುಗಳು = (ಬೈಟುಗಳು + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  ಬೈಟುಗಳು += struct_copy_ಗಾತ್ರ;

  cif->flags = flags;
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  ಆದರೆ ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      ಆಯ್ಕೆ (cif->abi)
	{
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

	ಸ್ಥಿತಿ FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  ನಿಲ್ಲಿಸು;
	}
    }
  ಹಿಂತಿರುಗಿ ffi_prep_cif_sysv_core (cif);
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

ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_ವಾದಗಳು_SYSV (extended_cif *ecif, ಚಿಹ್ನೆರಹಿತ *ಸ್ಥಿರಾಂಕ ರಾಶಿ)
{
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು = ecif->cif->ಬೈಟುಗಳು;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ flags = ecif->cif->flags;

  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ
  {
    ಅಕ್ಷರ *c;
    ಚಿಹ್ನೆರಹಿತ *u;
    ದೀರ್ಘ ದೀರ್ಘ *ll;
    ದಶಮಾಂಶ *f;
    ದ್ವಿಗುಣ *d;
  } valp;

  /* 'stacktop' points at the previous backchain pointer.  */
  valp stacktop;

  /* 'gpr_base' points at the space for gpr3, and grows upwards as
     we use GPR registers.  */
  valp gpr_ತಳ;
  valp gpr_ಕೊನೆ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  valp fpr_ತಳ;
  valp fpr_ಕೊನೆ;
#ಆದರೆ_ಅಂತ್ಯ

  /* 'copy_space' grows down as we put structures in it.  It should
     stay 16-byte aligned.  */
  valp copy_space;

  /* 'next_arg' grows up as we put parameters in it.  */
  valp ಮುಂದಿನ_arg;

  ಪೂರ್ಣಾಂಕ i;
  ffi_type **ptr;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
  ದ್ವಿಗುಣ double_tmp;
#ಆದರೆ_ಅಂತ್ಯ
  ಒಕ್ಕೂಟ
  {
    ಶೂನ್ಯ **v;
    ಅಕ್ಷರ **c;
    ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ **sc;
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ **uc;
    ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ **ss;
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ **us;
    ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ **ui;
    ದೀರ್ಘ ದೀರ್ಘ **ll;
    ದಶಮಾಂಶ **f;
    ದ್ವಿಗುಣ **d;
  } p_argv;
  size_t struct_copy_ಗಾತ್ರ;
  ಚಿಹ್ನೆರಹಿತ gprvalue;

  stacktop.c = (ಅಕ್ಷರ *) ರಾಶಿ + ಬೈಟುಗಳು;
  gpr_ಕೊನೆ.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_ತಳ.u = gpr_ಕೊನೆ.u - ಅಂಕೆ_GPR_ARG_REGISTERS;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
  fpr_ಕೊನೆ.d = gpr_ತಳ.d;
  fpr_ತಳ.d = fpr_ಕೊನೆ.d - ಅಂಕೆ_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_ತಳ.c : gpr_ತಳ.c);
#ಇಲ್ಲದಿದ್ದರೆ
  copy_space.c = gpr_ತಳ.c;
#ಆದರೆ_ಅಂತ್ಯ
  ಮುಂದಿನ_arg.u = ರಾಶಿ + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ಅಕ್ಷರ *) ರಾಶಿ & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((ಬೈಟುಗಳು & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= ಮುಂದಿನ_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  ಆದರೆ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_ತಳ.u++ = (ಚಿಹ್ನೆರಹಿತ) (ಅಕ್ಷರ *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  ಗಾಗಿ (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      ಆಯ್ಕೆ (typenum)
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
# ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  ಆದರೆ (fpr_ತಳ.d >= fpr_ಕೊನೆ.d - 1)
	    {
	      fpr_ತಳ.d = fpr_ಕೊನೆ.d;
	      ಆದರೆ (((ಮುಂದಿನ_arg.u - ರಾಶಿ) & 1) != 0)
		ಮುಂದಿನ_arg.u += 1;
	      *ಮುಂದಿನ_arg.d = double_tmp;
	      ಮುಂದಿನ_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *ಮುಂದಿನ_arg.d = double_tmp;
	      ಮುಂದಿನ_arg.u += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      *fpr_ತಳ.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_ತಳ.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;
# endif
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  ಆದರೆ (fpr_ತಳ.d >= fpr_ಕೊನೆ.d)
	    {
	      ಆದರೆ (((ಮುಂದಿನ_arg.u - ರಾಶಿ) & 1) != 0)
		ಮುಂದಿನ_arg.u += 1;
	      *ಮುಂದಿನ_arg.d = double_tmp;
	      ಮುಂದಿನ_arg.u += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    *fpr_ತಳ.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  ಆದರೆ (fpr_ತಳ.d >= fpr_ಕೊನೆ.d)
	    {
	      *ಮುಂದಿನ_arg.f = (ದಶಮಾಂಶ) double_tmp;
	      ಮುಂದಿನ_arg.u += 1;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    *fpr_ತಳ.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* have FPRs */

	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  ಆದರೆ (gpr_ತಳ.u >= gpr_ಕೊನೆ.u - 3)
	    {
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ii;
	      gpr_ತಳ.u = gpr_ಕೊನೆ.u;
	      ಗಾಗಿ (ii = 0; ii < 4; ii++)
		{
		  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ int_tmp = (*p_argv.ui)[ii];
		  *ಮುಂದಿನ_arg.u++ = int_tmp;
		}
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ii;
	      ಗಾಗಿ (ii = 0; ii < 4; ii++)
		{
		  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ int_tmp = (*p_argv.ui)[ii];
		  *gpr_ತಳ.u++ = int_tmp;
		}
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  ಆದರೆ (gpr_ತಳ.u >= gpr_ಕೊನೆ.u - 1)
	    {
	      gpr_ತಳ.u = gpr_ಕೊನೆ.u;
	      ಆದರೆ (((ಮುಂದಿನ_arg.u - ರಾಶಿ) & 1) != 0)
		ಮುಂದಿನ_arg.u++;
	      *ಮುಂದಿನ_arg.ll = **p_argv.ll;
	      ಮುಂದಿನ_arg.u += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      ಆದರೆ (((gpr_ಕೊನೆ.u - gpr_ತಳ.u) & 1) != 0)
		gpr_ತಳ.u++;
	      *gpr_ತಳ.ll++ = **p_argv.ll;
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  struct_copy_ಗಾತ್ರ = ((*ptr)->ಗಾತ್ರ_2 + 15) & ~0xF;
	  copy_space.c -= struct_copy_ಗಾತ್ರ;
	  ಸ್ಮರಣೆ_ನಕಲು (copy_space.c, *p_argv.c, (*ptr)->ಗಾತ್ರ_2);

	  gprvalue = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) copy_space.c;

	  FFI_ASSERT (copy_space.c > ಮುಂದಿನ_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  ಹೋಗು putgpr;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  ಹೋಗು putgpr;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:

	  gprvalue = **p_argv.ui;

	putgpr:
	  ಆದರೆ (gpr_ತಳ.u >= gpr_ಕೊನೆ.u)
	    *ಮುಂದಿನ_arg.u++ = gprvalue;
	  ಇಲ್ಲದಿದ್ದರೆ
	    *gpr_ತಳ.u++ = gprvalue;
	  ನಿಲ್ಲಿಸು;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= ಮುಂದಿನ_arg.c);
  FFI_ASSERT (gpr_ತಳ.u <= gpr_ಕೊನೆ.u);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
  FFI_ASSERT (fpr_ತಳ.u <= fpr_ಕೊನೆ.u);
#ಆದರೆ_ಅಂತ್ಯ
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_ಕೊನೆ.u - gpr_ತಳ.u < 4));
}

#ವ್ಯಾಖ್ಯಾನಿಸು MIN_CACHE_ಸಾಲು_ಗಾತ್ರ 8

ಸ್ಥಿರ ಶೂನ್ಯ
flush_icache (ಅಕ್ಷರ *wraddr, ಅಕ್ಷರ *xaddr, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_2; i += MIN_CACHE_ಸಾಲು_ಗಾತ್ರ)
    __asm__ ಅಸ್ಥಿರ ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ಅಸ್ಥಿರ ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + ಗಾತ್ರ_2 - 1), "r"(wraddr + ಗಾತ್ರ_2 - 1)
		    : "memory");
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_ಆವರಣ_loc_sysv (ffi_ಆವರಣ *ಆವರಣ,
			   ffi_cif *cif,
			   ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			   ಶೂನ್ಯ *codeloc)
{
  ಆದರೆ (cif->abi < FFI_SYSV || cif->abi >= FFI_ಕೊನೆಯ_ABI)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_EXEC_STATIC_TRAMP
  ಆದರೆ (ffi_tramp_is_present(ಆವರಣ))
    {
      /* Initialize the static trampoline's parameters. */
      ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ) = ffi_ಆವರಣ_SYSV;
      ffi_tramp_ಗಣ_parms (ಆವರಣ->ftramp, dest, ಆವರಣ);
    }
  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(ಶೂನ್ಯ **) &tramp[8] = (ಶೂನ್ಯ *) ffi_ಆವರಣ_SYSV; /* function */
      *(ಶೂನ್ಯ **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((ಅಕ್ಷರ *)tramp, (ಅಕ್ಷರ *)codeloc, 8 * 4);
    }

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_SYSV (ffi_cif *cif,
			 ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
			 ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			 ಶೂನ್ಯ *rvalue,
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pgr,
			 ffi_dblfl *pfr,
			 ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  ಶೂನ್ಯ **          avalue;
  ffi_type **      arg_types;
  ದೀರ್ಘ             i, avn;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
  ದೀರ್ಘ             nf = 0;   /* number of floating registers already used */
#ಆದರೆ_ಅಂತ್ಯ
  ದೀರ್ಘ             ng = 0;   /* number of general registers already used */

  ಚಿಹ್ನೆರಹಿತ       ಗಾತ್ರ_2     = cif->rtype->ಗಾತ್ರ_2;
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  ಆದರೆ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && ಗಾತ್ರ_2 <= 8))
    {
      rvalue = (ಶೂನ್ಯ *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಆಗ (i < avn) {
    ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    ಆಯ್ಕೆ (typenum)
      {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __NO_FPRS__
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	ಆದರೆ (nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    ದ್ವಿಗುಣ ತಾತ್ಕಾಲಿಕ = pfr->d;
	    pfr->f = (ದಶಮಾಂಶ) ತಾತ್ಕಾಲಿಕ;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಆದರೆ (nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    ಆದರೆ (((ದೀರ್ಘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	ನಿಲ್ಲಿಸು;

# ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಆದರೆ (nf < ಅಂಕೆ_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    ಆದರೆ (((ದೀರ್ಘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	ನಿಲ್ಲಿಸು;
# endif
#ಆದರೆ_ಅಂತ್ಯ

      ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ಅಕ್ಷರ *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = (ಅಕ್ಷರ *) pst + 3;
	    pst++;
	  }
	ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ಅಕ್ಷರ *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = (ಅಕ್ಷರ *) pst + 2;
	    pst++;
	  }
	ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (ಶೂನ್ಯ *) *pgr;
	    ng++;
	    pgr++;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    avalue[i] = (ಶೂನ್ಯ *) *pst;
	    pst++;
	  }
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	ಆದರೆ (ng < ಅಂಕೆ_GPR_ARG_REGISTERS - 1)
	  {
	    ಆದರೆ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    ಆದರೆ (((ದೀರ್ಘ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = ಅಂಕೆ_GPR_ARG_REGISTERS;
	  }
	ನಿಲ್ಲಿಸು;

      ಪೂರ್ವನಿಯೋಜಿತ:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  ಆಯ್ಕೆ (rtypenum)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಹಿಂತಿರುಗಿ PPC_LD_F32;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ಹಿಂತಿರುಗಿ PPC_LD_F64;
#ಆದರೆ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ಹಿಂತಿರುಗಿ PPC_LD_F128;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_U8;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_S8;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_U16;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_S16;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_U32;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_S32;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಹಿಂತಿರುಗಿ PPC_LD_PTR;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ಹಿಂತಿರುಗಿ PPC_LD_I64;
    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
      ಹಿಂತಿರುಗಿ PPC32_LD_R3R6;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (cif->abi & FFI_SYSV_STRUCT_RET)
	ಆಯ್ಕೆ (ಗಾತ್ರ_2)
	  {
	  ಸ್ಥಿತಿ 1:
	    ಹಿಂತಿರುಗಿ PPC_LD_U8;
	  ಸ್ಥಿತಿ 2:
	    ಹಿಂತಿರುಗಿ PPC_LD_U16;
	  ಸ್ಥಿತಿ 3:
	    ಹಿಂತಿರುಗಿ PPC32_SYSV_LD_STRUCT_3;
	  ಸ್ಥಿತಿ 4:
	    ಹಿಂತಿರುಗಿ PPC_LD_U32;
	  ಸ್ಥಿತಿ 5:
	    ಹಿಂತಿರುಗಿ PPC32_SYSV_LD_STRUCT_5;
	  ಸ್ಥಿತಿ 6:
	    ಹಿಂತಿರುಗಿ PPC32_SYSV_LD_STRUCT_6;
	  ಸ್ಥಿತಿ 7:
	    ಹಿಂತಿರುಗಿ PPC32_SYSV_LD_STRUCT_7;
	  ಸ್ಥಿತಿ 8:
	    ಹಿಂತಿರುಗಿ PPC_LD_I64;
	  }
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
}
#ಆದರೆ_ಅಂತ್ಯ

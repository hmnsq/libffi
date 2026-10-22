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

#include "ffi.h"
#include <tramp.h>
#include <stdlib.h>

#ifndef POWERPC64
#include "ffi_common.h"
#include "ffi_powerpc.h"


/* About the SYSV ABI.  */
#define ASM_NEEDS_REGISTERS 6
#define NUM_GPR_ARG_REGISTERS 8
#define NUM_FPR_ARG_REGISTERS 8


#ᰀᰧ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
void FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  ᰀᰧ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  ᰀᰧᰀ
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
static puur_naa_nka
translate_float (puur_naa_nka abi, puur_naa_nka type)
{
#ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  ᰀᰧ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  ᰀᰧ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      ᰀᰧ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      ᰀᰧᰀ ᰀᰧ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ᰀᰧᰀ ᰀᰧ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  ᰀᰧᰀ ᰀᰧ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      ᰀᰧ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  ᰀᰧᰌ type;
}

/* Perform machine dependent cif processing */
static ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fpr_count = 0, gpr_count = 0, stack_count = 0;
  unsigned flags = cif->flags;
  unsigned struct_copy_size = 0;
  unsigned type = cif->rtype->type;
  unsigned size = cif->rtype->size;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * parimaa_na (puur_naa_nka);

  /* Space for the GPR registers.  */
  bytes += NUM_GPR_ARG_REGISTERS * parimaa_na (puur_naa_nka);

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

  switch (type)
    {
#ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    pak_sa FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    pak_sa FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    pak_sa FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      ᰀᰧᰌ FFI_BAD_ABI;
#endif
      bha_nga;

    pak_sa FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      ᰀᰧ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  bha_nga;
	}
      gpr_count++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    pak_sa FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      bha_nga;

    default:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      bha_nga;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  for (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned short typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      switch (typenum)
	{
#ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
	  ᰀᰧ (fpr_count >= NUM_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_count = NUM_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      stack_count += stack_count & 1;
	      stack_count += 4;
	    }
	  ᰀᰧᰀ
	    fpr_count += 2;
#ifdef __NO_FPRS__
	  ᰀᰧᰌ FFI_BAD_ABI;
#endif
	  bha_nga;
#endif

	pak_sa FFI_TYPE_DOUBLE:
	  ᰀᰧ (fpr_count >= NUM_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      stack_count += stack_count & 1;
	      stack_count += 2;
	    }
	  ᰀᰧᰀ
	    fpr_count += 1;
#ifdef __NO_FPRS__
	  ᰀᰧᰌ FFI_BAD_ABI;
#endif
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	  ᰀᰧ (fpr_count >= NUM_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    stack_count += 1;
	  ᰀᰧᰀ
	    fpr_count += 1;
#ifdef __NO_FPRS__
	  ᰀᰧᰌ FFI_BAD_ABI;
#endif
	  bha_nga;

	pak_sa FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  ᰀᰧ (gpr_count >= NUM_GPR_ARG_REGISTERS - 3)
	    gpr_count = NUM_GPR_ARG_REGISTERS;
	  ᰀᰧ (gpr_count >= NUM_GPR_ARG_REGISTERS)
	    stack_count += 4;
	  ᰀᰧᰀ
	    gpr_count += 4;
	  bha_nga;

	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_count += gpr_count & 1;
	  ᰀᰧ (gpr_count >= NUM_GPR_ARG_REGISTERS)
	    {
	      stack_count += stack_count & 1;
	      stack_count += 2;
	    }
	  ᰀᰧᰀ
	    gpr_count += 2;
	  bha_nga;

	pak_sa FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_size += ((*ptr)->size + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	pak_sa FFI_TYPE_POINTER:
	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT16:
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT8:
	pak_sa FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  ᰀᰧ (gpr_count >= NUM_GPR_ARG_REGISTERS)
	    stack_count += 1;
	  ᰀᰧᰀ
	    gpr_count += 1;
	  bha_nga;

	default:
	  FFI_ASSERT (0);
	}
    }

  ᰀᰧ (fpr_count != 0)
    flags |= FLAG_FP_ARGUMENTS;
  ᰀᰧ (gpr_count > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  ᰀᰧ (struct_copy_size != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  ᰀᰧ (fpr_count != 0)
    bytes += NUM_FPR_ARG_REGISTERS * parimaa_na (dviguu_na);

  /* Stack space.  */
  bytes += stack_count * parimaa_na (puur_naa_nka);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_size;

  cif->flags = flags;
  cif->bytes = bytes;

  ᰀᰧᰌ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  ᰀᰧ ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      switch (cif->abi)
	{
	default:
	  ᰀᰧᰌ FFI_BAD_ABI;

	pak_sa FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  bha_nga;

	pak_sa FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  bha_nga;

	pak_sa FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  bha_nga;

	pak_sa FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  bha_nga;
	}
    }
  ᰀᰧᰌ ffi_prep_cif_sysv_core (cif);
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

void FFI_HIDDEN
ffi_prep_args_SYSV (extended_cif *ecif, unsigned *const stack)
{
  const unsigned bytes = ecif->cif->bytes;
  const unsigned flags = ecif->cif->flags;

  typedef sa_ngha
  {
    var_na *c;
    unsigned *u;
    long long *ll;
    float *f;
    dviguu_na *d;
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

  puur_naa_nka i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  dviguu_na double_tmp;
#endif
  sa_ngha
  {
    void **v;
    var_na **c;
    signed var_na **sc;
    unsigned var_na **uc;
    signed short **ss;
    unsigned short **us;
    unsigned puur_naa_nka **ui;
    long long **ll;
    float **f;
    dviguu_na **d;
  } p_argv;
  size_t struct_copy_size;
  unsigned gprvalue;

  stacktop.c = (var_na *) stack + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - NUM_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - NUM_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#ᰀᰧᰀ
  copy_space.c = gpr_base.c;
#endif
  next_arg.u = stack + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (var_na *) stack & 0xF) == 0);
  FFI_ASSERT (((unsigned long) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= next_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  ᰀᰧ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (unsigned) (var_na *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  for (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      unsigned puur_naa_nka typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      switch (typenum)
	{
#ifndef __NO_FPRS__
# ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	pak_sa FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  ᰀᰧ (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      ᰀᰧ (((next_arg.u - stack) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  ᰀᰧᰀ
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  bha_nga;
# endif
	pak_sa FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  ᰀᰧ (fpr_base.d >= fpr_end.d)
	    {
	      ᰀᰧ (((next_arg.u - stack) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  ᰀᰧᰀ
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  bha_nga;

	pak_sa FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  ᰀᰧ (fpr_base.d >= fpr_end.d)
	    {
	      *next_arg.f = (float) double_tmp;
	      next_arg.u += 1;
	    }
	  ᰀᰧᰀ
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  bha_nga;
#endif /* have FPRs */

	pak_sa FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  ᰀᰧ (gpr_base.u >= gpr_end.u - 3)
	    {
	      unsigned puur_naa_nka ii;
	      gpr_base.u = gpr_end.u;
	      for (ii = 0; ii < 4; ii++)
		{
		  unsigned puur_naa_nka int_tmp = (*p_argv.ui)[ii];
		  *next_arg.u++ = int_tmp;
		}
	    }
	  ᰀᰧᰀ
	    {
	      unsigned puur_naa_nka ii;
	      for (ii = 0; ii < 4; ii++)
		{
		  unsigned puur_naa_nka int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  bha_nga;

	pak_sa FFI_TYPE_UINT64:
	pak_sa FFI_TYPE_SINT64:
	  ᰀᰧ (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      ᰀᰧ (((next_arg.u - stack) & 1) != 0)
		next_arg.u++;
	      *next_arg.ll = **p_argv.ll;
	      next_arg.u += 2;
	    }
	  ᰀᰧᰀ
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      ᰀᰧ (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  bha_nga;

	pak_sa FFI_TYPE_STRUCT:
	  struct_copy_size = ((*ptr)->size + 15) & ~0xF;
	  copy_space.c -= struct_copy_size;
	  memcpy (copy_space.c, *p_argv.c, (*ptr)->size);

	  gprvalue = (unsigned long) copy_space.c;

	  FFI_ASSERT (copy_space.c > next_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  goto putgpr;

	pak_sa FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  goto putgpr;
	pak_sa FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  goto putgpr;
	pak_sa FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  goto putgpr;
	pak_sa FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  goto putgpr;

	pak_sa FFI_TYPE_INT:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  ᰀᰧ (gpr_base.u >= gpr_end.u)
	    *next_arg.u++ = gprvalue;
	  ᰀᰧᰀ
	    *gpr_base.u++ = gprvalue;
	  bha_nga;
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

#define MIN_CACHE_LINE_SIZE 8

static void
flush_icache (var_na *wraddr, var_na *xaddr, puur_naa_nka size)
{
  puur_naa_nka i;
  for (i = 0; i < size; i += MIN_CACHE_LINE_SIZE)
    __asm__ ca_ncala ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ ca_ncala ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   void (*fun) (ffi_cif *, void *, void **, void *),
			   void *user_data,
			   void *codeloc)
{
  ᰀᰧ (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    ᰀᰧᰌ FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  ᰀᰧ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      void (*dest)(void) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  ᰀᰧᰀ
#endif
    {
      unsigned puur_naa_nka *tramp = (unsigned puur_naa_nka *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(void **) &tramp[8] = (void *) ffi_closure_SYSV; /* function */
      *(void **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((var_na *)tramp, (var_na *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ᰀᰧᰌ FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

puur_naa_nka
ffi_closure_helper_SYSV (ffi_cif *cif,
			 void (*fun) (ffi_cif *, void *, void **, void *),
			 void *user_data,
			 void *rvalue,
			 unsigned long *pgr,
			 ffi_dblfl *pfr,
			 unsigned long *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  void **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
#ifndef __NO_FPRS__
  long             nf = 0;   /* number of floating registers already used */
#endif
  long             ng = 0;   /* number of general registers already used */

  unsigned       size     = cif->rtype->size;
  unsigned short rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * parimaa_na (void *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  ᰀᰧ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8))
    {
      rvalue = (void *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ᰀᰬ (i < avn) {
    unsigned short typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    switch (typenum)
      {
#ifndef __NO_FPRS__
      pak_sa FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	ᰀᰧ (nf < NUM_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    dviguu_na temp = pfr->d;
	    pfr->f = (float) temp;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	bha_nga;

      pak_sa FFI_TYPE_DOUBLE:
	ᰀᰧ (nf < NUM_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	ᰀᰧᰀ
	  {
	    ᰀᰧ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	bha_nga;

# ᰀᰧ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      pak_sa FFI_TYPE_LONGDOUBLE:
	ᰀᰧ (nf < NUM_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	ᰀᰧᰀ
	  {
	    ᰀᰧ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	bha_nga;
# endif
#endif

      pak_sa FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	bha_nga;

      pak_sa FFI_TYPE_SINT8:
      pak_sa FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (var_na *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = (var_na *) pst + 3;
	    pst++;
	  }
	bha_nga;
#endif

      pak_sa FFI_TYPE_SINT16:
      pak_sa FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (var_na *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = (var_na *) pst + 2;
	    pst++;
	  }
	bha_nga;
#endif

      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_POINTER:
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	bha_nga;

      pak_sa FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (void *) *pgr;
	    ng++;
	    pgr++;
	  }
	ᰀᰧᰀ
	  {
	    avalue[i] = (void *) *pst;
	    pst++;
	  }
	bha_nga;

      pak_sa FFI_TYPE_SINT64:
      pak_sa FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	ᰀᰧ (ng < NUM_GPR_ARG_REGISTERS - 1)
	  {
	    ᰀᰧ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	ᰀᰧᰀ
	  {
	    ᰀᰧ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = NUM_GPR_ARG_REGISTERS;
	  }
	bha_nga;

      default:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  switch (rtypenum)
    {
    pak_sa FFI_TYPE_VOID:
      ᰀᰧᰌ PPC_LD_NONE;
    pak_sa FFI_TYPE_FLOAT:
      ᰀᰧᰌ PPC_LD_F32;
    pak_sa FFI_TYPE_DOUBLE:
      ᰀᰧᰌ PPC_LD_F64;
#ᰀᰧ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    pak_sa FFI_TYPE_LONGDOUBLE:
      ᰀᰧᰌ PPC_LD_F128;
#endif
    pak_sa FFI_TYPE_UINT8:
      ᰀᰧᰌ PPC_LD_U8;
    pak_sa FFI_TYPE_SINT8:
      ᰀᰧᰌ PPC_LD_S8;
    pak_sa FFI_TYPE_UINT16:
      ᰀᰧᰌ PPC_LD_U16;
    pak_sa FFI_TYPE_SINT16:
      ᰀᰧᰌ PPC_LD_S16;
    pak_sa FFI_TYPE_UINT32:
      ᰀᰧᰌ PPC_LD_U32;
    pak_sa FFI_TYPE_INT:
    pak_sa FFI_TYPE_SINT32:
      ᰀᰧᰌ PPC_LD_S32;
    pak_sa FFI_TYPE_POINTER:
      ᰀᰧᰌ PPC_LD_PTR;
    pak_sa FFI_TYPE_UINT64:
    pak_sa FFI_TYPE_SINT64:
      ᰀᰧᰌ PPC_LD_I64;
    pak_sa FFI_TYPE_UINT128:
      ᰀᰧᰌ PPC32_LD_R3R6;
    pak_sa FFI_TYPE_STRUCT:
      ᰀᰧ (cif->abi & FFI_SYSV_STRUCT_RET)
	switch (size)
	  {
	  pak_sa 1:
	    ᰀᰧᰌ PPC_LD_U8;
	  pak_sa 2:
	    ᰀᰧᰌ PPC_LD_U16;
	  pak_sa 3:
	    ᰀᰧᰌ PPC32_SYSV_LD_STRUCT_3;
	  pak_sa 4:
	    ᰀᰧᰌ PPC_LD_U32;
	  pak_sa 5:
	    ᰀᰧᰌ PPC32_SYSV_LD_STRUCT_5;
	  pak_sa 6:
	    ᰀᰧᰌ PPC32_SYSV_LD_STRUCT_6;
	  pak_sa 7:
	    ᰀᰧᰌ PPC32_SYSV_LD_STRUCT_7;
	  pak_sa 8:
	    ᰀᰧᰌ PPC_LD_I64;
	  }
      ᰀᰧᰌ PPC_LD_NONE;
    default:
      abort();
    }
}
#endif

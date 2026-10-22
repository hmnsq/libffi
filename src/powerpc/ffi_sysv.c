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

#অন্তর্ভাব "ffi.h"
#অন্তর্ভাব <tramp.h>
#অন্তর্ভাব <stdlib.h>

#ifndef POWERPC64
#অন্তর্ভাব "ffi_common.h"
#অন্তর্ভাব "ffi_powerpc.h"


/* About the SYSV ABI.  */
#define ASM_NEEDS_REGISTERS 6
#define অঙ্ক_GPR_ARG_REGISTERS 8
#define অঙ্ক_FPR_ARG_REGISTERS 8


#যদি HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
শূন্য FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  যদি ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.আকার_2 = 8;
      ffi_type_longdouble.alignment = 8;
    }
  অন্যথা
    {
      ffi_type_longdouble.আকার_2 = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
স্থির পূর্ণসংখ্যা
translate_float (পূর্ণসংখ্যা abi, পূর্ণসংখ্যা type)
{
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  যদি (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  যদি ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      যদি (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      অন্যথা যদি (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      অন্যথা যদি (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  অন্যথা যদি ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      যদি (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  ফেরত type;
}

/* Perform machine dependent cif processing */
স্থির ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fpr_গুনতি = 0, gpr_গুনতি = 0, স্তূপ_গুনতি = 0;
  unsigned flags = cif->flags;
  unsigned struct_copy_আকার = 0;
  unsigned type = cif->rtype->type;
  unsigned আকার_2 = cif->rtype->আকার_2;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * আকার (পূর্ণসংখ্যা);

  /* Space for the GPR registers.  */
  bytes += অঙ্ক_GPR_ARG_REGISTERS * আকার (পূর্ণসংখ্যা);

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

  নির্বাচন (type)
    {
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    ক্ষেত্র FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ক্ষেত্র FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      ফেরত FFI_BAD_ABI;
#endif
      বিরতি;

    ক্ষেত্র FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      যদি ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && আকার_2 <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  বিরতি;
	}
      gpr_গুনতি++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    ক্ষেত্র FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      বিরতি;

    পূর্বনির্ধারিত:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      বিরতি;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  জন্য (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned short typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      নির্বাচন (typenum)
	{
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  যদি (fpr_গুনতি >= অঙ্ক_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_গুনতি = অঙ্ক_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      স্তূপ_গুনতি += স্তূপ_গুনতি & 1;
	      স্তূপ_গুনতি += 4;
	    }
	  অন্যথা
	    fpr_গুনতি += 2;
#ifdef __NO_FPRS__
	  ফেরত FFI_BAD_ABI;
#endif
	  বিরতি;
#endif

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি (fpr_গুনতি >= অঙ্ক_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      স্তূপ_গুনতি += স্তূপ_গুনতি & 1;
	      স্তূপ_গুনতি += 2;
	    }
	  অন্যথা
	    fpr_গুনতি += 1;
#ifdef __NO_FPRS__
	  ফেরত FFI_BAD_ABI;
#endif
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  যদি (fpr_গুনতি >= অঙ্ক_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    স্তূপ_গুনতি += 1;
	  অন্যথা
	    fpr_গুনতি += 1;
#ifdef __NO_FPRS__
	  ফেরত FFI_BAD_ABI;
#endif
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  যদি (gpr_গুনতি >= অঙ্ক_GPR_ARG_REGISTERS - 3)
	    gpr_গুনতি = অঙ্ক_GPR_ARG_REGISTERS;
	  যদি (gpr_গুনতি >= অঙ্ক_GPR_ARG_REGISTERS)
	    স্তূপ_গুনতি += 4;
	  অন্যথা
	    gpr_গুনতি += 4;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_গুনতি += gpr_গুনতি & 1;
	  যদি (gpr_গুনতি >= অঙ্ক_GPR_ARG_REGISTERS)
	    {
	      স্তূপ_গুনতি += স্তূপ_গুনতি & 1;
	      স্তূপ_গুনতি += 2;
	    }
	  অন্যথা
	    gpr_গুনতি += 2;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_আকার += ((*ptr)->আকার_2 + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	ক্ষেত্র FFI_TYPE_POINTER:
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  যদি (gpr_গুনতি >= অঙ্ক_GPR_ARG_REGISTERS)
	    স্তূপ_গুনতি += 1;
	  অন্যথা
	    gpr_গুনতি += 1;
	  বিরতি;

	পূর্বনির্ধারিত:
	  FFI_ASSERT (0);
	}
    }

  যদি (fpr_গুনতি != 0)
    flags |= FLAG_FP_ARGUMENTS;
  যদি (gpr_গুনতি > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  যদি (struct_copy_আকার != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  যদি (fpr_গুনতি != 0)
    bytes += অঙ্ক_FPR_ARG_REGISTERS * আকার (দ্বিগুণ);

  /* Stack space.  */
  bytes += স্তূপ_গুনতি * আকার (পূর্ণসংখ্যা);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_আকার;

  cif->flags = flags;
  cif->bytes = bytes;

  ফেরত FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  যদি ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      নির্বাচন (cif->abi)
	{
	পূর্বনির্ধারিত:
	  ফেরত FFI_BAD_ABI;

	ক্ষেত্র FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  বিরতি;

	ক্ষেত্র FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  বিরতি;

	ক্ষেত্র FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  বিরতি;

	ক্ষেত্র FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  বিরতি;
	}
    }
  ফেরত ffi_prep_cif_sysv_core (cif);
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

শূন্য FFI_HIDDEN
ffi_prep_args_SYSV (extended_cif *ecif, unsigned *ধ্রুবক স্তূপ)
{
  ধ্রুবক unsigned bytes = ecif->cif->bytes;
  ধ্রুবক unsigned flags = ecif->cif->flags;

  প্রকারলেখ সংঘ
  {
    অক্ষর *c;
    unsigned *u;
    long long *ll;
    দশমিক *f;
    দ্বিগুণ *d;
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
  valp পরবর্তী_arg;

  পূর্ণসংখ্যা i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  দ্বিগুণ double_tmp;
#endif
  সংঘ
  {
    শূন্য **v;
    অক্ষর **c;
    signed অক্ষর **sc;
    unsigned অক্ষর **uc;
    signed short **ss;
    unsigned short **us;
    unsigned পূর্ণসংখ্যা **ui;
    long long **ll;
    দশমিক **f;
    দ্বিগুণ **d;
  } p_argv;
  size_t struct_copy_আকার;
  unsigned gprvalue;

  stacktop.c = (অক্ষর *) স্তূপ + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - অঙ্ক_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - অঙ্ক_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#অন্যথা
  copy_space.c = gpr_base.c;
#endif
  পরবর্তী_arg.u = স্তূপ + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (অক্ষর *) স্তূপ & 0xF) == 0);
  FFI_ASSERT (((unsigned long) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= পরবর্তী_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  যদি (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (unsigned) (অক্ষর *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  জন্য (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      unsigned পূর্ণসংখ্যা typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      নির্বাচন (typenum)
	{
#ifndef __NO_FPRS__
# যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  যদি (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      যদি (((পরবর্তী_arg.u - স্তূপ) & 1) != 0)
		পরবর্তী_arg.u += 1;
	      *পরবর্তী_arg.d = double_tmp;
	      পরবর্তী_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *পরবর্তী_arg.d = double_tmp;
	      পরবর্তী_arg.u += 2;
	    }
	  অন্যথা
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  বিরতি;
# endif
	ক্ষেত্র FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  যদি (fpr_base.d >= fpr_end.d)
	    {
	      যদি (((পরবর্তী_arg.u - স্তূপ) & 1) != 0)
		পরবর্তী_arg.u += 1;
	      *পরবর্তী_arg.d = double_tmp;
	      পরবর্তী_arg.u += 2;
	    }
	  অন্যথা
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  যদি (fpr_base.d >= fpr_end.d)
	    {
	      *পরবর্তী_arg.f = (দশমিক) double_tmp;
	      পরবর্তী_arg.u += 1;
	    }
	  অন্যথা
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  বিরতি;
#endif /* have FPRs */

	ক্ষেত্র FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  যদি (gpr_base.u >= gpr_end.u - 3)
	    {
	      unsigned পূর্ণসংখ্যা ii;
	      gpr_base.u = gpr_end.u;
	      জন্য (ii = 0; ii < 4; ii++)
		{
		  unsigned পূর্ণসংখ্যা int_tmp = (*p_argv.ui)[ii];
		  *পরবর্তী_arg.u++ = int_tmp;
		}
	    }
	  অন্যথা
	    {
	      unsigned পূর্ণসংখ্যা ii;
	      জন্য (ii = 0; ii < 4; ii++)
		{
		  unsigned পূর্ণসংখ্যা int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  যদি (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      যদি (((পরবর্তী_arg.u - স্তূপ) & 1) != 0)
		পরবর্তী_arg.u++;
	      *পরবর্তী_arg.ll = **p_argv.ll;
	      পরবর্তী_arg.u += 2;
	    }
	  অন্যথা
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      যদি (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  বিরতি;

	ক্ষেত্র FFI_TYPE_STRUCT:
	  struct_copy_আকার = ((*ptr)->আকার_2 + 15) & ~0xF;
	  copy_space.c -= struct_copy_আকার;
	  memcpy (copy_space.c, *p_argv.c, (*ptr)->আকার_2);

	  gprvalue = (unsigned long) copy_space.c;

	  FFI_ASSERT (copy_space.c > পরবর্তী_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  যাও putgpr;

	ক্ষেত্র FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  যাও putgpr;
	ক্ষেত্র FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  যাও putgpr;
	ক্ষেত্র FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  যাও putgpr;
	ক্ষেত্র FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  যাও putgpr;

	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  যদি (gpr_base.u >= gpr_end.u)
	    *পরবর্তী_arg.u++ = gprvalue;
	  অন্যথা
	    *gpr_base.u++ = gprvalue;
	  বিরতি;
	}
    }

  /* Check that we didn't overrun the stack...  */
  FFI_ASSERT (copy_space.c >= পরবর্তী_arg.c);
  FFI_ASSERT (gpr_base.u <= gpr_end.u);
#ifndef __NO_FPRS__
  FFI_ASSERT (fpr_base.u <= fpr_end.u);
#endif
  FFI_ASSERT (((flags & FLAG_4_GPR_ARGUMENTS) != 0)
	      == (gpr_end.u - gpr_base.u < 4));
}

#define MIN_CACHE_সারি_আকার 8

স্থির শূন্য
flush_icache (অক্ষর *wraddr, অক্ষর *xaddr, পূর্ণসংখ্যা আকার_2)
{
  পূর্ণসংখ্যা i;
  জন্য (i = 0; i < আকার_2; i += MIN_CACHE_সারি_আকার)
    __asm__ অস্থির ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ অস্থির ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + আকার_2 - 1), "r"(wraddr + আকার_2 - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
			   শূন্য *user_তথ্য,
			   শূন্য *codeloc)
{
  যদি (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    ফেরত FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      শূন্য (*dest)(শূন্য) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  অন্যথা
#endif
    {
      unsigned পূর্ণসংখ্যা *tramp = (unsigned পূর্ণসংখ্যা *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(শূন্য **) &tramp[8] = (শূন্য *) ffi_closure_SYSV; /* function */
      *(শূন্য **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((অক্ষর *)tramp, (অক্ষর *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

পূর্ণসংখ্যা
ffi_closure_helper_SYSV (ffi_cif *cif,
			 শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
			 শূন্য *user_তথ্য,
			 শূন্য *rvalue,
			 unsigned long *pgr,
			 ffi_dblfl *pfr,
			 unsigned long *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  শূন্য **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
#ifndef __NO_FPRS__
  long             nf = 0;   /* number of floating registers already used */
#endif
  long             ng = 0;   /* number of general registers already used */

  unsigned       আকার_2     = cif->rtype->আকার_2;
  unsigned short rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * আকার (শূন্য *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  যদি (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && আকার_2 <= 8))
    {
      rvalue = (শূন্য *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  যখন (i < avn) {
    unsigned short typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    নির্বাচন (typenum)
      {
#ifndef __NO_FPRS__
      ক্ষেত্র FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	যদি (nf < অঙ্ক_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    দ্বিগুণ অস্থায়ী = pfr->d;
	    pfr->f = (দশমিক) অস্থায়ী;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	অন্যথা
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	বিরতি;

      ক্ষেত্র FFI_TYPE_DOUBLE:
	যদি (nf < অঙ্ক_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	অন্যথা
	  {
	    যদি (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	বিরতি;

# যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	যদি (nf < অঙ্ক_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	অন্যথা
	  {
	    যদি (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	বিরতি;
# endif
#endif

      ক্ষেত্র FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	অন্যথা
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	বিরতি;

      ক্ষেত্র FFI_TYPE_SINT8:
      ক্ষেত্র FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (অক্ষর *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	অন্যথা
	  {
	    avalue[i] = (অক্ষর *) pst + 3;
	    pst++;
	  }
	বিরতি;
#endif

      ক্ষেত্র FFI_TYPE_SINT16:
      ক্ষেত্র FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (অক্ষর *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	অন্যথা
	  {
	    avalue[i] = (অক্ষর *) pst + 2;
	    pst++;
	  }
	বিরতি;
#endif

      ক্ষেত্র FFI_TYPE_SINT32:
      ক্ষেত্র FFI_TYPE_UINT32:
      ক্ষেত্র FFI_TYPE_POINTER:
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	অন্যথা
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	বিরতি;

      ক্ষেত্র FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (শূন্য *) *pgr;
	    ng++;
	    pgr++;
	  }
	অন্যথা
	  {
	    avalue[i] = (শূন্য *) *pst;
	    pst++;
	  }
	বিরতি;

      ক্ষেত্র FFI_TYPE_SINT64:
      ক্ষেত্র FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	যদি (ng < অঙ্ক_GPR_ARG_REGISTERS - 1)
	  {
	    যদি (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	অন্যথা
	  {
	    যদি (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = অঙ্ক_GPR_ARG_REGISTERS;
	  }
	বিরতি;

      পূর্বনির্ধারিত:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_তথ্য);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  নির্বাচন (rtypenum)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      ফেরত PPC_LD_NONE;
    ক্ষেত্র FFI_TYPE_FLOAT:
      ফেরত PPC_LD_F32;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      ফেরত PPC_LD_F64;
#যদি FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      ফেরত PPC_LD_F128;
#endif
    ক্ষেত্র FFI_TYPE_UINT8:
      ফেরত PPC_LD_U8;
    ক্ষেত্র FFI_TYPE_SINT8:
      ফেরত PPC_LD_S8;
    ক্ষেত্র FFI_TYPE_UINT16:
      ফেরত PPC_LD_U16;
    ক্ষেত্র FFI_TYPE_SINT16:
      ফেরত PPC_LD_S16;
    ক্ষেত্র FFI_TYPE_UINT32:
      ফেরত PPC_LD_U32;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
      ফেরত PPC_LD_S32;
    ক্ষেত্র FFI_TYPE_POINTER:
      ফেরত PPC_LD_PTR;
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
      ফেরত PPC_LD_I64;
    ক্ষেত্র FFI_TYPE_UINT128:
      ফেরত PPC32_LD_R3R6;
    ক্ষেত্র FFI_TYPE_STRUCT:
      যদি (cif->abi & FFI_SYSV_STRUCT_RET)
	নির্বাচন (আকার_2)
	  {
	  ক্ষেত্র 1:
	    ফেরত PPC_LD_U8;
	  ক্ষেত্র 2:
	    ফেরত PPC_LD_U16;
	  ক্ষেত্র 3:
	    ফেরত PPC32_SYSV_LD_STRUCT_3;
	  ক্ষেত্র 4:
	    ফেরত PPC_LD_U32;
	  ক্ষেত্র 5:
	    ফেরত PPC32_SYSV_LD_STRUCT_5;
	  ক্ষেত্র 6:
	    ফেরত PPC32_SYSV_LD_STRUCT_6;
	  ক্ষেত্র 7:
	    ফেরত PPC32_SYSV_LD_STRUCT_7;
	  ক্ষেত্র 8:
	    ফেরত PPC_LD_I64;
	  }
      ফেরত PPC_LD_NONE;
    পূর্বনির্ধারিত:
      abort();
    }
}
#endif

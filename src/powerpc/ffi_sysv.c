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

#ifndef POWERPC64
#अन्तर्भाव "ffi_common.h"
#अन्तर्भाव "ffi_powerpc.h"


/* About the SYSV ABI.  */
#define ASM_NEEDS_REGISTERS 6
#define अङ्कः_GPR_ARG_REGISTERS 8
#define अङ्कः_FPR_ARG_REGISTERS 8


#जइ HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  जइ ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  अण्णहा
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
स्थिर पूर्णाङ्क
translate_float (पूर्णाङ्क abi, पूर्णाङ्क type)
{
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  जइ (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  जइ ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      जइ (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      अण्णहा जइ (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      अण्णहा जइ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  अण्णहा जइ ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      जइ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  प्रत्यागम type;
}

/* Perform machine dependent cif processing */
स्थिर ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  unsigned bytes;
  unsigned i, fpr_गणनम् = 0, gpr_गणनम् = 0, राशिः_गणनम् = 0;
  unsigned flags = cif->flags;
  unsigned struct_copy_size = 0;
  unsigned type = cif->rtype->type;
  unsigned size = cif->rtype->size;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * परिमाण (पूर्णाङ्क);

  /* Space for the GPR registers.  */
  bytes += अङ्कः_GPR_ARG_REGISTERS * परिमाण (पूर्णाङ्क);

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
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    पक्ष FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    पक्ष FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      प्रत्यागम FFI_BAD_ABI;
#endif
      भङ्ग;

    पक्ष FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      जइ ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8)
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
      unsigned short typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      विकल्प (typenum)
	{
#जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
	  जइ (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_गणनम् = अङ्कः_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 4;
	    }
	  अण्णहा
	    fpr_गणनम् += 2;
#ifdef __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#endif
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_DOUBLE:
	  जइ (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 2;
	    }
	  अण्णहा
	    fpr_गणनम् += 1;
#ifdef __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  जइ (fpr_गणनम् >= अङ्कः_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    राशिः_गणनम् += 1;
	  अण्णहा
	    fpr_गणनम् += 1;
#ifdef __NO_FPRS__
	  प्रत्यागम FFI_BAD_ABI;
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  जइ (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS - 3)
	    gpr_गणनम् = अङ्कः_GPR_ARG_REGISTERS;
	  जइ (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    राशिः_गणनम् += 4;
	  अण्णहा
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
	  जइ (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    {
	      राशिः_गणनम् += राशिः_गणनम् & 1;
	      राशिः_गणनम् += 2;
	    }
	  अण्णहा
	    gpr_गणनम् += 2;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_size += ((*ptr)->size + 15) & ~0xF;
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
	  जइ (gpr_गणनम् >= अङ्कः_GPR_ARG_REGISTERS)
	    राशिः_गणनम् += 1;
	  अण्णहा
	    gpr_गणनम् += 1;
	  भङ्ग;

	स्वभाव:
	  FFI_ASSERT (0);
	}
    }

  जइ (fpr_गणनम् != 0)
    flags |= FLAG_FP_ARGUMENTS;
  जइ (gpr_गणनम् > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  जइ (struct_copy_size != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  जइ (fpr_गणनम् != 0)
    bytes += अङ्कः_FPR_ARG_REGISTERS * परिमाण (द्विगुण);

  /* Stack space.  */
  bytes += राशिः_गणनम् * परिमाण (पूर्णाङ्क);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_size;

  cif->flags = flags;
  cif->bytes = bytes;

  प्रत्यागम FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  जइ ((cif->abi & FFI_SYSV) == 0)
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
ffi_prep_args_SYSV (extended_cif *ecif, unsigned *ध्रुव राशिः)
{
  ध्रुव unsigned bytes = ecif->cif->bytes;
  ध्रुव unsigned flags = ecif->cif->flags;

  प्रकारनिर्देश सङ्घ
  {
    वर्ण *c;
    unsigned *u;
    long long *ll;
    भिन्न *f;
    द्विगुण *d;
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

  पूर्णाङ्क i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  द्विगुण double_tmp;
#endif
  सङ्घ
  {
    शून्य **v;
    वर्ण **c;
    signed वर्ण **sc;
    unsigned वर्ण **uc;
    signed short **ss;
    unsigned short **us;
    unsigned पूर्णाङ्क **ui;
    long long **ll;
    भिन्न **f;
    द्विगुण **d;
  } p_argv;
  size_t struct_copy_size;
  unsigned gprvalue;

  stacktop.c = (वर्ण *) राशिः + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - अङ्कः_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - अङ्कः_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#अण्णहा
  copy_space.c = gpr_base.c;
#endif
  next_arg.u = राशिः + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((unsigned long) (वर्ण *) राशिः & 0xF) == 0);
  FFI_ASSERT (((unsigned long) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((unsigned long) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= next_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  जइ (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (unsigned) (वर्ण *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  प्रति (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      unsigned पूर्णाङ्क typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      विकल्प (typenum)
	{
#ifndef __NO_FPRS__
# जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  जइ (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      जइ (((next_arg.u - राशिः) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  अण्णहा
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;
# endif
	पक्ष FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  जइ (fpr_base.d >= fpr_end.d)
	    {
	      जइ (((next_arg.u - राशिः) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  अण्णहा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  जइ (fpr_base.d >= fpr_end.d)
	    {
	      *next_arg.f = (भिन्न) double_tmp;
	      next_arg.u += 1;
	    }
	  अण्णहा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;
#endif /* have FPRs */

	पक्ष FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  जइ (gpr_base.u >= gpr_end.u - 3)
	    {
	      unsigned पूर्णाङ्क ii;
	      gpr_base.u = gpr_end.u;
	      प्रति (ii = 0; ii < 4; ii++)
		{
		  unsigned पूर्णाङ्क int_tmp = (*p_argv.ui)[ii];
		  *next_arg.u++ = int_tmp;
		}
	    }
	  अण्णहा
	    {
	      unsigned पूर्णाङ्क ii;
	      प्रति (ii = 0; ii < 4; ii++)
		{
		  unsigned पूर्णाङ्क int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	  जइ (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      जइ (((next_arg.u - राशिः) & 1) != 0)
		next_arg.u++;
	      *next_arg.ll = **p_argv.ll;
	      next_arg.u += 2;
	    }
	  अण्णहा
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      जइ (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  struct_copy_size = ((*ptr)->size + 15) & ~0xF;
	  copy_space.c -= struct_copy_size;
	  memcpy (copy_space.c, *p_argv.c, (*ptr)->size);

	  gprvalue = (unsigned long) copy_space.c;

	  FFI_ASSERT (copy_space.c > next_arg.c);
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
	  जइ (gpr_base.u >= gpr_end.u)
	    *next_arg.u++ = gprvalue;
	  अण्णहा
	    *gpr_base.u++ = gprvalue;
	  भङ्ग;
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

#define MIN_CACHE_पङ्क्तिः_SIZE 8

स्थिर शून्य
flush_icache (वर्ण *wraddr, वर्ण *xaddr, पूर्णाङ्क size)
{
  पूर्णाङ्क i;
  प्रति (i = 0; i < size; i += MIN_CACHE_पङ्क्तिः_SIZE)
    __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ चञ्चल ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
		    : : "r"(xaddr + size - 1), "r"(wraddr + size - 1)
		    : "memory");
}

ffi_status FFI_HIDDEN
ffi_prep_closure_loc_sysv (ffi_closure *closure,
			   ffi_cif *cif,
			   शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			   शून्य *user_data,
			   शून्य *codeloc)
{
  जइ (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    प्रत्यागम FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  जइ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अण्णहा
#endif
    {
      unsigned पूर्णाङ्क *tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
      tramp[0] = 0x7c0802a6;  /*   mflr    r0 */
      tramp[1] = 0x429f0005;  /*   bcl     20,31,.+4 */
      tramp[2] = 0x7d6802a6;  /*   mflr    r11 */
      tramp[3] = 0x7c0803a6;  /*   mtlr    r0 */
      tramp[4] = 0x800b0018;  /*   lwz     r0,24(r11) */
      tramp[5] = 0x816b001c;  /*   lwz     r11,28(r11) */
      tramp[6] = 0x7c0903a6;  /*   mtctr   r0 */
      tramp[7] = 0x4e800420;  /*   bctr */
      *(शून्य **) &tramp[8] = (शून्य *) ffi_closure_SYSV; /* function */
      *(शून्य **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((वर्ण *)tramp, (वर्ण *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  प्रत्यागम FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

पूर्णाङ्क
ffi_closure_helper_SYSV (ffi_cif *cif,
			 शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			 शून्य *user_data,
			 शून्य *rvalue,
			 unsigned long *pgr,
			 ffi_dblfl *pfr,
			 unsigned long *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  शून्य **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
#ifndef __NO_FPRS__
  long             nf = 0;   /* number of floating registers already used */
#endif
  long             ng = 0;   /* number of general registers already used */

  unsigned       size     = cif->rtype->size;
  unsigned short rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * परिमाण (शून्य *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  जइ (rtypenum == FFI_TYPE_STRUCT
      && !((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8))
    {
      rvalue = (शून्य *) *pgr;
      ng++;
      pgr++;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  जाव (i < avn) {
    unsigned short typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    विकल्प (typenum)
      {
#ifndef __NO_FPRS__
      पक्ष FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	जइ (nf < अङ्कः_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    द्विगुण temp = pfr->d;
	    pfr->f = (भिन्न) temp;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अण्णहा
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_DOUBLE:
	जइ (nf < अङ्कः_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अण्णहा
	  {
	    जइ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	भङ्ग;

# जइ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      पक्ष FFI_TYPE_LONGDOUBLE:
	जइ (nf < अङ्कः_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	अण्णहा
	  {
	    जइ (((long) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	भङ्ग;
# endif
#endif

      पक्ष FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS - 3)
	  {
	    avalue[i] = pgr;
	    pgr += 4;
	    ng += 4;
	  }
	अण्णहा
	  {
	    avalue[i] = pst;
	    pst += 4;
	    ng = 8+4;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_SINT8:
      पक्ष FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (वर्ण *) pgr + 3;
	    ng++;
	    pgr++;
	  }
	अण्णहा
	  {
	    avalue[i] = (वर्ण *) pst + 3;
	    pst++;
	  }
	भङ्ग;
#endif

      पक्ष FFI_TYPE_SINT16:
      पक्ष FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (वर्ण *) pgr + 2;
	    ng++;
	    pgr++;
	  }
	अण्णहा
	  {
	    avalue[i] = (वर्ण *) pst + 2;
	    pst++;
	  }
	भङ्ग;
#endif

      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_POINTER:
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = pgr;
	    ng++;
	    pgr++;
	  }
	अण्णहा
	  {
	    avalue[i] = pst;
	    pst++;
	  }
	भङ्ग;

      पक्ष FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS)
	  {
	    avalue[i] = (शून्य *) *pgr;
	    ng++;
	    pgr++;
	  }
	अण्णहा
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
	जइ (ng < अङ्कः_GPR_ARG_REGISTERS - 1)
	  {
	    जइ (ng & 1)
	      {
		/* skip r4, r6, r8 as starting points */
		ng++;
		pgr++;
	      }
	    avalue[i] = pgr;
	    ng += 2;
	    pgr += 2;
	  }
	अण्णहा
	  {
	    जइ (((long) pst) & 4)
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

  (*fun) (cif, rvalue, avalue, user_data);

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
#जइ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      प्रत्यागम PPC_LD_F128;
#endif
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
      जइ (cif->abi & FFI_SYSV_STRUCT_RET)
	विकल्प (size)
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
#endif

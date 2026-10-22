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
#define अंक_GPR_ARG_REGISTERS 8
#define अंक_FPR_ARG_REGISTERS 8


#जर HAVE_LONG_DOUBLE_VARIANT && FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
/* Adjust size of ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types_sysv (ffi_abi abi)
{
  जर ((abi & (FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128)) == FFI_SYSV)
    {
      ffi_type_longdouble.size = 8;
      ffi_type_longdouble.alignment = 8;
    }
  अन्यथा
    {
      ffi_type_longdouble.size = 16;
      ffi_type_longdouble.alignment = 16;
    }
}
#endif

/* Transform long double, double and float to other types as per abi.  */
स्थिर पूर्णांक
translate_float (पूर्णांक abi, पूर्णांक type)
{
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
  जर (type == FFI_TYPE_LONGDOUBLE
      && (abi & FFI_SYSV_LONG_DOUBLE_128) == 0)
    type = FFI_TYPE_DOUBLE;
#endif
  जर ((abi & FFI_SYSV_SOFT_FLOAT) != 0)
    {
      जर (type == FFI_TYPE_FLOAT)
	type = FFI_TYPE_UINT32;
      अन्यथा जर (type == FFI_TYPE_DOUBLE)
	type = FFI_TYPE_UINT64;
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      अन्यथा जर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_UINT128;
    }
  अन्यथा जर ((abi & FFI_SYSV_IBM_LONG_DOUBLE) == 0)
    {
      जर (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif
    }
  परत type;
}

/* Perform machine dependent cif processing */
स्थिर ffi_status
ffi_prep_cif_sysv_core (ffi_cif *cif)
{
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित i, fpr_मोजणी = 0, gpr_मोजणी = 0, चळत_मोजणी = 0;
  अचिन्हित flags = cif->flags;
  अचिन्हित struct_copy_size = 0;
  अचिन्हित type = cif->rtype->type;
  अचिन्हित size = cif->rtype->size;

  /* The machine-independent calculation of cif->bytes doesn't work
     for us.  Redo the calculation.  */

  /* Space for the frame pointer, callee's LR, and the asm's temp regs.  */
  bytes = (2 + ASM_NEEDS_REGISTERS) * आकार (पूर्णांक);

  /* Space for the GPR registers.  */
  bytes += अंक_GPR_ARG_REGISTERS * आकार (पूर्णांक);

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

  निवड (type)
    {
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
#endif
    स्थिती FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    स्थिती FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
#ifdef __NO_FPRS__
      परत FFI_BAD_ABI;
#endif
      थांबा;

    स्थिती FFI_TYPE_UINT128:
      flags |= FLAG_RETURNS_128BITS;
      /* Fall through.  */
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      flags |= FLAG_RETURNS_64BITS;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      /* The final SYSV ABI says that structures smaller or equal 8 bytes
	 are returned in r3/r4.  A draft ABI used by linux instead
	 returns them in memory.  */
      जर ((cif->abi & FFI_SYSV_STRUCT_RET) != 0 && size <= 8)
	{
	  flags |= FLAG_RETURNS_SMST;
	  थांबा;
	}
      gpr_मोजणी++;
      flags |= FLAG_RETVAL_REFERENCE;
      /* Fall through.  */
    स्थिती FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      थांबा;

    पूर्वनिर्धारित:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      थांबा;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  Structures and long doubles (if not equivalent
     to double) are passed as a pointer to a copy of the structure.
     Stuff on the stack needs to keep proper alignment.  */
  साठी (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित लघु typenum = (*ptr)->type;

      typenum = translate_float (cif->abi, typenum);

      निवड (typenum)
	{
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
	  जर (fpr_मोजणी >= अंक_FPR_ARG_REGISTERS - 1)
	    {
	      fpr_मोजणी = अंक_FPR_ARG_REGISTERS;
	      /* 8-byte align long doubles.  */
	      चळत_मोजणी += चळत_मोजणी & 1;
	      चळत_मोजणी += 4;
	    }
	  अन्यथा
	    fpr_मोजणी += 2;
#ifdef __NO_FPRS__
	  परत FFI_BAD_ABI;
#endif
	  थांबा;
#endif

	स्थिती FFI_TYPE_DOUBLE:
	  जर (fpr_मोजणी >= अंक_FPR_ARG_REGISTERS)
	    {
	      /* 8-byte align doubles.  */
	      चळत_मोजणी += चळत_मोजणी & 1;
	      चळत_मोजणी += 2;
	    }
	  अन्यथा
	    fpr_मोजणी += 1;
#ifdef __NO_FPRS__
	  परत FFI_BAD_ABI;
#endif
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  जर (fpr_मोजणी >= अंक_FPR_ARG_REGISTERS)
	    /* Yes, we don't follow the ABI, but neither does gcc.  */
	    चळत_मोजणी += 1;
	  अन्यथा
	    fpr_मोजणी += 1;
#ifdef __NO_FPRS__
	  परत FFI_BAD_ABI;
#endif
	  थांबा;

	स्थिती FFI_TYPE_UINT128:
	  /* A long double in FFI_LINUX_SOFT_FLOAT can use only a set
	     of four consecutive gprs. If we do not have enough, we
	     have to adjust the gpr_count value.  */
	  जर (gpr_मोजणी >= अंक_GPR_ARG_REGISTERS - 3)
	    gpr_मोजणी = अंक_GPR_ARG_REGISTERS;
	  जर (gpr_मोजणी >= अंक_GPR_ARG_REGISTERS)
	    चळत_मोजणी += 4;
	  अन्यथा
	    gpr_मोजणी += 4;
	  थांबा;

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.

	     Also, only certain register pairs can be used for
	     passing long long int -- specifically (r3,r4), (r5,r6),
	     (r7,r8), (r9,r10).  */
	  gpr_मोजणी += gpr_मोजणी & 1;
	  जर (gpr_मोजणी >= अंक_GPR_ARG_REGISTERS)
	    {
	      चळत_मोजणी += चळत_मोजणी & 1;
	      चळत_मोजणी += 2;
	    }
	  अन्यथा
	    gpr_मोजणी += 2;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  /* We must allocate space for a copy of these to enforce
	     pass-by-value.  Pad the space up to a multiple of 16
	     bytes (the maximum alignment required for anything under
	     the SYSV ABI).  */
	  struct_copy_size += ((*ptr)->size + 15) & ~0xF;
	  /* Fall through (allocate space for the pointer).  */

	स्थिती FFI_TYPE_POINTER:
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT8:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  जर (gpr_मोजणी >= अंक_GPR_ARG_REGISTERS)
	    चळत_मोजणी += 1;
	  अन्यथा
	    gpr_मोजणी += 1;
	  थांबा;

	पूर्वनिर्धारित:
	  FFI_ASSERT (0);
	}
    }

  जर (fpr_मोजणी != 0)
    flags |= FLAG_FP_ARGUMENTS;
  जर (gpr_मोजणी > 4)
    flags |= FLAG_4_GPR_ARGUMENTS;
  जर (struct_copy_size != 0)
    flags |= FLAG_ARG_NEEDS_COPY;

  /* Space for the FPR registers, if needed.  */
  जर (fpr_मोजणी != 0)
    bytes += अंक_FPR_ARG_REGISTERS * आकार (द्विगुण);

  /* Stack space.  */
  bytes += चळत_मोजणी * आकार (पूर्णांक);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = (bytes + 15) & ~0xF;

  /* Add in the space for the copied structures.  */
  bytes += struct_copy_size;

  cif->flags = flags;
  cif->bytes = bytes;

  परत FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_sysv (ffi_cif *cif)
{
  जर ((cif->abi & FFI_SYSV) == 0)
    {
      /* This call is from old code.  Translate to new ABI values.  */
      cif->flags |= FLAG_COMPAT;
      निवड (cif->abi)
	{
	पूर्वनिर्धारित:
	  परत FFI_BAD_ABI;

	स्थिती FFI_COMPAT_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_STRUCT_RET | FFI_SYSV_LONG_DOUBLE_128;
	  थांबा;

	स्थिती FFI_COMPAT_GCC_SYSV:
	  cif->abi = FFI_SYSV | FFI_SYSV_LONG_DOUBLE_128;
	  थांबा;

	स्थिती FFI_COMPAT_LINUX:
	  cif->abi = (FFI_SYSV | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  थांबा;

	स्थिती FFI_COMPAT_LINUX_SOFT_FLOAT:
	  cif->abi = (FFI_SYSV | FFI_SYSV_SOFT_FLOAT | FFI_SYSV_IBM_LONG_DOUBLE
		      | FFI_SYSV_LONG_DOUBLE_128);
	  थांबा;
	}
    }
  परत ffi_prep_cif_sysv_core (cif);
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
ffi_prep_args_SYSV (extended_cif *ecif, अचिन्हित *स्थिरांक चळत)
{
  स्थिरांक अचिन्हित bytes = ecif->cif->bytes;
  स्थिरांक अचिन्हित flags = ecif->cif->flags;

  प्रकारव्याख्या संघ
  {
    अक्षर *c;
    अचिन्हित *u;
    दीर्घ दीर्घ *ll;
    दशांश *f;
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

  पूर्णांक i;
  ffi_type **ptr;
#ifndef __NO_FPRS__
  द्विगुण double_tmp;
#endif
  संघ
  {
    शून्य **v;
    अक्षर **c;
    चिन्हित अक्षर **sc;
    अचिन्हित अक्षर **uc;
    चिन्हित लघु **ss;
    अचिन्हित लघु **us;
    अचिन्हित पूर्णांक **ui;
    दीर्घ दीर्घ **ll;
    दशांश **f;
    द्विगुण **d;
  } p_argv;
  माप_प्रकार struct_copy_size;
  अचिन्हित gprvalue;

  stacktop.c = (अक्षर *) चळत + bytes;
  gpr_end.u = stacktop.u - ASM_NEEDS_REGISTERS;
  gpr_base.u = gpr_end.u - अंक_GPR_ARG_REGISTERS;
#ifndef __NO_FPRS__
  fpr_end.d = gpr_base.d;
  fpr_base.d = fpr_end.d - अंक_FPR_ARG_REGISTERS;
  copy_space.c = ((flags & FLAG_FP_ARGUMENTS) ? fpr_base.c : gpr_base.c);
#अन्यथा
  copy_space.c = gpr_base.c;
#endif
  next_arg.u = चळत + 2;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT (((अचिन्हित दीर्घ) (अक्षर *) चळत & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) copy_space.c & 0xF) == 0);
  FFI_ASSERT (((अचिन्हित दीर्घ) stacktop.c & 0xF) == 0);
  FFI_ASSERT ((bytes & 0xF) == 0);
  FFI_ASSERT (copy_space.c >= next_arg.c);

  /* Deal with return values that are actually pass-by-reference.  */
  जर (flags & FLAG_RETVAL_REFERENCE)
    *gpr_base.u++ = (अचिन्हित) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  p_argv.v = ecif->avalue;
  साठी (ptr = ecif->cif->arg_types, i = ecif->cif->nargs;
       i > 0;
       i--, ptr++, p_argv.v++)
    {
      अचिन्हित पूर्णांक typenum = (*ptr)->type;

      typenum = translate_float (ecif->cif->abi, typenum);

      /* Now test the translated value */
      निवड (typenum)
	{
#ifndef __NO_FPRS__
# जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
	  double_tmp = (*p_argv.d)[0];

	  जर (fpr_base.d >= fpr_end.d - 1)
	    {
	      fpr_base.d = fpr_end.d;
	      जर (((next_arg.u - चळत) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	      double_tmp = (*p_argv.d)[1];
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  अन्यथा
	    {
	      *fpr_base.d++ = double_tmp;
	      double_tmp = (*p_argv.d)[1];
	      *fpr_base.d++ = double_tmp;
	    }
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  थांबा;
# endif
	स्थिती FFI_TYPE_DOUBLE:
	  double_tmp = **p_argv.d;

	  जर (fpr_base.d >= fpr_end.d)
	    {
	      जर (((next_arg.u - चळत) & 1) != 0)
		next_arg.u += 1;
	      *next_arg.d = double_tmp;
	      next_arg.u += 2;
	    }
	  अन्यथा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  double_tmp = **p_argv.f;
	  जर (fpr_base.d >= fpr_end.d)
	    {
	      *next_arg.f = (दशांश) double_tmp;
	      next_arg.u += 1;
	    }
	  अन्यथा
	    *fpr_base.d++ = double_tmp;
	  FFI_ASSERT (flags & FLAG_FP_ARGUMENTS);
	  थांबा;
#endif /* have FPRs */

	स्थिती FFI_TYPE_UINT128:
	  /* The soft float ABI for long doubles works like this, a long double
	     is passed in four consecutive GPRs if available.  A maximum of 2
	     long doubles can be passed in gprs.  If we do not have 4 GPRs
	     left, the long double is passed on the stack, 4-byte aligned.  */
	  जर (gpr_base.u >= gpr_end.u - 3)
	    {
	      अचिन्हित पूर्णांक ii;
	      gpr_base.u = gpr_end.u;
	      साठी (ii = 0; ii < 4; ii++)
		{
		  अचिन्हित पूर्णांक int_tmp = (*p_argv.ui)[ii];
		  *next_arg.u++ = int_tmp;
		}
	    }
	  अन्यथा
	    {
	      अचिन्हित पूर्णांक ii;
	      साठी (ii = 0; ii < 4; ii++)
		{
		  अचिन्हित पूर्णांक int_tmp = (*p_argv.ui)[ii];
		  *gpr_base.u++ = int_tmp;
		}
	    }
	  थांबा;

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  जर (gpr_base.u >= gpr_end.u - 1)
	    {
	      gpr_base.u = gpr_end.u;
	      जर (((next_arg.u - चळत) & 1) != 0)
		next_arg.u++;
	      *next_arg.ll = **p_argv.ll;
	      next_arg.u += 2;
	    }
	  अन्यथा
	    {
	      /* The abi states only certain register pairs can be
		 used for passing long long int specifically (r3,r4),
		 (r5,r6), (r7,r8), (r9,r10).  If next arg is long long
		 but not correct starting register of pair then skip
		 until the proper starting register.  */
	      जर (((gpr_end.u - gpr_base.u) & 1) != 0)
		gpr_base.u++;
	      *gpr_base.ll++ = **p_argv.ll;
	    }
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  struct_copy_size = ((*ptr)->size + 15) & ~0xF;
	  copy_space.c -= struct_copy_size;
	  स_नकल (copy_space.c, *p_argv.c, (*ptr)->size);

	  gprvalue = (अचिन्हित दीर्घ) copy_space.c;

	  FFI_ASSERT (copy_space.c > next_arg.c);
	  FFI_ASSERT (flags & FLAG_ARG_NEEDS_COPY);
	  जा putgpr;

	स्थिती FFI_TYPE_UINT8:
	  gprvalue = **p_argv.uc;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT8:
	  gprvalue = **p_argv.sc;
	  जा putgpr;
	स्थिती FFI_TYPE_UINT16:
	  gprvalue = **p_argv.us;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT16:
	  gprvalue = **p_argv.ss;
	  जा putgpr;

	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_POINTER:

	  gprvalue = **p_argv.ui;

	putgpr:
	  जर (gpr_base.u >= gpr_end.u)
	    *next_arg.u++ = gprvalue;
	  अन्यथा
	    *gpr_base.u++ = gprvalue;
	  थांबा;
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

#define MIN_CACHE_ओळ_SIZE 8

स्थिर शून्य
flush_icache (अक्षर *wraddr, अक्षर *xaddr, पूर्णांक size)
{
  पूर्णांक i;
  साठी (i = 0; i < size; i += MIN_CACHE_ओळ_SIZE)
    __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;"
		      : : "r" (xaddr + i), "r" (wraddr + i) : "memory");
  __asm__ अस्थिर ("icbi 0,%0;" "dcbf 0,%1;" "sync;" "isync;"
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
  जर (cif->abi < FFI_SYSV || cif->abi >= FFI_LAST_ABI)
    परत FFI_BAD_ABI;

#ifdef FFI_EXEC_STATIC_TRAMP
  जर (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      शून्य (*dest)(शून्य) = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
    }
  अन्यथा
#endif
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
      *(शून्य **) &tramp[8] = (शून्य *) ffi_closure_SYSV; /* function */
      *(शून्य **) &tramp[9] = codeloc;			/* context */

      /* Flush the icache.  */
      flush_icache ((अक्षर *)tramp, (अक्षर *)codeloc, 8 * 4);
    }

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

/* Basically the trampoline invokes ffi_closure_SYSV, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_SYSV invokes the
   following helper function to do most of the work.  */

पूर्णांक
ffi_closure_helper_SYSV (ffi_cif *cif,
			 शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
			 शून्य *user_data,
			 शून्य *rvalue,
			 अचिन्हित दीर्घ *pgr,
			 ffi_dblfl *pfr,
			 अचिन्हित दीर्घ *pst)
{
  /* rvalue is the pointer to space for return value in closure assembly */
  /* pgr is the pointer to where r3-r10 are stored in ffi_closure_SYSV */
  /* pfr is the pointer to where f1-f8 are stored in ffi_closure_SYSV  */
  /* pst is the pointer to outgoing parameter stack in original caller */

  शून्य **          avalue;
  ffi_type **      arg_types;
  दीर्घ             i, avn;
#ifndef __NO_FPRS__
  दीर्घ             nf = 0;   /* number of floating registers already used */
#endif
  दीर्घ             ng = 0;   /* number of general registers already used */

  अचिन्हित       size     = cif->rtype->size;
  अचिन्हित लघु rtypenum = cif->rtype->type;

  avalue = alloca (cif->nargs * आकार (शून्य *));

  /* First translate for softfloat/nonlinux */
  rtypenum = translate_float (cif->abi, rtypenum);

  /* Copy the caller's structure return value address so that the closure
     returns the data directly to the caller.
     For FFI_SYSV the result is passed in r3/r4 if the struct size is less
     or equal 8 bytes.  */
  जर (rtypenum == FFI_TYPE_STRUCT
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
  पर्यंत (i < avn) {
    अचिन्हित लघु typenum = arg_types[i]->type;

    /* We may need to handle some values depending on ABI.  */
    typenum = translate_float (cif->abi, typenum);

    निवड (typenum)
      {
#ifndef __NO_FPRS__
      स्थिती FFI_TYPE_FLOAT:
	/* Unfortunately float values are stored as doubles
	   in the ffi_closure_SYSV code (since we don't check
	   the type in that routine).  */
	जर (nf < अंक_FPR_ARG_REGISTERS)
	  {
	    /* FIXME? here we are really changing the values
	       stored in the original calling routines outgoing
	       parameter stack.  This is probably a really
	       naughty thing to do but...  */
	    द्विगुण temp = pfr->d;
	    pfr->f = (दशांश) temp;
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    avalue[i] = pst;
	    pst += 1;
	  }
	थांबा;

      स्थिती FFI_TYPE_DOUBLE:
	जर (nf < अंक_FPR_ARG_REGISTERS)
	  {
	    avalue[i] = pfr;
	    nf++;
	    pfr++;
	  }
	अन्यथा
	  {
	    जर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	  }
	थांबा;

# जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      स्थिती FFI_TYPE_LONGDOUBLE:
	जर (nf < अंक_FPR_ARG_REGISTERS - 1)
	  {
	    avalue[i] = pfr;
	    pfr += 2;
	    nf += 2;
	  }
	अन्यथा
	  {
	    जर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 4;
	    nf = 8;
	  }
	थांबा;
# endif
#endif

      स्थिती FFI_TYPE_UINT128:
	/* Test if for the whole long double, 4 gprs are available.
	   otherwise the stuff ends up on the stack.  */
	जर (ng < अंक_GPR_ARG_REGISTERS - 3)
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
	थांबा;

      स्थिती FFI_TYPE_SINT8:
      स्थिती FFI_TYPE_UINT8:
#ifndef __LITTLE_ENDIAN__
	जर (ng < अंक_GPR_ARG_REGISTERS)
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
	थांबा;
#endif

      स्थिती FFI_TYPE_SINT16:
      स्थिती FFI_TYPE_UINT16:
#ifndef __LITTLE_ENDIAN__
	जर (ng < अंक_GPR_ARG_REGISTERS)
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
	थांबा;
#endif

      स्थिती FFI_TYPE_SINT32:
      स्थिती FFI_TYPE_UINT32:
      स्थिती FFI_TYPE_POINTER:
	जर (ng < अंक_GPR_ARG_REGISTERS)
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
	थांबा;

      स्थिती FFI_TYPE_STRUCT:
	/* Structs are passed by reference. The address will appear in a
	   gpr if it is one of the first 8 arguments.  */
	जर (ng < अंक_GPR_ARG_REGISTERS)
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
	थांबा;

      स्थिती FFI_TYPE_SINT64:
      स्थिती FFI_TYPE_UINT64:
	/* Passing long long ints are complex, they must
	   be passed in suitable register pairs such as
	   (r3,r4) or (r5,r6) or (r6,r7), or (r7,r8) or (r9,r10)
	   and if the entire pair aren't available then the outgoing
	   parameter stack is used for both but an alignment of 8
	   must will be kept.  So we must either look in pgr
	   or pst to find the correct address for this type
	   of parameter.  */
	जर (ng < अंक_GPR_ARG_REGISTERS - 1)
	  {
	    जर (ng & 1)
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
	    जर (((दीर्घ) pst) & 4)
	      pst++;
	    avalue[i] = pst;
	    pst += 2;
	    ng = अंक_GPR_ARG_REGISTERS;
	  }
	थांबा;

      पूर्वनिर्धारित:
	FFI_ASSERT (0);
      }

    i++;
  }

  (*fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_SYSV how to perform return type promotions.
     Because the FFI_SYSV ABI returns the structures <= 8 bytes in
     r3/r4 we have to tell ffi_closure_SYSV how to treat them. */
  निवड (rtypenum)
    {
    स्थिती FFI_TYPE_VOID:
      परत PPC_LD_NONE;
    स्थिती FFI_TYPE_FLOAT:
      परत PPC_LD_F32;
    स्थिती FFI_TYPE_DOUBLE:
      परत PPC_LD_F64;
#जर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      परत PPC_LD_F128;
#endif
    स्थिती FFI_TYPE_UINT8:
      परत PPC_LD_U8;
    स्थिती FFI_TYPE_SINT8:
      परत PPC_LD_S8;
    स्थिती FFI_TYPE_UINT16:
      परत PPC_LD_U16;
    स्थिती FFI_TYPE_SINT16:
      परत PPC_LD_S16;
    स्थिती FFI_TYPE_UINT32:
      परत PPC_LD_U32;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      परत PPC_LD_S32;
    स्थिती FFI_TYPE_POINTER:
      परत PPC_LD_PTR;
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      परत PPC_LD_I64;
    स्थिती FFI_TYPE_UINT128:
      परत PPC32_LD_R3R6;
    स्थिती FFI_TYPE_STRUCT:
      जर (cif->abi & FFI_SYSV_STRUCT_RET)
	निवड (size)
	  {
	  स्थिती 1:
	    परत PPC_LD_U8;
	  स्थिती 2:
	    परत PPC_LD_U16;
	  स्थिती 3:
	    परत PPC32_SYSV_LD_STRUCT_3;
	  स्थिती 4:
	    परत PPC_LD_U32;
	  स्थिती 5:
	    परत PPC32_SYSV_LD_STRUCT_5;
	  स्थिती 6:
	    परत PPC32_SYSV_LD_STRUCT_6;
	  स्थिती 7:
	    परत PPC32_SYSV_LD_STRUCT_7;
	  स्थिती 8:
	    परत PPC_LD_I64;
	  }
      परत PPC_LD_NONE;
    पूर्वनिर्धारित:
      पात();
    }
}
#endif

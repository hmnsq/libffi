/* -----------------------------------------------------------------------
   ffi_darwin.c

   Copyright (C) 1998 Geoffrey Keating
   Copyright (C) 2001 John Hornkvist
   Copyright (C) 2002, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
   Copyright (C) 2026 Anthony Green

   FFI support for Darwin and AIX.
   
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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>


ସଂରଚନା ffi_aix_trampoline_struct {
    ଶୂନ୍ୟ * code_pointer;	/* Pointer to ffi_closure_ASM */
    ଶୂନ୍ୟ * toc;			/* TOC */
    ଶୂନ୍ୟ * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#define PPC_LD_NONE		0
#define PPC_LD_R3		1
#define PPC_LD_R3R4		2
#define PPC_LD_F32		3
#define PPC_LD_F64		4
#define PPC_LD_F128		5
#define PPC_LD_U8		6
#define PPC_LD_S8		7
#define PPC_LD_U16		8
#define PPC_LD_S16		9

#ifndef POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#ଅନ୍ୟଥା
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_ASM (ଶୂନ୍ୟ);

#ଯଦି defined (FFI_GO_CLOSURES)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_ASM (ଶୂନ୍ୟ);
#endif

ଗଣନା {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128BITS	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64BITS	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
ଗଣନା {
  ଅଙ୍କ_GPR_ARG_REGISTERS = 8,
  ଅଙ୍କ_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

ଗଣନା { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments.
   
   m32/m64

   The stack layout we want looks like this:

   |   Return address from ffi_call_DARWIN      |	higher addresses
   |--------------------------------------------|
   |   Previous backchain pointer	4/8	|	stack pointer here
   |--------------------------------------------|<+ <<<	on entry to
   |   ASM_NEEDS_REGISTERS=r28-r31   4*(4/8)	| |	ffi_call_DARWIN
   |--------------------------------------------| |
   |   When we have any FP activity... the	| |
   |   FPRs occupy NUM_FPR_ARG_REGISTERS slots	| |
   |   here fp13 .. fp1 from high to low addr.	| |
   ~						~ ~
   |   Parameters      (at least 8*4/8=32/64)	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |	stack	|
   |   Reserved                       2*4/8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |	lower addresses
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |     stack pointer here
   |   Current backchain pointer	4/8	|-/	during
   |--------------------------------------------|   <<<	ffi_call_DARWIN

   */

#ଯଦି defined(POWERPC_DARWIN64)
ସ୍ଥିର ଶୂନ୍ୟ
darwin64_pass_struct_by_ମୂଲ୍ୟ 
  (ffi_type *, ଅକ୍ଷର *, unsigned, unsigned *, ଦ୍ୱିଗୁଣ **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

ଶୂନ୍ୟ
ffi_prep_args (extended_cif *ecif, unsigned long *ଅଚଳ ଗଦା)
{
  ଅଚଳ unsigned bytes = ecif->cif->bytes;
  ଅଚଳ unsigned flags = ecif->cif->flags;
  ଅଚଳ unsigned nargs = ecif->cif->nargs;
#ଯଦି !defined(POWERPC_DARWIN64) 
  ଅଚଳ ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *ଅଚଳ stacktop = ଗଦା + (bytes / ଆକାର(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ଦ୍ୱିଗୁଣ *fpr_base = (ଦ୍ୱିଗୁଣ *) (stacktop - ASM_NEEDS_REGISTERS) - ଅଙ୍କ_FPR_ARG_REGISTERS;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା gp_ଗଣନା = 0, fparg_ଗଣନା = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = ଗଦା + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଦ୍ୱିଗୁଣ double_tmp;
  ଶୂନ୍ୟ **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#ଯଦି !defined(POWERPC_DARWIN64) 
  ଅକ୍ଷର *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (ଅକ୍ଷର *) ଗଦା & 0xF) == 0);
  FFI_ASSERT(((unsigned) (ଅକ୍ଷର *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  ଯଦି (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (ଅକ୍ଷର *) ecif->rvalue;

  /* Now for the arguments.  */
  ପାଇଁ (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ବାଛ ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  double_tmp = *(ଦଶମିକ *) *p_argv;
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#ଯଦି defined(POWERPC_DARWIN)
	  *(ଦଶମିକ *)next_arg = *(ଦଶମିକ *) *p_argv;
#ଅନ୍ୟଥା
	  *(ଦ୍ୱିଗୁଣ *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_ଗଣନା++;
	  fparg_ଗଣନା++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  double_tmp = *(ଦ୍ୱିଗୁଣ *) *p_argv;
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ଦ୍ୱିଗୁଣ *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_ଗଣନା++;
#ଅନ୍ୟଥା
	  next_arg += 2;
	  gp_ଗଣନା += 2;
#endif
	  fparg_ଗଣନା++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#  ଯଦି defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS)
	    *(long ଦ୍ୱିଗୁଣ *) fpr_base++ = *(long ଦ୍ୱିଗୁଣ *) *p_argv;
	  ଅନ୍ୟଥା
	    *(long ଦ୍ୱିଗୁଣ *) next_arg = *(long ଦ୍ୱିଗୁଣ *) *p_argv;
	  next_arg += 2;
	  fparg_ଗଣନା += 2;
#  ଅନ୍ୟଥା
	  double_tmp = ((ଦ୍ୱିଗୁଣ *) *p_argv)[0];
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ଦ୍ୱିଗୁଣ *) next_arg = double_tmp;
#    ଯଦି defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ଗଣନା++;
#    ଅନ୍ୟଥା
	  next_arg += 2;
	  gp_ଗଣନା += 2;
#    endif
	  fparg_ଗଣନା++;
	  double_tmp = ((ଦ୍ୱିଗୁଣ *) *p_argv)[1];
	  ଯଦି (fparg_ଗଣନା < ଅଙ୍କ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ଦ୍ୱିଗୁଣ *) next_arg = double_tmp;
#    ଯଦି defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ଗଣନା++;
#    ଅନ୍ୟଥା
	  next_arg += 2;
	  gp_ଗଣନା += 2;
#    endif
	  fparg_ଗଣନା++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ଭାଙ୍ଗ;
#endif
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  ଯାଅ putgpr;
#ଅନ୍ୟଥା
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_ଗଣନା += 2;
#endif
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  gprvalue = *(unsigned ଅକ୍ଷର *) *p_argv;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  gprvalue = *(signed ଅକ୍ଷର *) *p_argv;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  ଯାଅ putgpr;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  ଯାଅ putgpr;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#ଯଦି defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((ଅକ୍ଷର *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_ମୂଲ୍ୟ (*ptr, (ଅକ୍ଷର *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &fparg_ଗଣନା,
					 &fpr_base, &next_arg);
#ଅନ୍ୟଥା
	  dest_cpy = (ଅକ୍ଷର *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  ଯଦି ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  ଯଦି defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((ଅକ୍ଷର *) dest_cpy, (ଅକ୍ଷର *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  ଅନ୍ୟଥା
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  ଯଦି (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((ଅକ୍ଷର *) dest_cpy, (ଅକ୍ଷର *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  gprvalue = *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) *p_argv;
	  ଯାଅ putgpr;

	ସ୍ଥିତି FFI_TYPE_UINT32:
	  gprvalue = *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_ଗଣନା++;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  ଭାଙ୍ଗ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#ଯଦି defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ସ୍ଥିର ଶୂନ୍ୟ
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ffi_type *p = s->elements[i];
      ବାଛ (p->type)
	{
	  ସ୍ଥିତି FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ଭାଙ୍ଗ;
	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    ଭାଙ୍ଗ;    
	}
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, ଅକ୍ଷର *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ଅକ୍ଷର *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ଯଦି (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      ବାଛ (p->type)
	{
	  ସ୍ଥିତି FFI_TYPE_STRUCT:
	    ଯଦି (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      ଫେରନ୍ତୁ 1;
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	    ଯଦି (*nfpr >= ଅଙ୍କ_FPR_ARG_REGISTERS)
	      ଫେରନ୍ତୁ 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (*nfpr >= ଅଙ୍କ_FPR_ARG_REGISTERS)
	      ଫେରନ୍ତୁ 1;
	    (*nfpr) += 1;
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ଯଦି (*nfpr >= ଅଙ୍କ_FPR_ARG_REGISTERS)
	      ଫେରନ୍ତୁ 1;
	    (*nfpr) += 1;
	    ଭାଙ୍ଗ;
	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    ଯଦି ((unsigned long)item_base >= 8*8) 
	      ଫେରନ୍ତୁ 1;
	    ଭାଙ୍ଗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ଫେରନ୍ତୁ 0;
}

/* Can this struct be returned by value?  */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା 
darwin64_struct_ret_by_ମୂଲ୍ୟ_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  ଯଦି (s->size > 168)
    ଫେରନ୍ତୁ 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  ଯଦି (nfp > 13)
    ଫେରନ୍ତୁ 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  ଯଦି (s->size <= 64)
    ଫେରନ୍ତୁ 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  ଯଦି (darwin64_struct_size_exceeds_gprs_p (s, ଖାଲି, &nfp))
    ଫେରନ୍ତୁ 0;
  
  ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ
darwin64_pass_struct_floats (ffi_type *s, ଅକ୍ଷର *src, 
			     unsigned *nfpr, ଦ୍ୱିଗୁଣ **fprs)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଦ୍ୱିଗୁଣ *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ଅକ୍ଷର *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ଯଦି (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      ବାଛ (p->type)
	{
	  ସ୍ଥିତି FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	    ଯଦି (*nfpr < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ଦ୍ୱିଗୁଣ *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (*nfpr < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ଦ୍ୱିଗୁଣ *)item_base;
	    (*nfpr) += 1;
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ଯଦି (*nfpr < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      *fpr_base++ = (ଦ୍ୱିଗୁଣ) *(ଦଶମିକ *)item_base;
	    (*nfpr) += 1;
	    ଭାଙ୍ଗ;
	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    ଭାଙ୍ଗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ସ୍ଥିର ଶୂନ୍ୟ
darwin64_pass_struct_by_ମୂଲ୍ୟ (ffi_type *s, ଅକ୍ଷର *src, unsigned size,
			       unsigned *nfpr, ଦ୍ୱିଗୁଣ **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  ଅକ୍ଷର *dest_cpy = (ଅକ୍ଷର *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ଯଦି (!size)
    ଫେରନ୍ତୁ;

  /* First... special cases.  */
  ଯଦି (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((ଅକ୍ଷର *) dest_cpy, src, size);
      next_arg++;
    }
  ଅନ୍ୟଥା ଯଦି (size == 16)
    {
      memcpy ((ଅକ୍ଷର *) dest_cpy, src, size);
      next_arg += 2;
    }
  ଅନ୍ୟଥା
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((ଅକ୍ଷର *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

ଦ୍ୱିଗୁଣ *
darwin64_struct_floats_to_mem (ffi_type *s, ଅକ୍ଷର *dest, ଦ୍ୱିଗୁଣ *fprs, unsigned *nf)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ଅକ୍ଷର *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ଯଦି (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      ବାଛ (p->type)
	{
	  ସ୍ଥିତି FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	    ଯଦି (*nf < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      {
		*(ଦ୍ୱିଗୁଣ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (*nf < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      {
		*(ଦ୍ୱିଗୁଣ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    ଭାଙ୍ଗ;
	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ଯଦି (*nf < ଅଙ୍କ_FPR_ARG_REGISTERS)
	      {
		*(ଦଶମିକ *)item_base = (ଦଶମିକ) *fprs++ ;
		(*nf) += 1;
	      }
	    ଭାଙ୍ଗ;
	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    ଭାଙ୍ଗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ଫେରନ୍ତୁ fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ସ୍ଥିର ଶୂନ୍ୟ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  ଯଦି (s->type != FFI_TYPE_STRUCT)
    ଫେରନ୍ତୁ;

  s->size = 0;
  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ffi_type *p;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା align;
      
      p = s->elements[i];
      ଯଦି (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#ଯଦି defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#ଅନ୍ୟଥା
      /* Natural alignment for the first item... */
      ଯଦି (i == 0)
	align = p->alignment;
      ଅନ୍ୟଥା ଯଦି (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      ଅନ୍ୟଥା
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  ଯଦି (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
aix_adjust_aggregate_sizes (ffi_type *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା outer_most_type_or_first_member)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nested_first_member=0, final_align, rc=0;

  ଯଦି (s->type != FFI_TYPE_STRUCT)
    ଫେରନ୍ତୁ 0;

  s->size = 0;
  ପାଇଁ (i = 0; s->elements[i] != ଖାଲି; i++)
    {
      ffi_type p;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      ଯଦି (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      ଅନ୍ୟଥା
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      ଯଦି (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  ଯଦି ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      ଯଦି (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  ଫେରନ୍ତୁ rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_ଗଣନା = 0, intarg_ଗଣନା = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  ଯଦି (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      ପାଇଁ (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  ଯଦି (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      ପାଇଁ (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * ଆକାର(unsigned long);

  /* Return value handling.  
    The rules m32 are as follows:
     - 32-bit (or less) integer values are returned in gpr3;
     - structures of size <= 4 bytes also returned in gpr3;
     - 64-bit integer values [??? and structures between 5 and 8 bytes] are
       returned in gpr3 and gpr4;
     - Single/double FP values are returned in fpr1;
     - Long double FP (if not equivalent to double) values are returned in
       fpr1 and fpr2;
     m64:
     - 64-bit or smaller integral values are returned in GPR3
     - Single/double FP values are returned in fpr1;
     - Long double FP values are returned in fpr1 and fpr2;
     m64 Structures:
     - If the structure could be accommodated in registers were it to be the
       first argument to a routine, then it is returned in those registers.
     m32/m64 structures otherwise:
     - Larger structures values are allocated space and a pointer is passed
       as the first argument.  */
  ବାଛ (cif->rtype->type)
    {

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      ଭାଙ୍ଗ;
#endif

    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_SINT64:
#ifdef POWERPC64
    ସ୍ଥିତି FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
#ଯଦି defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	ଯଦି (darwin64_struct_ret_by_ମୂଲ୍ୟ_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    ଯଦି (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    ଅନ୍ୟଥା
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    ଯଦି (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	ଅନ୍ୟଥା /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_ଗଣନା++;
	  }
      }
#ନଚେତଯଦି defined(DARWIN_PPC)
      ଯଦି (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      ଅନ୍ୟଥା /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_ଗଣନା++;
	}
#ଅନ୍ୟଥା /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_ଗଣନା++;
#endif
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ଭାଙ୍ଗ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  ପାଇଁ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_ଶବ୍ଦଗୁଡ଼ିକ;
      ବାଛ ((*ptr)->type)
	{
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  fparg_ଗଣନା++;
#ଯଦି !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  ଯଦି (fparg_ଗଣନା > ଅଙ୍କ_FPR_ARG_REGISTERS
	      && (intarg_ଗଣନା & 0x01) != 0)
	    intarg_ଗଣନା++;
#endif
	  ଭାଙ୍ଗ;

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  fparg_ଗଣନା += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  ଯଦି (fparg_ଗଣନା >= ଅଙ୍କ_FPR_ARG_REGISTERS)
#ଯଦି defined (POWERPC64)
	    intarg_ଗଣନା = FFI_ALIGN(intarg_ଗଣନା, 2);
#ଅନ୍ୟଥା
	    intarg_ଗଣନା = FFI_ALIGN(intarg_ଗଣନା, 4);
#endif
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
#ଯଦି defined(POWERPC64)
	  intarg_ଗଣନା++;
#ଅନ୍ୟଥା
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  ଯଦି (intarg_ଗଣନା == ଅଙ୍କ_GPR_ARG_REGISTERS-1
	      || (intarg_ଗଣନା >= ଅଙ୍କ_GPR_ARG_REGISTERS 
	          && (intarg_ଗଣନା & 0x01) != 0))
	    intarg_ଗଣନା++;
	  intarg_ଗଣନା += 2;
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#ଯଦି defined(POWERPC_DARWIN64)
	  align_ଶବ୍ଦଗୁଡ଼ିକ = (*ptr)->alignment >> 3;
	  ଯଦି (align_ଶବ୍ଦଗୁଡ଼ିକ)
	    intarg_ଗଣନା = FFI_ALIGN(intarg_ଗଣନା, align_ଶବ୍ଦଗୁଡ଼ିକ);
	  /* Base size of the struct.  */
	  intarg_ଗଣନା += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  ଯଦି (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_ଗଣନା) ;
#ଅନ୍ୟଥା
	  align_ଶବ୍ଦଗୁଡ଼ିକ = (*ptr)->alignment >> 2;
	  ଯଦି (align_ଶବ୍ଦଗୁଡ଼ିକ)
	    intarg_ଗଣନା = FFI_ALIGN(intarg_ଗଣନା, align_ଶବ୍ଦଗୁଡ଼ିକ);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_ଗଣନା += (size_al + 7) / 8;
#  ଅନ୍ୟଥା
	  intarg_ଗଣନା += (size_al + 3) / 4;
#  endif
#endif
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ଗଣନା++;
	  ଭାଙ୍ଗ;
	}
    }

  ଯଦି (fparg_ଗଣନା != 0)
    flags |= FLAG_FP_ARGUMENTS;

#ଯଦି defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  ଯଦି (fparg_ଗଣନା != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += ଅଙ୍କ_FPR_ARG_REGISTERS * ଆକାର(ଦ୍ୱିଗୁଣ);
#ଅନ୍ୟଥା
  /* Space for the FPR registers, if needed.  */
  ଯଦି (fparg_ଗଣନା != 0)
    bytes += ଅଙ୍କ_FPR_ARG_REGISTERS * ଆକାର(ଦ୍ୱିଗୁଣ);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  ଯଦି ((intarg_ଗଣନା + fparg_ଗଣନା) > ଅଙ୍କ_GPR_ARG_REGISTERS)
    bytes += (intarg_ଗଣନା + fparg_ଗଣନା) * ଆକାର(long);
#ଅନ୍ୟଥା
  ଯଦି ((intarg_ଗଣନା + 2 * fparg_ଗଣନା) > ଅଙ୍କ_GPR_ARG_REGISTERS)
    bytes += (intarg_ଗଣନା + 2 * fparg_ଗଣନା) * ଆକାର(long);
#endif
  ଅନ୍ୟଥା
    bytes += ଅଙ୍କ_GPR_ARG_REGISTERS * ଆକାର(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ (*fn2)(ଶୂନ୍ୟ));

#ଯଦି defined (FFI_GO_CLOSURES)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ (*fn2)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure);
#endif

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ (*fn2)(ଶୂନ୍ୟ), ffi_type*);

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ଯଦି ((rvalue == ଖାଲି) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}

#ଯଦି defined (FFI_GO_CLOSURES)
ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn) (ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue,
	     ଶୂନ୍ୟ *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ଯଦି ((rvalue == ଖାଲି) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ flush_icache(ଅକ୍ଷର *);
ସ୍ଥିର ଶୂନ୍ୟ flush_range(ଅକ୍ଷର *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା aix_fd_struct {
  ଶୂନ୍ୟ *code_pointer;
  ଶୂନ୍ୟ *toc;
} aix_fd;

/* here I'd like to add the stack frame layout we use in darwin_closure.S
   and aix_closure.S

   m32/m64

   The stack layout looks like this:

   |   Additional params...			| |     Higher address
   ~						~ ~
   |   Parameters      (at least 8*4/8=32/64)	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |
   |   Reserved                       2*4/8	| |
   |--------------------------------------------| |
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |
   |   Current backchain pointer	4/8	|-/ Parent's frame.
   |--------------------------------------------| <+ <<< on entry to ffi_closure_ASM
   |   Result Bytes			16	| |
   |--------------------------------------------| |
   ~   padding to 16-byte alignment		~ ~
   |--------------------------------------------| |
   |   NUM_FPR_ARG_REGISTERS slots		| |
   |   here fp13 .. fp1		       13*8	| |
   |--------------------------------------------| |
   |   R3..R10			  8*4/8=32/64	| | NUM_GPR_ARG_REGISTERS
   |--------------------------------------------| |
   |   TOC=R2 (AIX) Reserved (Darwin)   4/8	| |
   |--------------------------------------------| |	stack	|
   |   Reserved [compiler,binder]     2*4/8	| |	grows	|
   |--------------------------------------------| |	down	V
   |   Space for callee's LR		4/8	| |
   |--------------------------------------------| |	lower addresses
   |   Saved CR [low word for m64]      4/8	| |
   |--------------------------------------------| |     stack pointer here
   |   Current backchain pointer	4/8	|-/	during
   |--------------------------------------------|   <<<	ffi_closure_ASM.

*/

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp;
  ସଂରଚନା ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ବାଛ (cif->abi)
    {
      ସ୍ଥିତି FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
#ଯଦି defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((unsigned long *)&tramp[2]) = (unsigned long) ffi_closure_ASM; /* function  */
	*((unsigned long *)&tramp[4]) = (unsigned long) codeloc; /* context  */
#ଅନ୍ୟଥା
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (unsigned long) ffi_closure_ASM; /* function  */
	tramp[3] = (unsigned long) codeloc; /* context  */
#endif
	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_SIZE);

	ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_AIX:

      tramp_aix = (ସଂରଚନା ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(ଶୂନ୍ୟ *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_BAD_ABI;
      ଭାଙ୍ଗ;
    }
  ଫେରନ୍ତୁ FFI_OK;
}

#ଯଦି defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*))
{
  ବାଛ (cif->abi)
    {
      ସ୍ଥିତି FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (ଶୂନ୍ୟ *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        ଫେରନ୍ତୁ FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        ଫେରନ୍ତୁ FFI_BAD_ABI;
        ଭାଙ୍ଗ;
    }
  ଫେରନ୍ତୁ FFI_OK;
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ
flush_icache(ଅକ୍ଷର *addr)
{
#ifndef _AIX
  __asm__ ଅସ୍ଥିର (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

ସ୍ଥିର ଶୂନ୍ୟ
flush_range(ଅକ୍ଷର * addr1, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
#define MIN_ଧାଡ଼ି_SIZE 32
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପାଇଁ (i = 0; i < size; i += MIN_ଧାଡ଼ି_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂଘ
{
  ଦଶମିକ f;
  ଦ୍ୱିଗୁଣ d;
} ffi_dblfl;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_DARWIN (ffi_closure *, ଶୂନ୍ୟ *,
			   unsigned long *, ffi_dblfl *);

#ଯଦି defined (FFI_GO_CLOSURES)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_go_closure_helper_DARWIN (ffi_go_closure*, ଶୂନ୍ୟ *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_common (ffi_cif* cif,
			   ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			   ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  ପ୍ରକାରବ୍ୟାଖ୍ୟା ଦ୍ୱିଗୁଣ ldbits[2];

  ସଂଘ ldu
  {
    ldbits lb;
    long ଦ୍ୱିଗୁଣ ld;
  };

  ଶୂନ୍ୟ **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + ଅଙ୍କ_FPR_ARG_REGISTERS;
  unsigned         size_al;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା              struct_ret_by_ମୂଲ୍ୟ = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#ଯଦି defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * ଆକାର(ଶୂନ୍ୟ *));

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#ଯଦି defined(POWERPC_DARWIN64)
      ଯଦି (!darwin64_struct_ret_by_ମୂଲ୍ୟ_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (ଶୂନ୍ୟ *) *pgr;
	  pgr++;
	}
      ଅନ୍ୟଥା
	struct_ret_by_ମୂଲ୍ୟ = 1;
#ନଚେତଯଦି defined(DARWIN_PPC)
      ଯଦି (cif->rtype->size > 4)
	{
	  rvalue = (ଶୂନ୍ୟ *) *pgr;
	  pgr++;
	}
      ଅନ୍ୟଥା
	struct_ret_by_ମୂଲ୍ୟ = 1;
#ଅନ୍ୟଥା /* assume we return by ref.  */
      rvalue = (ଶୂନ୍ୟ *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ଯେତେବେଳେ (i < avn)
    {
      ବାଛ (arg_types[i]->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
#ଯଦି  defined(POWERPC64)
	  avalue[i] = (ଅକ୍ଷର *) pgr + 7;
#ଅନ୍ୟଥା
	  avalue[i] = (ଅକ୍ଷର *) pgr + 3;
#endif
	  pgr++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
#ଯଦି  defined(POWERPC64)
	  avalue[i] = (ଅକ୍ଷର *) pgr + 6;
#ଅନ୍ୟଥା
	  avalue[i] = (ଅକ୍ଷର *) pgr + 2;
#endif
	  pgr++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
#ଯଦି  defined(POWERPC64)
	  avalue[i] = (ଅକ୍ଷର *) pgr + 4;
#ଅନ୍ୟଥା
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#ଯଦି defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((ଅକ୍ଷର *)pgr, arg_types[i]->alignment);
	  ଯଦି (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((ଅକ୍ଷର *)pgr)+8-size_al;
	      ଯଦି (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < ଅଙ୍କ_FPR_ARG_REGISTERS)
		{
		  *(ଦଶମିକ *)pgr = (ଦଶମିକ) *(ଦ୍ୱିଗୁଣ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  ଅନ୍ୟଥା 
	    {
	      ଯଦି (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (ଅକ୍ଷର *)pgr,
						   (ଦ୍ୱିଗୁଣ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#ଅନ୍ୟଥା
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  ଯଦି (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  ଯଦି defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  ଅନ୍ୟଥା
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  ଯଦି (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (ଅକ୍ଷର*) pgr + 4 - size_al;
	  ଅନ୍ୟଥା
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
#ଯଦି  defined(POWERPC64)
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  ଭାଙ୍ଗ;
#ଅନ୍ୟଥା
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ଭାଙ୍ଗ;
#endif

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  ଯଦି (pfr < end_pfr)
	    {
	      ଦ୍ୱିଗୁଣ temp = pfr->d;
	      pfr->f = (ଦଶମିକ) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  ଯଦି (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#ଅନ୍ୟଥା
	  pgr += 2;
#endif
	  ଭାଙ୍ଗ;

#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  ଯଦି (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  ଅନ୍ୟଥା
	    {
	      ଯଦି (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#ଅନ୍ୟଥା  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  ଯଦି (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  ଅନ୍ୟଥା ଯଦି (pfr + 1 == end_pfr)
	    {
	      ସଂଘ ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, ଆକାର(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, ଆକାର(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  ଭାଙ୍ଗ;
#endif
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      ଫେରନ୍ତୁ PPC_LD_NONE;
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      ଯଦି (struct_ret_by_ମୂଲ୍ୟ)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  ଫେରନ୍ତୁ PPC_LD_STRUCT;
	}
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
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_helper_DARWIN (ffi_closure *closure, ଶୂନ୍ୟ *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  ଫେରନ୍ତୁ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#ଯଦି defined (FFI_GO_CLOSURES)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, ଶୂନ୍ୟ *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  ଫେରନ୍ତୁ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

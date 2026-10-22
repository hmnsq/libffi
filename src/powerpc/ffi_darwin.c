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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdlib.h>


সংরচনা ffi_aix_trampoline_struct {
    শূন্য * code_pointer;	/* Pointer to ffi_closure_ASM */
    শূন্য * toc;			/* TOC */
    শূন্য * static_chain;	/* Pointer to closure */
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
#অন্যথা
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

বাহ্য শূন্য ffi_closure_ASM (শূন্য);

#যদি defined (FFI_GO_CLOSURES)
বাহ্য শূন্য ffi_go_closure_ASM (শূন্য);
#endif

গণনা {
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
গণনা {
  অঙ্ক_GPR_ARG_REGISTERS = 8,
  অঙ্ক_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

গণনা { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#যদি defined(POWERPC_DARWIN64)
স্থির শূন্য
darwin64_pass_struct_by_মান 
  (ffi_type *, ৱর্ণ *, unsigned, unsigned *, দ্ৱিগুণ **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

শূন্য
ffi_prep_args (extended_cif *ecif, unsigned long *ধ্রুৱ স্তূপ)
{
  ধ্রুৱ unsigned bytes = ecif->cif->bytes;
  ধ্রুৱ unsigned flags = ecif->cif->flags;
  ধ্রুৱ unsigned nargs = ecif->cif->nargs;
#যদি !defined(POWERPC_DARWIN64) 
  ধ্রুৱ ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *ধ্রুৱ stacktop = স্তূপ + (bytes / পরিমাণ(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  দ্ৱিগুণ *fpr_base = (দ্ৱিগুণ *) (stacktop - ASM_NEEDS_REGISTERS) - অঙ্ক_FPR_ARG_REGISTERS;
  পূর্ণাঙ্ক gp_গুনতি = 0, fparg_গুনতি = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = স্তূপ + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  পূর্ণাঙ্ক i;
  দ্ৱিগুণ double_tmp;
  শূন্য **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#যদি !defined(POWERPC_DARWIN64) 
  ৱর্ণ *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (ৱর্ণ *) স্তূপ & 0xF) == 0);
  FFI_ASSERT(((unsigned) (ৱর্ণ *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  যদি (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (ৱর্ণ *) ecif->rvalue;

  /* Now for the arguments.  */
  বাবে (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ৱিকল্প ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	পক্ষ FFI_TYPE_FLOAT:
	  double_tmp = *(ভিন্ন *) *p_argv;
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#যদি defined(POWERPC_DARWIN)
	  *(ভিন্ন *)next_arg = *(ভিন্ন *) *p_argv;
#অন্যথা
	  *(দ্ৱিগুণ *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_গুনতি++;
	  fparg_গুনতি++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  double_tmp = *(দ্ৱিগুণ *) *p_argv;
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(দ্ৱিগুণ *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_গুনতি++;
#অন্যথা
	  next_arg += 2;
	  gp_গুনতি += 2;
#endif
	  fparg_গুনতি++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ভঙ্গ;

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	পক্ষ FFI_TYPE_LONGDOUBLE:
#  যদি defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS)
	    *(long দ্ৱিগুণ *) fpr_base++ = *(long দ্ৱিগুণ *) *p_argv;
	  অন্যথা
	    *(long দ্ৱিগুণ *) next_arg = *(long দ্ৱিগুণ *) *p_argv;
	  next_arg += 2;
	  fparg_গুনতি += 2;
#  অন্যথা
	  double_tmp = ((দ্ৱিগুণ *) *p_argv)[0];
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(দ্ৱিগুণ *) next_arg = double_tmp;
#    যদি defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_গুনতি++;
#    অন্যথা
	  next_arg += 2;
	  gp_গুনতি += 2;
#    endif
	  fparg_গুনতি++;
	  double_tmp = ((দ্ৱিগুণ *) *p_argv)[1];
	  যদি (fparg_গুনতি < অঙ্ক_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(দ্ৱিগুণ *) next_arg = double_tmp;
#    যদি defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_গুনতি++;
#    অন্যথা
	  next_arg += 2;
	  gp_গুনতি += 2;
#    endif
	  fparg_গুনতি++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ভঙ্গ;
#endif
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  গচ্ছ putgpr;
#অন্যথা
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_গুনতি += 2;
#endif
	  ভঙ্গ;
	পক্ষ FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_UINT8:
	  gprvalue = *(unsigned ৱর্ণ *) *p_argv;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_SINT8:
	  gprvalue = *(signed ৱর্ণ *) *p_argv;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  গচ্ছ putgpr;
	পক্ষ FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  গচ্ছ putgpr;

	পক্ষ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#যদি defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((ৱর্ণ *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_মান (*ptr, (ৱর্ণ *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned পূর্ণাঙ্ক *) &fparg_গুনতি,
					 &fpr_base, &next_arg);
#অন্যথা
	  dest_cpy = (ৱর্ণ *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  যদি ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  যদি defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((ৱর্ণ *) dest_cpy, (ৱর্ণ *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  অন্যথা
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  যদি (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((ৱর্ণ *) dest_cpy, (ৱর্ণ *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_SINT32:
	  gprvalue = *(signed পূর্ণাঙ্ক *) *p_argv;
	  গচ্ছ putgpr;

	পক্ষ FFI_TYPE_UINT32:
	  gprvalue = *(unsigned পূর্ণাঙ্ক *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_গুনতি++;
	  ভঙ্গ;
	স্ৱভাৱ:
	  ভঙ্গ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#যদি defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
স্থির শূন্য
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  পূর্ণাঙ্ক i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ffi_type *p = s->elements[i];
      ৱিকল্প (p->type)
	{
	  পক্ষ FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_DOUBLE:
	  পক্ষ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ভঙ্গ;
	  স্ৱভাৱ:
	    ভঙ্গ;    
	}
    }
}

স্থির পূর্ণাঙ্ক
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, ৱর্ণ *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ৱর্ণ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      যদি (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      ৱিকল্প (p->type)
	{
	  পক্ষ FFI_TYPE_STRUCT:
	    যদি (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      ঘূৰাই 1;
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_LONGDOUBLE:
	    যদি (*nfpr >= অঙ্ক_FPR_ARG_REGISTERS)
	      ঘূৰাই 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  পক্ষ FFI_TYPE_DOUBLE:
	    যদি (*nfpr >= অঙ্ক_FPR_ARG_REGISTERS)
	      ঘূৰাই 1;
	    (*nfpr) += 1;
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_FLOAT:
	    যদি (*nfpr >= অঙ্ক_FPR_ARG_REGISTERS)
	      ঘূৰাই 1;
	    (*nfpr) += 1;
	    ভঙ্গ;
	  স্ৱভাৱ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    যদি ((unsigned long)item_base >= 8*8) 
	      ঘূৰাই 1;
	    ভঙ্গ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ঘূৰাই 0;
}

/* Can this struct be returned by value?  */
পূর্ণাঙ্ক 
darwin64_struct_ret_by_মান_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  যদি (s->size > 168)
    ঘূৰাই 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  যদি (nfp > 13)
    ঘূৰাই 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  যদি (s->size <= 64)
    ঘূৰাই 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  যদি (darwin64_struct_size_exceeds_gprs_p (s, রিক্ত, &nfp))
    ঘূৰাই 0;
  
  ঘূৰাই 1;
}

শূন্য
darwin64_pass_struct_floats (ffi_type *s, ৱর্ণ *src, 
			     unsigned *nfpr, দ্ৱিগুণ **fprs)
{
  পূর্ণাঙ্ক i;
  দ্ৱিগুণ *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ৱর্ণ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      যদি (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      ৱিকল্প (p->type)
	{
	  পক্ষ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_LONGDOUBLE:
	    যদি (*nfpr < অঙ্ক_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(দ্ৱিগুণ *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  পক্ষ FFI_TYPE_DOUBLE:
	    যদি (*nfpr < অঙ্ক_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(দ্ৱিগুণ *)item_base;
	    (*nfpr) += 1;
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_FLOAT:
	    যদি (*nfpr < অঙ্ক_FPR_ARG_REGISTERS)
	      *fpr_base++ = (দ্ৱিগুণ) *(ভিন্ন *)item_base;
	    (*nfpr) += 1;
	    ভঙ্গ;
	  স্ৱভাৱ:
	    ভঙ্গ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
স্থির শূন্য
darwin64_pass_struct_by_মান (ffi_type *s, ৱর্ণ *src, unsigned size,
			       unsigned *nfpr, দ্ৱিগুণ **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  ৱর্ণ *dest_cpy = (ৱর্ণ *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  যদি (!size)
    ঘূৰাই;

  /* First... special cases.  */
  যদি (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((ৱর্ণ *) dest_cpy, src, size);
      next_arg++;
    }
  অন্যথা যদি (size == 16)
    {
      memcpy ((ৱর্ণ *) dest_cpy, src, size);
      next_arg += 2;
    }
  অন্যথা
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((ৱর্ণ *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

দ্ৱিগুণ *
darwin64_struct_floats_to_mem (ffi_type *s, ৱর্ণ *dest, দ্ৱিগুণ *fprs, unsigned *nf)
{
  পূর্ণাঙ্ক i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ৱর্ণ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      যদি (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      ৱিকল্প (p->type)
	{
	  পক্ষ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_LONGDOUBLE:
	    যদি (*nf < অঙ্ক_FPR_ARG_REGISTERS)
	      {
		*(দ্ৱিগুণ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  পক্ষ FFI_TYPE_DOUBLE:
	    যদি (*nf < অঙ্ক_FPR_ARG_REGISTERS)
	      {
		*(দ্ৱিগুণ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    ভঙ্গ;
	  পক্ষ FFI_TYPE_FLOAT:
	    যদি (*nf < অঙ্ক_FPR_ARG_REGISTERS)
	      {
		*(ভিন্ন *)item_base = (ভিন্ন) *fprs++ ;
		(*nf) += 1;
	      }
	    ভঙ্গ;
	  স্ৱভাৱ:
	    ভঙ্গ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ঘূৰাই fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

স্থির শূন্য
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  পূর্ণাঙ্ক i;

  যদি (s->type != FFI_TYPE_STRUCT)
    ঘূৰাই;

  s->size = 0;
  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ffi_type *p;
      পূর্ণাঙ্ক align;
      
      p = s->elements[i];
      যদি (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#যদি defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#অন্যথা
      /* Natural alignment for the first item... */
      যদি (i == 0)
	align = p->alignment;
      অন্যথা যদি (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      অন্যথা
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  যদি (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

স্থির পূর্ণাঙ্ক
aix_adjust_aggregate_sizes (ffi_type *s, পূর্ণাঙ্ক outer_most_type_or_first_member)
{
  পূর্ণাঙ্ক i, nested_first_member=0, final_align, rc=0;

  যদি (s->type != FFI_TYPE_STRUCT)
    ঘূৰাই 0;

  s->size = 0;
  বাবে (i = 0; s->elements[i] != রিক্ত; i++)
    {
      ffi_type p;
      পূর্ণাঙ্ক align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      যদি (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      অন্যথা
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      যদি (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  যদি ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      যদি (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  ঘূৰাই rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_গুনতি = 0, intarg_গুনতি = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  যদি (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      বাবে (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  যদি (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      বাবে (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * পরিমাণ(unsigned long);

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
  ৱিকল্প (cif->rtype->type)
    {

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    পক্ষ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      ভঙ্গ;
#endif

    পক্ষ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    পক্ষ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ভঙ্গ;

    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
#ifdef POWERPC64
    পক্ষ FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
#যদি defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	যদি (darwin64_struct_ret_by_মান_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    যদি (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    অন্যথা
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    যদি (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	অন্যথা /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_গুনতি++;
	  }
      }
#নোচেদ্যদি defined(DARWIN_PPC)
      যদি (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      অন্যথা /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_গুনতি++;
	}
#অন্যথা /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_গুনতি++;
#endif
      ভঙ্গ;
    পক্ষ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ভঙ্গ;

    স্ৱভাৱ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ভঙ্গ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  বাবে (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_শব্দগুলি;
      ৱিকল্প ((*ptr)->type)
	{
	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	  fparg_গুনতি++;
#যদি !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  যদি (fparg_গুনতি > অঙ্ক_FPR_ARG_REGISTERS
	      && (intarg_গুনতি & 0x01) != 0)
	    intarg_গুনতি++;
#endif
	  ভঙ্গ;

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  fparg_গুনতি += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  যদি (fparg_গুনতি >= অঙ্ক_FPR_ARG_REGISTERS)
#যদি defined (POWERPC64)
	    intarg_গুনতি = FFI_ALIGN(intarg_গুনতি, 2);
#অন্যথা
	    intarg_গুনতি = FFI_ALIGN(intarg_গুনতি, 4);
#endif
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
#যদি defined(POWERPC64)
	  intarg_গুনতি++;
#অন্যথা
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  যদি (intarg_গুনতি == অঙ্ক_GPR_ARG_REGISTERS-1
	      || (intarg_গুনতি >= অঙ্ক_GPR_ARG_REGISTERS 
	          && (intarg_গুনতি & 0x01) != 0))
	    intarg_গুনতি++;
	  intarg_গুনতি += 2;
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#যদি defined(POWERPC_DARWIN64)
	  align_শব্দগুলি = (*ptr)->alignment >> 3;
	  যদি (align_শব্দগুলি)
	    intarg_গুনতি = FFI_ALIGN(intarg_গুনতি, align_শব্দগুলি);
	  /* Base size of the struct.  */
	  intarg_গুনতি += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  যদি (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_গুনতি) ;
#অন্যথা
	  align_শব্দগুলি = (*ptr)->alignment >> 2;
	  যদি (align_শব্দগুলি)
	    intarg_গুনতি = FFI_ALIGN(intarg_গুনতি, align_শব্দগুলি);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_গুনতি += (size_al + 7) / 8;
#  অন্যথা
	  intarg_গুনতি += (size_al + 3) / 4;
#  endif
#endif
	  ভঙ্গ;

	স্ৱভাৱ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_গুনতি++;
	  ভঙ্গ;
	}
    }

  যদি (fparg_গুনতি != 0)
    flags |= FLAG_FP_ARGUMENTS;

#যদি defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  যদি (fparg_গুনতি != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += অঙ্ক_FPR_ARG_REGISTERS * পরিমাণ(দ্ৱিগুণ);
#অন্যথা
  /* Space for the FPR registers, if needed.  */
  যদি (fparg_গুনতি != 0)
    bytes += অঙ্ক_FPR_ARG_REGISTERS * পরিমাণ(দ্ৱিগুণ);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  যদি ((intarg_গুনতি + fparg_গুনতি) > অঙ্ক_GPR_ARG_REGISTERS)
    bytes += (intarg_গুনতি + fparg_গুনতি) * পরিমাণ(long);
#অন্যথা
  যদি ((intarg_গুনতি + 2 * fparg_গুনতি) > অঙ্ক_GPR_ARG_REGISTERS)
    bytes += (intarg_গুনতি + 2 * fparg_গুনতি) * পরিমাণ(long);
#endif
  অন্যথা
    bytes += অঙ্ক_GPR_ARG_REGISTERS * পরিমাণ(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  ঘূৰাই FFI_OK;
}

বাহ্য শূন্য ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 শূন্য (*fn)(শূন্য), শূন্য (*fn2)(শূন্য));

#যদি defined (FFI_GO_CLOSURES)
বাহ্য শূন্য ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    শূন্য (*fn)(শূন্য), শূন্য (*fn2)(শূন্য), শূন্য *closure);
#endif

বাহ্য শূন্য ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    শূন্য (*fn)(শূন্য), শূন্য (*fn2)(শূন্য), ffi_type*);

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  যদি ((rvalue == রিক্ত) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  ৱিকল্প (cif->abi)
    {
    পক্ষ FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      ভঙ্গ;
    পক্ষ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      ভঙ্গ;
    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
}

#যদি defined (FFI_GO_CLOSURES)
শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn) (শূন্য), শূন্য *rvalue, শূন্য **avalue,
	     শূন্য *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  যদি ((rvalue == রিক্ত) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  ৱিকল্প (cif->abi)
    {
    পক্ষ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      ভঙ্গ;
    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
}
#endif

স্থির শূন্য flush_icache(ৱর্ণ *);
স্থির শূন্য flush_range(ৱর্ণ *, পূর্ণাঙ্ক);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

প্রকারনির্দেশ সংরচনা aix_fd_struct {
  শূন্য *code_pointer;
  শূন্য *toc;
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
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp;
  সংরচনা ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ৱিকল্প (cif->abi)
    {
      পক্ষ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
#যদি defined(POWERPC_DARWIN64)
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
#অন্যথা
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

	ভঙ্গ;

    পক্ষ FFI_AIX:

      tramp_aix = (সংরচনা ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(শূন্য *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      ভঙ্গ;

    স্ৱভাৱ:
      ঘূৰাই FFI_BAD_ABI;
      ভঙ্গ;
    }
  ঘূৰাই FFI_OK;
}

#যদি defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
  ৱিকল্প (cif->abi)
    {
      পক্ষ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (শূন্য *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        ঘূৰাই FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      স্ৱভাৱ:
        ঘূৰাই FFI_BAD_ABI;
        ভঙ্গ;
    }
  ঘূৰাই FFI_OK;
}
#endif

স্থির শূন্য
flush_icache(ৱর্ণ *addr)
{
#ifndef _AIX
  __asm__ চঞ্চল (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

স্থির শূন্য
flush_range(ৱর্ণ * addr1, পূর্ণাঙ্ক size)
{
#define MIN_সাৰি_SIZE 32
  পূর্ণাঙ্ক i;
  বাবে (i = 0; i < size; i += MIN_সাৰি_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

প্রকারনির্দেশ সঙ্ঘ
{
  ভিন্ন f;
  দ্ৱিগুণ d;
} ffi_dblfl;

পূর্ণাঙ্ক
ffi_closure_helper_DARWIN (ffi_closure *, শূন্য *,
			   unsigned long *, ffi_dblfl *);

#যদি defined (FFI_GO_CLOSURES)
পূর্ণাঙ্ক
ffi_go_closure_helper_DARWIN (ffi_go_closure*, শূন্য *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

স্থির পূর্ণাঙ্ক
ffi_closure_helper_common (ffi_cif* cif,
			   শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			   শূন্য *user_data, শূন্য *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  প্রকারনির্দেশ দ্ৱিগুণ ldbits[2];

  সঙ্ঘ ldu
  {
    ldbits lb;
    long দ্ৱিগুণ ld;
  };

  শূন্য **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + অঙ্ক_FPR_ARG_REGISTERS;
  unsigned         size_al;
  পূর্ণাঙ্ক              struct_ret_by_মান = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#যদি defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * পরিমাণ(শূন্য *));

  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#যদি defined(POWERPC_DARWIN64)
      যদি (!darwin64_struct_ret_by_মান_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (শূন্য *) *pgr;
	  pgr++;
	}
      অন্যথা
	struct_ret_by_মান = 1;
#নোচেদ্যদি defined(DARWIN_PPC)
      যদি (cif->rtype->size > 4)
	{
	  rvalue = (শূন্য *) *pgr;
	  pgr++;
	}
      অন্যথা
	struct_ret_by_মান = 1;
#অন্যথা /* assume we return by ref.  */
      rvalue = (শূন্য *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  যেতিয়ালৈ (i < avn)
    {
      ৱিকল্প (arg_types[i]->type)
	{
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT8:
#যদি  defined(POWERPC64)
	  avalue[i] = (ৱর্ণ *) pgr + 7;
#অন্যথা
	  avalue[i] = (ৱর্ণ *) pgr + 3;
#endif
	  pgr++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT16:
#যদি  defined(POWERPC64)
	  avalue[i] = (ৱর্ণ *) pgr + 6;
#অন্যথা
	  avalue[i] = (ৱর্ণ *) pgr + 2;
#endif
	  pgr++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT32:
#যদি  defined(POWERPC64)
	  avalue[i] = (ৱর্ণ *) pgr + 4;
#অন্যথা
	পক্ষ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#যদি defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((ৱর্ণ *)pgr, arg_types[i]->alignment);
	  যদি (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((ৱর্ণ *)pgr)+8-size_al;
	      যদি (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < অঙ্ক_FPR_ARG_REGISTERS)
		{
		  *(ভিন্ন *)pgr = (ভিন্ন) *(দ্ৱিগুণ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  অন্যথা 
	    {
	      যদি (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (ৱর্ণ *)pgr,
						   (দ্ৱিগুণ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#অন্যথা
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  যদি (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  যদি defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  অন্যথা
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  যদি (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (ৱর্ণ*) pgr + 4 - size_al;
	  অন্যথা
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  ভঙ্গ;

	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_UINT64:
#যদি  defined(POWERPC64)
	পক্ষ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  ভঙ্গ;
#অন্যথা
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ভঙ্গ;
#endif

	পক্ষ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  যদি (pfr < end_pfr)
	    {
	      দ্ৱিগুণ temp = pfr->d;
	      pfr->f = (ভিন্ন) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  যদি (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  অন্যথা
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#অন্যথা
	  pgr += 2;
#endif
	  ভঙ্গ;

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	পক্ষ FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  যদি (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  অন্যথা
	    {
	      যদি (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#অন্যথা  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  যদি (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  অন্যথা যদি (pfr + 1 == end_pfr)
	    {
	      সঙ্ঘ ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, পরিমাণ(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, পরিমাণ(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  অন্যথা
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  ভঙ্গ;
#endif
	স্ৱভাৱ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ৱিকল্প (cif->rtype->type)
    {
    পক্ষ FFI_TYPE_VOID:
      ঘূৰাই PPC_LD_NONE;
    পক্ষ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      যদি (struct_ret_by_মান)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  ঘূৰাই PPC_LD_STRUCT;
	}
      ঘূৰাই PPC_LD_NONE;
    পক্ষ FFI_TYPE_FLOAT:
      ঘূৰাই PPC_LD_F32;
    পক্ষ FFI_TYPE_DOUBLE:
      ঘূৰাই PPC_LD_F64;
#যদি FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    পক্ষ FFI_TYPE_LONGDOUBLE:
      ঘূৰাই PPC_LD_F128;
#endif
    পক্ষ FFI_TYPE_UINT8:
      ঘূৰাই PPC_LD_U8;
    পক্ষ FFI_TYPE_SINT8:
      ঘূৰাই PPC_LD_S8;
    পক্ষ FFI_TYPE_UINT16:
      ঘূৰাই PPC_LD_U16;
    পক্ষ FFI_TYPE_SINT16:
      ঘূৰাই PPC_LD_S16;
    পক্ষ FFI_TYPE_UINT32:
      ঘূৰাই PPC_LD_U32;
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      ঘূৰাই PPC_LD_S32;
    পক্ষ FFI_TYPE_POINTER:
      ঘূৰাই PPC_LD_PTR;
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_SINT64:
      ঘূৰাই PPC_LD_I64;
    স্ৱভাৱ:
      abort();
    }
}

পূর্ণাঙ্ক
ffi_closure_helper_DARWIN (ffi_closure *closure, শূন্য *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  ঘূৰাই ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#যদি defined (FFI_GO_CLOSURES)
পূর্ণাঙ্ক
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, শূন্য *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  ঘূৰাই ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

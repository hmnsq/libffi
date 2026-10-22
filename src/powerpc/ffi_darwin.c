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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>


கட்டமைப்பு ffi_aix_trampoline_struct {
    வெறுமை * code_pointer;	/* Pointer to ffi_closure_ASM */
    வெறுமை * toc;			/* TOC */
    வெறுமை * static_chain;	/* Pointer to closure */
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
#இல்லையெனில்
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

வெளி வெறுமை ffi_closure_ASM (வெறுமை);

#எனில் defined (FFI_GO_CLOSURES)
வெளி வெறுமை ffi_go_closure_ASM (வெறுமை);
#endif

எண்ணிக்கை {
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
எண்ணிக்கை {
  இலக்கம்_GPR_ARG_REGISTERS = 8,
  இலக்கம்_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

எண்ணிக்கை { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#எனில் defined(POWERPC_DARWIN64)
நிலையான வெறுமை
darwin64_pass_struct_by_மதிப்பு 
  (ffi_type *, எழுத்து *, unsigned, unsigned *, இரட்டை **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

வெறுமை
ffi_prep_args (extended_cif *ecif, unsigned long *மாறிலி அடுக்கு)
{
  மாறிலி unsigned bytes = ecif->cif->bytes;
  மாறிலி unsigned flags = ecif->cif->flags;
  மாறிலி unsigned nargs = ecif->cif->nargs;
#எனில் !defined(POWERPC_DARWIN64) 
  மாறிலி ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *மாறிலி stacktop = அடுக்கு + (bytes / அளவு(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  இரட்டை *fpr_base = (இரட்டை *) (stacktop - ASM_NEEDS_REGISTERS) - இலக்கம்_FPR_ARG_REGISTERS;
  முழு gp_மொத்தம் = 0, fparg_மொத்தம் = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = அடுக்கு + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  முழு i;
  இரட்டை double_tmp;
  வெறுமை **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#எனில் !defined(POWERPC_DARWIN64) 
  எழுத்து *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (எழுத்து *) அடுக்கு & 0xF) == 0);
  FFI_ASSERT(((unsigned) (எழுத்து *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  எனில் (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (எழுத்து *) ecif->rvalue;

  /* Now for the arguments.  */
  க்கு (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      தேர்வு ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	நிலை FFI_TYPE_FLOAT:
	  double_tmp = *(தசம *) *p_argv;
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#எனில் defined(POWERPC_DARWIN)
	  *(தசம *)next_arg = *(தசம *) *p_argv;
#இல்லையெனில்
	  *(இரட்டை *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_மொத்தம்++;
	  fparg_மொத்தம்++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  double_tmp = *(இரட்டை *) *p_argv;
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(இரட்டை *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_மொத்தம்++;
#இல்லையெனில்
	  next_arg += 2;
	  gp_மொத்தம் += 2;
#endif
	  fparg_மொத்தம்++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  நிறுத்து;

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	நிலை FFI_TYPE_LONGDOUBLE:
#  எனில் defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS)
	    *(long இரட்டை *) fpr_base++ = *(long இரட்டை *) *p_argv;
	  இல்லையெனில்
	    *(long இரட்டை *) next_arg = *(long இரட்டை *) *p_argv;
	  next_arg += 2;
	  fparg_மொத்தம் += 2;
#  இல்லையெனில்
	  double_tmp = ((இரட்டை *) *p_argv)[0];
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(இரட்டை *) next_arg = double_tmp;
#    எனில் defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_மொத்தம்++;
#    இல்லையெனில்
	  next_arg += 2;
	  gp_மொத்தம் += 2;
#    endif
	  fparg_மொத்தம்++;
	  double_tmp = ((இரட்டை *) *p_argv)[1];
	  எனில் (fparg_மொத்தம் < இலக்கம்_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(இரட்டை *) next_arg = double_tmp;
#    எனில் defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_மொத்தம்++;
#    இல்லையெனில்
	  next_arg += 2;
	  gp_மொத்தம் += 2;
#    endif
	  fparg_மொத்தம்++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  நிறுத்து;
#endif
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  செல் putgpr;
#இல்லையெனில்
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_மொத்தம் += 2;
#endif
	  நிறுத்து;
	நிலை FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  செல் putgpr;
	நிலை FFI_TYPE_UINT8:
	  gprvalue = *(unsigned எழுத்து *) *p_argv;
	  செல் putgpr;
	நிலை FFI_TYPE_SINT8:
	  gprvalue = *(signed எழுத்து *) *p_argv;
	  செல் putgpr;
	நிலை FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  செல் putgpr;
	நிலை FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  செல் putgpr;

	நிலை FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#எனில் defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((எழுத்து *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_மதிப்பு (*ptr, (எழுத்து *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned முழு *) &fparg_மொத்தம்,
					 &fpr_base, &next_arg);
#இல்லையெனில்
	  dest_cpy = (எழுத்து *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  எனில் ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  எனில் defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((எழுத்து *) dest_cpy, (எழுத்து *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  இல்லையெனில்
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  எனில் (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((எழுத்து *) dest_cpy, (எழுத்து *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT32:
	  gprvalue = *(signed முழு *) *p_argv;
	  செல் putgpr;

	நிலை FFI_TYPE_UINT32:
	  gprvalue = *(unsigned முழு *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_மொத்தம்++;
	  நிறுத்து;
	இயல்பு:
	  நிறுத்து;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#எனில் defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
நிலையான வெறுமை
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  முழு i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      ffi_type *p = s->elements[i];
      தேர்வு (p->type)
	{
	  நிலை FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    நிறுத்து;
	  நிலை FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    நிறுத்து;
	  நிலை FFI_TYPE_DOUBLE:
	  நிலை FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    நிறுத்து;
	  இயல்பு:
	    நிறுத்து;    
	}
    }
}

நிலையான முழு
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, எழுத்து *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      எழுத்து *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      எனில் (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      தேர்வு (p->type)
	{
	  நிலை FFI_TYPE_STRUCT:
	    எனில் (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      திரும்பு 1;
	    நிறுத்து;
	  நிலை FFI_TYPE_LONGDOUBLE:
	    எனில் (*nfpr >= இலக்கம்_FPR_ARG_REGISTERS)
	      திரும்பு 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (*nfpr >= இலக்கம்_FPR_ARG_REGISTERS)
	      திரும்பு 1;
	    (*nfpr) += 1;
	    நிறுத்து;
	  நிலை FFI_TYPE_FLOAT:
	    எனில் (*nfpr >= இலக்கம்_FPR_ARG_REGISTERS)
	      திரும்பு 1;
	    (*nfpr) += 1;
	    நிறுத்து;
	  இயல்பு:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    எனில் ((unsigned long)item_base >= 8*8) 
	      திரும்பு 1;
	    நிறுத்து;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  திரும்பு 0;
}

/* Can this struct be returned by value?  */
முழு 
darwin64_struct_ret_by_மதிப்பு_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  எனில் (s->size > 168)
    திரும்பு 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  எனில் (nfp > 13)
    திரும்பு 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  எனில் (s->size <= 64)
    திரும்பு 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  எனில் (darwin64_struct_size_exceeds_gprs_p (s, வெற்று, &nfp))
    திரும்பு 0;
  
  திரும்பு 1;
}

வெறுமை
darwin64_pass_struct_floats (ffi_type *s, எழுத்து *src, 
			     unsigned *nfpr, இரட்டை **fprs)
{
  முழு i;
  இரட்டை *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      எழுத்து *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      எனில் (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      தேர்வு (p->type)
	{
	  நிலை FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    நிறுத்து;
	  நிலை FFI_TYPE_LONGDOUBLE:
	    எனில் (*nfpr < இலக்கம்_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(இரட்டை *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (*nfpr < இலக்கம்_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(இரட்டை *)item_base;
	    (*nfpr) += 1;
	    நிறுத்து;
	  நிலை FFI_TYPE_FLOAT:
	    எனில் (*nfpr < இலக்கம்_FPR_ARG_REGISTERS)
	      *fpr_base++ = (இரட்டை) *(தசம *)item_base;
	    (*nfpr) += 1;
	    நிறுத்து;
	  இயல்பு:
	    நிறுத்து;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
நிலையான வெறுமை
darwin64_pass_struct_by_மதிப்பு (ffi_type *s, எழுத்து *src, unsigned size,
			       unsigned *nfpr, இரட்டை **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  எழுத்து *dest_cpy = (எழுத்து *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  எனில் (!size)
    திரும்பு;

  /* First... special cases.  */
  எனில் (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((எழுத்து *) dest_cpy, src, size);
      next_arg++;
    }
  இல்லையெனில் எனில் (size == 16)
    {
      memcpy ((எழுத்து *) dest_cpy, src, size);
      next_arg += 2;
    }
  இல்லையெனில்
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((எழுத்து *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

இரட்டை *
darwin64_struct_floats_to_mem (ffi_type *s, எழுத்து *dest, இரட்டை *fprs, unsigned *nf)
{
  முழு i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      எழுத்து *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      எனில் (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      தேர்வு (p->type)
	{
	  நிலை FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    நிறுத்து;
	  நிலை FFI_TYPE_LONGDOUBLE:
	    எனில் (*nf < இலக்கம்_FPR_ARG_REGISTERS)
	      {
		*(இரட்டை *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (*nf < இலக்கம்_FPR_ARG_REGISTERS)
	      {
		*(இரட்டை *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    நிறுத்து;
	  நிலை FFI_TYPE_FLOAT:
	    எனில் (*nf < இலக்கம்_FPR_ARG_REGISTERS)
	      {
		*(தசம *)item_base = (தசம) *fprs++ ;
		(*nf) += 1;
	      }
	    நிறுத்து;
	  இயல்பு:
	    நிறுத்து;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  திரும்பு fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

நிலையான வெறுமை
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  முழு i;

  எனில் (s->type != FFI_TYPE_STRUCT)
    திரும்பு;

  s->size = 0;
  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      ffi_type *p;
      முழு align;
      
      p = s->elements[i];
      எனில் (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#எனில் defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#இல்லையெனில்
      /* Natural alignment for the first item... */
      எனில் (i == 0)
	align = p->alignment;
      இல்லையெனில் எனில் (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      இல்லையெனில்
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  எனில் (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

நிலையான முழு
aix_adjust_aggregate_sizes (ffi_type *s, முழு outer_most_type_or_first_member)
{
  முழு i, nested_first_member=0, final_align, rc=0;

  எனில் (s->type != FFI_TYPE_STRUCT)
    திரும்பு 0;

  s->size = 0;
  க்கு (i = 0; s->elements[i] != வெற்று; i++)
    {
      ffi_type p;
      முழு align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      எனில் (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      இல்லையெனில்
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      எனில் (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  எனில் ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      எனில் (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  திரும்பு rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_மொத்தம் = 0, intarg_மொத்தம் = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  எனில் (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      க்கு (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  எனில் (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      க்கு (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * அளவு(unsigned long);

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
  தேர்வு (cif->rtype->type)
    {

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      நிறுத்து;
#endif

    நிலை FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    நிலை FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      நிறுத்து;

    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
#ifdef POWERPC64
    நிலை FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
#எனில் defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	எனில் (darwin64_struct_ret_by_மதிப்பு_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    எனில் (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    இல்லையெனில்
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    எனில் (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	இல்லையெனில் /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_மொத்தம்++;
	  }
      }
#அல்லதெனில் defined(DARWIN_PPC)
      எனில் (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      இல்லையெனில் /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_மொத்தம்++;
	}
#இல்லையெனில் /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_மொத்தம்++;
#endif
      நிறுத்து;
    நிலை FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      நிறுத்து;

    இயல்பு:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      நிறுத்து;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  க்கு (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_சொற்கள்;
      தேர்வு ((*ptr)->type)
	{
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	  fparg_மொத்தம்++;
#எனில் !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  எனில் (fparg_மொத்தம் > இலக்கம்_FPR_ARG_REGISTERS
	      && (intarg_மொத்தம் & 0x01) != 0)
	    intarg_மொத்தம்++;
#endif
	  நிறுத்து;

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
	  fparg_மொத்தம் += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  எனில் (fparg_மொத்தம் >= இலக்கம்_FPR_ARG_REGISTERS)
#எனில் defined (POWERPC64)
	    intarg_மொத்தம் = FFI_ALIGN(intarg_மொத்தம், 2);
#இல்லையெனில்
	    intarg_மொத்தம் = FFI_ALIGN(intarg_மொத்தம், 4);
#endif
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
#எனில் defined(POWERPC64)
	  intarg_மொத்தம்++;
#இல்லையெனில்
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  எனில் (intarg_மொத்தம் == இலக்கம்_GPR_ARG_REGISTERS-1
	      || (intarg_மொத்தம் >= இலக்கம்_GPR_ARG_REGISTERS 
	          && (intarg_மொத்தம் & 0x01) != 0))
	    intarg_மொத்தம்++;
	  intarg_மொத்தம் += 2;
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#எனில் defined(POWERPC_DARWIN64)
	  align_சொற்கள் = (*ptr)->alignment >> 3;
	  எனில் (align_சொற்கள்)
	    intarg_மொத்தம் = FFI_ALIGN(intarg_மொத்தம், align_சொற்கள்);
	  /* Base size of the struct.  */
	  intarg_மொத்தம் += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  எனில் (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_மொத்தம்) ;
#இல்லையெனில்
	  align_சொற்கள் = (*ptr)->alignment >> 2;
	  எனில் (align_சொற்கள்)
	    intarg_மொத்தம் = FFI_ALIGN(intarg_மொத்தம், align_சொற்கள்);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_மொத்தம் += (size_al + 7) / 8;
#  இல்லையெனில்
	  intarg_மொத்தம் += (size_al + 3) / 4;
#  endif
#endif
	  நிறுத்து;

	இயல்பு:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_மொத்தம்++;
	  நிறுத்து;
	}
    }

  எனில் (fparg_மொத்தம் != 0)
    flags |= FLAG_FP_ARGUMENTS;

#எனில் defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  எனில் (fparg_மொத்தம் != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += இலக்கம்_FPR_ARG_REGISTERS * அளவு(இரட்டை);
#இல்லையெனில்
  /* Space for the FPR registers, if needed.  */
  எனில் (fparg_மொத்தம் != 0)
    bytes += இலக்கம்_FPR_ARG_REGISTERS * அளவு(இரட்டை);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  எனில் ((intarg_மொத்தம் + fparg_மொத்தம்) > இலக்கம்_GPR_ARG_REGISTERS)
    bytes += (intarg_மொத்தம் + fparg_மொத்தம்) * அளவு(long);
#இல்லையெனில்
  எனில் ((intarg_மொத்தம் + 2 * fparg_மொத்தம்) > இலக்கம்_GPR_ARG_REGISTERS)
    bytes += (intarg_மொத்தம் + 2 * fparg_மொத்தம்) * அளவு(long);
#endif
  இல்லையெனில்
    bytes += இலக்கம்_GPR_ARG_REGISTERS * அளவு(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 வெறுமை (*fn)(வெறுமை), வெறுமை (*fn2)(வெறுமை));

#எனில் defined (FFI_GO_CLOSURES)
வெளி வெறுமை ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    வெறுமை (*fn)(வெறுமை), வெறுமை (*fn2)(வெறுமை), வெறுமை *closure);
#endif

வெளி வெறுமை ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    வெறுமை (*fn)(வெறுமை), வெறுமை (*fn2)(வெறுமை), ffi_type*);

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  எனில் ((rvalue == வெற்று) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      நிறுத்து;
    நிலை FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}

#எனில் defined (FFI_GO_CLOSURES)
வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	     வெறுமை *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  எனில் ((rvalue == வெற்று) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}
#endif

நிலையான வெறுமை flush_icache(எழுத்து *);
நிலையான வெறுமை flush_range(எழுத்து *, முழு);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

வகைவரை கட்டமைப்பு aix_fd_struct {
  வெறுமை *code_pointer;
  வெறுமை *toc;
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
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp;
  கட்டமைப்பு ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  தேர்வு (cif->abi)
    {
      நிலை FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned முழு *) &closure->tramp[0];
#எனில் defined(POWERPC_DARWIN64)
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
#இல்லையெனில்
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

	நிறுத்து;

    நிலை FFI_AIX:

      tramp_aix = (கட்டமைப்பு ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(வெறுமை *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      நிறுத்து;

    இயல்பு:
      திரும்பு FFI_BAD_ABI;
      நிறுத்து;
    }
  திரும்பு FFI_OK;
}

#எனில் defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*))
{
  தேர்வு (cif->abi)
    {
      நிலை FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (வெறுமை *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        திரும்பு FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      இயல்பு:
        திரும்பு FFI_BAD_ABI;
        நிறுத்து;
    }
  திரும்பு FFI_OK;
}
#endif

நிலையான வெறுமை
flush_icache(எழுத்து *addr)
{
#ifndef _AIX
  __asm__ நிலையற்ற (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

நிலையான வெறுமை
flush_range(எழுத்து * addr1, முழு size)
{
#define MIN_வரி_SIZE 32
  முழு i;
  க்கு (i = 0; i < size; i += MIN_வரி_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

வகைவரை ஒன்றிப்பு
{
  தசம f;
  இரட்டை d;
} ffi_dblfl;

முழு
ffi_closure_helper_DARWIN (ffi_closure *, வெறுமை *,
			   unsigned long *, ffi_dblfl *);

#எனில் defined (FFI_GO_CLOSURES)
முழு
ffi_go_closure_helper_DARWIN (ffi_go_closure*, வெறுமை *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

நிலையான முழு
ffi_closure_helper_common (ffi_cif* cif,
			   வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			   வெறுமை *user_data, வெறுமை *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  வகைவரை இரட்டை ldbits[2];

  ஒன்றிப்பு ldu
  {
    ldbits lb;
    long இரட்டை ld;
  };

  வெறுமை **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + இலக்கம்_FPR_ARG_REGISTERS;
  unsigned         size_al;
  முழு              struct_ret_by_மதிப்பு = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#எனில் defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * அளவு(வெறுமை *));

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#எனில் defined(POWERPC_DARWIN64)
      எனில் (!darwin64_struct_ret_by_மதிப்பு_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (வெறுமை *) *pgr;
	  pgr++;
	}
      இல்லையெனில்
	struct_ret_by_மதிப்பு = 1;
#அல்லதெனில் defined(DARWIN_PPC)
      எனில் (cif->rtype->size > 4)
	{
	  rvalue = (வெறுமை *) *pgr;
	  pgr++;
	}
      இல்லையெனில்
	struct_ret_by_மதிப்பு = 1;
#இல்லையெனில் /* assume we return by ref.  */
      rvalue = (வெறுமை *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  போது (i < avn)
    {
      தேர்வு (arg_types[i]->type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
#எனில்  defined(POWERPC64)
	  avalue[i] = (எழுத்து *) pgr + 7;
#இல்லையெனில்
	  avalue[i] = (எழுத்து *) pgr + 3;
#endif
	  pgr++;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
#எனில்  defined(POWERPC64)
	  avalue[i] = (எழுத்து *) pgr + 6;
#இல்லையெனில்
	  avalue[i] = (எழுத்து *) pgr + 2;
#endif
	  pgr++;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
#எனில்  defined(POWERPC64)
	  avalue[i] = (எழுத்து *) pgr + 4;
#இல்லையெனில்
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#எனில் defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((எழுத்து *)pgr, arg_types[i]->alignment);
	  எனில் (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((எழுத்து *)pgr)+8-size_al;
	      எனில் (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < இலக்கம்_FPR_ARG_REGISTERS)
		{
		  *(தசம *)pgr = (தசம) *(இரட்டை *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  இல்லையெனில் 
	    {
	      எனில் (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (எழுத்து *)pgr,
						   (இரட்டை *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#இல்லையெனில்
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  எனில் (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  எனில் defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  இல்லையெனில்
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  எனில் (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (எழுத்து*) pgr + 4 - size_al;
	  இல்லையெனில்
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
#எனில்  defined(POWERPC64)
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  நிறுத்து;
#இல்லையெனில்
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  நிறுத்து;
#endif

	நிலை FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  எனில் (pfr < end_pfr)
	    {
	      இரட்டை temp = pfr->d;
	      pfr->f = (தசம) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  இல்லையெனில்
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  எனில் (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  இல்லையெனில்
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#இல்லையெனில்
	  pgr += 2;
#endif
	  நிறுத்து;

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	நிலை FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  எனில் (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  இல்லையெனில்
	    {
	      எனில் (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#இல்லையெனில்  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  எனில் (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  இல்லையெனில் எனில் (pfr + 1 == end_pfr)
	    {
	      ஒன்றிப்பு ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, அளவு(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, அளவு(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  இல்லையெனில்
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  நிறுத்து;
#endif
	இயல்பு:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      திரும்பு PPC_LD_NONE;
    நிலை FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      எனில் (struct_ret_by_மதிப்பு)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  திரும்பு PPC_LD_STRUCT;
	}
      திரும்பு PPC_LD_NONE;
    நிலை FFI_TYPE_FLOAT:
      திரும்பு PPC_LD_F32;
    நிலை FFI_TYPE_DOUBLE:
      திரும்பு PPC_LD_F64;
#எனில் FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    நிலை FFI_TYPE_LONGDOUBLE:
      திரும்பு PPC_LD_F128;
#endif
    நிலை FFI_TYPE_UINT8:
      திரும்பு PPC_LD_U8;
    நிலை FFI_TYPE_SINT8:
      திரும்பு PPC_LD_S8;
    நிலை FFI_TYPE_UINT16:
      திரும்பு PPC_LD_U16;
    நிலை FFI_TYPE_SINT16:
      திரும்பு PPC_LD_S16;
    நிலை FFI_TYPE_UINT32:
      திரும்பு PPC_LD_U32;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      திரும்பு PPC_LD_S32;
    நிலை FFI_TYPE_POINTER:
      திரும்பு PPC_LD_PTR;
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
      திரும்பு PPC_LD_I64;
    இயல்பு:
      abort();
    }
}

முழு
ffi_closure_helper_DARWIN (ffi_closure *closure, வெறுமை *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  திரும்பு ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#எனில் defined (FFI_GO_CLOSURES)
முழு
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, வெறுமை *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  திரும்பு ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

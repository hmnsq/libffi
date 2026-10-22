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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>


ساختار ffi_aix_trampoline_struct {
    تهی * code_pointer;	/* Pointer to ffi_closure_ASM */
    تهی * toc;			/* TOC */
    تهی * static_chain;	/* Pointer to closure */
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
#وگرنه
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

بیرونی تهی ffi_closure_ASM (تهی);

#اگر defined (FFI_GO_CLOSURES)
بیرونی تهی ffi_go_closure_ASM (تهی);
#endif

شمارش {
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
شمارش {
  رقم_GPR_ARG_REGISTERS = 8,
  رقم_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

شمارش { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#اگر defined(POWERPC_DARWIN64)
ایستا تهی
darwin64_pass_struct_by_مقدار 
  (ffi_type *, نویسه *, unsigned, unsigned *, دوگانه **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

تهی
ffi_prep_args (extended_cif *ecif, unsigned long *ثابت پشته)
{
  ثابت unsigned bytes = ecif->cif->bytes;
  ثابت unsigned flags = ecif->cif->flags;
  ثابت unsigned nargs = ecif->cif->nargs;
#اگر !defined(POWERPC_DARWIN64) 
  ثابت ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *ثابت stacktop = پشته + (bytes / اندازه(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  دوگانه *fpr_base = (دوگانه *) (stacktop - ASM_NEEDS_REGISTERS) - رقم_FPR_ARG_REGISTERS;
  صحیح gp_شمار = 0, fparg_شمار = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = پشته + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  صحیح i;
  دوگانه double_tmp;
  تهی **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#اگر !defined(POWERPC_DARWIN64) 
  نویسه *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (نویسه *) پشته & 0xF) == 0);
  FFI_ASSERT(((unsigned) (نویسه *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  اگر (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (نویسه *) ecif->rvalue;

  /* Now for the arguments.  */
  برای (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      انتخاب ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	حالت FFI_TYPE_FLOAT:
	  double_tmp = *(اعشاری *) *p_argv;
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#اگر defined(POWERPC_DARWIN)
	  *(اعشاری *)next_arg = *(اعشاری *) *p_argv;
#وگرنه
	  *(دوگانه *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_شمار++;
	  fparg_شمار++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  double_tmp = *(دوگانه *) *p_argv;
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(دوگانه *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_شمار++;
#وگرنه
	  next_arg += 2;
	  gp_شمار += 2;
#endif
	  fparg_شمار++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  بشکن;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالت FFI_TYPE_LONGDOUBLE:
#  اگر defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *(long دوگانه *) fpr_base++ = *(long دوگانه *) *p_argv;
	  وگرنه
	    *(long دوگانه *) next_arg = *(long دوگانه *) *p_argv;
	  next_arg += 2;
	  fparg_شمار += 2;
#  وگرنه
	  double_tmp = ((دوگانه *) *p_argv)[0];
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(دوگانه *) next_arg = double_tmp;
#    اگر defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_شمار++;
#    وگرنه
	  next_arg += 2;
	  gp_شمار += 2;
#    endif
	  fparg_شمار++;
	  double_tmp = ((دوگانه *) *p_argv)[1];
	  اگر (fparg_شمار < رقم_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(دوگانه *) next_arg = double_tmp;
#    اگر defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_شمار++;
#    وگرنه
	  next_arg += 2;
	  gp_شمار += 2;
#    endif
	  fparg_شمار++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  بشکن;
#endif
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  برو putgpr;
#وگرنه
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_شمار += 2;
#endif
	  بشکن;
	حالت FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  برو putgpr;
	حالت FFI_TYPE_UINT8:
	  gprvalue = *(unsigned نویسه *) *p_argv;
	  برو putgpr;
	حالت FFI_TYPE_SINT8:
	  gprvalue = *(signed نویسه *) *p_argv;
	  برو putgpr;
	حالت FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  برو putgpr;
	حالت FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  برو putgpr;

	حالت FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#اگر defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((نویسه *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_مقدار (*ptr, (نویسه *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned صحیح *) &fparg_شمار,
					 &fpr_base, &next_arg);
#وگرنه
	  dest_cpy = (نویسه *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  اگر ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  اگر defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((نویسه *) dest_cpy, (نویسه *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  وگرنه
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  اگر (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((نویسه *) dest_cpy, (نویسه *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  بشکن;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  gprvalue = *(signed صحیح *) *p_argv;
	  برو putgpr;

	حالت FFI_TYPE_UINT32:
	  gprvalue = *(unsigned صحیح *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_شمار++;
	  بشکن;
	پیشفرض:
	  بشکن;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#اگر defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ایستا تهی
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  صحیح i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      ffi_type *p = s->elements[i];
      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    بشکن;
	  حالت FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    بشکن;
	  حالت FFI_TYPE_DOUBLE:
	  حالت FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    بشکن;
	  پیشفرض:
	    بشکن;    
	}
    }
}

ایستا صحیح
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, نویسه *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      نویسه *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    اگر (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      بازگشت 1;
	    بشکن;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      بازگشت 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      بازگشت 1;
	    (*nfpr) += 1;
	    بشکن;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nfpr >= رقم_FPR_ARG_REGISTERS)
	      بازگشت 1;
	    (*nfpr) += 1;
	    بشکن;
	  پیشفرض:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    اگر ((unsigned long)item_base >= 8*8) 
	      بازگشت 1;
	    بشکن;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  بازگشت 0;
}

/* Can this struct be returned by value?  */
صحیح 
darwin64_struct_ret_by_مقدار_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  اگر (s->size > 168)
    بازگشت 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  اگر (nfp > 13)
    بازگشت 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  اگر (s->size <= 64)
    بازگشت 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  اگر (darwin64_struct_size_exceeds_gprs_p (s, پوچ, &nfp))
    بازگشت 0;
  
  بازگشت 1;
}

تهی
darwin64_pass_struct_floats (ffi_type *s, نویسه *src, 
			     unsigned *nfpr, دوگانه **fprs)
{
  صحیح i;
  دوگانه *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      نویسه *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    بشکن;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(دوگانه *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(دوگانه *)item_base;
	    (*nfpr) += 1;
	    بشکن;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nfpr < رقم_FPR_ARG_REGISTERS)
	      *fpr_base++ = (دوگانه) *(اعشاری *)item_base;
	    (*nfpr) += 1;
	    بشکن;
	  پیشفرض:
	    بشکن;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ایستا تهی
darwin64_pass_struct_by_مقدار (ffi_type *s, نویسه *src, unsigned size,
			       unsigned *nfpr, دوگانه **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  نویسه *dest_cpy = (نویسه *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  اگر (!size)
    بازگشت;

  /* First... special cases.  */
  اگر (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((نویسه *) dest_cpy, src, size);
      next_arg++;
    }
  وگرنه اگر (size == 16)
    {
      memcpy ((نویسه *) dest_cpy, src, size);
      next_arg += 2;
    }
  وگرنه
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((نویسه *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

دوگانه *
darwin64_struct_floats_to_mem (ffi_type *s, نویسه *dest, دوگانه *fprs, unsigned *nf)
{
  صحیح i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      نویسه *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      اگر (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      انتخاب (p->type)
	{
	  حالت FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    بشکن;
	  حالت FFI_TYPE_LONGDOUBLE:
	    اگر (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(دوگانه *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  حالت FFI_TYPE_DOUBLE:
	    اگر (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(دوگانه *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    بشکن;
	  حالت FFI_TYPE_FLOAT:
	    اگر (*nf < رقم_FPR_ARG_REGISTERS)
	      {
		*(اعشاری *)item_base = (اعشاری) *fprs++ ;
		(*nf) += 1;
	      }
	    بشکن;
	  پیشفرض:
	    بشکن;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  بازگشت fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ایستا تهی
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  صحیح i;

  اگر (s->type != FFI_TYPE_STRUCT)
    بازگشت;

  s->size = 0;
  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      ffi_type *p;
      صحیح align;
      
      p = s->elements[i];
      اگر (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#اگر defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#وگرنه
      /* Natural alignment for the first item... */
      اگر (i == 0)
	align = p->alignment;
      وگرنه اگر (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      وگرنه
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  اگر (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ایستا صحیح
aix_adjust_aggregate_sizes (ffi_type *s, صحیح outer_most_type_or_first_member)
{
  صحیح i, nested_first_member=0, final_align, rc=0;

  اگر (s->type != FFI_TYPE_STRUCT)
    بازگشت 0;

  s->size = 0;
  برای (i = 0; s->elements[i] != پوچ; i++)
    {
      ffi_type p;
      صحیح align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      اگر (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      وگرنه
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      اگر (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  اگر ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      اگر (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  بازگشت rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_شمار = 0, intarg_شمار = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  اگر (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      برای (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  اگر (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      برای (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * اندازه(unsigned long);

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
  انتخاب (cif->rtype->type)
    {

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      بشکن;
#endif

    حالت FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    حالت FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      بشکن;

    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
#ifdef POWERPC64
    حالت FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      بشکن;

    حالت FFI_TYPE_STRUCT:
#اگر defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	اگر (darwin64_struct_ret_by_مقدار_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    اگر (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    وگرنه
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    اگر (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	وگرنه /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_شمار++;
	  }
      }
#وگرنهاگر defined(DARWIN_PPC)
      اگر (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      وگرنه /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_شمار++;
	}
#وگرنه /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_شمار++;
#endif
      بشکن;
    حالت FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      بشکن;

    پیشفرض:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      بشکن;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  برای (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_واژگان;
      انتخاب ((*ptr)->type)
	{
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	  fparg_شمار++;
#اگر !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  اگر (fparg_شمار > رقم_FPR_ARG_REGISTERS
	      && (intarg_شمار & 0x01) != 0)
	    intarg_شمار++;
#endif
	  بشکن;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
	  fparg_شمار += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  اگر (fparg_شمار >= رقم_FPR_ARG_REGISTERS)
#اگر defined (POWERPC64)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, 2);
#وگرنه
	    intarg_شمار = FFI_ALIGN(intarg_شمار, 4);
#endif
	  بشکن;
#endif

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#اگر defined(POWERPC64)
	  intarg_شمار++;
#وگرنه
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  اگر (intarg_شمار == رقم_GPR_ARG_REGISTERS-1
	      || (intarg_شمار >= رقم_GPR_ARG_REGISTERS 
	          && (intarg_شمار & 0x01) != 0))
	    intarg_شمار++;
	  intarg_شمار += 2;
#endif
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#اگر defined(POWERPC_DARWIN64)
	  align_واژگان = (*ptr)->alignment >> 3;
	  اگر (align_واژگان)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, align_واژگان);
	  /* Base size of the struct.  */
	  intarg_شمار += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  اگر (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_شمار) ;
#وگرنه
	  align_واژگان = (*ptr)->alignment >> 2;
	  اگر (align_واژگان)
	    intarg_شمار = FFI_ALIGN(intarg_شمار, align_واژگان);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_شمار += (size_al + 7) / 8;
#  وگرنه
	  intarg_شمار += (size_al + 3) / 4;
#  endif
#endif
	  بشکن;

	پیشفرض:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_شمار++;
	  بشکن;
	}
    }

  اگر (fparg_شمار != 0)
    flags |= FLAG_FP_ARGUMENTS;

#اگر defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  اگر (fparg_شمار != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += رقم_FPR_ARG_REGISTERS * اندازه(دوگانه);
#وگرنه
  /* Space for the FPR registers, if needed.  */
  اگر (fparg_شمار != 0)
    bytes += رقم_FPR_ARG_REGISTERS * اندازه(دوگانه);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  اگر ((intarg_شمار + fparg_شمار) > رقم_GPR_ARG_REGISTERS)
    bytes += (intarg_شمار + fparg_شمار) * اندازه(long);
#وگرنه
  اگر ((intarg_شمار + 2 * fparg_شمار) > رقم_GPR_ARG_REGISTERS)
    bytes += (intarg_شمار + 2 * fparg_شمار) * اندازه(long);
#endif
  وگرنه
    bytes += رقم_GPR_ARG_REGISTERS * اندازه(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  بازگشت FFI_OK;
}

بیرونی تهی ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 تهی (*fn)(تهی), تهی (*fn2)(تهی));

#اگر defined (FFI_GO_CLOSURES)
بیرونی تهی ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    تهی (*fn)(تهی), تهی (*fn2)(تهی), تهی *closure);
#endif

بیرونی تهی ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    تهی (*fn)(تهی), تهی (*fn2)(تهی), ffi_type*);

تهی
ffi_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر ((rvalue == پوچ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      بشکن;
    حالت FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
}

#اگر defined (FFI_GO_CLOSURES)
تهی
ffi_call_go (ffi_cif *cif, تهی (*fn) (تهی), تهی *rvalue, تهی **avalue,
	     تهی *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  اگر ((rvalue == پوچ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
}
#endif

ایستا تهی flush_icache(نویسه *);
ایستا تهی flush_range(نویسه *, صحیح);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

تعریفنوع ساختار aix_fd_struct {
  تهی *code_pointer;
  تهی *toc;
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
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp;
  ساختار ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  انتخاب (cif->abi)
    {
      حالت FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned صحیح *) &closure->tramp[0];
#اگر defined(POWERPC_DARWIN64)
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
#وگرنه
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

	بشکن;

    حالت FFI_AIX:

      tramp_aix = (ساختار ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(تهی *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      بشکن;

    پیشفرض:
      بازگشت FFI_BAD_ABI;
      بشکن;
    }
  بازگشت FFI_OK;
}

#اگر defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*))
{
  انتخاب (cif->abi)
    {
      حالت FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (تهی *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        بازگشت FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      پیشفرض:
        بازگشت FFI_BAD_ABI;
        بشکن;
    }
  بازگشت FFI_OK;
}
#endif

ایستا تهی
flush_icache(نویسه *addr)
{
#ifndef _AIX
  __asm__ فرار (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

ایستا تهی
flush_range(نویسه * addr1, صحیح size)
{
#define MIN_سطر_SIZE 32
  صحیح i;
  برای (i = 0; i < size; i += MIN_سطر_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

تعریفنوع اجتماع
{
  اعشاری f;
  دوگانه d;
} ffi_dblfl;

صحیح
ffi_closure_helper_DARWIN (ffi_closure *, تهی *,
			   unsigned long *, ffi_dblfl *);

#اگر defined (FFI_GO_CLOSURES)
صحیح
ffi_go_closure_helper_DARWIN (ffi_go_closure*, تهی *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ایستا صحیح
ffi_closure_helper_common (ffi_cif* cif,
			   تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
			   تهی *user_data, تهی *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  تعریفنوع دوگانه ldbits[2];

  اجتماع ldu
  {
    ldbits lb;
    long دوگانه ld;
  };

  تهی **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + رقم_FPR_ARG_REGISTERS;
  unsigned         size_al;
  صحیح              struct_ret_by_مقدار = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#اگر defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * اندازه(تهی *));

  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#اگر defined(POWERPC_DARWIN64)
      اگر (!darwin64_struct_ret_by_مقدار_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (تهی *) *pgr;
	  pgr++;
	}
      وگرنه
	struct_ret_by_مقدار = 1;
#وگرنهاگر defined(DARWIN_PPC)
      اگر (cif->rtype->size > 4)
	{
	  rvalue = (تهی *) *pgr;
	  pgr++;
	}
      وگرنه
	struct_ret_by_مقدار = 1;
#وگرنه /* assume we return by ref.  */
      rvalue = (تهی *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  وقتی (i < avn)
    {
      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
#اگر  defined(POWERPC64)
	  avalue[i] = (نویسه *) pgr + 7;
#وگرنه
	  avalue[i] = (نویسه *) pgr + 3;
#endif
	  pgr++;
	  بشکن;

	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
#اگر  defined(POWERPC64)
	  avalue[i] = (نویسه *) pgr + 6;
#وگرنه
	  avalue[i] = (نویسه *) pgr + 2;
#endif
	  pgr++;
	  بشکن;

	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
#اگر  defined(POWERPC64)
	  avalue[i] = (نویسه *) pgr + 4;
#وگرنه
	حالت FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#اگر defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((نویسه *)pgr, arg_types[i]->alignment);
	  اگر (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((نویسه *)pgr)+8-size_al;
	      اگر (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < رقم_FPR_ARG_REGISTERS)
		{
		  *(اعشاری *)pgr = (اعشاری) *(دوگانه *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  وگرنه 
	    {
	      اگر (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (نویسه *)pgr,
						   (دوگانه *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#وگرنه
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  اگر (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  اگر defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  وگرنه
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  اگر (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (نویسه*) pgr + 4 - size_al;
	  وگرنه
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  بشکن;

	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
#اگر  defined(POWERPC64)
	حالت FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  بشکن;
#وگرنه
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  بشکن;
#endif

	حالت FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr < end_pfr)
	    {
	      دوگانه temp = pfr->d;
	      pfr->f = (اعشاری) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وگرنه
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  وگرنه
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#وگرنه
	  pgr += 2;
#endif
	  بشکن;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	حالت FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  اگر (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  وگرنه
	    {
	      اگر (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#وگرنه  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  اگر (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  وگرنه اگر (pfr + 1 == end_pfr)
	    {
	      اجتماع ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, اندازه(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, اندازه(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  وگرنه
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  بشکن;
#endif
	پیشفرض:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      بازگشت PPC_LD_NONE;
    حالت FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      اگر (struct_ret_by_مقدار)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  بازگشت PPC_LD_STRUCT;
	}
      بازگشت PPC_LD_NONE;
    حالت FFI_TYPE_FLOAT:
      بازگشت PPC_LD_F32;
    حالت FFI_TYPE_DOUBLE:
      بازگشت PPC_LD_F64;
#اگر FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    حالت FFI_TYPE_LONGDOUBLE:
      بازگشت PPC_LD_F128;
#endif
    حالت FFI_TYPE_UINT8:
      بازگشت PPC_LD_U8;
    حالت FFI_TYPE_SINT8:
      بازگشت PPC_LD_S8;
    حالت FFI_TYPE_UINT16:
      بازگشت PPC_LD_U16;
    حالت FFI_TYPE_SINT16:
      بازگشت PPC_LD_S16;
    حالت FFI_TYPE_UINT32:
      بازگشت PPC_LD_U32;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      بازگشت PPC_LD_S32;
    حالت FFI_TYPE_POINTER:
      بازگشت PPC_LD_PTR;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      بازگشت PPC_LD_I64;
    پیشفرض:
      abort();
    }
}

صحیح
ffi_closure_helper_DARWIN (ffi_closure *closure, تهی *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  بازگشت ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#اگر defined (FFI_GO_CLOSURES)
صحیح
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, تهی *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  بازگشت ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

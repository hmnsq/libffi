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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>


संरचना ffi_aix_trampoline_struct {
    शून्य * code_pointer;	/* Pointer to ffi_closure_ASM */
    शून्य * toc;			/* TOC */
    शून्य * static_chain;	/* Pointer to closure */
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
#अन्यथा
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

बाह्य शून्य ffi_closure_ASM (शून्य);

#यदि defined (FFI_GO_CLOSURES)
बाह्य शून्य ffi_go_closure_ASM (शून्य);
#endif

गणना {
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
गणना {
  अंक_GPR_ARG_REGISTERS = 8,
  अंक_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

गणना { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#यदि defined(POWERPC_DARWIN64)
स्थिर शून्य
darwin64_pass_struct_by_मूल्य 
  (ffi_type *, वर्ण *, unsigned, unsigned *, द्विगुण **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

शून्य
ffi_prep_args (extended_cif *ecif, unsigned long *ध्रुव ढेर)
{
  ध्रुव unsigned bytes = ecif->cif->bytes;
  ध्रुव unsigned flags = ecif->cif->flags;
  ध्रुव unsigned nargs = ecif->cif->nargs;
#यदि !defined(POWERPC_DARWIN64) 
  ध्रुव ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *ध्रुव stacktop = ढेर + (bytes / परिमाण(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  द्विगुण *fpr_base = (द्विगुण *) (stacktop - ASM_NEEDS_REGISTERS) - अंक_FPR_ARG_REGISTERS;
  पूर्णाङ्क gp_गिनती = 0, fparg_गिनती = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = ढेर + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  पूर्णाङ्क i;
  द्विगुण double_tmp;
  शून्य **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#यदि !defined(POWERPC_DARWIN64) 
  वर्ण *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (वर्ण *) ढेर & 0xF) == 0);
  FFI_ASSERT(((unsigned) (वर्ण *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  यदि (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (वर्ण *) ecif->rvalue;

  /* Now for the arguments.  */
  प्रति (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      विकल्प ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	पक्ष FFI_TYPE_FLOAT:
	  double_tmp = *(भिन्न *) *p_argv;
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#यदि defined(POWERPC_DARWIN)
	  *(भिन्न *)next_arg = *(भिन्न *) *p_argv;
#अन्यथा
	  *(द्विगुण *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_गिनती++;
	  fparg_गिनती++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  double_tmp = *(द्विगुण *) *p_argv;
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_गिनती++;
#अन्यथा
	  next_arg += 2;
	  gp_गिनती += 2;
#endif
	  fparg_गिनती++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	पक्ष FFI_TYPE_LONGDOUBLE:
#  यदि defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *(long द्विगुण *) fpr_base++ = *(long द्विगुण *) *p_argv;
	  अन्यथा
	    *(long द्विगुण *) next_arg = *(long द्विगुण *) *p_argv;
	  next_arg += 2;
	  fparg_गिनती += 2;
#  अन्यथा
	  double_tmp = ((द्विगुण *) *p_argv)[0];
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *) next_arg = double_tmp;
#    यदि defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_गिनती++;
#    अन्यथा
	  next_arg += 2;
	  gp_गिनती += 2;
#    endif
	  fparg_गिनती++;
	  double_tmp = ((द्विगुण *) *p_argv)[1];
	  यदि (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *) next_arg = double_tmp;
#    यदि defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_गिनती++;
#    अन्यथा
	  next_arg += 2;
	  gp_गिनती += 2;
#    endif
	  fparg_गिनती++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  भङ्ग;
#endif
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  गच्छ putgpr;
#अन्यथा
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_गिनती += 2;
#endif
	  भङ्ग;
	पक्ष FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_UINT8:
	  gprvalue = *(unsigned वर्ण *) *p_argv;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT8:
	  gprvalue = *(signed वर्ण *) *p_argv;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  गच्छ putgpr;
	पक्ष FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  गच्छ putgpr;

	पक्ष FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#यदि defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((वर्ण *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_मूल्य (*ptr, (वर्ण *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned पूर्णाङ्क *) &fparg_गिनती,
					 &fpr_base, &next_arg);
#अन्यथा
	  dest_cpy = (वर्ण *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  यदि ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  यदि defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((वर्ण *) dest_cpy, (वर्ण *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  यदि (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((वर्ण *) dest_cpy, (वर्ण *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	  gprvalue = *(signed पूर्णाङ्क *) *p_argv;
	  गच्छ putgpr;

	पक्ष FFI_TYPE_UINT32:
	  gprvalue = *(unsigned पूर्णाङ्क *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_गिनती++;
	  भङ्ग;
	स्वभाव:
	  भङ्ग;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#यदि defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
स्थिर शून्य
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  पूर्णाङ्क i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type *p = s->elements[i];
      विकल्प (p->type)
	{
	  पक्ष FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    भङ्ग;
	  पक्ष FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    भङ्ग;
	  पक्ष FFI_TYPE_DOUBLE:
	  पक्ष FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    भङ्ग;
	  स्वभाव:
	    भङ्ग;    
	}
    }
}

स्थिर पूर्णाङ्क
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, वर्ण *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      वर्ण *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      यदि (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      विकल्प (p->type)
	{
	  पक्ष FFI_TYPE_STRUCT:
	    यदि (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      प्रत्यागम 1;
	    भङ्ग;
	  पक्ष FFI_TYPE_LONGDOUBLE:
	    यदि (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      प्रत्यागम 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      प्रत्यागम 1;
	    (*nfpr) += 1;
	    भङ्ग;
	  पक्ष FFI_TYPE_FLOAT:
	    यदि (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      प्रत्यागम 1;
	    (*nfpr) += 1;
	    भङ्ग;
	  स्वभाव:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    यदि ((unsigned long)item_base >= 8*8) 
	      प्रत्यागम 1;
	    भङ्ग;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  प्रत्यागम 0;
}

/* Can this struct be returned by value?  */
पूर्णाङ्क 
darwin64_struct_ret_by_मूल्य_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  यदि (s->size > 168)
    प्रत्यागम 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  यदि (nfp > 13)
    प्रत्यागम 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  यदि (s->size <= 64)
    प्रत्यागम 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  यदि (darwin64_struct_size_exceeds_gprs_p (s, रिक्त, &nfp))
    प्रत्यागम 0;
  
  प्रत्यागम 1;
}

शून्य
darwin64_pass_struct_floats (ffi_type *s, वर्ण *src, 
			     unsigned *nfpr, द्विगुण **fprs)
{
  पूर्णाङ्क i;
  द्विगुण *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      वर्ण *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      यदि (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      विकल्प (p->type)
	{
	  पक्ष FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    भङ्ग;
	  पक्ष FFI_TYPE_LONGDOUBLE:
	    यदि (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुण *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुण *)item_base;
	    (*nfpr) += 1;
	    भङ्ग;
	  पक्ष FFI_TYPE_FLOAT:
	    यदि (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = (द्विगुण) *(भिन्न *)item_base;
	    (*nfpr) += 1;
	    भङ्ग;
	  स्वभाव:
	    भङ्ग;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
स्थिर शून्य
darwin64_pass_struct_by_मूल्य (ffi_type *s, वर्ण *src, unsigned size,
			       unsigned *nfpr, द्विगुण **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  वर्ण *dest_cpy = (वर्ण *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  यदि (!size)
    प्रत्यागम;

  /* First... special cases.  */
  यदि (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((वर्ण *) dest_cpy, src, size);
      next_arg++;
    }
  अन्यथा यदि (size == 16)
    {
      memcpy ((वर्ण *) dest_cpy, src, size);
      next_arg += 2;
    }
  अन्यथा
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((वर्ण *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

द्विगुण *
darwin64_struct_floats_to_mem (ffi_type *s, वर्ण *dest, द्विगुण *fprs, unsigned *nf)
{
  पूर्णाङ्क i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      वर्ण *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      यदि (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      विकल्प (p->type)
	{
	  पक्ष FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    भङ्ग;
	  पक्ष FFI_TYPE_LONGDOUBLE:
	    यदि (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुण *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुण *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    भङ्ग;
	  पक्ष FFI_TYPE_FLOAT:
	    यदि (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(भिन्न *)item_base = (भिन्न) *fprs++ ;
		(*nf) += 1;
	      }
	    भङ्ग;
	  स्वभाव:
	    भङ्ग;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  प्रत्यागम fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

स्थिर शून्य
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  पूर्णाङ्क i;

  यदि (s->type != FFI_TYPE_STRUCT)
    प्रत्यागम;

  s->size = 0;
  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type *p;
      पूर्णाङ्क align;
      
      p = s->elements[i];
      यदि (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#यदि defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#अन्यथा
      /* Natural alignment for the first item... */
      यदि (i == 0)
	align = p->alignment;
      अन्यथा यदि (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      अन्यथा
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  यदि (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

स्थिर पूर्णाङ्क
aix_adjust_aggregate_sizes (ffi_type *s, पूर्णाङ्क outer_most_type_or_first_member)
{
  पूर्णाङ्क i, nested_first_member=0, final_align, rc=0;

  यदि (s->type != FFI_TYPE_STRUCT)
    प्रत्यागम 0;

  s->size = 0;
  प्रति (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type p;
      पूर्णाङ्क align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      यदि (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      अन्यथा
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      यदि (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  यदि ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      यदि (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  प्रत्यागम rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_गिनती = 0, intarg_गिनती = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  यदि (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      प्रति (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  यदि (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      प्रति (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * परिमाण(unsigned long);

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
  विकल्प (cif->rtype->type)
    {

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      भङ्ग;
#endif

    पक्ष FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    पक्ष FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      भङ्ग;

    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
#ifdef POWERPC64
    पक्ष FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
#यदि defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	यदि (darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    यदि (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    अन्यथा
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    यदि (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	अन्यथा /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_गिनती++;
	  }
      }
#नोचेद्यदि defined(DARWIN_PPC)
      यदि (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      अन्यथा /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_गिनती++;
	}
#अन्यथा /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_गिनती++;
#endif
      भङ्ग;
    पक्ष FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      भङ्ग;

    स्वभाव:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      भङ्ग;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_शब्दगण;
      विकल्प ((*ptr)->type)
	{
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	  fparg_गिनती++;
#यदि !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  यदि (fparg_गिनती > अंक_FPR_ARG_REGISTERS
	      && (intarg_गिनती & 0x01) != 0)
	    intarg_गिनती++;
#endif
	  भङ्ग;

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	पक्ष FFI_TYPE_LONGDOUBLE:
	  fparg_गिनती += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  यदि (fparg_गिनती >= अंक_FPR_ARG_REGISTERS)
#यदि defined (POWERPC64)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, 2);
#अन्यथा
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, 4);
#endif
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
#यदि defined(POWERPC64)
	  intarg_गिनती++;
#अन्यथा
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  यदि (intarg_गिनती == अंक_GPR_ARG_REGISTERS-1
	      || (intarg_गिनती >= अंक_GPR_ARG_REGISTERS 
	          && (intarg_गिनती & 0x01) != 0))
	    intarg_गिनती++;
	  intarg_गिनती += 2;
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#यदि defined(POWERPC_DARWIN64)
	  align_शब्दगण = (*ptr)->alignment >> 3;
	  यदि (align_शब्दगण)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, align_शब्दगण);
	  /* Base size of the struct.  */
	  intarg_गिनती += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  यदि (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_गिनती) ;
#अन्यथा
	  align_शब्दगण = (*ptr)->alignment >> 2;
	  यदि (align_शब्दगण)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, align_शब्दगण);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_गिनती += (size_al + 7) / 8;
#  अन्यथा
	  intarg_गिनती += (size_al + 3) / 4;
#  endif
#endif
	  भङ्ग;

	स्वभाव:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_गिनती++;
	  भङ्ग;
	}
    }

  यदि (fparg_गिनती != 0)
    flags |= FLAG_FP_ARGUMENTS;

#यदि defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  यदि (fparg_गिनती != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += अंक_FPR_ARG_REGISTERS * परिमाण(द्विगुण);
#अन्यथा
  /* Space for the FPR registers, if needed.  */
  यदि (fparg_गिनती != 0)
    bytes += अंक_FPR_ARG_REGISTERS * परिमाण(द्विगुण);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  यदि ((intarg_गिनती + fparg_गिनती) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_गिनती + fparg_गिनती) * परिमाण(long);
#अन्यथा
  यदि ((intarg_गिनती + 2 * fparg_गिनती) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_गिनती + 2 * fparg_गिनती) * परिमाण(long);
#endif
  अन्यथा
    bytes += अंक_GPR_ARG_REGISTERS * परिमाण(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य));

#यदि defined (FFI_GO_CLOSURES)
बाह्य शून्य ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य), शून्य *closure);
#endif

बाह्य शून्य ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य), ffi_type*);

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  यदि ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      भङ्ग;
    पक्ष FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

#यदि defined (FFI_GO_CLOSURES)
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	     शून्य *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  यदि ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}
#endif

स्थिर शून्य flush_icache(वर्ण *);
स्थिर शून्य flush_range(वर्ण *, पूर्णाङ्क);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

प्रकारनिर्देश संरचना aix_fd_struct {
  शून्य *code_pointer;
  शून्य *toc;
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
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp;
  संरचना ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  विकल्प (cif->abi)
    {
      पक्ष FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
#यदि defined(POWERPC_DARWIN64)
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
#अन्यथा
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

	भङ्ग;

    पक्ष FFI_AIX:

      tramp_aix = (संरचना ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(शून्य *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      भङ्ग;

    स्वभाव:
      प्रत्यागम FFI_BAD_ABI;
      भङ्ग;
    }
  प्रत्यागम FFI_OK;
}

#यदि defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  विकल्प (cif->abi)
    {
      पक्ष FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (शून्य *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        प्रत्यागम FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      स्वभाव:
        प्रत्यागम FFI_BAD_ABI;
        भङ्ग;
    }
  प्रत्यागम FFI_OK;
}
#endif

स्थिर शून्य
flush_icache(वर्ण *addr)
{
#ifndef _AIX
  __asm__ चञ्चल (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

स्थिर शून्य
flush_range(वर्ण * addr1, पूर्णाङ्क size)
{
#define MIN_पंक्ति_SIZE 32
  पूर्णाङ्क i;
  प्रति (i = 0; i < size; i += MIN_पंक्ति_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

प्रकारनिर्देश सङ्घ
{
  भिन्न f;
  द्विगुण d;
} ffi_dblfl;

पूर्णाङ्क
ffi_closure_helper_DARWIN (ffi_closure *, शून्य *,
			   unsigned long *, ffi_dblfl *);

#यदि defined (FFI_GO_CLOSURES)
पूर्णाङ्क
ffi_go_closure_helper_DARWIN (ffi_go_closure*, शून्य *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

स्थिर पूर्णाङ्क
ffi_closure_helper_common (ffi_cif* cif,
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_data, शून्य *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  प्रकारनिर्देश द्विगुण ldbits[2];

  सङ्घ ldu
  {
    ldbits lb;
    long द्विगुण ld;
  };

  शून्य **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + अंक_FPR_ARG_REGISTERS;
  unsigned         size_al;
  पूर्णाङ्क              struct_ret_by_मूल्य = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#यदि defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * परिमाण(शून्य *));

  यदि (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#यदि defined(POWERPC_DARWIN64)
      यदि (!darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (शून्य *) *pgr;
	  pgr++;
	}
      अन्यथा
	struct_ret_by_मूल्य = 1;
#नोचेद्यदि defined(DARWIN_PPC)
      यदि (cif->rtype->size > 4)
	{
	  rvalue = (शून्य *) *pgr;
	  pgr++;
	}
      अन्यथा
	struct_ret_by_मूल्य = 1;
#अन्यथा /* assume we return by ref.  */
      rvalue = (शून्य *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  यावत् (i < avn)
    {
      विकल्प (arg_types[i]->type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
#यदि  defined(POWERPC64)
	  avalue[i] = (वर्ण *) pgr + 7;
#अन्यथा
	  avalue[i] = (वर्ण *) pgr + 3;
#endif
	  pgr++;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
#यदि  defined(POWERPC64)
	  avalue[i] = (वर्ण *) pgr + 6;
#अन्यथा
	  avalue[i] = (वर्ण *) pgr + 2;
#endif
	  pgr++;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
#यदि  defined(POWERPC64)
	  avalue[i] = (वर्ण *) pgr + 4;
#अन्यथा
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#यदि defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((वर्ण *)pgr, arg_types[i]->alignment);
	  यदि (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((वर्ण *)pgr)+8-size_al;
	      यदि (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < अंक_FPR_ARG_REGISTERS)
		{
		  *(भिन्न *)pgr = (भिन्न) *(द्विगुण *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  अन्यथा 
	    {
	      यदि (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (वर्ण *)pgr,
						   (द्विगुण *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#अन्यथा
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  यदि (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  यदि defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  यदि (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (वर्ण*) pgr + 4 - size_al;
	  अन्यथा
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  भङ्ग;

	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
#यदि  defined(POWERPC64)
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  भङ्ग;
#अन्यथा
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  भङ्ग;
#endif

	पक्ष FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  यदि (pfr < end_pfr)
	    {
	      द्विगुण temp = pfr->d;
	      pfr->f = (भिन्न) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  यदि (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#अन्यथा
	  pgr += 2;
#endif
	  भङ्ग;

#यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	पक्ष FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  यदि (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  अन्यथा
	    {
	      यदि (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#अन्यथा  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  यदि (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  अन्यथा यदि (pfr + 1 == end_pfr)
	    {
	      सङ्घ ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, परिमाण(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, परिमाण(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  भङ्ग;
#endif
	स्वभाव:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      प्रत्यागम PPC_LD_NONE;
    पक्ष FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      यदि (struct_ret_by_मूल्य)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  प्रत्यागम PPC_LD_STRUCT;
	}
      प्रत्यागम PPC_LD_NONE;
    पक्ष FFI_TYPE_FLOAT:
      प्रत्यागम PPC_LD_F32;
    पक्ष FFI_TYPE_DOUBLE:
      प्रत्यागम PPC_LD_F64;
#यदि FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
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
    स्वभाव:
      abort();
    }
}

पूर्णाङ्क
ffi_closure_helper_DARWIN (ffi_closure *closure, शून्य *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  प्रत्यागम ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#यदि defined (FFI_GO_CLOSURES)
पूर्णाङ्क
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, शून्य *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  प्रत्यागम ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>


સંરચના ffi_aix_trampoline_struct {
    શૂન્ય * code_pointer;	/* Pointer to ffi_closure_ASM */
    શૂન્ય * toc;			/* TOC */
    શૂન્ય * static_chain;	/* Pointer to closure */
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
#નહીંતર
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

બાહ્ય શૂન્ય ffi_closure_ASM (શૂન્ય);

#જો defined (FFI_GO_CLOSURES)
બાહ્ય શૂન્ય ffi_go_closure_ASM (શૂન્ય);
#endif

ગણના {
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
ગણના {
  અંક_GPR_ARG_REGISTERS = 8,
  અંક_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

ગણના { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#જો defined(POWERPC_DARWIN64)
સ્થિર શૂન્ય
darwin64_pass_struct_by_કિંમત 
  (ffi_type *, અક્ષર *, unsigned, unsigned *, દ્વિગુણ **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

શૂન્ય
ffi_prep_args (extended_cif *ecif, unsigned long *અચલ ઢગલો)
{
  અચલ unsigned bytes = ecif->cif->bytes;
  અચલ unsigned flags = ecif->cif->flags;
  અચલ unsigned nargs = ecif->cif->nargs;
#જો !defined(POWERPC_DARWIN64) 
  અચલ ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *અચલ stacktop = ઢગલો + (bytes / કદ(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  દ્વિગુણ *fpr_base = (દ્વિગુણ *) (stacktop - ASM_NEEDS_REGISTERS) - અંક_FPR_ARG_REGISTERS;
  પૂર્ણાંક gp_ગણતરી = 0, fparg_ગણતરી = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = ઢગલો + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  પૂર્ણાંક i;
  દ્વિગુણ double_tmp;
  શૂન્ય **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#જો !defined(POWERPC_DARWIN64) 
  અક્ષર *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (અક્ષર *) ઢગલો & 0xF) == 0);
  FFI_ASSERT(((unsigned) (અક્ષર *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  જો (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (અક્ષર *) ecif->rvalue;

  /* Now for the arguments.  */
  માટે (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      પસંદગી ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	સ્થિતિ FFI_TYPE_FLOAT:
	  double_tmp = *(દશાંશ *) *p_argv;
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#જો defined(POWERPC_DARWIN)
	  *(દશાંશ *)next_arg = *(દશાંશ *) *p_argv;
#નહીંતર
	  *(દ્વિગુણ *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_ગણતરી++;
	  fparg_ગણતરી++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  double_tmp = *(દ્વિગુણ *) *p_argv;
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(દ્વિગુણ *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_ગણતરી++;
#નહીંતર
	  next_arg += 2;
	  gp_ગણતરી += 2;
#endif
	  fparg_ગણતરી++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  તોડો;

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
#  જો defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS)
	    *(long દ્વિગુણ *) fpr_base++ = *(long દ્વિગુણ *) *p_argv;
	  નહીંતર
	    *(long દ્વિગુણ *) next_arg = *(long દ્વિગુણ *) *p_argv;
	  next_arg += 2;
	  fparg_ગણતરી += 2;
#  નહીંતર
	  double_tmp = ((દ્વિગુણ *) *p_argv)[0];
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(દ્વિગુણ *) next_arg = double_tmp;
#    જો defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ગણતરી++;
#    નહીંતર
	  next_arg += 2;
	  gp_ગણતરી += 2;
#    endif
	  fparg_ગણતરી++;
	  double_tmp = ((દ્વિગુણ *) *p_argv)[1];
	  જો (fparg_ગણતરી < અંક_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(દ્વિગુણ *) next_arg = double_tmp;
#    જો defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ગણતરી++;
#    નહીંતર
	  next_arg += 2;
	  gp_ગણતરી += 2;
#    endif
	  fparg_ગણતરી++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  તોડો;
#endif
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  જાઓ putgpr;
#નહીંતર
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_ગણતરી += 2;
#endif
	  તોડો;
	સ્થિતિ FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_UINT8:
	  gprvalue = *(unsigned અક્ષર *) *p_argv;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_SINT8:
	  gprvalue = *(signed અક્ષર *) *p_argv;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  જાઓ putgpr;
	સ્થિતિ FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  જાઓ putgpr;

	સ્થિતિ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#જો defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((અક્ષર *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_કિંમત (*ptr, (અક્ષર *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned પૂર્ણાંક *) &fparg_ગણતરી,
					 &fpr_base, &next_arg);
#નહીંતર
	  dest_cpy = (અક્ષર *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  જો ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  જો defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((અક્ષર *) dest_cpy, (અક્ષર *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  નહીંતર
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  જો (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((અક્ષર *) dest_cpy, (અક્ષર *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	  gprvalue = *(signed પૂર્ણાંક *) *p_argv;
	  જાઓ putgpr;

	સ્થિતિ FFI_TYPE_UINT32:
	  gprvalue = *(unsigned પૂર્ણાંક *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_ગણતરી++;
	  તોડો;
	મૂળભૂત:
	  તોડો;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#જો defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
સ્થિર શૂન્ય
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  પૂર્ણાંક i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      ffi_type *p = s->elements[i];
      પસંદગી (p->type)
	{
	  સ્થિતિ FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    તોડો;
	  સ્થિતિ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    તોડો;
	  સ્થિતિ FFI_TYPE_DOUBLE:
	  સ્થિતિ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    તોડો;
	  મૂળભૂત:
	    તોડો;    
	}
    }
}

સ્થિર પૂર્ણાંક
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, અક્ષર *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      અક્ષર *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      જો (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      પસંદગી (p->type)
	{
	  સ્થિતિ FFI_TYPE_STRUCT:
	    જો (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      પાછા 1;
	    તોડો;
	  સ્થિતિ FFI_TYPE_LONGDOUBLE:
	    જો (*nfpr >= અંક_FPR_ARG_REGISTERS)
	      પાછા 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (*nfpr >= અંક_FPR_ARG_REGISTERS)
	      પાછા 1;
	    (*nfpr) += 1;
	    તોડો;
	  સ્થિતિ FFI_TYPE_FLOAT:
	    જો (*nfpr >= અંક_FPR_ARG_REGISTERS)
	      પાછા 1;
	    (*nfpr) += 1;
	    તોડો;
	  મૂળભૂત:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    જો ((unsigned long)item_base >= 8*8) 
	      પાછા 1;
	    તોડો;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  પાછા 0;
}

/* Can this struct be returned by value?  */
પૂર્ણાંક 
darwin64_struct_ret_by_કિંમત_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  જો (s->size > 168)
    પાછા 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  જો (nfp > 13)
    પાછા 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  જો (s->size <= 64)
    પાછા 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  જો (darwin64_struct_size_exceeds_gprs_p (s, ખાલી, &nfp))
    પાછા 0;
  
  પાછા 1;
}

શૂન્ય
darwin64_pass_struct_floats (ffi_type *s, અક્ષર *src, 
			     unsigned *nfpr, દ્વિગુણ **fprs)
{
  પૂર્ણાંક i;
  દ્વિગુણ *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      અક્ષર *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      જો (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      પસંદગી (p->type)
	{
	  સ્થિતિ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    તોડો;
	  સ્થિતિ FFI_TYPE_LONGDOUBLE:
	    જો (*nfpr < અંક_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(દ્વિગુણ *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (*nfpr < અંક_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(દ્વિગુણ *)item_base;
	    (*nfpr) += 1;
	    તોડો;
	  સ્થિતિ FFI_TYPE_FLOAT:
	    જો (*nfpr < અંક_FPR_ARG_REGISTERS)
	      *fpr_base++ = (દ્વિગુણ) *(દશાંશ *)item_base;
	    (*nfpr) += 1;
	    તોડો;
	  મૂળભૂત:
	    તોડો;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
સ્થિર શૂન્ય
darwin64_pass_struct_by_કિંમત (ffi_type *s, અક્ષર *src, unsigned size,
			       unsigned *nfpr, દ્વિગુણ **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  અક્ષર *dest_cpy = (અક્ષર *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  જો (!size)
    પાછા;

  /* First... special cases.  */
  જો (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((અક્ષર *) dest_cpy, src, size);
      next_arg++;
    }
  નહીંતર જો (size == 16)
    {
      memcpy ((અક્ષર *) dest_cpy, src, size);
      next_arg += 2;
    }
  નહીંતર
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((અક્ષર *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

દ્વિગુણ *
darwin64_struct_floats_to_mem (ffi_type *s, અક્ષર *dest, દ્વિગુણ *fprs, unsigned *nf)
{
  પૂર્ણાંક i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      અક્ષર *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      જો (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      પસંદગી (p->type)
	{
	  સ્થિતિ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    તોડો;
	  સ્થિતિ FFI_TYPE_LONGDOUBLE:
	    જો (*nf < અંક_FPR_ARG_REGISTERS)
	      {
		*(દ્વિગુણ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (*nf < અંક_FPR_ARG_REGISTERS)
	      {
		*(દ્વિગુણ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    તોડો;
	  સ્થિતિ FFI_TYPE_FLOAT:
	    જો (*nf < અંક_FPR_ARG_REGISTERS)
	      {
		*(દશાંશ *)item_base = (દશાંશ) *fprs++ ;
		(*nf) += 1;
	      }
	    તોડો;
	  મૂળભૂત:
	    તોડો;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  પાછા fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

સ્થિર શૂન્ય
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  પૂર્ણાંક i;

  જો (s->type != FFI_TYPE_STRUCT)
    પાછા;

  s->size = 0;
  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      ffi_type *p;
      પૂર્ણાંક align;
      
      p = s->elements[i];
      જો (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#જો defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#નહીંતર
      /* Natural alignment for the first item... */
      જો (i == 0)
	align = p->alignment;
      નહીંતર જો (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      નહીંતર
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  જો (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

સ્થિર પૂર્ણાંક
aix_adjust_aggregate_sizes (ffi_type *s, પૂર્ણાંક outer_most_type_or_first_member)
{
  પૂર્ણાંક i, nested_first_member=0, final_align, rc=0;

  જો (s->type != FFI_TYPE_STRUCT)
    પાછા 0;

  s->size = 0;
  માટે (i = 0; s->elements[i] != ખાલી; i++)
    {
      ffi_type p;
      પૂર્ણાંક align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      જો (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      નહીંતર
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      જો (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  જો ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      જો (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  પાછા rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_ગણતરી = 0, intarg_ગણતરી = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  જો (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      માટે (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  જો (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      માટે (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * કદ(unsigned long);

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
  પસંદગી (cif->rtype->type)
    {

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      તોડો;
#endif

    સ્થિતિ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    સ્થિતિ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      તોડો;

    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
#ifdef POWERPC64
    સ્થિતિ FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
#જો defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	જો (darwin64_struct_ret_by_કિંમત_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    જો (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    નહીંતર
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    જો (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	નહીંતર /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_ગણતરી++;
	  }
      }
#અથવાજો defined(DARWIN_PPC)
      જો (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      નહીંતર /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_ગણતરી++;
	}
#નહીંતર /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_ગણતરી++;
#endif
      તોડો;
    સ્થિતિ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      તોડો;

    મૂળભૂત:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      તોડો;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  માટે (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_શબ્દો;
      પસંદગી ((*ptr)->type)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  fparg_ગણતરી++;
#જો !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  જો (fparg_ગણતરી > અંક_FPR_ARG_REGISTERS
	      && (intarg_ગણતરી & 0x01) != 0)
	    intarg_ગણતરી++;
#endif
	  તોડો;

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  fparg_ગણતરી += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  જો (fparg_ગણતરી >= અંક_FPR_ARG_REGISTERS)
#જો defined (POWERPC64)
	    intarg_ગણતરી = FFI_ALIGN(intarg_ગણતરી, 2);
#નહીંતર
	    intarg_ગણતરી = FFI_ALIGN(intarg_ગણતરી, 4);
#endif
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
#જો defined(POWERPC64)
	  intarg_ગણતરી++;
#નહીંતર
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  જો (intarg_ગણતરી == અંક_GPR_ARG_REGISTERS-1
	      || (intarg_ગણતરી >= અંક_GPR_ARG_REGISTERS 
	          && (intarg_ગણતરી & 0x01) != 0))
	    intarg_ગણતરી++;
	  intarg_ગણતરી += 2;
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#જો defined(POWERPC_DARWIN64)
	  align_શબ્દો = (*ptr)->alignment >> 3;
	  જો (align_શબ્દો)
	    intarg_ગણતરી = FFI_ALIGN(intarg_ગણતરી, align_શબ્દો);
	  /* Base size of the struct.  */
	  intarg_ગણતરી += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  જો (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_ગણતરી) ;
#નહીંતર
	  align_શબ્દો = (*ptr)->alignment >> 2;
	  જો (align_શબ્દો)
	    intarg_ગણતરી = FFI_ALIGN(intarg_ગણતરી, align_શબ્દો);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_ગણતરી += (size_al + 7) / 8;
#  નહીંતર
	  intarg_ગણતરી += (size_al + 3) / 4;
#  endif
#endif
	  તોડો;

	મૂળભૂત:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ગણતરી++;
	  તોડો;
	}
    }

  જો (fparg_ગણતરી != 0)
    flags |= FLAG_FP_ARGUMENTS;

#જો defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  જો (fparg_ગણતરી != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += અંક_FPR_ARG_REGISTERS * કદ(દ્વિગુણ);
#નહીંતર
  /* Space for the FPR registers, if needed.  */
  જો (fparg_ગણતરી != 0)
    bytes += અંક_FPR_ARG_REGISTERS * કદ(દ્વિગુણ);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  જો ((intarg_ગણતરી + fparg_ગણતરી) > અંક_GPR_ARG_REGISTERS)
    bytes += (intarg_ગણતરી + fparg_ગણતરી) * કદ(long);
#નહીંતર
  જો ((intarg_ગણતરી + 2 * fparg_ગણતરી) > અંક_GPR_ARG_REGISTERS)
    bytes += (intarg_ગણતરી + 2 * fparg_ગણતરી) * કદ(long);
#endif
  નહીંતર
    bytes += અંક_GPR_ARG_REGISTERS * કદ(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 શૂન્ય (*fn)(શૂન્ય), શૂન્ય (*fn2)(શૂન્ય));

#જો defined (FFI_GO_CLOSURES)
બાહ્ય શૂન્ય ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    શૂન્ય (*fn)(શૂન્ય), શૂન્ય (*fn2)(શૂન્ય), શૂન્ય *closure);
#endif

બાહ્ય શૂન્ય ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    શૂન્ય (*fn)(શૂન્ય), શૂન્ય (*fn2)(શૂન્ય), ffi_type*);

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  જો ((rvalue == ખાલી) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      તોડો;
    સ્થિતિ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}

#જો defined (FFI_GO_CLOSURES)
શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn) (શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue,
	     શૂન્ય *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  જો ((rvalue == ખાલી) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}
#endif

સ્થિર શૂન્ય flush_icache(અક્ષર *);
સ્થિર શૂન્ય flush_range(અક્ષર *, પૂર્ણાંક);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

પ્રકારવ્યાખ્યા સંરચના aix_fd_struct {
  શૂન્ય *code_pointer;
  શૂન્ય *toc;
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
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp;
  સંરચના ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  પસંદગી (cif->abi)
    {
      સ્થિતિ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
#જો defined(POWERPC_DARWIN64)
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
#નહીંતર
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

	તોડો;

    સ્થિતિ FFI_AIX:

      tramp_aix = (સંરચના ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(શૂન્ય *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      તોડો;

    મૂળભૂત:
      પાછા FFI_BAD_ABI;
      તોડો;
    }
  પાછા FFI_OK;
}

#જો defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
  પસંદગી (cif->abi)
    {
      સ્થિતિ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (શૂન્ય *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        પાછા FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      મૂળભૂત:
        પાછા FFI_BAD_ABI;
        તોડો;
    }
  પાછા FFI_OK;
}
#endif

સ્થિર શૂન્ય
flush_icache(અક્ષર *addr)
{
#ifndef _AIX
  __asm__ અસ્થિર (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

સ્થિર શૂન્ય
flush_range(અક્ષર * addr1, પૂર્ણાંક size)
{
#define MIN_લીટી_SIZE 32
  પૂર્ણાંક i;
  માટે (i = 0; i < size; i += MIN_લીટી_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

પ્રકારવ્યાખ્યા સંઘ
{
  દશાંશ f;
  દ્વિગુણ d;
} ffi_dblfl;

પૂર્ણાંક
ffi_closure_helper_DARWIN (ffi_closure *, શૂન્ય *,
			   unsigned long *, ffi_dblfl *);

#જો defined (FFI_GO_CLOSURES)
પૂર્ણાંક
ffi_go_closure_helper_DARWIN (ffi_go_closure*, શૂન્ય *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

સ્થિર પૂર્ણાંક
ffi_closure_helper_common (ffi_cif* cif,
			   શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			   શૂન્ય *user_data, શૂન્ય *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  પ્રકારવ્યાખ્યા દ્વિગુણ ldbits[2];

  સંઘ ldu
  {
    ldbits lb;
    long દ્વિગુણ ld;
  };

  શૂન્ય **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + અંક_FPR_ARG_REGISTERS;
  unsigned         size_al;
  પૂર્ણાંક              struct_ret_by_કિંમત = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#જો defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * કદ(શૂન્ય *));

  જો (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#જો defined(POWERPC_DARWIN64)
      જો (!darwin64_struct_ret_by_કિંમત_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (શૂન્ય *) *pgr;
	  pgr++;
	}
      નહીંતર
	struct_ret_by_કિંમત = 1;
#અથવાજો defined(DARWIN_PPC)
      જો (cif->rtype->size > 4)
	{
	  rvalue = (શૂન્ય *) *pgr;
	  pgr++;
	}
      નહીંતર
	struct_ret_by_કિંમત = 1;
#નહીંતર /* assume we return by ref.  */
      rvalue = (શૂન્ય *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  જ્યારે (i < avn)
    {
      પસંદગી (arg_types[i]->type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
#જો  defined(POWERPC64)
	  avalue[i] = (અક્ષર *) pgr + 7;
#નહીંતર
	  avalue[i] = (અક્ષર *) pgr + 3;
#endif
	  pgr++;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
#જો  defined(POWERPC64)
	  avalue[i] = (અક્ષર *) pgr + 6;
#નહીંતર
	  avalue[i] = (અક્ષર *) pgr + 2;
#endif
	  pgr++;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
#જો  defined(POWERPC64)
	  avalue[i] = (અક્ષર *) pgr + 4;
#નહીંતર
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#જો defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((અક્ષર *)pgr, arg_types[i]->alignment);
	  જો (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((અક્ષર *)pgr)+8-size_al;
	      જો (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < અંક_FPR_ARG_REGISTERS)
		{
		  *(દશાંશ *)pgr = (દશાંશ) *(દ્વિગુણ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  નહીંતર 
	    {
	      જો (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (અક્ષર *)pgr,
						   (દ્વિગુણ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#નહીંતર
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  જો (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  જો defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  નહીંતર
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  જો (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (અક્ષર*) pgr + 4 - size_al;
	  નહીંતર
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
#જો  defined(POWERPC64)
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  તોડો;
#નહીંતર
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  તોડો;
#endif

	સ્થિતિ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  જો (pfr < end_pfr)
	    {
	      દ્વિગુણ temp = pfr->d;
	      pfr->f = (દશાંશ) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  નહીંતર
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  જો (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  નહીંતર
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#નહીંતર
	  pgr += 2;
#endif
	  તોડો;

#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  જો (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  નહીંતર
	    {
	      જો (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#નહીંતર  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  જો (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  નહીંતર જો (pfr + 1 == end_pfr)
	    {
	      સંઘ ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, કદ(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, કદ(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  નહીંતર
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  તોડો;
#endif
	મૂળભૂત:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      પાછા PPC_LD_NONE;
    સ્થિતિ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      જો (struct_ret_by_કિંમત)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  પાછા PPC_LD_STRUCT;
	}
      પાછા PPC_LD_NONE;
    સ્થિતિ FFI_TYPE_FLOAT:
      પાછા PPC_LD_F32;
    સ્થિતિ FFI_TYPE_DOUBLE:
      પાછા PPC_LD_F64;
#જો FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      પાછા PPC_LD_F128;
#endif
    સ્થિતિ FFI_TYPE_UINT8:
      પાછા PPC_LD_U8;
    સ્થિતિ FFI_TYPE_SINT8:
      પાછા PPC_LD_S8;
    સ્થિતિ FFI_TYPE_UINT16:
      પાછા PPC_LD_U16;
    સ્થિતિ FFI_TYPE_SINT16:
      પાછા PPC_LD_S16;
    સ્થિતિ FFI_TYPE_UINT32:
      પાછા PPC_LD_U32;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
      પાછા PPC_LD_S32;
    સ્થિતિ FFI_TYPE_POINTER:
      પાછા PPC_LD_PTR;
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
      પાછા PPC_LD_I64;
    મૂળભૂત:
      abort();
    }
}

પૂર્ણાંક
ffi_closure_helper_DARWIN (ffi_closure *closure, શૂન્ય *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  પાછા ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#જો defined (FFI_GO_CLOSURES)
પૂર્ણાંક
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, શૂન્ય *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  પાછા ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

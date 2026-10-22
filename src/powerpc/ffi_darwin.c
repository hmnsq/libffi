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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>


ഘടന ffi_aix_trampoline_struct {
    ശൂന്യം * code_pointer;	/* Pointer to ffi_closure_ASM */
    ശൂന്യം * toc;			/* TOC */
    ശൂന്യം * static_chain;	/* Pointer to closure */
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
#അല്ലെങ്കിൽ
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

ബാഹ്യ ശൂന്യം ffi_closure_ASM (ശൂന്യം);

#എങ്കിൽ defined (FFI_GO_CLOSURES)
ബാഹ്യ ശൂന്യം ffi_go_closure_ASM (ശൂന്യം);
#endif

എണ്ണൽ {
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
എണ്ണൽ {
  അക്കം_GPR_ARG_REGISTERS = 8,
  അക്കം_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

എണ്ണൽ { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#എങ്കിൽ defined(POWERPC_DARWIN64)
സ്ഥിതം ശൂന്യം
darwin64_pass_struct_by_മൂല്യം 
  (ffi_type *, അക്ഷരം *, unsigned, unsigned *, ഇരട്ട **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

ശൂന്യം
ffi_prep_args (extended_cif *ecif, unsigned long *സ്ഥിരം അട്ടി)
{
  സ്ഥിരം unsigned bytes = ecif->cif->bytes;
  സ്ഥിരം unsigned flags = ecif->cif->flags;
  സ്ഥിരം unsigned nargs = ecif->cif->nargs;
#എങ്കിൽ !defined(POWERPC_DARWIN64) 
  സ്ഥിരം ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *സ്ഥിരം stacktop = അട്ടി + (bytes / വലുപ്പം(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ഇരട്ട *fpr_base = (ഇരട്ട *) (stacktop - ASM_NEEDS_REGISTERS) - അക്കം_FPR_ARG_REGISTERS;
  പൂർണ്ണസംഖ്യ gp_എണ്ണം = 0, fparg_എണ്ണം = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = അട്ടി + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  പൂർണ്ണസംഖ്യ i;
  ഇരട്ട double_tmp;
  ശൂന്യം **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#എങ്കിൽ !defined(POWERPC_DARWIN64) 
  അക്ഷരം *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (അക്ഷരം *) അട്ടി & 0xF) == 0);
  FFI_ASSERT(((unsigned) (അക്ഷരം *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  എങ്കിൽ (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (അക്ഷരം *) ecif->rvalue;

  /* Now for the arguments.  */
  വേണ്ടി (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      തിരഞ്ഞെടുപ്പ് ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	അവസ്ഥ FFI_TYPE_FLOAT:
	  double_tmp = *(ദശാംശം *) *p_argv;
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#എങ്കിൽ defined(POWERPC_DARWIN)
	  *(ദശാംശം *)next_arg = *(ദശാംശം *) *p_argv;
#അല്ലെങ്കിൽ
	  *(ഇരട്ട *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_എണ്ണം++;
	  fparg_എണ്ണം++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  double_tmp = *(ഇരട്ട *) *p_argv;
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ഇരട്ട *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_എണ്ണം++;
#അല്ലെങ്കിൽ
	  next_arg += 2;
	  gp_എണ്ണം += 2;
#endif
	  fparg_എണ്ണം++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#  എങ്കിൽ defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS)
	    *(long ഇരട്ട *) fpr_base++ = *(long ഇരട്ട *) *p_argv;
	  അല്ലെങ്കിൽ
	    *(long ഇരട്ട *) next_arg = *(long ഇരട്ട *) *p_argv;
	  next_arg += 2;
	  fparg_എണ്ണം += 2;
#  അല്ലെങ്കിൽ
	  double_tmp = ((ഇരട്ട *) *p_argv)[0];
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ഇരട്ട *) next_arg = double_tmp;
#    എങ്കിൽ defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_എണ്ണം++;
#    അല്ലെങ്കിൽ
	  next_arg += 2;
	  gp_എണ്ണം += 2;
#    endif
	  fparg_എണ്ണം++;
	  double_tmp = ((ഇരട്ട *) *p_argv)[1];
	  എങ്കിൽ (fparg_എണ്ണം < അക്കം_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ഇരട്ട *) next_arg = double_tmp;
#    എങ്കിൽ defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_എണ്ണം++;
#    അല്ലെങ്കിൽ
	  next_arg += 2;
	  gp_എണ്ണം += 2;
#    endif
	  fparg_എണ്ണം++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  നിർത്തുക;
#endif
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  പോകുക putgpr;
#അല്ലെങ്കിൽ
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_എണ്ണം += 2;
#endif
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_UINT8:
	  gprvalue = *(unsigned അക്ഷരം *) *p_argv;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT8:
	  gprvalue = *(signed അക്ഷരം *) *p_argv;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  പോകുക putgpr;
	അവസ്ഥ FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  പോകുക putgpr;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#എങ്കിൽ defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((അക്ഷരം *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_മൂല്യം (*ptr, (അക്ഷരം *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned പൂർണ്ണസംഖ്യ *) &fparg_എണ്ണം,
					 &fpr_base, &next_arg);
#അല്ലെങ്കിൽ
	  dest_cpy = (അക്ഷരം *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  എങ്കിൽ ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  എങ്കിൽ defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((അക്ഷരം *) dest_cpy, (അക്ഷരം *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  അല്ലെങ്കിൽ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  എങ്കിൽ (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((അക്ഷരം *) dest_cpy, (അക്ഷരം *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT32:
	  gprvalue = *(signed പൂർണ്ണസംഖ്യ *) *p_argv;
	  പോകുക putgpr;

	അവസ്ഥ FFI_TYPE_UINT32:
	  gprvalue = *(unsigned പൂർണ്ണസംഖ്യ *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_എണ്ണം++;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  നിർത്തുക;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#എങ്കിൽ defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
സ്ഥിതം ശൂന്യം
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  പൂർണ്ണസംഖ്യ i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      ffi_type *p = s->elements[i];
      തിരഞ്ഞെടുപ്പ് (p->type)
	{
	  അവസ്ഥ FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	  അവസ്ഥ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    നിർത്തുക;
	  സ്ഥിരസ്ഥിതി:
	    നിർത്തുക;    
	}
    }
}

സ്ഥിതം പൂർണ്ണസംഖ്യ
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, അക്ഷരം *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      അക്ഷരം *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      എങ്കിൽ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      തിരഞ്ഞെടുപ്പ് (p->type)
	{
	  അവസ്ഥ FFI_TYPE_STRUCT:
	    എങ്കിൽ (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      തിരികെ 1;
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	    എങ്കിൽ (*nfpr >= അക്കം_FPR_ARG_REGISTERS)
	      തിരികെ 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (*nfpr >= അക്കം_FPR_ARG_REGISTERS)
	      തിരികെ 1;
	    (*nfpr) += 1;
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_FLOAT:
	    എങ്കിൽ (*nfpr >= അക്കം_FPR_ARG_REGISTERS)
	      തിരികെ 1;
	    (*nfpr) += 1;
	    നിർത്തുക;
	  സ്ഥിരസ്ഥിതി:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    എങ്കിൽ ((unsigned long)item_base >= 8*8) 
	      തിരികെ 1;
	    നിർത്തുക;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  തിരികെ 0;
}

/* Can this struct be returned by value?  */
പൂർണ്ണസംഖ്യ 
darwin64_struct_ret_by_മൂല്യം_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  എങ്കിൽ (s->size > 168)
    തിരികെ 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  എങ്കിൽ (nfp > 13)
    തിരികെ 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  എങ്കിൽ (s->size <= 64)
    തിരികെ 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  എങ്കിൽ (darwin64_struct_size_exceeds_gprs_p (s, ശൂന്യ, &nfp))
    തിരികെ 0;
  
  തിരികെ 1;
}

ശൂന്യം
darwin64_pass_struct_floats (ffi_type *s, അക്ഷരം *src, 
			     unsigned *nfpr, ഇരട്ട **fprs)
{
  പൂർണ്ണസംഖ്യ i;
  ഇരട്ട *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      അക്ഷരം *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      എങ്കിൽ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      തിരഞ്ഞെടുപ്പ് (p->type)
	{
	  അവസ്ഥ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	    എങ്കിൽ (*nfpr < അക്കം_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ഇരട്ട *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (*nfpr < അക്കം_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ഇരട്ട *)item_base;
	    (*nfpr) += 1;
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_FLOAT:
	    എങ്കിൽ (*nfpr < അക്കം_FPR_ARG_REGISTERS)
	      *fpr_base++ = (ഇരട്ട) *(ദശാംശം *)item_base;
	    (*nfpr) += 1;
	    നിർത്തുക;
	  സ്ഥിരസ്ഥിതി:
	    നിർത്തുക;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
സ്ഥിതം ശൂന്യം
darwin64_pass_struct_by_മൂല്യം (ffi_type *s, അക്ഷരം *src, unsigned size,
			       unsigned *nfpr, ഇരട്ട **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  അക്ഷരം *dest_cpy = (അക്ഷരം *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  എങ്കിൽ (!size)
    തിരികെ;

  /* First... special cases.  */
  എങ്കിൽ (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((അക്ഷരം *) dest_cpy, src, size);
      next_arg++;
    }
  അല്ലെങ്കിൽ എങ്കിൽ (size == 16)
    {
      memcpy ((അക്ഷരം *) dest_cpy, src, size);
      next_arg += 2;
    }
  അല്ലെങ്കിൽ
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((അക്ഷരം *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

ഇരട്ട *
darwin64_struct_floats_to_mem (ffi_type *s, അക്ഷരം *dest, ഇരട്ട *fprs, unsigned *nf)
{
  പൂർണ്ണസംഖ്യ i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      അക്ഷരം *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      എങ്കിൽ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      തിരഞ്ഞെടുപ്പ് (p->type)
	{
	  അവസ്ഥ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	    എങ്കിൽ (*nf < അക്കം_FPR_ARG_REGISTERS)
	      {
		*(ഇരട്ട *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (*nf < അക്കം_FPR_ARG_REGISTERS)
	      {
		*(ഇരട്ട *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    നിർത്തുക;
	  അവസ്ഥ FFI_TYPE_FLOAT:
	    എങ്കിൽ (*nf < അക്കം_FPR_ARG_REGISTERS)
	      {
		*(ദശാംശം *)item_base = (ദശാംശം) *fprs++ ;
		(*nf) += 1;
	      }
	    നിർത്തുക;
	  സ്ഥിരസ്ഥിതി:
	    നിർത്തുക;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  തിരികെ fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

സ്ഥിതം ശൂന്യം
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  പൂർണ്ണസംഖ്യ i;

  എങ്കിൽ (s->type != FFI_TYPE_STRUCT)
    തിരികെ;

  s->size = 0;
  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      ffi_type *p;
      പൂർണ്ണസംഖ്യ align;
      
      p = s->elements[i];
      എങ്കിൽ (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#എങ്കിൽ defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#അല്ലെങ്കിൽ
      /* Natural alignment for the first item... */
      എങ്കിൽ (i == 0)
	align = p->alignment;
      അല്ലെങ്കിൽ എങ്കിൽ (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      അല്ലെങ്കിൽ
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  എങ്കിൽ (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

സ്ഥിതം പൂർണ്ണസംഖ്യ
aix_adjust_aggregate_sizes (ffi_type *s, പൂർണ്ണസംഖ്യ outer_most_type_or_first_member)
{
  പൂർണ്ണസംഖ്യ i, nested_first_member=0, final_align, rc=0;

  എങ്കിൽ (s->type != FFI_TYPE_STRUCT)
    തിരികെ 0;

  s->size = 0;
  വേണ്ടി (i = 0; s->elements[i] != ശൂന്യ; i++)
    {
      ffi_type p;
      പൂർണ്ണസംഖ്യ align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      എങ്കിൽ (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      അല്ലെങ്കിൽ
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      എങ്കിൽ (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  എങ്കിൽ ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      എങ്കിൽ (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  തിരികെ rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_എണ്ണം = 0, intarg_എണ്ണം = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  എങ്കിൽ (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      വേണ്ടി (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  എങ്കിൽ (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      വേണ്ടി (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * വലുപ്പം(unsigned long);

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
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      നിർത്തുക;
#endif

    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
#ifdef POWERPC64
    അവസ്ഥ FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
#എങ്കിൽ defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	എങ്കിൽ (darwin64_struct_ret_by_മൂല്യം_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    എങ്കിൽ (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    അല്ലെങ്കിൽ
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    എങ്കിൽ (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	അല്ലെങ്കിൽ /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_എണ്ണം++;
	  }
      }
#അല്ലെങ്കിലെങ്കിൽ defined(DARWIN_PPC)
      എങ്കിൽ (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      അല്ലെങ്കിൽ /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_എണ്ണം++;
	}
#അല്ലെങ്കിൽ /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_എണ്ണം++;
#endif
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      നിർത്തുക;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_വാക്കുകൾ;
      തിരഞ്ഞെടുപ്പ് ((*ptr)->type)
	{
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  fparg_എണ്ണം++;
#എങ്കിൽ !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  എങ്കിൽ (fparg_എണ്ണം > അക്കം_FPR_ARG_REGISTERS
	      && (intarg_എണ്ണം & 0x01) != 0)
	    intarg_എണ്ണം++;
#endif
	  നിർത്തുക;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  fparg_എണ്ണം += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  എങ്കിൽ (fparg_എണ്ണം >= അക്കം_FPR_ARG_REGISTERS)
#എങ്കിൽ defined (POWERPC64)
	    intarg_എണ്ണം = FFI_ALIGN(intarg_എണ്ണം, 2);
#അല്ലെങ്കിൽ
	    intarg_എണ്ണം = FFI_ALIGN(intarg_എണ്ണം, 4);
#endif
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
#എങ്കിൽ defined(POWERPC64)
	  intarg_എണ്ണം++;
#അല്ലെങ്കിൽ
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  എങ്കിൽ (intarg_എണ്ണം == അക്കം_GPR_ARG_REGISTERS-1
	      || (intarg_എണ്ണം >= അക്കം_GPR_ARG_REGISTERS 
	          && (intarg_എണ്ണം & 0x01) != 0))
	    intarg_എണ്ണം++;
	  intarg_എണ്ണം += 2;
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#എങ്കിൽ defined(POWERPC_DARWIN64)
	  align_വാക്കുകൾ = (*ptr)->alignment >> 3;
	  എങ്കിൽ (align_വാക്കുകൾ)
	    intarg_എണ്ണം = FFI_ALIGN(intarg_എണ്ണം, align_വാക്കുകൾ);
	  /* Base size of the struct.  */
	  intarg_എണ്ണം += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  എങ്കിൽ (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_എണ്ണം) ;
#അല്ലെങ്കിൽ
	  align_വാക്കുകൾ = (*ptr)->alignment >> 2;
	  എങ്കിൽ (align_വാക്കുകൾ)
	    intarg_എണ്ണം = FFI_ALIGN(intarg_എണ്ണം, align_വാക്കുകൾ);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_എണ്ണം += (size_al + 7) / 8;
#  അല്ലെങ്കിൽ
	  intarg_എണ്ണം += (size_al + 3) / 4;
#  endif
#endif
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_എണ്ണം++;
	  നിർത്തുക;
	}
    }

  എങ്കിൽ (fparg_എണ്ണം != 0)
    flags |= FLAG_FP_ARGUMENTS;

#എങ്കിൽ defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  എങ്കിൽ (fparg_എണ്ണം != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += അക്കം_FPR_ARG_REGISTERS * വലുപ്പം(ഇരട്ട);
#അല്ലെങ്കിൽ
  /* Space for the FPR registers, if needed.  */
  എങ്കിൽ (fparg_എണ്ണം != 0)
    bytes += അക്കം_FPR_ARG_REGISTERS * വലുപ്പം(ഇരട്ട);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  എങ്കിൽ ((intarg_എണ്ണം + fparg_എണ്ണം) > അക്കം_GPR_ARG_REGISTERS)
    bytes += (intarg_എണ്ണം + fparg_എണ്ണം) * വലുപ്പം(long);
#അല്ലെങ്കിൽ
  എങ്കിൽ ((intarg_എണ്ണം + 2 * fparg_എണ്ണം) > അക്കം_GPR_ARG_REGISTERS)
    bytes += (intarg_എണ്ണം + 2 * fparg_എണ്ണം) * വലുപ്പം(long);
#endif
  അല്ലെങ്കിൽ
    bytes += അക്കം_GPR_ARG_REGISTERS * വലുപ്പം(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം (*fn2)(ശൂന്യം));

#എങ്കിൽ defined (FFI_GO_CLOSURES)
ബാഹ്യ ശൂന്യം ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം (*fn2)(ശൂന്യം), ശൂന്യം *closure);
#endif

ബാഹ്യ ശൂന്യം ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം (*fn2)(ശൂന്യം), ffi_type*);

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  എങ്കിൽ ((rvalue == ശൂന്യ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      നിർത്തുക;
    അവസ്ഥ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}

#എങ്കിൽ defined (FFI_GO_CLOSURES)
ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn) (ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue,
	     ശൂന്യം *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  എങ്കിൽ ((rvalue == ശൂന്യ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}
#endif

സ്ഥിതം ശൂന്യം flush_icache(അക്ഷരം *);
സ്ഥിതം ശൂന്യം flush_range(അക്ഷരം *, പൂർണ്ണസംഖ്യ);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

തരനിർവചനം ഘടന aix_fd_struct {
  ശൂന്യം *code_pointer;
  ശൂന്യം *toc;
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
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp;
  ഘടന ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
      അവസ്ഥ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
#എങ്കിൽ defined(POWERPC_DARWIN64)
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
#അല്ലെങ്കിൽ
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

	നിർത്തുക;

    അവസ്ഥ FFI_AIX:

      tramp_aix = (ഘടന ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(ശൂന്യം *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_ABI;
      നിർത്തുക;
    }
  തിരികെ FFI_OK;
}

#എങ്കിൽ defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*))
{
  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
      അവസ്ഥ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (ശൂന്യം *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        തിരികെ FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      സ്ഥിരസ്ഥിതി:
        തിരികെ FFI_BAD_ABI;
        നിർത്തുക;
    }
  തിരികെ FFI_OK;
}
#endif

സ്ഥിതം ശൂന്യം
flush_icache(അക്ഷരം *addr)
{
#ifndef _AIX
  __asm__ അസ്ഥിര (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

സ്ഥിതം ശൂന്യം
flush_range(അക്ഷരം * addr1, പൂർണ്ണസംഖ്യ size)
{
#define MIN_വരി_SIZE 32
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി (i = 0; i < size; i += MIN_വരി_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

തരനിർവചനം സംഘം
{
  ദശാംശം f;
  ഇരട്ട d;
} ffi_dblfl;

പൂർണ്ണസംഖ്യ
ffi_closure_helper_DARWIN (ffi_closure *, ശൂന്യം *,
			   unsigned long *, ffi_dblfl *);

#എങ്കിൽ defined (FFI_GO_CLOSURES)
പൂർണ്ണസംഖ്യ
ffi_go_closure_helper_DARWIN (ffi_go_closure*, ശൂന്യം *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_closure_helper_common (ffi_cif* cif,
			   ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
			   ശൂന്യം *user_data, ശൂന്യം *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  തരനിർവചനം ഇരട്ട ldbits[2];

  സംഘം ldu
  {
    ldbits lb;
    long ഇരട്ട ld;
  };

  ശൂന്യം **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + അക്കം_FPR_ARG_REGISTERS;
  unsigned         size_al;
  പൂർണ്ണസംഖ്യ              struct_ret_by_മൂല്യം = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#എങ്കിൽ defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * വലുപ്പം(ശൂന്യം *));

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#എങ്കിൽ defined(POWERPC_DARWIN64)
      എങ്കിൽ (!darwin64_struct_ret_by_മൂല്യം_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (ശൂന്യം *) *pgr;
	  pgr++;
	}
      അല്ലെങ്കിൽ
	struct_ret_by_മൂല്യം = 1;
#അല്ലെങ്കിലെങ്കിൽ defined(DARWIN_PPC)
      എങ്കിൽ (cif->rtype->size > 4)
	{
	  rvalue = (ശൂന്യം *) *pgr;
	  pgr++;
	}
      അല്ലെങ്കിൽ
	struct_ret_by_മൂല്യം = 1;
#അല്ലെങ്കിൽ /* assume we return by ref.  */
      rvalue = (ശൂന്യം *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ആകുമ്പോൾ (i < avn)
    {
      തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
#എങ്കിൽ  defined(POWERPC64)
	  avalue[i] = (അക്ഷരം *) pgr + 7;
#അല്ലെങ്കിൽ
	  avalue[i] = (അക്ഷരം *) pgr + 3;
#endif
	  pgr++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
#എങ്കിൽ  defined(POWERPC64)
	  avalue[i] = (അക്ഷരം *) pgr + 6;
#അല്ലെങ്കിൽ
	  avalue[i] = (അക്ഷരം *) pgr + 2;
#endif
	  pgr++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
#എങ്കിൽ  defined(POWERPC64)
	  avalue[i] = (അക്ഷരം *) pgr + 4;
#അല്ലെങ്കിൽ
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#എങ്കിൽ defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((അക്ഷരം *)pgr, arg_types[i]->alignment);
	  എങ്കിൽ (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((അക്ഷരം *)pgr)+8-size_al;
	      എങ്കിൽ (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < അക്കം_FPR_ARG_REGISTERS)
		{
		  *(ദശാംശം *)pgr = (ദശാംശം) *(ഇരട്ട *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  അല്ലെങ്കിൽ 
	    {
	      എങ്കിൽ (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (അക്ഷരം *)pgr,
						   (ഇരട്ട *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#അല്ലെങ്കിൽ
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  എങ്കിൽ (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  എങ്കിൽ defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  അല്ലെങ്കിൽ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  എങ്കിൽ (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (അക്ഷരം*) pgr + 4 - size_al;
	  അല്ലെങ്കിൽ
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
#എങ്കിൽ  defined(POWERPC64)
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  നിർത്തുക;
#അല്ലെങ്കിൽ
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  നിർത്തുക;
#endif

	അവസ്ഥ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  എങ്കിൽ (pfr < end_pfr)
	    {
	      ഇരട്ട temp = pfr->d;
	      pfr->f = (ദശാംശം) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  എങ്കിൽ (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#അല്ലെങ്കിൽ
	  pgr += 2;
#endif
	  നിർത്തുക;

#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  എങ്കിൽ (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  അല്ലെങ്കിൽ
	    {
	      എങ്കിൽ (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#അല്ലെങ്കിൽ  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  എങ്കിൽ (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  അല്ലെങ്കിൽ എങ്കിൽ (pfr + 1 == end_pfr)
	    {
	      സംഘം ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, വലുപ്പം(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, വലുപ്പം(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  അല്ലെങ്കിൽ
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  നിർത്തുക;
#endif
	സ്ഥിരസ്ഥിതി:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      തിരികെ PPC_LD_NONE;
    അവസ്ഥ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      എങ്കിൽ (struct_ret_by_മൂല്യം)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  തിരികെ PPC_LD_STRUCT;
	}
      തിരികെ PPC_LD_NONE;
    അവസ്ഥ FFI_TYPE_FLOAT:
      തിരികെ PPC_LD_F32;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      തിരികെ PPC_LD_F64;
#എങ്കിൽ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      തിരികെ PPC_LD_F128;
#endif
    അവസ്ഥ FFI_TYPE_UINT8:
      തിരികെ PPC_LD_U8;
    അവസ്ഥ FFI_TYPE_SINT8:
      തിരികെ PPC_LD_S8;
    അവസ്ഥ FFI_TYPE_UINT16:
      തിരികെ PPC_LD_U16;
    അവസ്ഥ FFI_TYPE_SINT16:
      തിരികെ PPC_LD_S16;
    അവസ്ഥ FFI_TYPE_UINT32:
      തിരികെ PPC_LD_U32;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      തിരികെ PPC_LD_S32;
    അവസ്ഥ FFI_TYPE_POINTER:
      തിരികെ PPC_LD_PTR;
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      തിരികെ PPC_LD_I64;
    സ്ഥിരസ്ഥിതി:
      abort();
    }
}

പൂർണ്ണസംഖ്യ
ffi_closure_helper_DARWIN (ffi_closure *closure, ശൂന്യം *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  തിരികെ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#എങ്കിൽ defined (FFI_GO_CLOSURES)
പൂർണ്ണസംഖ്യ
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, ശൂന്യം *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  തിരികെ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

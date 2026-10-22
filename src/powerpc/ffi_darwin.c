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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>


ಸಂರಚನಾ ffi_aix_trampoline_struct {
    ಶೂನ್ಯ * code_pointer;	/* Pointer to ffi_closure_ASM */
    ಶೂನ್ಯ * toc;			/* TOC */
    ಶೂನ್ಯ * static_chain;	/* Pointer to closure */
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
#ಅನ್ಯಥಾ
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#endif

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_ASM (ಶೂನ್ಯ);

#ಯದಿ defined (FFI_GO_CLOSURES)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_ASM (ಶೂನ್ಯ);
#endif

ಗಣನಾ {
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
ಗಣನಾ {
  ಅಂಕೆ_GPR_ARG_REGISTERS = 8,
  ಅಂಕೆ_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

ಗಣನಾ { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#ಯದಿ defined(POWERPC_DARWIN64)
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_pass_struct_by_ಮೌಲ್ಯ 
  (ffi_type *, ವರ್ಣ *, unsigned, unsigned *, ದ್ವಿಗುಣ **, unsigned long **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

ಶೂನ್ಯ
ffi_prep_args (extended_cif *ecif, unsigned long *ಧ್ರುವ ರಾಶಿ)
{
  ಧ್ರುವ unsigned bytes = ecif->cif->bytes;
  ಧ್ರುವ unsigned flags = ecif->cif->flags;
  ಧ್ರುವ unsigned nargs = ecif->cif->nargs;
#ಯದಿ !defined(POWERPC_DARWIN64) 
  ಧ್ರುವ ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  unsigned long *ಧ್ರುವ stacktop = ರಾಶಿ + (bytes / ಪರಿಮಾಣ(unsigned long));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ದ್ವಿಗುಣ *fpr_base = (ದ್ವಿಗುಣ *) (stacktop - ASM_NEEDS_REGISTERS) - ಅಂಕೆ_FPR_ARG_REGISTERS;
  ಪೂರ್ಣಾಙ್ಕ gp_ಎಣಿಕೆ = 0, fparg_ಎಣಿಕೆ = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  unsigned long *next_arg = ರಾಶಿ + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  ಪೂರ್ಣಾಙ್ಕ i;
  ದ್ವಿಗುಣ double_tmp;
  ಶೂನ್ಯ **p_argv = ecif->avalue;
  unsigned long gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#ಯದಿ !defined(POWERPC_DARWIN64) 
  ವರ್ಣ *dest_cpy;
#endif
  unsigned size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((unsigned) (ವರ್ಣ *) ರಾಶಿ & 0xF) == 0);
  FFI_ASSERT(((unsigned) (ವರ್ಣ *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  ಯದಿ (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (unsigned long) (ವರ್ಣ *) ecif->rvalue;

  /* Now for the arguments.  */
  ಪ್ರತಿ (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ವಿಕಲ್ಪ ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  double_tmp = *(ಭಿನ್ನ *) *p_argv;
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#ಯದಿ defined(POWERPC_DARWIN)
	  *(ಭಿನ್ನ *)next_arg = *(ಭಿನ್ನ *) *p_argv;
#ಅನ್ಯಥಾ
	  *(ದ್ವಿಗುಣ *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_ಎಣಿಕೆ++;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  double_tmp = *(ದ್ವಿಗುಣ *) *p_argv;
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ದ್ವಿಗುಣ *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_ಎಣಿಕೆ++;
#ಅನ್ಯಥಾ
	  next_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#endif
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ಭಙ್ಗ;

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#  ಯದಿ defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *(long ದ್ವಿಗುಣ *) fpr_base++ = *(long ದ್ವಿಗುಣ *) *p_argv;
	  ಅನ್ಯಥಾ
	    *(long ದ್ವಿಗುಣ *) next_arg = *(long ದ್ವಿಗುಣ *) *p_argv;
	  next_arg += 2;
	  fparg_ಎಣಿಕೆ += 2;
#  ಅನ್ಯಥಾ
	  double_tmp = ((ದ್ವಿಗುಣ *) *p_argv)[0];
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ದ್ವಿಗುಣ *) next_arg = double_tmp;
#    ಯದಿ defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ಎಣಿಕೆ++;
#    ಅನ್ಯಥಾ
	  next_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#    endif
	  fparg_ಎಣಿಕೆ++;
	  double_tmp = ((ದ್ವಿಗುಣ *) *p_argv)[1];
	  ಯದಿ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(ದ್ವಿಗುಣ *) next_arg = double_tmp;
#    ಯದಿ defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_ಎಣಿಕೆ++;
#    ಅನ್ಯಥಾ
	  next_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#    endif
	  fparg_ಎಣಿಕೆ++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ಭಙ್ಗ;
#endif
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(long long *) *p_argv;
	  ಗಚ್ಛ putgpr;
#ಅನ್ಯಥಾ
	  *(long long *) next_arg = *(long long *) *p_argv;
	  next_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#endif
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  gprvalue = *(unsigned long *) *p_argv;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  gprvalue = *(unsigned ವರ್ಣ *) *p_argv;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  gprvalue = *(signed ವರ್ಣ *) *p_argv;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  gprvalue = *(unsigned short *) *p_argv;
	  ಗಚ್ಛ putgpr;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  gprvalue = *(signed short *) *p_argv;
	  ಗಚ್ಛ putgpr;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#ಯದಿ defined(POWERPC_DARWIN64)
	  next_arg = (unsigned long *)FFI_ALIGN((ವರ್ಣ *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_ಮೌಲ್ಯ (*ptr, (ವರ್ಣ *) *p_argv, 
					 (unsigned) size_al,
					 (unsigned ಪೂರ್ಣಾಙ್ಕ *) &fparg_ಎಣಿಕೆ,
					 &fpr_base, &next_arg);
#ಅನ್ಯಥಾ
	  dest_cpy = (ವರ್ಣ *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  ಯದಿ ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  ಯದಿ defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  memcpy ((ವರ್ಣ *) dest_cpy, (ವರ್ಣ *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  ಅನ್ಯಥಾ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  ಯದಿ (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  memcpy((ವರ್ಣ *) dest_cpy, (ವರ್ಣ *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	  gprvalue = *(signed ಪೂರ್ಣಾಙ್ಕ *) *p_argv;
	  ಗಚ್ಛ putgpr;

	ಪಕ್ಷ FFI_TYPE_UINT32:
	  gprvalue = *(unsigned ಪೂರ್ಣಾಙ್ಕ *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_ಎಣಿಕೆ++;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  ಭಙ್ಗ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#ಯದಿ defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_scan_struct_for_floats (ffi_type *s, unsigned *nfpr)
{
  ಪೂರ್ಣಾಙ್ಕ i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ffi_type *p = s->elements[i];
      ವಿಕಲ್ಪ (p->type)
	{
	  ಪಕ್ಷ FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ಭಙ್ಗ;
	  ಸ್ವಭಾವ:
	    ಭಙ್ಗ;    
	}
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, ವರ್ಣ *src, unsigned *nfpr)
{
  unsigned struct_offset=0, i;

  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ವರ್ಣ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಯದಿ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      ವಿಕಲ್ಪ (p->type)
	{
	  ಪಕ್ಷ FFI_TYPE_STRUCT:
	    ಯದಿ (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      ಪ್ರತ್ಯಾಗಮ 1;
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	    ಯದಿ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಪ್ರತ್ಯಾಗಮ 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಪ್ರತ್ಯಾಗಮ 1;
	    (*nfpr) += 1;
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಯದಿ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಪ್ರತ್ಯಾಗಮ 1;
	    (*nfpr) += 1;
	    ಭಙ್ಗ;
	  ಸ್ವಭಾವ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    ಯದಿ ((unsigned long)item_base >= 8*8) 
	      ಪ್ರತ್ಯಾಗಮ 1;
	    ಭಙ್ಗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/* Can this struct be returned by value?  */
ಪೂರ್ಣಾಙ್ಕ 
darwin64_struct_ret_by_ಮೌಲ್ಯ_p (ffi_type *s)
{
  unsigned nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  ಯದಿ (s->size > 168)
    ಪ್ರತ್ಯಾಗಮ 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  ಯದಿ (nfp > 13)
    ಪ್ರತ್ಯಾಗಮ 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  ಯದಿ (s->size <= 64)
    ಪ್ರತ್ಯಾಗಮ 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  ಯದಿ (darwin64_struct_size_exceeds_gprs_p (s, ರಿಕ್ತ, &nfp))
    ಪ್ರತ್ಯಾಗಮ 0;
  
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಶೂನ್ಯ
darwin64_pass_struct_floats (ffi_type *s, ವರ್ಣ *src, 
			     unsigned *nfpr, ದ್ವಿಗುಣ **fprs)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  ದ್ವಿಗುಣ *fpr_base = *fprs;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ವರ್ಣ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಯದಿ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      ವಿಕಲ್ಪ (p->type)
	{
	  ಪಕ್ಷ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	    ಯದಿ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ದ್ವಿಗುಣ *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(ದ್ವಿಗುಣ *)item_base;
	    (*nfpr) += 1;
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಯದಿ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_base++ = (ದ್ವಿಗುಣ) *(ಭಿನ್ನ *)item_base;
	    (*nfpr) += 1;
	    ಭಙ್ಗ;
	  ಸ್ವಭಾವ:
	    ಭಙ್ಗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_pass_struct_by_ಮೌಲ್ಯ (ffi_type *s, ವರ್ಣ *src, unsigned size,
			       unsigned *nfpr, ದ್ವಿಗುಣ **fprs, unsigned long **arg)
{
  unsigned long *next_arg = *arg;
  ವರ್ಣ *dest_cpy = (ವರ್ಣ *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ಯದಿ (!size)
    ಪ್ರತ್ಯಾಗಮ;

  /* First... special cases.  */
  ಯದಿ (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      memcpy ((ವರ್ಣ *) dest_cpy, src, size);
      next_arg++;
    }
  ಅನ್ಯಥಾ ಯದಿ (size == 16)
    {
      memcpy ((ವರ್ಣ *) dest_cpy, src, size);
      next_arg += 2;
    }
  ಅನ್ಯಥಾ
    {
      /* now the general case, we consider embedded floats.  */
      memcpy ((ವರ್ಣ *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

ದ್ವಿಗುಣ *
darwin64_struct_floats_to_mem (ffi_type *s, ವರ್ಣ *dest, ದ್ವಿಗುಣ *fprs, unsigned *nf)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  unsigned struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ವರ್ಣ *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಯದಿ (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      ವಿಕಲ್ಪ (p->type)
	{
	  ಪಕ್ಷ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	    ಯದಿ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ದ್ವಿಗುಣ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ದ್ವಿಗುಣ *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    ಭಙ್ಗ;
	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಯದಿ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ಭಿನ್ನ *)item_base = (ಭಿನ್ನ) *fprs++ ;
		(*nf) += 1;
	      }
	    ಭಙ್ಗ;
	  ಸ್ವಭಾವ:
	    ಭಙ್ಗ;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  ಪ್ರತ್ಯಾಗಮ fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ಸ್ಥಿರ ಶೂನ್ಯ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  ಪೂರ್ಣಾಙ್ಕ i;

  ಯದಿ (s->type != FFI_TYPE_STRUCT)
    ಪ್ರತ್ಯಾಗಮ;

  s->size = 0;
  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ffi_type *p;
      ಪೂರ್ಣಾಙ್ಕ align;
      
      p = s->elements[i];
      ಯದಿ (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#ಯದಿ defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#ಅನ್ಯಥಾ
      /* Natural alignment for the first item... */
      ಯದಿ (i == 0)
	align = p->alignment;
      ಅನ್ಯಥಾ ಯದಿ (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      ಅನ್ಯಥಾ
	/* .. or align is 4.  */
	align = 4;
#endif
      /* Pad, if necessary, before adding the current item.  */
      s->size = FFI_ALIGN(s->size, align) + p->size;
    }
  
  s->size = FFI_ALIGN(s->size, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  ಯದಿ (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
aix_adjust_aggregate_sizes (ffi_type *s, ಪೂರ್ಣಾಙ್ಕ outer_most_type_or_first_member)
{
  ಪೂರ್ಣಾಙ್ಕ i, nested_first_member=0, final_align, rc=0;

  ಯದಿ (s->type != FFI_TYPE_STRUCT)
    ಪ್ರತ್ಯಾಗಮ 0;

  s->size = 0;
  ಪ್ರತಿ (i = 0; s->elements[i] != ರಿಕ್ತ; i++)
    {
      ffi_type p;
      ಪೂರ್ಣಾಙ್ಕ align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      ಯದಿ (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      ಅನ್ಯಥಾ
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      ಯದಿ (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  ಯದಿ ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      ಯದಿ (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  ಪ್ರತ್ಯಾಗಮ rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  unsigned i;
  ffi_type **ptr;
  unsigned bytes;
  unsigned fparg_ಎಣಿಕೆ = 0, intarg_ಎಣಿಕೆ = 0;
  unsigned flags = 0;
  unsigned size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  ಯದಿ (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  ಯದಿ (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * ಪರಿಮಾಣ(unsigned long);

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
  ವಿಕಲ್ಪ (cif->rtype->type)
    {

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      ಭಙ್ಗ;
#endif

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
#ifdef POWERPC64
    ಪಕ್ಷ FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
#ಯದಿ defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	ಯದಿ (darwin64_struct_ret_by_ಮೌಲ್ಯ_p (cif->rtype))
	  {
	    unsigned nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    ಯದಿ (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    ಅನ್ಯಥಾ
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    ಯದಿ (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	ಅನ್ಯಥಾ /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_ಎಣಿಕೆ++;
	  }
      }
#ನೋಚೇದ್ಯದಿ defined(DARWIN_PPC)
      ಯದಿ (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      ಅನ್ಯಥಾ /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_ಎಣಿಕೆ++;
	}
#ಅನ್ಯಥಾ /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_ಎಣಿಕೆ++;
#endif
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ಭಙ್ಗ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      unsigned align_ಪದಗಳು;
      ವಿಕಲ್ಪ ((*ptr)->type)
	{
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  fparg_ಎಣಿಕೆ++;
#ಯದಿ !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  ಯದಿ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS
	      && (intarg_ಎಣಿಕೆ & 0x01) != 0)
	    intarg_ಎಣಿಕೆ++;
#endif
	  ಭಙ್ಗ;

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  fparg_ಎಣಿಕೆ += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  ಯದಿ (fparg_ಎಣಿಕೆ >= ಅಂಕೆ_FPR_ARG_REGISTERS)
#ಯದಿ defined (POWERPC64)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, 2);
#ಅನ್ಯಥಾ
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, 4);
#endif
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
#ಯದಿ defined(POWERPC64)
	  intarg_ಎಣಿಕೆ++;
#ಅನ್ಯಥಾ
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  ಯದಿ (intarg_ಎಣಿಕೆ == ಅಂಕೆ_GPR_ARG_REGISTERS-1
	      || (intarg_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS 
	          && (intarg_ಎಣಿಕೆ & 0x01) != 0))
	    intarg_ಎಣಿಕೆ++;
	  intarg_ಎಣಿಕೆ += 2;
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#ಯದಿ defined(POWERPC_DARWIN64)
	  align_ಪದಗಳು = (*ptr)->alignment >> 3;
	  ಯದಿ (align_ಪದಗಳು)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, align_ಪದಗಳು);
	  /* Base size of the struct.  */
	  intarg_ಎಣಿಕೆ += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  ಯದಿ (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_ಎಣಿಕೆ) ;
#ಅನ್ಯಥಾ
	  align_ಪದಗಳು = (*ptr)->alignment >> 2;
	  ಯದಿ (align_ಪದಗಳು)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, align_ಪದಗಳು);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_ಎಣಿಕೆ += (size_al + 7) / 8;
#  ಅನ್ಯಥಾ
	  intarg_ಎಣಿಕೆ += (size_al + 3) / 4;
#  endif
#endif
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ಎಣಿಕೆ++;
	  ಭಙ್ಗ;
	}
    }

  ಯದಿ (fparg_ಎಣಿಕೆ != 0)
    flags |= FLAG_FP_ARGUMENTS;

#ಯದಿ defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  ಯದಿ (fparg_ಎಣಿಕೆ != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += ಅಂಕೆ_FPR_ARG_REGISTERS * ಪರಿಮಾಣ(ದ್ವಿಗುಣ);
#ಅನ್ಯಥಾ
  /* Space for the FPR registers, if needed.  */
  ಯದಿ (fparg_ಎಣಿಕೆ != 0)
    bytes += ಅಂಕೆ_FPR_ARG_REGISTERS * ಪರಿಮಾಣ(ದ್ವಿಗುಣ);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  ಯದಿ ((intarg_ಎಣಿಕೆ + fparg_ಎಣಿಕೆ) > ಅಂಕೆ_GPR_ARG_REGISTERS)
    bytes += (intarg_ಎಣಿಕೆ + fparg_ಎಣಿಕೆ) * ಪರಿಮಾಣ(long);
#ಅನ್ಯಥಾ
  ಯದಿ ((intarg_ಎಣಿಕೆ + 2 * fparg_ಎಣಿಕೆ) > ಅಂಕೆ_GPR_ARG_REGISTERS)
    bytes += (intarg_ಎಣಿಕೆ + 2 * fparg_ಎಣಿಕೆ) * ಪರಿಮಾಣ(long);
#endif
  ಅನ್ಯಥಾ
    bytes += ಅಂಕೆ_GPR_ARG_REGISTERS * ಪರಿಮಾಣ(long);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_AIX(extended_cif *, long, unsigned, unsigned *,
			 ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ));

#ಯದಿ defined (FFI_GO_CLOSURES)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_go_AIX(extended_cif *, long, unsigned, unsigned *,
			    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ), ಶೂನ್ಯ *closure);
#endif

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_DARWIN(extended_cif *, long, unsigned, unsigned *,
			    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ), ffi_type*);

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಯದಿ ((rvalue == ರಿಕ್ತ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_AIX:
      ffi_call_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}

#ಯದಿ defined (FFI_GO_CLOSURES)
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಯದಿ ((rvalue == ರಿಕ್ತ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(long)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ flush_icache(ವರ್ಣ *);
ಸ್ಥಿರ ಶೂನ್ಯ flush_range(ವರ್ಣ *, ಪೂರ್ಣಾಙ್ಕ);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ aix_fd_struct {
  ಶೂನ್ಯ *code_pointer;
  ಶೂನ್ಯ *toc;
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
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp;
  ಸಂರಚನಾ ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ವಿಕಲ್ಪ (cif->abi)
    {
      ಪಕ್ಷ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
#ಯದಿ defined(POWERPC_DARWIN64)
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
#ಅನ್ಯಥಾ
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

	ಭಙ್ಗ;

    ಪಕ್ಷ FFI_AIX:

      tramp_aix = (ಸಂರಚನಾ ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(ಶೂನ್ಯ *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
      ಭಙ್ಗ;
    }
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ಯದಿ defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ವಿಕಲ್ಪ (cif->abi)
    {
      ಪಕ್ಷ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (ಶೂನ್ಯ *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        ಪ್ರತ್ಯಾಗಮ FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      ಸ್ವಭಾವ:
        ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
        ಭಙ್ಗ;
    }
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ
flush_icache(ವರ್ಣ *addr)
{
#ifndef _AIX
  __asm__ ಚಞ್ಚಲ (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

ಸ್ಥಿರ ಶೂನ್ಯ
flush_range(ವರ್ಣ * addr1, ಪೂರ್ಣಾಙ್ಕ size)
{
#define MIN_ಸಾಲು_SIZE 32
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪ್ರತಿ (i = 0; i < size; i += MIN_ಸಾಲು_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಙ್ಘ
{
  ಭಿನ್ನ f;
  ದ್ವಿಗುಣ d;
} ffi_dblfl;

ಪೂರ್ಣಾಙ್ಕ
ffi_closure_helper_DARWIN (ffi_closure *, ಶೂನ್ಯ *,
			   unsigned long *, ffi_dblfl *);

#ಯದಿ defined (FFI_GO_CLOSURES)
ಪೂರ್ಣಾಙ್ಕ
ffi_go_closure_helper_DARWIN (ffi_go_closure*, ಶೂನ್ಯ *,
			      unsigned long *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_closure_helper_common (ffi_cif* cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_data, ಶೂನ್ಯ *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  ಪ್ರಕಾರನಿರ್ದೇಶ ದ್ವಿಗುಣ ldbits[2];

  ಸಙ್ಘ ldu
  {
    ldbits lb;
    long ದ್ವಿಗುಣ ld;
  };

  ಶೂನ್ಯ **          avalue;
  ffi_type **      arg_types;
  long             i, avn;
  ffi_dblfl *      end_pfr = pfr + ಅಂಕೆ_FPR_ARG_REGISTERS;
  unsigned         size_al;
  ಪೂರ್ಣಾಙ್ಕ              struct_ret_by_ಮೌಲ್ಯ = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  unsigned long *  pgr0 = pgr;
#ಯದಿ defined(POWERPC_DARWIN64)
  unsigned 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#ಯದಿ defined(POWERPC_DARWIN64)
      ಯದಿ (!darwin64_struct_ret_by_ಮೌಲ್ಯ_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (ಶೂನ್ಯ *) *pgr;
	  pgr++;
	}
      ಅನ್ಯಥಾ
	struct_ret_by_ಮೌಲ್ಯ = 1;
#ನೋಚೇದ್ಯದಿ defined(DARWIN_PPC)
      ಯದಿ (cif->rtype->size > 4)
	{
	  rvalue = (ಶೂನ್ಯ *) *pgr;
	  pgr++;
	}
      ಅನ್ಯಥಾ
	struct_ret_by_ಮೌಲ್ಯ = 1;
#ಅನ್ಯಥಾ /* assume we return by ref.  */
      rvalue = (ಶೂನ್ಯ *) *pgr;
      pgr++;
#endif
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಯಾವತ್ (i < avn)
    {
      ವಿಕಲ್ಪ (arg_types[i]->type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
#ಯದಿ  defined(POWERPC64)
	  avalue[i] = (ವರ್ಣ *) pgr + 7;
#ಅನ್ಯಥಾ
	  avalue[i] = (ವರ್ಣ *) pgr + 3;
#endif
	  pgr++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
#ಯದಿ  defined(POWERPC64)
	  avalue[i] = (ವರ್ಣ *) pgr + 6;
#ಅನ್ಯಥಾ
	  avalue[i] = (ವರ್ಣ *) pgr + 2;
#endif
	  pgr++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
#ಯದಿ  defined(POWERPC64)
	  avalue[i] = (ವರ್ಣ *) pgr + 4;
#ಅನ್ಯಥಾ
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#ಯದಿ defined(POWERPC_DARWIN64)
	  pgr = (unsigned long *)FFI_ALIGN((ವರ್ಣ *)pgr, arg_types[i]->alignment);
	  ಯದಿ (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((ವರ್ಣ *)pgr)+8-size_al;
	      ಯದಿ (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < ಅಂಕೆ_FPR_ARG_REGISTERS)
		{
		  *(ಭಿನ್ನ *)pgr = (ಭಿನ್ನ) *(ದ್ವಿಗುಣ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  ಅನ್ಯಥಾ 
	    {
	      ಯದಿ (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (ವರ್ಣ *)pgr,
						   (ದ್ವಿಗುಣ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#ಅನ್ಯಥಾ
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  ಯದಿ (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  ಯದಿ defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  ಅನ್ಯಥಾ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  ಯದಿ (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (ವರ್ಣ*) pgr + 4 - size_al;
	  ಅನ್ಯಥಾ
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
#ಯದಿ  defined(POWERPC64)
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  ಭಙ್ಗ;
#ಅನ್ಯಥಾ
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ಭಙ್ಗ;
#endif

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  ಯದಿ (pfr < end_pfr)
	    {
	      ದ್ವಿಗುಣ temp = pfr->d;
	      pfr->f = (ಭಿನ್ನ) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  ಯದಿ (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
	    {
	      avalue[i] = pgr;
	    }
#ifdef POWERPC64
	  pgr++;
#ಅನ್ಯಥಾ
	  pgr += 2;
#endif
	  ಭಙ್ಗ;

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  ಯದಿ (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  ಅನ್ಯಥಾ
	    {
	      ಯದಿ (pfr < end_pfr)
		{
		  *pgr = *(unsigned long *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#ಅನ್ಯಥಾ  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  ಯದಿ (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  ಅನ್ಯಥಾ ಯದಿ (pfr + 1 == end_pfr)
	    {
	      ಸಙ್ಘ ldu temp_ld;
	      memcpy (&temp_ld.lb[0], pfr, ಪರಿಮಾಣ(ldbits));
	      memcpy (&temp_ld.lb[1], pgr + 2, ಪರಿಮಾಣ(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  ಅನ್ಯಥಾ
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  ಭಙ್ಗ;
#endif
	ಸ್ವಭಾವ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_NONE;
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      ಯದಿ (struct_ret_by_ಮೌಲ್ಯ)
	{
	  *pgr0 = (unsigned long) cif->rtype;
	  ಪ್ರತ್ಯಾಗಮ PPC_LD_STRUCT;
	}
      ಪ್ರತ್ಯಾಗಮ PPC_LD_NONE;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F32;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F64;
#ಯದಿ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_F128;
#endif
    ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U8;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S8;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U16;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S16;
    ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_U32;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_S32;
    ಪಕ್ಷ FFI_TYPE_POINTER:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_PTR;
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
      ಪ್ರತ್ಯಾಗಮ PPC_LD_I64;
    ಸ್ವಭಾವ:
      abort();
    }
}

ಪೂರ್ಣಾಙ್ಕ
ffi_closure_helper_DARWIN (ffi_closure *closure, ಶೂನ್ಯ *rvalue,
			   unsigned long *pgr, ffi_dblfl *pfr)
{
  ಪ್ರತ್ಯಾಗಮ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#ಯದಿ defined (FFI_GO_CLOSURES)
ಪೂರ್ಣಾಙ್ಕ
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, ಶೂನ್ಯ *rvalue,
			      unsigned long *pgr, ffi_dblfl *pfr)
{
  ಪ್ರತ್ಯಾಗಮ ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

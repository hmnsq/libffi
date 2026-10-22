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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>


ਬਣਤਰ ffi_aix_trampoline_struct {
    ਖਾਲੀ * ਕੋਡ_ਸੂਚਕ;	/* Pointer to ffi_closure_ASM */
    ਖਾਲੀ * toc;			/* TOC */
    ਖਾਲੀ * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_NONE		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_R3		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_R3R4		2
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F32		3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F64		4
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_F128		5
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U8		6
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S8		7
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_U16		8
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PPC_LD_S16		9

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#ਨਹੀਂਤਾਂ
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_ASM (ਖਾਲੀ);

#ਜੇ defined (FFI_GO_CLOSURES)
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_ASM (ਖਾਲੀ);
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128_ਬਿੱਟ	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64_ਬਿੱਟ	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
ਗਿਣਤੀ {
  ਅੰਕ_GPR_ARG_REGISTERS = 8,
  ਅੰਕ_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

ਗਿਣਤੀ { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#ਜੇ defined(POWERPC_DARWIN64)
ਸਥਿਰ ਖਾਲੀ
darwin64_pass_struct_by_ਮੁੱਲ 
  (ffi_type *, ਅੱਖਰ *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *, ਦੁੱਗਣਾ **, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ **);
#ਜੇ_ਅੰਤ

/* This depends on GPR_SIZE = sizeof (unsigned long) */

ਖਾਲੀ
ffi_prep_ਦਲੀਲਾਂ (extended_cif *ecif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਚਲ ਢੇਰ)
{
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ = ecif->cif->ਬਾਈਟਾਂ;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ flags = ecif->cif->flags;
  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ nargs = ecif->cif->nargs;
#ਜੇ !defined(POWERPC_DARWIN64) 
  ਅਚਲ ffi_abi abi = ecif->cif->abi;
#ਜੇ_ਅੰਤ

  /* 'stacktop' points at the previous backchain pointer.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਚਲ stacktop = ਢੇਰ + (ਬਾਈਟਾਂ / ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ਦੁੱਗਣਾ *fpr_ਅਧਾਰ = (ਦੁੱਗਣਾ *) (stacktop - ASM_NEEDS_REGISTERS) - ਅੰਕ_FPR_ARG_REGISTERS;
  ਪੂਰਨ gp_ਗਿਣਤੀ = 0, fparg_ਗਿਣਤੀ = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਗਲਾ_arg = ਢੇਰ + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  ਪੂਰਨ i;
  ਦੁੱਗਣਾ double_tmp;
  ਖਾਲੀ **p_argv = ecif->avalue;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#ਜੇ !defined(POWERPC_DARWIN64) 
  ਅੱਖਰ *dest_cpy;
#ਜੇ_ਅੰਤ
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((ਬਿਨਾਨਿਸ਼ਾਨੀ) (ਅੱਖਰ *) ਢੇਰ & 0xF) == 0);
  FFI_ASSERT(((ਬਿਨਾਨਿਸ਼ਾਨੀ) (ਅੱਖਰ *) stacktop & 0xF) == 0);
  FFI_ASSERT((ਬਾਈਟਾਂ & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  ਜੇ (flags & FLAG_RETVAL_REFERENCE)
    *ਅਗਲਾ_arg++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) (ਅੱਖਰ *) ecif->rvalue;

  /* Now for the arguments.  */
  ਲਈ (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ਚੋਣ ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  double_tmp = *(ਦਸ਼ਮਲਵ *) *p_argv;
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS)
	    *fpr_ਅਧਾਰ++ = double_tmp;
#ਜੇ defined(POWERPC_DARWIN)
	  *(ਦਸ਼ਮਲਵ *)ਅਗਲਾ_arg = *(ਦਸ਼ਮਲਵ *) *p_argv;
#ਨਹੀਂਤਾਂ
	  *(ਦੁੱਗਣਾ *)ਅਗਲਾ_arg = double_tmp;
#ਜੇ_ਅੰਤ
	  ਅਗਲਾ_arg++;
	  gp_ਗਿਣਤੀ++;
	  fparg_ਗਿਣਤੀ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  double_tmp = *(ਦੁੱਗਣਾ *) *p_argv;
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS)
	    *fpr_ਅਧਾਰ++ = double_tmp;
	  *(ਦੁੱਗਣਾ *)ਅਗਲਾ_arg = double_tmp;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
	  ਅਗਲਾ_arg++;
	  gp_ਗਿਣਤੀ++;
#ਨਹੀਂਤਾਂ
	  ਅਗਲਾ_arg += 2;
	  gp_ਗਿਣਤੀ += 2;
#ਜੇ_ਅੰਤ
	  fparg_ਗਿਣਤੀ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#  ਜੇ defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS)
	    *(ਲੰਮਾ ਦੁੱਗਣਾ *) fpr_ਅਧਾਰ++ = *(ਲੰਮਾ ਦੁੱਗਣਾ *) *p_argv;
	  ਨਹੀਂਤਾਂ
	    *(ਲੰਮਾ ਦੁੱਗਣਾ *) ਅਗਲਾ_arg = *(ਲੰਮਾ ਦੁੱਗਣਾ *) *p_argv;
	  ਅਗਲਾ_arg += 2;
	  fparg_ਗਿਣਤੀ += 2;
#  ਨਹੀਂਤਾਂ
	  double_tmp = ((ਦੁੱਗਣਾ *) *p_argv)[0];
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS)
	    *fpr_ਅਧਾਰ++ = double_tmp;
	  *(ਦੁੱਗਣਾ *) ਅਗਲਾ_arg = double_tmp;
#    ਜੇ defined(POWERPC_DARWIN64)
	  ਅਗਲਾ_arg++;
	  gp_ਗਿਣਤੀ++;
#    ਨਹੀਂਤਾਂ
	  ਅਗਲਾ_arg += 2;
	  gp_ਗਿਣਤੀ += 2;
#    endif
	  fparg_ਗਿਣਤੀ++;
	  double_tmp = ((ਦੁੱਗਣਾ *) *p_argv)[1];
	  ਜੇ (fparg_ਗਿਣਤੀ < ਅੰਕ_FPR_ARG_REGISTERS)
	    *fpr_ਅਧਾਰ++ = double_tmp;
	  *(ਦੁੱਗਣਾ *) ਅਗਲਾ_arg = double_tmp;
#    ਜੇ defined(POWERPC_DARWIN64)
	  ਅਗਲਾ_arg++;
	  gp_ਗਿਣਤੀ++;
#    ਨਹੀਂਤਾਂ
	  ਅਗਲਾ_arg += 2;
	  gp_ਗਿਣਤੀ += 2;
#    endif
	  fparg_ਗਿਣਤੀ++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
	  gprvalue = *(ਲੰਮਾ ਲੰਮਾ *) *p_argv;
	  ਜਾਓ putgpr;
#ਨਹੀਂਤਾਂ
	  *(ਲੰਮਾ ਲੰਮਾ *) ਅਗਲਾ_arg = *(ਲੰਮਾ ਲੰਮਾ *) *p_argv;
	  ਅਗਲਾ_arg += 2;
	  gp_ਗਿਣਤੀ += 2;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) *p_argv;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_UINT8:
	  gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *) *p_argv;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT8:
	  gprvalue = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ *) *p_argv;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) *p_argv;
	  ਜਾਓ putgpr;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  gprvalue = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਛੋਟਾ *) *p_argv;
	  ਜਾਓ putgpr;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ_al = (*ptr)->ਮਾਪ;
#ਜੇ defined(POWERPC_DARWIN64)
	  ਅਗਲਾ_arg = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)FFI_ALIGN((ਅੱਖਰ *)ਅਗਲਾ_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_ਮੁੱਲ (*ptr, (ਅੱਖਰ *) *p_argv, 
					 (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਮਾਪ_al,
					 (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &fparg_ਗਿਣਤੀ,
					 &fpr_ਅਧਾਰ, &ਅਗਲਾ_arg);
#ਨਹੀਂਤਾਂ
	  dest_cpy = (ਅੱਖਰ *) ਅਗਲਾ_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  ਜੇ ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    ਮਾਪ_al = FFI_ALIGN((*ptr)->ਮਾਪ, 8);

#  ਜੇ defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) dest_cpy, (ਅੱਖਰ *) *p_argv, ਮਾਪ_al);
	  ਅਗਲਾ_arg += (ਮਾਪ_al + 7) / 8;
#  ਨਹੀਂਤਾਂ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  ਜੇ (ਮਾਪ_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - ਮਾਪ_al;

	  ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ *) dest_cpy, (ਅੱਖਰ *) *p_argv, ਮਾਪ_al);
	  ਅਗਲਾ_arg += (ਮਾਪ_al + 3) / 4;
#  endif
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  gprvalue = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) *p_argv;
	  ਜਾਓ putgpr;

	ਹਾਲਤ FFI_TYPE_UINT32:
	  gprvalue = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) *p_argv;
	putgpr:
	  *ਅਗਲਾ_arg++ = gprvalue;
	  gp_ਗਿਣਤੀ++;
	  ਰੋਕੋ;
	ਮੂਲ:
	  ਰੋਕੋ;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#ਜੇ defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ਸਥਿਰ ਖਾਲੀ
darwin64_ਜਾਂਚ_struct_for_floats (ffi_type *s, ਬਿਨਾਨਿਸ਼ਾਨੀ *nfpr)
{
  ਪੂਰਨ i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ffi_type *p = s->elements[i];
      ਚੋਣ (p->type)
	{
	  ਹਾਲਤ FFI_TYPE_STRUCT:
	    darwin64_ਜਾਂਚ_struct_for_floats (p, nfpr);
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ਰੋਕੋ;
	  ਮੂਲ:
	    ਰੋਕੋ;    
	}
    }
}

ਸਥਿਰ ਪੂਰਨ
darwin64_struct_ਮਾਪ_exceeds_gprs_p (ffi_type *s, ਅੱਖਰ *src, ਬਿਨਾਨਿਸ਼ਾਨੀ *nfpr)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ struct_ਵਿੱਥ=0, i;

  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ਅੱਖਰ *ਵਸਤੂ_ਅਧਾਰ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ਜੇ (i > 0)
        struct_ਵਿੱਥ = FFI_ALIGN(struct_ਵਿੱਥ, p->alignment);

      ਵਸਤੂ_ਅਧਾਰ = src + struct_ਵਿੱਥ;

      ਚੋਣ (p->type)
	{
	  ਹਾਲਤ FFI_TYPE_STRUCT:
	    ਜੇ (darwin64_struct_ਮਾਪ_exceeds_gprs_p (p, ਵਸਤੂ_ਅਧਾਰ, nfpr))
	      ਵਾਪਸ 1;
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	    ਜੇ (*nfpr >= ਅੰਕ_FPR_ARG_REGISTERS)
	      ਵਾਪਸ 1;
	    (*nfpr) += 1;
	    ਵਸਤੂ_ਅਧਾਰ += 8;
	  /* FALL THROUGH */
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (*nfpr >= ਅੰਕ_FPR_ARG_REGISTERS)
	      ਵਾਪਸ 1;
	    (*nfpr) += 1;
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਜੇ (*nfpr >= ਅੰਕ_FPR_ARG_REGISTERS)
	      ਵਾਪਸ 1;
	    (*nfpr) += 1;
	    ਰੋਕੋ;
	  ਮੂਲ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਵਸਤੂ_ਅਧਾਰ >= 8*8) 
	      ਵਾਪਸ 1;
	    ਰੋਕੋ;    
	}
      /* now count the size of what we just used.  */
      struct_ਵਿੱਥ += p->ਮਾਪ;
    }
  ਵਾਪਸ 0;
}

/* Can this struct be returned by value?  */
ਪੂਰਨ 
darwin64_struct_ret_by_ਮੁੱਲ_p (ffi_type *s)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  ਜੇ (s->ਮਾਪ > 168)
    ਵਾਪਸ 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_ਜਾਂਚ_struct_for_floats (s, &nfp);
  ਜੇ (nfp > 13)
    ਵਾਪਸ 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  ਜੇ (s->ਮਾਪ <= 64)
    ਵਾਪਸ 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  ਜੇ (darwin64_struct_ਮਾਪ_exceeds_gprs_p (s, ਨਿਰਾ, &nfp))
    ਵਾਪਸ 0;
  
  ਵਾਪਸ 1;
}

ਖਾਲੀ
darwin64_pass_struct_floats (ffi_type *s, ਅੱਖਰ *src, 
			     ਬਿਨਾਨਿਸ਼ਾਨੀ *nfpr, ਦੁੱਗਣਾ **fprs)
{
  ਪੂਰਨ i;
  ਦੁੱਗਣਾ *fpr_ਅਧਾਰ = *fprs;
  ਬਿਨਾਨਿਸ਼ਾਨੀ struct_ਵਿੱਥ = 0;

  /* We don't assume anything about the alignment of the source.  */
  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ਅੱਖਰ *ਵਸਤੂ_ਅਧਾਰ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ਜੇ (i > 0)
        struct_ਵਿੱਥ = FFI_ALIGN(struct_ਵਿੱਥ, p->alignment);
      ਵਸਤੂ_ਅਧਾਰ = src + struct_ਵਿੱਥ;

      ਚੋਣ (p->type)
	{
	  ਹਾਲਤ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, ਵਸਤੂ_ਅਧਾਰ, nfpr,
					   &fpr_ਅਧਾਰ);
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	    ਜੇ (*nfpr < ਅੰਕ_FPR_ARG_REGISTERS)
	      *fpr_ਅਧਾਰ++ = *(ਦੁੱਗਣਾ *)ਵਸਤੂ_ਅਧਾਰ;
	    (*nfpr) += 1;
	    ਵਸਤੂ_ਅਧਾਰ += 8;
	  /* FALL THROUGH */
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (*nfpr < ਅੰਕ_FPR_ARG_REGISTERS)
	      *fpr_ਅਧਾਰ++ = *(ਦੁੱਗਣਾ *)ਵਸਤੂ_ਅਧਾਰ;
	    (*nfpr) += 1;
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਜੇ (*nfpr < ਅੰਕ_FPR_ARG_REGISTERS)
	      *fpr_ਅਧਾਰ++ = (ਦੁੱਗਣਾ) *(ਦਸ਼ਮਲਵ *)ਵਸਤੂ_ਅਧਾਰ;
	    (*nfpr) += 1;
	    ਰੋਕੋ;
	  ਮੂਲ:
	    ਰੋਕੋ;    
	}
      /* now count the size of what we just used.  */
      struct_ਵਿੱਥ += p->ਮਾਪ;
    }
  /* Update the scores.  */
  *fprs = fpr_ਅਧਾਰ;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ਸਥਿਰ ਖਾਲੀ
darwin64_pass_struct_by_ਮੁੱਲ (ffi_type *s, ਅੱਖਰ *src, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ,
			       ਬਿਨਾਨਿਸ਼ਾਨੀ *nfpr, ਦੁੱਗਣਾ **fprs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ **arg)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *ਅਗਲਾ_arg = *arg;
  ਅੱਖਰ *dest_cpy = (ਅੱਖਰ *)ਅਗਲਾ_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ਜੇ (!ਮਾਪ)
    ਵਾਪਸ;

  /* First... special cases.  */
  ਜੇ (ਮਾਪ < 3
      || (ਮਾਪ == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *ਅਗਲਾ_arg = 0UL; 
      dest_cpy += 8 - ਮਾਪ;
      ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) dest_cpy, src, ਮਾਪ);
      ਅਗਲਾ_arg++;
    }
  ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ == 16)
    {
      ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) dest_cpy, src, ਮਾਪ);
      ਅਗਲਾ_arg += 2;
    }
  ਨਹੀਂਤਾਂ
    {
      /* now the general case, we consider embedded floats.  */
      ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) dest_cpy, src, ਮਾਪ);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      ਅਗਲਾ_arg += (ਮਾਪ+7)/8;
    }
    
  *arg = ਅਗਲਾ_arg;
}

ਦੁੱਗਣਾ *
darwin64_struct_floats_ਮੰਜ਼ਿਲ_mem (ffi_type *s, ਅੱਖਰ *dest, ਦੁੱਗਣਾ *fprs, ਬਿਨਾਨਿਸ਼ਾਨੀ *nf)
{
  ਪੂਰਨ i;
  ਬਿਨਾਨਿਸ਼ਾਨੀ struct_ਵਿੱਥ = 0;

  /* We don't assume anything about the alignment of the source.  */
  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ਅੱਖਰ *ਵਸਤੂ_ਅਧਾਰ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ਜੇ (i > 0)
        struct_ਵਿੱਥ = FFI_ALIGN(struct_ਵਿੱਥ, p->alignment);
      ਵਸਤੂ_ਅਧਾਰ = dest + struct_ਵਿੱਥ;

      ਚੋਣ (p->type)
	{
	  ਹਾਲਤ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_ਮੰਜ਼ਿਲ_mem (p, ਵਸਤੂ_ਅਧਾਰ, fprs, nf);
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	    ਜੇ (*nf < ਅੰਕ_FPR_ARG_REGISTERS)
	      {
		*(ਦੁੱਗਣਾ *)ਵਸਤੂ_ਅਧਾਰ = *fprs++ ;
		(*nf) += 1;
	      }
	    ਵਸਤੂ_ਅਧਾਰ += 8;
	  /* FALL THROUGH */
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (*nf < ਅੰਕ_FPR_ARG_REGISTERS)
	      {
		*(ਦੁੱਗਣਾ *)ਵਸਤੂ_ਅਧਾਰ = *fprs++ ;
		(*nf) += 1;
	      }
	    ਰੋਕੋ;
	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਜੇ (*nf < ਅੰਕ_FPR_ARG_REGISTERS)
	      {
		*(ਦਸ਼ਮਲਵ *)ਵਸਤੂ_ਅਧਾਰ = (ਦਸ਼ਮਲਵ) *fprs++ ;
		(*nf) += 1;
	      }
	    ਰੋਕੋ;
	  ਮੂਲ:
	    ਰੋਕੋ;    
	}
      /* now count the size of what we just used.  */
      struct_ਵਿੱਥ += p->ਮਾਪ;
    }
  ਵਾਪਸ fprs;
}

#ਜੇ_ਅੰਤ

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ਸਥਿਰ ਖਾਲੀ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  ਪੂਰਨ i;

  ਜੇ (s->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ;

  s->ਮਾਪ = 0;
  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ffi_type *p;
      ਪੂਰਨ align;
      
      p = s->elements[i];
      ਜੇ (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#ਜੇ defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#ਨਹੀਂਤਾਂ
      /* Natural alignment for the first item... */
      ਜੇ (i == 0)
	align = p->alignment;
      ਨਹੀਂਤਾਂ ਜੇ (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      ਨਹੀਂਤਾਂ
	/* .. or align is 4.  */
	align = 4;
#ਜੇ_ਅੰਤ
      /* Pad, if necessary, before adding the current item.  */
      s->ਮਾਪ = FFI_ALIGN(s->ਮਾਪ, align) + p->ਮਾਪ;
    }
  
  s->ਮਾਪ = FFI_ALIGN(s->ਮਾਪ, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  ਜੇ (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ਸਥਿਰ ਪੂਰਨ
aix_adjust_aggregate_sizes (ffi_type *s, ਪੂਰਨ outer_most_type_or_ਪਹਿਲਾ_member)
{
  ਪੂਰਨ i, nested_ਪਹਿਲਾ_member=0, final_align, rc=0;

  ਜੇ (s->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ 0;

  s->ਮਾਪ = 0;
  ਲਈ (i = 0; s->elements[i] != ਨਿਰਾ; i++)
    {
      ffi_type p;
      ਪੂਰਨ align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      ਜੇ (i == 0)
        nested_ਪਹਿਲਾ_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_ਪਹਿਲਾ_member);
      ਨਹੀਂਤਾਂ
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      ਜੇ (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->ਮਾਪ = FFI_ALIGN(s->ਮਾਪ, align) + p.ਮਾਪ;
    }

  final_align=s->alignment;
  ਜੇ ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_ਪਹਿਲਾ_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      ਜੇ (outer_most_type_or_ਪਹਿਲਾ_member)
        s->alignment=final_align;
  }

  s->ਮਾਪ = FFI_ALIGN(s->ਮਾਪ, final_align);
  ਵਾਪਸ rc;
}

/* Perform machine dependent cif processing.  */
ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ i;
  ffi_type **ptr;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ fparg_ਗਿਣਤੀ = 0, intarg_ਗਿਣਤੀ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  ਜੇ (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      ਲਈ (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  ਜੇ (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      ਲਈ (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  ਬਾਈਟਾਂ = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ);

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
  ਚੋਣ (cif->rtype->type)
    {

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_ਬਿੱਟ;
      flags |= FLAG_RETURNS_FP;
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      /* Fall through.  */
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
#ਜੇ_ਅੰਤ
      flags |= FLAG_RETURNS_64_ਬਿੱਟ;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	ਜੇ (darwin64_struct_ret_by_ਮੁੱਲ_p (cif->rtype))
	  {
	    ਬਿਨਾਨਿਸ਼ਾਨੀ nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    ਜੇ (cif->rtype->ਮਾਪ != 16)
	      darwin64_ਜਾਂਚ_struct_for_floats (cif->rtype, &nfpr) ;
	    ਨਹੀਂਤਾਂ
	      flags |= FLAG_RETURNS_128_ਬਿੱਟ;
	    /* Will be 0 for 16byte struct.  */
	    ਜੇ (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	ਨਹੀਂਤਾਂ /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_ਗਿਣਤੀ++;
	  }
      }
#ਜਾਂਜੇ defined(DARWIN_PPC)
      ਜੇ (cif->rtype->ਮਾਪ <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      ਨਹੀਂਤਾਂ /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_ਗਿਣਤੀ++;
	}
#ਨਹੀਂਤਾਂ /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ਰੋਕੋ;

    ਮੂਲ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ਰੋਕੋ;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ align_ਸ਼ਬਦ;
      ਚੋਣ ((*ptr)->type)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  fparg_ਗਿਣਤੀ++;
#ਜੇ !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  ਜੇ (fparg_ਗਿਣਤੀ > ਅੰਕ_FPR_ARG_REGISTERS
	      && (intarg_ਗਿਣਤੀ & 0x01) != 0)
	    intarg_ਗਿਣਤੀ++;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  fparg_ਗਿਣਤੀ += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  ਜੇ (fparg_ਗਿਣਤੀ >= ਅੰਕ_FPR_ARG_REGISTERS)
#ਜੇ defined (POWERPC64)
	    intarg_ਗਿਣਤੀ = FFI_ALIGN(intarg_ਗਿਣਤੀ, 2);
#ਨਹੀਂਤਾਂ
	    intarg_ਗਿਣਤੀ = FFI_ALIGN(intarg_ਗਿਣਤੀ, 4);
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ defined(POWERPC64)
	  intarg_ਗਿਣਤੀ++;
#ਨਹੀਂਤਾਂ
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  ਜੇ (intarg_ਗਿਣਤੀ == ਅੰਕ_GPR_ARG_REGISTERS-1
	      || (intarg_ਗਿਣਤੀ >= ਅੰਕ_GPR_ARG_REGISTERS 
	          && (intarg_ਗਿਣਤੀ & 0x01) != 0))
	    intarg_ਗਿਣਤੀ++;
	  intarg_ਗਿਣਤੀ += 2;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ_al = (*ptr)->ਮਾਪ;
#ਜੇ defined(POWERPC_DARWIN64)
	  align_ਸ਼ਬਦ = (*ptr)->alignment >> 3;
	  ਜੇ (align_ਸ਼ਬਦ)
	    intarg_ਗਿਣਤੀ = FFI_ALIGN(intarg_ਗਿਣਤੀ, align_ਸ਼ਬਦ);
	  /* Base size of the struct.  */
	  intarg_ਗਿਣਤੀ += (ਮਾਪ_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  ਜੇ (ਮਾਪ_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_ਜਾਂਚ_struct_for_floats (*ptr, &fparg_ਗਿਣਤੀ) ;
#ਨਹੀਂਤਾਂ
	  align_ਸ਼ਬਦ = (*ptr)->alignment >> 2;
	  ਜੇ (align_ਸ਼ਬਦ)
	    intarg_ਗਿਣਤੀ = FFI_ALIGN(intarg_ਗਿਣਤੀ, align_ਸ਼ਬਦ);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_ਗਿਣਤੀ += (ਮਾਪ_al + 7) / 8;
#  ਨਹੀਂਤਾਂ
	  intarg_ਗਿਣਤੀ += (ਮਾਪ_al + 3) / 4;
#  endif
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਮੂਲ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ਗਿਣਤੀ++;
	  ਰੋਕੋ;
	}
    }

  ਜੇ (fparg_ਗਿਣਤੀ != 0)
    flags |= FLAG_FP_ARGUMENTS;

#ਜੇ defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  ਜੇ (fparg_ਗਿਣਤੀ != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    ਬਾਈਟਾਂ += ਅੰਕ_FPR_ARG_REGISTERS * ਆਕਾਰ(ਦੁੱਗਣਾ);
#ਨਹੀਂਤਾਂ
  /* Space for the FPR registers, if needed.  */
  ਜੇ (fparg_ਗਿਣਤੀ != 0)
    ਬਾਈਟਾਂ += ਅੰਕ_FPR_ARG_REGISTERS * ਆਕਾਰ(ਦੁੱਗਣਾ);
#ਜੇ_ਅੰਤ

  /* Stack space.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ਜੇ ((intarg_ਗਿਣਤੀ + fparg_ਗਿਣਤੀ) > ਅੰਕ_GPR_ARG_REGISTERS)
    ਬਾਈਟਾਂ += (intarg_ਗਿਣਤੀ + fparg_ਗਿਣਤੀ) * ਆਕਾਰ(ਲੰਮਾ);
#ਨਹੀਂਤਾਂ
  ਜੇ ((intarg_ਗਿਣਤੀ + 2 * fparg_ਗਿਣਤੀ) > ਅੰਕ_GPR_ARG_REGISTERS)
    ਬਾਈਟਾਂ += (intarg_ਗਿਣਤੀ + 2 * fparg_ਗਿਣਤੀ) * ਆਕਾਰ(ਲੰਮਾ);
#ਜੇ_ਅੰਤ
  ਨਹੀਂਤਾਂ
    ਬਾਈਟਾਂ += ਅੰਕ_GPR_ARG_REGISTERS * ਆਕਾਰ(ਲੰਮਾ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, 16) ;

  cif->flags = flags;
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_AIX(extended_cif *, ਲੰਮਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			 ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ (*fn2)(ਖਾਲੀ));

#ਜੇ defined (FFI_GO_CLOSURES)
ਬਾਹਰੀ ਖਾਲੀ ffi_call_go_AIX(extended_cif *, ਲੰਮਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			    ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ (*fn2)(ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ);
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ ffi_call_DARWIN(extended_cif *, ਲੰਮਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			    ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ (*fn2)(ਖਾਲੀ), ffi_type*);

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ਜੇ ((rvalue == ਨਿਰਾ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_AIX:
      ffi_call_AIX(&ecif, -(ਲੰਮਾ)cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_ਦਲੀਲਾਂ));
      ਰੋਕੋ;
    ਹਾਲਤ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(ਲੰਮਾ)cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_ਦਲੀਲਾਂ), cif->rtype);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}

#ਜੇ defined (FFI_GO_CLOSURES)
ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	     ਖਾਲੀ *ਘੇਰਾ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ਜੇ ((rvalue == ਨਿਰਾ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(ਲੰਮਾ)cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_ਦਲੀਲਾਂ), ਘੇਰਾ);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ flush_icache(ਅੱਖਰ *);
ਸਥਿਰ ਖਾਲੀ flush_range(ਅੱਖਰ *, ਪੂਰਨ);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ aix_fd_struct {
  ਖਾਲੀ *ਕੋਡ_ਸੂਚਕ;
  ਖਾਲੀ *toc;
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

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp;
  ਬਣਤਰ ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ਚੋਣ (cif->abi)
    {
      ਹਾਲਤ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
#ਜੇ defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)&tramp[2]) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_ASM; /* function  */
	*((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)&tramp[4]) = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) codeloc; /* context  */
#ਨਹੀਂਤਾਂ
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_ASM; /* function  */
	tramp[3] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) codeloc; /* context  */
#ਜੇ_ਅੰਤ
	ਘੇਰਾ->cif = cif;
	ਘੇਰਾ->fun = fun;
	ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_ਮਾਪ);

	ਰੋਕੋ;

    ਹਾਲਤ FFI_AIX:

      tramp_aix = (ਬਣਤਰ ffi_aix_trampoline_struct *) (ਘੇਰਾ->tramp);
      fd = (aix_fd *)(ਖਾਲੀ *)ffi_ਘੇਰਾ_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->ਕੋਡ_ਸੂਚਕ = fd->ਕੋਡ_ਸੂਚਕ;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      ਘੇਰਾ->cif = cif;
      ਘੇਰਾ->fun = fun;
      ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
      ਰੋਕੋ;

    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_ABI;
      ਰੋਕੋ;
    }
  ਵਾਪਸ FFI_OK;
}

#ਜੇ defined (FFI_GO_CLOSURES)
ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ,
		     ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
  ਚੋਣ (cif->abi)
    {
      ਹਾਲਤ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        ਘੇਰਾ->tramp = (ਖਾਲੀ *)ffi_go_ਘੇਰਾ_ASM;
        ਘੇਰਾ->cif = cif;
        ਘੇਰਾ->fun = fun;
        ਵਾਪਸ FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      ਮੂਲ:
        ਵਾਪਸ FFI_BAD_ABI;
        ਰੋਕੋ;
    }
  ਵਾਪਸ FFI_OK;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ
flush_icache(ਅੱਖਰ *addr)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _AIX
  __asm__ ਅਸਥਿਰ (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ
flush_range(ਅੱਖਰ * addr1, ਪੂਰਨ ਮਾਪ)
{
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MIN_ਸਤਰ_ਮਾਪ 32
  ਪੂਰਨ i;
  ਲਈ (i = 0; i < ਮਾਪ; i += MIN_ਸਤਰ_ਮਾਪ)
    flush_icache(addr1+i);
  flush_icache(addr1+ਮਾਪ-1);
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਸੰਘ
{
  ਦਸ਼ਮਲਵ f;
  ਦੁੱਗਣਾ d;
} ffi_dblfl;

ਪੂਰਨ
ffi_ਘੇਰਾ_helper_DARWIN (ffi_ਘੇਰਾ *, ਖਾਲੀ *,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *, ffi_dblfl *);

#ਜੇ defined (FFI_GO_CLOSURES)
ਪੂਰਨ
ffi_go_ਘੇਰਾ_helper_DARWIN (ffi_go_ਘੇਰਾ*, ਖਾਲੀ *,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *, ffi_dblfl *);
#ਜੇ_ਅੰਤ

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ਸਥਿਰ ਪੂਰਨ
ffi_ਘੇਰਾ_helper_common (ffi_cif* cif,
			   ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			   ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *rvalue,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਦੁੱਗਣਾ ldbits[2];

  ਸੰਘ ldu
  {
    ldbits lb;
    ਲੰਮਾ ਦੁੱਗਣਾ ld;
  };

  ਖਾਲੀ **          avalue;
  ffi_type **      arg_types;
  ਲੰਮਾ             i, avn;
  ffi_dblfl *      ਸਿਰਾ_pfr = pfr + ਅੰਕ_FPR_ARG_REGISTERS;
  ਬਿਨਾਨਿਸ਼ਾਨੀ         ਮਾਪ_al;
  ਪੂਰਨ              struct_ret_by_ਮੁੱਲ = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *  pgr0 = pgr;
#ਜੇ defined(POWERPC_DARWIN64)
  ਬਿਨਾਨਿਸ਼ਾਨੀ 	   fpsused = 0;
#ਜੇ_ਅੰਤ

  avalue = alloca (cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#ਜੇ defined(POWERPC_DARWIN64)
      ਜੇ (!darwin64_struct_ret_by_ਮੁੱਲ_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (ਖਾਲੀ *) *pgr;
	  pgr++;
	}
      ਨਹੀਂਤਾਂ
	struct_ret_by_ਮੁੱਲ = 1;
#ਜਾਂਜੇ defined(DARWIN_PPC)
      ਜੇ (cif->rtype->ਮਾਪ > 4)
	{
	  rvalue = (ਖਾਲੀ *) *pgr;
	  pgr++;
	}
      ਨਹੀਂਤਾਂ
	struct_ret_by_ਮੁੱਲ = 1;
#ਨਹੀਂਤਾਂ /* assume we return by ref.  */
      rvalue = (ਖਾਲੀ *) *pgr;
      pgr++;
#ਜੇ_ਅੰਤ
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਜਦੋਂ (i < avn)
    {
      ਚੋਣ (arg_types[i]->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
#ਜੇ  defined(POWERPC64)
	  avalue[i] = (ਅੱਖਰ *) pgr + 7;
#ਨਹੀਂਤਾਂ
	  avalue[i] = (ਅੱਖਰ *) pgr + 3;
#ਜੇ_ਅੰਤ
	  pgr++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
#ਜੇ  defined(POWERPC64)
	  avalue[i] = (ਅੱਖਰ *) pgr + 6;
#ਨਹੀਂਤਾਂ
	  avalue[i] = (ਅੱਖਰ *) pgr + 2;
#ਜੇ_ਅੰਤ
	  pgr++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
#ਜੇ  defined(POWERPC64)
	  avalue[i] = (ਅੱਖਰ *) pgr + 4;
#ਨਹੀਂਤਾਂ
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = pgr;
#ਜੇ_ਅੰਤ
	  pgr++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ_al = arg_types[i]->ਮਾਪ;
#ਜੇ defined(POWERPC_DARWIN64)
	  pgr = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)FFI_ALIGN((ਅੱਖਰ *)pgr, arg_types[i]->alignment);
	  ਜੇ (ਮਾਪ_al < 3 || ਮਾਪ_al == 4)
	    {
	      avalue[i] = ((ਅੱਖਰ *)pgr)+8-ਮਾਪ_al;
	      ਜੇ (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < ਅੰਕ_FPR_ARG_REGISTERS)
		{
		  *(ਦਸ਼ਮਲਵ *)pgr = (ਦਸ਼ਮਲਵ) *(ਦੁੱਗਣਾ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  ਨਹੀਂਤਾਂ 
	    {
	      ਜੇ (ਮਾਪ_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_ਮੰਜ਼ਿਲ_mem (arg_types[i], (ਅੱਖਰ *)pgr,
						   (ਦੁੱਗਣਾ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (ਮਾਪ_al + 7) / 8;
#ਨਹੀਂਤਾਂ
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  ਜੇ (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    ਮਾਪ_al = FFI_ALIGN(arg_types[i]->ਮਾਪ, 8);
#  ਜੇ defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (ਮਾਪ_al + 7) / 8;
#  ਨਹੀਂਤਾਂ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  ਜੇ (ਮਾਪ_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (ਅੱਖਰ*) pgr + 4 - ਮਾਪ_al;
	  ਨਹੀਂਤਾਂ
	    avalue[i] = pgr;
	  pgr += (ਮਾਪ_al + 3) / 4;
#  endif
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
#ਜੇ  defined(POWERPC64)
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = pgr;
	  pgr++;
	  ਰੋਕੋ;
#ਨਹੀਂਤਾਂ
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  ਜੇ (pfr < ਸਿਰਾ_pfr)
	    {
	      ਦੁੱਗਣਾ ਆਰਜ਼ੀ = pfr->d;
	      pfr->f = (ਦਸ਼ਮਲਵ) ਆਰਜ਼ੀ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  ਜੇ (pfr < ਸਿਰਾ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = pgr;
	    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
	  pgr++;
#ਨਹੀਂਤਾਂ
	  pgr += 2;
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
	  ਜੇ (pfr + 1 < ਸਿਰਾ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਜੇ (pfr < ਸਿਰਾ_pfr)
		{
		  *pgr = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#ਨਹੀਂਤਾਂ  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  ਜੇ (pfr + 1 < ਸਿਰਾ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  ਨਹੀਂਤਾਂ ਜੇ (pfr + 1 == ਸਿਰਾ_pfr)
	    {
	      ਸੰਘ ldu ਆਰਜ਼ੀ_ld;
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&ਆਰਜ਼ੀ_ld.lb[0], pfr, ਆਕਾਰ(ldbits));
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ (&ਆਰਜ਼ੀ_ld.lb[1], pgr + 2, ਆਕਾਰ(ldbits));
	      avalue[i] = &ਆਰਜ਼ੀ_ld.ld;
	      pfr++;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#ਜੇ_ਅੰਤ  /* POWERPC64 */
	  ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	ਮੂਲ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      ਵਾਪਸ PPC_LD_NONE;
    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      ਜੇ (struct_ret_by_ਮੁੱਲ)
	{
	  *pgr0 = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) cif->rtype;
	  ਵਾਪਸ PPC_LD_STRUCT;
	}
      ਵਾਪਸ PPC_LD_NONE;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਵਾਪਸ PPC_LD_F32;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਵਾਪਸ PPC_LD_F64;
#ਜੇ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      ਵਾਪਸ PPC_LD_F128;
#ਜੇ_ਅੰਤ
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਵਾਪਸ PPC_LD_U8;
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਵਾਪਸ PPC_LD_S8;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਵਾਪਸ PPC_LD_U16;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਵਾਪਸ PPC_LD_S16;
    ਹਾਲਤ FFI_TYPE_UINT32:
      ਵਾਪਸ PPC_LD_U32;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      ਵਾਪਸ PPC_LD_S32;
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਵਾਪਸ PPC_LD_PTR;
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      ਵਾਪਸ PPC_LD_I64;
    ਮੂਲ:
      abort();
    }
}

ਪੂਰਨ
ffi_ਘੇਰਾ_helper_DARWIN (ffi_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *rvalue,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pgr, ffi_dblfl *pfr)
{
  ਵਾਪਸ ffi_ਘੇਰਾ_helper_common (ਘੇਰਾ->cif, ਘੇਰਾ->fun,
				    ਘੇਰਾ->user_ਅੰਕੜੇ, rvalue, pgr, pfr);
}

#ਜੇ defined (FFI_GO_CLOSURES)
ਪੂਰਨ
ffi_go_ਘੇਰਾ_helper_DARWIN (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ਖਾਲੀ *rvalue,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *pgr, ffi_dblfl *pfr)
{
  ਵਾਪਸ ffi_ਘੇਰਾ_helper_common (ਘੇਰਾ->cif, ਘੇਰਾ->fun,
				    ਘੇਰਾ, rvalue, pgr, pfr);
}
#ਜੇ_ਅੰਤ

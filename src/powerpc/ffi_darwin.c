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


ರಚನೆ ffi_aix_trampoline_struct {
    ಶೂನ್ಯ * ಸಂಕೇತ_ಸೂಚಕ;	/* Pointer to ffi_closure_ASM */
    ಶೂನ್ಯ * toc;			/* TOC */
    ಶೂನ್ಯ * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_NONE		0
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_R3		1
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_R3R4		2
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F32		3
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F64		4
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_F128		5
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U8		6
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S8		7
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_U16		8
#ವ್ಯಾಖ್ಯಾನಿಸು PPC_LD_S16		9

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ POWERPC64
# define PPC_LD_U32		PPC_LD_R3
# define PPC_LD_S32		PPC_LD_R3
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3R4
# define PPC_LD_STRUCT		10
#ಇಲ್ಲದಿದ್ದರೆ
# define PPC_LD_U32		10
# define PPC_LD_S32		11
# define PPC_LD_PTR		PPC_LD_R3
# define PPC_LD_I64		PPC_LD_R3
# define PPC_LD_STRUCT		12
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_ASM (ಶೂನ್ಯ);

#ಆದರೆ defined (FFI_GO_CLOSURES)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_ASM (ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ {
  /* The assembly depends on these exact flags.  
     For Darwin64 (when FLAG_RETURNS_STRUCT is set):
       FLAG_RETURNS_FP indicates that the structure embeds FP data.
       FLAG_RETURNS_128BITS signals a special struct size that is not
       expanded for float content.  */
  FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು	= 1 << (31-31), /* These go in cr7  */
  FLAG_RETURNS_NOTHING	= 1 << (31-30),
  FLAG_RETURNS_FP	= 1 << (31-29),
  FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು	= 1 << (31-28),

  FLAG_RETURNS_STRUCT	= 1 << (31-27), /* This goes in cr6  */

  FLAG_ARG_NEEDS_COPY   = 1 << (31- 7),
  FLAG_FP_ARGUMENTS     = 1 << (31- 6), /* cr1.eq; specified by ABI  */
  FLAG_4_GPR_ARGUMENTS  = 1 << (31- 5),
  FLAG_RETVAL_REFERENCE = 1 << (31- 4)
};

/* About the DARWIN ABI.  */
ಎಣಿಕೆ {
  ಅಂಕೆ_GPR_ARG_REGISTERS = 8,
  ಅಂಕೆ_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

ಎಣಿಕೆ { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#ಆದರೆ defined(POWERPC_DARWIN64)
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_pass_struct_by_ಮೌಲ್ಯ 
  (ffi_type *, ಅಕ್ಷರ *, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *, ದ್ವಿಗುಣ **, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ **);
#ಆದರೆ_ಅಂತ್ಯ

/* This depends on GPR_SIZE = sizeof (unsigned long) */

ಶೂನ್ಯ
ffi_prep_ವಾದಗಳು (extended_cif *ecif, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಸ್ಥಿರಾಂಕ ರಾಶಿ)
{
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು = ecif->cif->ಬೈಟುಗಳು;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ flags = ecif->cif->flags;
  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ nargs = ecif->cif->nargs;
#ಆದರೆ !defined(POWERPC_DARWIN64) 
  ಸ್ಥಿರಾಂಕ ffi_abi abi = ecif->cif->abi;
#ಆದರೆ_ಅಂತ್ಯ

  /* 'stacktop' points at the previous backchain pointer.  */
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಸ್ಥಿರಾಂಕ stacktop = ರಾಶಿ + (ಬೈಟುಗಳು / ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  ದ್ವಿಗುಣ *fpr_ತಳ = (ದ್ವಿಗುಣ *) (stacktop - ASM_NEEDS_REGISTERS) - ಅಂಕೆ_FPR_ARG_REGISTERS;
  ಪೂರ್ಣಾಂಕ gp_ಎಣಿಕೆ = 0, fparg_ಎಣಿಕೆ = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಮುಂದಿನ_arg = ರಾಶಿ + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  ಪೂರ್ಣಾಂಕ i;
  ದ್ವಿಗುಣ double_tmp;
  ಶೂನ್ಯ **p_argv = ecif->avalue;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#ಆದರೆ !defined(POWERPC_DARWIN64) 
  ಅಕ್ಷರ *dest_cpy;
#ಆದರೆ_ಅಂತ್ಯ
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((ಚಿಹ್ನೆರಹಿತ) (ಅಕ್ಷರ *) ರಾಶಿ & 0xF) == 0);
  FFI_ASSERT(((ಚಿಹ್ನೆರಹಿತ) (ಅಕ್ಷರ *) stacktop & 0xF) == 0);
  FFI_ASSERT((ಬೈಟುಗಳು & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  ಆದರೆ (flags & FLAG_RETVAL_REFERENCE)
    *ಮುಂದಿನ_arg++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) (ಅಕ್ಷರ *) ecif->rvalue;

  /* Now for the arguments.  */
  ಗಾಗಿ (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      ಆಯ್ಕೆ ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  double_tmp = *(ದಶಮಾಂಶ *) *p_argv;
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_ತಳ++ = double_tmp;
#ಆದರೆ defined(POWERPC_DARWIN)
	  *(ದಶಮಾಂಶ *)ಮುಂದಿನ_arg = *(ದಶಮಾಂಶ *) *p_argv;
#ಇಲ್ಲದಿದ್ದರೆ
	  *(ದ್ವಿಗುಣ *)ಮುಂದಿನ_arg = double_tmp;
#ಆದರೆ_ಅಂತ್ಯ
	  ಮುಂದಿನ_arg++;
	  gp_ಎಣಿಕೆ++;
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  double_tmp = *(ದ್ವಿಗುಣ *) *p_argv;
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_ತಳ++ = double_tmp;
	  *(ದ್ವಿಗುಣ *)ಮುಂದಿನ_arg = double_tmp;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
	  ಮುಂದಿನ_arg++;
	  gp_ಎಣಿಕೆ++;
#ಇಲ್ಲದಿದ್ದರೆ
	  ಮುಂದಿನ_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#ಆದರೆ_ಅಂತ್ಯ
	  fparg_ಎಣಿಕೆ++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#  ಆದರೆ defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *(ದೀರ್ಘ ದ್ವಿಗುಣ *) fpr_ತಳ++ = *(ದೀರ್ಘ ದ್ವಿಗುಣ *) *p_argv;
	  ಇಲ್ಲದಿದ್ದರೆ
	    *(ದೀರ್ಘ ದ್ವಿಗುಣ *) ಮುಂದಿನ_arg = *(ದೀರ್ಘ ದ್ವಿಗುಣ *) *p_argv;
	  ಮುಂದಿನ_arg += 2;
	  fparg_ಎಣಿಕೆ += 2;
#  ಇಲ್ಲದಿದ್ದರೆ
	  double_tmp = ((ದ್ವಿಗುಣ *) *p_argv)[0];
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_ತಳ++ = double_tmp;
	  *(ದ್ವಿಗುಣ *) ಮುಂದಿನ_arg = double_tmp;
#    ಆದರೆ defined(POWERPC_DARWIN64)
	  ಮುಂದಿನ_arg++;
	  gp_ಎಣಿಕೆ++;
#    ಇಲ್ಲದಿದ್ದರೆ
	  ಮುಂದಿನ_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#    endif
	  fparg_ಎಣಿಕೆ++;
	  double_tmp = ((ದ್ವಿಗುಣ *) *p_argv)[1];
	  ಆದರೆ (fparg_ಎಣಿಕೆ < ಅಂಕೆ_FPR_ARG_REGISTERS)
	    *fpr_ತಳ++ = double_tmp;
	  *(ದ್ವಿಗುಣ *) ಮುಂದಿನ_arg = double_tmp;
#    ಆದರೆ defined(POWERPC_DARWIN64)
	  ಮುಂದಿನ_arg++;
	  gp_ಎಣಿಕೆ++;
#    ಇಲ್ಲದಿದ್ದರೆ
	  ಮುಂದಿನ_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#    endif
	  fparg_ಎಣಿಕೆ++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
	  gprvalue = *(ದೀರ್ಘ ದೀರ್ಘ *) *p_argv;
	  ಹೋಗು putgpr;
#ಇಲ್ಲದಿದ್ದರೆ
	  *(ದೀರ್ಘ ದೀರ್ಘ *) ಮುಂದಿನ_arg = *(ದೀರ್ಘ ದೀರ್ಘ *) *p_argv;
	  ಮುಂದಿನ_arg += 2;
	  gp_ಎಣಿಕೆ += 2;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  gprvalue = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) *p_argv;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *) *p_argv;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  gprvalue = *(ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ *) *p_argv;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) *p_argv;
	  ಹೋಗು putgpr;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  gprvalue = *(ಚಿಹ್ನೆಯುಳ್ಳ ಗಿಡ್ಡ *) *p_argv;
	  ಹೋಗು putgpr;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_al = (*ptr)->ಗಾತ್ರ_2;
#ಆದರೆ defined(POWERPC_DARWIN64)
	  ಮುಂದಿನ_arg = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)FFI_ALIGN((ಅಕ್ಷರ *)ಮುಂದಿನ_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_ಮೌಲ್ಯ (*ptr, (ಅಕ್ಷರ *) *p_argv, 
					 (ಚಿಹ್ನೆರಹಿತ) ಗಾತ್ರ_al,
					 (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &fparg_ಎಣಿಕೆ,
					 &fpr_ತಳ, &ಮುಂದಿನ_arg);
#ಇಲ್ಲದಿದ್ದರೆ
	  dest_cpy = (ಅಕ್ಷರ *) ಮುಂದಿನ_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  ಆದರೆ ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    ಗಾತ್ರ_al = FFI_ALIGN((*ptr)->ಗಾತ್ರ_2, 8);

#  ಆದರೆ defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) dest_cpy, (ಅಕ್ಷರ *) *p_argv, ಗಾತ್ರ_al);
	  ಮುಂದಿನ_arg += (ಗಾತ್ರ_al + 7) / 8;
#  ಇಲ್ಲದಿದ್ದರೆ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  ಆದರೆ (ಗಾತ್ರ_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - ಗಾತ್ರ_al;

	  ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ *) dest_cpy, (ಅಕ್ಷರ *) *p_argv, ಗಾತ್ರ_al);
	  ಮುಂದಿನ_arg += (ಗಾತ್ರ_al + 3) / 4;
#  endif
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  gprvalue = *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) *p_argv;
	  ಹೋಗು putgpr;

	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  gprvalue = *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) *p_argv;
	putgpr:
	  *ಮುಂದಿನ_arg++ = gprvalue;
	  gp_ಎಣಿಕೆ++;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ನಿಲ್ಲಿಸು;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#ಆದರೆ defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_ಪರಿಶೀಲನೆ_struct_for_floats (ffi_type *s, ಚಿಹ್ನೆರಹಿತ *nfpr)
{
  ಪೂರ್ಣಾಂಕ i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ffi_type *p = s->elements[i];
      ಆಯ್ಕೆ (p->type)
	{
	  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	    darwin64_ಪರಿಶೀಲನೆ_struct_for_floats (p, nfpr);
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    ನಿಲ್ಲಿಸು;
	  ಪೂರ್ವನಿಯೋಜಿತ:
	    ನಿಲ್ಲಿಸು;    
	}
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
darwin64_struct_ಗಾತ್ರ_exceeds_gprs_p (ffi_type *s, ಅಕ್ಷರ *src, ಚಿಹ್ನೆರಹಿತ *nfpr)
{
  ಚಿಹ್ನೆರಹಿತ struct_ಸ್ಥಾನಾಂತರ=0, i;

  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ಅಕ್ಷರ *ಅಂಶ_ತಳ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಆದರೆ (i > 0)
        struct_ಸ್ಥಾನಾಂತರ = FFI_ALIGN(struct_ಸ್ಥಾನಾಂತರ, p->alignment);

      ಅಂಶ_ತಳ = src + struct_ಸ್ಥಾನಾಂತರ;

      ಆಯ್ಕೆ (p->type)
	{
	  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	    ಆದರೆ (darwin64_struct_ಗಾತ್ರ_exceeds_gprs_p (p, ಅಂಶ_ತಳ, nfpr))
	      ಹಿಂತಿರುಗಿ 1;
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	    ಆದರೆ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಹಿಂತಿರುಗಿ 1;
	    (*nfpr) += 1;
	    ಅಂಶ_ತಳ += 8;
	  /* FALL THROUGH */
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಹಿಂತಿರುಗಿ 1;
	    (*nfpr) += 1;
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಆದರೆ (*nfpr >= ಅಂಕೆ_FPR_ARG_REGISTERS)
	      ಹಿಂತಿರುಗಿ 1;
	    (*nfpr) += 1;
	    ನಿಲ್ಲಿಸು;
	  ಪೂರ್ವನಿಯೋಜಿತ:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಅಂಶ_ತಳ >= 8*8) 
	      ಹಿಂತಿರುಗಿ 1;
	    ನಿಲ್ಲಿಸು;    
	}
      /* now count the size of what we just used.  */
      struct_ಸ್ಥಾನಾಂತರ += p->ಗಾತ್ರ_2;
    }
  ಹಿಂತಿರುಗಿ 0;
}

/* Can this struct be returned by value?  */
ಪೂರ್ಣಾಂಕ 
darwin64_struct_ret_by_ಮೌಲ್ಯ_p (ffi_type *s)
{
  ಚಿಹ್ನೆರಹಿತ nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  ಆದರೆ (s->ಗಾತ್ರ_2 > 168)
    ಹಿಂತಿರುಗಿ 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_ಪರಿಶೀಲನೆ_struct_for_floats (s, &nfp);
  ಆದರೆ (nfp > 13)
    ಹಿಂತಿರುಗಿ 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  ಆದರೆ (s->ಗಾತ್ರ_2 <= 64)
    ಹಿಂತಿರುಗಿ 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  ಆದರೆ (darwin64_struct_ಗಾತ್ರ_exceeds_gprs_p (s, ಖಾಲಿ, &nfp))
    ಹಿಂತಿರುಗಿ 0;
  
  ಹಿಂತಿರುಗಿ 1;
}

ಶೂನ್ಯ
darwin64_pass_struct_floats (ffi_type *s, ಅಕ್ಷರ *src, 
			     ಚಿಹ್ನೆರಹಿತ *nfpr, ದ್ವಿಗುಣ **fprs)
{
  ಪೂರ್ಣಾಂಕ i;
  ದ್ವಿಗುಣ *fpr_ತಳ = *fprs;
  ಚಿಹ್ನೆರಹಿತ struct_ಸ್ಥಾನಾಂತರ = 0;

  /* We don't assume anything about the alignment of the source.  */
  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ಅಕ್ಷರ *ಅಂಶ_ತಳ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಆದರೆ (i > 0)
        struct_ಸ್ಥಾನಾಂತರ = FFI_ALIGN(struct_ಸ್ಥಾನಾಂತರ, p->alignment);
      ಅಂಶ_ತಳ = src + struct_ಸ್ಥಾನಾಂತರ;

      ಆಯ್ಕೆ (p->type)
	{
	  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, ಅಂಶ_ತಳ, nfpr,
					   &fpr_ತಳ);
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	    ಆದರೆ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_ತಳ++ = *(ದ್ವಿಗುಣ *)ಅಂಶ_ತಳ;
	    (*nfpr) += 1;
	    ಅಂಶ_ತಳ += 8;
	  /* FALL THROUGH */
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_ತಳ++ = *(ದ್ವಿಗುಣ *)ಅಂಶ_ತಳ;
	    (*nfpr) += 1;
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಆದರೆ (*nfpr < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      *fpr_ತಳ++ = (ದ್ವಿಗುಣ) *(ದಶಮಾಂಶ *)ಅಂಶ_ತಳ;
	    (*nfpr) += 1;
	    ನಿಲ್ಲಿಸು;
	  ಪೂರ್ವನಿಯೋಜಿತ:
	    ನಿಲ್ಲಿಸು;    
	}
      /* now count the size of what we just used.  */
      struct_ಸ್ಥಾನಾಂತರ += p->ಗಾತ್ರ_2;
    }
  /* Update the scores.  */
  *fprs = fpr_ತಳ;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
ಸ್ಥಿರ ಶೂನ್ಯ
darwin64_pass_struct_by_ಮೌಲ್ಯ (ffi_type *s, ಅಕ್ಷರ *src, ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_2,
			       ಚಿಹ್ನೆರಹಿತ *nfpr, ದ್ವಿಗುಣ **fprs, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ **arg)
{
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *ಮುಂದಿನ_arg = *arg;
  ಅಕ್ಷರ *dest_cpy = (ಅಕ್ಷರ *)ಮುಂದಿನ_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  ಆದರೆ (!ಗಾತ್ರ_2)
    ಹಿಂತಿರುಗಿ;

  /* First... special cases.  */
  ಆದರೆ (ಗಾತ್ರ_2 < 3
      || (ಗಾತ್ರ_2 == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *ಮುಂದಿನ_arg = 0UL; 
      dest_cpy += 8 - ಗಾತ್ರ_2;
      ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) dest_cpy, src, ಗಾತ್ರ_2);
      ಮುಂದಿನ_arg++;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಗಾತ್ರ_2 == 16)
    {
      ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) dest_cpy, src, ಗಾತ್ರ_2);
      ಮುಂದಿನ_arg += 2;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* now the general case, we consider embedded floats.  */
      ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) dest_cpy, src, ಗಾತ್ರ_2);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      ಮುಂದಿನ_arg += (ಗಾತ್ರ_2+7)/8;
    }
    
  *arg = ಮುಂದಿನ_arg;
}

ದ್ವಿಗುಣ *
darwin64_struct_floats_ಗಮ್ಯ_mem (ffi_type *s, ಅಕ್ಷರ *dest, ದ್ವಿಗುಣ *fprs, ಚಿಹ್ನೆರಹಿತ *nf)
{
  ಪೂರ್ಣಾಂಕ i;
  ಚಿಹ್ನೆರಹಿತ struct_ಸ್ಥಾನಾಂತರ = 0;

  /* We don't assume anything about the alignment of the source.  */
  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ಅಕ್ಷರ *ಅಂಶ_ತಳ;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      ಆದರೆ (i > 0)
        struct_ಸ್ಥಾನಾಂತರ = FFI_ALIGN(struct_ಸ್ಥಾನಾಂತರ, p->alignment);
      ಅಂಶ_ತಳ = dest + struct_ಸ್ಥಾನಾಂತರ;

      ಆಯ್ಕೆ (p->type)
	{
	  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_ಗಮ್ಯ_mem (p, ಅಂಶ_ತಳ, fprs, nf);
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	    ಆದರೆ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ದ್ವಿಗುಣ *)ಅಂಶ_ತಳ = *fprs++ ;
		(*nf) += 1;
	      }
	    ಅಂಶ_ತಳ += 8;
	  /* FALL THROUGH */
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ದ್ವಿಗುಣ *)ಅಂಶ_ತಳ = *fprs++ ;
		(*nf) += 1;
	      }
	    ನಿಲ್ಲಿಸು;
	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಆದರೆ (*nf < ಅಂಕೆ_FPR_ARG_REGISTERS)
	      {
		*(ದಶಮಾಂಶ *)ಅಂಶ_ತಳ = (ದಶಮಾಂಶ) *fprs++ ;
		(*nf) += 1;
	      }
	    ನಿಲ್ಲಿಸು;
	  ಪೂರ್ವನಿಯೋಜಿತ:
	    ನಿಲ್ಲಿಸು;    
	}
      /* now count the size of what we just used.  */
      struct_ಸ್ಥಾನಾಂತರ += p->ಗಾತ್ರ_2;
    }
  ಹಿಂತಿರುಗಿ fprs;
}

#ಆದರೆ_ಅಂತ್ಯ

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

ಸ್ಥಿರ ಶೂನ್ಯ
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  ಪೂರ್ಣಾಂಕ i;

  ಆದರೆ (s->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ;

  s->ಗಾತ್ರ_2 = 0;
  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ffi_type *p;
      ಪೂರ್ಣಾಂಕ align;
      
      p = s->elements[i];
      ಆದರೆ (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#ಆದರೆ defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#ಇಲ್ಲದಿದ್ದರೆ
      /* Natural alignment for the first item... */
      ಆದರೆ (i == 0)
	align = p->alignment;
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      ಇಲ್ಲದಿದ್ದರೆ
	/* .. or align is 4.  */
	align = 4;
#ಆದರೆ_ಅಂತ್ಯ
      /* Pad, if necessary, before adding the current item.  */
      s->ಗಾತ್ರ_2 = FFI_ALIGN(s->ಗಾತ್ರ_2, align) + p->ಗಾತ್ರ_2;
    }
  
  s->ಗಾತ್ರ_2 = FFI_ALIGN(s->ಗಾತ್ರ_2, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  ಆದರೆ (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
aix_adjust_aggregate_sizes (ffi_type *s, ಪೂರ್ಣಾಂಕ outer_most_type_or_ಮೊದಲ_member)
{
  ಪೂರ್ಣಾಂಕ i, nested_ಮೊದಲ_member=0, final_align, rc=0;

  ಆದರೆ (s->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ 0;

  s->ಗಾತ್ರ_2 = 0;
  ಗಾಗಿ (i = 0; s->elements[i] != ಖಾಲಿ; i++)
    {
      ffi_type p;
      ಪೂರ್ಣಾಂಕ align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      ಆದರೆ (i == 0)
        nested_ಮೊದಲ_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_ಮೊದಲ_member);
      ಇಲ್ಲದಿದ್ದರೆ
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      ಆದರೆ (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->ಗಾತ್ರ_2 = FFI_ALIGN(s->ಗಾತ್ರ_2, align) + p.ಗಾತ್ರ_2;
    }

  final_align=s->alignment;
  ಆದರೆ ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_ಮೊದಲ_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      ಆದರೆ (outer_most_type_or_ಮೊದಲ_member)
        s->alignment=final_align;
  }

  s->ಗಾತ್ರ_2 = FFI_ALIGN(s->ಗಾತ್ರ_2, final_align);
  ಹಿಂತಿರುಗಿ rc;
}

/* Perform machine dependent cif processing.  */
ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  ಚಿಹ್ನೆರಹಿತ i;
  ffi_type **ptr;
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು;
  ಚಿಹ್ನೆರಹಿತ fparg_ಎಣಿಕೆ = 0, intarg_ಎಣಿಕೆ = 0;
  ಚಿಹ್ನೆರಹಿತ flags = 0;
  ಚಿಹ್ನೆರಹಿತ ಗಾತ್ರ_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  ಆದರೆ (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      ಗಾಗಿ (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  ಆದರೆ (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      ಗಾಗಿ (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  ಬೈಟುಗಳು = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ);

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
  ಆಯ್ಕೆ (cif->rtype->type)
    {

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು;
      flags |= FLAG_RETURNS_FP;
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      /* Fall through.  */
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
#ಆದರೆ_ಅಂತ್ಯ
      flags |= FLAG_RETURNS_64_ಬಿಟ್ಟುಗಳು;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
#ಆದರೆ defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	ಆದರೆ (darwin64_struct_ret_by_ಮೌಲ್ಯ_p (cif->rtype))
	  {
	    ಚಿಹ್ನೆರಹಿತ nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 != 16)
	      darwin64_ಪರಿಶೀಲನೆ_struct_for_floats (cif->rtype, &nfpr) ;
	    ಇಲ್ಲದಿದ್ದರೆ
	      flags |= FLAG_RETURNS_128_ಬಿಟ್ಟುಗಳು;
	    /* Will be 0 for 16byte struct.  */
	    ಆದರೆ (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	ಇಲ್ಲದಿದ್ದರೆ /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_ಎಣಿಕೆ++;
	  }
      }
#ಅಥವಾಆದರೆ defined(DARWIN_PPC)
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      ಇಲ್ಲದಿದ್ದರೆ /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_ಎಣಿಕೆ++;
	}
#ಇಲ್ಲದಿದ್ದರೆ /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      ನಿಲ್ಲಿಸು;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      ಚಿಹ್ನೆರಹಿತ align_ಪದಗಳು;
      ಆಯ್ಕೆ ((*ptr)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  fparg_ಎಣಿಕೆ++;
#ಆದರೆ !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  ಆದರೆ (fparg_ಎಣಿಕೆ > ಅಂಕೆ_FPR_ARG_REGISTERS
	      && (intarg_ಎಣಿಕೆ & 0x01) != 0)
	    intarg_ಎಣಿಕೆ++;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  fparg_ಎಣಿಕೆ += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  ಆದರೆ (fparg_ಎಣಿಕೆ >= ಅಂಕೆ_FPR_ARG_REGISTERS)
#ಆದರೆ defined (POWERPC64)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, 2);
#ಇಲ್ಲದಿದ್ದರೆ
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, 4);
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
#ಆದರೆ defined(POWERPC64)
	  intarg_ಎಣಿಕೆ++;
#ಇಲ್ಲದಿದ್ದರೆ
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  ಆದರೆ (intarg_ಎಣಿಕೆ == ಅಂಕೆ_GPR_ARG_REGISTERS-1
	      || (intarg_ಎಣಿಕೆ >= ಅಂಕೆ_GPR_ARG_REGISTERS 
	          && (intarg_ಎಣಿಕೆ & 0x01) != 0))
	    intarg_ಎಣಿಕೆ++;
	  intarg_ಎಣಿಕೆ += 2;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_al = (*ptr)->ಗಾತ್ರ_2;
#ಆದರೆ defined(POWERPC_DARWIN64)
	  align_ಪದಗಳು = (*ptr)->alignment >> 3;
	  ಆದರೆ (align_ಪದಗಳು)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, align_ಪದಗಳು);
	  /* Base size of the struct.  */
	  intarg_ಎಣಿಕೆ += (ಗಾತ್ರ_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  ಆದರೆ (ಗಾತ್ರ_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_ಪರಿಶೀಲನೆ_struct_for_floats (*ptr, &fparg_ಎಣಿಕೆ) ;
#ಇಲ್ಲದಿದ್ದರೆ
	  align_ಪದಗಳು = (*ptr)->alignment >> 2;
	  ಆದರೆ (align_ಪದಗಳು)
	    intarg_ಎಣಿಕೆ = FFI_ALIGN(intarg_ಎಣಿಕೆ, align_ಪದಗಳು);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_ಎಣಿಕೆ += (ಗಾತ್ರ_al + 7) / 8;
#  ಇಲ್ಲದಿದ್ದರೆ
	  intarg_ಎಣಿಕೆ += (ಗಾತ್ರ_al + 3) / 4;
#  endif
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_ಎಣಿಕೆ++;
	  ನಿಲ್ಲಿಸು;
	}
    }

  ಆದರೆ (fparg_ಎಣಿಕೆ != 0)
    flags |= FLAG_FP_ARGUMENTS;

#ಆದರೆ defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  ಆದರೆ (fparg_ಎಣಿಕೆ != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    ಬೈಟುಗಳು += ಅಂಕೆ_FPR_ARG_REGISTERS * ಗಾತ್ರ(ದ್ವಿಗುಣ);
#ಇಲ್ಲದಿದ್ದರೆ
  /* Space for the FPR registers, if needed.  */
  ಆದರೆ (fparg_ಎಣಿಕೆ != 0)
    ಬೈಟುಗಳು += ಅಂಕೆ_FPR_ARG_REGISTERS * ಗಾತ್ರ(ದ್ವಿಗುಣ);
#ಆದರೆ_ಅಂತ್ಯ

  /* Stack space.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ಆದರೆ ((intarg_ಎಣಿಕೆ + fparg_ಎಣಿಕೆ) > ಅಂಕೆ_GPR_ARG_REGISTERS)
    ಬೈಟುಗಳು += (intarg_ಎಣಿಕೆ + fparg_ಎಣಿಕೆ) * ಗಾತ್ರ(ದೀರ್ಘ);
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ ((intarg_ಎಣಿಕೆ + 2 * fparg_ಎಣಿಕೆ) > ಅಂಕೆ_GPR_ARG_REGISTERS)
    ಬೈಟುಗಳು += (intarg_ಎಣಿಕೆ + 2 * fparg_ಎಣಿಕೆ) * ಗಾತ್ರ(ದೀರ್ಘ);
#ಆದರೆ_ಅಂತ್ಯ
  ಇಲ್ಲದಿದ್ದರೆ
    ಬೈಟುಗಳು += ಅಂಕೆ_GPR_ARG_REGISTERS * ಗಾತ್ರ(ದೀರ್ಘ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  ಬೈಟುಗಳು = FFI_ALIGN(ಬೈಟುಗಳು, 16) ;

  cif->flags = flags;
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_AIX(extended_cif *, ದೀರ್ಘ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *,
			 ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ));

#ಆದರೆ defined (FFI_GO_CLOSURES)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_go_AIX(extended_cif *, ದೀರ್ಘ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *,
			    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ);
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_DARWIN(extended_cif *, ದೀರ್ಘ, ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *,
			    ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ (*fn2)(ಶೂನ್ಯ), ffi_type*);

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಆದರೆ ((rvalue == ಖಾಲಿ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_AIX:
      ffi_call_AIX(&ecif, -(ದೀರ್ಘ)cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_ವಾದಗಳು));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(ದೀರ್ಘ)cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_ವಾದಗಳು), cif->rtype);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}

#ಆದರೆ defined (FFI_GO_CLOSURES)
ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *ಆವರಣ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  ಆದರೆ ((rvalue == ಖಾಲಿ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
    }
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
    {
    ಸ್ಥಿತಿ FFI_AIX:
      ffi_call_go_AIX(&ecif, -(ದೀರ್ಘ)cif->ಬೈಟುಗಳು, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_ವಾದಗಳು), ಆವರಣ);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ flush_icache(ಅಕ್ಷರ *);
ಸ್ಥಿರ ಶೂನ್ಯ flush_range(ಅಕ್ಷರ *, ಪೂರ್ಣಾಂಕ);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ aix_fd_struct {
  ಶೂನ್ಯ *ಸಂಕೇತ_ಸೂಚಕ;
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

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp;
  ರಚನೆ ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  ಆಯ್ಕೆ (cif->abi)
    {
      ಸ್ಥಿತಿ FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
#ಆದರೆ defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)&tramp[2]) = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_ASM; /* function  */
	*((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)&tramp[4]) = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) codeloc; /* context  */
#ಇಲ್ಲದಿದ್ದರೆ
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_ASM; /* function  */
	tramp[3] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) codeloc; /* context  */
#ಆದರೆ_ಅಂತ್ಯ
	ಆವರಣ->cif = cif;
	ಆವರಣ->fun = fun;
	ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_ಗಾತ್ರ);

	ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_AIX:

      tramp_aix = (ರಚನೆ ffi_aix_trampoline_struct *) (ಆವರಣ->tramp);
      fd = (aix_fd *)(ಶೂನ್ಯ *)ffi_ಆವರಣ_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->ಸಂಕೇತ_ಸೂಚಕ = fd->ಸಂಕೇತ_ಸೂಚಕ;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      ಆವರಣ->cif = cif;
      ಆವರಣ->fun = fun;
      ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
      ನಿಲ್ಲಿಸು;
    }
  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ defined (FFI_GO_CLOSURES)
ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ,
		     ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಆಯ್ಕೆ (cif->abi)
    {
      ಸ್ಥಿತಿ FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        ಆವರಣ->tramp = (ಶೂನ್ಯ *)ffi_go_ಆವರಣ_ASM;
        ಆವರಣ->cif = cif;
        ಆವರಣ->fun = fun;
        ಹಿಂತಿರುಗಿ FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      ಪೂರ್ವನಿಯೋಜಿತ:
        ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
        ನಿಲ್ಲಿಸು;
    }
  ಹಿಂತಿರುಗಿ FFI_OK;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ
flush_icache(ಅಕ್ಷರ *addr)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _AIX
  __asm__ ಅಸ್ಥಿರ (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ
flush_range(ಅಕ್ಷರ * addr1, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
#ವ್ಯಾಖ್ಯಾನಿಸು MIN_ಸಾಲು_ಗಾತ್ರ 32
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_2; i += MIN_ಸಾಲು_ಗಾತ್ರ)
    flush_icache(addr1+i);
  flush_icache(addr1+ಗಾತ್ರ_2-1);
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಒಕ್ಕೂಟ
{
  ದಶಮಾಂಶ f;
  ದ್ವಿಗುಣ d;
} ffi_dblfl;

ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_DARWIN (ffi_ಆವರಣ *, ಶೂನ್ಯ *,
			   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *, ffi_dblfl *);

#ಆದರೆ defined (FFI_GO_CLOSURES)
ಪೂರ್ಣಾಂಕ
ffi_go_ಆವರಣ_helper_DARWIN (ffi_go_ಆವರಣ*, ಶೂನ್ಯ *,
			      ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *, ffi_dblfl *);
#ಆದರೆ_ಅಂತ್ಯ

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_common (ffi_cif* cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *rvalue,
			   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ದ್ವಿಗುಣ ldbits[2];

  ಒಕ್ಕೂಟ ldu
  {
    ldbits lb;
    ದೀರ್ಘ ದ್ವಿಗುಣ ld;
  };

  ಶೂನ್ಯ **          avalue;
  ffi_type **      arg_types;
  ದೀರ್ಘ             i, avn;
  ffi_dblfl *      ಕೊನೆ_pfr = pfr + ಅಂಕೆ_FPR_ARG_REGISTERS;
  ಚಿಹ್ನೆರಹಿತ         ಗಾತ್ರ_al;
  ಪೂರ್ಣಾಂಕ              struct_ret_by_ಮೌಲ್ಯ = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *  pgr0 = pgr;
#ಆದರೆ defined(POWERPC_DARWIN64)
  ಚಿಹ್ನೆರಹಿತ 	   fpsused = 0;
#ಆದರೆ_ಅಂತ್ಯ

  avalue = alloca (cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#ಆದರೆ defined(POWERPC_DARWIN64)
      ಆದರೆ (!darwin64_struct_ret_by_ಮೌಲ್ಯ_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (ಶೂನ್ಯ *) *pgr;
	  pgr++;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	struct_ret_by_ಮೌಲ್ಯ = 1;
#ಅಥವಾಆದರೆ defined(DARWIN_PPC)
      ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 > 4)
	{
	  rvalue = (ಶೂನ್ಯ *) *pgr;
	  pgr++;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	struct_ret_by_ಮೌಲ್ಯ = 1;
#ಇಲ್ಲದಿದ್ದರೆ /* assume we return by ref.  */
      rvalue = (ಶೂನ್ಯ *) *pgr;
      pgr++;
#ಆದರೆ_ಅಂತ್ಯ
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಆಗ (i < avn)
    {
      ಆಯ್ಕೆ (arg_types[i]->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
#ಆದರೆ  defined(POWERPC64)
	  avalue[i] = (ಅಕ್ಷರ *) pgr + 7;
#ಇಲ್ಲದಿದ್ದರೆ
	  avalue[i] = (ಅಕ್ಷರ *) pgr + 3;
#ಆದರೆ_ಅಂತ್ಯ
	  pgr++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
#ಆದರೆ  defined(POWERPC64)
	  avalue[i] = (ಅಕ್ಷರ *) pgr + 6;
#ಇಲ್ಲದಿದ್ದರೆ
	  avalue[i] = (ಅಕ್ಷರ *) pgr + 2;
#ಆದರೆ_ಅಂತ್ಯ
	  pgr++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
#ಆದರೆ  defined(POWERPC64)
	  avalue[i] = (ಅಕ್ಷರ *) pgr + 4;
#ಇಲ್ಲದಿದ್ದರೆ
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = pgr;
#ಆದರೆ_ಅಂತ್ಯ
	  pgr++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_al = arg_types[i]->ಗಾತ್ರ_2;
#ಆದರೆ defined(POWERPC_DARWIN64)
	  pgr = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *)FFI_ALIGN((ಅಕ್ಷರ *)pgr, arg_types[i]->alignment);
	  ಆದರೆ (ಗಾತ್ರ_al < 3 || ಗಾತ್ರ_al == 4)
	    {
	      avalue[i] = ((ಅಕ್ಷರ *)pgr)+8-ಗಾತ್ರ_al;
	      ಆದರೆ (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < ಅಂಕೆ_FPR_ARG_REGISTERS)
		{
		  *(ದಶಮಾಂಶ *)pgr = (ದಶಮಾಂಶ) *(ದ್ವಿಗುಣ *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  ಇಲ್ಲದಿದ್ದರೆ 
	    {
	      ಆದರೆ (ಗಾತ್ರ_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_ಗಮ್ಯ_mem (arg_types[i], (ಅಕ್ಷರ *)pgr,
						   (ದ್ವಿಗುಣ *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (ಗಾತ್ರ_al + 7) / 8;
#ಇಲ್ಲದಿದ್ದರೆ
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  ಆದರೆ (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    ಗಾತ್ರ_al = FFI_ALIGN(arg_types[i]->ಗಾತ್ರ_2, 8);
#  ಆದರೆ defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (ಗಾತ್ರ_al + 7) / 8;
#  ಇಲ್ಲದಿದ್ದರೆ
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  ಆದರೆ (ಗಾತ್ರ_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (ಅಕ್ಷರ*) pgr + 4 - ಗಾತ್ರ_al;
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = pgr;
	  pgr += (ಗಾತ್ರ_al + 3) / 4;
#  endif
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
#ಆದರೆ  defined(POWERPC64)
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = pgr;
	  pgr++;
	  ನಿಲ್ಲಿಸು;
#ಇಲ್ಲದಿದ್ದರೆ
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  ಆದರೆ (pfr < ಕೊನೆ_pfr)
	    {
	      ದ್ವಿಗುಣ ತಾತ್ಕಾಲಿಕ = pfr->d;
	      pfr->f = (ದಶಮಾಂಶ) ತಾತ್ಕಾಲಿಕ;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  ಆದರೆ (pfr < ಕೊನೆ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = pgr;
	    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
	  pgr++;
#ಇಲ್ಲದಿದ್ದರೆ
	  pgr += 2;
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;

#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
	  ಆದರೆ (pfr + 1 < ಕೊನೆ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಆದರೆ (pfr < ಕೊನೆ_pfr)
		{
		  *pgr = *(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#ಇಲ್ಲದಿದ್ದರೆ  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  ಆದರೆ (pfr + 1 < ಕೊನೆ_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pfr + 1 == ಕೊನೆ_pfr)
	    {
	      ಒಕ್ಕೂಟ ldu ತಾತ್ಕಾಲಿಕ_ld;
	      ಸ್ಮರಣೆ_ನಕಲು (&ತಾತ್ಕಾಲಿಕ_ld.lb[0], pfr, ಗಾತ್ರ(ldbits));
	      ಸ್ಮರಣೆ_ನಕಲು (&ತಾತ್ಕಾಲಿಕ_ld.lb[1], pgr + 2, ಗಾತ್ರ(ldbits));
	      avalue[i] = &ತಾತ್ಕಾಲಿಕ_ld.ld;
	      pfr++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#ಆದರೆ_ಅಂತ್ಯ  /* POWERPC64 */
	  ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ವನಿಯೋಜಿತ:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      ಆದರೆ (struct_ret_by_ಮೌಲ್ಯ)
	{
	  *pgr0 = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) cif->rtype;
	  ಹಿಂತಿರುಗಿ PPC_LD_STRUCT;
	}
      ಹಿಂತಿರುಗಿ PPC_LD_NONE;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಹಿಂತಿರುಗಿ PPC_LD_F32;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ಹಿಂತಿರುಗಿ PPC_LD_F64;
#ಆದರೆ FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      ಹಿಂತಿರುಗಿ PPC_LD_F128;
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_U8;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಹಿಂತಿರುಗಿ PPC_LD_S8;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_U16;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಹಿಂತಿರುಗಿ PPC_LD_S16;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_U32;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಹಿಂತಿರುಗಿ PPC_LD_S32;
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      ಹಿಂತಿರುಗಿ PPC_LD_PTR;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ಹಿಂತಿರುಗಿ PPC_LD_I64;
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
}

ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_helper_DARWIN (ffi_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *rvalue,
			   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pgr, ffi_dblfl *pfr)
{
  ಹಿಂತಿರುಗಿ ffi_ಆವರಣ_helper_common (ಆವರಣ->cif, ಆವರಣ->fun,
				    ಆವರಣ->user_ದತ್ತಾಂಶ, rvalue, pgr, pfr);
}

#ಆದರೆ defined (FFI_GO_CLOSURES)
ಪೂರ್ಣಾಂಕ
ffi_go_ಆವರಣ_helper_DARWIN (ffi_go_ಆವರಣ *ಆವರಣ, ಶೂನ್ಯ *rvalue,
			      ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *pgr, ffi_dblfl *pfr)
{
  ಹಿಂತಿರುಗಿ ffi_ಆವರಣ_helper_common (ಆವರಣ->cif, ಆವರಣ->fun,
				    ಆವರಣ, rvalue, pgr, pfr);
}
#ಆದರೆ_ಅಂತ್ಯ

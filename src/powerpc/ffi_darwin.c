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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>


काष्ठा ffi_aix_trampoline_struct {
    व्योम * code_pointer;	/* Pointer to ffi_closure_ASM */
    व्योम * toc;			/* TOC */
    व्योम * static_chain;	/* Pointer to closure */
};

/* Closure jump table indexes returned by ffi_closure_helper_common and
   consumed by the jump tables in aix_closure.S.  These mirror the
   definitions in ffi_powerpc.h, which this file cannot include because
   it defines its own (differing) FLAG_* and ffi_dblfl.  */
#घोषणा PPC_LD_NONE		0
#घोषणा PPC_LD_R3		1
#घोषणा PPC_LD_R3R4		2
#घोषणा PPC_LD_F32		3
#घोषणा PPC_LD_F64		4
#घोषणा PPC_LD_F128		5
#घोषणा PPC_LD_U8		6
#घोषणा PPC_LD_S8		7
#घोषणा PPC_LD_U16		8
#घोषणा PPC_LD_S16		9

#अगर_अघोषित POWERPC64
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
#पूर्ण_अगर

बाह्य व्योम ffi_closure_ASM (व्योम);

#अगर defined (FFI_GO_CLOSURES)
बाह्य व्योम ffi_go_closure_ASM (व्योम);
#पूर्ण_अगर

क्रमागत {
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
क्रमागत {
  अंक_GPR_ARG_REGISTERS = 8,
  अंक_FPR_ARG_REGISTERS = 13,
  LINKAGE_AREA_GPRS = 6
};

क्रमागत { ASM_NEEDS_REGISTERS = 4 }; /* r28-r31 */

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

#अगर defined(POWERPC_DARWIN64)
अटल व्योम
darwin64_pass_struct_by_मूल्य 
  (ffi_type *, अक्षर *, अचिन्हित, अचिन्हित *, द्विगुन **, अचिन्हित दीर्घ **);
#पूर्ण_अगर

/* This depends on GPR_SIZE = sizeof (unsigned long) */

व्योम
ffi_prep_args (extended_cif *ecif, अचिन्हित दीर्घ *स्थिर ढेर)
{
  स्थिर अचिन्हित bytes = ecif->cif->bytes;
  स्थिर अचिन्हित flags = ecif->cif->flags;
  स्थिर अचिन्हित nargs = ecif->cif->nargs;
#अगर !defined(POWERPC_DARWIN64) 
  स्थिर ffi_abi abi = ecif->cif->abi;
#पूर्ण_अगर

  /* 'stacktop' points at the previous backchain pointer.  */
  अचिन्हित दीर्घ *स्थिर stacktop = ढेर + (bytes / माप(अचिन्हित दीर्घ));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  द्विगुन *fpr_base = (द्विगुन *) (stacktop - ASM_NEEDS_REGISTERS) - अंक_FPR_ARG_REGISTERS;
  पूर्णांक gp_गिनती = 0, fparg_गिनती = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  अचिन्हित दीर्घ *अग्र_arg = ढेर + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  पूर्णांक i;
  द्विगुन double_tmp;
  व्योम **p_argv = ecif->avalue;
  अचिन्हित दीर्घ gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#अगर !defined(POWERPC_DARWIN64) 
  अक्षर *dest_cpy;
#पूर्ण_अगर
  अचिन्हित आकार_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((अचिन्हित) (अक्षर *) ढेर & 0xF) == 0);
  FFI_ASSERT(((अचिन्हित) (अक्षर *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  अगर (flags & FLAG_RETVAL_REFERENCE)
    *अग्र_arg++ = (अचिन्हित दीर्घ) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  क्रम (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      चयन ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	हाल FFI_TYPE_FLOAT:
	  double_tmp = *(भग्न *) *p_argv;
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#अगर defined(POWERPC_DARWIN)
	  *(भग्न *)अग्र_arg = *(भग्न *) *p_argv;
#अन्यथा
	  *(द्विगुन *)अग्र_arg = double_tmp;
#पूर्ण_अगर
	  अग्र_arg++;
	  gp_गिनती++;
	  fparg_गिनती++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  double_tmp = *(द्विगुन *) *p_argv;
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुन *)अग्र_arg = double_tmp;
#अगर_घोषित POWERPC64
	  अग्र_arg++;
	  gp_गिनती++;
#अन्यथा
	  अग्र_arg += 2;
	  gp_गिनती += 2;
#पूर्ण_अगर
	  fparg_गिनती++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  अवरोध;

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	हाल FFI_TYPE_LONGDOUBLE:
#  अगर defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *(दीर्घ द्विगुन *) fpr_base++ = *(दीर्घ द्विगुन *) *p_argv;
	  अन्यथा
	    *(दीर्घ द्विगुन *) अग्र_arg = *(दीर्घ द्विगुन *) *p_argv;
	  अग्र_arg += 2;
	  fparg_गिनती += 2;
#  अन्यथा
	  double_tmp = ((द्विगुन *) *p_argv)[0];
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुन *) अग्र_arg = double_tmp;
#    अगर defined(POWERPC_DARWIN64)
	  अग्र_arg++;
	  gp_गिनती++;
#    अन्यथा
	  अग्र_arg += 2;
	  gp_गिनती += 2;
#    endif
	  fparg_गिनती++;
	  double_tmp = ((द्विगुन *) *p_argv)[1];
	  अगर (fparg_गिनती < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुन *) अग्र_arg = double_tmp;
#    अगर defined(POWERPC_DARWIN64)
	  अग्र_arg++;
	  gp_गिनती++;
#    अन्यथा
	  अग्र_arg += 2;
	  gp_गिनती += 2;
#    endif
	  fparg_गिनती++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  अवरोध;
#पूर्ण_अगर
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
#अगर_घोषित POWERPC64
	  gprvalue = *(दीर्घ दीर्घ *) *p_argv;
	  जाओ putgpr;
#अन्यथा
	  *(दीर्घ दीर्घ *) अग्र_arg = *(दीर्घ दीर्घ *) *p_argv;
	  अग्र_arg += 2;
	  gp_गिनती += 2;
#पूर्ण_अगर
	  अवरोध;
	हाल FFI_TYPE_POINTER:
	  gprvalue = *(अचिन्हित दीर्घ *) *p_argv;
	  जाओ putgpr;
	हाल FFI_TYPE_UINT8:
	  gprvalue = *(अचिन्हित अक्षर *) *p_argv;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT8:
	  gprvalue = *(चिन्हित अक्षर *) *p_argv;
	  जाओ putgpr;
	हाल FFI_TYPE_UINT16:
	  gprvalue = *(अचिन्हित लघु *) *p_argv;
	  जाओ putgpr;
	हाल FFI_TYPE_SINT16:
	  gprvalue = *(चिन्हित लघु *) *p_argv;
	  जाओ putgpr;

	हाल FFI_TYPE_STRUCT:
	  आकार_al = (*ptr)->आकार;
#अगर defined(POWERPC_DARWIN64)
	  अग्र_arg = (अचिन्हित दीर्घ *)FFI_ALIGN((अक्षर *)अग्र_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_मूल्य (*ptr, (अक्षर *) *p_argv, 
					 (अचिन्हित) आकार_al,
					 (अचिन्हित पूर्णांक *) &fparg_गिनती,
					 &fpr_base, &अग्र_arg);
#अन्यथा
	  dest_cpy = (अक्षर *) अग्र_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  अगर ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    आकार_al = FFI_ALIGN((*ptr)->आकार, 8);

#  अगर defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  स_नकल ((अक्षर *) dest_cpy, (अक्षर *) *p_argv, आकार_al);
	  अग्र_arg += (आकार_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  अगर (आकार_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - आकार_al;

	  स_नकल((अक्षर *) dest_cpy, (अक्षर *) *p_argv, आकार_al);
	  अग्र_arg += (आकार_al + 3) / 4;
#  endif
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT32:
	  gprvalue = *(चिन्हित पूर्णांक *) *p_argv;
	  जाओ putgpr;

	हाल FFI_TYPE_UINT32:
	  gprvalue = *(अचिन्हित पूर्णांक *) *p_argv;
	putgpr:
	  *अग्र_arg++ = gprvalue;
	  gp_गिनती++;
	  अवरोध;
	शेष:
	  अवरोध;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#अगर defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
अटल व्योम
darwin64_scan_struct_for_floats (ffi_type *s, अचिन्हित *nfpr)
{
  पूर्णांक i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      ffi_type *p = s->elements[i];
      चयन (p->type)
	{
	  हाल FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    अवरोध;
	  हाल FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    अवरोध;
	  हाल FFI_TYPE_DOUBLE:
	  हाल FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    अवरोध;
	  शेष:
	    अवरोध;    
	}
    }
}

अटल पूर्णांक
darwin64_struct_आकार_exceeds_gprs_p (ffi_type *s, अक्षर *src, अचिन्हित *nfpr)
{
  अचिन्हित struct_offset=0, i;

  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      अगर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      चयन (p->type)
	{
	  हाल FFI_TYPE_STRUCT:
	    अगर (darwin64_struct_आकार_exceeds_gprs_p (p, item_base, nfpr))
	      वापस 1;
	    अवरोध;
	  हाल FFI_TYPE_LONGDOUBLE:
	    अगर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      वापस 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  हाल FFI_TYPE_DOUBLE:
	    अगर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      वापस 1;
	    (*nfpr) += 1;
	    अवरोध;
	  हाल FFI_TYPE_FLOAT:
	    अगर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      वापस 1;
	    (*nfpr) += 1;
	    अवरोध;
	  शेष:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    अगर ((अचिन्हित दीर्घ)item_base >= 8*8) 
	      वापस 1;
	    अवरोध;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->आकार;
    }
  वापस 0;
}

/* Can this struct be returned by value?  */
पूर्णांक 
darwin64_struct_ret_by_मूल्य_p (ffi_type *s)
{
  अचिन्हित nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  अगर (s->आकार > 168)
    वापस 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  अगर (nfp > 13)
    वापस 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  अगर (s->आकार <= 64)
    वापस 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  अगर (darwin64_struct_आकार_exceeds_gprs_p (s, शून्य, &nfp))
    वापस 0;
  
  वापस 1;
}

व्योम
darwin64_pass_struct_floats (ffi_type *s, अक्षर *src, 
			     अचिन्हित *nfpr, द्विगुन **fprs)
{
  पूर्णांक i;
  द्विगुन *fpr_base = *fprs;
  अचिन्हित struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      अगर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      चयन (p->type)
	{
	  हाल FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    अवरोध;
	  हाल FFI_TYPE_LONGDOUBLE:
	    अगर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुन *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  हाल FFI_TYPE_DOUBLE:
	    अगर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुन *)item_base;
	    (*nfpr) += 1;
	    अवरोध;
	  हाल FFI_TYPE_FLOAT:
	    अगर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = (द्विगुन) *(भग्न *)item_base;
	    (*nfpr) += 1;
	    अवरोध;
	  शेष:
	    अवरोध;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->आकार;
    }
  /* Update the scores.  */
  *fprs = fpr_base;
}

/* Darwin64 special rules.
   Break out a struct into params and float registers.  */
अटल व्योम
darwin64_pass_struct_by_मूल्य (ffi_type *s, अक्षर *src, अचिन्हित आकार,
			       अचिन्हित *nfpr, द्विगुन **fprs, अचिन्हित दीर्घ **arg)
{
  अचिन्हित दीर्घ *अग्र_arg = *arg;
  अक्षर *dest_cpy = (अक्षर *)अग्र_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  अगर (!आकार)
    वापस;

  /* First... special cases.  */
  अगर (आकार < 3
      || (आकार == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *अग्र_arg = 0UL; 
      dest_cpy += 8 - आकार;
      स_नकल ((अक्षर *) dest_cpy, src, आकार);
      अग्र_arg++;
    }
  अन्यथा अगर (आकार == 16)
    {
      स_नकल ((अक्षर *) dest_cpy, src, आकार);
      अग्र_arg += 2;
    }
  अन्यथा
    {
      /* now the general case, we consider embedded floats.  */
      स_नकल ((अक्षर *) dest_cpy, src, आकार);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      अग्र_arg += (आकार+7)/8;
    }
    
  *arg = अग्र_arg;
}

द्विगुन *
darwin64_struct_floats_to_mem (ffi_type *s, अक्षर *dest, द्विगुन *fprs, अचिन्हित *nf)
{
  पूर्णांक i;
  अचिन्हित struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      अगर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      चयन (p->type)
	{
	  हाल FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    अवरोध;
	  हाल FFI_TYPE_LONGDOUBLE:
	    अगर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुन *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  हाल FFI_TYPE_DOUBLE:
	    अगर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुन *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    अवरोध;
	  हाल FFI_TYPE_FLOAT:
	    अगर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(भग्न *)item_base = (भग्न) *fprs++ ;
		(*nf) += 1;
	      }
	    अवरोध;
	  शेष:
	    अवरोध;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->आकार;
    }
  वापस fprs;
}

#पूर्ण_अगर

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

अटल व्योम
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  पूर्णांक i;

  अगर (s->type != FFI_TYPE_STRUCT)
    वापस;

  s->आकार = 0;
  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      ffi_type *p;
      पूर्णांक align;
      
      p = s->elements[i];
      अगर (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#अगर defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#अन्यथा
      /* Natural alignment for the first item... */
      अगर (i == 0)
	align = p->alignment;
      अन्यथा अगर (p->alignment == 16 || p->alignment < 4)
	/* .. subsequent items with vector or align < 4 have natural align.  */
	align = p->alignment;
      अन्यथा
	/* .. or align is 4.  */
	align = 4;
#पूर्ण_अगर
      /* Pad, if necessary, before adding the current item.  */
      s->आकार = FFI_ALIGN(s->आकार, align) + p->आकार;
    }
  
  s->आकार = FFI_ALIGN(s->आकार, s->alignment);
  
  /* This should not be necessary on m64, but harmless.  */
  अगर (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

अटल पूर्णांक
aix_adjust_aggregate_sizes (ffi_type *s, पूर्णांक outer_most_type_or_first_member)
{
  पूर्णांक i, nested_first_member=0, final_align, rc=0;

  अगर (s->type != FFI_TYPE_STRUCT)
    वापस 0;

  s->आकार = 0;
  क्रम (i = 0; s->elements[i] != शून्य; i++)
    {
      ffi_type p;
      पूर्णांक align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      अगर (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      अन्यथा
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      अगर (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->आकार = FFI_ALIGN(s->आकार, align) + p.आकार;
    }

  final_align=s->alignment;
  अगर ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      अगर (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->आकार = FFI_ALIGN(s->आकार, final_align);
  वापस rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  अचिन्हित i;
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित fparg_गिनती = 0, intarg_गिनती = 0;
  अचिन्हित flags = 0;
  अचिन्हित आकार_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  अगर (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      क्रम (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  अगर (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      क्रम (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * माप(अचिन्हित दीर्घ);

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
  चयन (cif->rtype->type)
    {

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      अवरोध;
#पूर्ण_अगर

    हाल FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    हाल FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      अवरोध;

    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
#अगर_घोषित POWERPC64
    हाल FFI_TYPE_POINTER:
#पूर्ण_अगर
      flags |= FLAG_RETURNS_64BITS;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
#अगर defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	अगर (darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	  {
	    अचिन्हित nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    अगर (cif->rtype->आकार != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    अन्यथा
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    अगर (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	अन्यथा /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_गिनती++;
	  }
      }
#या_अगर defined(DARWIN_PPC)
      अगर (cif->rtype->आकार <= 4)
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
#पूर्ण_अगर
      अवरोध;
    हाल FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      अवरोध;

    शेष:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      अवरोध;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित align_शब्दगण;
      चयन ((*ptr)->type)
	{
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	  fparg_गिनती++;
#अगर !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  अगर (fparg_गिनती > अंक_FPR_ARG_REGISTERS
	      && (intarg_गिनती & 0x01) != 0)
	    intarg_गिनती++;
#पूर्ण_अगर
	  अवरोध;

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	हाल FFI_TYPE_LONGDOUBLE:
	  fparg_गिनती += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  अगर (fparg_गिनती >= अंक_FPR_ARG_REGISTERS)
#अगर defined (POWERPC64)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, 2);
#अन्यथा
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, 4);
#पूर्ण_अगर
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_SINT64:
#अगर defined(POWERPC64)
	  intarg_गिनती++;
#अन्यथा
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  अगर (intarg_गिनती == अंक_GPR_ARG_REGISTERS-1
	      || (intarg_गिनती >= अंक_GPR_ARG_REGISTERS 
	          && (intarg_गिनती & 0x01) != 0))
	    intarg_गिनती++;
	  intarg_गिनती += 2;
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  आकार_al = (*ptr)->आकार;
#अगर defined(POWERPC_DARWIN64)
	  align_शब्दगण = (*ptr)->alignment >> 3;
	  अगर (align_शब्दगण)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, align_शब्दगण);
	  /* Base size of the struct.  */
	  intarg_गिनती += (आकार_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  अगर (आकार_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_गिनती) ;
#अन्यथा
	  align_शब्दगण = (*ptr)->alignment >> 2;
	  अगर (align_शब्दगण)
	    intarg_गिनती = FFI_ALIGN(intarg_गिनती, align_शब्दगण);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_गिनती += (आकार_al + 7) / 8;
#  अन्यथा
	  intarg_गिनती += (आकार_al + 3) / 4;
#  endif
#पूर्ण_अगर
	  अवरोध;

	शेष:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_गिनती++;
	  अवरोध;
	}
    }

  अगर (fparg_गिनती != 0)
    flags |= FLAG_FP_ARGUMENTS;

#अगर defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  अगर (fparg_गिनती != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += अंक_FPR_ARG_REGISTERS * माप(द्विगुन);
#अन्यथा
  /* Space for the FPR registers, if needed.  */
  अगर (fparg_गिनती != 0)
    bytes += अंक_FPR_ARG_REGISTERS * माप(द्विगुन);
#पूर्ण_अगर

  /* Stack space.  */
#अगर_घोषित POWERPC64
  अगर ((intarg_गिनती + fparg_गिनती) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_गिनती + fparg_गिनती) * माप(दीर्घ);
#अन्यथा
  अगर ((intarg_गिनती + 2 * fparg_गिनती) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_गिनती + 2 * fparg_गिनती) * माप(दीर्घ);
#पूर्ण_अगर
  अन्यथा
    bytes += अंक_GPR_ARG_REGISTERS * माप(दीर्घ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  वापस FFI_OK;
}

बाह्य व्योम ffi_call_AIX(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			 व्योम (*fn)(व्योम), व्योम (*fn2)(व्योम));

#अगर defined (FFI_GO_CLOSURES)
बाह्य व्योम ffi_call_go_AIX(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			    व्योम (*fn)(व्योम), व्योम (*fn2)(व्योम), व्योम *closure);
#पूर्ण_अगर

बाह्य व्योम ffi_call_DARWIN(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			    व्योम (*fn)(व्योम), व्योम (*fn2)(व्योम), ffi_type*);

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  अगर ((rvalue == शून्य) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_AIX:
      ffi_call_AIX(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      अवरोध;
    हाल FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }
}

#अगर defined (FFI_GO_CLOSURES)
व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue,
	     व्योम *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  अगर ((rvalue == शून्य) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_AIX:
      ffi_call_go_AIX(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }
}
#पूर्ण_अगर

अटल व्योम flush_icache(अक्षर *);
अटल व्योम flush_range(अक्षर *, पूर्णांक);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

प्रकार काष्ठा aix_fd_struct {
  व्योम *code_pointer;
  व्योम *toc;
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
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp;
  काष्ठा ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  चयन (cif->abi)
    {
      हाल FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
#अगर defined(POWERPC_DARWIN64)
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f0015;  /*   bcl-    20,4*cr7+so,  +0x18 (L1)  */
	/* We put the addresses here.  */
	tramp[6] = 0x7d6802a6;  /*L1:   mflr    r11  */
	tramp[7] = 0xe98b0000;  /*   ld     r12,0(r11) function address  */
	tramp[8] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[9] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[10] = 0xe96b0008;  /*   lwz     r11,8(r11) static chain  */
	tramp[11] = 0x4e800420;  /*   bctr  */

	*((अचिन्हित दीर्घ *)&tramp[2]) = (अचिन्हित दीर्घ) ffi_closure_ASM; /* function  */
	*((अचिन्हित दीर्घ *)&tramp[4]) = (अचिन्हित दीर्घ) codeloc; /* context  */
#अन्यथा
	tramp[0] = 0x7c0802a6;  /*   mflr    r0  */
	tramp[1] = 0x429f000d;  /*   bcl-    20,4*cr7+so,0x10  */
	tramp[4] = 0x7d6802a6;  /*   mflr    r11  */
	tramp[5] = 0x818b0000;  /*   lwz     r12,0(r11) function address  */
	tramp[6] = 0x7c0803a6;  /*   mtlr    r0   */
	tramp[7] = 0x7d8903a6;  /*   mtctr   r12  */
	tramp[8] = 0x816b0004;  /*   lwz     r11,4(r11) static chain  */
	tramp[9] = 0x4e800420;  /*   bctr  */
	tramp[2] = (अचिन्हित दीर्घ) ffi_closure_ASM; /* function  */
	tramp[3] = (अचिन्हित दीर्घ) codeloc; /* context  */
#पूर्ण_अगर
	closure->cif = cif;
	closure->fun = fun;
	closure->user_तथ्य = user_तथ्य;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_आकार);

	अवरोध;

    हाल FFI_AIX:

      tramp_aix = (काष्ठा ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(व्योम *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_तथ्य = user_तथ्य;
      अवरोध;

    शेष:
      वापस FFI_BAD_ABI;
      अवरोध;
    }
  वापस FFI_OK;
}

#अगर defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*))
{
  चयन (cif->abi)
    {
      हाल FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (व्योम *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        वापस FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      शेष:
        वापस FFI_BAD_ABI;
        अवरोध;
    }
  वापस FFI_OK;
}
#पूर्ण_अगर

अटल व्योम
flush_icache(अक्षर *addr)
{
#अगर_अघोषित _AIX
  __asm__ अस्थिर (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#पूर्ण_अगर
}

अटल व्योम
flush_range(अक्षर * addr1, पूर्णांक आकार)
{
#घोषणा MIN_पंक्ति_आकार 32
  पूर्णांक i;
  क्रम (i = 0; i < आकार; i += MIN_पंक्ति_आकार)
    flush_icache(addr1+i);
  flush_icache(addr1+आकार-1);
}

प्रकार जोड़
{
  भग्न f;
  द्विगुन d;
} ffi_dblfl;

पूर्णांक
ffi_closure_helper_DARWIN (ffi_closure *, व्योम *,
			   अचिन्हित दीर्घ *, ffi_dblfl *);

#अगर defined (FFI_GO_CLOSURES)
पूर्णांक
ffi_go_closure_helper_DARWIN (ffi_go_closure*, व्योम *,
			      अचिन्हित दीर्घ *, ffi_dblfl *);
#पूर्ण_अगर

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

अटल पूर्णांक
ffi_closure_helper_common (ffi_cif* cif,
			   व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
			   व्योम *user_तथ्य, व्योम *rvalue,
			   अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  प्रकार द्विगुन ldbits[2];

  जोड़ ldu
  {
    ldbits lb;
    दीर्घ द्विगुन ld;
  };

  व्योम **          avalue;
  ffi_type **      arg_types;
  दीर्घ             i, avn;
  ffi_dblfl *      end_pfr = pfr + अंक_FPR_ARG_REGISTERS;
  अचिन्हित         आकार_al;
  पूर्णांक              struct_ret_by_मूल्य = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  अचिन्हित दीर्घ *  pgr0 = pgr;
#अगर defined(POWERPC_DARWIN64)
  अचिन्हित 	   fpsused = 0;
#पूर्ण_अगर

  avalue = alloca (cif->nargs * माप(व्योम *));

  अगर (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#अगर defined(POWERPC_DARWIN64)
      अगर (!darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (व्योम *) *pgr;
	  pgr++;
	}
      अन्यथा
	struct_ret_by_मूल्य = 1;
#या_अगर defined(DARWIN_PPC)
      अगर (cif->rtype->आकार > 4)
	{
	  rvalue = (व्योम *) *pgr;
	  pgr++;
	}
      अन्यथा
	struct_ret_by_मूल्य = 1;
#अन्यथा /* assume we return by ref.  */
      rvalue = (व्योम *) *pgr;
      pgr++;
#पूर्ण_अगर
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  जबतक (i < avn)
    {
      चयन (arg_types[i]->type)
	{
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
#अगर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 7;
#अन्यथा
	  avalue[i] = (अक्षर *) pgr + 3;
#पूर्ण_अगर
	  pgr++;
	  अवरोध;

	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
#अगर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 6;
#अन्यथा
	  avalue[i] = (अक्षर *) pgr + 2;
#पूर्ण_अगर
	  pgr++;
	  अवरोध;

	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
#अगर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 4;
#अन्यथा
	हाल FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#पूर्ण_अगर
	  pgr++;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  आकार_al = arg_types[i]->आकार;
#अगर defined(POWERPC_DARWIN64)
	  pgr = (अचिन्हित दीर्घ *)FFI_ALIGN((अक्षर *)pgr, arg_types[i]->alignment);
	  अगर (आकार_al < 3 || आकार_al == 4)
	    {
	      avalue[i] = ((अक्षर *)pgr)+8-आकार_al;
	      अगर (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < अंक_FPR_ARG_REGISTERS)
		{
		  *(भग्न *)pgr = (भग्न) *(द्विगुन *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  अन्यथा 
	    {
	      अगर (आकार_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (अक्षर *)pgr,
						   (द्विगुन *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (आकार_al + 7) / 8;
#अन्यथा
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  अगर (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    आकार_al = FFI_ALIGN(arg_types[i]->आकार, 8);
#  अगर defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (आकार_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  अगर (आकार_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (अक्षर*) pgr + 4 - आकार_al;
	  अन्यथा
	    avalue[i] = pgr;
	  pgr += (आकार_al + 3) / 4;
#  endif
#पूर्ण_अगर
	  अवरोध;

	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
#अगर  defined(POWERPC64)
	हाल FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  अवरोध;
#अन्यथा
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  अवरोध;
#पूर्ण_अगर

	हाल FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  अगर (pfr < end_pfr)
	    {
	      द्विगुन अस्थायी = pfr->d;
	      pfr->f = (भग्न) अस्थायी;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  अगर (pfr < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
#अगर_घोषित POWERPC64
	  pgr++;
#अन्यथा
	  pgr += 2;
#पूर्ण_अगर
	  अवरोध;

#अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	हाल FFI_TYPE_LONGDOUBLE:
#अगर_घोषित POWERPC64
	  अगर (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  अन्यथा
	    {
	      अगर (pfr < end_pfr)
		{
		  *pgr = *(अचिन्हित दीर्घ *) pfr;
		  pfr++;
		}
	      avalue[i] = pgr;
	    }
	  pgr += 2;
#अन्यथा  /* POWERPC64 */
	  /* A long double value consumes four GPRs and two FPRs.
	     There are 13 64bit floating point registers.  */
	  अगर (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  अन्यथा अगर (pfr + 1 == end_pfr)
	    {
	      जोड़ ldu अस्थायी_ld;
	      स_नकल (&अस्थायी_ld.lb[0], pfr, माप(ldbits));
	      स_नकल (&अस्थायी_ld.lb[1], pgr + 2, माप(ldbits));
	      avalue[i] = &अस्थायी_ld.ld;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#पूर्ण_अगर  /* POWERPC64 */
	  अवरोध;
#पूर्ण_अगर
	शेष:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_तथ्य);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      वापस PPC_LD_NONE;
    हाल FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      अगर (struct_ret_by_मूल्य)
	{
	  *pgr0 = (अचिन्हित दीर्घ) cif->rtype;
	  वापस PPC_LD_STRUCT;
	}
      वापस PPC_LD_NONE;
    हाल FFI_TYPE_FLOAT:
      वापस PPC_LD_F32;
    हाल FFI_TYPE_DOUBLE:
      वापस PPC_LD_F64;
#अगर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    हाल FFI_TYPE_LONGDOUBLE:
      वापस PPC_LD_F128;
#पूर्ण_अगर
    हाल FFI_TYPE_UINT8:
      वापस PPC_LD_U8;
    हाल FFI_TYPE_SINT8:
      वापस PPC_LD_S8;
    हाल FFI_TYPE_UINT16:
      वापस PPC_LD_U16;
    हाल FFI_TYPE_SINT16:
      वापस PPC_LD_S16;
    हाल FFI_TYPE_UINT32:
      वापस PPC_LD_U32;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
      वापस PPC_LD_S32;
    हाल FFI_TYPE_POINTER:
      वापस PPC_LD_PTR;
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      वापस PPC_LD_I64;
    शेष:
      पात();
    }
}

पूर्णांक
ffi_closure_helper_DARWIN (ffi_closure *closure, व्योम *rvalue,
			   अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  वापस ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_तथ्य, rvalue, pgr, pfr);
}

#अगर defined (FFI_GO_CLOSURES)
पूर्णांक
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, व्योम *rvalue,
			      अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  वापस ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#पूर्ण_अगर

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

#जर defined (FFI_GO_CLOSURES)
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

#जर defined(POWERPC_DARWIN64)
स्थिर शून्य
darwin64_pass_struct_by_मूल्य 
  (ffi_type *, अक्षर *, अचिन्हित, अचिन्हित *, द्विगुण **, अचिन्हित दीर्घ **);
#endif

/* This depends on GPR_SIZE = sizeof (unsigned long) */

शून्य
ffi_prep_args (extended_cif *ecif, अचिन्हित दीर्घ *स्थिरांक चळत)
{
  स्थिरांक अचिन्हित bytes = ecif->cif->bytes;
  स्थिरांक अचिन्हित flags = ecif->cif->flags;
  स्थिरांक अचिन्हित nargs = ecif->cif->nargs;
#जर !defined(POWERPC_DARWIN64) 
  स्थिरांक ffi_abi abi = ecif->cif->abi;
#endif

  /* 'stacktop' points at the previous backchain pointer.  */
  अचिन्हित दीर्घ *स्थिरांक stacktop = चळत + (bytes / आकार(अचिन्हित दीर्घ));

  /* 'fpr_base' points at the space for fpr1, and grows upwards as
     we use FPR registers.  */
  द्विगुण *fpr_base = (द्विगुण *) (stacktop - ASM_NEEDS_REGISTERS) - अंक_FPR_ARG_REGISTERS;
  पूर्णांक gp_मोजणी = 0, fparg_मोजणी = 0;

  /* 'next_arg' grows up as we put parameters in it.  */
  अचिन्हित दीर्घ *next_arg = चळत + LINKAGE_AREA_GPRS; /* 6 reserved positions.  */

  पूर्णांक i;
  द्विगुण double_tmp;
  शून्य **p_argv = ecif->avalue;
  अचिन्हित दीर्घ gprvalue;
  ffi_type** ptr = ecif->cif->arg_types;
#जर !defined(POWERPC_DARWIN64) 
  अक्षर *dest_cpy;
#endif
  अचिन्हित size_al = 0;

  /* Check that everything starts aligned properly.  */
  FFI_ASSERT(((अचिन्हित) (अक्षर *) चळत & 0xF) == 0);
  FFI_ASSERT(((अचिन्हित) (अक्षर *) stacktop & 0xF) == 0);
  FFI_ASSERT((bytes & 0xF) == 0);

  /* Deal with return values that are actually pass-by-reference.
     Rule:
     Return values are referenced by r3, so r4 is the first parameter.  */

  जर (flags & FLAG_RETVAL_REFERENCE)
    *next_arg++ = (अचिन्हित दीर्घ) (अक्षर *) ecif->rvalue;

  /* Now for the arguments.  */
  साठी (i = nargs; i > 0; i--, ptr++, p_argv++)
    {
      निवड ((*ptr)->type)
	{
	/* If a floating-point parameter appears before all of the general-
	   purpose registers are filled, the corresponding GPRs that match
	   the size of the floating-point parameter are skipped.  */
	स्थिती FFI_TYPE_FLOAT:
	  double_tmp = *(दशांश *) *p_argv;
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
#जर defined(POWERPC_DARWIN)
	  *(दशांश *)next_arg = *(दशांश *) *p_argv;
#अन्यथा
	  *(द्विगुण *)next_arg = double_tmp;
#endif
	  next_arg++;
	  gp_मोजणी++;
	  fparg_मोजणी++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  double_tmp = *(द्विगुण *) *p_argv;
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *)next_arg = double_tmp;
#ifdef POWERPC64
	  next_arg++;
	  gp_मोजणी++;
#अन्यथा
	  next_arg += 2;
	  gp_मोजणी += 2;
#endif
	  fparg_मोजणी++;
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  थांबा;

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	स्थिती FFI_TYPE_LONGDOUBLE:
#  जर defined(POWERPC64) && !defined(POWERPC_DARWIN64)
	  /* ??? This will exceed the regs count when the value starts at fp13
	     and it will not put the extra bit on the stack.  */
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS)
	    *(दीर्घ द्विगुण *) fpr_base++ = *(दीर्घ द्विगुण *) *p_argv;
	  अन्यथा
	    *(दीर्घ द्विगुण *) next_arg = *(दीर्घ द्विगुण *) *p_argv;
	  next_arg += 2;
	  fparg_मोजणी += 2;
#  अन्यथा
	  double_tmp = ((द्विगुण *) *p_argv)[0];
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *) next_arg = double_tmp;
#    जर defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_मोजणी++;
#    अन्यथा
	  next_arg += 2;
	  gp_मोजणी += 2;
#    endif
	  fparg_मोजणी++;
	  double_tmp = ((द्विगुण *) *p_argv)[1];
	  जर (fparg_मोजणी < अंक_FPR_ARG_REGISTERS)
	    *fpr_base++ = double_tmp;
	  *(द्विगुण *) next_arg = double_tmp;
#    जर defined(POWERPC_DARWIN64)
	  next_arg++;
	  gp_मोजणी++;
#    अन्यथा
	  next_arg += 2;
	  gp_मोजणी += 2;
#    endif
	  fparg_मोजणी++;
#  endif
	  FFI_ASSERT(flags & FLAG_FP_ARGUMENTS);
	  थांबा;
#endif
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
#ifdef POWERPC64
	  gprvalue = *(दीर्घ दीर्घ *) *p_argv;
	  जा putgpr;
#अन्यथा
	  *(दीर्घ दीर्घ *) next_arg = *(दीर्घ दीर्घ *) *p_argv;
	  next_arg += 2;
	  gp_मोजणी += 2;
#endif
	  थांबा;
	स्थिती FFI_TYPE_POINTER:
	  gprvalue = *(अचिन्हित दीर्घ *) *p_argv;
	  जा putgpr;
	स्थिती FFI_TYPE_UINT8:
	  gprvalue = *(अचिन्हित अक्षर *) *p_argv;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT8:
	  gprvalue = *(चिन्हित अक्षर *) *p_argv;
	  जा putgpr;
	स्थिती FFI_TYPE_UINT16:
	  gprvalue = *(अचिन्हित लघु *) *p_argv;
	  जा putgpr;
	स्थिती FFI_TYPE_SINT16:
	  gprvalue = *(चिन्हित लघु *) *p_argv;
	  जा putgpr;

	स्थिती FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#जर defined(POWERPC_DARWIN64)
	  next_arg = (अचिन्हित दीर्घ *)FFI_ALIGN((अक्षर *)next_arg, (*ptr)->alignment);
	  darwin64_pass_struct_by_मूल्य (*ptr, (अक्षर *) *p_argv, 
					 (अचिन्हित) size_al,
					 (अचिन्हित पूर्णांक *) &fparg_मोजणी,
					 &fpr_base, &next_arg);
#अन्यथा
	  dest_cpy = (अक्षर *) next_arg;

	  /* If the first member of the struct is a double, then include enough
	     padding in the struct size to align it to double-word.  */
	  जर ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8);

#  जर defined(POWERPC64) 
	  FFI_ASSERT (abi != FFI_DARWIN);
	  स_नकल ((अक्षर *) dest_cpy, (अक्षर *) *p_argv, size_al);
	  next_arg += (size_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.
	     Structures with 3 byte in size are padded upwards.  */
	  जर (size_al < 3 && abi == FFI_DARWIN)
	    dest_cpy += 4 - size_al;

	  स_नकल((अक्षर *) dest_cpy, (अक्षर *) *p_argv, size_al);
	  next_arg += (size_al + 3) / 4;
#  endif
#endif
	  थांबा;

	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT32:
	  gprvalue = *(चिन्हित पूर्णांक *) *p_argv;
	  जा putgpr;

	स्थिती FFI_TYPE_UINT32:
	  gprvalue = *(अचिन्हित पूर्णांक *) *p_argv;
	putgpr:
	  *next_arg++ = gprvalue;
	  gp_मोजणी++;
	  थांबा;
	पूर्वनिर्धारित:
	  थांबा;
	}
    }

  /* Check that we didn't overrun the stack...  */
  /* FFI_ASSERT(gpr_base <= stacktop - ASM_NEEDS_REGISTERS);
     FFI_ASSERT((unsigned *)fpr_base
     	     <= stacktop - ASM_NEEDS_REGISTERS - NUM_GPR_ARG_REGISTERS);
     FFI_ASSERT(flags & FLAG_4_GPR_ARGUMENTS || intarg_count <= 4);  */
}

#जर defined(POWERPC_DARWIN64)

/* See if we can put some of the struct into fprs.
   This should not be called for structures of size 16 bytes, since these are not
   broken out this way.  */
स्थिर शून्य
darwin64_scan_struct_for_floats (ffi_type *s, अचिन्हित *nfpr)
{
  पूर्णांक i;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type *p = s->elements[i];
      निवड (p->type)
	{
	  स्थिती FFI_TYPE_STRUCT:
	    darwin64_scan_struct_for_floats (p, nfpr);
	    थांबा;
	  स्थिती FFI_TYPE_LONGDOUBLE:
	    (*nfpr) += 2;
	    थांबा;
	  स्थिती FFI_TYPE_DOUBLE:
	  स्थिती FFI_TYPE_FLOAT:
	    (*nfpr) += 1;
	    थांबा;
	  पूर्वनिर्धारित:
	    थांबा;    
	}
    }
}

स्थिर पूर्णांक
darwin64_struct_size_exceeds_gprs_p (ffi_type *s, अक्षर *src, अचिन्हित *nfpr)
{
  अचिन्हित struct_offset=0, i;

  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      जर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);

      item_base = src + struct_offset;

      निवड (p->type)
	{
	  स्थिती FFI_TYPE_STRUCT:
	    जर (darwin64_struct_size_exceeds_gprs_p (p, item_base, nfpr))
	      परत 1;
	    थांबा;
	  स्थिती FFI_TYPE_LONGDOUBLE:
	    जर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      परत 1;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  स्थिती FFI_TYPE_DOUBLE:
	    जर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      परत 1;
	    (*nfpr) += 1;
	    थांबा;
	  स्थिती FFI_TYPE_FLOAT:
	    जर (*nfpr >= अंक_FPR_ARG_REGISTERS)
	      परत 1;
	    (*nfpr) += 1;
	    थांबा;
	  पूर्वनिर्धारित:
	    /* If we try and place any item, that is non-float, once we've
	       exceeded the 8 GPR mark, then we can't fit the struct.  */
	    जर ((अचिन्हित दीर्घ)item_base >= 8*8) 
	      परत 1;
	    थांबा;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  परत 0;
}

/* Can this struct be returned by value?  */
पूर्णांक 
darwin64_struct_ret_by_मूल्य_p (ffi_type *s)
{
  अचिन्हित nfp = 0;

  FFI_ASSERT (s && s->type == FFI_TYPE_STRUCT);
  
  /* The largest structure we can return is 8long + 13 doubles.  */
  जर (s->size > 168)
    परत 0;
  
  /* We can't pass more than 13 floats.  */
  darwin64_scan_struct_for_floats (s, &nfp);
  जर (nfp > 13)
    परत 0;
  
  /* If there are not too many floats, and the struct is
     small enough to accommodate in the GPRs, then it must be OK.  */
  जर (s->size <= 64)
    परत 1;
  
  /* Well, we have to look harder.  */
  nfp = 0;
  जर (darwin64_struct_size_exceeds_gprs_p (s, रिक्त, &nfp))
    परत 0;
  
  परत 1;
}

शून्य
darwin64_pass_struct_floats (ffi_type *s, अक्षर *src, 
			     अचिन्हित *nfpr, द्विगुण **fprs)
{
  पूर्णांक i;
  द्विगुण *fpr_base = *fprs;
  अचिन्हित struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      जर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = src + struct_offset;

      निवड (p->type)
	{
	  स्थिती FFI_TYPE_STRUCT:
	    darwin64_pass_struct_floats (p, item_base, nfpr,
					   &fpr_base);
	    थांबा;
	  स्थिती FFI_TYPE_LONGDOUBLE:
	    जर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुण *)item_base;
	    (*nfpr) += 1;
	    item_base += 8;
	  /* FALL THROUGH */
	  स्थिती FFI_TYPE_DOUBLE:
	    जर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = *(द्विगुण *)item_base;
	    (*nfpr) += 1;
	    थांबा;
	  स्थिती FFI_TYPE_FLOAT:
	    जर (*nfpr < अंक_FPR_ARG_REGISTERS)
	      *fpr_base++ = (द्विगुण) *(दशांश *)item_base;
	    (*nfpr) += 1;
	    थांबा;
	  पूर्वनिर्धारित:
	    थांबा;    
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
darwin64_pass_struct_by_मूल्य (ffi_type *s, अक्षर *src, अचिन्हित size,
			       अचिन्हित *nfpr, द्विगुण **fprs, अचिन्हित दीर्घ **arg)
{
  अचिन्हित दीर्घ *next_arg = *arg;
  अक्षर *dest_cpy = (अक्षर *)next_arg;

  FFI_ASSERT (s->type == FFI_TYPE_STRUCT)

  जर (!size)
    परत;

  /* First... special cases.  */
  जर (size < 3
      || (size == 4 
	  && s->elements[0] 
	  && s->elements[0]->type != FFI_TYPE_FLOAT))
    {
      /* Must be at least one GPR, padding is unspecified in value, 
	 let's make it zero.  */
      *next_arg = 0UL; 
      dest_cpy += 8 - size;
      स_नकल ((अक्षर *) dest_cpy, src, size);
      next_arg++;
    }
  अन्यथा जर (size == 16)
    {
      स_नकल ((अक्षर *) dest_cpy, src, size);
      next_arg += 2;
    }
  अन्यथा
    {
      /* now the general case, we consider embedded floats.  */
      स_नकल ((अक्षर *) dest_cpy, src, size);
      darwin64_pass_struct_floats (s, src, nfpr, fprs);
      next_arg += (size+7)/8;
    }
    
  *arg = next_arg;
}

द्विगुण *
darwin64_struct_floats_to_mem (ffi_type *s, अक्षर *dest, द्विगुण *fprs, अचिन्हित *nf)
{
  पूर्णांक i;
  अचिन्हित struct_offset = 0;

  /* We don't assume anything about the alignment of the source.  */
  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      अक्षर *item_base;
      ffi_type *p = s->elements[i];
      /* Find the start of this item (0 for the first one).  */
      जर (i > 0)
        struct_offset = FFI_ALIGN(struct_offset, p->alignment);
      item_base = dest + struct_offset;

      निवड (p->type)
	{
	  स्थिती FFI_TYPE_STRUCT:
	    fprs = darwin64_struct_floats_to_mem (p, item_base, fprs, nf);
	    थांबा;
	  स्थिती FFI_TYPE_LONGDOUBLE:
	    जर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुण *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    item_base += 8;
	  /* FALL THROUGH */
	  स्थिती FFI_TYPE_DOUBLE:
	    जर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(द्विगुण *)item_base = *fprs++ ;
		(*nf) += 1;
	      }
	    थांबा;
	  स्थिती FFI_TYPE_FLOAT:
	    जर (*nf < अंक_FPR_ARG_REGISTERS)
	      {
		*(दशांश *)item_base = (दशांश) *fprs++ ;
		(*nf) += 1;
	      }
	    थांबा;
	  पूर्वनिर्धारित:
	    थांबा;    
	}
      /* now count the size of what we just used.  */
      struct_offset += p->size;
    }
  परत fprs;
}

#endif

/* Adjust the size of S to be correct for Darwin.
   On Darwin m32, the first field of a structure has natural alignment.  
   On Darwin m64, all fields have natural alignment.  */

स्थिर शून्य
darwin_adjust_aggregate_sizes (ffi_type *s)
{
  पूर्णांक i;

  जर (s->type != FFI_TYPE_STRUCT)
    परत;

  s->size = 0;
  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type *p;
      पूर्णांक align;
      
      p = s->elements[i];
      जर (p->type == FFI_TYPE_STRUCT)
	darwin_adjust_aggregate_sizes (p);
#जर defined(POWERPC_DARWIN64)
      /* Natural alignment for all items.  */
      align = p->alignment;
#अन्यथा
      /* Natural alignment for the first item... */
      जर (i == 0)
	align = p->alignment;
      अन्यथा जर (p->alignment == 16 || p->alignment < 4)
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
  जर (s->elements[0]->type == FFI_TYPE_UINT64
      || s->elements[0]->type == FFI_TYPE_SINT64
      || s->elements[0]->type == FFI_TYPE_DOUBLE
      || s->elements[0]->alignment == 8)
    s->alignment = s->alignment > 8 ? s->alignment : 8;
  /* Do not add additional tail padding.  */
}

/* Adjust the size of S to be correct for AIX.
   Word-align double unless it is the first member of a structure recursively.
   Return non-zero if we found a recursive first member aggregate of interest. */

स्थिर पूर्णांक
aix_adjust_aggregate_sizes (ffi_type *s, पूर्णांक outer_most_type_or_first_member)
{
  पूर्णांक i, nested_first_member=0, final_align, rc=0;

  जर (s->type != FFI_TYPE_STRUCT)
    परत 0;

  s->size = 0;
  साठी (i = 0; s->elements[i] != रिक्त; i++)
    {
      ffi_type p;
      पूर्णांक align;

      /* nested aggregates layout differently on AIX, so take a copy of the type */
      p = *(s->elements[i]);
      जर (i == 0)
        nested_first_member = aix_adjust_aggregate_sizes(&p, outer_most_type_or_first_member);
      अन्यथा
        aix_adjust_aggregate_sizes(&p, 0);
      align = p.alignment;
      जर (i != 0 && p.type == FFI_TYPE_DOUBLE)
        align = 4;
      s->size = FFI_ALIGN(s->size, align) + p.size;
    }

  final_align=s->alignment;
  जर ((s->elements[0]->type == FFI_TYPE_UINT64
          || s->elements[0]->type == FFI_TYPE_SINT64
          || s->elements[0]->type == FFI_TYPE_DOUBLE
          || s->elements[0]->alignment == 8 || nested_first_member)) {
      final_align = s->alignment > 8 ? s->alignment : 8;
      rc=1;
      /* still use the adjusted alignment to calculate tail padding, but don't adjust the types alignment if
         we aren't in the recursive first position */
      जर (outer_most_type_or_first_member)
        s->alignment=final_align;
  }

  s->size = FFI_ALIGN(s->size, final_align);
  परत rc;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep (ffi_cif *cif)
{
  /* All this is for the DARWIN ABI.  */
  अचिन्हित i;
  ffi_type **ptr;
  अचिन्हित bytes;
  अचिन्हित fparg_मोजणी = 0, intarg_मोजणी = 0;
  अचिन्हित flags = 0;
  अचिन्हित size_al = 0;

  /* All the machine-independent calculation of cif->bytes will be wrong.
     All the calculation of structure sizes will also be wrong.
     Redo the calculation for DARWIN.  */

  जर (cif->abi == FFI_DARWIN)
    {
      darwin_adjust_aggregate_sizes (cif->rtype);
      साठी (i = 0; i < cif->nargs; i++)
	darwin_adjust_aggregate_sizes (cif->arg_types[i]);
    }

  जर (cif->abi == FFI_AIX)
    {
      aix_adjust_aggregate_sizes (cif->rtype, 1);
      साठी (i = 0; i < cif->nargs; i++)
	aix_adjust_aggregate_sizes (cif->arg_types[i], 1);
    }

  /* Space for the frame pointer, callee's LR, CR, etc, and for
     the asm's temp regs.  */

  bytes = (LINKAGE_AREA_GPRS + ASM_NEEDS_REGISTERS) * आकार(अचिन्हित दीर्घ);

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
  निवड (cif->rtype->type)
    {

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      flags |= FLAG_RETURNS_128BITS;
      flags |= FLAG_RETURNS_FP;
      थांबा;
#endif

    स्थिती FFI_TYPE_DOUBLE:
      flags |= FLAG_RETURNS_64BITS;
      /* Fall through.  */
    स्थिती FFI_TYPE_FLOAT:
      flags |= FLAG_RETURNS_FP;
      थांबा;

    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
#ifdef POWERPC64
    स्थिती FFI_TYPE_POINTER:
#endif
      flags |= FLAG_RETURNS_64BITS;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
#जर defined(POWERPC_DARWIN64)
      {
	/* Can we fit the struct into regs?  */
	जर (darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	  {
	    अचिन्हित nfpr = 0;
	    flags |= FLAG_RETURNS_STRUCT;
	    जर (cif->rtype->size != 16)
	      darwin64_scan_struct_for_floats (cif->rtype, &nfpr) ;
	    अन्यथा
	      flags |= FLAG_RETURNS_128BITS;
	    /* Will be 0 for 16byte struct.  */
	    जर (nfpr)
	      flags |= FLAG_RETURNS_FP;
	  }
	अन्यथा /* By ref. */
	  {
	    flags |= FLAG_RETVAL_REFERENCE;
	    flags |= FLAG_RETURNS_NOTHING;
	    intarg_मोजणी++;
	  }
      }
#किंवाजर defined(DARWIN_PPC)
      जर (cif->rtype->size <= 4)
	flags |= FLAG_RETURNS_STRUCT;
      अन्यथा /* else by reference.  */
	{
	  flags |= FLAG_RETVAL_REFERENCE;
	  flags |= FLAG_RETURNS_NOTHING;
	  intarg_मोजणी++;
	}
#अन्यथा /* assume we pass by ref.  */
      flags |= FLAG_RETVAL_REFERENCE;
      flags |= FLAG_RETURNS_NOTHING;
      intarg_मोजणी++;
#endif
      थांबा;
    स्थिती FFI_TYPE_VOID:
      flags |= FLAG_RETURNS_NOTHING;
      थांबा;

    पूर्वनिर्धारित:
      /* Returns 32-bit integer, or similar.  Nothing to do here.  */
      थांबा;
    }

  /* The first NUM_GPR_ARG_REGISTERS words of integer arguments, and the
     first NUM_FPR_ARG_REGISTERS fp arguments, go in registers; the rest
     goes on the stack.  
     ??? Structures are passed as a pointer to a copy of the structure. 
     Stuff on the stack needs to keep proper alignment.  
     For m64 the count is effectively of half-GPRs.  */
  साठी (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {
      अचिन्हित align_शब्द;
      निवड ((*ptr)->type)
	{
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	  fparg_मोजणी++;
#जर !defined(POWERPC_DARWIN64)
	  /* If this FP arg is going on the stack, it must be
	     8-byte-aligned.  */
	  जर (fparg_मोजणी > अंक_FPR_ARG_REGISTERS
	      && (intarg_मोजणी & 0x01) != 0)
	    intarg_मोजणी++;
#endif
	  थांबा;

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	स्थिती FFI_TYPE_LONGDOUBLE:
	  fparg_मोजणी += 2;
	  /* If this FP arg is going on the stack, it must be
	     16-byte-aligned.  */
	  जर (fparg_मोजणी >= अंक_FPR_ARG_REGISTERS)
#जर defined (POWERPC64)
	    intarg_मोजणी = FFI_ALIGN(intarg_मोजणी, 2);
#अन्यथा
	    intarg_मोजणी = FFI_ALIGN(intarg_मोजणी, 4);
#endif
	  थांबा;
#endif

	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
#जर defined(POWERPC64)
	  intarg_मोजणी++;
#अन्यथा
	  /* 'long long' arguments are passed as two words, but
	     either both words must fit in registers or both go
	     on the stack.  If they go on the stack, they must
	     be 8-byte-aligned.  */
	  जर (intarg_मोजणी == अंक_GPR_ARG_REGISTERS-1
	      || (intarg_मोजणी >= अंक_GPR_ARG_REGISTERS 
	          && (intarg_मोजणी & 0x01) != 0))
	    intarg_मोजणी++;
	  intarg_मोजणी += 2;
#endif
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  size_al = (*ptr)->size;
#जर defined(POWERPC_DARWIN64)
	  align_शब्द = (*ptr)->alignment >> 3;
	  जर (align_शब्द)
	    intarg_मोजणी = FFI_ALIGN(intarg_मोजणी, align_शब्द);
	  /* Base size of the struct.  */
	  intarg_मोजणी += (size_al + 7) / 8;
	  /* If 16 bytes then don't worry about floats.  */
	  जर (size_al != 16)
	    /* Scan through for floats to be placed in regs.  */
	    darwin64_scan_struct_for_floats (*ptr, &fparg_मोजणी) ;
#अन्यथा
	  align_शब्द = (*ptr)->alignment >> 2;
	  जर (align_शब्द)
	    intarg_मोजणी = FFI_ALIGN(intarg_मोजणी, align_शब्द);
	  /* If the first member of the struct is a double, then align
	     the struct to double-word. 
	  if ((*ptr)->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN((*ptr)->size, 8); */
#  ifdef POWERPC64
	  intarg_मोजणी += (size_al + 7) / 8;
#  अन्यथा
	  intarg_मोजणी += (size_al + 3) / 4;
#  endif
#endif
	  थांबा;

	पूर्वनिर्धारित:
	  /* Everything else is passed as a 4-byte word in a GPR, either
	     the object itself or a pointer to it.  */
	  intarg_मोजणी++;
	  थांबा;
	}
    }

  जर (fparg_मोजणी != 0)
    flags |= FLAG_FP_ARGUMENTS;

#जर defined(POWERPC_DARWIN64)
  /* Space to image the FPR registers, if needed - which includes when they might be
     used in a struct return.  */
  जर (fparg_मोजणी != 0 
      || ((flags & FLAG_RETURNS_STRUCT)
	   && (flags & FLAG_RETURNS_FP)))
    bytes += अंक_FPR_ARG_REGISTERS * आकार(द्विगुण);
#अन्यथा
  /* Space for the FPR registers, if needed.  */
  जर (fparg_मोजणी != 0)
    bytes += अंक_FPR_ARG_REGISTERS * आकार(द्विगुण);
#endif

  /* Stack space.  */
#ifdef POWERPC64
  जर ((intarg_मोजणी + fparg_मोजणी) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_मोजणी + fparg_मोजणी) * आकार(दीर्घ);
#अन्यथा
  जर ((intarg_मोजणी + 2 * fparg_मोजणी) > अंक_GPR_ARG_REGISTERS)
    bytes += (intarg_मोजणी + 2 * fparg_मोजणी) * आकार(दीर्घ);
#endif
  अन्यथा
    bytes += अंक_GPR_ARG_REGISTERS * आकार(दीर्घ);

  /* The stack space allocated needs to be a multiple of 16 bytes.  */
  bytes = FFI_ALIGN(bytes, 16) ;

  cif->flags = flags;
  cif->bytes = bytes;

  परत FFI_OK;
}

बाह्य शून्य ffi_call_AIX(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			 शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य));

#जर defined (FFI_GO_CLOSURES)
बाह्य शून्य ffi_call_go_AIX(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			    शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य), शून्य *closure);
#endif

बाह्य शून्य ffi_call_DARWIN(extended_cif *, दीर्घ, अचिन्हित, अचिन्हित *,
			    शून्य (*fn)(शून्य), शून्य (*fn2)(शून्य), ffi_type*);

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  जर ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi)
    {
    स्थिती FFI_AIX:
      ffi_call_AIX(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		   FFI_FN(ffi_prep_args));
      थांबा;
    स्थिती FFI_DARWIN:
      ffi_call_DARWIN(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), cif->rtype);
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}

#जर defined (FFI_GO_CLOSURES)
शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	     शून्य *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */

  जर ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi)
    {
    स्थिती FFI_AIX:
      ffi_call_go_AIX(&ecif, -(दीर्घ)cif->bytes, cif->flags, ecif.rvalue, fn,
		      FFI_FN(ffi_prep_args), closure);
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}
#endif

स्थिर शून्य flush_icache(अक्षर *);
स्थिर शून्य flush_range(अक्षर *, पूर्णांक);

/* The layout of a function descriptor.  A C function pointer really
   points to one of these.  */

प्रकारव्याख्या संरचना aix_fd_struct {
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
  अचिन्हित पूर्णांक *tramp;
  संरचना ffi_aix_trampoline_struct *tramp_aix;
  aix_fd *fd;

  निवड (cif->abi)
    {
      स्थिती FFI_DARWIN:

	FFI_ASSERT (cif->abi == FFI_DARWIN);

	tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
#जर defined(POWERPC_DARWIN64)
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
#endif
	closure->cif = cif;
	closure->fun = fun;
	closure->user_data = user_data;

	/* Flush the icache. Only necessary on Darwin.  */
	flush_range(codeloc, FFI_TRAMPOLINE_SIZE);

	थांबा;

    स्थिती FFI_AIX:

      tramp_aix = (संरचना ffi_aix_trampoline_struct *) (closure->tramp);
      fd = (aix_fd *)(शून्य *)ffi_closure_ASM;

      FFI_ASSERT (cif->abi == FFI_AIX);

      tramp_aix->code_pointer = fd->code_pointer;
      tramp_aix->toc = fd->toc;
      tramp_aix->static_chain = codeloc;
      closure->cif = cif;
      closure->fun = fun;
      closure->user_data = user_data;
      थांबा;

    पूर्वनिर्धारित:
      परत FFI_BAD_ABI;
      थांबा;
    }
  परत FFI_OK;
}

#जर defined (FFI_GO_CLOSURES)
ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  निवड (cif->abi)
    {
      स्थिती FFI_AIX:

        FFI_ASSERT (cif->abi == FFI_AIX);

        closure->tramp = (शून्य *)ffi_go_closure_ASM;
        closure->cif = cif;
        closure->fun = fun;
        परत FFI_OK;
      
      // For now, ffi_prep_go_closure is only implemented for AIX, not for Darwin
      पूर्वनिर्धारित:
        परत FFI_BAD_ABI;
        थांबा;
    }
  परत FFI_OK;
}
#endif

स्थिर शून्य
flush_icache(अक्षर *addr)
{
#ifndef _AIX
  __asm__ अस्थिर (
		"dcbf 0,%0\n"
		"\tsync\n"
		"\ticbi 0,%0\n"
		"\tsync\n"
		"\tisync"
		: : "r"(addr) : "memory");
#endif
}

स्थिर शून्य
flush_range(अक्षर * addr1, पूर्णांक size)
{
#define MIN_ओळ_SIZE 32
  पूर्णांक i;
  साठी (i = 0; i < size; i += MIN_ओळ_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
}

प्रकारव्याख्या संघ
{
  दशांश f;
  द्विगुण d;
} ffi_dblfl;

पूर्णांक
ffi_closure_helper_DARWIN (ffi_closure *, शून्य *,
			   अचिन्हित दीर्घ *, ffi_dblfl *);

#जर defined (FFI_GO_CLOSURES)
पूर्णांक
ffi_go_closure_helper_DARWIN (ffi_go_closure*, शून्य *,
			      अचिन्हित दीर्घ *, ffi_dblfl *);
#endif

/* Basically the trampoline invokes ffi_closure_ASM, and on
   entry, r11 holds the address of the closure.
   After storing the registers that could possibly contain
   parameters to be passed into the stack frame and setting
   up space for a return value, ffi_closure_ASM invokes the
   following helper function to do most of the work.  */

स्थिर पूर्णांक
ffi_closure_helper_common (ffi_cif* cif,
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_data, शून्य *rvalue,
			   अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  /* rvalue is the pointer to space for return value in closure assembly
     pgr is the pointer to where r3-r10 are stored in ffi_closure_ASM
     pfr is the pointer to where f1-f13 are stored in ffi_closure_ASM.  */

  प्रकारव्याख्या द्विगुण ldbits[2];

  संघ ldu
  {
    ldbits lb;
    दीर्घ द्विगुण ld;
  };

  शून्य **          avalue;
  ffi_type **      arg_types;
  दीर्घ             i, avn;
  ffi_dblfl *      end_pfr = pfr + अंक_FPR_ARG_REGISTERS;
  अचिन्हित         size_al;
  पूर्णांक              struct_ret_by_मूल्य = 0;
  /* When a struct is returned by value, ffi_closure_ASM's jump-table
     dispatch carries only a small integer return code (see PPC_LD_* above),
     with no room for cif->rtype.  We hand cif->rtype back in the first
     parameter-save slot -- which is dead by the time we return -- for the
     PPC_LD_STRUCT fragment in darwin_closure.S to recover.  */
  अचिन्हित दीर्घ *  pgr0 = pgr;
#जर defined(POWERPC_DARWIN64)
  अचिन्हित 	   fpsused = 0;
#endif

  avalue = alloca (cif->nargs * आकार(शून्य *));

  जर (cif->rtype->type == FFI_TYPE_STRUCT)
    {
#जर defined(POWERPC_DARWIN64)
      जर (!darwin64_struct_ret_by_मूल्य_p (cif->rtype))
	{
    	  /* Won't fit into the regs - return by ref.  */
	  rvalue = (शून्य *) *pgr;
	  pgr++;
	}
      अन्यथा
	struct_ret_by_मूल्य = 1;
#किंवाजर defined(DARWIN_PPC)
      जर (cif->rtype->size > 4)
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
  पर्यंत (i < avn)
    {
      निवड (arg_types[i]->type)
	{
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
#जर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 7;
#अन्यथा
	  avalue[i] = (अक्षर *) pgr + 3;
#endif
	  pgr++;
	  थांबा;

	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
#जर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 6;
#अन्यथा
	  avalue[i] = (अक्षर *) pgr + 2;
#endif
	  pgr++;
	  थांबा;

	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
#जर  defined(POWERPC64)
	  avalue[i] = (अक्षर *) pgr + 4;
#अन्यथा
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = pgr;
#endif
	  pgr++;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  size_al = arg_types[i]->size;
#जर defined(POWERPC_DARWIN64)
	  pgr = (अचिन्हित दीर्घ *)FFI_ALIGN((अक्षर *)pgr, arg_types[i]->alignment);
	  जर (size_al < 3 || size_al == 4)
	    {
	      avalue[i] = ((अक्षर *)pgr)+8-size_al;
	      जर (arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT
		  && fpsused < अंक_FPR_ARG_REGISTERS)
		{
		  *(दशांश *)pgr = (दशांश) *(द्विगुण *)pfr;
		  pfr++;
		  fpsused++;
		}
	    }
	  अन्यथा 
	    {
	      जर (size_al != 16)
		pfr = (ffi_dblfl *) 
		    darwin64_struct_floats_to_mem (arg_types[i], (अक्षर *)pgr,
						   (द्विगुण *)pfr, &fpsused);
	      avalue[i] = pgr;
	    }
	  pgr += (size_al + 7) / 8;
#अन्यथा
	  /* If the first member of the struct is a double, then align
	     the struct to double-word.  */
	  जर (arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
	    size_al = FFI_ALIGN(arg_types[i]->size, 8);
#  जर defined(POWERPC64)
	  FFI_ASSERT (cif->abi != FFI_DARWIN);
	  avalue[i] = pgr;
	  pgr += (size_al + 7) / 8;
#  अन्यथा
	  /* Structures that match the basic modes (QI 1 byte, HI 2 bytes,
	     SI 4 bytes) are aligned as if they were those modes.  */
	  जर (size_al < 3 && cif->abi == FFI_DARWIN)
	    avalue[i] = (अक्षर*) pgr + 4 - size_al;
	  अन्यथा
	    avalue[i] = pgr;
	  pgr += (size_al + 3) / 4;
#  endif
#endif
	  थांबा;

	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
#जर  defined(POWERPC64)
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = pgr;
	  pgr++;
	  थांबा;
#अन्यथा
	  /* Long long ints are passed in two gpr's.  */
	  avalue[i] = pgr;
	  pgr += 2;
	  थांबा;
#endif

	स्थिती FFI_TYPE_FLOAT:
	  /* A float value consumes a GPR.
	     There are 13 64bit floating point registers.  */
	  जर (pfr < end_pfr)
	    {
	      द्विगुण temp = pfr->d;
	      pfr->f = (दशांश) temp;
	      avalue[i] = pfr;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr++;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  /* A double value consumes two GPRs.
	     There are 13 64bit floating point registers.  */
	  जर (pfr < end_pfr)
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
	  थांबा;

#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE

	स्थिती FFI_TYPE_LONGDOUBLE:
#ifdef POWERPC64
	  जर (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  अन्यथा
	    {
	      जर (pfr < end_pfr)
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
	  जर (pfr + 1 < end_pfr)
	    {
	      avalue[i] = pfr;
	      pfr += 2;
	    }
	  /* Here we have the situation where one part of the long double
	     is stored in fpr13 and the other part is already on the stack.
	     We use a union to pass the long double to avalue[i].  */
	  अन्यथा जर (pfr + 1 == end_pfr)
	    {
	      संघ ldu temp_ld;
	      स_नकल (&temp_ld.lb[0], pfr, आकार(ldbits));
	      स_नकल (&temp_ld.lb[1], pgr + 2, आकार(ldbits));
	      avalue[i] = &temp_ld.ld;
	      pfr++;
	    }
	  अन्यथा
	    {
	      avalue[i] = pgr;
	    }
	  pgr += 4;
#endif  /* POWERPC64 */
	  थांबा;
#endif
	पूर्वनिर्धारित:
	  FFI_ASSERT(0);
	}
      i++;
    }

  (fun) (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_ASM to perform return type promotions.  */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      परत PPC_LD_NONE;
    स्थिती FFI_TYPE_STRUCT:
      /* A by-reference struct return needs nothing further here: the result
	 was written straight to the caller's buffer.  A by-value struct
	 return is loaded into registers by darwin_closure.S, which needs
	 cif->rtype -- hand it back in the first parameter-save slot.  */
      जर (struct_ret_by_मूल्य)
	{
	  *pgr0 = (अचिन्हित दीर्घ) cif->rtype;
	  परत PPC_LD_STRUCT;
	}
      परत PPC_LD_NONE;
    स्थिती FFI_TYPE_FLOAT:
      परत PPC_LD_F32;
    स्थिती FFI_TYPE_DOUBLE:
      परत PPC_LD_F64;
#जर FFI_TYPE_DOUBLE != FFI_TYPE_LONGDOUBLE
    स्थिती FFI_TYPE_LONGDOUBLE:
      परत PPC_LD_F128;
#endif
    स्थिती FFI_TYPE_UINT8:
      परत PPC_LD_U8;
    स्थिती FFI_TYPE_SINT8:
      परत PPC_LD_S8;
    स्थिती FFI_TYPE_UINT16:
      परत PPC_LD_U16;
    स्थिती FFI_TYPE_SINT16:
      परत PPC_LD_S16;
    स्थिती FFI_TYPE_UINT32:
      परत PPC_LD_U32;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
      परत PPC_LD_S32;
    स्थिती FFI_TYPE_POINTER:
      परत PPC_LD_PTR;
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
      परत PPC_LD_I64;
    पूर्वनिर्धारित:
      पात();
    }
}

पूर्णांक
ffi_closure_helper_DARWIN (ffi_closure *closure, शून्य *rvalue,
			   अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  परत ffi_closure_helper_common (closure->cif, closure->fun,
				    closure->user_data, rvalue, pgr, pfr);
}

#जर defined (FFI_GO_CLOSURES)
पूर्णांक
ffi_go_closure_helper_DARWIN (ffi_go_closure *closure, शून्य *rvalue,
			      अचिन्हित दीर्घ *pgr, ffi_dblfl *pfr)
{
  परत ffi_closure_helper_common (closure->cif, closure->fun,
				    closure, rvalue, pgr, pfr);
}
#endif

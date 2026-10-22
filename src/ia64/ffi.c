/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 1998, 2007, 2008, 2012 Red Hat, Inc.
	   Copyright (c) 2000 Hewlett Packard Company
	   Copyright (c) 2011, 2026 Anthony Green
   
   IA64 Foreign Function Interface 

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>
#समावेश <stdbool.h>
#समावेश <float.h>

#समावेश "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#अगर_घोषित __hpux
प्रकार व्योम *PTR64;
#अन्यथा // some other unix
प्रकार व्योम *PTR64 __attribute__((mode(DI)));
#पूर्ण_अगर

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
प्रकार काष्ठा
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

काष्ठा ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

अटल अंतरभूत व्योम *
endian_adjust (व्योम *addr, माप_प्रकार len)
{
#अगर_घोषित __BIG_ENDIAN__
  वापस addr + (8 - len);
#अन्यथा
  वापस addr;
#पूर्ण_अगर
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#अगर_घोषित __hpux
#घोषणा stf_spill(addr, मूल्य)
#अन्यथा
#घोषणा stf_spill(addr, मूल्य)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(मूल्य));
#पूर्ण_अगर

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#अगर_घोषित __hpux
#घोषणा ldf_fill(परिणाम, addr)
#अन्यथा
#घोषणा ldf_fill(परिणाम, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(परिणाम) : "m"(*addr));
#पूर्ण_अगर

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

अटल माप_प्रकार
hfa_type_आकार (पूर्णांक type)
{
  चयन (type)
    {
    हाल FFI_IA64_TYPE_HFA_FLOAT:
      वापस माप(भग्न);
    हाल FFI_IA64_TYPE_HFA_DOUBLE:
      वापस माप(द्विगुन);
    हाल FFI_IA64_TYPE_HFA_LDOUBLE:
      वापस माप(__float80);
    शेष:
      पात ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

अटल व्योम
hfa_type_load (fpreg *fpaddr, पूर्णांक type, व्योम *addr)
{
  चयन (type)
    {
    हाल FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(भग्न *) addr);
      वापस;
    हाल FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(द्विगुन *) addr);
      वापस;
    हाल FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      वापस;
    शेष:
      पात ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

अटल व्योम
hfa_type_store (पूर्णांक type, व्योम *addr, fpreg *fpaddr)
{
  चयन (type)
    {
    हाल FFI_IA64_TYPE_HFA_FLOAT:
      {
	भग्न परिणाम;
	ldf_fill (परिणाम, fpaddr);
	*(भग्न *) addr = परिणाम;
	अवरोध;
      }
    हाल FFI_IA64_TYPE_HFA_DOUBLE:
      {
	द्विगुन परिणाम;
	ldf_fill (परिणाम, fpaddr);
	*(द्विगुन *) addr = परिणाम;
	अवरोध;
      }
    हाल FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 परिणाम;
	ldf_fill (परिणाम, fpaddr);
	*(__float80 *) addr = परिणाम;
	अवरोध;
      }
    शेष:
      पात ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

अटल पूर्णांक
hfa_element_type (ffi_type *type, पूर्णांक nested)
{
  पूर्णांक element = FFI_TYPE_VOID;

  चयन (type->type)
    {
    हाल FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      अगर (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      अवरोध;

    हाल FFI_TYPE_DOUBLE:
      /* Similarly.  */
      अगर (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      अवरोध;

    हाल FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      अगर (द_द्वि_पूर्ण == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	क्रम (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    पूर्णांक sub_element = hfa_element_type (*ptr, 1);
	    अगर (sub_element == FFI_TYPE_VOID)
	      वापस FFI_TYPE_VOID;

	    अगर (element == FFI_TYPE_VOID)
	      element = sub_element;
	    अन्यथा अगर (element != sub_element)
	      वापस FFI_TYPE_VOID;
	  }
      }
      अवरोध;

    शेष:
      वापस FFI_TYPE_VOID;
    }

  वापस element;
}


/* Perform machine dependent cif processing. */

अटल ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  पूर्णांक flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += दुरत्व(काष्ठा ia64_args, gp_regs[0]);
  अगर (cif->bytes < माप(काष्ठा ia64_args))
    cif->bytes = माप(काष्ठा ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      अगर (द_द्वि_पूर्ण != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      {
        माप_प्रकार आकार = cif->rtype->आकार;
  	पूर्णांक hfa_type = hfa_element_type (cif->rtype, 0);

	अगर (hfa_type != FFI_TYPE_VOID)
	  {
	    माप_प्रकार nelts = आकार / hfa_type_आकार (hfa_type);
	    अगर (nelts <= 8)
	      flags = hfa_type | (आकार << 8);
	  }
	अन्यथा
	  {
	    अगर (आकार <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (आकार << 8);
	  }
      }
      अवरोध;

    शेष:
      अवरोध;
    }
  cif->flags = flags;

  वापस FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  वापस ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 अचिन्हित पूर्णांक nfixedargs,
			 अचिन्हित पूर्णांक ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  वापस ffi_prep_cif_machdep_core(cif);
}

बाह्य पूर्णांक ffi_call_unix (काष्ठा ia64_args *, PTR64, व्योम (*)(व्योम), UINT64);

व्योम
ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  काष्ठा ia64_args *ढेर;
  दीर्घ i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  अगर (rvalue == शून्य && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->आकार);
    
  /* Allocate the stack frame.  */
  ढेर = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  क्रम (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      चयन ((*p_arg)->type)
	{
	हाल FFI_TYPE_SINT8:
	  ढेर->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_UINT8:
	  ढेर->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_SINT16:
	  ढेर->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_UINT16:
	  ढेर->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_SINT32:
	  ढेर->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_UINT32:
	  ढेर->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  अवरोध;
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	  ढेर->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  अवरोध;

	हाल FFI_TYPE_POINTER:
	  ढेर->gp_regs[gpcount++] = (UINT64)(PTR64) *(व्योम **)avalue[i];
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  अगर (gpcount < 8 && fpcount < 8)
	    stf_spill (&ढेर->fp_regs[fpcount++], *(भग्न *)avalue[i]);
	  {
	    UINT32 tmp;
	    स_नकल (&tmp, avalue[i], माप (UINT32));
	    ढेर->gp_regs[gpcount++] = tmp;
	  }
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर (gpcount < 8 && fpcount < 8)
	    stf_spill (&ढेर->fp_regs[fpcount++], *(द्विगुन *)avalue[i]);
	  स_नकल (&ढेर->gp_regs[gpcount++], avalue[i], माप (UINT64));
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	  अगर (gpcount & 1)
	    gpcount++;
	  अगर (द_द्वि_पूर्ण == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ढेर->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  स_नकल (&ढेर->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  {
	    माप_प्रकार आकार = (*p_arg)->आकार;
	    माप_प्रकार align = (*p_arg)->alignment;
	    पूर्णांक hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    अगर (align == 16 && (gpcount & 1))
	      gpcount++;

	    अगर (hfa_type != FFI_TYPE_VOID)
	      {
		माप_प्रकार hfa_आकार = hfa_type_आकार (hfa_type);
		माप_प्रकार offset = 0;
		माप_प्रकार gp_offset = gpcount * 8;

		जबतक (fpcount < 8
		       && offset < आकार
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&ढेर->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_आकार;
		    gp_offset += hfa_आकार;
		    fpcount += 1;
		  }
	      }

	    स_नकल (&ढेर->gp_regs[gpcount], avalue[i], आकार);
	    gpcount += (आकार + 7) / 8;
	  }
	  अवरोध;

	शेष:
	  पात ();
	}
    }

  ffi_call_unix (ढेर, rvalue, fn, cif->flags);
}

/* Closures represent a pair consisting of a function pointer, and
   some user data.  A closure is invoked by reinterpreting the closure
   as a function pointer, and branching to it.  Thus we can make an
   interpreted function callable as a C function: We turn the
   interpreter itself, together with a pointer specifying the
   interpreted procedure, into a closure.

   For IA64, function pointer are already pairs consisting of a code
   pointer, and a gp pointer.  The latter is needed to access global
   variables.  Here we set up such a pair as the first two words of
   the closure (in the "trampoline" area), but we replace the gp
   pointer with a pointer to the closure itself.  We also add the real
   gp pointer to the closure.  This allows the function entry code to
   both retrieve the user data, and to restore the correct gp pointer.  */

बाह्य व्योम ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  काष्ठा ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  काष्ठा ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  काष्ठा ffi_ia64_trampoline_struct *tramp;
  काष्ठा ia64_fd *fd;

  अगर (cif->abi != FFI_UNIX)
    वापस FFI_BAD_ABI;

  tramp = (काष्ठा ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (काष्ठा ia64_fd *)(व्योम *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_तथ्य = user_तथ्य;
  closure->fun = fun;

  वापस FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, काष्ठा ia64_args *ढेर,
			व्योम *rvalue, व्योम *r8)
{
  ffi_cif *cif;
  व्योम **avalue;
  ffi_type **p_arg;
  दीर्घ i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * माप (व्योम *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  अगर (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  क्रम (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      पूर्णांक named = i < nfixedargs;
      चयन ((*p_arg)->type)
	{
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ढेर->gp_regs[gpcount++], 1);
	  अवरोध;
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ढेर->gp_regs[gpcount++], 2);
	  अवरोध;
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ढेर->gp_regs[gpcount++], 4);
	  अवरोध;
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	  avalue[i] = &ढेर->gp_regs[gpcount++];
	  अवरोध;
	हाल FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&ढेर->gp_regs[gpcount++], माप(व्योम*));
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  अगर (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ढेर->fp_regs[fpcount++];
	      भग्न परिणाम;
	      avalue[i] = addr;
	      ldf_fill (परिणाम, addr);
	      *(भग्न *)addr = परिणाम;
	    }
	  अन्यथा
	    avalue[i] = endian_adjust(&ढेर->gp_regs[gpcount], 4);
	  gpcount++;
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ढेर->fp_regs[fpcount++];
	      द्विगुन परिणाम;
	      avalue[i] = addr;
	      ldf_fill (परिणाम, addr);
	      *(द्विगुन *)addr = परिणाम;
	    }
	  अन्यथा
	    avalue[i] = &ढेर->gp_regs[gpcount];
	  gpcount++;
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	  अगर (gpcount & 1)
	    gpcount++;
	  अगर (द_द्वि_पूर्ण == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ढेर->fp_regs[fpcount++];
	      __float80 परिणाम;
	      avalue[i] = addr;
	      ldf_fill (परिणाम, addr);
	      *(__float80 *)addr = परिणाम;
	    }
	  अन्यथा
	    avalue[i] = &ढेर->gp_regs[gpcount];
	  gpcount += 2;
	  अवरोध;

	हाल FFI_TYPE_STRUCT:
	  {
	    माप_प्रकार आकार = (*p_arg)->आकार;
	    माप_प्रकार align = (*p_arg)->alignment;
	    पूर्णांक hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    अगर (align == 16 && (gpcount & 1))
	      gpcount++;

	    अगर (hfa_type != FFI_TYPE_VOID)
	      {
		माप_प्रकार hfa_आकार = hfa_type_आकार (hfa_type);
		माप_प्रकार offset = 0;
		माप_प्रकार gp_offset = gpcount * 8;
		व्योम *addr = alloca (आकार);

		avalue[i] = addr;

		जबतक (fpcount < 8
		       && offset < आकार
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ढेर->fp_regs[fpcount]);
		    offset += hfa_आकार;
		    gp_offset += hfa_आकार;
		    fpcount += 1;
		  }

		अगर (offset < आकार)
		  स_नकल (addr + offset, (अक्षर *)ढेर->gp_regs + gp_offset,
			  आकार - offset);
	      }
	    अन्यथा
	      avalue[i] = &ढेर->gp_regs[gpcount];

	    gpcount += (आकार + 7) / 8;
	  }
	  अवरोध;

	शेष:
	  पात ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_तथ्य);

  वापस cif->flags;
}

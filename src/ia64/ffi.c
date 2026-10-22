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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <float.h>

#अन्तर्भाव "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
प्रकारव्याख्या शून्य *PTR64;
#अन्यथा // some other unix
प्रकारव्याख्या शून्य *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
प्रकारव्याख्या संरचना
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

संरचना ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

स्थिर ओळीत शून्य *
endian_adjust (शून्य *addr, माप_प्रकार len)
{
#ifdef __BIG_ENDIAN__
  परत addr + (8 - len);
#अन्यथा
  परत addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, मूल्य)
#अन्यथा
#define stf_spill(addr, मूल्य)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(मूल्य));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#अन्यथा
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

स्थिर माप_प्रकार
hfa_type_size (पूर्णांक type)
{
  निवड (type)
    {
    स्थिती FFI_IA64_TYPE_HFA_FLOAT:
      परत आकार(दशांश);
    स्थिती FFI_IA64_TYPE_HFA_DOUBLE:
      परत आकार(द्विगुण);
    स्थिती FFI_IA64_TYPE_HFA_LDOUBLE:
      परत आकार(__float80);
    पूर्वनिर्धारित:
      पात ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

स्थिर शून्य
hfa_type_load (fpreg *fpaddr, पूर्णांक type, शून्य *addr)
{
  निवड (type)
    {
    स्थिती FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(दशांश *) addr);
      परत;
    स्थिती FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(द्विगुण *) addr);
      परत;
    स्थिती FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      परत;
    पूर्वनिर्धारित:
      पात ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

स्थिर शून्य
hfa_type_store (पूर्णांक type, शून्य *addr, fpreg *fpaddr)
{
  निवड (type)
    {
    स्थिती FFI_IA64_TYPE_HFA_FLOAT:
      {
	दशांश result;
	ldf_fill (result, fpaddr);
	*(दशांश *) addr = result;
	थांबा;
      }
    स्थिती FFI_IA64_TYPE_HFA_DOUBLE:
      {
	द्विगुण result;
	ldf_fill (result, fpaddr);
	*(द्विगुण *) addr = result;
	थांबा;
      }
    स्थिती FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	थांबा;
      }
    पूर्वनिर्धारित:
      पात ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

स्थिर पूर्णांक
hfa_element_type (ffi_type *type, पूर्णांक nested)
{
  पूर्णांक element = FFI_TYPE_VOID;

  निवड (type->type)
    {
    स्थिती FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      जर (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      थांबा;

    स्थिती FFI_TYPE_DOUBLE:
      /* Similarly.  */
      जर (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      थांबा;

    स्थिती FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      जर (द_द्वि_पूर्ण == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	साठी (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    पूर्णांक sub_element = hfa_element_type (*ptr, 1);
	    जर (sub_element == FFI_TYPE_VOID)
	      परत FFI_TYPE_VOID;

	    जर (element == FFI_TYPE_VOID)
	      element = sub_element;
	    अन्यथा जर (element != sub_element)
	      परत FFI_TYPE_VOID;
	  }
      }
      थांबा;

    पूर्वनिर्धारित:
      परत FFI_TYPE_VOID;
    }

  परत element;
}


/* Perform machine dependent cif processing. */

स्थिर ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  पूर्णांक flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += दुरत्व(संरचना ia64_args, gp_regs[0]);
  जर (cif->bytes < आकार(संरचना ia64_args))
    cif->bytes = आकार(संरचना ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      जर (द_द्वि_पूर्ण != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      {
        माप_प्रकार size = cif->rtype->size;
  	पूर्णांक hfa_type = hfa_element_type (cif->rtype, 0);

	जर (hfa_type != FFI_TYPE_VOID)
	  {
	    माप_प्रकार nelts = size / hfa_type_size (hfa_type);
	    जर (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	अन्यथा
	  {
	    जर (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      थांबा;

    पूर्वनिर्धारित:
      थांबा;
    }
  cif->flags = flags;

  परत FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  परत ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 अचिन्हित पूर्णांक nfixedargs,
			 अचिन्हित पूर्णांक ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  परत ffi_prep_cif_machdep_core(cif);
}

बाह्य पूर्णांक ffi_call_unix (संरचना ia64_args *, PTR64, शून्य (*)(शून्य), UINT64);

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  संरचना ia64_args *चळत;
  दीर्घ i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  जर (rvalue == रिक्त && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  चळत = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  साठी (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      निवड ((*p_arg)->type)
	{
	स्थिती FFI_TYPE_SINT8:
	  चळत->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_UINT8:
	  चळत->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_SINT16:
	  चळत->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_UINT16:
	  चळत->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_SINT32:
	  चळत->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_UINT32:
	  चळत->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  थांबा;
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  चळत->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  थांबा;

	स्थिती FFI_TYPE_POINTER:
	  चळत->gp_regs[gpcount++] = (UINT64)(PTR64) *(शून्य **)avalue[i];
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  जर (gpcount < 8 && fpcount < 8)
	    stf_spill (&चळत->fp_regs[fpcount++], *(दशांश *)avalue[i]);
	  {
	    UINT32 tmp;
	    स_नकल (&tmp, avalue[i], आकार (UINT32));
	    चळत->gp_regs[gpcount++] = tmp;
	  }
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  जर (gpcount < 8 && fpcount < 8)
	    stf_spill (&चळत->fp_regs[fpcount++], *(द्विगुण *)avalue[i]);
	  स_नकल (&चळत->gp_regs[gpcount++], avalue[i], आकार (UINT64));
	  थांबा;

	स्थिती FFI_TYPE_LONGDOUBLE:
	  जर (gpcount & 1)
	    gpcount++;
	  जर (द_द्वि_पूर्ण == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&चळत->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  स_नकल (&चळत->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  {
	    माप_प्रकार size = (*p_arg)->size;
	    माप_प्रकार align = (*p_arg)->alignment;
	    पूर्णांक hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    जर (align == 16 && (gpcount & 1))
	      gpcount++;

	    जर (hfa_type != FFI_TYPE_VOID)
	      {
		माप_प्रकार hfa_size = hfa_type_size (hfa_type);
		माप_प्रकार offset = 0;
		माप_प्रकार gp_offset = gpcount * 8;

		पर्यंत (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&चळत->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    स_नकल (&चळत->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  थांबा;

	पूर्वनिर्धारित:
	  पात ();
	}
    }

  ffi_call_unix (चळत, rvalue, fn, cif->flags);
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

बाह्य शून्य ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  संरचना ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  संरचना ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  संरचना ffi_ia64_trampoline_struct *tramp;
  संरचना ia64_fd *fd;

  जर (cif->abi != FFI_UNIX)
    परत FFI_BAD_ABI;

  tramp = (संरचना ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (संरचना ia64_fd *)(शून्य *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  परत FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, संरचना ia64_args *चळत,
			शून्य *rvalue, शून्य *r8)
{
  ffi_cif *cif;
  शून्य **avalue;
  ffi_type **p_arg;
  दीर्घ i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * आकार (शून्य *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  जर (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  साठी (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      पूर्णांक named = i < nfixedargs;
      निवड ((*p_arg)->type)
	{
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&चळत->gp_regs[gpcount++], 1);
	  थांबा;
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&चळत->gp_regs[gpcount++], 2);
	  थांबा;
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&चळत->gp_regs[gpcount++], 4);
	  थांबा;
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  avalue[i] = &चळत->gp_regs[gpcount++];
	  थांबा;
	स्थिती FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&चळत->gp_regs[gpcount++], आकार(शून्य*));
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  जर (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &चळत->fp_regs[fpcount++];
	      दशांश result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(दशांश *)addr = result;
	    }
	  अन्यथा
	    avalue[i] = endian_adjust(&चळत->gp_regs[gpcount], 4);
	  gpcount++;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  जर (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &चळत->fp_regs[fpcount++];
	      द्विगुण result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(द्विगुण *)addr = result;
	    }
	  अन्यथा
	    avalue[i] = &चळत->gp_regs[gpcount];
	  gpcount++;
	  थांबा;

	स्थिती FFI_TYPE_LONGDOUBLE:
	  जर (gpcount & 1)
	    gpcount++;
	  जर (द_द्वि_पूर्ण == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &चळत->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  अन्यथा
	    avalue[i] = &चळत->gp_regs[gpcount];
	  gpcount += 2;
	  थांबा;

	स्थिती FFI_TYPE_STRUCT:
	  {
	    माप_प्रकार size = (*p_arg)->size;
	    माप_प्रकार align = (*p_arg)->alignment;
	    पूर्णांक hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    जर (align == 16 && (gpcount & 1))
	      gpcount++;

	    जर (hfa_type != FFI_TYPE_VOID)
	      {
		माप_प्रकार hfa_size = hfa_type_size (hfa_type);
		माप_प्रकार offset = 0;
		माप_प्रकार gp_offset = gpcount * 8;
		शून्य *addr = alloca (size);

		avalue[i] = addr;

		पर्यंत (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &चळत->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		जर (offset < size)
		  स_नकल (addr + offset, (अक्षर *)चळत->gp_regs + gp_offset,
			  size - offset);
	      }
	    अन्यथा
	      avalue[i] = &चळत->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  थांबा;

	पूर्वनिर्धारित:
	  पात ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  परत cif->flags;
}

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
प्रकारनिर्देश शून्य *PTR64;
#अञ्ञथा // some other unix
प्रकारनिर्देश शून्य *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
प्रकारनिर्देश संरचना
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

स्थिर अन्तःस्थ शून्य *
endian_adjust (शून्य *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  पटिनिवत्त addr + (8 - len);
#अञ्ञथा
  पटिनिवत्त addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, मानम्)
#अञ्ञथा
#define stf_spill(addr, मानम्)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(मानम्));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#अञ्ञथा
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

स्थिर size_t
hfa_type_size (पूर्णाङ्क type)
{
  विकल्प (type)
    {
    पक्ष FFI_IA64_TYPE_HFA_FLOAT:
      पटिनिवत्त परिमाण(भिन्न);
    पक्ष FFI_IA64_TYPE_HFA_DOUBLE:
      पटिनिवत्त परिमाण(द्विगुण);
    पक्ष FFI_IA64_TYPE_HFA_LDOUBLE:
      पटिनिवत्त परिमाण(__float80);
    स्वभाव:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

स्थिर शून्य
hfa_type_load (fpreg *fpaddr, पूर्णाङ्क type, शून्य *addr)
{
  विकल्प (type)
    {
    पक्ष FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(भिन्न *) addr);
      पटिनिवत्त;
    पक्ष FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(द्विगुण *) addr);
      पटिनिवत्त;
    पक्ष FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      पटिनिवत्त;
    स्वभाव:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

स्थिर शून्य
hfa_type_store (पूर्णाङ्क type, शून्य *addr, fpreg *fpaddr)
{
  विकल्प (type)
    {
    पक्ष FFI_IA64_TYPE_HFA_FLOAT:
      {
	भिन्न result;
	ldf_fill (result, fpaddr);
	*(भिन्न *) addr = result;
	भङ्ग;
      }
    पक्ष FFI_IA64_TYPE_HFA_DOUBLE:
      {
	द्विगुण result;
	ldf_fill (result, fpaddr);
	*(द्विगुण *) addr = result;
	भङ्ग;
      }
    पक्ष FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	भङ्ग;
      }
    स्वभाव:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

स्थिर पूर्णाङ्क
hfa_element_type (ffi_type *type, पूर्णाङ्क nested)
{
  पूर्णाङ्क element = FFI_TYPE_VOID;

  विकल्प (type->type)
    {
    पक्ष FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      सचे (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      भङ्ग;

    पक्ष FFI_TYPE_DOUBLE:
      /* Similarly.  */
      सचे (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      भङ्ग;

    पक्ष FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      सचे (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	अत्थाय (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    पूर्णाङ्क sub_element = hfa_element_type (*ptr, 1);
	    सचे (sub_element == FFI_TYPE_VOID)
	      पटिनिवत्त FFI_TYPE_VOID;

	    सचे (element == FFI_TYPE_VOID)
	      element = sub_element;
	    अञ्ञथा सचे (element != sub_element)
	      पटिनिवत्त FFI_TYPE_VOID;
	  }
      }
      भङ्ग;

    स्वभाव:
      पटिनिवत्त FFI_TYPE_VOID;
    }

  पटिनिवत्त element;
}


/* Perform machine dependent cif processing. */

स्थिर ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  पूर्णाङ्क flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(संरचना ia64_args, gp_regs[0]);
  सचे (cif->bytes < परिमाण(संरचना ia64_args))
    cif->bytes = परिमाण(संरचना ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      सचे (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	पूर्णाङ्क hfa_type = hfa_element_type (cif->rtype, 0);

	सचे (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    सचे (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	अञ्ञथा
	  {
	    सचे (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      भङ्ग;

    स्वभाव:
      भङ्ग;
    }
  cif->flags = flags;

  पटिनिवत्त FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  पटिनिवत्त ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned पूर्णाङ्क nfixedargs,
			 unsigned पूर्णाङ्क ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  पटिनिवत्त ffi_prep_cif_machdep_core(cif);
}

बाह्य पूर्णाङ्क ffi_call_unix (संरचना ia64_args *, PTR64, शून्य (*)(शून्य), UINT64);

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  संरचना ia64_args *राशिः;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  सचे (rvalue == रिक्त && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  राशिः = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  अत्थाय (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      विकल्प ((*p_arg)->type)
	{
	पक्ष FFI_TYPE_SINT8:
	  राशिः->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_UINT8:
	  राशिः->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	  राशिः->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_UINT16:
	  राशिः->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_SINT32:
	  राशिः->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_UINT32:
	  राशिः->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  भङ्ग;
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	  राशिः->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  भङ्ग;

	पक्ष FFI_TYPE_POINTER:
	  राशिः->gp_regs[gpcount++] = (UINT64)(PTR64) *(शून्य **)avalue[i];
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  सचे (gpcount < 8 && fpcount < 8)
	    stf_spill (&राशिः->fp_regs[fpcount++], *(भिन्न *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], परिमाण (UINT32));
	    राशिः->gp_regs[gpcount++] = tmp;
	  }
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  सचे (gpcount < 8 && fpcount < 8)
	    stf_spill (&राशिः->fp_regs[fpcount++], *(द्विगुण *)avalue[i]);
	  memcpy (&राशिः->gp_regs[gpcount++], avalue[i], परिमाण (UINT64));
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	  सचे (gpcount & 1)
	    gpcount++;
	  सचे (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&राशिः->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&राशिः->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    पूर्णाङ्क hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    सचे (align == 16 && (gpcount & 1))
	      gpcount++;

	    सचे (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		याव (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&राशिः->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&राशिः->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  भङ्ग;

	स्वभाव:
	  abort ();
	}
    }

  ffi_call_unix (राशिः, rvalue, fn, cif->flags);
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

  सचे (cif->abi != FFI_UNIX)
    पटिनिवत्त FFI_BAD_ABI;

  tramp = (संरचना ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (संरचना ia64_fd *)(शून्य *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, संरचना ia64_args *राशिः,
			शून्य *rvalue, शून्य *r8)
{
  ffi_cif *cif;
  शून्य **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * परिमाण (शून्य *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  सचे (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  अत्थाय (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      पूर्णाङ्क named = i < nfixedargs;
      विकल्प ((*p_arg)->type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&राशिः->gp_regs[gpcount++], 1);
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&राशिः->gp_regs[gpcount++], 2);
	  भङ्ग;
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&राशिः->gp_regs[gpcount++], 4);
	  भङ्ग;
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	  avalue[i] = &राशिः->gp_regs[gpcount++];
	  भङ्ग;
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&राशिः->gp_regs[gpcount++], परिमाण(शून्य*));
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  सचे (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &राशिः->fp_regs[fpcount++];
	      भिन्न result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(भिन्न *)addr = result;
	    }
	  अञ्ञथा
	    avalue[i] = endian_adjust(&राशिः->gp_regs[gpcount], 4);
	  gpcount++;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  सचे (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &राशिः->fp_regs[fpcount++];
	      द्विगुण result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(द्विगुण *)addr = result;
	    }
	  अञ्ञथा
	    avalue[i] = &राशिः->gp_regs[gpcount];
	  gpcount++;
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	  सचे (gpcount & 1)
	    gpcount++;
	  सचे (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &राशिः->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  अञ्ञथा
	    avalue[i] = &राशिः->gp_regs[gpcount];
	  gpcount += 2;
	  भङ्ग;

	पक्ष FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    पूर्णाङ्क hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    सचे (align == 16 && (gpcount & 1))
	      gpcount++;

	    सचे (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		शून्य *addr = alloca (size);

		avalue[i] = addr;

		याव (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &राशिः->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		सचे (offset < size)
		  memcpy (addr + offset, (वर्ण *)राशिः->gp_regs + gp_offset,
			  size - offset);
	      }
	    अञ्ञथा
	      avalue[i] = &राशिः->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  भङ्ग;

	स्वभाव:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  पटिनिवत्त cif->flags;
}

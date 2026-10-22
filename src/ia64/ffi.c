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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <float.h>

#અન્તર્ભાવ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
પ્રકારવ્યાખ્યા શૂન્ય *PTR64;
#નહીંતર // some other unix
પ્રકારવ્યાખ્યા શૂન્ય *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
પ્રકારવ્યાખ્યા સંરચના
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

સંરચના ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

સ્થિર અંતઃરેખ શૂન્ય *
endian_adjust (શૂન્ય *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  પાછા addr + (8 - len);
#નહીંતર
  પાછા addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, કિંમત)
#નહીંતર
#define stf_spill(addr, કિંમત)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(કિંમત));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#નહીંતર
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

સ્થિર size_t
hfa_type_size (પૂર્ણાંક type)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_IA64_TYPE_HFA_FLOAT:
      પાછા કદ(દશાંશ);
    સ્થિતિ FFI_IA64_TYPE_HFA_DOUBLE:
      પાછા કદ(દ્વિગુણ);
    સ્થિતિ FFI_IA64_TYPE_HFA_LDOUBLE:
      પાછા કદ(__float80);
    મૂળભૂત:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

સ્થિર શૂન્ય
hfa_type_load (fpreg *fpaddr, પૂર્ણાંક type, શૂન્ય *addr)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(દશાંશ *) addr);
      પાછા;
    સ્થિતિ FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(દ્વિગુણ *) addr);
      પાછા;
    સ્થિતિ FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      પાછા;
    મૂળભૂત:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

સ્થિર શૂન્ય
hfa_type_store (પૂર્ણાંક type, શૂન્ય *addr, fpreg *fpaddr)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_IA64_TYPE_HFA_FLOAT:
      {
	દશાંશ result;
	ldf_fill (result, fpaddr);
	*(દશાંશ *) addr = result;
	તોડો;
      }
    સ્થિતિ FFI_IA64_TYPE_HFA_DOUBLE:
      {
	દ્વિગુણ result;
	ldf_fill (result, fpaddr);
	*(દ્વિગુણ *) addr = result;
	તોડો;
      }
    સ્થિતિ FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	તોડો;
      }
    મૂળભૂત:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

સ્થિર પૂર્ણાંક
hfa_element_type (ffi_type *type, પૂર્ણાંક nested)
{
  પૂર્ણાંક element = FFI_TYPE_VOID;

  પસંદગી (type->type)
    {
    સ્થિતિ FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      જો (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      તોડો;

    સ્થિતિ FFI_TYPE_DOUBLE:
      /* Similarly.  */
      જો (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      તોડો;

    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      જો (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	માટે (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    પૂર્ણાંક sub_element = hfa_element_type (*ptr, 1);
	    જો (sub_element == FFI_TYPE_VOID)
	      પાછા FFI_TYPE_VOID;

	    જો (element == FFI_TYPE_VOID)
	      element = sub_element;
	    નહીંતર જો (element != sub_element)
	      પાછા FFI_TYPE_VOID;
	  }
      }
      તોડો;

    મૂળભૂત:
      પાછા FFI_TYPE_VOID;
    }

  પાછા element;
}


/* Perform machine dependent cif processing. */

સ્થિર ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  પૂર્ણાંક flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(સંરચના ia64_args, gp_regs[0]);
  જો (cif->bytes < કદ(સંરચના ia64_args))
    cif->bytes = કદ(સંરચના ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  પસંદગી (cif->rtype->type)
    {
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      જો (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	પૂર્ણાંક hfa_type = hfa_element_type (cif->rtype, 0);

	જો (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    જો (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	નહીંતર
	  {
	    જો (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      તોડો;

    મૂળભૂત:
      તોડો;
    }
  cif->flags = flags;

  પાછા FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  પાછા ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned પૂર્ણાંક nfixedargs,
			 unsigned પૂર્ણાંક ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  પાછા ffi_prep_cif_machdep_core(cif);
}

બાહ્ય પૂર્ણાંક ffi_call_unix (સંરચના ia64_args *, PTR64, શૂન્ય (*)(શૂન્ય), UINT64);

શૂન્ય
ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  સંરચના ia64_args *ઢગલો;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  જો (rvalue == ખાલી && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  ઢગલો = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  માટે (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      પસંદગી ((*p_arg)->type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	  ઢગલો->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT8:
	  ઢગલો->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	  ઢગલો->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT16:
	  ઢગલો->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT32:
	  ઢગલો->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT32:
	  ઢગલો->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  ઢગલો->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  તોડો;

	સ્થિતિ FFI_TYPE_POINTER:
	  ઢગલો->gp_regs[gpcount++] = (UINT64)(PTR64) *(શૂન્ય **)avalue[i];
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  જો (gpcount < 8 && fpcount < 8)
	    stf_spill (&ઢગલો->fp_regs[fpcount++], *(દશાંશ *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], કદ (UINT32));
	    ઢગલો->gp_regs[gpcount++] = tmp;
	  }
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (gpcount < 8 && fpcount < 8)
	    stf_spill (&ઢગલો->fp_regs[fpcount++], *(દ્વિગુણ *)avalue[i]);
	  memcpy (&ઢગલો->gp_regs[gpcount++], avalue[i], કદ (UINT64));
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  જો (gpcount & 1)
	    gpcount++;
	  જો (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ઢગલો->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&ઢગલો->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    પૂર્ણાંક hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    જો (align == 16 && (gpcount & 1))
	      gpcount++;

	    જો (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		જ્યારે (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&ઢગલો->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&ઢગલો->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  તોડો;

	મૂળભૂત:
	  abort ();
	}
    }

  ffi_call_unix (ઢગલો, rvalue, fn, cif->flags);
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

બાહ્ય શૂન્ય ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  સંરચના ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  સંરચના ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  સંરચના ffi_ia64_trampoline_struct *tramp;
  સંરચના ia64_fd *fd;

  જો (cif->abi != FFI_UNIX)
    પાછા FFI_BAD_ABI;

  tramp = (સંરચના ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (સંરચના ia64_fd *)(શૂન્ય *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  પાછા FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, સંરચના ia64_args *ઢગલો,
			શૂન્ય *rvalue, શૂન્ય *r8)
{
  ffi_cif *cif;
  શૂન્ય **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * કદ (શૂન્ય *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  જો (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  માટે (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      પૂર્ણાંક named = i < nfixedargs;
      પસંદગી ((*p_arg)->type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ઢગલો->gp_regs[gpcount++], 1);
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ઢગલો->gp_regs[gpcount++], 2);
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ઢગલો->gp_regs[gpcount++], 4);
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  avalue[i] = &ઢગલો->gp_regs[gpcount++];
	  તોડો;
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&ઢગલો->gp_regs[gpcount++], કદ(શૂન્ય*));
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  જો (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ઢગલો->fp_regs[fpcount++];
	      દશાંશ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(દશાંશ *)addr = result;
	    }
	  નહીંતર
	    avalue[i] = endian_adjust(&ઢગલો->gp_regs[gpcount], 4);
	  gpcount++;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ઢગલો->fp_regs[fpcount++];
	      દ્વિગુણ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(દ્વિગુણ *)addr = result;
	    }
	  નહીંતર
	    avalue[i] = &ઢગલો->gp_regs[gpcount];
	  gpcount++;
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  જો (gpcount & 1)
	    gpcount++;
	  જો (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ઢગલો->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  નહીંતર
	    avalue[i] = &ઢગલો->gp_regs[gpcount];
	  gpcount += 2;
	  તોડો;

	સ્થિતિ FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    પૂર્ણાંક hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    જો (align == 16 && (gpcount & 1))
	      gpcount++;

	    જો (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		શૂન્ય *addr = alloca (size);

		avalue[i] = addr;

		જ્યારે (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ઢગલો->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		જો (offset < size)
		  memcpy (addr + offset, (અક્ષર *)ઢગલો->gp_regs + gp_offset,
			  size - offset);
	      }
	    નહીંતર
	      avalue[i] = &ઢગલો->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  તોડો;

	મૂળભૂત:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  પાછા cif->flags;
}

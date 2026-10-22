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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <float.h>

#ଅନ୍ତର୍ଭାଵ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ *PTR64;
#ଅନ୍ୟଥା // some other unix
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

ସଂରଚନା ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ *
endian_adjust (ଶୂନ୍ୟ *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  ଫେରନ୍ତୁ addr + (8 - len);
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, ମୂଲ୍ୟ)
#ଅନ୍ୟଥା
#define stf_spill(addr, ମୂଲ୍ୟ)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(ମୂଲ୍ୟ));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#ଅନ୍ୟଥା
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ସ୍ଥିର size_t
hfa_type_size (ପୂର୍ଣ୍ଣସଂଖ୍ୟା type)
{
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_FLOAT:
      ଫେରନ୍ତୁ ଆକାର(ଦଶମିକ);
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_DOUBLE:
      ଫେରନ୍ତୁ ଆକାର(ଦ୍ୱିଗୁଣ);
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_LDOUBLE:
      ଫେରନ୍ତୁ ଆକାର(__float80);
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ସ୍ଥିର ଶୂନ୍ୟ
hfa_type_load (fpreg *fpaddr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *addr)
{
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(ଦଶମିକ *) addr);
      ଫେରନ୍ତୁ;
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(ଦ୍ୱିଗୁଣ *) addr);
      ଫେରନ୍ତୁ;
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      ଫେରନ୍ତୁ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ସ୍ଥିର ଶୂନ୍ୟ
hfa_type_store (ପୂର୍ଣ୍ଣସଂଖ୍ୟା type, ଶୂନ୍ୟ *addr, fpreg *fpaddr)
{
  ବାଛ (type)
    {
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_FLOAT:
      {
	ଦଶମିକ result;
	ldf_fill (result, fpaddr);
	*(ଦଶମିକ *) addr = result;
	ଭାଙ୍ଗ;
      }
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_DOUBLE:
      {
	ଦ୍ୱିଗୁଣ result;
	ldf_fill (result, fpaddr);
	*(ଦ୍ୱିଗୁଣ *) addr = result;
	ଭାଙ୍ଗ;
      }
    ସ୍ଥିତି FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	ଭାଙ୍ଗ;
      }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
hfa_element_type (ffi_type *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା nested)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା element = FFI_TYPE_VOID;

  ବାଛ (type->type)
    {
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      ଯଦି (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      /* Similarly.  */
      ଯଦି (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      ଯଦି (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	ପାଇଁ (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା sub_element = hfa_element_type (*ptr, 1);
	    ଯଦି (sub_element == FFI_TYPE_VOID)
	      ଫେରନ୍ତୁ FFI_TYPE_VOID;

	    ଯଦି (element == FFI_TYPE_VOID)
	      element = sub_element;
	    ଅନ୍ୟଥା ଯଦି (element != sub_element)
	      ଫେରନ୍ତୁ FFI_TYPE_VOID;
	  }
      }
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ FFI_TYPE_VOID;
    }

  ଫେରନ୍ତୁ element;
}


/* Perform machine dependent cif processing. */

ସ୍ଥିର ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(ସଂରଚନା ia64_args, gp_regs[0]);
  ଯଦି (cif->bytes < ଆକାର(ସଂରଚନା ia64_args))
    cif->bytes = ଆକାର(ସଂରଚନା ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  ବାଛ (cif->rtype->type)
    {
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      ଯଦି (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hfa_type = hfa_element_type (cif->rtype, 0);

	ଯଦି (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    ଯଦି (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	ଅନ୍ୟଥା
	  {
	    ଯଦି (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଭାଙ୍ଗ;
    }
  cif->flags = flags;

  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ଫେରନ୍ତୁ ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
			 unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  ଫେରନ୍ତୁ ffi_prep_cif_machdep_core(cif);
}

ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_call_unix (ସଂରଚନା ia64_args *, PTR64, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ), UINT64);

ଶୂନ୍ୟ
ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ସଂରଚନା ia64_args *ଗଦା;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  ଯଦି (rvalue == ଖାଲି && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  ଗଦା = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  ପାଇଁ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ବାଛ ((*p_arg)->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  ଗଦା->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  ଗଦା->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  ଗଦା->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  ଗଦା->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  ଗଦା->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  ଗଦା->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	  ଗଦା->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_POINTER:
	  ଗଦା->gp_regs[gpcount++] = (UINT64)(PTR64) *(ଶୂନ୍ୟ **)avalue[i];
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ଯଦି (gpcount < 8 && fpcount < 8)
	    stf_spill (&ଗଦା->fp_regs[fpcount++], *(ଦଶମିକ *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], ଆକାର (UINT32));
	    ଗଦା->gp_regs[gpcount++] = tmp;
	  }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (gpcount < 8 && fpcount < 8)
	    stf_spill (&ଗଦା->fp_regs[fpcount++], *(ଦ୍ୱିଗୁଣ *)avalue[i]);
	  memcpy (&ଗଦା->gp_regs[gpcount++], avalue[i], ଆକାର (UINT64));
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  ଯଦି (gpcount & 1)
	    gpcount++;
	  ଯଦି (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ଗଦା->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&ଗଦା->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ଯଦି (align == 16 && (gpcount & 1))
	      gpcount++;

	    ଯଦି (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		ଯେତେବେଳେ (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&ଗଦା->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&ଗଦା->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort ();
	}
    }

  ffi_call_unix (ଗଦା, rvalue, fn, cif->flags);
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

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  ସଂରଚନା ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  ସଂରଚନା ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  ସଂରଚନା ffi_ia64_trampoline_struct *tramp;
  ସଂରଚନା ia64_fd *fd;

  ଯଦି (cif->abi != FFI_UNIX)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  tramp = (ସଂରଚନା ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (ସଂରଚନା ia64_fd *)(ଶୂନ୍ୟ *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, ସଂରଚନା ia64_args *ଗଦା,
			ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ *r8)
{
  ffi_cif *cif;
  ଶୂନ୍ୟ **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * ଆକାର (ଶୂନ୍ୟ *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  ଯଦି (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  ପାଇଁ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା named = i < nfixedargs;
      ବାଛ ((*p_arg)->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ଗଦା->gp_regs[gpcount++], 1);
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ଗଦା->gp_regs[gpcount++], 2);
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ଗଦା->gp_regs[gpcount++], 4);
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	  avalue[i] = &ଗଦା->gp_regs[gpcount++];
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&ଗଦା->gp_regs[gpcount++], ଆକାର(ଶୂନ୍ୟ*));
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ଯଦି (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ଗଦା->fp_regs[fpcount++];
	      ଦଶମିକ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(ଦଶମିକ *)addr = result;
	    }
	  ଅନ୍ୟଥା
	    avalue[i] = endian_adjust(&ଗଦା->gp_regs[gpcount], 4);
	  gpcount++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ଗଦା->fp_regs[fpcount++];
	      ଦ୍ୱିଗୁଣ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(ଦ୍ୱିଗୁଣ *)addr = result;
	    }
	  ଅନ୍ୟଥା
	    avalue[i] = &ଗଦା->gp_regs[gpcount];
	  gpcount++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  ଯଦି (gpcount & 1)
	    gpcount++;
	  ଯଦି (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ଗଦା->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  ଅନ୍ୟଥା
	    avalue[i] = &ଗଦା->gp_regs[gpcount];
	  gpcount += 2;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    ପୂର୍ଣ୍ଣସଂଖ୍ୟା hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ଯଦି (align == 16 && (gpcount & 1))
	      gpcount++;

	    ଯଦି (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		ଶୂନ୍ୟ *addr = alloca (size);

		avalue[i] = addr;

		ଯେତେବେଳେ (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ଗଦା->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		ଯଦି (offset < size)
		  memcpy (addr + offset, (ଅକ୍ଷର *)ଗଦା->gp_regs + gp_offset,
			  size - offset);
	      }
	    ଅନ୍ୟଥା
	      avalue[i] = &ଗଦା->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  ଫେରନ୍ତୁ cif->flags;
}

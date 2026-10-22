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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <float.h>

#అన్తర్భావ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#నిర్వచితమైతే __hpux
రకనిర్వచనం శూన్య *PTR64;
#లేకపోతే // some other unix
రకనిర్వచనం శూన్య *PTR64 __attribute__((బహుళకం(DI)));
#అయితే_ముగింపు

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
రకనిర్వచనం నిర్మాణం
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

నిర్మాణం ia64_వాదనలు
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_వాదనలు[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

స్థిర అంతఃవరుస శూన్య *
endian_adjust (శూన్య *addr, size_t len)
{
#నిర్వచితమైతే __BIG_ENDIAN__
  తిరిగి addr + (8 - len);
#లేకపోతే
  తిరిగి addr;
#అయితే_ముగింపు
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#నిర్వచితమైతే __hpux
#నిర్వచించు stf_spill(addr, విలువ)
#లేకపోతే
#నిర్వచించు stf_spill(addr, విలువ)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(విలువ));
#అయితే_ముగింపు

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#నిర్వచితమైతే __hpux
#నిర్వచించు ldf_fill(ఫలితం, addr)
#లేకపోతే
#నిర్వచించు ldf_fill(ఫలితం, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(ఫలితం) : "m"(*addr));
#అయితే_ముగింపు

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

స్థిర size_t
hfa_type_కొలత (పూర్ణాంకం type)
{
  ఎంపిక (type)
    {
    స్థితి FFI_IA64_TYPE_HFA_FLOAT:
      తిరిగి పరిమాణం(దశాంశం);
    స్థితి FFI_IA64_TYPE_HFA_DOUBLE:
      తిరిగి పరిమాణం(ద్విగుణ);
    స్థితి FFI_IA64_TYPE_HFA_LDOUBLE:
      తిరిగి పరిమాణం(__float80);
    అప్రమేయ:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

స్థిర శూన్య
hfa_type_ఎక్కించు (fpreg *fpaddr, పూర్ణాంకం type, శూన్య *addr)
{
  ఎంపిక (type)
    {
    స్థితి FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(దశాంశం *) addr);
      తిరిగి;
    స్థితి FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(ద్విగుణ *) addr);
      తిరిగి;
    స్థితి FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      తిరిగి;
    అప్రమేయ:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

స్థిర శూన్య
hfa_type_store (పూర్ణాంకం type, శూన్య *addr, fpreg *fpaddr)
{
  ఎంపిక (type)
    {
    స్థితి FFI_IA64_TYPE_HFA_FLOAT:
      {
	దశాంశం ఫలితం;
	ldf_fill (ఫలితం, fpaddr);
	*(దశాంశం *) addr = ఫలితం;
	ఆపు;
      }
    స్థితి FFI_IA64_TYPE_HFA_DOUBLE:
      {
	ద్విగుణ ఫలితం;
	ldf_fill (ఫలితం, fpaddr);
	*(ద్విగుణ *) addr = ఫలితం;
	ఆపు;
      }
    స్థితి FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 ఫలితం;
	ldf_fill (ఫలితం, fpaddr);
	*(__float80 *) addr = ఫలితం;
	ఆపు;
      }
    అప్రమేయ:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

స్థిర పూర్ణాంకం
hfa_element_type (ffi_type *type, పూర్ణాంకం nested)
{
  పూర్ణాంకం element = FFI_TYPE_VOID;

  ఎంపిక (type->type)
    {
    స్థితి FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      ఒకవేళ (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      ఆపు;

    స్థితి FFI_TYPE_DOUBLE:
      /* Similarly.  */
      ఒకవేళ (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      ఆపు;

    స్థితి FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      ఒకవేళ (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	కొరకు (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    పూర్ణాంకం sub_element = hfa_element_type (*ptr, 1);
	    ఒకవేళ (sub_element == FFI_TYPE_VOID)
	      తిరిగి FFI_TYPE_VOID;

	    ఒకవేళ (element == FFI_TYPE_VOID)
	      element = sub_element;
	    లేకపోతే ఒకవేళ (element != sub_element)
	      తిరిగి FFI_TYPE_VOID;
	  }
      }
      ఆపు;

    అప్రమేయ:
      తిరిగి FFI_TYPE_VOID;
    }

  తిరిగి element;
}


/* Perform machine dependent cif processing. */

స్థిర ffi_స్థితి
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  పూర్ణాంకం flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->బైట్లు += offsetof(నిర్మాణం ia64_వాదనలు, gp_regs[0]);
  ఒకవేళ (cif->బైట్లు < పరిమాణం(నిర్మాణం ia64_వాదనలు))
    cif->బైట్లు = పరిమాణం(నిర్మాణం ia64_వాదనలు);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      ఒకవేళ (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      {
        size_t కొలత = cif->rtype->కొలత;
  	పూర్ణాంకం hfa_type = hfa_element_type (cif->rtype, 0);

	ఒకవేళ (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = కొలత / hfa_type_కొలత (hfa_type);
	    ఒకవేళ (nelts <= 8)
	      flags = hfa_type | (కొలత << 8);
	  }
	లేకపోతే
	  {
	    ఒకవేళ (కొలత <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (కొలత << 8);
	  }
      }
      ఆపు;

    అప్రమేయ:
      ఆపు;
    }
  cif->flags = flags;

  తిరిగి FFI_OK;
}

ffi_స్థితి
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  తిరిగి ffi_prep_cif_machdep_core(cif);
}

ffi_స్థితి
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 గుర్తులేని పూర్ణాంకం nfixedargs,
			 గుర్తులేని పూర్ణాంకం ntotalargs MAYBE_వాడని)
{
  cif->nfixedargs = nfixedargs;
  తిరిగి ffi_prep_cif_machdep_core(cif);
}

బాహ్య పూర్ణాంకం ffi_call_unix (నిర్మాణం ia64_వాదనలు *, PTR64, శూన్య (*)(శూన్య), UINT64);

శూన్య
ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  నిర్మాణం ia64_వాదనలు *దొంతర;
  దీర్ఘ i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  ఒకవేళ (rvalue == ఖాళీ && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->కొలత);
    
  /* Allocate the stack frame.  */
  దొంతర = alloca (cif->బైట్లు);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  కొరకు (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ఎంపిక ((*p_arg)->type)
	{
	స్థితి FFI_TYPE_SINT8:
	  దొంతర->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_UINT8:
	  దొంతర->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	  దొంతర->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_UINT16:
	  దొంతర->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_SINT32:
	  దొంతర->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_UINT32:
	  దొంతర->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  ఆపు;
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  దొంతర->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  ఆపు;

	స్థితి FFI_TYPE_సూచిక:
	  దొంతర->gp_regs[gpcount++] = (UINT64)(PTR64) *(శూన్య **)avalue[i];
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  ఒకవేళ (gpcount < 8 && fpcount < 8)
	    stf_spill (&దొంతర->fp_regs[fpcount++], *(దశాంశం *)avalue[i]);
	  {
	    UINT32 tmp;
	    జ్ఞాపకం_నకలు (&tmp, avalue[i], పరిమాణం (UINT32));
	    దొంతర->gp_regs[gpcount++] = tmp;
	  }
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (gpcount < 8 && fpcount < 8)
	    stf_spill (&దొంతర->fp_regs[fpcount++], *(ద్విగుణ *)avalue[i]);
	  జ్ఞాపకం_నకలు (&దొంతర->gp_regs[gpcount++], avalue[i], పరిమాణం (UINT64));
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	  ఒకవేళ (gpcount & 1)
	    gpcount++;
	  ఒకవేళ (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&దొంతర->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  జ్ఞాపకం_నకలు (&దొంతర->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  {
	    size_t కొలత = (*p_arg)->కొలత;
	    size_t align = (*p_arg)->alignment;
	    పూర్ణాంకం hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ఒకవేళ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ఒకవేళ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_కొలత = hfa_type_కొలత (hfa_type);
		size_t offset = 0;
		size_t gp_స్థానభ్రంశం = gpcount * 8;

		అయితే (fpcount < 8
		       && offset < కొలత
		       && gp_స్థానభ్రంశం < 8 * 8)
		  {
		    hfa_type_ఎక్కించు (&దొంతర->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_కొలత;
		    gp_స్థానభ్రంశం += hfa_కొలత;
		    fpcount += 1;
		  }
	      }

	    జ్ఞాపకం_నకలు (&దొంతర->gp_regs[gpcount], avalue[i], కొలత);
	    gpcount += (కొలత + 7) / 8;
	  }
	  ఆపు;

	అప్రమేయ:
	  abort ();
	}
    }

  ffi_call_unix (దొంతర, rvalue, fn, cif->flags);
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

బాహ్య శూన్య ffi_సంవృతం_unix ();

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  నిర్మాణం ia64_fd
  {
    UINT64 సంకేతం_సూచిక;
    UINT64 gp;
  };

  నిర్మాణం ffi_ia64_trampoline_struct
  {
    UINT64 సంకేతం_సూచిక;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  నిర్మాణం ffi_ia64_trampoline_struct *tramp;
  నిర్మాణం ia64_fd *fd;

  ఒకవేళ (cif->abi != FFI_UNIX)
    తిరిగి FFI_BAD_ABI;

  tramp = (నిర్మాణం ffi_ia64_trampoline_struct *)సంవృతం->tramp;
  fd = (నిర్మాణం ia64_fd *)(శూన్య *)ffi_సంవృతం_unix;

  tramp->సంకేతం_సూచిక = fd->సంకేతం_సూచిక;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  సంవృతం->cif = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}


UINT64
ffi_సంవృతం_unix_inner (ffi_సంవృతం *సంవృతం, నిర్మాణం ia64_వాదనలు *దొంతర,
			శూన్య *rvalue, శూన్య *r8)
{
  ffi_cif *cif;
  శూన్య **avalue;
  ffi_type **p_arg;
  దీర్ఘ i, avn, gpcount, fpcount, nfixedargs;

  cif = సంవృతం->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * పరిమాణం (శూన్య *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  ఒకవేళ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  కొరకు (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      పూర్ణాంకం named = i < nfixedargs;
      ఎంపిక ((*p_arg)->type)
	{
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&దొంతర->gp_regs[gpcount++], 1);
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&దొంతర->gp_regs[gpcount++], 2);
	  ఆపు;
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&దొంతర->gp_regs[gpcount++], 4);
	  ఆపు;
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  avalue[i] = &దొంతర->gp_regs[gpcount++];
	  ఆపు;
	స్థితి FFI_TYPE_సూచిక:
	  avalue[i] = endian_adjust(&దొంతర->gp_regs[gpcount++], పరిమాణం(శూన్య*));
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  ఒకవేళ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &దొంతర->fp_regs[fpcount++];
	      దశాంశం ఫలితం;
	      avalue[i] = addr;
	      ldf_fill (ఫలితం, addr);
	      *(దశాంశం *)addr = ఫలితం;
	    }
	  లేకపోతే
	    avalue[i] = endian_adjust(&దొంతర->gp_regs[gpcount], 4);
	  gpcount++;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &దొంతర->fp_regs[fpcount++];
	      ద్విగుణ ఫలితం;
	      avalue[i] = addr;
	      ldf_fill (ఫలితం, addr);
	      *(ద్విగుణ *)addr = ఫలితం;
	    }
	  లేకపోతే
	    avalue[i] = &దొంతర->gp_regs[gpcount];
	  gpcount++;
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	  ఒకవేళ (gpcount & 1)
	    gpcount++;
	  ఒకవేళ (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &దొంతర->fp_regs[fpcount++];
	      __float80 ఫలితం;
	      avalue[i] = addr;
	      ldf_fill (ఫలితం, addr);
	      *(__float80 *)addr = ఫలితం;
	    }
	  లేకపోతే
	    avalue[i] = &దొంతర->gp_regs[gpcount];
	  gpcount += 2;
	  ఆపు;

	స్థితి FFI_TYPE_STRUCT:
	  {
	    size_t కొలత = (*p_arg)->కొలత;
	    size_t align = (*p_arg)->alignment;
	    పూర్ణాంకం hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ఒకవేళ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ఒకవేళ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_కొలత = hfa_type_కొలత (hfa_type);
		size_t offset = 0;
		size_t gp_స్థానభ్రంశం = gpcount * 8;
		శూన్య *addr = alloca (కొలత);

		avalue[i] = addr;

		అయితే (fpcount < 8
		       && offset < కొలత
		       && gp_స్థానభ్రంశం < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &దొంతర->fp_regs[fpcount]);
		    offset += hfa_కొలత;
		    gp_స్థానభ్రంశం += hfa_కొలత;
		    fpcount += 1;
		  }

		ఒకవేళ (offset < కొలత)
		  జ్ఞాపకం_నకలు (addr + offset, (అక్షరం *)దొంతర->gp_regs + gp_స్థానభ్రంశం,
			  కొలత - offset);
	      }
	    లేకపోతే
	      avalue[i] = &దొంతర->gp_regs[gpcount];

	    gpcount += (కొలత + 7) / 8;
	  }
	  ఆపు;

	అప్రమేయ:
	  abort ();
	}
    }

  సంవృతం->fun (cif, rvalue, avalue, సంవృతం->user_దత్తాంశం);

  తిరిగి cif->flags;
}

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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <float.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __hpux
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ *PTR64;
#ਨਹੀਂਤਾਂ // some other unix
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ *PTR64 __attribute__((ਬਹੁਲਕ(DI)));
#ਜੇ_ਅੰਤ

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

ਬਣਤਰ ia64_ਦਲੀਲਾਂ
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_ਦਲੀਲਾਂ[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ *
endian_adjust (ਖਾਲੀ *addr, size_t len)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __BIG_ENDIAN__
  ਵਾਪਸ addr + (8 - len);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ addr;
#ਜੇ_ਅੰਤ
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __hpux
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ stf_spill(addr, ਮੁੱਲ)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ stf_spill(addr, ਮੁੱਲ)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(ਮੁੱਲ));
#ਜੇ_ਅੰਤ

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __hpux
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldf_fill(ਨਤੀਜਾ, addr)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ldf_fill(ਨਤੀਜਾ, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(ਨਤੀਜਾ) : "m"(*addr));
#ਜੇ_ਅੰਤ

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ਸਥਿਰ size_t
hfa_type_ਮਾਪ (ਪੂਰਨ type)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_IA64_TYPE_HFA_FLOAT:
      ਵਾਪਸ ਆਕਾਰ(ਦਸ਼ਮਲਵ);
    ਹਾਲਤ FFI_IA64_TYPE_HFA_DOUBLE:
      ਵਾਪਸ ਆਕਾਰ(ਦੁੱਗਣਾ);
    ਹਾਲਤ FFI_IA64_TYPE_HFA_LDOUBLE:
      ਵਾਪਸ ਆਕਾਰ(__float80);
    ਮੂਲ:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ਸਥਿਰ ਖਾਲੀ
hfa_type_ਲੱਦੋ (fpreg *fpaddr, ਪੂਰਨ type, ਖਾਲੀ *addr)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(ਦਸ਼ਮਲਵ *) addr);
      ਵਾਪਸ;
    ਹਾਲਤ FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(ਦੁੱਗਣਾ *) addr);
      ਵਾਪਸ;
    ਹਾਲਤ FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      ਵਾਪਸ;
    ਮੂਲ:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ਸਥਿਰ ਖਾਲੀ
hfa_type_store (ਪੂਰਨ type, ਖਾਲੀ *addr, fpreg *fpaddr)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_IA64_TYPE_HFA_FLOAT:
      {
	ਦਸ਼ਮਲਵ ਨਤੀਜਾ;
	ldf_fill (ਨਤੀਜਾ, fpaddr);
	*(ਦਸ਼ਮਲਵ *) addr = ਨਤੀਜਾ;
	ਰੋਕੋ;
      }
    ਹਾਲਤ FFI_IA64_TYPE_HFA_DOUBLE:
      {
	ਦੁੱਗਣਾ ਨਤੀਜਾ;
	ldf_fill (ਨਤੀਜਾ, fpaddr);
	*(ਦੁੱਗਣਾ *) addr = ਨਤੀਜਾ;
	ਰੋਕੋ;
      }
    ਹਾਲਤ FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 ਨਤੀਜਾ;
	ldf_fill (ਨਤੀਜਾ, fpaddr);
	*(__float80 *) addr = ਨਤੀਜਾ;
	ਰੋਕੋ;
      }
    ਮੂਲ:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ਸਥਿਰ ਪੂਰਨ
hfa_element_type (ffi_type *type, ਪੂਰਨ nested)
{
  ਪੂਰਨ element = FFI_TYPE_VOID;

  ਚੋਣ (type->type)
    {
    ਹਾਲਤ FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      ਜੇ (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_DOUBLE:
      /* Similarly.  */
      ਜੇ (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      ਜੇ (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	ਲਈ (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    ਪੂਰਨ sub_element = hfa_element_type (*ptr, 1);
	    ਜੇ (sub_element == FFI_TYPE_VOID)
	      ਵਾਪਸ FFI_TYPE_VOID;

	    ਜੇ (element == FFI_TYPE_VOID)
	      element = sub_element;
	    ਨਹੀਂਤਾਂ ਜੇ (element != sub_element)
	      ਵਾਪਸ FFI_TYPE_VOID;
	  }
      }
      ਰੋਕੋ;

    ਮੂਲ:
      ਵਾਪਸ FFI_TYPE_VOID;
    }

  ਵਾਪਸ element;
}


/* Perform machine dependent cif processing. */

ਸਥਿਰ ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ਪੂਰਨ flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->ਬਾਈਟਾਂ += offsetof(ਬਣਤਰ ia64_ਦਲੀਲਾਂ, gp_regs[0]);
  ਜੇ (cif->ਬਾਈਟਾਂ < ਆਕਾਰ(ਬਣਤਰ ia64_ਦਲੀਲਾਂ))
    cif->ਬਾਈਟਾਂ = ਆਕਾਰ(ਬਣਤਰ ia64_ਦਲੀਲਾਂ);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  ਚੋਣ (cif->rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      ਜੇ (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      {
        size_t ਮਾਪ = cif->rtype->ਮਾਪ;
  	ਪੂਰਨ hfa_type = hfa_element_type (cif->rtype, 0);

	ਜੇ (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = ਮਾਪ / hfa_type_ਮਾਪ (hfa_type);
	    ਜੇ (nelts <= 8)
	      flags = hfa_type | (ਮਾਪ << 8);
	  }
	ਨਹੀਂਤਾਂ
	  {
	    ਜੇ (ਮਾਪ <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (ਮਾਪ << 8);
	  }
      }
      ਰੋਕੋ;

    ਮੂਲ:
      ਰੋਕੋ;
    }
  cif->flags = flags;

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ਵਾਪਸ ffi_prep_cif_machdep_core(cif);
}

ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs MAYBE_ਅਣਵਰਤਿਆ)
{
  cif->nfixedargs = nfixedargs;
  ਵਾਪਸ ffi_prep_cif_machdep_core(cif);
}

ਬਾਹਰੀ ਪੂਰਨ ffi_call_unix (ਬਣਤਰ ia64_ਦਲੀਲਾਂ *, PTR64, ਖਾਲੀ (*)(ਖਾਲੀ), UINT64);

ਖਾਲੀ
ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ਬਣਤਰ ia64_ਦਲੀਲਾਂ *ਢੇਰ;
  ਲੰਮਾ i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  ਜੇ (rvalue == ਨਿਰਾ && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->ਮਾਪ);
    
  /* Allocate the stack frame.  */
  ਢੇਰ = alloca (cif->ਬਾਈਟਾਂ);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  ਲਈ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ਚੋਣ ((*p_arg)->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  ਢੇਰ->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT8:
	  ਢੇਰ->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  ਢੇਰ->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  ਢੇਰ->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT32:
	  ਢੇਰ->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT32:
	  ਢੇਰ->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  ਢੇਰ->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  ਢੇਰ->gp_regs[gpcount++] = (UINT64)(PTR64) *(ਖਾਲੀ **)avalue[i];
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਜੇ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ਢੇਰ->fp_regs[fpcount++], *(ਦਸ਼ਮਲਵ *)avalue[i]);
	  {
	    UINT32 tmp;
	    ਸਮ੍ਰਿਤੀ_ਨਕਲ (&tmp, avalue[i], ਆਕਾਰ (UINT32));
	    ਢੇਰ->gp_regs[gpcount++] = tmp;
	  }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ਢੇਰ->fp_regs[fpcount++], *(ਦੁੱਗਣਾ *)avalue[i]);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (&ਢੇਰ->gp_regs[gpcount++], avalue[i], ਆਕਾਰ (UINT64));
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  ਜੇ (gpcount & 1)
	    gpcount++;
	  ਜੇ (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ਢੇਰ->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (&ਢੇਰ->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  {
	    size_t ਮਾਪ = (*p_arg)->ਮਾਪ;
	    size_t align = (*p_arg)->alignment;
	    ਪੂਰਨ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ਜੇ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ਜੇ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_ਮਾਪ = hfa_type_ਮਾਪ (hfa_type);
		size_t offset = 0;
		size_t gp_ਵਿੱਥ = gpcount * 8;

		ਜਦੋਂ (fpcount < 8
		       && offset < ਮਾਪ
		       && gp_ਵਿੱਥ < 8 * 8)
		  {
		    hfa_type_ਲੱਦੋ (&ਢੇਰ->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_ਮਾਪ;
		    gp_ਵਿੱਥ += hfa_ਮਾਪ;
		    fpcount += 1;
		  }
	      }

	    ਸਮ੍ਰਿਤੀ_ਨਕਲ (&ਢੇਰ->gp_regs[gpcount], avalue[i], ਮਾਪ);
	    gpcount += (ਮਾਪ + 7) / 8;
	  }
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort ();
	}
    }

  ffi_call_unix (ਢੇਰ, rvalue, fn, cif->flags);
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

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_unix ();

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  ਬਣਤਰ ia64_fd
  {
    UINT64 ਕੋਡ_ਸੂਚਕ;
    UINT64 gp;
  };

  ਬਣਤਰ ffi_ia64_trampoline_struct
  {
    UINT64 ਕੋਡ_ਸੂਚਕ;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  ਬਣਤਰ ffi_ia64_trampoline_struct *tramp;
  ਬਣਤਰ ia64_fd *fd;

  ਜੇ (cif->abi != FFI_UNIX)
    ਵਾਪਸ FFI_BAD_ABI;

  tramp = (ਬਣਤਰ ffi_ia64_trampoline_struct *)ਘੇਰਾ->tramp;
  fd = (ਬਣਤਰ ia64_fd *)(ਖਾਲੀ *)ffi_ਘੇਰਾ_unix;

  tramp->ਕੋਡ_ਸੂਚਕ = fd->ਕੋਡ_ਸੂਚਕ;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}


UINT64
ffi_ਘੇਰਾ_unix_inner (ffi_ਘੇਰਾ *ਘੇਰਾ, ਬਣਤਰ ia64_ਦਲੀਲਾਂ *ਢੇਰ,
			ਖਾਲੀ *rvalue, ਖਾਲੀ *r8)
{
  ffi_cif *cif;
  ਖਾਲੀ **avalue;
  ffi_type **p_arg;
  ਲੰਮਾ i, avn, gpcount, fpcount, nfixedargs;

  cif = ਘੇਰਾ->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * ਆਕਾਰ (ਖਾਲੀ *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  ਜੇ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  ਲਈ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ਪੂਰਨ named = i < nfixedargs;
      ਚੋਣ ((*p_arg)->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ਢੇਰ->gp_regs[gpcount++], 1);
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ਢੇਰ->gp_regs[gpcount++], 2);
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ਢੇਰ->gp_regs[gpcount++], 4);
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  avalue[i] = &ਢੇਰ->gp_regs[gpcount++];
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = endian_adjust(&ਢੇਰ->gp_regs[gpcount++], ਆਕਾਰ(ਖਾਲੀ*));
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਜੇ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ਢੇਰ->fp_regs[fpcount++];
	      ਦਸ਼ਮਲਵ ਨਤੀਜਾ;
	      avalue[i] = addr;
	      ldf_fill (ਨਤੀਜਾ, addr);
	      *(ਦਸ਼ਮਲਵ *)addr = ਨਤੀਜਾ;
	    }
	  ਨਹੀਂਤਾਂ
	    avalue[i] = endian_adjust(&ਢੇਰ->gp_regs[gpcount], 4);
	  gpcount++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ਢੇਰ->fp_regs[fpcount++];
	      ਦੁੱਗਣਾ ਨਤੀਜਾ;
	      avalue[i] = addr;
	      ldf_fill (ਨਤੀਜਾ, addr);
	      *(ਦੁੱਗਣਾ *)addr = ਨਤੀਜਾ;
	    }
	  ਨਹੀਂਤਾਂ
	    avalue[i] = &ਢੇਰ->gp_regs[gpcount];
	  gpcount++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  ਜੇ (gpcount & 1)
	    gpcount++;
	  ਜੇ (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ਢੇਰ->fp_regs[fpcount++];
	      __float80 ਨਤੀਜਾ;
	      avalue[i] = addr;
	      ldf_fill (ਨਤੀਜਾ, addr);
	      *(__float80 *)addr = ਨਤੀਜਾ;
	    }
	  ਨਹੀਂਤਾਂ
	    avalue[i] = &ਢੇਰ->gp_regs[gpcount];
	  gpcount += 2;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
	  {
	    size_t ਮਾਪ = (*p_arg)->ਮਾਪ;
	    size_t align = (*p_arg)->alignment;
	    ਪੂਰਨ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ਜੇ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ਜੇ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_ਮਾਪ = hfa_type_ਮਾਪ (hfa_type);
		size_t offset = 0;
		size_t gp_ਵਿੱਥ = gpcount * 8;
		ਖਾਲੀ *addr = alloca (ਮਾਪ);

		avalue[i] = addr;

		ਜਦੋਂ (fpcount < 8
		       && offset < ਮਾਪ
		       && gp_ਵਿੱਥ < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ਢੇਰ->fp_regs[fpcount]);
		    offset += hfa_ਮਾਪ;
		    gp_ਵਿੱਥ += hfa_ਮਾਪ;
		    fpcount += 1;
		  }

		ਜੇ (offset < ਮਾਪ)
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ (addr + offset, (ਅੱਖਰ *)ਢੇਰ->gp_regs + gp_ਵਿੱਥ,
			  ਮਾਪ - offset);
	      }
	    ਨਹੀਂਤਾਂ
	      avalue[i] = &ਢੇਰ->gp_regs[gpcount];

	    gpcount += (ਮਾਪ + 7) / 8;
	  }
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort ();
	}
    }

  ਘੇਰਾ->fun (cif, rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

  ਵਾਪਸ cif->flags;
}

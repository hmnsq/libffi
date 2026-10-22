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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <float.h>

#அந்தர்பாவ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
வகைவரை வெறுமை *PTR64;
#இல்லையெனில் // some other unix
வகைவரை வெறுமை *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
வகைவரை கட்டமைப்பு
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

கட்டமைப்பு ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

நிலையான உள்வரி வெறுமை *
endian_adjust (வெறுமை *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  திரும்பு addr + (8 - len);
#இல்லையெனில்
  திரும்பு addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, மதிப்பு)
#இல்லையெனில்
#define stf_spill(addr, மதிப்பு)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(மதிப்பு));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#இல்லையெனில்
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

நிலையான size_t
hfa_type_size (முழு type)
{
  தேர்வு (type)
    {
    நிலை FFI_IA64_TYPE_HFA_FLOAT:
      திரும்பு அளவு(தசம);
    நிலை FFI_IA64_TYPE_HFA_DOUBLE:
      திரும்பு அளவு(இரட்டை);
    நிலை FFI_IA64_TYPE_HFA_LDOUBLE:
      திரும்பு அளவு(__float80);
    இயல்பு:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

நிலையான வெறுமை
hfa_type_load (fpreg *fpaddr, முழு type, வெறுமை *addr)
{
  தேர்வு (type)
    {
    நிலை FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(தசம *) addr);
      திரும்பு;
    நிலை FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(இரட்டை *) addr);
      திரும்பு;
    நிலை FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      திரும்பு;
    இயல்பு:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

நிலையான வெறுமை
hfa_type_store (முழு type, வெறுமை *addr, fpreg *fpaddr)
{
  தேர்வு (type)
    {
    நிலை FFI_IA64_TYPE_HFA_FLOAT:
      {
	தசம result;
	ldf_fill (result, fpaddr);
	*(தசம *) addr = result;
	நிறுத்து;
      }
    நிலை FFI_IA64_TYPE_HFA_DOUBLE:
      {
	இரட்டை result;
	ldf_fill (result, fpaddr);
	*(இரட்டை *) addr = result;
	நிறுத்து;
      }
    நிலை FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	நிறுத்து;
      }
    இயல்பு:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

நிலையான முழு
hfa_element_type (ffi_type *type, முழு nested)
{
  முழு element = FFI_TYPE_VOID;

  தேர்வு (type->type)
    {
    நிலை FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      எனில் (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      நிறுத்து;

    நிலை FFI_TYPE_DOUBLE:
      /* Similarly.  */
      எனில் (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      நிறுத்து;

    நிலை FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      எனில் (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	க்கு (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    முழு sub_element = hfa_element_type (*ptr, 1);
	    எனில் (sub_element == FFI_TYPE_VOID)
	      திரும்பு FFI_TYPE_VOID;

	    எனில் (element == FFI_TYPE_VOID)
	      element = sub_element;
	    இல்லையெனில் எனில் (element != sub_element)
	      திரும்பு FFI_TYPE_VOID;
	  }
      }
      நிறுத்து;

    இயல்பு:
      திரும்பு FFI_TYPE_VOID;
    }

  திரும்பு element;
}


/* Perform machine dependent cif processing. */

நிலையான ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  முழு flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(கட்டமைப்பு ia64_args, gp_regs[0]);
  எனில் (cif->bytes < அளவு(கட்டமைப்பு ia64_args))
    cif->bytes = அளவு(கட்டமைப்பு ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  தேர்வு (cif->rtype->type)
    {
    நிலை FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      எனில் (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	முழு hfa_type = hfa_element_type (cif->rtype, 0);

	எனில் (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    எனில் (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	இல்லையெனில்
	  {
	    எனில் (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      நிறுத்து;

    இயல்பு:
      நிறுத்து;
    }
  cif->flags = flags;

  திரும்பு FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  திரும்பு ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned முழு nfixedargs,
			 unsigned முழு ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  திரும்பு ffi_prep_cif_machdep_core(cif);
}

வெளி முழு ffi_call_unix (கட்டமைப்பு ia64_args *, PTR64, வெறுமை (*)(வெறுமை), UINT64);

வெறுமை
ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  கட்டமைப்பு ia64_args *அடுக்கு;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  எனில் (rvalue == வெற்று && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  அடுக்கு = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  க்கு (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      தேர்வு ((*p_arg)->type)
	{
	நிலை FFI_TYPE_SINT8:
	  அடுக்கு->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_UINT8:
	  அடுக்கு->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_SINT16:
	  அடுக்கு->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_UINT16:
	  அடுக்கு->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_SINT32:
	  அடுக்கு->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_UINT32:
	  அடுக்கு->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  நிறுத்து;
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	  அடுக்கு->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  நிறுத்து;

	நிலை FFI_TYPE_POINTER:
	  அடுக்கு->gp_regs[gpcount++] = (UINT64)(PTR64) *(வெறுமை **)avalue[i];
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  எனில் (gpcount < 8 && fpcount < 8)
	    stf_spill (&அடுக்கு->fp_regs[fpcount++], *(தசம *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], அளவு (UINT32));
	    அடுக்கு->gp_regs[gpcount++] = tmp;
	  }
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் (gpcount < 8 && fpcount < 8)
	    stf_spill (&அடுக்கு->fp_regs[fpcount++], *(இரட்டை *)avalue[i]);
	  memcpy (&அடுக்கு->gp_regs[gpcount++], avalue[i], அளவு (UINT64));
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	  எனில் (gpcount & 1)
	    gpcount++;
	  எனில் (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&அடுக்கு->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&அடுக்கு->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    முழு hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    எனில் (align == 16 && (gpcount & 1))
	      gpcount++;

	    எனில் (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		போது (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&அடுக்கு->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&அடுக்கு->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  நிறுத்து;

	இயல்பு:
	  abort ();
	}
    }

  ffi_call_unix (அடுக்கு, rvalue, fn, cif->flags);
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

வெளி வெறுமை ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  கட்டமைப்பு ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  கட்டமைப்பு ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  கட்டமைப்பு ffi_ia64_trampoline_struct *tramp;
  கட்டமைப்பு ia64_fd *fd;

  எனில் (cif->abi != FFI_UNIX)
    திரும்பு FFI_BAD_ABI;

  tramp = (கட்டமைப்பு ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (கட்டமைப்பு ia64_fd *)(வெறுமை *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  திரும்பு FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, கட்டமைப்பு ia64_args *அடுக்கு,
			வெறுமை *rvalue, வெறுமை *r8)
{
  ffi_cif *cif;
  வெறுமை **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * அளவு (வெறுமை *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  எனில் (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  க்கு (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      முழு named = i < nfixedargs;
      தேர்வு ((*p_arg)->type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&அடுக்கு->gp_regs[gpcount++], 1);
	  நிறுத்து;
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&அடுக்கு->gp_regs[gpcount++], 2);
	  நிறுத்து;
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&அடுக்கு->gp_regs[gpcount++], 4);
	  நிறுத்து;
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	  avalue[i] = &அடுக்கு->gp_regs[gpcount++];
	  நிறுத்து;
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&அடுக்கு->gp_regs[gpcount++], அளவு(வெறுமை*));
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  எனில் (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &அடுக்கு->fp_regs[fpcount++];
	      தசம result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(தசம *)addr = result;
	    }
	  இல்லையெனில்
	    avalue[i] = endian_adjust(&அடுக்கு->gp_regs[gpcount], 4);
	  gpcount++;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &அடுக்கு->fp_regs[fpcount++];
	      இரட்டை result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(இரட்டை *)addr = result;
	    }
	  இல்லையெனில்
	    avalue[i] = &அடுக்கு->gp_regs[gpcount];
	  gpcount++;
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	  எனில் (gpcount & 1)
	    gpcount++;
	  எனில் (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &அடுக்கு->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  இல்லையெனில்
	    avalue[i] = &அடுக்கு->gp_regs[gpcount];
	  gpcount += 2;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    முழு hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    எனில் (align == 16 && (gpcount & 1))
	      gpcount++;

	    எனில் (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		வெறுமை *addr = alloca (size);

		avalue[i] = addr;

		போது (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &அடுக்கு->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		எனில் (offset < size)
		  memcpy (addr + offset, (எழுத்து *)அடுக்கு->gp_regs + gp_offset,
			  size - offset);
	      }
	    இல்லையெனில்
	      avalue[i] = &அடுக்கு->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  நிறுத்து;

	இயல்பு:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  திரும்பு cif->flags;
}

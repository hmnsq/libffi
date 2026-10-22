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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>
#شامل <stdbool.h>
#شامل <float.h>

#شامل "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
تعریفنوع تهی *PTR64;
#وگرنه // some other unix
تعریفنوع تهی *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
تعریفنوع ساختار
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

ساختار ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ایستا درونخطی تهی *
endian_adjust (تهی *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  بازگشت addr + (8 - len);
#وگرنه
  بازگشت addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, مقدار)
#وگرنه
#define stf_spill(addr, مقدار)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(مقدار));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#وگرنه
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ایستا size_t
hfa_type_size (صحیح type)
{
  انتخاب (type)
    {
    حالت FFI_IA64_TYPE_HFA_FLOAT:
      بازگشت اندازه(اعشاری);
    حالت FFI_IA64_TYPE_HFA_DOUBLE:
      بازگشت اندازه(دوگانه);
    حالت FFI_IA64_TYPE_HFA_LDOUBLE:
      بازگشت اندازه(__float80);
    پیشفرض:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ایستا تهی
hfa_type_load (fpreg *fpaddr, صحیح type, تهی *addr)
{
  انتخاب (type)
    {
    حالت FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(اعشاری *) addr);
      بازگشت;
    حالت FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(دوگانه *) addr);
      بازگشت;
    حالت FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      بازگشت;
    پیشفرض:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ایستا تهی
hfa_type_store (صحیح type, تهی *addr, fpreg *fpaddr)
{
  انتخاب (type)
    {
    حالت FFI_IA64_TYPE_HFA_FLOAT:
      {
	اعشاری result;
	ldf_fill (result, fpaddr);
	*(اعشاری *) addr = result;
	بشکن;
      }
    حالت FFI_IA64_TYPE_HFA_DOUBLE:
      {
	دوگانه result;
	ldf_fill (result, fpaddr);
	*(دوگانه *) addr = result;
	بشکن;
      }
    حالت FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	بشکن;
      }
    پیشفرض:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ایستا صحیح
hfa_element_type (ffi_type *type, صحیح nested)
{
  صحیح element = FFI_TYPE_VOID;

  انتخاب (type->type)
    {
    حالت FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      اگر (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      بشکن;

    حالت FFI_TYPE_DOUBLE:
      /* Similarly.  */
      اگر (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      بشکن;

    حالت FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      اگر (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	برای (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    صحیح sub_element = hfa_element_type (*ptr, 1);
	    اگر (sub_element == FFI_TYPE_VOID)
	      بازگشت FFI_TYPE_VOID;

	    اگر (element == FFI_TYPE_VOID)
	      element = sub_element;
	    وگرنه اگر (element != sub_element)
	      بازگشت FFI_TYPE_VOID;
	  }
      }
      بشکن;

    پیشفرض:
      بازگشت FFI_TYPE_VOID;
    }

  بازگشت element;
}


/* Perform machine dependent cif processing. */

ایستا ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  صحیح flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(ساختار ia64_args, gp_regs[0]);
  اگر (cif->bytes < اندازه(ساختار ia64_args))
    cif->bytes = اندازه(ساختار ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      اگر (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      بشکن;

    حالت FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	صحیح hfa_type = hfa_element_type (cif->rtype, 0);

	اگر (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    اگر (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	وگرنه
	  {
	    اگر (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      بشکن;

    پیشفرض:
      بشکن;
    }
  cif->flags = flags;

  بازگشت FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  بازگشت ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned صحیح nfixedargs,
			 unsigned صحیح ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  بازگشت ffi_prep_cif_machdep_core(cif);
}

بیرونی صحیح ffi_call_unix (ساختار ia64_args *, PTR64, تهی (*)(تهی), UINT64);

تهی
ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ساختار ia64_args *پشته;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  اگر (rvalue == پوچ && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  پشته = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  برای (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      انتخاب ((*p_arg)->type)
	{
	حالت FFI_TYPE_SINT8:
	  پشته->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_UINT8:
	  پشته->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_SINT16:
	  پشته->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_UINT16:
	  پشته->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_SINT32:
	  پشته->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_UINT32:
	  پشته->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  بشکن;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  پشته->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  بشکن;

	حالت FFI_TYPE_POINTER:
	  پشته->gp_regs[gpcount++] = (UINT64)(PTR64) *(تهی **)avalue[i];
	  بشکن;

	حالت FFI_TYPE_FLOAT:
	  اگر (gpcount < 8 && fpcount < 8)
	    stf_spill (&پشته->fp_regs[fpcount++], *(اعشاری *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], اندازه (UINT32));
	    پشته->gp_regs[gpcount++] = tmp;
	  }
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر (gpcount < 8 && fpcount < 8)
	    stf_spill (&پشته->fp_regs[fpcount++], *(دوگانه *)avalue[i]);
	  memcpy (&پشته->gp_regs[gpcount++], avalue[i], اندازه (UINT64));
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	  اگر (gpcount & 1)
	    gpcount++;
	  اگر (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&پشته->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&پشته->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    صحیح hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    اگر (align == 16 && (gpcount & 1))
	      gpcount++;

	    اگر (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		وقتی (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&پشته->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&پشته->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  بشکن;

	پیشفرض:
	  abort ();
	}
    }

  ffi_call_unix (پشته, rvalue, fn, cif->flags);
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

بیرونی تهی ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  ساختار ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  ساختار ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  ساختار ffi_ia64_trampoline_struct *tramp;
  ساختار ia64_fd *fd;

  اگر (cif->abi != FFI_UNIX)
    بازگشت FFI_BAD_ABI;

  tramp = (ساختار ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (ساختار ia64_fd *)(تهی *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  بازگشت FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, ساختار ia64_args *پشته,
			تهی *rvalue, تهی *r8)
{
  ffi_cif *cif;
  تهی **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * اندازه (تهی *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  اگر (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  برای (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      صحیح named = i < nfixedargs;
      انتخاب ((*p_arg)->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&پشته->gp_regs[gpcount++], 1);
	  بشکن;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&پشته->gp_regs[gpcount++], 2);
	  بشکن;
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&پشته->gp_regs[gpcount++], 4);
	  بشکن;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  avalue[i] = &پشته->gp_regs[gpcount++];
	  بشکن;
	حالت FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&پشته->gp_regs[gpcount++], اندازه(تهی*));
	  بشکن;

	حالت FFI_TYPE_FLOAT:
	  اگر (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &پشته->fp_regs[fpcount++];
	      اعشاری result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(اعشاری *)addr = result;
	    }
	  وگرنه
	    avalue[i] = endian_adjust(&پشته->gp_regs[gpcount], 4);
	  gpcount++;
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &پشته->fp_regs[fpcount++];
	      دوگانه result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(دوگانه *)addr = result;
	    }
	  وگرنه
	    avalue[i] = &پشته->gp_regs[gpcount];
	  gpcount++;
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	  اگر (gpcount & 1)
	    gpcount++;
	  اگر (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &پشته->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  وگرنه
	    avalue[i] = &پشته->gp_regs[gpcount];
	  gpcount += 2;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    صحیح hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    اگر (align == 16 && (gpcount & 1))
	      gpcount++;

	    اگر (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		تهی *addr = alloca (size);

		avalue[i] = addr;

		وقتی (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &پشته->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		اگر (offset < size)
		  memcpy (addr + offset, (نویسه *)پشته->gp_regs + gp_offset,
			  size - offset);
	      }
	    وگرنه
	      avalue[i] = &پشته->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  بشکن;

	پیشفرض:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  بازگشت cif->flags;
}

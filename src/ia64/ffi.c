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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <float.h>

#অন্তর্ভাব "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ifdef __hpux
প্রকারলেখ শূন্য *PTR64;
#অন্যথা // some other unix
প্রকারলেখ শূন্য *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
প্রকারলেখ কাঠামো
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

কাঠামো ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

স্থির অন্তঃরেখ শূন্য *
endian_adjust (শূন্য *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  ফেরত addr + (8 - len);
#অন্যথা
  ফেরত addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, মান)
#অন্যথা
#define stf_spill(addr, মান)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(মান));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(ফলাফল, addr)
#অন্যথা
#define ldf_fill(ফলাফল, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(ফলাফল) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

স্থির size_t
hfa_type_আকার (পূর্ণসংখ্যা type)
{
  নির্বাচন (type)
    {
    ক্ষেত্র FFI_IA64_TYPE_HFA_FLOAT:
      ফেরত আকার(দশমিক);
    ক্ষেত্র FFI_IA64_TYPE_HFA_DOUBLE:
      ফেরত আকার(দ্বিগুণ);
    ক্ষেত্র FFI_IA64_TYPE_HFA_LDOUBLE:
      ফেরত আকার(__float80);
    পূর্বনির্ধারিত:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

স্থির শূন্য
hfa_type_load (fpreg *fpaddr, পূর্ণসংখ্যা type, শূন্য *addr)
{
  নির্বাচন (type)
    {
    ক্ষেত্র FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(দশমিক *) addr);
      ফেরত;
    ক্ষেত্র FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(দ্বিগুণ *) addr);
      ফেরত;
    ক্ষেত্র FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      ফেরত;
    পূর্বনির্ধারিত:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

স্থির শূন্য
hfa_type_store (পূর্ণসংখ্যা type, শূন্য *addr, fpreg *fpaddr)
{
  নির্বাচন (type)
    {
    ক্ষেত্র FFI_IA64_TYPE_HFA_FLOAT:
      {
	দশমিক ফলাফল;
	ldf_fill (ফলাফল, fpaddr);
	*(দশমিক *) addr = ফলাফল;
	বিরতি;
      }
    ক্ষেত্র FFI_IA64_TYPE_HFA_DOUBLE:
      {
	দ্বিগুণ ফলাফল;
	ldf_fill (ফলাফল, fpaddr);
	*(দ্বিগুণ *) addr = ফলাফল;
	বিরতি;
      }
    ক্ষেত্র FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 ফলাফল;
	ldf_fill (ফলাফল, fpaddr);
	*(__float80 *) addr = ফলাফল;
	বিরতি;
      }
    পূর্বনির্ধারিত:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

স্থির পূর্ণসংখ্যা
hfa_element_type (ffi_type *type, পূর্ণসংখ্যা nested)
{
  পূর্ণসংখ্যা element = FFI_TYPE_VOID;

  নির্বাচন (type->type)
    {
    ক্ষেত্র FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      যদি (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      বিরতি;

    ক্ষেত্র FFI_TYPE_DOUBLE:
      /* Similarly.  */
      যদি (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      বিরতি;

    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      যদি (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	জন্য (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    পূর্ণসংখ্যা sub_element = hfa_element_type (*ptr, 1);
	    যদি (sub_element == FFI_TYPE_VOID)
	      ফেরত FFI_TYPE_VOID;

	    যদি (element == FFI_TYPE_VOID)
	      element = sub_element;
	    অন্যথা যদি (element != sub_element)
	      ফেরত FFI_TYPE_VOID;
	  }
      }
      বিরতি;

    পূর্বনির্ধারিত:
      ফেরত FFI_TYPE_VOID;
    }

  ফেরত element;
}


/* Perform machine dependent cif processing. */

স্থির ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  পূর্ণসংখ্যা flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(কাঠামো ia64_args, gp_regs[0]);
  যদি (cif->bytes < আকার(কাঠামো ia64_args))
    cif->bytes = আকার(কাঠামো ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      যদি (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      {
        size_t আকার_2 = cif->rtype->আকার_2;
  	পূর্ণসংখ্যা hfa_type = hfa_element_type (cif->rtype, 0);

	যদি (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = আকার_2 / hfa_type_আকার (hfa_type);
	    যদি (nelts <= 8)
	      flags = hfa_type | (আকার_2 << 8);
	  }
	অন্যথা
	  {
	    যদি (আকার_2 <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (আকার_2 << 8);
	  }
      }
      বিরতি;

    পূর্বনির্ধারিত:
      বিরতি;
    }
  cif->flags = flags;

  ফেরত FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ফেরত ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned পূর্ণসংখ্যা nfixedargs,
			 unsigned পূর্ণসংখ্যা ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  ফেরত ffi_prep_cif_machdep_core(cif);
}

বাহ্যিক পূর্ণসংখ্যা ffi_call_unix (কাঠামো ia64_args *, PTR64, শূন্য (*)(শূন্য), UINT64);

শূন্য
ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  কাঠামো ia64_args *স্তূপ;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  যদি (rvalue == ফাঁকা && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->আকার_2);
    
  /* Allocate the stack frame.  */
  স্তূপ = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  জন্য (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      নির্বাচন ((*p_arg)->type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	  স্তূপ->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_UINT8:
	  স্তূপ->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	  স্তূপ->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_UINT16:
	  স্তূপ->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT32:
	  স্তূপ->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_UINT32:
	  স্তূপ->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  স্তূপ->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  বিরতি;

	ক্ষেত্র FFI_TYPE_POINTER:
	  স্তূপ->gp_regs[gpcount++] = (UINT64)(PTR64) *(শূন্য **)avalue[i];
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  যদি (gpcount < 8 && fpcount < 8)
	    stf_spill (&স্তূপ->fp_regs[fpcount++], *(দশমিক *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], আকার (UINT32));
	    স্তূপ->gp_regs[gpcount++] = tmp;
	  }
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি (gpcount < 8 && fpcount < 8)
	    stf_spill (&স্তূপ->fp_regs[fpcount++], *(দ্বিগুণ *)avalue[i]);
	  memcpy (&স্তূপ->gp_regs[gpcount++], avalue[i], আকার (UINT64));
	  বিরতি;

	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  যদি (gpcount & 1)
	    gpcount++;
	  যদি (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&স্তূপ->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&স্তূপ->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_STRUCT:
	  {
	    size_t আকার_2 = (*p_arg)->আকার_2;
	    size_t align = (*p_arg)->alignment;
	    পূর্ণসংখ্যা hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    যদি (align == 16 && (gpcount & 1))
	      gpcount++;

	    যদি (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_আকার = hfa_type_আকার (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		যখন (fpcount < 8
		       && offset < আকার_2
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&স্তূপ->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_আকার;
		    gp_offset += hfa_আকার;
		    fpcount += 1;
		  }
	      }

	    memcpy (&স্তূপ->gp_regs[gpcount], avalue[i], আকার_2);
	    gpcount += (আকার_2 + 7) / 8;
	  }
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort ();
	}
    }

  ffi_call_unix (স্তূপ, rvalue, fn, cif->flags);
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

বাহ্যিক শূন্য ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  কাঠামো ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  কাঠামো ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  কাঠামো ffi_ia64_trampoline_struct *tramp;
  কাঠামো ia64_fd *fd;

  যদি (cif->abi != FFI_UNIX)
    ফেরত FFI_BAD_ABI;

  tramp = (কাঠামো ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (কাঠামো ia64_fd *)(শূন্য *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_তথ্য = user_তথ্য;
  closure->fun = fun;

  ফেরত FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, কাঠামো ia64_args *স্তূপ,
			শূন্য *rvalue, শূন্য *r8)
{
  ffi_cif *cif;
  শূন্য **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * আকার (শূন্য *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  যদি (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  জন্য (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      পূর্ণসংখ্যা named = i < nfixedargs;
      নির্বাচন ((*p_arg)->type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&স্তূপ->gp_regs[gpcount++], 1);
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&স্তূপ->gp_regs[gpcount++], 2);
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&স্তূপ->gp_regs[gpcount++], 4);
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  avalue[i] = &স্তূপ->gp_regs[gpcount++];
	  বিরতি;
	ক্ষেত্র FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&স্তূপ->gp_regs[gpcount++], আকার(শূন্য*));
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  যদি (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &স্তূপ->fp_regs[fpcount++];
	      দশমিক ফলাফল;
	      avalue[i] = addr;
	      ldf_fill (ফলাফল, addr);
	      *(দশমিক *)addr = ফলাফল;
	    }
	  অন্যথা
	    avalue[i] = endian_adjust(&স্তূপ->gp_regs[gpcount], 4);
	  gpcount++;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &স্তূপ->fp_regs[fpcount++];
	      দ্বিগুণ ফলাফল;
	      avalue[i] = addr;
	      ldf_fill (ফলাফল, addr);
	      *(দ্বিগুণ *)addr = ফলাফল;
	    }
	  অন্যথা
	    avalue[i] = &স্তূপ->gp_regs[gpcount];
	  gpcount++;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  যদি (gpcount & 1)
	    gpcount++;
	  যদি (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &স্তূপ->fp_regs[fpcount++];
	      __float80 ফলাফল;
	      avalue[i] = addr;
	      ldf_fill (ফলাফল, addr);
	      *(__float80 *)addr = ফলাফল;
	    }
	  অন্যথা
	    avalue[i] = &স্তূপ->gp_regs[gpcount];
	  gpcount += 2;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_STRUCT:
	  {
	    size_t আকার_2 = (*p_arg)->আকার_2;
	    size_t align = (*p_arg)->alignment;
	    পূর্ণসংখ্যা hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    যদি (align == 16 && (gpcount & 1))
	      gpcount++;

	    যদি (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_আকার = hfa_type_আকার (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		শূন্য *addr = alloca (আকার_2);

		avalue[i] = addr;

		যখন (fpcount < 8
		       && offset < আকার_2
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &স্তূপ->fp_regs[fpcount]);
		    offset += hfa_আকার;
		    gp_offset += hfa_আকার;
		    fpcount += 1;
		  }

		যদি (offset < আকার_2)
		  memcpy (addr + offset, (অক্ষর *)স্তূপ->gp_regs + gp_offset,
			  আকার_2 - offset);
	      }
	    অন্যথা
	      avalue[i] = &স্তূপ->gp_regs[gpcount];

	    gpcount += (আকার_2 + 7) / 8;
	  }
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_তথ্য);

  ফেরত cif->flags;
}

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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>
#ضمن <stdbool.h>
#ضمن <float.h>

#ضمن "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#که_تعریف_شوی __hpux
تعريفنوع فراغ *PTR64;
#وإلا // some other unix
تعريفنوع فراغ *PTR64 __attribute__((ډېرپېښ(DI)));
#د_که_پای

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
تعريفنوع بنية
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

بنية ia64_دلیلونه
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_دلیلونه[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ساكن مضمن فراغ *
endian_adjust (فراغ *addr, size_t len)
{
#که_تعریف_شوی __BIG_ENDIAN__
  أرجع addr + (8 - len);
#وإلا
  أرجع addr;
#د_که_پای
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#که_تعریف_شوی __hpux
#تعریف_کړه stf_spill(addr, ارزښت)
#وإلا
#تعریف_کړه stf_spill(addr, ارزښت)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(ارزښت));
#د_که_پای

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#که_تعریف_شوی __hpux
#تعریف_کړه ldf_fill(پایله, addr)
#وإلا
#تعریف_کړه ldf_fill(پایله, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(پایله) : "m"(*addr));
#د_که_پای

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ساكن size_t
hfa_type_اندازه (صحيح type)
{
  بدل (type)
    {
    حالة FFI_IA64_TYPE_HFA_FLOAT:
      أرجع حجم(عائم);
    حالة FFI_IA64_TYPE_HFA_DOUBLE:
      أرجع حجم(مزدوج);
    حالة FFI_IA64_TYPE_HFA_LDOUBLE:
      أرجع حجم(__float80);
    افتراضي:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ساكن فراغ
hfa_type_بارول (fpreg *fpaddr, صحيح type, فراغ *addr)
{
  بدل (type)
    {
    حالة FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(عائم *) addr);
      أرجع;
    حالة FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(مزدوج *) addr);
      أرجع;
    حالة FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      أرجع;
    افتراضي:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ساكن فراغ
hfa_type_store (صحيح type, فراغ *addr, fpreg *fpaddr)
{
  بدل (type)
    {
    حالة FFI_IA64_TYPE_HFA_FLOAT:
      {
	عائم پایله;
	ldf_fill (پایله, fpaddr);
	*(عائم *) addr = پایله;
	اكسر;
      }
    حالة FFI_IA64_TYPE_HFA_DOUBLE:
      {
	مزدوج پایله;
	ldf_fill (پایله, fpaddr);
	*(مزدوج *) addr = پایله;
	اكسر;
      }
    حالة FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 پایله;
	ldf_fill (پایله, fpaddr);
	*(__float80 *) addr = پایله;
	اكسر;
      }
    افتراضي:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ساكن صحيح
hfa_element_type (ffi_type *type, صحيح nested)
{
  صحيح element = FFI_TYPE_VOID;

  بدل (type->type)
    {
    حالة FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      إذا (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      اكسر;

    حالة FFI_TYPE_DOUBLE:
      /* Similarly.  */
      إذا (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      اكسر;

    حالة FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      إذا (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	لكل (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    صحيح sub_element = hfa_element_type (*ptr, 1);
	    إذا (sub_element == FFI_TYPE_VOID)
	      أرجع FFI_TYPE_VOID;

	    إذا (element == FFI_TYPE_VOID)
	      element = sub_element;
	    وإلا إذا (element != sub_element)
	      أرجع FFI_TYPE_VOID;
	  }
      }
      اكسر;

    افتراضي:
      أرجع FFI_TYPE_VOID;
    }

  أرجع element;
}


/* Perform machine dependent cif processing. */

ساكن ffi_حالت
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  صحيح flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->بایټونه += offsetof(بنية ia64_دلیلونه, gp_regs[0]);
  إذا (cif->بایټونه < حجم(بنية ia64_دلیلونه))
    cif->بایټونه = حجم(بنية ia64_دلیلونه);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      إذا (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      اكسر;

    حالة FFI_TYPE_STRUCT:
      {
        size_t اندازه = cif->rtype->اندازه;
  	صحيح hfa_type = hfa_element_type (cif->rtype, 0);

	إذا (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = اندازه / hfa_type_اندازه (hfa_type);
	    إذا (nelts <= 8)
	      flags = hfa_type | (اندازه << 8);
	  }
	وإلا
	  {
	    إذا (اندازه <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (اندازه << 8);
	  }
      }
      اكسر;

    افتراضي:
      اكسر;
    }
  cif->flags = flags;

  أرجع FFI_OK;
}

ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

ffi_حالت
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 بینښه صحيح nfixedargs,
			 بینښه صحيح ntotalargs MAYBE_ناکارول_شوی)
{
  cif->nfixedargs = nfixedargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

خارجي صحيح ffi_call_unix (بنية ia64_دلیلونه *, PTR64, فراغ (*)(فراغ), UINT64);

فراغ
ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  بنية ia64_دلیلونه *ډېری;
  اوږد i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  إذا (rvalue == عدم && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->اندازه);
    
  /* Allocate the stack frame.  */
  ډېری = alloca (cif->بایټونه);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      بدل ((*p_arg)->type)
	{
	حالة FFI_TYPE_SINT8:
	  ډېری->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_UINT8:
	  ډېری->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_SINT16:
	  ډېری->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_UINT16:
	  ډېری->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_SINT32:
	  ډېری->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_UINT32:
	  ډېری->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  ډېری->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  اكسر;

	حالة FFI_TYPE_اشاره_ګر:
	  ډېری->gp_regs[gpcount++] = (UINT64)(PTR64) *(فراغ **)avalue[i];
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  إذا (gpcount < 8 && fpcount < 8)
	    stf_spill (&ډېری->fp_regs[fpcount++], *(عائم *)avalue[i]);
	  {
	    UINT32 tmp;
	    حافظه_نقل (&tmp, avalue[i], حجم (UINT32));
	    ډېری->gp_regs[gpcount++] = tmp;
	  }
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (gpcount < 8 && fpcount < 8)
	    stf_spill (&ډېری->fp_regs[fpcount++], *(مزدوج *)avalue[i]);
	  حافظه_نقل (&ډېری->gp_regs[gpcount++], avalue[i], حجم (UINT64));
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	  إذا (gpcount & 1)
	    gpcount++;
	  إذا (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ډېری->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  حافظه_نقل (&ډېری->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  {
	    size_t اندازه = (*p_arg)->اندازه;
	    size_t align = (*p_arg)->alignment;
	    صحيح hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    إذا (align == 16 && (gpcount & 1))
	      gpcount++;

	    إذا (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_اندازه = hfa_type_اندازه (hfa_type);
		size_t offset = 0;
		size_t gp_دننه_واټن = gpcount * 8;

		طالما (fpcount < 8
		       && offset < اندازه
		       && gp_دننه_واټن < 8 * 8)
		  {
		    hfa_type_بارول (&ډېری->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_اندازه;
		    gp_دننه_واټن += hfa_اندازه;
		    fpcount += 1;
		  }
	      }

	    حافظه_نقل (&ډېری->gp_regs[gpcount], avalue[i], اندازه);
	    gpcount += (اندازه + 7) / 8;
	  }
	  اكسر;

	افتراضي:
	  abort ();
	}
    }

  ffi_call_unix (ډېری, rvalue, fn, cif->flags);
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

خارجي فراغ ffi_تړون_unix ();

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  بنية ia64_fd
  {
    UINT64 کوډ_اشاره_ګر;
    UINT64 gp;
  };

  بنية ffi_ia64_trampoline_struct
  {
    UINT64 کوډ_اشاره_ګر;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  بنية ffi_ia64_trampoline_struct *tramp;
  بنية ia64_fd *fd;

  إذا (cif->abi != FFI_UNIX)
    أرجع FFI_BAD_ABI;

  tramp = (بنية ffi_ia64_trampoline_struct *)تړون->tramp;
  fd = (بنية ia64_fd *)(فراغ *)ffi_تړون_unix;

  tramp->کوډ_اشاره_ګر = fd->کوډ_اشاره_ګر;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  تړون->cif = cif;
  تړون->user_معلومات = user_معلومات;
  تړون->fun = fun;

  أرجع FFI_OK;
}


UINT64
ffi_تړون_unix_inner (ffi_تړون *تړون, بنية ia64_دلیلونه *ډېری,
			فراغ *rvalue, فراغ *r8)
{
  ffi_cif *cif;
  فراغ **avalue;
  ffi_type **p_arg;
  اوږد i, avn, gpcount, fpcount, nfixedargs;

  cif = تړون->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * حجم (فراغ *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  إذا (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  لكل (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      صحيح named = i < nfixedargs;
      بدل ((*p_arg)->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ډېری->gp_regs[gpcount++], 1);
	  اكسر;
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ډېری->gp_regs[gpcount++], 2);
	  اكسر;
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ډېری->gp_regs[gpcount++], 4);
	  اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  avalue[i] = &ډېری->gp_regs[gpcount++];
	  اكسر;
	حالة FFI_TYPE_اشاره_ګر:
	  avalue[i] = endian_adjust(&ډېری->gp_regs[gpcount++], حجم(فراغ*));
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  إذا (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ډېری->fp_regs[fpcount++];
	      عائم پایله;
	      avalue[i] = addr;
	      ldf_fill (پایله, addr);
	      *(عائم *)addr = پایله;
	    }
	  وإلا
	    avalue[i] = endian_adjust(&ډېری->gp_regs[gpcount], 4);
	  gpcount++;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ډېری->fp_regs[fpcount++];
	      مزدوج پایله;
	      avalue[i] = addr;
	      ldf_fill (پایله, addr);
	      *(مزدوج *)addr = پایله;
	    }
	  وإلا
	    avalue[i] = &ډېری->gp_regs[gpcount];
	  gpcount++;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	  إذا (gpcount & 1)
	    gpcount++;
	  إذا (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ډېری->fp_regs[fpcount++];
	      __float80 پایله;
	      avalue[i] = addr;
	      ldf_fill (پایله, addr);
	      *(__float80 *)addr = پایله;
	    }
	  وإلا
	    avalue[i] = &ډېری->gp_regs[gpcount];
	  gpcount += 2;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
	  {
	    size_t اندازه = (*p_arg)->اندازه;
	    size_t align = (*p_arg)->alignment;
	    صحيح hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    إذا (align == 16 && (gpcount & 1))
	      gpcount++;

	    إذا (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_اندازه = hfa_type_اندازه (hfa_type);
		size_t offset = 0;
		size_t gp_دننه_واټن = gpcount * 8;
		فراغ *addr = alloca (اندازه);

		avalue[i] = addr;

		طالما (fpcount < 8
		       && offset < اندازه
		       && gp_دننه_واټن < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ډېری->fp_regs[fpcount]);
		    offset += hfa_اندازه;
		    gp_دننه_واټن += hfa_اندازه;
		    fpcount += 1;
		  }

		إذا (offset < اندازه)
		  حافظه_نقل (addr + offset, (حرف *)ډېری->gp_regs + gp_دننه_واټن,
			  اندازه - offset);
	      }
	    وإلا
	      avalue[i] = &ډېری->gp_regs[gpcount];

	    gpcount += (اندازه + 7) / 8;
	  }
	  اكسر;

	افتراضي:
	  abort ();
	}
    }

  تړون->fun (cif, rvalue, avalue, تړون->user_معلومات);

  أرجع cif->flags;
}

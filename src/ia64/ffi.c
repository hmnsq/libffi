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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <float.h>

#ಅನ್ತರ್ಭಾವ "ia64_flags.h"

/* A 64-bit pointer value.  In LP64 mode, this is effectively a plain
   pointer.  In ILP32 mode, it's a pointer that's been extended to 
   64 bits by "addp4".  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __hpux
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ *PTR64;
#ಇಲ್ಲದಿದ್ದರೆ // some other unix
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ *PTR64 __attribute__((ಬಹುಲಕ(DI)));
#ಆದರೆ_ಅಂತ್ಯ

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

ರಚನೆ ia64_ವಾದಗಳು
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_ವಾದಗಳು[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ *
endian_adjust (ಶೂನ್ಯ *addr, size_t len)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __BIG_ENDIAN__
  ಹಿಂತಿರುಗಿ addr + (8 - len);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ addr;
#ಆದರೆ_ಅಂತ್ಯ
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __hpux
#ವ್ಯಾಖ್ಯಾನಿಸು stf_spill(addr, ಮೌಲ್ಯ)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು stf_spill(addr, ಮೌಲ್ಯ)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(ಮೌಲ್ಯ));
#ಆದರೆ_ಅಂತ್ಯ

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __hpux
#ವ್ಯಾಖ್ಯಾನಿಸು ldf_fill(ಫಲಿತಾಂಶ, addr)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ldf_fill(ಫಲಿತಾಂಶ, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(ಫಲಿತಾಂಶ) : "m"(*addr));
#ಆದರೆ_ಅಂತ್ಯ

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ size_t
hfa_type_ಗಾತ್ರ (ಪೂರ್ಣಾಂಕ type)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_FLOAT:
      ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ದಶಮಾಂಶ);
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_DOUBLE:
      ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ದ್ವಿಗುಣ);
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_LDOUBLE:
      ಹಿಂತಿರುಗಿ ಗಾತ್ರ(__float80);
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ ಶೂನ್ಯ
hfa_type_ತುಂಬು (fpreg *fpaddr, ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *addr)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(ದಶಮಾಂಶ *) addr);
      ಹಿಂತಿರುಗಿ;
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(ದ್ವಿಗುಣ *) addr);
      ಹಿಂತಿರುಗಿ;
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      ಹಿಂತಿರುಗಿ;
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ ಶೂನ್ಯ
hfa_type_store (ಪೂರ್ಣಾಂಕ type, ಶೂನ್ಯ *addr, fpreg *fpaddr)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_FLOAT:
      {
	ದಶಮಾಂಶ ಫಲಿತಾಂಶ;
	ldf_fill (ಫಲಿತಾಂಶ, fpaddr);
	*(ದಶಮಾಂಶ *) addr = ಫಲಿತಾಂಶ;
	ನಿಲ್ಲಿಸು;
      }
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_DOUBLE:
      {
	ದ್ವಿಗುಣ ಫಲಿತಾಂಶ;
	ldf_fill (ಫಲಿತಾಂಶ, fpaddr);
	*(ದ್ವಿಗುಣ *) addr = ಫಲಿತಾಂಶ;
	ನಿಲ್ಲಿಸು;
      }
    ಸ್ಥಿತಿ FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 ಫಲಿತಾಂಶ;
	ldf_fill (ಫಲಿತಾಂಶ, fpaddr);
	*(__float80 *) addr = ಫಲಿತಾಂಶ;
	ನಿಲ್ಲಿಸು;
      }
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
hfa_element_type (ffi_type *type, ಪೂರ್ಣಾಂಕ nested)
{
  ಪೂರ್ಣಾಂಕ element = FFI_TYPE_VOID;

  ಆಯ್ಕೆ (type->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      ಆದರೆ (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      /* Similarly.  */
      ಆದರೆ (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      ಆದರೆ (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	ಗಾಗಿ (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    ಪೂರ್ಣಾಂಕ sub_element = hfa_element_type (*ptr, 1);
	    ಆದರೆ (sub_element == FFI_TYPE_VOID)
	      ಹಿಂತಿರುಗಿ FFI_TYPE_VOID;

	    ಆದರೆ (element == FFI_TYPE_VOID)
	      element = sub_element;
	    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (element != sub_element)
	      ಹಿಂತಿರುಗಿ FFI_TYPE_VOID;
	  }
      }
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_TYPE_VOID;
    }

  ಹಿಂತಿರುಗಿ element;
}


/* Perform machine dependent cif processing. */

ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ಪೂರ್ಣಾಂಕ flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->ಬೈಟುಗಳು += offsetof(ರಚನೆ ia64_ವಾದಗಳು, gp_regs[0]);
  ಆದರೆ (cif->ಬೈಟುಗಳು < ಗಾತ್ರ(ರಚನೆ ia64_ವಾದಗಳು))
    cif->ಬೈಟುಗಳು = ಗಾತ್ರ(ರಚನೆ ia64_ವಾದಗಳು);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  ಆಯ್ಕೆ (cif->rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      ಆದರೆ (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      {
        size_t ಗಾತ್ರ_2 = cif->rtype->ಗಾತ್ರ_2;
  	ಪೂರ್ಣಾಂಕ hfa_type = hfa_element_type (cif->rtype, 0);

	ಆದರೆ (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = ಗಾತ್ರ_2 / hfa_type_ಗಾತ್ರ (hfa_type);
	    ಆದರೆ (nelts <= 8)
	      flags = hfa_type | (ಗಾತ್ರ_2 << 8);
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    ಆದರೆ (ಗಾತ್ರ_2 <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (ಗಾತ್ರ_2 << 8);
	  }
      }
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      ನಿಲ್ಲಿಸು;
    }
  cif->flags = flags;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_core(cif);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
			 ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs MAYBE_ಬಳಸದ)
{
  cif->nfixedargs = nfixedargs;
  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_core(cif);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ffi_call_unix (ರಚನೆ ia64_ವಾದಗಳು *, PTR64, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), UINT64);

ಶೂನ್ಯ
ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ರಚನೆ ia64_ವಾದಗಳು *ರಾಶಿ;
  ದೀರ್ಘ i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  ಆದರೆ (rvalue == ಖಾಲಿ && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);
    
  /* Allocate the stack frame.  */
  ರಾಶಿ = alloca (cif->ಬೈಟುಗಳು);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  ಗಾಗಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ಆಯ್ಕೆ ((*p_arg)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  ರಾಶಿ->gp_regs[gpcount++] = (UINT64)(PTR64) *(ಶೂನ್ಯ **)avalue[i];
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಆದರೆ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(ದಶಮಾಂಶ *)avalue[i]);
	  {
	    UINT32 tmp;
	    ಸ್ಮರಣೆ_ನಕಲು (&tmp, avalue[i], ಗಾತ್ರ (UINT32));
	    ರಾಶಿ->gp_regs[gpcount++] = tmp;
	  }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(ದ್ವಿಗುಣ *)avalue[i]);
	  ಸ್ಮರಣೆ_ನಕಲು (&ರಾಶಿ->gp_regs[gpcount++], avalue[i], ಗಾತ್ರ (UINT64));
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  ಆದರೆ (gpcount & 1)
	    gpcount++;
	  ಆದರೆ (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  ಸ್ಮರಣೆ_ನಕಲು (&ರಾಶಿ->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  {
	    size_t ಗಾತ್ರ_2 = (*p_arg)->ಗಾತ್ರ_2;
	    size_t align = (*p_arg)->alignment;
	    ಪೂರ್ಣಾಂಕ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ಆದರೆ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ಆದರೆ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_ಗಾತ್ರ = hfa_type_ಗಾತ್ರ (hfa_type);
		size_t offset = 0;
		size_t gp_ಸ್ಥಾನಾಂತರ = gpcount * 8;

		ಆಗ (fpcount < 8
		       && offset < ಗಾತ್ರ_2
		       && gp_ಸ್ಥಾನಾಂತರ < 8 * 8)
		  {
		    hfa_type_ತುಂಬು (&ರಾಶಿ->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_ಗಾತ್ರ;
		    gp_ಸ್ಥಾನಾಂತರ += hfa_ಗಾತ್ರ;
		    fpcount += 1;
		  }
	      }

	    ಸ್ಮರಣೆ_ನಕಲು (&ರಾಶಿ->gp_regs[gpcount], avalue[i], ಗಾತ್ರ_2);
	    gpcount += (ಗಾತ್ರ_2 + 7) / 8;
	  }
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort ();
	}
    }

  ffi_call_unix (ರಾಶಿ, rvalue, fn, cif->flags);
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

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_unix ();

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  ರಚನೆ ia64_fd
  {
    UINT64 ಸಂಕೇತ_ಸೂಚಕ;
    UINT64 gp;
  };

  ರಚನೆ ffi_ia64_trampoline_struct
  {
    UINT64 ಸಂಕೇತ_ಸೂಚಕ;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  ರಚನೆ ffi_ia64_trampoline_struct *tramp;
  ರಚನೆ ia64_fd *fd;

  ಆದರೆ (cif->abi != FFI_UNIX)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  tramp = (ರಚನೆ ffi_ia64_trampoline_struct *)ಆವರಣ->tramp;
  fd = (ರಚನೆ ia64_fd *)(ಶೂನ್ಯ *)ffi_ಆವರಣ_unix;

  tramp->ಸಂಕೇತ_ಸೂಚಕ = fd->ಸಂಕೇತ_ಸೂಚಕ;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  ಆವರಣ->cif = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}


UINT64
ffi_ಆವರಣ_unix_inner (ffi_ಆವರಣ *ಆವರಣ, ರಚನೆ ia64_ವಾದಗಳು *ರಾಶಿ,
			ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ *r8)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  ದೀರ್ಘ i, avn, gpcount, fpcount, nfixedargs;

  cif = ಆವರಣ->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * ಗಾತ್ರ (ಶೂನ್ಯ *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  ಆದರೆ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  ಗಾಗಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ಪೂರ್ಣಾಂಕ named = i < nfixedargs;
      ಆಯ್ಕೆ ((*p_arg)->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 1);
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 2);
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 4);
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  avalue[i] = &ರಾಶಿ->gp_regs[gpcount++];
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], ಗಾತ್ರ(ಶೂನ್ಯ*));
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಆದರೆ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      ದಶಮಾಂಶ ಫಲಿತಾಂಶ;
	      avalue[i] = addr;
	      ldf_fill (ಫಲಿತಾಂಶ, addr);
	      *(ದಶಮಾಂಶ *)addr = ಫಲಿತಾಂಶ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount], 4);
	  gpcount++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      ದ್ವಿಗುಣ ಫಲಿತಾಂಶ;
	      avalue[i] = addr;
	      ldf_fill (ಫಲಿತಾಂಶ, addr);
	      *(ದ್ವಿಗುಣ *)addr = ಫಲಿತಾಂಶ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = &ರಾಶಿ->gp_regs[gpcount];
	  gpcount++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  ಆದರೆ (gpcount & 1)
	    gpcount++;
	  ಆದರೆ (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      __float80 ಫಲಿತಾಂಶ;
	      avalue[i] = addr;
	      ldf_fill (ಫಲಿತಾಂಶ, addr);
	      *(__float80 *)addr = ಫಲಿತಾಂಶ;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    avalue[i] = &ರಾಶಿ->gp_regs[gpcount];
	  gpcount += 2;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  {
	    size_t ಗಾತ್ರ_2 = (*p_arg)->ಗಾತ್ರ_2;
	    size_t align = (*p_arg)->alignment;
	    ಪೂರ್ಣಾಂಕ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ಆದರೆ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ಆದರೆ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_ಗಾತ್ರ = hfa_type_ಗಾತ್ರ (hfa_type);
		size_t offset = 0;
		size_t gp_ಸ್ಥಾನಾಂತರ = gpcount * 8;
		ಶೂನ್ಯ *addr = alloca (ಗಾತ್ರ_2);

		avalue[i] = addr;

		ಆಗ (fpcount < 8
		       && offset < ಗಾತ್ರ_2
		       && gp_ಸ್ಥಾನಾಂತರ < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ರಾಶಿ->fp_regs[fpcount]);
		    offset += hfa_ಗಾತ್ರ;
		    gp_ಸ್ಥಾನಾಂತರ += hfa_ಗಾತ್ರ;
		    fpcount += 1;
		  }

		ಆದರೆ (offset < ಗಾತ್ರ_2)
		  ಸ್ಮರಣೆ_ನಕಲು (addr + offset, (ಅಕ್ಷರ *)ರಾಶಿ->gp_regs + gp_ಸ್ಥಾನಾಂತರ,
			  ಗಾತ್ರ_2 - offset);
	      }
	    ಇಲ್ಲದಿದ್ದರೆ
	      avalue[i] = &ರಾಶಿ->gp_regs[gpcount];

	    gpcount += (ಗಾತ್ರ_2 + 7) / 8;
	  }
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort ();
	}
    }

  ಆವರಣ->fun (cif, rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ cif->flags;
}

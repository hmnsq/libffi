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
#ifdef __hpux
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ *PTR64;
#ಅನ್ಯಥಾ // some other unix
ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ *PTR64 __attribute__((mode(DI)));
#endif

/* Memory image of fp register contents.  This is the implementation
   specific format used by ldf.fill/stf.spill.  All we care about is
   that it wants a 16 byte aligned slot.  */
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ
{
  UINT64 x[2] __attribute__((aligned(16)));
} fpreg;


/* The stack layout given to ffi_call_unix and ffi_closure_unix_inner.  */

ಸಂರಚನಾ ia64_args
{
  fpreg fp_regs[8];	/* Contents of 8 fp arg registers.  */
  UINT64 gp_regs[8];	/* Contents of 8 gp arg registers.  */
  UINT64 other_args[];	/* Arguments passed on stack, variable size.  */
};


/* Adjust ADDR, a pointer to an 8 byte slot, to point to the low LEN bytes.  */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ *
endian_adjust (ಶೂನ್ಯ *addr, size_t len)
{
#ifdef __BIG_ENDIAN__
  ಪ್ರತ್ಯಾಗಮ addr + (8 - len);
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ addr;
#endif
}

/* Store VALUE to ADDR in the current cpu implementation's fp spill format.
   This is a macro instead of a function, so that it works for all 3 floating
   point types without type conversions.  Type conversion to long double breaks
   the denorm support.  */

#ifdef __hpux
#define stf_spill(addr, ಮೌಲ್ಯ)
#ಅನ್ಯಥಾ
#define stf_spill(addr, ಮೌಲ್ಯ)	\
  __asm__ ("stf.spill %0 = %1%P0" : "=m" (*addr) : "f"(ಮೌಲ್ಯ));
#endif

/* Load a value from ADDR, which is in the current cpu implementation's
   fp spill format.  As above, this must also be a macro.  */

#ifdef __hpux
#define ldf_fill(result, addr)
#ಅನ್ಯಥಾ
#define ldf_fill(result, addr)	\
  __asm__ ("ldf.fill %0 = %1%P1" : "=f"(result) : "m"(*addr));
#endif

/* Return the size of the C type associated with with TYPE.  Which will
   be one of the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ size_t
hfa_type_size (ಪೂರ್ಣಾಙ್ಕ type)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_FLOAT:
      ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(ಭಿನ್ನ);
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_DOUBLE:
      ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(ದ್ವಿಗುಣ);
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_LDOUBLE:
      ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(__float80);
    ಸ್ವಭಾವ:
      abort ();
    }
}

/* Load from ADDR a value indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ ಶೂನ್ಯ
hfa_type_load (fpreg *fpaddr, ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *addr)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_FLOAT:
      stf_spill (fpaddr, *(ಭಿನ್ನ *) addr);
      ಪ್ರತ್ಯಾಗಮ;
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_DOUBLE:
      stf_spill (fpaddr, *(ದ್ವಿಗುಣ *) addr);
      ಪ್ರತ್ಯಾಗಮ;
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_LDOUBLE:
      stf_spill (fpaddr, *(__float80 *) addr);
      ಪ್ರತ್ಯಾಗಮ;
    ಸ್ವಭಾವ:
      abort ();
    }
}

/* Load VALUE into ADDR as indicated by TYPE.  Which will be one of
   the FFI_IA64_TYPE_HFA_* values.  */

ಸ್ಥಿರ ಶೂನ್ಯ
hfa_type_store (ಪೂರ್ಣಾಙ್ಕ type, ಶೂನ್ಯ *addr, fpreg *fpaddr)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_FLOAT:
      {
	ಭಿನ್ನ result;
	ldf_fill (result, fpaddr);
	*(ಭಿನ್ನ *) addr = result;
	ಭಙ್ಗ;
      }
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_DOUBLE:
      {
	ದ್ವಿಗುಣ result;
	ldf_fill (result, fpaddr);
	*(ದ್ವಿಗುಣ *) addr = result;
	ಭಙ್ಗ;
      }
    ಪಕ್ಷ FFI_IA64_TYPE_HFA_LDOUBLE:
      {
	__float80 result;
	ldf_fill (result, fpaddr);
	*(__float80 *) addr = result;
	ಭಙ್ಗ;
      }
    ಸ್ವಭಾವ:
      abort ();
    }
}

/* Is TYPE a struct containing floats, doubles, or extended doubles,
   all of the same fp type?  If so, return the element type.  Return
   FFI_TYPE_VOID if not.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
hfa_element_type (ffi_type *type, ಪೂರ್ಣಾಙ್ಕ nested)
{
  ಪೂರ್ಣಾಙ್ಕ element = FFI_TYPE_VOID;

  ವಿಕಲ್ಪ (type->type)
    {
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      /* We want to return VOID for raw floating-point types, but the
	 synthetic HFA type if we're nested within an aggregate.  */
      ಯದಿ (nested)
	element = FFI_IA64_TYPE_HFA_FLOAT;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      /* Similarly.  */
      ಯದಿ (nested)
	element = FFI_IA64_TYPE_HFA_DOUBLE;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      /* Similarly, except that that HFA is true for double extended,
	 but not quad precision.  Both have sizeof == 16, so tell the
	 difference based on the precision.  */
      ಯದಿ (LDBL_MANT_DIG == 64 && nested)
	element = FFI_IA64_TYPE_HFA_LDOUBLE;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      {
	ffi_type **ptr = &type->elements[0];

	ಪ್ರತಿ (ptr = &type->elements[0]; *ptr ; ptr++)
	  {
	    ಪೂರ್ಣಾಙ್ಕ sub_element = hfa_element_type (*ptr, 1);
	    ಯದಿ (sub_element == FFI_TYPE_VOID)
	      ಪ್ರತ್ಯಾಗಮ FFI_TYPE_VOID;

	    ಯದಿ (element == FFI_TYPE_VOID)
	      element = sub_element;
	    ಅನ್ಯಥಾ ಯದಿ (element != sub_element)
	      ಪ್ರತ್ಯಾಗಮ FFI_TYPE_VOID;
	  }
      }
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ FFI_TYPE_VOID;
    }

  ಪ್ರತ್ಯಾಗಮ element;
}


/* Perform machine dependent cif processing. */

ಸ್ಥಿರ ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ಪೂರ್ಣಾಙ್ಕ flags;

  /* Adjust cif->bytes to include space for the bits of the ia64_args frame
     that precedes the integer register portion.  The estimate that the
     generic bits did for the argument space required is good enough for the
     integer component.  */
  cif->bytes += offsetof(ಸಂರಚನಾ ia64_args, gp_regs[0]);
  ಯದಿ (cif->bytes < ಪರಿಮಾಣ(ಸಂರಚನಾ ia64_args))
    cif->bytes = ಪರಿಮಾಣ(ಸಂರಚನಾ ia64_args);

  /* Set the return type flag. */
  flags = cif->rtype->type;
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
      /* Leave FFI_TYPE_LONGDOUBLE as meaning double extended precision,
	 and encode quad precision as a two-word integer structure.  */
      ಯದಿ (LDBL_MANT_DIG != 64)
	flags = FFI_IA64_TYPE_SMALL_STRUCT | (16 << 8);
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      {
        size_t size = cif->rtype->size;
  	ಪೂರ್ಣಾಙ್ಕ hfa_type = hfa_element_type (cif->rtype, 0);

	ಯದಿ (hfa_type != FFI_TYPE_VOID)
	  {
	    size_t nelts = size / hfa_type_size (hfa_type);
	    ಯದಿ (nelts <= 8)
	      flags = hfa_type | (size << 8);
	  }
	ಅನ್ಯಥಾ
	  {
	    ಯದಿ (size <= 32)
	      flags = FFI_IA64_TYPE_SMALL_STRUCT | (size << 8);
	  }
      }
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      ಭಙ್ಗ;
    }
  cif->flags = flags;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep_core(cif);
}

ffi_status
ffi_prep_cif_machdep_var(ffi_cif *cif,
			 unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
			 unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs MAYBE_UNUSED)
{
  cif->nfixedargs = nfixedargs;
  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep_core(cif);
}

ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ffi_call_unix (ಸಂರಚನಾ ia64_args *, PTR64, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), UINT64);

ಶೂನ್ಯ
ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಸಂರಚನಾ ia64_args *ರಾಶಿ;
  long i, avn, gpcount, fpcount;
  ffi_type **p_arg;

  FFI_ASSERT (cif->abi == FFI_UNIX);

  /* If we have no spot for a return value, make one.  */
  ಯದಿ (rvalue == ರಿಕ್ತ && cif->rtype->type != FFI_TYPE_VOID)
    rvalue = alloca (cif->rtype->size);
    
  /* Allocate the stack frame.  */
  ರಾಶಿ = alloca (cif->bytes);

  gpcount = fpcount = 0;
  avn = cif->nargs;
  ಪ್ರತಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ವಿಕಲ್ಪ ((*p_arg)->type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT8 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT8 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT16 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT16 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT32:
	  ರಾಶಿ->gp_regs[gpcount++] = *(SINT32 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT32 *)avalue[i];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	  ರಾಶಿ->gp_regs[gpcount++] = *(UINT64 *)avalue[i];
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_POINTER:
	  ರಾಶಿ->gp_regs[gpcount++] = (UINT64)(PTR64) *(ಶೂನ್ಯ **)avalue[i];
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  ಯದಿ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(ಭಿನ್ನ *)avalue[i]);
	  {
	    UINT32 tmp;
	    memcpy (&tmp, avalue[i], ಪರಿಮಾಣ (UINT32));
	    ರಾಶಿ->gp_regs[gpcount++] = tmp;
	  }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ (gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(ದ್ವಿಗುಣ *)avalue[i]);
	  memcpy (&ರಾಶಿ->gp_regs[gpcount++], avalue[i], ಪರಿಮಾಣ (UINT64));
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  ಯದಿ (gpcount & 1)
	    gpcount++;
	  ಯದಿ (LDBL_MANT_DIG == 64 && gpcount < 8 && fpcount < 8)
	    stf_spill (&ರಾಶಿ->fp_regs[fpcount++], *(__float80 *)avalue[i]);
	  memcpy (&ರಾಶಿ->gp_regs[gpcount], avalue[i], 16);
	  gpcount += 2;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    ಪೂರ್ಣಾಙ್ಕ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ಯದಿ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ಯದಿ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;

		ಯಾವತ್ (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_load (&ರಾಶಿ->fp_regs[fpcount], hfa_type,
				   avalue[i] + offset);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }
	      }

	    memcpy (&ರಾಶಿ->gp_regs[gpcount], avalue[i], size);
	    gpcount += (size + 7) / 8;
	  }
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
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

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_unix ();

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  /* The layout of a function descriptor.  A C function pointer really 
     points to one of these.  */
  ಸಂರಚನಾ ia64_fd
  {
    UINT64 code_pointer;
    UINT64 gp;
  };

  ಸಂರಚನಾ ffi_ia64_trampoline_struct
  {
    UINT64 code_pointer;	/* Pointer to ffi_closure_unix.  */
    UINT64 fake_gp;		/* Pointer to closure, installed as gp.  */
    UINT64 real_gp;		/* Real gp value.  */
  };

  ಸಂರಚನಾ ffi_ia64_trampoline_struct *tramp;
  ಸಂರಚನಾ ia64_fd *fd;

  ಯದಿ (cif->abi != FFI_UNIX)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  tramp = (ಸಂರಚನಾ ffi_ia64_trampoline_struct *)closure->tramp;
  fd = (ಸಂರಚನಾ ia64_fd *)(ಶೂನ್ಯ *)ffi_closure_unix;

  tramp->code_pointer = fd->code_pointer;
  tramp->real_gp = fd->gp;
  tramp->fake_gp = (UINT64)(PTR64)codeloc;
  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}


UINT64
ffi_closure_unix_inner (ffi_closure *closure, ಸಂರಚನಾ ia64_args *ರಾಶಿ,
			ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ *r8)
{
  ffi_cif *cif;
  ಶೂನ್ಯ **avalue;
  ffi_type **p_arg;
  long i, avn, gpcount, fpcount, nfixedargs;

  cif = closure->cif;
  avn = cif->nargs;
  nfixedargs = cif->nfixedargs;
  avalue = alloca (avn * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  /* If the structure return value is passed in memory get that location
     from r8 so as to pass the value directly back to the caller.  */
  ಯದಿ (cif->flags == FFI_TYPE_STRUCT)
    rvalue = r8;

  gpcount = fpcount = 0;
  ಪ್ರತಿ (i = 0, p_arg = cif->arg_types; i < avn; i++, p_arg++)
    {
      ಪೂರ್ಣಾಙ್ಕ named = i < nfixedargs;
      ವಿಕಲ್ಪ ((*p_arg)->type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 1);
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 2);
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], 4);
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	  avalue[i] = &ರಾಶಿ->gp_regs[gpcount++];
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount++], ಪರಿಮಾಣ(ಶೂನ್ಯ*));
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  ಯದಿ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      ಭಿನ್ನ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(ಭಿನ್ನ *)addr = result;
	    }
	  ಅನ್ಯಥಾ
	    avalue[i] = endian_adjust(&ರಾಶಿ->gp_regs[gpcount], 4);
	  gpcount++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ (named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      ದ್ವಿಗುಣ result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(ದ್ವಿಗುಣ *)addr = result;
	    }
	  ಅನ್ಯಥಾ
	    avalue[i] = &ರಾಶಿ->gp_regs[gpcount];
	  gpcount++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  ಯದಿ (gpcount & 1)
	    gpcount++;
	  ಯದಿ (LDBL_MANT_DIG == 64 && named && gpcount < 8 && fpcount < 8)
	    {
	      fpreg *addr = &ರಾಶಿ->fp_regs[fpcount++];
	      __float80 result;
	      avalue[i] = addr;
	      ldf_fill (result, addr);
	      *(__float80 *)addr = result;
	    }
	  ಅನ್ಯಥಾ
	    avalue[i] = &ರಾಶಿ->gp_regs[gpcount];
	  gpcount += 2;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  {
	    size_t size = (*p_arg)->size;
	    size_t align = (*p_arg)->alignment;
	    ಪೂರ್ಣಾಙ್ಕ hfa_type = hfa_element_type (*p_arg, 0);

	    FFI_ASSERT (align <= 16);
	    ಯದಿ (align == 16 && (gpcount & 1))
	      gpcount++;

	    ಯದಿ (hfa_type != FFI_TYPE_VOID)
	      {
		size_t hfa_size = hfa_type_size (hfa_type);
		size_t offset = 0;
		size_t gp_offset = gpcount * 8;
		ಶೂನ್ಯ *addr = alloca (size);

		avalue[i] = addr;

		ಯಾವತ್ (fpcount < 8
		       && offset < size
		       && gp_offset < 8 * 8)
		  {
		    hfa_type_store (hfa_type, addr + offset,
				    &ರಾಶಿ->fp_regs[fpcount]);
		    offset += hfa_size;
		    gp_offset += hfa_size;
		    fpcount += 1;
		  }

		ಯದಿ (offset < size)
		  memcpy (addr + offset, (ವರ್ಣ *)ರಾಶಿ->gp_regs + gp_offset,
			  size - offset);
	      }
	    ಅನ್ಯಥಾ
	      avalue[i] = &ರಾಶಿ->gp_regs[gpcount];

	    gpcount += (size + 7) / 8;
	  }
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  abort ();
	}
    }

  closure->fun (cif, rvalue, avalue, closure->user_data);

  ಪ್ರತ್ಯಾಗಮ cif->flags;
}

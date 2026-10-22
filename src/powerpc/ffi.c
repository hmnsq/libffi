/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2013 IBM
           Copyright (C) 2011, 2026 Anthony Green
           Copyright (C) 2011 Kyle Moffett
           Copyright (C) 2008 Red Hat, Inc
           Copyright (C) 2007, 2008 Free Software Foundation, Inc
	   Copyright (c) 1998 Geoffrey Keating

   PowerPC Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ಅನ್ತರ್ಭಾವ "ffi.h"
#ಅನ್ತರ್ಭಾವ "ffi_common.h"
#ಅನ್ತರ್ಭಾವ "ffi_powerpc.h"
#ಅನ್ತರ್ಭಾವ "internal.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>

#ಆದರೆ HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
ಶೂನ್ಯ FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  ಇಲ್ಲದಿದ್ದರೆ
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#ಆದರೆ_ಅಂತ್ಯ

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ಹಿಂತಿರುಗಿ ffi_prep_cif_linux64 (cif);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ffi_prep_cif_sysv (cif);
#ಆದರೆ_ಅಂತ್ಯ
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs MAYBE_ಬಳಸದ,
			  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs MAYBE_ಬಳಸದ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ಹಿಂತಿರುಗಿ ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ffi_prep_cif_sysv (cif);
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif,
	      ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ),
	      ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue,
	      ಶೂನ್ಯ *ಆವರಣ)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_ಬಫರ್[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  ಆದರೆ ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_ಬಫರ್;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->ಗಾತ್ರ_2);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, ಆವರಣ,
		    -(ದೀರ್ಘ) cif->ಬೈಟುಗಳು);
#ಇಲ್ಲದಿದ್ದರೆ
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, ಆವರಣ, -cif->ಬೈಟುಗಳು);
#ಆದರೆ_ಅಂತ್ಯ

  /* Check for a bounce-buffered return value */
  ಆದರೆ (rvalue && ecif.rvalue == smst_ಬಫರ್)
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ rsize = cif->rtype->ಗಾತ್ರ_2;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      ಆದರೆ (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->ಗಾತ್ರ_2;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#ಇಲ್ಲದಿದ್ದರೆ
	  size_t off = 0;
#ಆದರೆ_ಅಂತ್ಯ
	  ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) rvalue,         (ಅಕ್ಷರ *) smst_ಬಫರ್ + off, hsize);
	  ಸ್ಮರಣೆ_ನಕಲು ((ಅಕ್ಷರ *) rvalue + hsize, (ಅಕ್ಷರ *) smst_ಬಫರ್ + 8 + off, hsize);
	}
      ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      ಆದರೆ (rsize <= 4)
	ಸ್ಮರಣೆ_ನಕಲು (rvalue, (ಅಕ್ಷರ *) smst_ಬಫರ್ + 4 - rsize, rsize);
      ಇಲ್ಲದಿದ್ದರೆ
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	ಆದರೆ (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  ಸ್ಮರಣೆ_ನಕಲು (rvalue, (ಅಕ್ಷರ *) smst_ಬಫರ್ + 8 - rsize, rsize);
	ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	  ಸ್ಮರಣೆ_ನಕಲು (rvalue, smst_ಬಫರ್, rsize);
    }
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ಹಿಂತಿರುಗಿ ffi_prep_ಆವರಣ_loc_linux64 (ಆವರಣ, cif, fun, user_ದತ್ತಾಂಶ, codeloc);
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ ffi_prep_ಆವರಣ_loc_sysv (ಆವರಣ, cif, fun, user_ದತ್ತಾಂಶ, codeloc);
#ಆದರೆ_ಅಂತ್ಯ
}

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ,
		     ffi_cif *cif,
		     ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *))
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ POWERPC64
  ಆವರಣ->tramp = ffi_go_ಆವರಣ_linux64;
#ಇಲ್ಲದಿದ್ದರೆ
  ಆವರಣ->tramp = ffi_go_ಆವರಣ_sysv;
#ಆದರೆ_ಅಂತ್ಯ
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_EXEC_STATIC_TRAMP
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_ಗಾತ್ರ, size_t *map_ಗಾತ್ರ)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
  *tramp_ಗಾತ್ರ = PPC_TRAMP_ಗಾತ್ರ;
  *map_ಗಾತ್ರ = PPC_TRAMP_MAP_ಗಾತ್ರ;
#ಆದರೆ defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  ಹಿಂತಿರುಗಿ *(ಶೂನ್ಯ **)trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ &trampoline_ಸಂಕೇತ_ಕೋಷ್ಟಕ;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ

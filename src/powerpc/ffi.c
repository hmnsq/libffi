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

#ਅਨ੍ਤਰ੍ਭਾਵ "ffi.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_common.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_powerpc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

#ਜੇ HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
ਖਾਲੀ FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  ਨਹੀਂਤਾਂ
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#ਜੇ_ਅੰਤ

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ਵਾਪਸ ffi_prep_cif_linux64 (cif);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ffi_prep_cif_sysv (cif);
#ਜੇ_ਅੰਤ
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs MAYBE_ਅਣਵਰਤਿਆ,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs MAYBE_ਅਣਵਰਤਿਆ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ਵਾਪਸ ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ffi_prep_cif_sysv (cif);
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif,
	      ਖਾਲੀ (*fn) (ਖਾਲੀ),
	      ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue,
	      ਖਾਲੀ *ਘੇਰਾ)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_ਬਫਰ[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  ਜੇ ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_ਬਫਰ;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  ਨਹੀਂਤਾਂ ਜੇ (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->ਮਾਪ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, ਘੇਰਾ,
		    -(ਲੰਮਾ) cif->ਬਾਈਟਾਂ);
#ਨਹੀਂਤਾਂ
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, ਘੇਰਾ, -cif->ਬਾਈਟਾਂ);
#ਜੇ_ਅੰਤ

  /* Check for a bounce-buffered return value */
  ਜੇ (rvalue && ecif.rvalue == smst_ਬਫਰ)
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ rsize = cif->rtype->ਮਾਪ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      ਜੇ (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->ਮਾਪ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#ਨਹੀਂਤਾਂ
	  size_t off = 0;
#ਜੇ_ਅੰਤ
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) rvalue,         (ਅੱਖਰ *) smst_ਬਫਰ + off, hsize);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ ((ਅੱਖਰ *) rvalue + hsize, (ਅੱਖਰ *) smst_ਬਫਰ + 8 + off, hsize);
	}
      ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      ਜੇ (rsize <= 4)
	ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, (ਅੱਖਰ *) smst_ਬਫਰ + 4 - rsize, rsize);
      ਨਹੀਂਤਾਂ
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	ਜੇ (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, (ਅੱਖਰ *) smst_ਬਫਰ + 8 - rsize, rsize);
	ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, smst_ਬਫਰ, rsize);
    }
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn) (ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue,
	     ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ਵਾਪਸ ffi_prep_ਘੇਰਾ_loc_linux64 (ਘੇਰਾ, cif, fun, user_ਅੰਕੜੇ, codeloc);
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ ffi_prep_ਘੇਰਾ_loc_sysv (ਘੇਰਾ, cif, fun, user_ਅੰਕੜੇ, codeloc);
#ਜੇ_ਅੰਤ
}

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ,
		     ffi_cif *cif,
		     ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *))
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ POWERPC64
  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_linux64;
#ਨਹੀਂਤਾਂ
  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_sysv;
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_EXEC_STATIC_TRAMP
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;
  *tramp_ਮਾਪ = PPC_TRAMP_ਮਾਪ;
  *map_ਮਾਪ = PPC_TRAMP_MAP_ਮਾਪ;
#ਜੇ defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  ਵਾਪਸ *(ਖਾਲੀ **)trampoline_ਕੋਡ_ਸਾਰਣੀ;
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

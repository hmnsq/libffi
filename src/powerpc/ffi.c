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

#அந்தர்பாவ "ffi.h"
#அந்தர்பாவ "ffi_common.h"
#அந்தர்பாவ "ffi_powerpc.h"
#அந்தர்பாவ "internal.h"
#அந்தர்பாவ <tramp.h>

#எனில் HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
வெறுமை FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  இல்லையெனில்
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#endif

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#ifdef POWERPC64
  திரும்பு ffi_prep_cif_linux64 (cif);
#இல்லையெனில்
  திரும்பு ffi_prep_cif_sysv (cif);
#endif
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  unsigned முழு nfixedargs MAYBE_UNUSED,
			  unsigned முழு ntotalargs MAYBE_UNUSED)
{
#ifdef POWERPC64
  திரும்பு ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#இல்லையெனில்
  திரும்பு ffi_prep_cif_sysv (cif);
#endif
}

நிலையான வெறுமை
ffi_call_int (ffi_cif *cif,
	      வெறுமை (*fn) (வெறுமை),
	      வெறுமை *rvalue,
	      வெறுமை **avalue,
	      வெறுமை *closure)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_buffer[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  எனில் ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_buffer;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  இல்லையெனில் எனில் (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->size);

#ifdef POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, closure,
		    -(long) cif->bytes);
#இல்லையெனில்
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, closure, -cif->bytes);
#endif

  /* Check for a bounce-buffered return value */
  எனில் (rvalue && ecif.rvalue == smst_buffer)
    {
      unsigned முழு rsize = cif->rtype->size;
#ifdef FFI_TARGET_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      எனில் (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->size;
#ifndef __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#இல்லையெனில்
	  size_t off = 0;
#endif
	  memcpy ((எழுத்து *) rvalue,         (எழுத்து *) smst_buffer + off, hsize);
	  memcpy ((எழுத்து *) rvalue + hsize, (எழுத்து *) smst_buffer + 8 + off, hsize);
	}
      இல்லையெனில்
#endif
#ifndef __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      எனில் (rsize <= 4)
	memcpy (rvalue, (எழுத்து *) smst_buffer + 4 - rsize, rsize);
      இல்லையெனில்
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	எனில் (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  memcpy (rvalue, (எழுத்து *) smst_buffer + 8 - rsize, rsize);
	இல்லையெனில்
#endif
	  memcpy (rvalue, smst_buffer, rsize);
    }
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn) (வெறுமை), வெறுமை *rvalue, வெறுமை **avalue,
	     வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
#ifdef POWERPC64
  திரும்பு ffi_prep_closure_loc_linux64 (closure, cif, fun, user_data, codeloc);
#இல்லையெனில்
  திரும்பு ffi_prep_closure_loc_sysv (closure, cif, fun, user_data, codeloc);
#endif
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure,
		     ffi_cif *cif,
		     வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *))
{
#ifdef POWERPC64
  closure->tramp = ffi_go_closure_linux64;
#இல்லையெனில்
  closure->tramp = ffi_go_closure_sysv;
#endif
  closure->cif = cif;
  closure->fun = fun;
  திரும்பு FFI_OK;
}

#ifdef FFI_EXEC_STATIC_TRAMP
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;
  *tramp_size = PPC_TRAMP_SIZE;
  *map_size = PPC_TRAMP_MAP_SIZE;
#எனில் defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  திரும்பு *(வெறுமை **)trampoline_code_table;
#இல்லையெனில்
  திரும்பு &trampoline_code_table;
#endif
}
#endif

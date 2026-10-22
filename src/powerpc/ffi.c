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

#अन्तर्भाव "ffi.h"
#अन्तर्भाव "ffi_common.h"
#अन्तर्भाव "ffi_powerpc.h"
#अन्तर्भाव "internal.h"
#अन्तर्भाव <tramp.h>

#यदि HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
शून्य FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# यदि FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  अन्यथा
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#यदि_अन्तः

/* Perform machine dependent cif processing */
ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#यदि_परिभाषितम् POWERPC64
  प्रत्यागम ffi_prep_cif_linux64 (cif);
#अन्यथा
  प्रत्यागम ffi_prep_cif_sysv (cif);
#यदि_अन्तः
}

ffi_स्थितिः FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  अचिह्नित पूर्णाङ्क nfixedargs MAYBE_अप्रयुक्तम्,
			  अचिह्नित पूर्णाङ्क ntotalargs MAYBE_अप्रयुक्तम्)
{
#यदि_परिभाषितम् POWERPC64
  प्रत्यागम ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#अन्यथा
  प्रत्यागम ffi_prep_cif_sysv (cif);
#यदि_अन्तः
}

स्थिर शून्य
ffi_call_int (ffi_cif *cif,
	      शून्य (*fn) (शून्य),
	      शून्य *rvalue,
	      शून्य **avalue,
	      शून्य *संवृतिः)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_बफर[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  यदि ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_बफर;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  अन्यथा यदि (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->आकार);

#यदि_परिभाषितम् POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, संवृतिः,
		    -(दीर्घ) cif->अष्टकानि);
#अन्यथा
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, संवृतिः, -cif->अष्टकानि);
#यदि_अन्तः

  /* Check for a bounce-buffered return value */
  यदि (rvalue && ecif.rvalue == smst_बफर)
    {
      अचिह्नित पूर्णाङ्क rsize = cif->rtype->आकार;
#यदि_परिभाषितम् FFI_TARGET_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      यदि (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->आकार;
#यदि_अपरिभाषितम् __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#अन्यथा
	  size_t off = 0;
#यदि_अन्तः
	  स्मृतिं_प्रतिलिख ((वर्ण *) rvalue,         (वर्ण *) smst_बफर + off, hsize);
	  स्मृतिं_प्रतिलिख ((वर्ण *) rvalue + hsize, (वर्ण *) smst_बफर + 8 + off, hsize);
	}
      अन्यथा
#यदि_अन्तः
#यदि_अपरिभाषितम् __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      यदि (rsize <= 4)
	स्मृतिं_प्रतिलिख (rvalue, (वर्ण *) smst_बफर + 4 - rsize, rsize);
      अन्यथा
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	यदि (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  स्मृतिं_प्रतिलिख (rvalue, (वर्ण *) smst_बफर + 8 - rsize, rsize);
	अन्यथा
#यदि_अन्तः
	  स्मृतिं_प्रतिलिख (rvalue, smst_बफर, rsize);
    }
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn) (शून्य), शून्य *rvalue, शून्य **avalue,
	     शून्य *संवृतिः)
{
  ffi_call_int (cif, fn, rvalue, avalue, संवृतिः);
}

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः,
		      ffi_cif *cif,
		      शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
#यदि_परिभाषितम् POWERPC64
  प्रत्यागम ffi_prep_संवृतिः_loc_linux64 (संवृतिः, cif, fun, user_दत्त, codeloc);
#अन्यथा
  प्रत्यागम ffi_prep_संवृतिः_loc_sysv (संवृतिः, cif, fun, user_दत्त, codeloc);
#यदि_अन्तः
}

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः *संवृतिः,
		     ffi_cif *cif,
		     शून्य (*fun) (ffi_cif *, शून्य *, शून्य **, शून्य *))
{
#यदि_परिभाषितम् POWERPC64
  संवृतिः->tramp = ffi_go_संवृतिः_linux64;
#अन्यथा
  संवृतिः->tramp = ffi_go_संवृतिः_sysv;
#यदि_अन्तः
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  प्रत्यागम FFI_OK;
}

#यदि_परिभाषितम् FFI_EXEC_STATIC_TRAMP
शून्य *
ffi_tramp_arch (size_t *tramp_आकार, size_t *map_आकार)
{
  बाह्य शून्य *trampoline_संहिता_सारणी;
  *tramp_आकार = PPC_TRAMP_आकार;
  *map_आकार = PPC_TRAMP_MAP_आकार;
#यदि defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  प्रत्यागम *(शून्य **)trampoline_संहिता_सारणी;
#अन्यथा
  प्रत्यागम &trampoline_संहिता_सारणी;
#यदि_अन्तः
}
#यदि_अन्तः

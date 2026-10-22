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

#समावेश "ffi.h"
#समावेश "ffi_common.h"
#समावेश "ffi_powerpc.h"
#समावेश "internal.h"
#समावेश <tramp.h>

#अगर HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
व्योम FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# अगर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  अन्यथा
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#पूर्ण_अगर

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#अगर_घोषित POWERPC64
  वापस ffi_prep_cif_linux64 (cif);
#अन्यथा
  वापस ffi_prep_cif_sysv (cif);
#पूर्ण_अगर
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  अचिन्हित पूर्णांक nfixedargs MAYBE_UNUSED,
			  अचिन्हित पूर्णांक ntotalargs MAYBE_UNUSED)
{
#अगर_घोषित POWERPC64
  वापस ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#अन्यथा
  वापस ffi_prep_cif_sysv (cif);
#पूर्ण_अगर
}

अटल व्योम
ffi_call_int (ffi_cif *cif,
	      व्योम (*fn) (व्योम),
	      व्योम *rvalue,
	      व्योम **avalue,
	      व्योम *closure)
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
  अगर ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_बफर;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  अन्यथा अगर (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->आकार);

#अगर_घोषित POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, closure,
		    -(दीर्घ) cif->bytes);
#अन्यथा
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, closure, -cif->bytes);
#पूर्ण_अगर

  /* Check for a bounce-buffered return value */
  अगर (rvalue && ecif.rvalue == smst_बफर)
    {
      अचिन्हित पूर्णांक rsize = cif->rtype->आकार;
#अगर_घोषित FFI_TARGET_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      अगर (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  माप_प्रकार hsize = cif->rtype->elements[0]->आकार;
#अगर_अघोषित __LITTLE_ENDIAN__
	  माप_प्रकार off = 8 - hsize;
#अन्यथा
	  माप_प्रकार off = 0;
#पूर्ण_अगर
	  स_नकल ((अक्षर *) rvalue,         (अक्षर *) smst_बफर + off, hsize);
	  स_नकल ((अक्षर *) rvalue + hsize, (अक्षर *) smst_बफर + 8 + off, hsize);
	}
      अन्यथा
#पूर्ण_अगर
#अगर_अघोषित __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      अगर (rsize <= 4)
	स_नकल (rvalue, (अक्षर *) smst_बफर + 4 - rsize, rsize);
      अन्यथा
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	अगर (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  स_नकल (rvalue, (अक्षर *) smst_बफर + 8 - rsize, rsize);
	अन्यथा
#पूर्ण_अगर
	  स_नकल (rvalue, smst_बफर, rsize);
    }
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, शून्य);
}

व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn) (व्योम), व्योम *rvalue, व्योम **avalue,
	     व्योम *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
#अगर_घोषित POWERPC64
  वापस ffi_prep_closure_loc_linux64 (closure, cif, fun, user_तथ्य, codeloc);
#अन्यथा
  वापस ffi_prep_closure_loc_sysv (closure, cif, fun, user_तथ्य, codeloc);
#पूर्ण_अगर
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure,
		     ffi_cif *cif,
		     व्योम (*fun) (ffi_cif *, व्योम *, व्योम **, व्योम *))
{
#अगर_घोषित POWERPC64
  closure->tramp = ffi_go_closure_linux64;
#अन्यथा
  closure->tramp = ffi_go_closure_sysv;
#पूर्ण_अगर
  closure->cif = cif;
  closure->fun = fun;
  वापस FFI_OK;
}

#अगर_घोषित FFI_EXEC_STATIC_TRAMP
व्योम *
ffi_tramp_arch (माप_प्रकार *tramp_आकार, माप_प्रकार *map_आकार)
{
  बाह्य व्योम *trampoline_code_table;
  *tramp_आकार = PPC_TRAMP_आकार;
  *map_आकार = PPC_TRAMP_MAP_आकार;
#अगर defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  वापस *(व्योम **)trampoline_code_table;
#अन्यथा
  वापस &trampoline_code_table;
#पूर्ण_अगर
}
#पूर्ण_अगर

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

#అన్తర్భావ "ffi.h"
#అన్తర్భావ "ffi_common.h"
#అన్తర్భావ "ffi_powerpc.h"
#అన్తర్భావ "internal.h"
#అన్తర్భావ <tramp.h>

#ఒకవేళ HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
శూన్య FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  లేకపోతే
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#అయితే_ముగింపు

/* Perform machine dependent cif processing */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#నిర్వచితమైతే POWERPC64
  తిరిగి ffi_prep_cif_linux64 (cif);
#లేకపోతే
  తిరిగి ffi_prep_cif_sysv (cif);
#అయితే_ముగింపు
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  గుర్తులేని పూర్ణాంకం nfixedargs MAYBE_వాడని,
			  గుర్తులేని పూర్ణాంకం ntotalargs MAYBE_వాడని)
{
#నిర్వచితమైతే POWERPC64
  తిరిగి ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#లేకపోతే
  తిరిగి ffi_prep_cif_sysv (cif);
#అయితే_ముగింపు
}

స్థిర శూన్య
ffi_call_int (ffi_cif *cif,
	      శూన్య (*fn) (శూన్య),
	      శూన్య *rvalue,
	      శూన్య **avalue,
	      శూన్య *సంవృతం)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_బఫరు[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  ఒకవేళ ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_బఫరు;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  లేకపోతే ఒకవేళ (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->కొలత);

#నిర్వచితమైతే POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, సంవృతం,
		    -(దీర్ఘ) cif->బైట్లు);
#లేకపోతే
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, సంవృతం, -cif->బైట్లు);
#అయితే_ముగింపు

  /* Check for a bounce-buffered return value */
  ఒకవేళ (rvalue && ecif.rvalue == smst_బఫరు)
    {
      గుర్తులేని పూర్ణాంకం rsize = cif->rtype->కొలత;
#నిర్వచితమైతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      ఒకవేళ (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->కొలత;
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#లేకపోతే
	  size_t off = 0;
#అయితే_ముగింపు
	  జ్ఞాపకం_నకలు ((అక్షరం *) rvalue,         (అక్షరం *) smst_బఫరు + off, hsize);
	  జ్ఞాపకం_నకలు ((అక్షరం *) rvalue + hsize, (అక్షరం *) smst_బఫరు + 8 + off, hsize);
	}
      లేకపోతే
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      ఒకవేళ (rsize <= 4)
	జ్ఞాపకం_నకలు (rvalue, (అక్షరం *) smst_బఫరు + 4 - rsize, rsize);
      లేకపోతే
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	ఒకవేళ (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  జ్ఞాపకం_నకలు (rvalue, (అక్షరం *) smst_బఫరు + 8 - rsize, rsize);
	లేకపోతే
#అయితే_ముగింపు
	  జ్ఞాపకం_నకలు (rvalue, smst_బఫరు, rsize);
    }
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn) (శూన్య), శూన్య *rvalue, శూన్య **avalue,
	     శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
		      ffi_cif *cif,
		      శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
#నిర్వచితమైతే POWERPC64
  తిరిగి ffi_prep_సంవృతం_loc_linux64 (సంవృతం, cif, fun, user_దత్తాంశం, codeloc);
#లేకపోతే
  తిరిగి ffi_prep_సంవృతం_loc_sysv (సంవృతం, cif, fun, user_దత్తాంశం, codeloc);
#అయితే_ముగింపు
}

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం,
		     ffi_cif *cif,
		     శూన్య (*fun) (ffi_cif *, శూన్య *, శూన్య **, శూన్య *))
{
#నిర్వచితమైతే POWERPC64
  సంవృతం->tramp = ffi_go_సంవృతం_linux64;
#లేకపోతే
  సంవృతం->tramp = ffi_go_సంవృతం_sysv;
#అయితే_ముగింపు
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  తిరిగి FFI_OK;
}

#నిర్వచితమైతే FFI_EXEC_STATIC_TRAMP
శూన్య *
ffi_tramp_arch (size_t *tramp_కొలత, size_t *map_కొలత)
{
  బాహ్య శూన్య *trampoline_సంకేతం_పట్టిక;
  *tramp_కొలత = PPC_TRAMP_కొలత;
  *map_కొలత = PPC_TRAMP_MAP_కొలత;
#ఒకవేళ defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  తిరిగి *(శూన్య **)trampoline_సంకేతం_పట్టిక;
#లేకపోతే
  తిరిగి &trampoline_సంకేతం_పట్టిక;
#అయితే_ముగింపు
}
#అయితే_ముగింపు

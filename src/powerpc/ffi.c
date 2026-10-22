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

#ضمن "ffi.h"
#ضمن "ffi_common.h"
#ضمن "ffi_powerpc.h"
#ضمن "internal.h"
#ضمن <tramp.h>

#إذا HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
فراغ FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  وإلا
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#جے_انت

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#جے_تعریف_شدہ POWERPC64
  أرجع ffi_prep_cif_linux64 (cif);
#وإلا
  أرجع ffi_prep_cif_sysv (cif);
#جے_انت
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  بنانشانی صحيح nfixedargs MAYBE_بن_ورتیا,
			  بنانشانی صحيح ntotalargs MAYBE_بن_ورتیا)
{
#جے_تعریف_شدہ POWERPC64
  أرجع ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#وإلا
  أرجع ffi_prep_cif_sysv (cif);
#جے_انت
}

ساكن فراغ
ffi_call_int (ffi_cif *cif,
	      فراغ (*fn) (فراغ),
	      فراغ *rvalue,
	      فراغ **avalue,
	      فراغ *گھیرا)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_بفر[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  إذا ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_بفر;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  وإلا إذا (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->ناپ);

#جے_تعریف_شدہ POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, گھیرا,
		    -(لما) cif->بائٹاں);
#وإلا
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, گھیرا, -cif->بائٹاں);
#جے_انت

  /* Check for a bounce-buffered return value */
  إذا (rvalue && ecif.rvalue == smst_بفر)
    {
      بنانشانی صحيح rsize = cif->rtype->ناپ;
#جے_تعریف_شدہ FFI_نشانہ_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      إذا (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->ناپ;
#جے_بنا_تعریف __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#وإلا
	  size_t off = 0;
#جے_انت
	  حافظہ_نقل ((حرف *) rvalue,         (حرف *) smst_بفر + off, hsize);
	  حافظہ_نقل ((حرف *) rvalue + hsize, (حرف *) smst_بفر + 8 + off, hsize);
	}
      وإلا
#جے_انت
#جے_بنا_تعریف __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      إذا (rsize <= 4)
	حافظہ_نقل (rvalue, (حرف *) smst_بفر + 4 - rsize, rsize);
      وإلا
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	إذا (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  حافظہ_نقل (rvalue, (حرف *) smst_بفر + 8 - rsize, rsize);
	وإلا
#جے_انت
	  حافظہ_نقل (rvalue, smst_بفر, rsize);
    }
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	     فراغ *گھیرا)
{
  ffi_call_int (cif, fn, rvalue, avalue, گھیرا);
}

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا *گھیرا,
		      ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
#جے_تعریف_شدہ POWERPC64
  أرجع ffi_prep_گھیرا_loc_linux64 (گھیرا, cif, fun, user_انکڑے, codeloc);
#وإلا
  أرجع ffi_prep_گھیرا_loc_sysv (گھیرا, cif, fun, user_انکڑے, codeloc);
#جے_انت
}

ffi_حالت
ffi_prep_go_گھیرا (ffi_go_گھیرا *گھیرا,
		     ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
#جے_تعریف_شدہ POWERPC64
  گھیرا->tramp = ffi_go_گھیرا_linux64;
#وإلا
  گھیرا->tramp = ffi_go_گھیرا_sysv;
#جے_انت
  گھیرا->cif = cif;
  گھیرا->fun = fun;
  أرجع FFI_OK;
}

#جے_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_ناپ, size_t *map_ناپ)
{
  خارجي فراغ *trampoline_کوڈ_جدول;
  *tramp_ناپ = PPC_TRAMP_ناپ;
  *map_ناپ = PPC_TRAMP_MAP_ناپ;
#إذا defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  أرجع *(فراغ **)trampoline_کوڈ_جدول;
#وإلا
  أرجع &trampoline_کوڈ_جدول;
#جے_انت
}
#جے_انت

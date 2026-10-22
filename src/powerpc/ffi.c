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
#پایان_اگر

/* Perform machine dependent cif processing */
ffi_وضعیت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#اگر_تعریف_شده POWERPC64
  أرجع ffi_prep_cif_linux64 (cif);
#وإلا
  أرجع ffi_prep_cif_sysv (cif);
#پایان_اگر
}

ffi_وضعیت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  بیعلامت صحيح nfixedargs MAYBE_استفادهنشده,
			  بیعلامت صحيح ntotalargs MAYBE_استفادهنشده)
{
#اگر_تعریف_شده POWERPC64
  أرجع ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#وإلا
  أرجع ffi_prep_cif_sysv (cif);
#پایان_اگر
}

ساكن فراغ
ffi_call_int (ffi_cif *cif,
	      فراغ (*fn) (فراغ),
	      فراغ *rvalue,
	      فراغ **avalue,
	      فراغ *بستگی)
{
  /* The final SYSV ABI says that structures smaller or equal 8 bytes
     are returned in r3/r4.  A draft ABI used by linux instead returns
     them in memory.

     We bounce-buffer SYSV small struct return values so that sysv.S
     can write r3 and r4 to memory without worrying about struct size.
   
     For ELFv2 ABI, use a bounce buffer for homogeneous structs too,
     for similar reasons. This bounce buffer must be aligned to 16
     bytes for use with homogeneous structs of vectors (float128).  */
  float128 smst_میانگیر[8];
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;

  ecif.rvalue = rvalue;
  إذا ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_میانگیر;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  وإلا إذا (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->اندازه);

#اگر_تعریف_شده POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, بستگی,
		    -(دراز) cif->بایتها);
#وإلا
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, بستگی, -cif->بایتها);
#پایان_اگر

  /* Check for a bounce-buffered return value */
  إذا (rvalue && ecif.rvalue == smst_میانگیر)
    {
      بیعلامت صحيح rsize = cif->rtype->اندازه;
#اگر_تعریف_شده FFI_هدف_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      إذا (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->اندازه;
#اگر_تعریف_نشده __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#وإلا
	  size_t off = 0;
#پایان_اگر
	  کپی_حافظه ((حرف *) rvalue,         (حرف *) smst_میانگیر + off, hsize);
	  کپی_حافظه ((حرف *) rvalue + hsize, (حرف *) smst_میانگیر + 8 + off, hsize);
	}
      وإلا
#پایان_اگر
#اگر_تعریف_نشده __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      إذا (rsize <= 4)
	کپی_حافظه (rvalue, (حرف *) smst_میانگیر + 4 - rsize, rsize);
      وإلا
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	إذا (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  کپی_حافظه (rvalue, (حرف *) smst_میانگیر + 8 - rsize, rsize);
	وإلا
#پایان_اگر
	  کپی_حافظه (rvalue, smst_میانگیر, rsize);
    }
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn) (فراغ), فراغ *rvalue, فراغ **avalue,
	     فراغ *بستگی)
{
  ffi_call_int (cif, fn, rvalue, avalue, بستگی);
}

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی *بستگی,
		      ffi_cif *cif,
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_داده,
		      فراغ *codeloc)
{
#اگر_تعریف_شده POWERPC64
  أرجع ffi_prep_بستگی_loc_linux64 (بستگی, cif, fun, user_داده, codeloc);
#وإلا
  أرجع ffi_prep_بستگی_loc_sysv (بستگی, cif, fun, user_داده, codeloc);
#پایان_اگر
}

ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی *بستگی,
		     ffi_cif *cif,
		     فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *))
{
#اگر_تعریف_شده POWERPC64
  بستگی->tramp = ffi_go_بستگی_linux64;
#وإلا
  بستگی->tramp = ffi_go_بستگی_sysv;
#پایان_اگر
  بستگی->cif = cif;
  بستگی->fun = fun;
  أرجع FFI_OK;
}

#اگر_تعریف_شده FFI_EXEC_STATIC_TRAMP
فراغ *
ffi_tramp_arch (size_t *tramp_اندازه, size_t *map_اندازه)
{
  خارجي فراغ *trampoline_کد_جدول;
  *tramp_اندازه = PPC_TRAMP_اندازه;
  *map_اندازه = PPC_TRAMP_MAP_اندازه;
#إذا defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  أرجع *(فراغ **)trampoline_کد_جدول;
#وإلا
  أرجع &trampoline_کد_جدول;
#پایان_اگر
}
#پایان_اگر

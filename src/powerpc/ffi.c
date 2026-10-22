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

#شامل "ffi.h"
#شامل "ffi_common.h"
#شامل "ffi_powerpc.h"
#شامل "internal.h"
#شامل <tramp.h>

#اگر HAVE_LONG_DOUBLE_VARIANT
/* Adjust ffi_type_longdouble.  */
خالی FFI_HIDDEN
ffi_prep_types (ffi_abi abi)
{
# اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
#  ifdef POWERPC64
  ffi_prep_types_linux64 (abi);
#  ورنہ
  ffi_prep_types_sysv (abi);
#  endif
# endif
}
#اگر_ختم

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep (ffi_cif *cif)
{
#اگر_تعریف_شدہ POWERPC64
  واپس ffi_prep_cif_linux64 (cif);
#ورنہ
  واپس ffi_prep_cif_sysv (cif);
#اگر_ختم
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var (ffi_cif *cif,
			  غیرعلامتی صحیح nfixedargs MAYBE_غیر_مستعمل,
			  غیرعلامتی صحیح ntotalargs MAYBE_غیر_مستعمل)
{
#اگر_تعریف_شدہ POWERPC64
  واپس ffi_prep_cif_linux64_var (cif, nfixedargs, ntotalargs);
#ورنہ
  واپس ffi_prep_cif_sysv (cif);
#اگر_ختم
}

ساکن خالی
ffi_call_int (ffi_cif *cif,
	      خالی (*fn) (خالی),
	      خالی *rvalue,
	      خالی **avalue,
	      خالی *احاطہ)
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
  اگر ((cif->flags & FLAG_RETURNS_SMST) != 0)
    ecif.rvalue = smst_بفر;
  /* Ensure that we have a valid struct return value.
     FIXME: Isn't this just papering over a user problem?  */
  ورنہ اگر (!rvalue && cif->rtype->type == FFI_TYPE_STRUCT)
    ecif.rvalue = alloca (cif->rtype->سائز);

#اگر_تعریف_شدہ POWERPC64
  ffi_call_LINUX64 (&ecif, fn, ecif.rvalue, cif->flags, احاطہ,
		    -(طویل) cif->بائٹیں);
#ورنہ
  ffi_call_SYSV (&ecif, fn, ecif.rvalue, cif->flags, احاطہ, -cif->بائٹیں);
#اگر_ختم

  /* Check for a bounce-buffered return value */
  اگر (rvalue && ecif.rvalue == smst_بفر)
    {
      غیرعلامتی صحیح rsize = cif->rtype->سائز;
#اگر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
      /* Integer-typed _Complex: the .Lsmall_struct return path stored
	 r3 at buffer offset 0 and r4 at offset 8.  Extract one inner-
	 sized half from each slot — right-justified on BE, low-address
	 on LE — and lay them out as the natural packed complex value
	 the caller expects (real@0, imag@hsize).  */
      اگر (cif->rtype->type == FFI_TYPE_COMPLEX
	  && (cif->flags & (FLAG_RETURNS_FP | FLAG_RETURNS_VEC)) == 0)
	{
	  size_t hsize = cif->rtype->elements[0]->سائز;
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
	  size_t off = 8 - hsize;
#ورنہ
	  size_t off = 0;
#اگر_ختم
	  حافظہ_نقل ((حرف *) rvalue,         (حرف *) smst_بفر + off, hsize);
	  حافظہ_نقل ((حرف *) rvalue + hsize, (حرف *) smst_بفر + 8 + off, hsize);
	}
      ورنہ
#اگر_ختم
#اگر_غیر_تعریف_شدہ __LITTLE_ENDIAN__
      /* The SYSV ABI returns a structure of up to 4 bytes in size
	 left-padded in r3.  */
# ifndef POWERPC64
      اگر (rsize <= 4)
	حافظہ_نقل (rvalue, (حرف *) smst_بفر + 4 - rsize, rsize);
      ورنہ
# endif
	/* The SYSV ABI returns a structure of up to 8 bytes in size
	   left-padded in r3/r4, and the ELFv2 ABI similarly returns a
	   structure of up to 8 bytes in size left-padded in r3. But
	   note that a structure of a single float is not paddded.  */
	اگر (rsize <= 8 && (cif->flags & FLAG_RETURNS_FP) == 0)
	  حافظہ_نقل (rvalue, (حرف *) smst_بفر + 8 - rsize, rsize);
	ورنہ
#اگر_ختم
	  حافظہ_نقل (rvalue, smst_بفر, rsize);
    }
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn) (خالی), خالی *rvalue, خالی **avalue,
	     خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
		      ffi_cif *cif,
		      خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
#اگر_تعریف_شدہ POWERPC64
  واپس ffi_prep_احاطہ_loc_linux64 (احاطہ, cif, fun, user_ڈیٹا, codeloc);
#ورنہ
  واپس ffi_prep_احاطہ_loc_sysv (احاطہ, cif, fun, user_ڈیٹا, codeloc);
#اگر_ختم
}

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ,
		     ffi_cif *cif,
		     خالی (*fun) (ffi_cif *, خالی *, خالی **, خالی *))
{
#اگر_تعریف_شدہ POWERPC64
  احاطہ->tramp = ffi_go_احاطہ_linux64;
#ورنہ
  احاطہ->tramp = ffi_go_احاطہ_sysv;
#اگر_ختم
  احاطہ->cif = cif;
  احاطہ->fun = fun;
  واپس FFI_OK;
}

#اگر_تعریف_شدہ FFI_EXEC_STATIC_TRAMP
خالی *
ffi_tramp_arch (size_t *tramp_سائز, size_t *map_سائز)
{
  خارجی خالی *trampoline_رمز_جدول;
  *tramp_سائز = PPC_TRAMP_سائز;
  *map_سائز = PPC_TRAMP_MAP_سائز;
#اگر defined (_CALL_AIX) || _CALL_ELF == 1
  /* The caller wants the entry point address of the trampoline code,
     not the address of the function descriptor.  */
  واپس *(خالی **)trampoline_رمز_جدول;
#ورنہ
  واپس &trampoline_رمز_جدول;
#اگر_ختم
}
#اگر_ختم

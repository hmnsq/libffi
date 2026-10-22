/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2012, 2026  Anthony Green
	   Copyright (c) 1998, 2001, 2007, 2008  Red Hat, Inc.

   Alpha Foreign Function Interface

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
#ಅನ್ತರ್ಭಾವ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ಆದರೆ defined(__LONG_DOUBLE_128__)
# ಆದರೆ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ಹೊರಹೋಗು ದ date
# endif
#ಇಲ್ಲದಿದ್ದರೆ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_osf(ಶೂನ್ಯ *ರಾಶಿ, ಶೂನ್ಯ *ಚೌಕಟ್ಟು, ಚಿಹ್ನೆರಹಿತ flags,
			 ಶೂನ್ಯ *raddr, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ)
	FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_osf(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_osf(ಶೂನ್ಯ) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ UINT64 lds(ಶೂನ್ಯ *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  ಹಿಂತಿರುಗಿ ret;
}

/* And the reverse.  */
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ sts(ಶೂನ್ಯ *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t ಬೈಟುಗಳು = 0;
  ಪೂರ್ಣಾಂಕ flags, i, avn;
  ffi_type *rtype, *itype;

  ಆದರೆ (cif->abi != FFI_OSF)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  ಗಾಗಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      ಆಯ್ಕೆ (itype->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  ಬೈಟುಗಳು += 8;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_VOID:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  ಬೈಟುಗಳು += FFI_ALIGN(itype->ಗಾತ್ರ_2, FFI_SIZEOF_ARG);
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  ಆದರೆ (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    ಬೈಟುಗಳು += 8;
	  ಇಲ್ಲದಿದ್ದರೆ
	    ಬೈಟುಗಳು += 16;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  ಆಯ್ಕೆ (rtype->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT128:
    ಸ್ಥಿತಿ FFI_TYPE_UINT128:
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_ಒಳ_MEM;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      ಆಯ್ಕೆ (itype->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  ಆದರೆ (rtype->ಗಾತ್ರ_2 <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  ಇಲ್ಲದಿದ್ದರೆ
	    flags = ALPHA_RET_ಒಳ_MEM;
	  ನಿಲ್ಲಿಸು;
	}
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  ಆದರೆ (flags == ALPHA_RET_ಒಳ_MEM)
    ಬೈಟುಗಳು += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  ಆದರೆ (ಬೈಟುಗಳು < 6*8)
    ಬೈಟುಗಳು = 6*8;
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ
extend_basic_type(ಶೂನ್ಯ *valp, ಪೂರ್ಣಾಂಕ type, ಪೂರ್ಣಾಂಕ argn)
{
  ಆಯ್ಕೆ (type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      ಹಿಂತಿರುಗಿ *(SINT8 *)valp;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      ಹಿಂತಿರುಗಿ *(UINT8 *)valp;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      ಹಿಂತಿರುಗಿ *(SINT16 *)valp;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      ಹಿಂತಿರುಗಿ *(UINT16 *)valp;

    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಆದರೆ (argn < 6)
	ಹಿಂತಿರುಗಿ lds(valp);
      /* FALLTHRU */

    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      ಹಿಂತಿರುಗಿ *(SINT32 *)valp;

    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      ಹಿಂತಿರುಗಿ *(UINT64 *)valp;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *argp;
  ದೀರ್ಘ i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  ಶೂನ್ಯ *ಚೌಕಟ್ಟು;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  ಆದರೆ (rvalue == ಖಾಲಿ && flags == ALPHA_RET_ಒಳ_MEM)
    rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = ಚೌಕಟ್ಟು = alloca(cif->ಬೈಟುಗಳು + 4*FFI_SIZEOF_ARG);
  ಚೌಕಟ್ಟು += cif->ಬೈಟುಗಳು;

  argn = 0;
  ಆದರೆ (flags == ALPHA_RET_ಒಳ_MEM)
    argp[argn++] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಗಾಗಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *valp = avalue[i];
      ಪೂರ್ಣಾಂಕ type = ty->type;
      size_t ಗಾತ್ರ_2;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)valp;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_VOID:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_2 = ty->ಗಾತ್ರ_2;
	  ಸ್ಮರಣೆ_ನಕಲು(argp + argn, valp, ಗಾತ್ರ_2);
	  argn += FFI_ALIGN(ಗಾತ್ರ_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ಆದರೆ (type == FFI_TYPE_LONGDOUBLE)
	    ಹೋಗು by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  ಗಾತ್ರ_2 = ty->elements[0]->ಗಾತ್ರ_2;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + ಗಾತ್ರ_2, type, argn + 1);
	  argn += 2;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_ಸರಿಸು) & 0xff;
  ffi_call_osf(argp, ಚೌಕಟ್ಟು, flags, rvalue, fn, ಆವರಣ);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಆವರಣ);
}

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp;

  ಆದರೆ (cif->abi != FFI_OSF)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(ಶೂನ್ಯ **) &tramp[4] = ffi_ಆವರಣ_osf;

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ ಅಸ್ಥಿರ ("call_pal 0x86" : : : "memory");

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ,
		     ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಆದರೆ (cif->abi != FFI_OSF)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = (ಶೂನ್ಯ *)ffi_go_ಆವರಣ_osf;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ದೀರ್ಘ FFI_HIDDEN
ffi_ಆವರಣ_osf_inner (ffi_cif *cif,
		       ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		       ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		       ಶೂನ್ಯ *rvalue, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *argp)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **arg_types;
  ದೀರ್ಘ i, avn, argn, flags;

  avalue = alloca(cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ಆದರೆ (flags == ALPHA_RET_ಒಳ_MEM)
    {
      rvalue = (ಶೂನ್ಯ *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಗಾಗಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ಪೂರ್ಣಾಂಕ type = ty->type;
      ಶೂನ್ಯ *valp = &argp[argn];
      size_t ಗಾತ್ರ_2;

      ಆಯ್ಕೆ (type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  argn += 1;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_SINT128:
	ಸ್ಥಿತಿ FFI_TYPE_UINT128:
	ಸ್ಥಿತಿ FFI_TYPE_VOID:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_2 = ty->ಗಾತ್ರ_2;
	  argn += FFI_ALIGN(ಗಾತ್ರ_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  ಆದರೆ (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (ಶೂನ್ಯ *)argp[argn];
	  argn += 1;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ಆಯ್ಕೆ (type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_INT:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      ಗಾತ್ರ_2 = ty->elements[0]->ಗಾತ್ರ_2;
	      ಸ್ಮರಣೆ_ನಕಲು(valp + ಗಾತ್ರ_2, valp + 8, ಗಾತ್ರ_2);
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      ಆದರೆ (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      ಆದರೆ (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      ಇಲ್ಲದಿದ್ದರೆ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      ಆದರೆ (argn < 5)
		valp = &argp[argn - 6];
	      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      ನಿಲ್ಲಿಸು;

	    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	      ಹೋಗು by_reference;

	    ಪೂರ್ವನಿಯೋಜಿತ:
	      abort();
	    }
	  argn += 2;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ (flags >> ALPHA_LD_ಸರಿಸು) & 0xff;
}

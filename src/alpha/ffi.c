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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ਜੇ defined(__LONG_DOUBLE_128__)
# ਜੇ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
#ਨਹੀਂਤਾਂ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ ffi_call_osf(ਖਾਲੀ *ਢੇਰ, ਖਾਲੀ *ਚੌਖਟਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ flags,
			 ਖਾਲੀ *raddr, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ)
	FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_osf(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_osf(ਖਾਲੀ) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ਸਥਿਰ ਸਤਰਅੰਦਰ UINT64 lds(ਖਾਲੀ *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  ਵਾਪਸ ret;
}

/* And the reverse.  */
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ sts(ਖਾਲੀ *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t ਬਾਈਟਾਂ = 0;
  ਪੂਰਨ flags, i, avn;
  ffi_type *rtype, *itype;

  ਜੇ (cif->abi != FFI_OSF)
    ਵਾਪਸ FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  ਲਈ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      ਚੋਣ (itype->type)
	{
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  ਬਾਈਟਾਂ += 8;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_VOID:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  ਬਾਈਟਾਂ += FFI_ALIGN(itype->ਮਾਪ, FFI_SIZEOF_ARG);
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  ਜੇ (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    ਬਾਈਟਾਂ += 8;
	  ਨਹੀਂਤਾਂ
	    ਬਾਈਟਾਂ += 16;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  ਚੋਣ (rtype->type)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT128:
    ਹਾਲਤ FFI_TYPE_UINT128:
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
    ਹਾਲਤ FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_ਵਿੱਚ_MEM;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      ਚੋਣ (itype->type)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  ਰੋਕੋ;
	ਮੂਲ:
	  ਜੇ (rtype->ਮਾਪ <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  ਨਹੀਂਤਾਂ
	    flags = ALPHA_RET_ਵਿੱਚ_MEM;
	  ਰੋਕੋ;
	}
      ਰੋਕੋ;
    ਮੂਲ:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  ਜੇ (flags == ALPHA_RET_ਵਿੱਚ_MEM)
    ਬਾਈਟਾਂ += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  ਜੇ (ਬਾਈਟਾਂ < 6*8)
    ਬਾਈਟਾਂ = 6*8;
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ
extend_basic_type(ਖਾਲੀ *valp, ਪੂਰਨ type, ਪੂਰਨ argn)
{
  ਚੋਣ (type)
    {
    ਹਾਲਤ FFI_TYPE_SINT8:
      ਵਾਪਸ *(SINT8 *)valp;
    ਹਾਲਤ FFI_TYPE_UINT8:
      ਵਾਪਸ *(UINT8 *)valp;
    ਹਾਲਤ FFI_TYPE_SINT16:
      ਵਾਪਸ *(SINT16 *)valp;
    ਹਾਲਤ FFI_TYPE_UINT16:
      ਵਾਪਸ *(UINT16 *)valp;

    ਹਾਲਤ FFI_TYPE_FLOAT:
      ਜੇ (argn < 6)
	ਵਾਪਸ lds(valp);
      /* FALLTHRU */

    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      ਵਾਪਸ *(SINT32 *)valp;

    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      ਵਾਪਸ *(UINT64 *)valp;

    ਮੂਲ:
      abort();
    }
}

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *argp;
  ਲੰਮਾ i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  ਖਾਲੀ *ਚੌਖਟਾ;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  ਜੇ (rvalue == ਨਿਰਾ && flags == ALPHA_RET_ਵਿੱਚ_MEM)
    rvalue = alloca(cif->rtype->ਮਾਪ);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = ਚੌਖਟਾ = alloca(cif->ਬਾਈਟਾਂ + 4*FFI_SIZEOF_ARG);
  ਚੌਖਟਾ += cif->ਬਾਈਟਾਂ;

  argn = 0;
  ਜੇ (flags == ALPHA_RET_ਵਿੱਚ_MEM)
    argp[argn++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  ਲਈ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ਖਾਲੀ *valp = avalue[i];
      ਪੂਰਨ type = ty->type;
      size_t ਮਾਪ;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)valp;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_VOID:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ = ty->ਮਾਪ;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp + argn, valp, ਮਾਪ);
	  argn += FFI_ALIGN(ਮਾਪ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ਜੇ (type == FFI_TYPE_LONGDOUBLE)
	    ਜਾਓ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  ਮਾਪ = ty->elements[0]->ਮਾਪ;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + ਮਾਪ, type, argn + 1);
	  argn += 2;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_ਖਿਸਕਾਓ) & 0xff;
  ffi_call_osf(argp, ਚੌਖਟਾ, flags, rvalue, fn, ਘੇਰਾ);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਘੇਰਾ);
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp;

  ਜੇ (cif->abi != FFI_OSF)
    ਵਾਪਸ FFI_BAD_ABI;

  tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(ਖਾਲੀ **) &tramp[4] = ffi_ਘੇਰਾ_osf;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ ਅਸਥਿਰ ("call_pal 0x86" : : : "memory");

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ,
		     ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
  ਜੇ (cif->abi != FFI_OSF)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = (ਖਾਲੀ *)ffi_go_ਘੇਰਾ_osf;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

ਲੰਮਾ FFI_HIDDEN
ffi_ਘੇਰਾ_osf_inner (ffi_cif *cif,
		       ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		       ਖਾਲੀ *user_ਅੰਕੜੇ,
		       ਖਾਲੀ *rvalue, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *argp)
{
  ਖਾਲੀ **avalue;
  ffi_type **arg_types;
  ਲੰਮਾ i, avn, argn, flags;

  avalue = alloca(cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ਜੇ (flags == ALPHA_RET_ਵਿੱਚ_MEM)
    {
      rvalue = (ਖਾਲੀ *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਲਈ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ਪੂਰਨ type = ty->type;
      ਖਾਲੀ *valp = &argp[argn];
      size_t ਮਾਪ;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  argn += 1;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
	ਹਾਲਤ FFI_TYPE_VOID:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ = ty->ਮਾਪ;
	  argn += FFI_ALIGN(ਮਾਪ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  ਜੇ (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (ਖਾਲੀ *)argp[argn];
	  argn += 1;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ਚੋਣ (type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT64:
	    ਹਾਲਤ FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_INT:
	    ਹਾਲਤ FFI_TYPE_SINT8:
	    ਹਾਲਤ FFI_TYPE_UINT8:
	    ਹਾਲਤ FFI_TYPE_SINT16:
	    ਹਾਲਤ FFI_TYPE_UINT16:
	    ਹਾਲਤ FFI_TYPE_SINT32:
	    ਹਾਲਤ FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      ਮਾਪ = ty->elements[0]->ਮਾਪ;
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ(valp + ਮਾਪ, valp + 8, ਮਾਪ);
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      ਜੇ (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      ਜੇ (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      ਨਹੀਂਤਾਂ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      ਜੇ (argn < 5)
		valp = &argp[argn - 6];
	      ਨਹੀਂਤਾਂ ਜੇ (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	      ਜਾਓ by_reference;

	    ਮੂਲ:
	      abort();
	    }
	  argn += 2;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  ਵਾਪਸ (flags >> ALPHA_LD_ਖਿਸਕਾਓ) & 0xff;
}

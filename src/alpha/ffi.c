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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>
#ضمن "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#إذا defined(__LONG_DOUBLE_128__)
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ٻاهريون جو date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#جيڪڏهن_پورو

خارجي فراغ ffi_call_osf(فراغ *ڍير, فراغ *چوکٺو, بنانشان flags,
			 فراغ *raddr, فراغ (*fn)(فراغ), فراغ *گهيرو)
	FFI_HIDDEN;
خارجي فراغ ffi_گهيرو_osf(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_گهيرو_osf(فراغ) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ساكن مضمن UINT64 lds(فراغ *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  أرجع ret;
}

/* And the reverse.  */
ساكن مضمن فراغ sts(فراغ *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t بائيٽون = 0;
  صحيح flags, i, avn;
  ffi_type *rtype, *itype;

  إذا (cif->abi != FFI_OSF)
    أرجع FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  لكل (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      بدل (itype->type)
	{
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_اشارو:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  بائيٽون += 8;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  بائيٽون += FFI_ALIGN(itype->سائيز, FFI_SIZEOF_ARG);
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  إذا (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    بائيٽون += 8;
	  وإلا
	    بائيٽون += 16;
	  اكسر;

	افتراضي:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  بدل (rtype->type)
    {
    حالة FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      اكسر;
    حالة FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      اكسر;
    حالة FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      اكسر;
    حالة FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      اكسر;
    حالة FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      اكسر;
    حالة FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      اكسر;
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_اشارو:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      اكسر;
    حالة FFI_TYPE_SINT128:
    حالة FFI_TYPE_UINT128:
    حالة FFI_TYPE_LONGDOUBLE:
    حالة FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_اندريون_MEM;
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      بدل (itype->type)
	{
	حالة FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  اكسر;
	افتراضي:
	  إذا (rtype->سائيز <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  وإلا
	    flags = ALPHA_RET_اندريون_MEM;
	  اكسر;
	}
      اكسر;
    افتراضي:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  إذا (flags == ALPHA_RET_اندريون_MEM)
    بائيٽون += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  إذا (بائيٽون < 6*8)
    بائيٽون = 6*8;
  cif->بائيٽون = بائيٽون;

  أرجع FFI_OK;
}

ساكن بنانشان ڊگهو
extend_basic_type(فراغ *valp, صحيح type, صحيح argn)
{
  بدل (type)
    {
    حالة FFI_TYPE_SINT8:
      أرجع *(SINT8 *)valp;
    حالة FFI_TYPE_UINT8:
      أرجع *(UINT8 *)valp;
    حالة FFI_TYPE_SINT16:
      أرجع *(SINT16 *)valp;
    حالة FFI_TYPE_UINT16:
      أرجع *(UINT16 *)valp;

    حالة FFI_TYPE_FLOAT:
      إذا (argn < 6)
	أرجع lds(valp);
      /* FALLTHRU */

    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      أرجع *(SINT32 *)valp;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_اشارو:
    حالة FFI_TYPE_DOUBLE:
      أرجع *(UINT64 *)valp;

    افتراضي:
      abort();
    }
}

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *گهيرو)
{
  بنانشان ڊگهو *argp;
  ڊگهو i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  فراغ *چوکٺو;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  إذا (rvalue == عدم && flags == ALPHA_RET_اندريون_MEM)
    rvalue = alloca(cif->rtype->سائيز);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = چوکٺو = alloca(cif->بائيٽون + 4*FFI_SIZEOF_ARG);
  چوکٺو += cif->بائيٽون;

  argn = 0;
  إذا (flags == ALPHA_RET_اندريون_MEM)
    argp[argn++] = (بنانشان ڊگهو)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  لكل (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      فراغ *valp = avalue[i];
      صحيح type = ty->type;
      size_t سائيز;

      بدل (type)
	{
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_اشارو:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (بنانشان ڊگهو)valp;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  سائيز = ty->سائيز;
	  يادگيري_نقل(argp + argn, valp, سائيز);
	  argn += FFI_ALIGN(سائيز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  إذا (type == FFI_TYPE_LONGDOUBLE)
	    اذهب by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  سائيز = ty->elements[0]->سائيز;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + سائيز, type, argn + 1);
	  argn += 2;
	  اكسر;

	افتراضي:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_سرڪايو) & 0xff;
  ffi_call_osf(argp, چوکٺو, flags, rvalue, fn, گهيرو);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int(cif, fn, rvalue, avalue, گهيرو);
}

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بنانشان صحيح *tramp;

  إذا (cif->abi != FFI_OSF)
    أرجع FFI_BAD_ABI;

  tramp = (بنانشان صحيح *) &گهيرو->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(فراغ **) &tramp[4] = ffi_گهيرو_osf;

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ متطاير ("call_pal 0x86" : : : "memory");

  أرجع FFI_OK;
}

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو* گهيرو,
		     ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  إذا (cif->abi != FFI_OSF)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = (فراغ *)ffi_go_گهيرو_osf;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

ڊگهو FFI_HIDDEN
ffi_گهيرو_osf_inner (ffi_cif *cif,
		       فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		       فراغ *user_معلومات,
		       فراغ *rvalue, بنانشان ڊگهو *argp)
{
  فراغ **avalue;
  ffi_type **arg_types;
  ڊگهو i, avn, argn, flags;

  avalue = alloca(cif->nargs * حجم(فراغ *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  إذا (flags == ALPHA_RET_اندريون_MEM)
    {
      rvalue = (فراغ *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      صحيح type = ty->type;
      فراغ *valp = &argp[argn];
      size_t سائيز;

      بدل (type)
	{
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_اشارو:
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  سائيز = ty->سائيز;
	  argn += FFI_ALIGN(سائيز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  إذا (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (فراغ *)argp[argn];
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  بدل (type)
	    {
	    حالة FFI_TYPE_SINT64:
	    حالة FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      اكسر;

	    حالة FFI_TYPE_INT:
	    حالة FFI_TYPE_SINT8:
	    حالة FFI_TYPE_UINT8:
	    حالة FFI_TYPE_SINT16:
	    حالة FFI_TYPE_UINT16:
	    حالة FFI_TYPE_SINT32:
	    حالة FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      سائيز = ty->elements[0]->سائيز;
	      يادگيري_نقل(valp + سائيز, valp + 8, سائيز);
	      اكسر;

	    حالة FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      إذا (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      إذا (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      وإلا
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      اكسر;

	    حالة FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      إذا (argn < 5)
		valp = &argp[argn - 6];
	      وإلا إذا (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      اكسر;

	    حالة FFI_TYPE_LONGDOUBLE:
	      اذهب by_reference;

	    افتراضي:
	      abort();
	    }
	  argn += 2;
	  اكسر;

	افتراضي:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_معلومات);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  أرجع (flags >> ALPHA_LD_سرڪايو) & 0xff;
}

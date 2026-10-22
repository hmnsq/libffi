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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>
#شامل "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#اگر defined(__LONG_DOUBLE_128__)
# اگر FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE برآمد کا date
# endif
#ورنہ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#اگر_ختم

خارجی خالی ffi_call_osf(خالی *ڈھیر, خالی *چوکھٹا, غیرعلامتی flags,
			 خالی *raddr, خالی (*fn)(خالی), خالی *احاطہ)
	FFI_HIDDEN;
خارجی خالی ffi_احاطہ_osf(خالی) FFI_HIDDEN;
خارجی خالی ffi_go_احاطہ_osf(خالی) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ساکن اندرونی UINT64 lds(خالی *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  واپس ret;
}

/* And the reverse.  */
ساکن اندرونی خالی sts(خالی *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t بائٹیں = 0;
  صحیح flags, i, avn;
  ffi_type *rtype, *itype;

  اگر (cif->abi != FFI_OSF)
    واپس FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  برائے (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      انتخاب (itype->type)
	{
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_اشارہ_گر:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  بائٹیں += 8;
	  توڑو;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  بائٹیں += FFI_ALIGN(itype->سائز, FFI_SIZEOF_ARG);
	  توڑو;

	حالت FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  اگر (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    بائٹیں += 8;
	  ورنہ
	    بائٹیں += 16;
	  توڑو;

	طےشدہ:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  انتخاب (rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      توڑو;
    حالت FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      توڑو;
    حالت FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      توڑو;
    حالت FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      توڑو;
    حالت FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      توڑو;
    حالت FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      توڑو;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_اشارہ_گر:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      توڑو;
    حالت FFI_TYPE_SINT128:
    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_LONGDOUBLE:
    حالت FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_اندر_MEM;
      توڑو;
    حالت FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      انتخاب (itype->type)
	{
	حالت FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  توڑو;
	حالت FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  توڑو;
	طےشدہ:
	  اگر (rtype->سائز <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  ورنہ
	    flags = ALPHA_RET_اندر_MEM;
	  توڑو;
	}
      توڑو;
    طےشدہ:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  اگر (flags == ALPHA_RET_اندر_MEM)
    بائٹیں += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  اگر (بائٹیں < 6*8)
    بائٹیں = 6*8;
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

ساکن غیرعلامتی طویل
extend_basic_type(خالی *valp, صحیح type, صحیح argn)
{
  انتخاب (type)
    {
    حالت FFI_TYPE_SINT8:
      واپس *(SINT8 *)valp;
    حالت FFI_TYPE_UINT8:
      واپس *(UINT8 *)valp;
    حالت FFI_TYPE_SINT16:
      واپس *(SINT16 *)valp;
    حالت FFI_TYPE_UINT16:
      واپس *(UINT16 *)valp;

    حالت FFI_TYPE_FLOAT:
      اگر (argn < 6)
	واپس lds(valp);
      /* FALLTHRU */

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      واپس *(SINT32 *)valp;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_اشارہ_گر:
    حالت FFI_TYPE_DOUBLE:
      واپس *(UINT64 *)valp;

    طےشدہ:
      abort();
    }
}

ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  غیرعلامتی طویل *argp;
  طویل i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  خالی *چوکھٹا;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  اگر (rvalue == تہی && flags == ALPHA_RET_اندر_MEM)
    rvalue = alloca(cif->rtype->سائز);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = چوکھٹا = alloca(cif->بائٹیں + 4*FFI_SIZEOF_ARG);
  چوکھٹا += cif->بائٹیں;

  argn = 0;
  اگر (flags == ALPHA_RET_اندر_MEM)
    argp[argn++] = (غیرعلامتی طویل)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  برائے (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      خالی *valp = avalue[i];
      صحیح type = ty->type;
      size_t سائز;

      انتخاب (type)
	{
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_اشارہ_گر:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  توڑو;

	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (غیرعلامتی طویل)valp;
	  توڑو;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  سائز = ty->سائز;
	  حافظہ_نقل(argp + argn, valp, سائز);
	  argn += FFI_ALIGN(سائز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  توڑو;

	حالت FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  اگر (type == FFI_TYPE_LONGDOUBLE)
	    جاؤ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  سائز = ty->elements[0]->سائز;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + سائز, type, argn + 1);
	  argn += 2;
	  توڑو;

	طےشدہ:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_سرکاؤ) & 0xff;
  ffi_call_osf(argp, چوکھٹا, flags, rvalue, fn, احاطہ);
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int(cif, fn, rvalue, avalue, احاطہ);
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp;

  اگر (cif->abi != FFI_OSF)
    واپس FFI_BAD_ABI;

  tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(خالی **) &tramp[4] = ffi_احاطہ_osf;

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ متلون ("call_pal 0x86" : : : "memory");

  واپس FFI_OK;
}

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ,
		     ffi_cif* cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
  اگر (cif->abi != FFI_OSF)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = (خالی *)ffi_go_احاطہ_osf;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

طویل FFI_HIDDEN
ffi_احاطہ_osf_inner (ffi_cif *cif,
		       خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		       خالی *user_ڈیٹا,
		       خالی *rvalue, غیرعلامتی طویل *argp)
{
  خالی **avalue;
  ffi_type **arg_types;
  طویل i, avn, argn, flags;

  avalue = alloca(cif->nargs * حجم(خالی *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  اگر (flags == ALPHA_RET_اندر_MEM)
    {
      rvalue = (خالی *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  برائے (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      صحیح type = ty->type;
      خالی *valp = &argp[argn];
      size_t سائز;

      انتخاب (type)
	{
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  argn += 1;
	  توڑو;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  سائز = ty->سائز;
	  argn += FFI_ALIGN(سائز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  توڑو;

	حالت FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  اگر (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	  اگر (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  توڑو;

	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (خالی *)argp[argn];
	  argn += 1;
	  توڑو;

	حالت FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT64:
	    حالت FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      توڑو;

	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_SINT32:
	    حالت FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      سائز = ty->elements[0]->سائز;
	      حافظہ_نقل(valp + سائز, valp + 8, سائز);
	      توڑو;

	    حالت FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      اگر (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      اگر (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      ورنہ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      توڑو;

	    حالت FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      اگر (argn < 5)
		valp = &argp[argn - 6];
	      ورنہ اگر (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      توڑو;

	    حالت FFI_TYPE_LONGDOUBLE:
	      جاؤ by_reference;

	    طےشدہ:
	      abort();
	    }
	  argn += 2;
	  توڑو;

	طےشدہ:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  واپس (flags >> ALPHA_LD_سرکاؤ) & 0xff;
}

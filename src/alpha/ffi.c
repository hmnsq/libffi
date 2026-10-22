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
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#وگرنه
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

بیرونی تهی ffi_call_osf(تهی *پشته, تهی *frame, unsigned flags,
			 تهی *raddr, تهی (*fn)(تهی), تهی *closure)
	FFI_HIDDEN;
بیرونی تهی ffi_closure_osf(تهی) FFI_HIDDEN;
بیرونی تهی ffi_go_closure_osf(تهی) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ایستا درونخطی UINT64 lds(تهی *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  بازگشت ret;
}

/* And the reverse.  */
ایستا درونخطی تهی sts(تهی *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  صحیح flags, i, avn;
  ffi_type *rtype, *itype;

  اگر (cif->abi != FFI_OSF)
    بازگشت FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  برای (i = 0, avn = cif->nargs; i < avn; i++)
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
	حالت FFI_TYPE_POINTER:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  بشکن;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  بشکن;

	حالت FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  اگر (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  وگرنه
	    bytes += 16;
	  بشکن;

	پیشفرض:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  انتخاب (rtype->type)
    {
    حالت FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      بشکن;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      بشکن;
    حالت FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      بشکن;
    حالت FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      بشکن;
    حالت FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      بشکن;
    حالت FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      بشکن;
    حالت FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      بشکن;
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      بشکن;
    حالت FFI_TYPE_SINT128:
    حالت FFI_TYPE_UINT128:
    حالت FFI_TYPE_LONGDOUBLE:
    حالت FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      بشکن;
    حالت FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      انتخاب (itype->type)
	{
	حالت FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  بشکن;
	حالت FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  بشکن;
	پیشفرض:
	  اگر (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  وگرنه
	    flags = ALPHA_RET_IN_MEM;
	  بشکن;
	}
      بشکن;
    پیشفرض:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  اگر (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  اگر (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  بازگشت FFI_OK;
}

ایستا unsigned long
extend_basic_type(تهی *valp, صحیح type, صحیح argn)
{
  انتخاب (type)
    {
    حالت FFI_TYPE_SINT8:
      بازگشت *(SINT8 *)valp;
    حالت FFI_TYPE_UINT8:
      بازگشت *(UINT8 *)valp;
    حالت FFI_TYPE_SINT16:
      بازگشت *(SINT16 *)valp;
    حالت FFI_TYPE_UINT16:
      بازگشت *(UINT16 *)valp;

    حالت FFI_TYPE_FLOAT:
      اگر (argn < 6)
	بازگشت lds(valp);
      /* FALLTHRU */

    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      بازگشت *(SINT32 *)valp;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_POINTER:
    حالت FFI_TYPE_DOUBLE:
      بازگشت *(UINT64 *)valp;

    پیشفرض:
      abort();
    }
}

ایستا تهی
ffi_call_int (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	      تهی **avalue, تهی *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  تهی *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  اگر (rvalue == پوچ && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  اگر (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  برای (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      تهی *valp = avalue[i];
      صحیح type = ty->type;
      size_t size;

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
	حالت FFI_TYPE_POINTER:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  بشکن;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  بشکن;

	حالت FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  اگر (type == FFI_TYPE_LONGDOUBLE)
	    برو by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  بشکن;

	پیشفرض:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp;

  اگر (cif->abi != FFI_OSF)
    بازگشت FFI_BAD_ABI;

  tramp = (unsigned صحیح *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(تهی **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ فرار ("call_pal 0x86" : : : "memory");

  بازگشت FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*))
{
  اگر (cif->abi != FFI_OSF)
    بازگشت FFI_BAD_ABI;

  closure->tramp = (تهی *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		       تهی *user_data,
		       تهی *rvalue, unsigned long *argp)
{
  تهی **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * اندازه(تهی *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  اگر (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (تهی *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  برای (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      صحیح type = ty->type;
      تهی *valp = &argp[argn];
      size_t size;

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
	حالت FFI_TYPE_POINTER:
	  argn += 1;
	  بشکن;

	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	حالت FFI_TYPE_VOID:
	حالت FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  بشکن;

	حالت FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  اگر (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (تهی *)argp[argn];
	  argn += 1;
	  بشکن;

	حالت FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  انتخاب (type)
	    {
	    حالت FFI_TYPE_SINT64:
	    حالت FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      بشکن;

	    حالت FFI_TYPE_INT:
	    حالت FFI_TYPE_SINT8:
	    حالت FFI_TYPE_UINT8:
	    حالت FFI_TYPE_SINT16:
	    حالت FFI_TYPE_UINT16:
	    حالت FFI_TYPE_SINT32:
	    حالت FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      بشکن;

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
	      وگرنه
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      بشکن;

	    حالت FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      اگر (argn < 5)
		valp = &argp[argn - 6];
	      وگرنه اگر (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      بشکن;

	    حالت FFI_TYPE_LONGDOUBLE:
	      برو by_reference;

	    پیشفرض:
	      abort();
	    }
	  argn += 2;
	  بشکن;

	پیشفرض:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  بازگشت (flags >> ALPHA_LD_SHIFT) & 0xff;
}

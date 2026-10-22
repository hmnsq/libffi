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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#यदि defined(__LONG_DOUBLE_128__)
# यदि FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

बाह्य शून्य ffi_call_osf(शून्य *चळत, शून्य *frame, unsigned flags,
			 शून्य *raddr, शून्य (*fn)(शून्य), शून्य *closure)
	FFI_HIDDEN;
बाह्य शून्य ffi_closure_osf(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_osf(शून्य) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
स्थिर अन्तःस्थ UINT64 lds(शून्य *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  प्रत्यागम ret;
}

/* And the reverse.  */
स्थिर अन्तःस्थ शून्य sts(शून्य *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  पूर्णाङ्क flags, i, avn;
  ffi_type *rtype, *itype;

  यदि (cif->abi != FFI_OSF)
    प्रत्यागम FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  प्रति (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      विकल्प (itype->type)
	{
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_VOID:
	पक्ष FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  भङ्ग;

	पक्ष FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  यदि (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  अन्यथा
	    bytes += 16;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  विकल्प (rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      भङ्ग;
    पक्ष FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      भङ्ग;
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      भङ्ग;
    पक्ष FFI_TYPE_SINT128:
    पक्ष FFI_TYPE_UINT128:
    पक्ष FFI_TYPE_LONGDOUBLE:
    पक्ष FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      भङ्ग;
    पक्ष FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      विकल्प (itype->type)
	{
	पक्ष FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  भङ्ग;
	स्वभाव:
	  यदि (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  अन्यथा
	    flags = ALPHA_RET_IN_MEM;
	  भङ्ग;
	}
      भङ्ग;
    स्वभाव:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  यदि (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  यदि (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  प्रत्यागम FFI_OK;
}

स्थिर unsigned long
extend_basic_type(शून्य *valp, पूर्णाङ्क type, पूर्णाङ्क argn)
{
  विकल्प (type)
    {
    पक्ष FFI_TYPE_SINT8:
      प्रत्यागम *(SINT8 *)valp;
    पक्ष FFI_TYPE_UINT8:
      प्रत्यागम *(UINT8 *)valp;
    पक्ष FFI_TYPE_SINT16:
      प्रत्यागम *(SINT16 *)valp;
    पक्ष FFI_TYPE_UINT16:
      प्रत्यागम *(UINT16 *)valp;

    पक्ष FFI_TYPE_FLOAT:
      यदि (argn < 6)
	प्रत्यागम lds(valp);
      /* FALLTHRU */

    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      प्रत्यागम *(SINT32 *)valp;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_POINTER:
    पक्ष FFI_TYPE_DOUBLE:
      प्रत्यागम *(UINT64 *)valp;

    स्वभाव:
      abort();
    }
}

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  शून्य *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  यदि (rvalue == रिक्त && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  यदि (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  प्रति (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *valp = avalue[i];
      पूर्णाङ्क type = ty->type;
      size_t size;

      विकल्प (type)
	{
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_VOID:
	पक्ष FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  भङ्ग;

	पक्ष FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  यदि (type == FFI_TYPE_LONGDOUBLE)
	    गच्छ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp;

  यदि (cif->abi != FFI_OSF)
    प्रत्यागम FFI_BAD_ABI;

  tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(शून्य **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ चञ्चल ("call_pal 0x86" : : : "memory");

  प्रत्यागम FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  यदि (cif->abi != FFI_OSF)
    प्रत्यागम FFI_BAD_ABI;

  closure->tramp = (शून्य *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  प्रत्यागम FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		       शून्य *user_data,
		       शून्य *rvalue, unsigned long *argp)
{
  शून्य **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * परिमाण(शून्य *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  यदि (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (शून्य *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  प्रति (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णाङ्क type = ty->type;
      शून्य *valp = &argp[argn];
      size_t size;

      विकल्प (type)
	{
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	  argn += 1;
	  भङ्ग;

	पक्ष FFI_TYPE_SINT128:
	पक्ष FFI_TYPE_UINT128:
	पक्ष FFI_TYPE_VOID:
	पक्ष FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  भङ्ग;

	पक्ष FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  यदि (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  भङ्ग;

	पक्ष FFI_TYPE_DOUBLE:
	  यदि (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (शून्य *)argp[argn];
	  argn += 1;
	  भङ्ग;

	पक्ष FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  विकल्प (type)
	    {
	    पक्ष FFI_TYPE_SINT64:
	    पक्ष FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      भङ्ग;

	    पक्ष FFI_TYPE_INT:
	    पक्ष FFI_TYPE_SINT8:
	    पक्ष FFI_TYPE_UINT8:
	    पक्ष FFI_TYPE_SINT16:
	    पक्ष FFI_TYPE_UINT16:
	    पक्ष FFI_TYPE_SINT32:
	    पक्ष FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      भङ्ग;

	    पक्ष FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      यदि (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      यदि (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      अन्यथा
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      भङ्ग;

	    पक्ष FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      यदि (argn < 5)
		valp = &argp[argn - 6];
	      अन्यथा यदि (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      भङ्ग;

	    पक्ष FFI_TYPE_LONGDOUBLE:
	      गच्छ by_reference;

	    स्वभाव:
	      abort();
	    }
	  argn += 2;
	  भङ्ग;

	स्वभाव:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  प्रत्यागम (flags >> ALPHA_LD_SHIFT) & 0xff;
}

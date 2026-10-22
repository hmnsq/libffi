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
#जर defined(__LONG_DOUBLE_128__)
# जर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

बाह्य शून्य ffi_call_osf(शून्य *चळत, शून्य *frame, अचिन्हित flags,
			 शून्य *raddr, शून्य (*fn)(शून्य), शून्य *closure)
	FFI_HIDDEN;
बाह्य शून्य ffi_closure_osf(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_osf(शून्य) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
स्थिर ओळीत UINT64 lds(शून्य *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  परत ret;
}

/* And the reverse.  */
स्थिर ओळीत शून्य sts(शून्य *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  माप_प्रकार bytes = 0;
  पूर्णांक flags, i, avn;
  ffi_type *rtype, *itype;

  जर (cif->abi != FFI_OSF)
    परत FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  साठी (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      निवड (itype->type)
	{
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_POINTER:
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  थांबा;

	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
	स्थिती FFI_TYPE_VOID:
	स्थिती FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  थांबा;

	स्थिती FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  जर (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  अन्यथा
	    bytes += 16;
	  थांबा;

	पूर्वनिर्धारित:
	  पात();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  निवड (rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      थांबा;
    स्थिती FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      थांबा;
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      थांबा;
    स्थिती FFI_TYPE_SINT128:
    स्थिती FFI_TYPE_UINT128:
    स्थिती FFI_TYPE_LONGDOUBLE:
    स्थिती FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      थांबा;
    स्थिती FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      निवड (itype->type)
	{
	स्थिती FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  थांबा;
	स्थिती FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  थांबा;
	पूर्वनिर्धारित:
	  जर (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  अन्यथा
	    flags = ALPHA_RET_IN_MEM;
	  थांबा;
	}
      थांबा;
    पूर्वनिर्धारित:
      पात();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  जर (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  जर (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  परत FFI_OK;
}

स्थिर अचिन्हित दीर्घ
extend_basic_type(शून्य *valp, पूर्णांक type, पूर्णांक argn)
{
  निवड (type)
    {
    स्थिती FFI_TYPE_SINT8:
      परत *(SINT8 *)valp;
    स्थिती FFI_TYPE_UINT8:
      परत *(UINT8 *)valp;
    स्थिती FFI_TYPE_SINT16:
      परत *(SINT16 *)valp;
    स्थिती FFI_TYPE_UINT16:
      परत *(UINT16 *)valp;

    स्थिती FFI_TYPE_FLOAT:
      जर (argn < 6)
	परत lds(valp);
      /* FALLTHRU */

    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      परत *(SINT32 *)valp;

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_POINTER:
    स्थिती FFI_TYPE_DOUBLE:
      परत *(UINT64 *)valp;

    पूर्वनिर्धारित:
      पात();
    }
}

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  अचिन्हित दीर्घ *argp;
  दीर्घ i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  शून्य *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  जर (rvalue == रिक्त && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  जर (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (अचिन्हित दीर्घ)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  साठी (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      शून्य *valp = avalue[i];
      पूर्णांक type = ty->type;
      माप_प्रकार size;

      निवड (type)
	{
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_POINTER:
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  थांबा;

	स्थिती FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (अचिन्हित दीर्घ)valp;
	  थांबा;

	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
	स्थिती FFI_TYPE_VOID:
	स्थिती FFI_TYPE_STRUCT:
	  size = ty->size;
	  स_नकल(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  थांबा;

	स्थिती FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  जर (type == FFI_TYPE_LONGDOUBLE)
	    जा by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  थांबा;

	पूर्वनिर्धारित:
	  पात();
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
  अचिन्हित पूर्णांक *tramp;

  जर (cif->abi != FFI_OSF)
    परत FFI_BAD_ABI;

  tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
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
  __asm__ अस्थिर ("call_pal 0x86" : : : "memory");

  परत FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  जर (cif->abi != FFI_OSF)
    परत FFI_BAD_ABI;

  closure->tramp = (शून्य *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

दीर्घ FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		       शून्य *user_data,
		       शून्य *rvalue, अचिन्हित दीर्घ *argp)
{
  शून्य **avalue;
  ffi_type **arg_types;
  दीर्घ i, avn, argn, flags;

  avalue = alloca(cif->nargs * आकार(शून्य *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  जर (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (शून्य *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  साठी (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णांक type = ty->type;
      शून्य *valp = &argp[argn];
      माप_प्रकार size;

      निवड (type)
	{
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_POINTER:
	  argn += 1;
	  थांबा;

	स्थिती FFI_TYPE_SINT128:
	स्थिती FFI_TYPE_UINT128:
	स्थिती FFI_TYPE_VOID:
	स्थिती FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  थांबा;

	स्थिती FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  जर (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	  जर (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  थांबा;

	स्थिती FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (शून्य *)argp[argn];
	  argn += 1;
	  थांबा;

	स्थिती FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  निवड (type)
	    {
	    स्थिती FFI_TYPE_SINT64:
	    स्थिती FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      थांबा;

	    स्थिती FFI_TYPE_INT:
	    स्थिती FFI_TYPE_SINT8:
	    स्थिती FFI_TYPE_UINT8:
	    स्थिती FFI_TYPE_SINT16:
	    स्थिती FFI_TYPE_UINT16:
	    स्थिती FFI_TYPE_SINT32:
	    स्थिती FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      स_नकल(valp + size, valp + 8, size);
	      थांबा;

	    स्थिती FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      जर (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      जर (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      अन्यथा
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      थांबा;

	    स्थिती FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      जर (argn < 5)
		valp = &argp[argn - 6];
	      अन्यथा जर (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      थांबा;

	    स्थिती FFI_TYPE_LONGDOUBLE:
	      जा by_reference;

	    पूर्वनिर्धारित:
	      पात();
	    }
	  argn += 2;
	  थांबा;

	पूर्वनिर्धारित:
	  पात ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  परत (flags >> ALPHA_LD_SHIFT) & 0xff;
}

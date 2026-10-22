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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>
#समावेश "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#अगर defined(__LONG_DOUBLE_128__)
# अगर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#पूर्ण_अगर

बाह्य व्योम ffi_call_osf(व्योम *ढेर, व्योम *frame, अचिन्हित flags,
			 व्योम *raddr, व्योम (*fn)(व्योम), व्योम *closure)
	FFI_HIDDEN;
बाह्य व्योम ffi_closure_osf(व्योम) FFI_HIDDEN;
बाह्य व्योम ffi_go_closure_osf(व्योम) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
अटल अंतरभूत UINT64 lds(व्योम *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  वापस ret;
}

/* And the reverse.  */
अटल अंतरभूत व्योम sts(व्योम *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  माप_प्रकार bytes = 0;
  पूर्णांक flags, i, avn;
  ffi_type *rtype, *itype;

  अगर (cif->abi != FFI_OSF)
    वापस FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  क्रम (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      चयन (itype->type)
	{
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_POINTER:
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	हाल FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  अवरोध;

	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_VOID:
	हाल FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->आकार, FFI_SIZEOF_ARG);
	  अवरोध;

	हाल FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  अगर (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  अन्यथा
	    bytes += 16;
	  अवरोध;

	शेष:
	  पात();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  चयन (rtype->type)
    {
    हाल FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      अवरोध;
    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      अवरोध;
    हाल FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      अवरोध;
    हाल FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      अवरोध;
    हाल FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      अवरोध;
    हाल FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      अवरोध;
    हाल FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      अवरोध;
    हाल FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      अवरोध;
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      अवरोध;
    हाल FFI_TYPE_SINT128:
    हाल FFI_TYPE_UINT128:
    हाल FFI_TYPE_LONGDOUBLE:
    हाल FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      अवरोध;
    हाल FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      चयन (itype->type)
	{
	हाल FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  अवरोध;
	हाल FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  अवरोध;
	शेष:
	  अगर (rtype->आकार <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  अन्यथा
	    flags = ALPHA_RET_IN_MEM;
	  अवरोध;
	}
      अवरोध;
    शेष:
      पात();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  अगर (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  अगर (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  वापस FFI_OK;
}

अटल अचिन्हित दीर्घ
extend_basic_type(व्योम *valp, पूर्णांक type, पूर्णांक argn)
{
  चयन (type)
    {
    हाल FFI_TYPE_SINT8:
      वापस *(SINT8 *)valp;
    हाल FFI_TYPE_UINT8:
      वापस *(UINT8 *)valp;
    हाल FFI_TYPE_SINT16:
      वापस *(SINT16 *)valp;
    हाल FFI_TYPE_UINT16:
      वापस *(UINT16 *)valp;

    हाल FFI_TYPE_FLOAT:
      अगर (argn < 6)
	वापस lds(valp);
      /* FALLTHRU */

    हाल FFI_TYPE_INT:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      वापस *(SINT32 *)valp;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_POINTER:
    हाल FFI_TYPE_DOUBLE:
      वापस *(UINT64 *)valp;

    शेष:
      पात();
    }
}

अटल व्योम
ffi_call_int (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	      व्योम **avalue, व्योम *closure)
{
  अचिन्हित दीर्घ *argp;
  दीर्घ i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  व्योम *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  अगर (rvalue == शून्य && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->आकार);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  अगर (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (अचिन्हित दीर्घ)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  क्रम (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      व्योम *valp = avalue[i];
      पूर्णांक type = ty->type;
      माप_प्रकार आकार;

      चयन (type)
	{
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_POINTER:
	हाल FFI_TYPE_FLOAT:
	हाल FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (अचिन्हित दीर्घ)valp;
	  अवरोध;

	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_VOID:
	हाल FFI_TYPE_STRUCT:
	  आकार = ty->आकार;
	  स_नकल(argp + argn, valp, आकार);
	  argn += FFI_ALIGN(आकार, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  अवरोध;

	हाल FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  अगर (type == FFI_TYPE_LONGDOUBLE)
	    जाओ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  आकार = ty->elements[0]->आकार;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + आकार, type, argn + 1);
	  argn += 2;
	  अवरोध;

	शेष:
	  पात();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

व्योम
ffi_call (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, शून्य);
}

व्योम
ffi_call_go (ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue,
	     व्योम **avalue, व्योम *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp;

  अगर (cif->abi != FFI_OSF)
    वापस FFI_BAD_ABI;

  tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(व्योम **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ अस्थिर ("call_pal 0x86" : : : "memory");

  वापस FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*))
{
  अगर (cif->abi != FFI_OSF)
    वापस FFI_BAD_ABI;

  closure->tramp = (व्योम *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  वापस FFI_OK;
}

दीर्घ FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		       व्योम *user_तथ्य,
		       व्योम *rvalue, अचिन्हित दीर्घ *argp)
{
  व्योम **avalue;
  ffi_type **arg_types;
  दीर्घ i, avn, argn, flags;

  avalue = alloca(cif->nargs * माप(व्योम *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  अगर (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (व्योम *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  क्रम (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णांक type = ty->type;
      व्योम *valp = &argp[argn];
      माप_प्रकार आकार;

      चयन (type)
	{
	हाल FFI_TYPE_INT:
	हाल FFI_TYPE_SINT8:
	हाल FFI_TYPE_UINT8:
	हाल FFI_TYPE_SINT16:
	हाल FFI_TYPE_UINT16:
	हाल FFI_TYPE_SINT32:
	हाल FFI_TYPE_UINT32:
	हाल FFI_TYPE_SINT64:
	हाल FFI_TYPE_UINT64:
	हाल FFI_TYPE_POINTER:
	  argn += 1;
	  अवरोध;

	हाल FFI_TYPE_SINT128:
	हाल FFI_TYPE_UINT128:
	हाल FFI_TYPE_VOID:
	हाल FFI_TYPE_STRUCT:
	  आकार = ty->आकार;
	  argn += FFI_ALIGN(आकार, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  अवरोध;

	हाल FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  अगर (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  अवरोध;

	हाल FFI_TYPE_DOUBLE:
	  अगर (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  अवरोध;

	हाल FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (व्योम *)argp[argn];
	  argn += 1;
	  अवरोध;

	हाल FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  चयन (type)
	    {
	    हाल FFI_TYPE_SINT64:
	    हाल FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      अवरोध;

	    हाल FFI_TYPE_INT:
	    हाल FFI_TYPE_SINT8:
	    हाल FFI_TYPE_UINT8:
	    हाल FFI_TYPE_SINT16:
	    हाल FFI_TYPE_UINT16:
	    हाल FFI_TYPE_SINT32:
	    हाल FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      आकार = ty->elements[0]->आकार;
	      स_नकल(valp + आकार, valp + 8, आकार);
	      अवरोध;

	    हाल FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      अगर (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      अगर (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      अन्यथा
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      अवरोध;

	    हाल FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      अगर (argn < 5)
		valp = &argp[argn - 6];
	      अन्यथा अगर (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      अवरोध;

	    हाल FFI_TYPE_LONGDOUBLE:
	      जाओ by_reference;

	    शेष:
	      पात();
	    }
	  argn += 2;
	  अवरोध;

	शेष:
	  पात ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_तथ्य);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  वापस (flags >> ALPHA_LD_SHIFT) & 0xff;
}

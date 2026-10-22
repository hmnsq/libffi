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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#જો defined(__LONG_DOUBLE_128__)
# જો FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#નહીંતર
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

બાહ્ય શૂન્ય ffi_call_osf(શૂન્ય *ઢગલો, શૂન્ય *frame, unsigned flags,
			 શૂન્ય *raddr, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *closure)
	FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_closure_osf(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_go_closure_osf(શૂન્ય) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
સ્થિર અંતઃરેખ UINT64 lds(શૂન્ય *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  પાછા ret;
}

/* And the reverse.  */
સ્થિર અંતઃરેખ શૂન્ય sts(શૂન્ય *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  પૂર્ણાંક flags, i, avn;
  ffi_type *rtype, *itype;

  જો (cif->abi != FFI_OSF)
    પાછા FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  માટે (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      પસંદગી (itype->type)
	{
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_VOID:
	સ્થિતિ FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  તોડો;

	સ્થિતિ FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  જો (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  નહીંતર
	    bytes += 16;
	  તોડો;

	મૂળભૂત:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  પસંદગી (rtype->type)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      તોડો;
    સ્થિતિ FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      તોડો;
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      તોડો;
    સ્થિતિ FFI_TYPE_SINT128:
    સ્થિતિ FFI_TYPE_UINT128:
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
    સ્થિતિ FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      તોડો;
    સ્થિતિ FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      પસંદગી (itype->type)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  તોડો;
	સ્થિતિ FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  તોડો;
	મૂળભૂત:
	  જો (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  નહીંતર
	    flags = ALPHA_RET_IN_MEM;
	  તોડો;
	}
      તોડો;
    મૂળભૂત:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  જો (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  જો (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  પાછા FFI_OK;
}

સ્થિર unsigned long
extend_basic_type(શૂન્ય *valp, પૂર્ણાંક type, પૂર્ણાંક argn)
{
  પસંદગી (type)
    {
    સ્થિતિ FFI_TYPE_SINT8:
      પાછા *(SINT8 *)valp;
    સ્થિતિ FFI_TYPE_UINT8:
      પાછા *(UINT8 *)valp;
    સ્થિતિ FFI_TYPE_SINT16:
      પાછા *(SINT16 *)valp;
    સ્થિતિ FFI_TYPE_UINT16:
      પાછા *(UINT16 *)valp;

    સ્થિતિ FFI_TYPE_FLOAT:
      જો (argn < 6)
	પાછા lds(valp);
      /* FALLTHRU */

    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      પાછા *(SINT32 *)valp;

    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_POINTER:
    સ્થિતિ FFI_TYPE_DOUBLE:
      પાછા *(UINT64 *)valp;

    મૂળભૂત:
      abort();
    }
}

સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  શૂન્ય *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  જો (rvalue == ખાલી && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  જો (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  માટે (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      શૂન્ય *valp = avalue[i];
      પૂર્ણાંક type = ty->type;
      size_t size;

      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_VOID:
	સ્થિતિ FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  તોડો;

	સ્થિતિ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  જો (type == FFI_TYPE_LONGDOUBLE)
	    જાઓ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  તોડો;

	મૂળભૂત:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp;

  જો (cif->abi != FFI_OSF)
    પાછા FFI_BAD_ABI;

  tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(શૂન્ય **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ અસ્થિર ("call_pal 0x86" : : : "memory");

  પાછા FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
  જો (cif->abi != FFI_OSF)
    પાછા FFI_BAD_ABI;

  closure->tramp = (શૂન્ય *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		       શૂન્ય *user_data,
		       શૂન્ય *rvalue, unsigned long *argp)
{
  શૂન્ય **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * કદ(શૂન્ય *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  જો (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (શૂન્ય *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  માટે (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      પૂર્ણાંક type = ty->type;
      શૂન્ય *valp = &argp[argn];
      size_t size;

      પસંદગી (type)
	{
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  argn += 1;
	  તોડો;

	સ્થિતિ FFI_TYPE_SINT128:
	સ્થિતિ FFI_TYPE_UINT128:
	સ્થિતિ FFI_TYPE_VOID:
	સ્થિતિ FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  તોડો;

	સ્થિતિ FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  જો (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (શૂન્ય *)argp[argn];
	  argn += 1;
	  તોડો;

	સ્થિતિ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  પસંદગી (type)
	    {
	    સ્થિતિ FFI_TYPE_SINT64:
	    સ્થિતિ FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      તોડો;

	    સ્થિતિ FFI_TYPE_INT:
	    સ્થિતિ FFI_TYPE_SINT8:
	    સ્થિતિ FFI_TYPE_UINT8:
	    સ્થિતિ FFI_TYPE_SINT16:
	    સ્થિતિ FFI_TYPE_UINT16:
	    સ્થિતિ FFI_TYPE_SINT32:
	    સ્થિતિ FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      તોડો;

	    સ્થિતિ FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      જો (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      જો (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      નહીંતર
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      તોડો;

	    સ્થિતિ FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      જો (argn < 5)
		valp = &argp[argn - 6];
	      નહીંતર જો (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      તોડો;

	    સ્થિતિ FFI_TYPE_LONGDOUBLE:
	      જાઓ by_reference;

	    મૂળભૂત:
	      abort();
	    }
	  argn += 2;
	  તોડો;

	મૂળભૂત:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  પાછા (flags >> ALPHA_LD_SHIFT) & 0xff;
}

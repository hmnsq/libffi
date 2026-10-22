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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#എങ്കിൽ defined(__LONG_DOUBLE_128__)
# എങ്കിൽ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#അല്ലെങ്കിൽ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

ബാഹ്യ ശൂന്യം ffi_call_osf(ശൂന്യം *അട്ടി, ശൂന്യം *frame, unsigned flags,
			 ശൂന്യം *raddr, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *closure)
	FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_closure_osf(ശൂന്യം) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_go_closure_osf(ശൂന്യം) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
സ്ഥിതം വരിയിൽ UINT64 lds(ശൂന്യം *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  തിരികെ ret;
}

/* And the reverse.  */
സ്ഥിതം വരിയിൽ ശൂന്യം sts(ശൂന്യം *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  പൂർണ്ണസംഖ്യ flags, i, avn;
  ffi_type *rtype, *itype;

  എങ്കിൽ (cif->abi != FFI_OSF)
    തിരികെ FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  വേണ്ടി (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      തിരഞ്ഞെടുപ്പ് (itype->type)
	{
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT128:
	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_VOID:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  എങ്കിൽ (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  അല്ലെങ്കിൽ
	    bytes += 16;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  തിരഞ്ഞെടുപ്പ് (rtype->type)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT128:
    അവസ്ഥ FFI_TYPE_UINT128:
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
    അവസ്ഥ FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      തിരഞ്ഞെടുപ്പ് (itype->type)
	{
	അവസ്ഥ FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  എങ്കിൽ (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  അല്ലെങ്കിൽ
	    flags = ALPHA_RET_IN_MEM;
	  നിർത്തുക;
	}
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  എങ്കിൽ (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  എങ്കിൽ (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  തിരികെ FFI_OK;
}

സ്ഥിതം unsigned long
extend_basic_type(ശൂന്യം *valp, പൂർണ്ണസംഖ്യ type, പൂർണ്ണസംഖ്യ argn)
{
  തിരഞ്ഞെടുപ്പ് (type)
    {
    അവസ്ഥ FFI_TYPE_SINT8:
      തിരികെ *(SINT8 *)valp;
    അവസ്ഥ FFI_TYPE_UINT8:
      തിരികെ *(UINT8 *)valp;
    അവസ്ഥ FFI_TYPE_SINT16:
      തിരികെ *(SINT16 *)valp;
    അവസ്ഥ FFI_TYPE_UINT16:
      തിരികെ *(UINT16 *)valp;

    അവസ്ഥ FFI_TYPE_FLOAT:
      എങ്കിൽ (argn < 6)
	തിരികെ lds(valp);
      /* FALLTHRU */

    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      തിരികെ *(SINT32 *)valp;

    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_POINTER:
    അവസ്ഥ FFI_TYPE_DOUBLE:
      തിരികെ *(UINT64 *)valp;

    സ്ഥിരസ്ഥിതി:
      abort();
    }
}

സ്ഥിതം ശൂന്യം
ffi_call_int (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  ശൂന്യം *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  എങ്കിൽ (rvalue == ശൂന്യ && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  എങ്കിൽ (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  വേണ്ടി (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ശൂന്യം *valp = avalue[i];
      പൂർണ്ണസംഖ്യ type = ty->type;
      size_t size;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT128:
	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_VOID:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  എങ്കിൽ (type == FFI_TYPE_LONGDOUBLE)
	    പോകുക by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

ശൂന്യം
ffi_call (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp;

  എങ്കിൽ (cif->abi != FFI_OSF)
    തിരികെ FFI_BAD_ABI;

  tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(ശൂന്യം **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ അസ്ഥിര ("call_pal 0x86" : : : "memory");

  തിരികെ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*))
{
  എങ്കിൽ (cif->abi != FFI_OSF)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = (ശൂന്യം *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		       ശൂന്യം *user_data,
		       ശൂന്യം *rvalue, unsigned long *argp)
{
  ശൂന്യം **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * വലുപ്പം(ശൂന്യം *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  എങ്കിൽ (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (ശൂന്യം *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  വേണ്ടി (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      പൂർണ്ണസംഖ്യ type = ty->type;
      ശൂന്യം *valp = &argp[argn];
      size_t size;

      തിരഞ്ഞെടുപ്പ് (type)
	{
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  argn += 1;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_SINT128:
	അവസ്ഥ FFI_TYPE_UINT128:
	അവസ്ഥ FFI_TYPE_VOID:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  എങ്കിൽ (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (ശൂന്യം *)argp[argn];
	  argn += 1;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  തിരഞ്ഞെടുപ്പ് (type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT64:
	    അവസ്ഥ FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_INT:
	    അവസ്ഥ FFI_TYPE_SINT8:
	    അവസ്ഥ FFI_TYPE_UINT8:
	    അവസ്ഥ FFI_TYPE_SINT16:
	    അവസ്ഥ FFI_TYPE_UINT16:
	    അവസ്ഥ FFI_TYPE_SINT32:
	    അവസ്ഥ FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      എങ്കിൽ (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      എങ്കിൽ (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      അല്ലെങ്കിൽ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      എങ്കിൽ (argn < 5)
		valp = &argp[argn - 6];
	      അല്ലെങ്കിൽ എങ്കിൽ (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      നിർത്തുക;

	    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	      പോകുക by_reference;

	    സ്ഥിരസ്ഥിതി:
	      abort();
	    }
	  argn += 2;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  തിരികെ (flags >> ALPHA_LD_SHIFT) & 0xff;
}

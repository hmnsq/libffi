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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#எனில் defined(__LONG_DOUBLE_128__)
# எனில் FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#இல்லையெனில்
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

வெளி வெறுமை ffi_call_osf(வெறுமை *அடுக்கு, வெறுமை *frame, unsigned flags,
			 வெறுமை *raddr, வெறுமை (*fn)(வெறுமை), வெறுமை *closure)
	FFI_HIDDEN;
வெளி வெறுமை ffi_closure_osf(வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_go_closure_osf(வெறுமை) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
நிலையான உள்வரி UINT64 lds(வெறுமை *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  திரும்பு ret;
}

/* And the reverse.  */
நிலையான உள்வரி வெறுமை sts(வெறுமை *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  முழு flags, i, avn;
  ffi_type *rtype, *itype;

  எனில் (cif->abi != FFI_OSF)
    திரும்பு FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  க்கு (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      தேர்வு (itype->type)
	{
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_POINTER:
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_VOID:
	நிலை FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  நிறுத்து;

	நிலை FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  எனில் (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  இல்லையெனில்
	    bytes += 16;
	  நிறுத்து;

	இயல்பு:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  தேர்வு (rtype->type)
    {
    நிலை FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_UINT32:
    நிலை FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      நிறுத்து;
    நிலை FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      நிறுத்து;
    நிலை FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      நிறுத்து;
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      நிறுத்து;
    நிலை FFI_TYPE_SINT128:
    நிலை FFI_TYPE_UINT128:
    நிலை FFI_TYPE_LONGDOUBLE:
    நிலை FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      நிறுத்து;
    நிலை FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      தேர்வு (itype->type)
	{
	நிலை FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  நிறுத்து;
	நிலை FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  நிறுத்து;
	இயல்பு:
	  எனில் (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  இல்லையெனில்
	    flags = ALPHA_RET_IN_MEM;
	  நிறுத்து;
	}
      நிறுத்து;
    இயல்பு:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  எனில் (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  எனில் (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  திரும்பு FFI_OK;
}

நிலையான unsigned long
extend_basic_type(வெறுமை *valp, முழு type, முழு argn)
{
  தேர்வு (type)
    {
    நிலை FFI_TYPE_SINT8:
      திரும்பு *(SINT8 *)valp;
    நிலை FFI_TYPE_UINT8:
      திரும்பு *(UINT8 *)valp;
    நிலை FFI_TYPE_SINT16:
      திரும்பு *(SINT16 *)valp;
    நிலை FFI_TYPE_UINT16:
      திரும்பு *(UINT16 *)valp;

    நிலை FFI_TYPE_FLOAT:
      எனில் (argn < 6)
	திரும்பு lds(valp);
      /* FALLTHRU */

    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
    நிலை FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      திரும்பு *(SINT32 *)valp;

    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_POINTER:
    நிலை FFI_TYPE_DOUBLE:
      திரும்பு *(UINT64 *)valp;

    இயல்பு:
      abort();
    }
}

நிலையான வெறுமை
ffi_call_int (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	      வெறுமை **avalue, வெறுமை *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  வெறுமை *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  எனில் (rvalue == வெற்று && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  எனில் (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  க்கு (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      வெறுமை *valp = avalue[i];
      முழு type = ty->type;
      size_t size;

      தேர்வு (type)
	{
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_POINTER:
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_VOID:
	நிலை FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  நிறுத்து;

	நிலை FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  எனில் (type == FFI_TYPE_LONGDOUBLE)
	    செல் by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  நிறுத்து;

	இயல்பு:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp;

  எனில் (cif->abi != FFI_OSF)
    திரும்பு FFI_BAD_ABI;

  tramp = (unsigned முழு *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(வெறுமை **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ நிலையற்ற ("call_pal 0x86" : : : "memory");

  திரும்பு FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*))
{
  எனில் (cif->abi != FFI_OSF)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = (வெறுமை *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		       வெறுமை *user_data,
		       வெறுமை *rvalue, unsigned long *argp)
{
  வெறுமை **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * அளவு(வெறுமை *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  எனில் (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (வெறுமை *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  க்கு (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      முழு type = ty->type;
      வெறுமை *valp = &argp[argn];
      size_t size;

      தேர்வு (type)
	{
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_POINTER:
	  argn += 1;
	  நிறுத்து;

	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
	நிலை FFI_TYPE_VOID:
	நிலை FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  எனில் (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (வெறுமை *)argp[argn];
	  argn += 1;
	  நிறுத்து;

	நிலை FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  தேர்வு (type)
	    {
	    நிலை FFI_TYPE_SINT64:
	    நிலை FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      நிறுத்து;

	    நிலை FFI_TYPE_INT:
	    நிலை FFI_TYPE_SINT8:
	    நிலை FFI_TYPE_UINT8:
	    நிலை FFI_TYPE_SINT16:
	    நிலை FFI_TYPE_UINT16:
	    நிலை FFI_TYPE_SINT32:
	    நிலை FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      நிறுத்து;

	    நிலை FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      எனில் (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      எனில் (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      இல்லையெனில்
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      நிறுத்து;

	    நிலை FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      எனில் (argn < 5)
		valp = &argp[argn - 6];
	      இல்லையெனில் எனில் (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      நிறுத்து;

	    நிலை FFI_TYPE_LONGDOUBLE:
	      செல் by_reference;

	    இயல்பு:
	      abort();
	    }
	  argn += 2;
	  நிறுத்து;

	இயல்பு:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  திரும்பு (flags >> ALPHA_LD_SHIFT) & 0xff;
}

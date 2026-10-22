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
#𐤀𐤌 defined(__LONG_DOUBLE_128__)
# 𐤀𐤌 FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

خارجي فراغ ffi_call_osf(فراغ *stack, فراغ *frame, unsigned flags,
			 فراغ *raddr, فراغ (*fn)(فراغ), فراغ *closure)
	FFI_HIDDEN;
خارجي فراغ ffi_closure_osf(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_closure_osf(فراغ) FFI_HIDDEN;

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

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  صحيح flags, i, avn;
  ffi_type *rtype, *itype;

  𐤀𐤌 (cif->abi != FFI_OSF)
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
	حالة FFI_TYPE_POINTER:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  𐤀𐤌 (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  وإلا
	    bytes += 16;
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
    حالة FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      اكسر;
    حالة FFI_TYPE_SINT128:
    حالة FFI_TYPE_UINT128:
    حالة FFI_TYPE_LONGDOUBLE:
    حالة FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
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
	  𐤀𐤌 (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  وإلا
	    flags = ALPHA_RET_IN_MEM;
	  اكسر;
	}
      اكسر;
    افتراضي:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  𐤀𐤌 (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  𐤀𐤌 (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  أرجع FFI_OK;
}

ساكن unsigned long
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
      𐤀𐤌 (argn < 6)
	أرجع lds(valp);
      /* FALLTHRU */

    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      أرجع *(SINT32 *)valp;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_POINTER:
    حالة FFI_TYPE_DOUBLE:
      أرجع *(UINT64 *)valp;

    افتراضي:
      abort();
    }
}

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  فراغ *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  𐤀𐤌 (rvalue == عدم && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  𐤀𐤌 (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  لكل (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      فراغ *valp = avalue[i];
      صحيح type = ty->type;
      size_t size;

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
	حالة FFI_TYPE_POINTER:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  𐤀𐤌 (type == FFI_TYPE_LONGDOUBLE)
	    اذهب by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  اكسر;

	افتراضي:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  unsigned صحيح *tramp;

  𐤀𐤌 (cif->abi != FFI_OSF)
    أرجع FFI_BAD_ABI;

  tramp = (unsigned صحيح *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(فراغ **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ متطاير ("call_pal 0x86" : : : "memory");

  أرجع FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  𐤀𐤌 (cif->abi != FFI_OSF)
    أرجع FFI_BAD_ABI;

  closure->tramp = (فراغ *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  أرجع FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		       فراغ *user_data,
		       فراغ *rvalue, unsigned long *argp)
{
  فراغ **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * حجم(فراغ *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  𐤀𐤌 (flags == ALPHA_RET_IN_MEM)
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
      size_t size;

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
	حالة FFI_TYPE_POINTER:
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	حالة FFI_TYPE_VOID:
	حالة FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  𐤀𐤌 (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  𐤀𐤌 (argn < 6)
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
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      اكسر;

	    حالة FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      𐤀𐤌 (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      𐤀𐤌 (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      وإلا
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      اكسر;

	    حالة FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      𐤀𐤌 (argn < 5)
		valp = &argp[argn - 6];
	      وإلا 𐤀𐤌 (argn == 5)
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
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  أرجع (flags >> ALPHA_LD_SHIFT) & 0xff;
}

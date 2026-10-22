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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ಯದಿ defined(__LONG_DOUBLE_128__)
# ಯದಿ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ಅನ್ಯಥಾ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_osf(ಶೂನ್ಯ *ರಾಶಿ, ಶೂನ್ಯ *frame, unsigned flags,
			 ಶೂನ್ಯ *raddr, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *closure)
	FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_osf(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_osf(ಶೂನ್ಯ) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ UINT64 lds(ಶೂನ್ಯ *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  ಪ್ರತ್ಯಾಗಮ ret;
}

/* And the reverse.  */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ sts(ಶೂನ್ಯ *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  ಪೂರ್ಣಾಙ್ಕ flags, i, avn;
  ffi_type *rtype, *itype;

  ಯದಿ (cif->abi != FFI_OSF)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  ಪ್ರತಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      ವಿಕಲ್ಪ (itype->type)
	{
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT128:
	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_VOID:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->size, FFI_SIZEOF_ARG);
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  ಯದಿ (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  ಅನ್ಯಥಾ
	    bytes += 16;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  ವಿಕಲ್ಪ (rtype->type)
    {
    ಪಕ್ಷ FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_UINT32:
    ಪಕ್ಷ FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_SINT128:
    ಪಕ್ಷ FFI_TYPE_UINT128:
    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
    ಪಕ್ಷ FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      ಭಙ್ಗ;
    ಪಕ್ಷ FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      ವಿಕಲ್ಪ (itype->type)
	{
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  ಯದಿ (rtype->size <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  ಅನ್ಯಥಾ
	    flags = ALPHA_RET_IN_MEM;
	  ಭಙ್ಗ;
	}
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  ಯದಿ (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  ಯದಿ (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಸ್ಥಿರ unsigned long
extend_basic_type(ಶೂನ್ಯ *valp, ಪೂರ್ಣಾಙ್ಕ type, ಪೂರ್ಣಾಙ್ಕ argn)
{
  ವಿಕಲ್ಪ (type)
    {
    ಪಕ್ಷ FFI_TYPE_SINT8:
      ಪ್ರತ್ಯಾಗಮ *(SINT8 *)valp;
    ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪ್ರತ್ಯಾಗಮ *(UINT8 *)valp;
    ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪ್ರತ್ಯಾಗಮ *(SINT16 *)valp;
    ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪ್ರತ್ಯಾಗಮ *(UINT16 *)valp;

    ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಯದಿ (argn < 6)
	ಪ್ರತ್ಯಾಗಮ lds(valp);
      /* FALLTHRU */

    ಪಕ್ಷ FFI_TYPE_INT:
    ಪಕ್ಷ FFI_TYPE_SINT32:
    ಪಕ್ಷ FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      ಪ್ರತ್ಯಾಗಮ *(SINT32 *)valp;

    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
    ಪಕ್ಷ FFI_TYPE_POINTER:
    ಪಕ್ಷ FFI_TYPE_DOUBLE:
      ಪ್ರತ್ಯಾಗಮ *(UINT64 *)valp;

    ಸ್ವಭಾವ:
      abort();
    }
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  ಶೂನ್ಯ *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  ಯದಿ (rvalue == ರಿಕ್ತ && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->size);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  ಯದಿ (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಪ್ರತಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *valp = avalue[i];
      ಪೂರ್ಣಾಙ್ಕ type = ty->type;
      size_t size;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT128:
	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_VOID:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  size = ty->size;
	  memcpy(argp + argn, valp, size);
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ಯದಿ (type == FFI_TYPE_LONGDOUBLE)
	    ಗಚ್ಛ by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  size = ty->elements[0]->size;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + size, type, argn + 1);
	  argn += 2;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp;

  ಯದಿ (cif->abi != FFI_OSF)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(ಶೂನ್ಯ **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ ಚಞ್ಚಲ ("call_pal 0x86" : : : "memory");

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಯದಿ (cif->abi != FFI_OSF)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = (ಶೂನ್ಯ *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		       ಶೂನ್ಯ *user_data,
		       ಶೂನ್ಯ *rvalue, unsigned long *argp)
{
  ಶೂನ್ಯ **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ಯದಿ (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (ಶೂನ್ಯ *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಪ್ರತಿ (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ type = ty->type;
      ಶೂನ್ಯ *valp = &argp[argn];
      size_t size;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_SINT64:
	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  argn += 1;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_SINT128:
	ಪಕ್ಷ FFI_TYPE_UINT128:
	ಪಕ್ಷ FFI_TYPE_VOID:
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  size = ty->size;
	  argn += FFI_ALIGN(size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  ಯದಿ (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (ಶೂನ್ಯ *)argp[argn];
	  argn += 1;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ವಿಕಲ್ಪ (type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT64:
	    ಪಕ್ಷ FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_INT:
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	    ಪಕ್ಷ FFI_TYPE_SINT32:
	    ಪಕ್ಷ FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      size = ty->elements[0]->size;
	      memcpy(valp + size, valp + 8, size);
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      ಯದಿ (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      ಯದಿ (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      ಅನ್ಯಥಾ
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      ಯದಿ (argn < 5)
		valp = &argp[argn - 6];
	      ಅನ್ಯಥಾ ಯದಿ (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      ಭಙ್ಗ;

	    ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	      ಗಚ್ಛ by_reference;

	    ಸ್ವಭಾವ:
	      abort();
	    }
	  argn += 2;
	  ಭಙ್ಗ;

	ಸ್ವಭಾವ:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  ಪ್ರತ್ಯಾಗಮ (flags >> ALPHA_LD_SHIFT) & 0xff;
}

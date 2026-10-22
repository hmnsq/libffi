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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#যদি defined(__LONG_DOUBLE_128__)
# যদি FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#অন্যথা
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

বাহ্যিক শূন্য ffi_call_osf(শূন্য *স্তূপ, শূন্য *frame, unsigned flags,
			 শূন্য *raddr, শূন্য (*fn)(শূন্য), শূন্য *closure)
	FFI_HIDDEN;
বাহ্যিক শূন্য ffi_closure_osf(শূন্য) FFI_HIDDEN;
বাহ্যিক শূন্য ffi_go_closure_osf(শূন্য) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
স্থির অন্তঃরেখ UINT64 lds(শূন্য *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  ফেরত ret;
}

/* And the reverse.  */
স্থির অন্তঃরেখ শূন্য sts(শূন্য *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t bytes = 0;
  পূর্ণসংখ্যা flags, i, avn;
  ffi_type *rtype, *itype;

  যদি (cif->abi != FFI_OSF)
    ফেরত FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  জন্য (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      নির্বাচন (itype->type)
	{
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_POINTER:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  bytes += 8;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
	ক্ষেত্র FFI_TYPE_VOID:
	ক্ষেত্র FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  bytes += FFI_ALIGN(itype->আকার_2, FFI_SIZEOF_ARG);
	  বিরতি;

	ক্ষেত্র FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  যদি (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    bytes += 8;
	  অন্যথা
	    bytes += 16;
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  নির্বাচন (rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      বিরতি;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      বিরতি;
    ক্ষেত্র FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      বিরতি;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_POINTER:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT128:
    ক্ষেত্র FFI_TYPE_UINT128:
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
    ক্ষেত্র FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_IN_MEM;
      বিরতি;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      নির্বাচন (itype->type)
	{
	ক্ষেত্র FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  বিরতি;
	ক্ষেত্র FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  বিরতি;
	পূর্বনির্ধারিত:
	  যদি (rtype->আকার_2 <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  অন্যথা
	    flags = ALPHA_RET_IN_MEM;
	  বিরতি;
	}
      বিরতি;
    পূর্বনির্ধারিত:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  যদি (flags == ALPHA_RET_IN_MEM)
    bytes += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  যদি (bytes < 6*8)
    bytes = 6*8;
  cif->bytes = bytes;

  ফেরত FFI_OK;
}

স্থির unsigned long
extend_basic_type(শূন্য *valp, পূর্ণসংখ্যা type, পূর্ণসংখ্যা argn)
{
  নির্বাচন (type)
    {
    ক্ষেত্র FFI_TYPE_SINT8:
      ফেরত *(SINT8 *)valp;
    ক্ষেত্র FFI_TYPE_UINT8:
      ফেরত *(UINT8 *)valp;
    ক্ষেত্র FFI_TYPE_SINT16:
      ফেরত *(SINT16 *)valp;
    ক্ষেত্র FFI_TYPE_UINT16:
      ফেরত *(UINT16 *)valp;

    ক্ষেত্র FFI_TYPE_FLOAT:
      যদি (argn < 6)
	ফেরত lds(valp);
      /* FALLTHRU */

    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      ফেরত *(SINT32 *)valp;

    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_POINTER:
    ক্ষেত্র FFI_TYPE_DOUBLE:
      ফেরত *(UINT64 *)valp;

    পূর্বনির্ধারিত:
      abort();
    }
}

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  unsigned long *argp;
  long i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  শূন্য *frame;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  যদি (rvalue == ফাঁকা && flags == ALPHA_RET_IN_MEM)
    rvalue = alloca(cif->rtype->আকার_2);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = frame = alloca(cif->bytes + 4*FFI_SIZEOF_ARG);
  frame += cif->bytes;

  argn = 0;
  যদি (flags == ALPHA_RET_IN_MEM)
    argp[argn++] = (unsigned long)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  জন্য (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      শূন্য *valp = avalue[i];
      পূর্ণসংখ্যা type = ty->type;
      size_t আকার_2;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_POINTER:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (unsigned long)valp;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
	ক্ষেত্র FFI_TYPE_VOID:
	ক্ষেত্র FFI_TYPE_STRUCT:
	  আকার_2 = ty->আকার_2;
	  memcpy(argp + argn, valp, আকার_2);
	  argn += FFI_ALIGN(আকার_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  যদি (type == FFI_TYPE_LONGDOUBLE)
	    যাও by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  আকার_2 = ty->elements[0]->আকার_2;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + আকার_2, type, argn + 1);
	  argn += 2;
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_SHIFT) & 0xff;
  ffi_call_osf(argp, frame, flags, rvalue, fn, closure);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  unsigned পূর্ণসংখ্যা *tramp;

  যদি (cif->abi != FFI_OSF)
    ফেরত FFI_BAD_ABI;

  tramp = (unsigned পূর্ণসংখ্যা *) &closure->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(শূন্য **) &tramp[4] = ffi_closure_osf;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ অস্থির ("call_pal 0x86" : : : "memory");

  ফেরত FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure,
		     ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
  যদি (cif->abi != FFI_OSF)
    ফেরত FFI_BAD_ABI;

  closure->tramp = (শূন্য *)ffi_go_closure_osf;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

long FFI_HIDDEN
ffi_closure_osf_inner (ffi_cif *cif,
		       শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		       শূন্য *user_তথ্য,
		       শূন্য *rvalue, unsigned long *argp)
{
  শূন্য **avalue;
  ffi_type **arg_types;
  long i, avn, argn, flags;

  avalue = alloca(cif->nargs * আকার(শূন্য *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  যদি (flags == ALPHA_RET_IN_MEM)
    {
      rvalue = (শূন্য *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  জন্য (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      পূর্ণসংখ্যা type = ty->type;
      শূন্য *valp = &argp[argn];
      size_t আকার_2;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_POINTER:
	  argn += 1;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
	ক্ষেত্র FFI_TYPE_VOID:
	ক্ষেত্র FFI_TYPE_STRUCT:
	  আকার_2 = ty->আকার_2;
	  argn += FFI_ALIGN(আকার_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  যদি (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (শূন্য *)argp[argn];
	  argn += 1;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  নির্বাচন (type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT64:
	    ক্ষেত্র FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_INT:
	    ক্ষেত্র FFI_TYPE_SINT8:
	    ক্ষেত্র FFI_TYPE_UINT8:
	    ক্ষেত্র FFI_TYPE_SINT16:
	    ক্ষেত্র FFI_TYPE_UINT16:
	    ক্ষেত্র FFI_TYPE_SINT32:
	    ক্ষেত্র FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      আকার_2 = ty->elements[0]->আকার_2;
	      memcpy(valp + আকার_2, valp + 8, আকার_2);
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      যদি (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      যদি (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      অন্যথা
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      যদি (argn < 5)
		valp = &argp[argn - 6];
	      অন্যথা যদি (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	      যাও by_reference;

	    পূর্বনির্ধারিত:
	      abort();
	    }
	  argn += 2;
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_তথ্য);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  ফেরত (flags >> ALPHA_LD_SHIFT) & 0xff;
}

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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ఒకవేళ defined(__LONG_DOUBLE_128__)
# ఒకవేళ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE బహిర్గతం యొక్క date
# endif
#లేకపోతే
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#అయితే_ముగింపు

బాహ్య శూన్య ffi_call_osf(శూన్య *దొంతర, శూన్య *చట్రం, గుర్తులేని flags,
			 శూన్య *raddr, శూన్య (*fn)(శూన్య), శూన్య *సంవృతం)
	FFI_HIDDEN;
బాహ్య శూన్య ffi_సంవృతం_osf(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_go_సంవృతం_osf(శూన్య) FFI_HIDDEN;

/* Promote a float value to its in-register double representation.
   Unlike actually casting to double, this does not trap on NaN.  */
స్థిర అంతఃవరుస UINT64 lds(శూన్య *ptr)
{
  UINT64 ret;
  __asm__("lds %0,%1" : "=f"(ret) : "m"(*(UINT32 *)ptr));
  తిరిగి ret;
}

/* And the reverse.  */
స్థిర అంతఃవరుస శూన్య sts(శూన్య *ptr, UINT64 val)
{
  __asm__("sts %1,%0" : "=m"(*(UINT32 *)ptr) : "f"(val));
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t బైట్లు = 0;
  పూర్ణాంకం flags, i, avn;
  ffi_type *rtype, *itype;

  ఒకవేళ (cif->abi != FFI_OSF)
    తిరిగి FFI_BAD_ABI;

  /* Compute the size of the argument area.  */
  కొరకు (i = 0, avn = cif->nargs; i < avn; i++)
    {
      itype = cif->arg_types[i];
      ఎంపిక (itype->type)
	{
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_సూచిక:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	  /* All take one 8 byte slot.  */
	  బైట్లు += 8;
	  ఆపు;

	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_VOID:
	స్థితి FFI_TYPE_STRUCT:
	  /* Passed by value in N slots.  */
	  బైట్లు += FFI_ALIGN(itype->కొలత, FFI_SIZEOF_ARG);
	  ఆపు;

	స్థితి FFI_TYPE_COMPLEX:
	  /* _Complex long double passed by reference; others in 2 slots.  */
	  ఒకవేళ (itype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
	    బైట్లు += 8;
	  లేకపోతే
	    బైట్లు += 16;
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
    }

  /* Set the return type flag */
  rtype = cif->rtype;
  ఎంపిక (rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      flags = ALPHA_FLAGS(ALPHA_ST_VOID, ALPHA_LD_VOID);
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_SINT32:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT32);
      ఆపు;
    స్థితి FFI_TYPE_FLOAT:
      flags = ALPHA_FLAGS(ALPHA_ST_FLOAT, ALPHA_LD_FLOAT);
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      flags = ALPHA_FLAGS(ALPHA_ST_DOUBLE, ALPHA_LD_DOUBLE);
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT8);
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT8);
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_UINT16);
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_SINT16);
      ఆపు;
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_సూచిక:
      flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
      ఆపు;
    స్థితి FFI_TYPE_SINT128:
    స్థితి FFI_TYPE_UINT128:
    స్థితి FFI_TYPE_LONGDOUBLE:
    స్థితి FFI_TYPE_STRUCT:
      /* Passed in memory, with a hidden pointer.  */
      flags = ALPHA_RET_లోపల_MEM;
      ఆపు;
    స్థితి FFI_TYPE_COMPLEX:
      itype = rtype->elements[0];
      ఎంపిక (itype->type)
	{
	స్థితి FFI_TYPE_FLOAT:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXF, ALPHA_LD_CPLXF);
	  ఆపు;
	స్థితి FFI_TYPE_DOUBLE:
	  flags = ALPHA_FLAGS(ALPHA_ST_CPLXD, ALPHA_LD_CPLXD);
	  ఆపు;
	అప్రమేయ:
	  ఒకవేళ (rtype->కొలత <= 8)
	    flags = ALPHA_FLAGS(ALPHA_ST_INT, ALPHA_LD_INT64);
	  లేకపోతే
	    flags = ALPHA_RET_లోపల_MEM;
	  ఆపు;
	}
      ఆపు;
    అప్రమేయ:
      abort();
    }
  cif->flags = flags;

  /* Include the hidden structure pointer in args requirement.  */
  ఒకవేళ (flags == ALPHA_RET_లోపల_MEM)
    బైట్లు += 8;
  /* Minimum size is 6 slots, so that ffi_call_osf can pop them.  */
  ఒకవేళ (బైట్లు < 6*8)
    బైట్లు = 6*8;
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

స్థిర గుర్తులేని దీర్ఘ
extend_basic_type(శూన్య *valp, పూర్ణాంకం type, పూర్ణాంకం argn)
{
  ఎంపిక (type)
    {
    స్థితి FFI_TYPE_SINT8:
      తిరిగి *(SINT8 *)valp;
    స్థితి FFI_TYPE_UINT8:
      తిరిగి *(UINT8 *)valp;
    స్థితి FFI_TYPE_SINT16:
      తిరిగి *(SINT16 *)valp;
    స్థితి FFI_TYPE_UINT16:
      తిరిగి *(UINT16 *)valp;

    స్థితి FFI_TYPE_FLOAT:
      ఒకవేళ (argn < 6)
	తిరిగి lds(valp);
      /* FALLTHRU */

    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
      /* Note that unsigned 32-bit quantities are sign extended.  */
      తిరిగి *(SINT32 *)valp;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_సూచిక:
    స్థితి FFI_TYPE_DOUBLE:
      తిరిగి *(UINT64 *)valp;

    అప్రమేయ:
      abort();
    }
}

స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  గుర్తులేని దీర్ఘ *argp;
  దీర్ఘ i, avn, argn, flags = cif->flags;
  ffi_type **arg_types;
  శూన్య *చట్రం;

  /* If the return value is a struct and we don't have a return
     value address then we need to make one.  */
  ఒకవేళ (rvalue == ఖాళీ && flags == ALPHA_RET_లోపల_MEM)
    rvalue = alloca(cif->rtype->కొలత);

  /* Allocate the space for the arguments, plus 4 words of temp
     space for ffi_call_osf.  */
  argp = చట్రం = alloca(cif->బైట్లు + 4*FFI_SIZEOF_ARG);
  చట్రం += cif->బైట్లు;

  argn = 0;
  ఒకవేళ (flags == ALPHA_RET_లోపల_MEM)
    argp[argn++] = (గుర్తులేని దీర్ఘ)rvalue;

  avn = cif->nargs;
  arg_types = cif->arg_types;

  కొరకు (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      శూన్య *valp = avalue[i];
      పూర్ణాంకం type = ty->type;
      size_t కొలత;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_సూచిక:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argn++;
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Note that 128-bit long double is passed by reference.  */
	  argp[argn++] = (గుర్తులేని దీర్ఘ)valp;
	  ఆపు;

	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_VOID:
	స్థితి FFI_TYPE_STRUCT:
	  కొలత = ty->కొలత;
	  జ్ఞాపకం_నకలు(argp + argn, valp, కొలత);
	  argn += FFI_ALIGN(కొలత, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ఆపు;

	స్థితి FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ఒకవేళ (type == FFI_TYPE_LONGDOUBLE)
	    వెళ్ళు by_reference;

	  /* Most complex types passed as two separate arguments.  */
	  కొలత = ty->elements[0]->కొలత;
	  argp[argn] = extend_basic_type(valp, type, argn);
	  argp[argn + 1] = extend_basic_type(valp + కొలత, type, argn + 1);
	  argn += 2;
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
    }

  flags = (flags >> ALPHA_ST_జరుపు) & 0xff;
  ffi_call_osf(argp, చట్రం, flags, rvalue, fn, సంవృతం);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int(cif, fn, rvalue, avalue, సంవృతం);
}

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp;

  ఒకవేళ (cif->abi != FFI_OSF)
    తిరిగి FFI_BAD_ABI;

  tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  tramp[0] = 0x47fb0401;	/* mov $27,$1		*/
  tramp[1] = 0xa77b0010;	/* ldq $27,16($27)	*/
  tramp[2] = 0x6bfb0000;	/* jmp $31,($27),0	*/
  tramp[3] = 0x47ff041f;	/* nop			*/
  *(శూన్య **) &tramp[4] = ffi_సంవృతం_osf;

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  /* Flush the Icache.

     Tru64 UNIX as doesn't understand the imb mnemonic, so use call_pal
     instead, since both Compaq as and gas can handle it.

     0x86 is PAL_imb in Tru64 UNIX <alpha/pal.h>.  */
  __asm__ అస్థిర ("call_pal 0x86" : : : "memory");

  తిరిగి FFI_OK;
}

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం,
		     ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
  ఒకవేళ (cif->abi != FFI_OSF)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = (శూన్య *)ffi_go_సంవృతం_osf;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

దీర్ఘ FFI_HIDDEN
ffi_సంవృతం_osf_inner (ffi_cif *cif,
		       శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		       శూన్య *user_దత్తాంశం,
		       శూన్య *rvalue, గుర్తులేని దీర్ఘ *argp)
{
  శూన్య **avalue;
  ffi_type **arg_types;
  దీర్ఘ i, avn, argn, flags;

  avalue = alloca(cif->nargs * పరిమాణం(శూన్య *));
  flags = cif->flags;
  argn = 0;

  /* Copy the caller's structure return address to that the closure
     returns the data directly to the caller.  */
  ఒకవేళ (flags == ALPHA_RET_లోపల_MEM)
    {
      rvalue = (శూన్య *) argp[0];
      argn = 1;
    }

  arg_types = cif->arg_types;

  /* Grab the addresses of the arguments from the stack frame.  */
  కొరకు (i = 0, avn = cif->nargs; i < avn; i++)
    {
      ffi_type *ty = arg_types[i];
      పూర్ణాంకం type = ty->type;
      శూన్య *valp = &argp[argn];
      size_t కొలత;

      ఎంపిక (type)
	{
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_సూచిక:
	  argn += 1;
	  ఆపు;

	స్థితి FFI_TYPE_SINT128:
	స్థితి FFI_TYPE_UINT128:
	స్థితి FFI_TYPE_VOID:
	స్థితి FFI_TYPE_STRUCT:
	  కొలత = ty->కొలత;
	  argn += FFI_ALIGN(కొలత, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
	  ఆపు;

	స్థితి FFI_TYPE_FLOAT:
	  /* Floats coming from registers need conversion from double
	     back to float format.  */
	  ఒకవేళ (argn < 6)
	    {
	      valp = &argp[argn - 6];
	      sts(valp, argp[argn - 6]);
	    }
	  argn += 1;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (argn < 6)
	    valp = &argp[argn - 6];
	  argn += 1;
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* 128-bit long double is passed by reference.  */
	  valp = (శూన్య *)argp[argn];
	  argn += 1;
	  ఆపు;

	స్థితి FFI_TYPE_COMPLEX:
	  type = ty->elements[0]->type;
	  ఎంపిక (type)
	    {
	    స్థితి FFI_TYPE_SINT64:
	    స్థితి FFI_TYPE_UINT64:
	      /* Passed as separate arguments, but they wind up sequential.  */
	      ఆపు;

	    స్థితి FFI_TYPE_INT:
	    స్థితి FFI_TYPE_SINT8:
	    స్థితి FFI_TYPE_UINT8:
	    స్థితి FFI_TYPE_SINT16:
	    స్థితి FFI_TYPE_UINT16:
	    స్థితి FFI_TYPE_SINT32:
	    స్థితి FFI_TYPE_UINT32:
	      /* Passed as separate arguments.  Disjoint, but there's room
		 enough in one slot to hold the pair.  */
	      కొలత = ty->elements[0]->కొలత;
	      జ్ఞాపకం_నకలు(valp + కొలత, valp + 8, కొలత);
	      ఆపు;

	    స్థితి FFI_TYPE_FLOAT:
	      /* Passed as separate arguments.  Disjoint, and each piece
		 may need conversion back to float.  */
	      ఒకవేళ (argn < 6)
		{
		  valp = &argp[argn - 6];
		  sts(valp, argp[argn - 6]);
		}
	      ఒకవేళ (argn + 1 < 6)
		sts(valp + 4, argp[argn + 1 - 6]);
	      లేకపోతే
		*(UINT32 *)(valp + 4) = argp[argn + 1];
	      ఆపు;

	    స్థితి FFI_TYPE_DOUBLE:
	      /* Passed as separate arguments.  Only disjoint if one part
		 is in fp regs and the other is on the stack.  */
	      ఒకవేళ (argn < 5)
		valp = &argp[argn - 6];
	      లేకపోతే ఒకవేళ (argn == 5)
		{
		  valp = alloca(16);
		  ((UINT64 *)valp)[0] = argp[5 - 6];
		  ((UINT64 *)valp)[1] = argp[6];
		}
	      ఆపు;

	    స్థితి FFI_TYPE_LONGDOUBLE:
	      వెళ్ళు by_reference;

	    అప్రమేయ:
	      abort();
	    }
	  argn += 2;
	  ఆపు;

	అప్రమేయ:
	  abort ();
	}

      avalue[i] = valp;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_osf how to perform return type promotions.  */
  తిరిగి (flags >> ALPHA_LD_జరుపు) & 0xff;
}

/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011, 2013, 2026 Anthony Green
           Copyright (c) 1996, 2003-2004, 2007-2008 Red Hat, Inc.
   
   SPARC Foreign Function Interface 

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

#నిర్వచితం_కాకపోతే SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ఒకవేళ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ఒకవేళ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE బహిర్గతం యొక్క date
# endif
#లేకపోతే
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#అయితే_ముగింపు

/* Perform machine dependent cif processing */
ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  పూర్ణాంకం rtt = rtype->type;
  size_t బైట్లు;
  పూర్ణాంకం i, n, flags;

  /* Set the return type flag */
  ఎంపిక (rtt)
    {
    స్థితి FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ఆపు;
    స్థితి FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ఆపు;
    స్థితి FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ఆపు;
    స్థితి FFI_TYPE_LONGDOUBLE:
    స్థితి FFI_TYPE_STRUCT:
      flags = (rtype->కొలత & 0xfff) << SPARC_SIZEMASK_జరుపు;
      flags |= SPARC_RET_STRUCT;
      ఆపు;
    స్థితి FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ఆపు;
    స్థితి FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ఆపు;
    స్థితి FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ఆపు;
    స్థితి FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ఆపు;
    స్థితి FFI_TYPE_INT:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_సూచిక:
      flags = SPARC_RET_UINT32;
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      ఆపు;
    స్థితి FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      ఎంపిక (rtt)
	{
	స్థితి FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  ఆపు;
	స్థితి FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  ఆపు;
	స్థితి FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  ఆపు;
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  ఆపు;
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	స్థితి FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  ఆపు;
	స్థితి FFI_TYPE_SINT8:
	స్థితి FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  ఆపు;
	అప్రమేయ:
	  abort();
	}
      ఆపు;
    అప్రమేయ:
      abort();
    }
  cif->flags = flags;

  బైట్లు = 0;
  కొరకు (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->కొలత;
      పూర్ణాంకం tt = ty->type;

      ఎంపిక (tt)
	{
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  ఆపు;

	స్థితి FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  ఒకవేళ (tt == FFI_TYPE_FLOAT || z > 8)
	    వెళ్ళు by_reference;
	  /* FALLTHRU */

	అప్రమేయ:
	  z = FFI_ALIGN(z, 4);
	}
      బైట్లు += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  ఒకవేళ (బైట్లు < 6 * 4)
    బైట్లు = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  బైట్లు += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  బైట్లు = FFI_ALIGN(బైట్లు, 2 * 4);

  /* Include the call frame to prep_args.  */
  బైట్లు += 4*16 + 4*8;
  cif->బైట్లు = బైట్లు;

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_v8(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
			శూన్య **avalue, size_t బైట్లు, శూన్య *సంవృతం) FFI_HIDDEN;

పూర్ణాంకం FFI_HIDDEN
ffi_prep_వాదనలు_v8(ffi_cif *cif, గుర్తులేని దీర్ఘ *argp, శూన్య *rvalue, శూన్య **avalue)
{
  ffi_type **p_arg;
  పూర్ణాంకం flags = cif->flags;
  పూర్ణాంకం i, nargs;

  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ ((flags & SPARC_FLAG_RET_ముసుగు) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (అక్షరం *)argp + cif->బైట్లు;
	}
      లేకపోతే
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (గుర్తులేని దీర్ఘ)rvalue;

#నిర్వచితమైతే USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#అయితే_ముగింపు

  p_arg = cif->arg_types;
  కొరకు (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      శూన్య *a = avalue[i];
      పూర్ణాంకం tt = ty->type;
      size_t z;

      ఎంపిక (tt)
	{
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (గుర్తులేని దీర్ఘ)a;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	  జ్ఞాపకం_నకలు(argp, a, 8);
	  argp += 2;
	  ఆపు;

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_సూచిక:
	  *argp++ = *(గుర్తులేని *)a;
	  ఆపు;

	స్థితి FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ఆపు;

        స్థితి FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->కొలత;
	  ఒకవేళ (tt == FFI_TYPE_FLOAT || z > 8)
	    వెళ్ళు by_reference;
	  ఒకవేళ (z < 4)
	    {
	      జ్ఞాపకం_నకలు((అక్షరం *)argp + 4 - z, a, z);
	      argp++;
	    }
	  లేకపోతే
	    {
	      జ్ఞాపకం_నకలు(argp, a, z);
	      argp += z / 4;
	    }
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
    }

  తిరిగి flags;
}

స్థిర శూన్య
ffi_call_int (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  size_t బైట్లు = cif->బైట్లు;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  ఒకవేళ (rvalue == ఖాళీ
      && (cif->flags & SPARC_FLAG_RET_ముసుగు) == SPARC_RET_STRUCT)
    బైట్లు += FFI_ALIGN (cif->rtype->కొలత, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT)
        {
          అక్షరం *argcopy = alloca (కొలత);
          ఒకవేళ (avalue_copy == ఖాళీ)
            {
              avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
              జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
              avalue = avalue_copy;
            }
          జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -బైట్లు, సంవృతం);
}

శూన్య
ffi_call (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}

#నిర్వచితమైతే __GNUC__
స్థిర అంతఃవరుస శూన్య
ffi_flush_icache (శూన్య *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ అస్థిర ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#లేకపోతే
బాహ్య శూన్య ffi_flush_icache (శూన్య *) FFI_HIDDEN;
#అయితే_ముగింపు

బాహ్య శూన్య ffi_సంవృతం_v8(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_go_సంవృతం_v8(శూన్య) FFI_HIDDEN;

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
		      ffi_cif *cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  గుర్తులేని దీర్ఘ ctx = (గుర్తులేని దీర్ఘ) సంవృతం;
  గుర్తులేని దీర్ఘ fn = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_v8;

  ఒకవేళ (cif->abi != FFI_V8)
    తిరిగి FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  ffi_flush_icache (సంవృతం);

  తిరిగి FFI_OK;
}

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం *సంవృతం, ffi_cif *cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
  ఒకవేళ (cif->abi != FFI_V8)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = ffi_go_సంవృతం_v8;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_sparc_inner_v8(ffi_cif *cif, 
			   శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			   శూన్య *user_దత్తాంశం, శూన్య *rvalue,
			   గుర్తులేని దీర్ఘ *argp)
{
  ffi_type **arg_types;
  శూన్య **avalue;
  పూర్ణాంకం i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * పరిమాణం(శూన్య *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  ఒకవేళ ((flags & SPARC_FLAG_RET_ముసుగు) == SPARC_RET_STRUCT)
    {
      శూన్య *new_rvalue = (శూన్య *)*argp;
      *(శూన్య **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      పూర్ణాంకం tt = ty->type;
      శూన్య *a = argp;
      size_t z;

      ఎంపిక (tt)
	{
	స్థితి FFI_TYPE_STRUCT:
	స్థితి FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (శూన్య *)*argp;
	  ఆపు;

	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	  ఒకవేళ ((గుర్తులేని దీర్ఘ)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  ఆపు;

	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	స్థితి FFI_TYPE_సూచిక:
	  ఆపు;
        స్థితి FFI_TYPE_UINT16:
        స్థితి FFI_TYPE_SINT16:
	  a += 2;
	  ఆపు;
        స్థితి FFI_TYPE_UINT8:
        స్థితి FFI_TYPE_SINT8:
	  a += 3;
	  ఆపు;

        స్థితి FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->కొలత;
	  ఒకవేళ (tt == FFI_TYPE_FLOAT || z > 8)
	    వెళ్ళు by_reference;
	  ఒకవేళ (z < 4)
	    a += 4 - z;
	  లేకపోతే ఒకవేళ (z > 4)
	    argp++;
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  తిరిగి flags;
}
#అయితే_ముగింపు /* !SPARC64 */

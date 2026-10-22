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

#నిర్వచితమైతే SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

స్థిర పూర్ణాంకం
ffi_struct_float_ముసుగు (ffi_type *outer_type, పూర్ణాంకం కొలత_ముసుగు)
{
  ffi_type **elts;
  ffi_type *t;

  ఒకవేళ (outer_type->type == FFI_TYPE_COMPLEX)
    {
      పూర్ణాంకం m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->కొలత;

      ఒకవేళ (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      తిరిగి (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  కొరకు (elts = outer_type->elements; (t = *elts) != ఖాళీ; elts++)
    {
      size_t z = t->కొలత;
      పూర్ణాంకం o, m, tt;

      కొలత_ముసుగు = FFI_ALIGN(కొలత_ముసుగు, t->alignment);
      ఎంపిక (t->type)
	{
	స్థితి FFI_TYPE_STRUCT:
	  కొలత_ముసుగు = ffi_struct_float_ముసుగు (t, కొలత_ముసుగు);
	  కొనసాగు;
	స్థితి FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  ఒకవేళ (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    ఆపు;
	  /* FALLTHRU */
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (కొలత_ముసుగు >> 2) & 0x3f;	/* extract word offset */
	  కొలత_ముసుగు |= m << (o + 8);	/* insert mask into place */
	  ఆపు;
	}
      కొలత_ముసుగు += z;
    }

  కొలత_ముసుగు = FFI_ALIGN(కొలత_ముసుగు, outer_type->alignment);
  FFI_ASSERT ((కొలత_ముసుగు & 0xff) == outer_type->కొలత);

  తిరిగి కొలత_ముసుగు;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

స్థిర శూన్య *
ffi_struct_float_విలీనం (పూర్ణాంకం కొలత_ముసుగు, శూన్య *vi, శూన్య *vf)
{
  పూర్ణాంకం కొలత = కొలత_ముసుగు & 0xff;
  పూర్ణాంకం ముసుగు = కొలత_ముసుగు >> 8;
  పూర్ణాంకం n = కొలత >> 2;

  ఒకవేళ (ముసుగు == 0)
    తిరిగి vi;
  లేకపోతే ఒకవేళ (ముసుగు == (1 << n) - 1)
    తిరిగి vf;
  లేకపోతే
    {
      గుర్తులేని పూర్ణాంకం *wi = vi, *wf = vf;
      పూర్ణాంకం i;

      కొరకు (i = 0; i < n; ++i)
	ఒకవేళ ((ముసుగు >> i) & 1)
	  wi[i] = wf[i];

      తిరిగి vi;
    }
}

/* Similar, but place the data into VD in the end.  */

శూన్య FFI_HIDDEN
ffi_struct_float_copy (పూర్ణాంకం కొలత_ముసుగు, శూన్య *vd, శూన్య *vi, శూన్య *vf)
{
  పూర్ణాంకం కొలత = కొలత_ముసుగు & 0xff;
  పూర్ణాంకం ముసుగు = కొలత_ముసుగు >> 8;
  పూర్ణాంకం n = కొలత >> 2;

  ఒకవేళ (ముసుగు == 0)
    ;
  లేకపోతే ఒకవేళ (ముసుగు == (1 << n) - 1)
    vi = vf;
  లేకపోతే
    {
      గుర్తులేని పూర్ణాంకం *wd = vd, *wi = vi, *wf = vf;
      పూర్ణాంకం i;

      కొరకు (i = 0; i < n; ++i)
	wd[i] = ((ముసుగు >> i) & 1 ? wf : wi)[i];
      తిరిగి;
    }
  జ్ఞాపకం_నకలు (vd, vi, కొలత);
}

/* Perform machine dependent cif processing */

స్థిర ffi_స్థితి
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  పూర్ణాంకం rtt = rtype->type;
  size_t బైట్లు = 0;
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
      flags = SPARC_RET_F_4;
      ఆపు;

    స్థితి FFI_TYPE_COMPLEX:
    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ (rtype->కొలత > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_లోపల_MEM;
	  బైట్లు = 8;
	}
      లేకపోతే
	{
	  పూర్ణాంకం కొలత_ముసుగు = ffi_struct_float_ముసుగు (rtype, 0);
	  పూర్ణాంకం మాట_కొలత_2 = (కొలత_ముసుగు >> 2) & 0x3f;
	  పూర్ణాంకం all_ముసుగు = (1 << మాట_కొలత_2) - 1;
	  పూర్ణాంకం fp_ముసుగు = కొలత_ముసుగు >> 8;

	  flags = (కొలత_ముసుగు << SPARC_SIZEMASK_జరుపు) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  ఒకవేళ (fp_ముసుగు == 0)
	    {
	      ఒకవేళ (rtype->alignment >= 8)
		{
		  ఒకవేళ (rtype->కొలత == 8)
		    flags = SPARC_RET_INT64;
		  లేకపోతే ఒకవేళ (rtype->కొలత == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  లేకపోతే ఒకవేళ (fp_ముసుగు == all_ముసుగు)
	    ఎంపిక (మాట_కొలత_2)
	      {
	      స్థితి 1: flags = SPARC_RET_F_1; ఆపు;
	      స్థితి 2: flags = SPARC_RET_F_2; ఆపు;
	      స్థితి 3: flags = SP_V9_RET_F_3; ఆపు;
	      స్థితి 4: flags = SPARC_RET_F_4; ఆపు;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      స్థితి 6: flags = SPARC_RET_F_6; ఆపు;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      స్థితి 8: flags = SPARC_RET_F_8; ఆపు;
	      }
	}
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
      flags = SP_V9_RET_SINT32;
      ఆపు;
    స్థితి FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      ఆపు;
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_సూచిక:
      flags = SPARC_RET_INT64;
      ఆపు;

    అప్రమేయ:
      abort();
    }

  బైట్లు = 0;
  కొరకు (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->కొలత;
      size_t a = ty->alignment;

      ఎంపిక (ty->type)
	{
	స్థితి FFI_TYPE_COMPLEX:
	స్థితి FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  ఒకవేళ (z > 16)
	    {
	      a = z = 8;
	      ఆపు;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  ఒకవేళ (బైట్లు >= 16*8)
	    ఆపు;
	  ఒకవేళ ((ffi_struct_float_ముసుగు (ty, 0) & 0xff00) == 0)
	    ఆపు;
	  /* FALLTHRU */
	స్థితి FFI_TYPE_FLOAT:
	స్థితి FFI_TYPE_DOUBLE:
	స్థితి FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_వాదనలు;
	  ఆపు;
	}
      బైట్లు = FFI_ALIGN(బైట్లు, a);
      బైట్లు += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  ఒకవేళ (బైట్లు < 6 * 8)
    బైట్లు = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  బైట్లు = FFI_ALIGN(బైట్లు, 16);

  /* Include the call frame to prep_args.  */
  బైట్లు += 8*16 + 8*8;

  cif->బైట్లు = బైట్లు;
  cif->flags = flags;
  తిరిగి FFI_OK;
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  తిరిగి ffi_prep_cif_machdep_core(cif);
}

ffi_స్థితి FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, గుర్తులేని nfixedargs, గుర్తులేని ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  తిరిగి ffi_prep_cif_machdep_core(cif);
}

బాహ్య శూన్య ffi_call_v9(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
			శూన్య **avalue, size_t బైట్లు, శూన్య *సంవృతం) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

పూర్ణాంకం FFI_HIDDEN
ffi_prep_వాదనలు_v9(ffi_cif *cif, గుర్తులేని దీర్ఘ *argp, శూన్య *rvalue, శూన్య **avalue)
{
  ffi_type **p_arg;
  పూర్ణాంకం flags = cif->flags;
  పూర్ణాంకం i, nargs;

  ఒకవేళ (rvalue == ఖాళీ)
    {
      ఒకవేళ (flags & SPARC_FLAG_RET_లోపల_MEM)
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

#నిర్వచితమైతే USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#అయితే_ముగింపు

  ఒకవేళ (flags & SPARC_FLAG_RET_లోపల_MEM)
    *argp++ = (గుర్తులేని దీర్ఘ)rvalue;

  p_arg = cif->arg_types;
  కొరకు (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      శూన్య *a = avalue[i];
      size_t z;

      ఎంపిక (ty->type)
	{
	స్థితి FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_సూచిక:
	  *argp++ = *(UINT64 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_వాదనలు;
	  *argp++ = *(UINT32 *)a;
	  ఆపు;
	స్థితి FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_వాదనలు;
	  *argp++ = *(UINT64 *)a;
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	స్థితి FFI_TYPE_COMPLEX:
	స్థితి FFI_TYPE_STRUCT:
	  z = ty->కొలత;
	  ఒకవేళ (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (గుర్తులేని దీర్ఘ)a;
	      ఆపు;
	    }
	  ఒకవేళ (((గుర్తులేని దీర్ఘ)argp & 15) && ty->alignment > 8)
	    argp++;
	  జ్ఞాపకం_నకలు(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
    }

  తిరిగి flags;
}

స్థిర శూన్య
ffi_call_int(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  size_t బైట్లు = cif->బైట్లు;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;

  FFI_ASSERT (cif->abi == FFI_V9);

  ఒకవేళ (rvalue == ఖాళీ && (cif->flags & SPARC_FLAG_RET_లోపల_MEM))
    బైట్లు += FFI_ALIGN (cif->rtype->కొలత, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT && కొలత > 4)
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
  
  ffi_call_v9(cif, fn, rvalue, avalue, -బైట్లు, సంవృతం);
}

శూన్య
ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	    శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int(cif, fn, rvalue, avalue, సంవృతం);
}

#నిర్వచితమైతే __GNUC__
స్థిర అంతఃవరుస శూన్య
ffi_flush_icache (శూన్య *p)
{
  __asm__ అస్థిర ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#లేకపోతే
బాహ్య శూన్య ffi_flush_icache (శూన్య *) FFI_HIDDEN;
#అయితే_ముగింపు

బాహ్య శూన్య ffi_సంవృతం_v9(శూన్య) FFI_HIDDEN;
బాహ్య శూన్య ffi_go_సంవృతం_v9(శూన్య) FFI_HIDDEN;

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
		      ffi_cif* cif,
		      శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  గుర్తులేని దీర్ఘ fn;

  ఒకవేళ (cif->abi != FFI_V9)
    తిరిగి FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((గుర్తులేని దీర్ఘ *) &tramp[4]) = fn;

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

  ffi_flush_icache (సంవృతం);

  తిరిగి FFI_OK;
}

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*))
{
  ఒకవేళ (cif->abi != FFI_V9)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->tramp = ffi_go_సంవృతం_v9;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

పూర్ణాంకం FFI_HIDDEN
ffi_సంవృతం_sparc_inner_v9(ffi_cif *cif,
			   శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			   శూన్య *user_దత్తాంశం, శూన్య *rvalue,
			   గుర్తులేని దీర్ఘ *gpr, గుర్తులేని దీర్ఘ *fpr)
{
  ffi_type **arg_types;
  శూన్య **avalue;
  పూర్ణాంకం i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * పరిమాణం(శూన్య *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  ఒకవేళ (flags & SPARC_FLAG_RET_లోపల_MEM)
    {
      rvalue = (శూన్య *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  లేకపోతే
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  కొరకు (i = 0; i < nargs; i++, argn = argx)
    {
      పూర్ణాంకం named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      శూన్య *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      ఎంపిక (ty->type)
	{
	స్థితి FFI_TYPE_COMPLEX:
	స్థితి FFI_TYPE_STRUCT:
	  z = ty->కొలత;
	  ఒకవేళ (z > 16)
	    a = *(శూన్య **)a;
	  లేకపోతే
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      ఒకవేళ (named && argn < 16)
		{
		  పూర్ణాంకం కొలత_ముసుగు = ffi_struct_float_ముసుగు (ty, 0);
		  పూర్ణాంకం argn_ముసుగు = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  కొలత_ముసుగు = (కొలత_ముసుగు & 0xff) | (కొలత_ముసుగు & argn_ముసుగు);
		  a = ffi_struct_float_విలీనం (కొలత_ముసుగు, gpr+argn, fpr+argn);
		}
	    }
	  ఆపు;

	స్థితి FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  ఆపు;
	స్థితి FFI_TYPE_DOUBLE:
	  ఒకవేళ (named && argn < 16)
	    a = fpr + argn;
	  ఆపు;
	స్థితి FFI_TYPE_FLOAT:
	  ఒకవేళ (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  ఆపు;

	స్థితి FFI_TYPE_UINT64:
	స్థితి FFI_TYPE_SINT64:
	స్థితి FFI_TYPE_సూచిక:
	  ఆపు;
	స్థితి FFI_TYPE_INT:
	స్థితి FFI_TYPE_UINT32:
	స్థితి FFI_TYPE_SINT32:
	  a += 4;
	  ఆపు;
        స్థితి FFI_TYPE_UINT16:
        స్థితి FFI_TYPE_SINT16:
	  a += 6;
	  ఆపు;
        స్థితి FFI_TYPE_UINT8:
        స్థితి FFI_TYPE_SINT8:
	  a += 7;
	  ఆపు;

	అప్రమేయ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_దత్తాంశం);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  తిరిగి flags;
}
#అయితే_ముగింపు /* SPARC64 */

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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>
#شامل "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# اگر FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE برآمد کا date
# endif
#ورنہ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#اگر_ختم

#اگر_تعریف_شدہ SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ساکن صحیح
ffi_struct_float_نقاب (ffi_type *outer_type, صحیح سائز_نقاب)
{
  ffi_type **elts;
  ffi_type *t;

  اگر (outer_type->type == FFI_TYPE_COMPLEX)
    {
      صحیح m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->سائز;

      اگر (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      واپس (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  برائے (elts = outer_type->elements; (t = *elts) != تہی; elts++)
    {
      size_t z = t->سائز;
      صحیح o, m, tt;

      سائز_نقاب = FFI_ALIGN(سائز_نقاب, t->alignment);
      انتخاب (t->type)
	{
	حالت FFI_TYPE_STRUCT:
	  سائز_نقاب = ffi_struct_float_نقاب (t, سائز_نقاب);
	  جاری;
	حالت FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  اگر (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    توڑو;
	  /* FALLTHRU */
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (سائز_نقاب >> 2) & 0x3f;	/* extract word offset */
	  سائز_نقاب |= m << (o + 8);	/* insert mask into place */
	  توڑو;
	}
      سائز_نقاب += z;
    }

  سائز_نقاب = FFI_ALIGN(سائز_نقاب, outer_type->alignment);
  FFI_ASSERT ((سائز_نقاب & 0xff) == outer_type->سائز);

  واپس سائز_نقاب;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ساکن خالی *
ffi_struct_float_ملاؤ (صحیح سائز_نقاب, خالی *vi, خالی *vf)
{
  صحیح سائز = سائز_نقاب & 0xff;
  صحیح نقاب = سائز_نقاب >> 8;
  صحیح n = سائز >> 2;

  اگر (نقاب == 0)
    واپس vi;
  ورنہ اگر (نقاب == (1 << n) - 1)
    واپس vf;
  ورنہ
    {
      غیرعلامتی صحیح *wi = vi, *wf = vf;
      صحیح i;

      برائے (i = 0; i < n; ++i)
	اگر ((نقاب >> i) & 1)
	  wi[i] = wf[i];

      واپس vi;
    }
}

/* Similar, but place the data into VD in the end.  */

خالی FFI_HIDDEN
ffi_struct_float_copy (صحیح سائز_نقاب, خالی *vd, خالی *vi, خالی *vf)
{
  صحیح سائز = سائز_نقاب & 0xff;
  صحیح نقاب = سائز_نقاب >> 8;
  صحیح n = سائز >> 2;

  اگر (نقاب == 0)
    ;
  ورنہ اگر (نقاب == (1 << n) - 1)
    vi = vf;
  ورنہ
    {
      غیرعلامتی صحیح *wd = vd, *wi = vi, *wf = vf;
      صحیح i;

      برائے (i = 0; i < n; ++i)
	wd[i] = ((نقاب >> i) & 1 ? wf : wi)[i];
      واپس;
    }
  حافظہ_نقل (vd, vi, سائز);
}

/* Perform machine dependent cif processing */

ساکن ffi_حالت
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحیح rtt = rtype->type;
  size_t بائٹیں = 0;
  صحیح i, n, flags;

  /* Set the return type flag */
  انتخاب (rtt)
    {
    حالت FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      توڑو;
    حالت FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      توڑو;
    حالت FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      توڑو;
    حالت FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      توڑو;

    حالت FFI_TYPE_COMPLEX:
    حالت FFI_TYPE_STRUCT:
      اگر (rtype->سائز > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_اندر_MEM;
	  بائٹیں = 8;
	}
      ورنہ
	{
	  صحیح سائز_نقاب = ffi_struct_float_نقاب (rtype, 0);
	  صحیح لفظ_سائز_2 = (سائز_نقاب >> 2) & 0x3f;
	  صحیح all_نقاب = (1 << لفظ_سائز_2) - 1;
	  صحیح fp_نقاب = سائز_نقاب >> 8;

	  flags = (سائز_نقاب << SPARC_SIZEMASK_سرکاؤ) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  اگر (fp_نقاب == 0)
	    {
	      اگر (rtype->alignment >= 8)
		{
		  اگر (rtype->سائز == 8)
		    flags = SPARC_RET_INT64;
		  ورنہ اگر (rtype->سائز == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  ورنہ اگر (fp_نقاب == all_نقاب)
	    انتخاب (لفظ_سائز_2)
	      {
	      حالت 1: flags = SPARC_RET_F_1; توڑو;
	      حالت 2: flags = SPARC_RET_F_2; توڑو;
	      حالت 3: flags = SP_V9_RET_F_3; توڑو;
	      حالت 4: flags = SPARC_RET_F_4; توڑو;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      حالت 6: flags = SPARC_RET_F_6; توڑو;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      حالت 8: flags = SPARC_RET_F_8; توڑو;
	      }
	}
      توڑو;

    حالت FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      توڑو;
    حالت FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      توڑو;
    حالت FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      توڑو;
    حالت FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      توڑو;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      توڑو;
    حالت FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_اشارہ_گر:
      flags = SPARC_RET_INT64;
      توڑو;

    طےشدہ:
      abort();
    }

  بائٹیں = 0;
  برائے (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->سائز;
      size_t a = ty->alignment;

      انتخاب (ty->type)
	{
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  اگر (z > 16)
	    {
	      a = z = 8;
	      توڑو;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  اگر (بائٹیں >= 16*8)
	    توڑو;
	  اگر ((ffi_struct_float_نقاب (ty, 0) & 0xff00) == 0)
	    توڑو;
	  /* FALLTHRU */
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_دلائل;
	  توڑو;
	}
      بائٹیں = FFI_ALIGN(بائٹیں, a);
      بائٹیں += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  اگر (بائٹیں < 6 * 8)
    بائٹیں = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  بائٹیں = FFI_ALIGN(بائٹیں, 16);

  /* Include the call frame to prep_args.  */
  بائٹیں += 8*16 + 8*8;

  cif->بائٹیں = بائٹیں;
  cif->flags = flags;
  واپس FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  واپس ffi_prep_cif_machdep_core(cif);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, غیرعلامتی nfixedargs, غیرعلامتی ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  واپس ffi_prep_cif_machdep_core(cif);
}

خارجی خالی ffi_call_v9(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
			خالی **avalue, size_t بائٹیں, خالی *احاطہ) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

صحیح FFI_HIDDEN
ffi_prep_دلائل_v9(ffi_cif *cif, غیرعلامتی طویل *argp, خالی *rvalue, خالی **avalue)
{
  ffi_type **p_arg;
  صحیح flags = cif->flags;
  صحیح i, nargs;

  اگر (rvalue == تہی)
    {
      اگر (flags & SPARC_FLAG_RET_اندر_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (حرف *)argp + cif->بائٹیں;
	}
      ورنہ
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#اگر_تعریف_شدہ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#اگر_ختم

  اگر (flags & SPARC_FLAG_RET_اندر_MEM)
    *argp++ = (غیرعلامتی طویل)rvalue;

  p_arg = cif->arg_types;
  برائے (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      خالی *a = avalue[i];
      size_t z;

      انتخاب (ty->type)
	{
	حالت FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  توڑو;
	حالت FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  توڑو;
	حالت FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  توڑو;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  توڑو;
	حالت FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  توڑو;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  *argp++ = *(UINT64 *)a;
	  توڑو;
	حالت FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_دلائل;
	  *argp++ = *(UINT32 *)a;
	  توڑو;
	حالت FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_دلائل;
	  *argp++ = *(UINT64 *)a;
	  توڑو;

	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  z = ty->سائز;
	  اگر (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (غیرعلامتی طویل)a;
	      توڑو;
	    }
	  اگر (((غیرعلامتی طویل)argp & 15) && ty->alignment > 8)
	    argp++;
	  حافظہ_نقل(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  توڑو;

	طےشدہ:
	  abort();
	}
    }

  واپس flags;
}

ساکن خالی
ffi_call_int(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  size_t بائٹیں = cif->بائٹیں;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;

  FFI_ASSERT (cif->abi == FFI_V9);

  اگر (rvalue == تہی && (cif->flags & SPARC_FLAG_RET_اندر_MEM))
    بائٹیں += FFI_ALIGN (cif->rtype->سائز, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      اگر (at->type == FFI_TYPE_STRUCT && سائز > 4)
        {
          حرف *argcopy = alloca (سائز);
          اگر (avalue_copy == تہی)
            {
              avalue_copy = alloca (nargs * حجم (خالی *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (خالی *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], سائز);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -بائٹیں, احاطہ);
}

خالی
ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	    خالی **avalue, خالی *احاطہ)
{
  ffi_call_int(cif, fn, rvalue, avalue, احاطہ);
}

#اگر_تعریف_شدہ __GNUC__
ساکن اندرونی خالی
ffi_flush_icache (خالی *p)
{
  __asm__ متلون ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#ورنہ
خارجی خالی ffi_flush_icache (خالی *) FFI_HIDDEN;
#اگر_ختم

خارجی خالی ffi_احاطہ_v9(خالی) FFI_HIDDEN;
خارجی خالی ffi_go_احاطہ_v9(خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  غیرعلامتی طویل fn;

  اگر (cif->abi != FFI_V9)
    واپس FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (غیرعلامتی طویل) ffi_احاطہ_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((غیرعلامتی طویل *) &tramp[4]) = fn;

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  ffi_flush_icache (احاطہ);

  واپس FFI_OK;
}

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ, ffi_cif* cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
  اگر (cif->abi != FFI_V9)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = ffi_go_احاطہ_v9;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

صحیح FFI_HIDDEN
ffi_احاطہ_sparc_inner_v9(ffi_cif *cif,
			   خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			   خالی *user_ڈیٹا, خالی *rvalue,
			   غیرعلامتی طویل *gpr, غیرعلامتی طویل *fpr)
{
  ffi_type **arg_types;
  خالی **avalue;
  صحیح i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * حجم(خالی *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  اگر (flags & SPARC_FLAG_RET_اندر_MEM)
    {
      rvalue = (خالی *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  ورنہ
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  برائے (i = 0; i < nargs; i++, argn = argx)
    {
      صحیح named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      خالی *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      انتخاب (ty->type)
	{
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  z = ty->سائز;
	  اگر (z > 16)
	    a = *(خالی **)a;
	  ورنہ
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      اگر (named && argn < 16)
		{
		  صحیح سائز_نقاب = ffi_struct_float_نقاب (ty, 0);
		  صحیح argn_نقاب = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  سائز_نقاب = (سائز_نقاب & 0xff) | (سائز_نقاب & argn_نقاب);
		  a = ffi_struct_float_ملاؤ (سائز_نقاب, gpr+argn, fpr+argn);
		}
	    }
	  توڑو;

	حالت FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  توڑو;
	حالت FFI_TYPE_DOUBLE:
	  اگر (named && argn < 16)
	    a = fpr + argn;
	  توڑو;
	حالت FFI_TYPE_FLOAT:
	  اگر (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  توڑو;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_اشارہ_گر:
	  توڑو;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	  a += 4;
	  توڑو;
        حالت FFI_TYPE_UINT16:
        حالت FFI_TYPE_SINT16:
	  a += 6;
	  توڑو;
        حالت FFI_TYPE_UINT8:
        حالت FFI_TYPE_SINT8:
	  a += 7;
	  توڑو;

	طےشدہ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  واپس flags;
}
#اگر_ختم /* SPARC64 */

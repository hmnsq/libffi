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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>
#ضمن "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ٻاهريون جو date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ساكن صحيح
ffi_struct_float_نقاب (ffi_type *outer_type, صحيح سائيز_نقاب)
{
  ffi_type **elts;
  ffi_type *t;

  إذا (outer_type->type == FFI_TYPE_COMPLEX)
    {
      صحيح m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->سائيز;

      إذا (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      أرجع (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  لكل (elts = outer_type->elements; (t = *elts) != عدم; elts++)
    {
      size_t z = t->سائيز;
      صحيح o, m, tt;

      سائيز_نقاب = FFI_ALIGN(سائيز_نقاب, t->alignment);
      بدل (t->type)
	{
	حالة FFI_TYPE_STRUCT:
	  سائيز_نقاب = ffi_struct_float_نقاب (t, سائيز_نقاب);
	  استمر;
	حالة FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  إذا (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    اكسر;
	  /* FALLTHRU */
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (سائيز_نقاب >> 2) & 0x3f;	/* extract word offset */
	  سائيز_نقاب |= m << (o + 8);	/* insert mask into place */
	  اكسر;
	}
      سائيز_نقاب += z;
    }

  سائيز_نقاب = FFI_ALIGN(سائيز_نقاب, outer_type->alignment);
  FFI_ASSERT ((سائيز_نقاب & 0xff) == outer_type->سائيز);

  أرجع سائيز_نقاب;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ساكن فراغ *
ffi_struct_float_ملايو (صحيح سائيز_نقاب, فراغ *vi, فراغ *vf)
{
  صحيح سائيز = سائيز_نقاب & 0xff;
  صحيح نقاب = سائيز_نقاب >> 8;
  صحيح n = سائيز >> 2;

  إذا (نقاب == 0)
    أرجع vi;
  وإلا إذا (نقاب == (1 << n) - 1)
    أرجع vf;
  وإلا
    {
      بنانشان صحيح *wi = vi, *wf = vf;
      صحيح i;

      لكل (i = 0; i < n; ++i)
	إذا ((نقاب >> i) & 1)
	  wi[i] = wf[i];

      أرجع vi;
    }
}

/* Similar, but place the data into VD in the end.  */

فراغ FFI_HIDDEN
ffi_struct_float_copy (صحيح سائيز_نقاب, فراغ *vd, فراغ *vi, فراغ *vf)
{
  صحيح سائيز = سائيز_نقاب & 0xff;
  صحيح نقاب = سائيز_نقاب >> 8;
  صحيح n = سائيز >> 2;

  إذا (نقاب == 0)
    ;
  وإلا إذا (نقاب == (1 << n) - 1)
    vi = vf;
  وإلا
    {
      بنانشان صحيح *wd = vd, *wi = vi, *wf = vf;
      صحيح i;

      لكل (i = 0; i < n; ++i)
	wd[i] = ((نقاب >> i) & 1 ? wf : wi)[i];
      أرجع;
    }
  يادگيري_نقل (vd, vi, سائيز);
}

/* Perform machine dependent cif processing */

ساكن ffi_حالت
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحيح rtt = rtype->type;
  size_t بائيٽون = 0;
  صحيح i, n, flags;

  /* Set the return type flag */
  بدل (rtt)
    {
    حالة FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      اكسر;
    حالة FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      اكسر;
    حالة FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      اكسر;
    حالة FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      اكسر;

    حالة FFI_TYPE_COMPLEX:
    حالة FFI_TYPE_STRUCT:
      إذا (rtype->سائيز > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_اندريون_MEM;
	  بائيٽون = 8;
	}
      وإلا
	{
	  صحيح سائيز_نقاب = ffi_struct_float_نقاب (rtype, 0);
	  صحيح لفظ_سائيز_2 = (سائيز_نقاب >> 2) & 0x3f;
	  صحيح all_نقاب = (1 << لفظ_سائيز_2) - 1;
	  صحيح fp_نقاب = سائيز_نقاب >> 8;

	  flags = (سائيز_نقاب << SPARC_SIZEMASK_سرڪايو) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  إذا (fp_نقاب == 0)
	    {
	      إذا (rtype->alignment >= 8)
		{
		  إذا (rtype->سائيز == 8)
		    flags = SPARC_RET_INT64;
		  وإلا إذا (rtype->سائيز == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  وإلا إذا (fp_نقاب == all_نقاب)
	    بدل (لفظ_سائيز_2)
	      {
	      حالة 1: flags = SPARC_RET_F_1; اكسر;
	      حالة 2: flags = SPARC_RET_F_2; اكسر;
	      حالة 3: flags = SP_V9_RET_F_3; اكسر;
	      حالة 4: flags = SPARC_RET_F_4; اكسر;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      حالة 6: flags = SPARC_RET_F_6; اكسر;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      حالة 8: flags = SPARC_RET_F_8; اكسر;
	      }
	}
      اكسر;

    حالة FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      اكسر;
    حالة FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      اكسر;
    حالة FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      اكسر;
    حالة FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      اكسر;
    حالة FFI_TYPE_INT:
    حالة FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      اكسر;
    حالة FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_اشارو:
      flags = SPARC_RET_INT64;
      اكسر;

    افتراضي:
      abort();
    }

  بائيٽون = 0;
  لكل (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->سائيز;
      size_t a = ty->alignment;

      بدل (ty->type)
	{
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  إذا (z > 16)
	    {
	      a = z = 8;
	      اكسر;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  إذا (بائيٽون >= 16*8)
	    اكسر;
	  إذا ((ffi_struct_float_نقاب (ty, 0) & 0xff00) == 0)
	    اكسر;
	  /* FALLTHRU */
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_دليلون;
	  اكسر;
	}
      بائيٽون = FFI_ALIGN(بائيٽون, a);
      بائيٽون += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  إذا (بائيٽون < 6 * 8)
    بائيٽون = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  بائيٽون = FFI_ALIGN(بائيٽون, 16);

  /* Include the call frame to prep_args.  */
  بائيٽون += 8*16 + 8*8;

  cif->بائيٽون = بائيٽون;
  cif->flags = flags;
  أرجع FFI_OK;
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, بنانشان nfixedargs, بنانشان ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

خارجي فراغ ffi_call_v9(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
			فراغ **avalue, size_t بائيٽون, فراغ *گهيرو) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

صحيح FFI_HIDDEN
ffi_prep_دليلون_v9(ffi_cif *cif, بنانشان ڊگهو *argp, فراغ *rvalue, فراغ **avalue)
{
  ffi_type **p_arg;
  صحيح flags = cif->flags;
  صحيح i, nargs;

  إذا (rvalue == عدم)
    {
      إذا (flags & SPARC_FLAG_RET_اندريون_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (حرف *)argp + cif->بائيٽون;
	}
      وإلا
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#جيڪڏهن_وصف_ٿيل USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#جيڪڏهن_پورو

  إذا (flags & SPARC_FLAG_RET_اندريون_MEM)
    *argp++ = (بنانشان ڊگهو)rvalue;

  p_arg = cif->arg_types;
  لكل (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      فراغ *a = avalue[i];
      size_t z;

      بدل (ty->type)
	{
	حالة FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  اكسر;
	حالة FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  اكسر;
	حالة FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  اكسر;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  اكسر;
	حالة FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_اشارو:
	  *argp++ = *(UINT64 *)a;
	  اكسر;
	حالة FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_دليلون;
	  *argp++ = *(UINT32 *)a;
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_دليلون;
	  *argp++ = *(UINT64 *)a;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_STRUCT:
	  z = ty->سائيز;
	  إذا (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (بنانشان ڊگهو)a;
	      اكسر;
	    }
	  إذا (((بنانشان ڊگهو)argp & 15) && ty->alignment > 8)
	    argp++;
	  يادگيري_نقل(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  اكسر;

	افتراضي:
	  abort();
	}
    }

  أرجع flags;
}

ساكن فراغ
ffi_call_int(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  size_t بائيٽون = cif->بائيٽون;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;

  FFI_ASSERT (cif->abi == FFI_V9);

  إذا (rvalue == عدم && (cif->flags & SPARC_FLAG_RET_اندريون_MEM))
    بائيٽون += FFI_ALIGN (cif->rtype->سائيز, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح سائيز = at->سائيز;
      إذا (at->type == FFI_TYPE_STRUCT && سائيز > 4)
        {
          حرف *argcopy = alloca (سائيز);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              يادگيري_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          يادگيري_نقل (argcopy, avalue[i], سائيز);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -بائيٽون, گهيرو);
}

فراغ
ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	    فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int(cif, fn, rvalue, avalue, گهيرو);
}

#جيڪڏهن_وصف_ٿيل __GNUC__
ساكن مضمن فراغ
ffi_flush_icache (فراغ *p)
{
  __asm__ متطاير ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#وإلا
خارجي فراغ ffi_flush_icache (فراغ *) FFI_HIDDEN;
#جيڪڏهن_پورو

خارجي فراغ ffi_گهيرو_v9(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_گهيرو_v9(فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بنانشان صحيح *tramp = (بنانشان صحيح *) &گهيرو->tramp[0];
  بنانشان ڊگهو fn;

  إذا (cif->abi != FFI_V9)
    أرجع FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (بنانشان ڊگهو) ffi_گهيرو_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((بنانشان ڊگهو *) &tramp[4]) = fn;

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  ffi_flush_icache (گهيرو);

  أرجع FFI_OK;
}

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو* گهيرو, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  إذا (cif->abi != FFI_V9)
    أرجع FFI_BAD_ABI;

  گهيرو->tramp = ffi_go_گهيرو_v9;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

صحيح FFI_HIDDEN
ffi_گهيرو_sparc_inner_v9(ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_معلومات, فراغ *rvalue,
			   بنانشان ڊگهو *gpr, بنانشان ڊگهو *fpr)
{
  ffi_type **arg_types;
  فراغ **avalue;
  صحيح i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * حجم(فراغ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  إذا (flags & SPARC_FLAG_RET_اندريون_MEM)
    {
      rvalue = (فراغ *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  وإلا
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0; i < nargs; i++, argn = argx)
    {
      صحيح named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      فراغ *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      بدل (ty->type)
	{
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_STRUCT:
	  z = ty->سائيز;
	  إذا (z > 16)
	    a = *(فراغ **)a;
	  وإلا
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      إذا (named && argn < 16)
		{
		  صحيح سائيز_نقاب = ffi_struct_float_نقاب (ty, 0);
		  صحيح argn_نقاب = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  سائيز_نقاب = (سائيز_نقاب & 0xff) | (سائيز_نقاب & argn_نقاب);
		  a = ffi_struct_float_ملايو (سائيز_نقاب, gpr+argn, fpr+argn);
		}
	    }
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  إذا (named && argn < 16)
	    a = fpr + argn;
	  اكسر;
	حالة FFI_TYPE_FLOAT:
	  إذا (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_اشارو:
	  اكسر;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	  a += 4;
	  اكسر;
        حالة FFI_TYPE_UINT16:
        حالة FFI_TYPE_SINT16:
	  a += 6;
	  اكسر;
        حالة FFI_TYPE_UINT8:
        حالة FFI_TYPE_SINT8:
	  a += 7;
	  اكسر;

	افتراضي:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_معلومات);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  أرجع flags;
}
#جيڪڏهن_پورو /* SPARC64 */

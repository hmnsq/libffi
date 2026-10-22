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
#  error FFI_TYPE_LONGDOUBLE بیرون از date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#پایان_اگر

#اگر_تعریف_شده SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ساكن صحيح
ffi_struct_float_ماسک (ffi_type *outer_type, صحيح اندازه_ماسک)
{
  ffi_type **elts;
  ffi_type *t;

  إذا (outer_type->type == FFI_TYPE_COMPLEX)
    {
      صحيح m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->اندازه;

      إذا (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      أرجع (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  لكل (elts = outer_type->elements; (t = *elts) != عدم; elts++)
    {
      size_t z = t->اندازه;
      صحيح o, m, tt;

      اندازه_ماسک = FFI_ALIGN(اندازه_ماسک, t->alignment);
      بدل (t->type)
	{
	حالة FFI_TYPE_STRUCT:
	  اندازه_ماسک = ffi_struct_float_ماسک (t, اندازه_ماسک);
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
	  o = (اندازه_ماسک >> 2) & 0x3f;	/* extract word offset */
	  اندازه_ماسک |= m << (o + 8);	/* insert mask into place */
	  اكسر;
	}
      اندازه_ماسک += z;
    }

  اندازه_ماسک = FFI_ALIGN(اندازه_ماسک, outer_type->alignment);
  FFI_ASSERT ((اندازه_ماسک & 0xff) == outer_type->اندازه);

  أرجع اندازه_ماسک;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ساكن فراغ *
ffi_struct_float_ادغام (صحيح اندازه_ماسک, فراغ *vi, فراغ *vf)
{
  صحيح اندازه = اندازه_ماسک & 0xff;
  صحيح ماسک = اندازه_ماسک >> 8;
  صحيح n = اندازه >> 2;

  إذا (ماسک == 0)
    أرجع vi;
  وإلا إذا (ماسک == (1 << n) - 1)
    أرجع vf;
  وإلا
    {
      بیعلامت صحيح *wi = vi, *wf = vf;
      صحيح i;

      لكل (i = 0; i < n; ++i)
	إذا ((ماسک >> i) & 1)
	  wi[i] = wf[i];

      أرجع vi;
    }
}

/* Similar, but place the data into VD in the end.  */

فراغ FFI_HIDDEN
ffi_struct_float_copy (صحيح اندازه_ماسک, فراغ *vd, فراغ *vi, فراغ *vf)
{
  صحيح اندازه = اندازه_ماسک & 0xff;
  صحيح ماسک = اندازه_ماسک >> 8;
  صحيح n = اندازه >> 2;

  إذا (ماسک == 0)
    ;
  وإلا إذا (ماسک == (1 << n) - 1)
    vi = vf;
  وإلا
    {
      بیعلامت صحيح *wd = vd, *wi = vi, *wf = vf;
      صحيح i;

      لكل (i = 0; i < n; ++i)
	wd[i] = ((ماسک >> i) & 1 ? wf : wi)[i];
      أرجع;
    }
  کپی_حافظه (vd, vi, اندازه);
}

/* Perform machine dependent cif processing */

ساكن ffi_وضعیت
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحيح rtt = rtype->type;
  size_t بایتها = 0;
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
      إذا (rtype->اندازه > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_درون_MEM;
	  بایتها = 8;
	}
      وإلا
	{
	  صحيح اندازه_ماسک = ffi_struct_float_ماسک (rtype, 0);
	  صحيح کلمه_اندازه_2 = (اندازه_ماسک >> 2) & 0x3f;
	  صحيح all_ماسک = (1 << کلمه_اندازه_2) - 1;
	  صحيح fp_ماسک = اندازه_ماسک >> 8;

	  flags = (اندازه_ماسک << SPARC_SIZEMASK_لغزش) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  إذا (fp_ماسک == 0)
	    {
	      إذا (rtype->alignment >= 8)
		{
		  إذا (rtype->اندازه == 8)
		    flags = SPARC_RET_INT64;
		  وإلا إذا (rtype->اندازه == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  وإلا إذا (fp_ماسک == all_ماسک)
	    بدل (کلمه_اندازه_2)
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
    حالة FFI_TYPE_اشاره_گر:
      flags = SPARC_RET_INT64;
      اكسر;

    افتراضي:
      abort();
    }

  بایتها = 0;
  لكل (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->اندازه;
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
	  إذا (بایتها >= 16*8)
	    اكسر;
	  إذا ((ffi_struct_float_ماسک (ty, 0) & 0xff00) == 0)
	    اكسر;
	  /* FALLTHRU */
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_آرگومانها;
	  اكسر;
	}
      بایتها = FFI_ALIGN(بایتها, a);
      بایتها += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  إذا (بایتها < 6 * 8)
    بایتها = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  بایتها = FFI_ALIGN(بایتها, 16);

  /* Include the call frame to prep_args.  */
  بایتها += 8*16 + 8*8;

  cif->بایتها = بایتها;
  cif->flags = flags;
  أرجع FFI_OK;
}

ffi_وضعیت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

ffi_وضعیت FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, بیعلامت nfixedargs, بیعلامت ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  أرجع ffi_prep_cif_machdep_core(cif);
}

خارجي فراغ ffi_call_v9(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
			فراغ **avalue, size_t بایتها, فراغ *بستگی) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

صحيح FFI_HIDDEN
ffi_prep_آرگومانها_v9(ffi_cif *cif, بیعلامت دراز *argp, فراغ *rvalue, فراغ **avalue)
{
  ffi_type **p_arg;
  صحيح flags = cif->flags;
  صحيح i, nargs;

  إذا (rvalue == عدم)
    {
      إذا (flags & SPARC_FLAG_RET_درون_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (حرف *)argp + cif->بایتها;
	}
      وإلا
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#اگر_تعریف_شده USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#پایان_اگر

  إذا (flags & SPARC_FLAG_RET_درون_MEM)
    *argp++ = (بیعلامت دراز)rvalue;

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
	حالة FFI_TYPE_اشاره_گر:
	  *argp++ = *(UINT64 *)a;
	  اكسر;
	حالة FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_آرگومانها;
	  *argp++ = *(UINT32 *)a;
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_آرگومانها;
	  *argp++ = *(UINT64 *)a;
	  اكسر;

	حالة FFI_TYPE_LONGDOUBLE:
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_STRUCT:
	  z = ty->اندازه;
	  إذا (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (بیعلامت دراز)a;
	      اكسر;
	    }
	  إذا (((بیعلامت دراز)argp & 15) && ty->alignment > 8)
	    argp++;
	  کپی_حافظه(argp, a, z);
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
	     فراغ **avalue, فراغ *بستگی)
{
  size_t بایتها = cif->بایتها;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;

  FFI_ASSERT (cif->abi == FFI_V9);

  إذا (rvalue == عدم && (cif->flags & SPARC_FLAG_RET_درون_MEM))
    بایتها += FFI_ALIGN (cif->rtype->اندازه, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح اندازه = at->اندازه;
      إذا (at->type == FFI_TYPE_STRUCT && اندازه > 4)
        {
          حرف *argcopy = alloca (اندازه);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              کپی_حافظه (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          کپی_حافظه (argcopy, avalue[i], اندازه);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -بایتها, بستگی);
}

فراغ
ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	    فراغ **avalue, فراغ *بستگی)
{
  ffi_call_int(cif, fn, rvalue, avalue, بستگی);
}

#اگر_تعریف_شده __GNUC__
ساكن مضمن فراغ
ffi_flush_icache (فراغ *p)
{
  __asm__ متطاير ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#وإلا
خارجي فراغ ffi_flush_icache (فراغ *) FFI_HIDDEN;
#پایان_اگر

خارجي فراغ ffi_بستگی_v9(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_بستگی_v9(فراغ) FFI_HIDDEN;

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_داده,
		      فراغ *codeloc)
{
  بیعلامت صحيح *tramp = (بیعلامت صحيح *) &بستگی->tramp[0];
  بیعلامت دراز fn;

  إذا (cif->abi != FFI_V9)
    أرجع FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (بیعلامت دراز) ffi_بستگی_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((بیعلامت دراز *) &tramp[4]) = fn;

  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;

  ffi_flush_icache (بستگی);

  أرجع FFI_OK;
}

ffi_وضعیت
ffi_prep_go_بستگی (ffi_go_بستگی* بستگی, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  إذا (cif->abi != FFI_V9)
    أرجع FFI_BAD_ABI;

  بستگی->tramp = ffi_go_بستگی_v9;
  بستگی->cif = cif;
  بستگی->fun = fun;

  أرجع FFI_OK;
}

صحيح FFI_HIDDEN
ffi_بستگی_sparc_inner_v9(ffi_cif *cif,
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_داده, فراغ *rvalue,
			   بیعلامت دراز *gpr, بیعلامت دراز *fpr)
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
  إذا (flags & SPARC_FLAG_RET_درون_MEM)
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
	  z = ty->اندازه;
	  إذا (z > 16)
	    a = *(فراغ **)a;
	  وإلا
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      إذا (named && argn < 16)
		{
		  صحيح اندازه_ماسک = ffi_struct_float_ماسک (ty, 0);
		  صحيح argn_ماسک = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  اندازه_ماسک = (اندازه_ماسک & 0xff) | (اندازه_ماسک & argn_ماسک);
		  a = ffi_struct_float_ادغام (اندازه_ماسک, gpr+argn, fpr+argn);
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
	حالة FFI_TYPE_اشاره_گر:
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
  fun (cif, rvalue, avalue, user_داده);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  أرجع flags;
}
#پایان_اگر /* SPARC64 */

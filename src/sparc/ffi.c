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

#که_نه_تعریف_شوی SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# إذا FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE بهر د date
# endif
#وإلا
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#د_که_پای

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحيح rtt = rtype->type;
  size_t بایټونه;
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
    حالة FFI_TYPE_STRUCT:
      flags = (rtype->اندازه & 0xfff) << SPARC_SIZEMASK_ښويدنه;
      flags |= SPARC_RET_STRUCT;
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
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_اشاره_ګر:
      flags = SPARC_RET_UINT32;
      اكسر;
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      اكسر;
    حالة FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      بدل (rtt)
	{
	حالة FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  اكسر;
	حالة FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  اكسر;
	حالة FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  اكسر;
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  اكسر;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  اكسر;
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  اكسر;
	افتراضي:
	  abort();
	}
      اكسر;
    افتراضي:
      abort();
    }
  cif->flags = flags;

  بایټونه = 0;
  لكل (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->اندازه;
      صحيح tt = ty->type;

      بدل (tt)
	{
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  اكسر;

	حالة FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  إذا (tt == FFI_TYPE_FLOAT || z > 8)
	    اذهب by_reference;
	  /* FALLTHRU */

	افتراضي:
	  z = FFI_ALIGN(z, 4);
	}
      بایټونه += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  إذا (بایټونه < 6 * 4)
    بایټونه = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  بایټونه += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  بایټونه = FFI_ALIGN(بایټونه, 2 * 4);

  /* Include the call frame to prep_args.  */
  بایټونه += 4*16 + 4*8;
  cif->بایټونه = بایټونه;

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_v8(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
			فراغ **avalue, size_t بایټونه, فراغ *تړون) FFI_HIDDEN;

صحيح FFI_HIDDEN
ffi_prep_دلیلونه_v8(ffi_cif *cif, بینښه اوږد *argp, فراغ *rvalue, فراغ **avalue)
{
  ffi_type **p_arg;
  صحيح flags = cif->flags;
  صحيح i, nargs;

  إذا (rvalue == عدم)
    {
      إذا ((flags & SPARC_FLAG_RET_ماسک) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (حرف *)argp + cif->بایټونه;
	}
      وإلا
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (بینښه اوږد)rvalue;

#که_تعریف_شوی USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#د_که_پای

  p_arg = cif->arg_types;
  لكل (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      فراغ *a = avalue[i];
      صحيح tt = ty->type;
      size_t z;

      بدل (tt)
	{
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (بینښه اوږد)a;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
	  حافظه_نقل(argp, a, 8);
	  argp += 2;
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_اشاره_ګر:
	  *argp++ = *(بینښه *)a;
	  اكسر;

	حالة FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  اكسر;
	حالة FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  اكسر;
	حالة FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  اكسر;

        حالة FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->اندازه;
	  إذا (tt == FFI_TYPE_FLOAT || z > 8)
	    اذهب by_reference;
	  إذا (z < 4)
	    {
	      حافظه_نقل((حرف *)argp + 4 - z, a, z);
	      argp++;
	    }
	  وإلا
	    {
	      حافظه_نقل(argp, a, z);
	      argp += z / 4;
	    }
	  اكسر;

	افتراضي:
	  abort();
	}
    }

  أرجع flags;
}

ساكن فراغ
ffi_call_int (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	      فراغ **avalue, فراغ *تړون)
{
  size_t بایټونه = cif->بایټونه;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  إذا (rvalue == عدم
      && (cif->flags & SPARC_FLAG_RET_ماسک) == SPARC_RET_STRUCT)
    بایټونه += FFI_ALIGN (cif->rtype->اندازه, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح اندازه = at->اندازه;
      إذا (at->type == FFI_TYPE_STRUCT)
        {
          حرف *argcopy = alloca (اندازه);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              حافظه_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          حافظه_نقل (argcopy, avalue[i], اندازه);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -بایټونه, تړون);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *تړون)
{
  ffi_call_int (cif, fn, rvalue, avalue, تړون);
}

#که_تعریف_شوی __GNUC__
ساكن مضمن فراغ
ffi_flush_icache (فراغ *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ متطاير ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#وإلا
خارجي فراغ ffi_flush_icache (فراغ *) FFI_HIDDEN;
#د_که_پای

خارجي فراغ ffi_تړون_v8(فراغ) FFI_HIDDEN;
خارجي فراغ ffi_go_تړون_v8(فراغ) FFI_HIDDEN;

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون *تړون,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بینښه صحيح *tramp = (بینښه صحيح *) &تړون->tramp[0];
  بینښه اوږد ctx = (بینښه اوږد) تړون;
  بینښه اوږد fn = (بینښه اوږد) ffi_تړون_v8;

  إذا (cif->abi != FFI_V8)
    أرجع FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  ffi_flush_icache (تړون);

  أرجع FFI_OK;
}

ffi_حالت
ffi_prep_go_تړون (ffi_go_تړون *تړون, ffi_cif *cif,
		     فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*))
{
  إذا (cif->abi != FFI_V8)
    أرجع FFI_BAD_ABI;

  تړون->tramp = ffi_go_تړون_v8;
  تړون->cif = cif;
  تړون->fun = fun;

  أرجع FFI_OK;
}

صحيح FFI_HIDDEN
ffi_تړون_sparc_inner_v8(ffi_cif *cif, 
			   فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			   فراغ *user_معلومات, فراغ *rvalue,
			   بینښه اوږد *argp)
{
  ffi_type **arg_types;
  فراغ **avalue;
  صحيح i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * حجم(فراغ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  إذا ((flags & SPARC_FLAG_RET_ماسک) == SPARC_RET_STRUCT)
    {
      فراغ *new_rvalue = (فراغ *)*argp;
      *(فراغ **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      صحيح tt = ty->type;
      فراغ *a = argp;
      size_t z;

      بدل (tt)
	{
	حالة FFI_TYPE_STRUCT:
	حالة FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (فراغ *)*argp;
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	حالة FFI_TYPE_SINT64:
	حالة FFI_TYPE_UINT64:
	  إذا ((بینښه اوږد)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  اكسر;

	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_اشاره_ګر:
	  اكسر;
        حالة FFI_TYPE_UINT16:
        حالة FFI_TYPE_SINT16:
	  a += 2;
	  اكسر;
        حالة FFI_TYPE_UINT8:
        حالة FFI_TYPE_SINT8:
	  a += 3;
	  اكسر;

        حالة FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->اندازه;
	  إذا (tt == FFI_TYPE_FLOAT || z > 8)
	    اذهب by_reference;
	  إذا (z < 4)
	    a += 4 - z;
	  وإلا إذا (z > 4)
	    argp++;
	  اكسر;

	افتراضي:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_معلومات);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  أرجع flags;
}
#د_که_پای /* !SPARC64 */

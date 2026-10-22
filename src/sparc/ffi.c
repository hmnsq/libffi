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

#اگر_غیر_تعریف_شدہ SPARC64

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

/* Perform machine dependent cif processing */
ffi_حالت FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحیح rtt = rtype->type;
  size_t بائٹیں;
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
    حالت FFI_TYPE_STRUCT:
      flags = (rtype->سائز & 0xfff) << SPARC_SIZEMASK_سرکاؤ;
      flags |= SPARC_RET_STRUCT;
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
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_اشارہ_گر:
      flags = SPARC_RET_UINT32;
      توڑو;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      توڑو;
    حالت FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      انتخاب (rtt)
	{
	حالت FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  توڑو;
	حالت FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  توڑو;
	حالت FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  توڑو;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  توڑو;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  توڑو;
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  توڑو;
	طےشدہ:
	  abort();
	}
      توڑو;
    طےشدہ:
      abort();
    }
  cif->flags = flags;

  بائٹیں = 0;
  برائے (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->سائز;
      صحیح tt = ty->type;

      انتخاب (tt)
	{
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  توڑو;

	حالت FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  اگر (tt == FFI_TYPE_FLOAT || z > 8)
	    جاؤ by_reference;
	  /* FALLTHRU */

	طےشدہ:
	  z = FFI_ALIGN(z, 4);
	}
      بائٹیں += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  اگر (بائٹیں < 6 * 4)
    بائٹیں = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  بائٹیں += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  بائٹیں = FFI_ALIGN(بائٹیں, 2 * 4);

  /* Include the call frame to prep_args.  */
  بائٹیں += 4*16 + 4*8;
  cif->بائٹیں = بائٹیں;

  واپس FFI_OK;
}

خارجی خالی ffi_call_v8(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
			خالی **avalue, size_t بائٹیں, خالی *احاطہ) FFI_HIDDEN;

صحیح FFI_HIDDEN
ffi_prep_دلائل_v8(ffi_cif *cif, غیرعلامتی طویل *argp, خالی *rvalue, خالی **avalue)
{
  ffi_type **p_arg;
  صحیح flags = cif->flags;
  صحیح i, nargs;

  اگر (rvalue == تہی)
    {
      اگر ((flags & SPARC_FLAG_RET_نقاب) == SPARC_RET_STRUCT)
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

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (غیرعلامتی طویل)rvalue;

#اگر_تعریف_شدہ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#اگر_ختم

  p_arg = cif->arg_types;
  برائے (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      خالی *a = avalue[i];
      صحیح tt = ty->type;
      size_t z;

      انتخاب (tt)
	{
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (غیرعلامتی طویل)a;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	  حافظہ_نقل(argp, a, 8);
	  argp += 2;
	  توڑو;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_اشارہ_گر:
	  *argp++ = *(غیرعلامتی *)a;
	  توڑو;

	حالت FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  توڑو;
	حالت FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  توڑو;
	حالت FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  توڑو;

        حالت FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->سائز;
	  اگر (tt == FFI_TYPE_FLOAT || z > 8)
	    جاؤ by_reference;
	  اگر (z < 4)
	    {
	      حافظہ_نقل((حرف *)argp + 4 - z, a, z);
	      argp++;
	    }
	  ورنہ
	    {
	      حافظہ_نقل(argp, a, z);
	      argp += z / 4;
	    }
	  توڑو;

	طےشدہ:
	  abort();
	}
    }

  واپس flags;
}

ساکن خالی
ffi_call_int (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	      خالی **avalue, خالی *احاطہ)
{
  size_t بائٹیں = cif->بائٹیں;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  اگر (rvalue == تہی
      && (cif->flags & SPARC_FLAG_RET_نقاب) == SPARC_RET_STRUCT)
    بائٹیں += FFI_ALIGN (cif->rtype->سائز, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      اگر (at->type == FFI_TYPE_STRUCT)
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

  ffi_call_v8(cif, fn, rvalue, avalue, -بائٹیں, احاطہ);
}

خالی
ffi_call (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}

#اگر_تعریف_شدہ __GNUC__
ساکن اندرونی خالی
ffi_flush_icache (خالی *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ متلون ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#ورنہ
خارجی خالی ffi_flush_icache (خالی *) FFI_HIDDEN;
#اگر_ختم

خارجی خالی ffi_احاطہ_v8(خالی) FFI_HIDDEN;
خارجی خالی ffi_go_احاطہ_v8(خالی) FFI_HIDDEN;

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
		      ffi_cif *cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  غیرعلامتی طویل ctx = (غیرعلامتی طویل) احاطہ;
  غیرعلامتی طویل fn = (غیرعلامتی طویل) ffi_احاطہ_v8;

  اگر (cif->abi != FFI_V8)
    واپس FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  ffi_flush_icache (احاطہ);

  واپس FFI_OK;
}

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ *احاطہ, ffi_cif *cif,
		     خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*))
{
  اگر (cif->abi != FFI_V8)
    واپس FFI_BAD_ABI;

  احاطہ->tramp = ffi_go_احاطہ_v8;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

صحیح FFI_HIDDEN
ffi_احاطہ_sparc_inner_v8(ffi_cif *cif, 
			   خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			   خالی *user_ڈیٹا, خالی *rvalue,
			   غیرعلامتی طویل *argp)
{
  ffi_type **arg_types;
  خالی **avalue;
  صحیح i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * حجم(خالی *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  اگر ((flags & SPARC_FLAG_RET_نقاب) == SPARC_RET_STRUCT)
    {
      خالی *new_rvalue = (خالی *)*argp;
      *(خالی **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      صحیح tt = ty->type;
      خالی *a = argp;
      size_t z;

      انتخاب (tt)
	{
	حالت FFI_TYPE_STRUCT:
	حالت FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (خالی *)*argp;
	  توڑو;

	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	  اگر ((غیرعلامتی طویل)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  توڑو;

	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_اشارہ_گر:
	  توڑو;
        حالت FFI_TYPE_UINT16:
        حالت FFI_TYPE_SINT16:
	  a += 2;
	  توڑو;
        حالت FFI_TYPE_UINT8:
        حالت FFI_TYPE_SINT8:
	  a += 3;
	  توڑو;

        حالت FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->سائز;
	  اگر (tt == FFI_TYPE_FLOAT || z > 8)
	    جاؤ by_reference;
	  اگر (z < 4)
	    a += 4 - z;
	  ورنہ اگر (z > 4)
	    argp++;
	  توڑو;

	طےشدہ:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ڈیٹا);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  واپس flags;
}
#اگر_ختم /* !SPARC64 */

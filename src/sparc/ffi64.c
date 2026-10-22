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
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#وگرنه
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

#ifdef SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ایستا صحیح
ffi_struct_float_mask (ffi_type *outer_type, صحیح size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  اگر (outer_type->type == FFI_TYPE_COMPLEX)
    {
      صحیح m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      اگر (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      بازگشت (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  برای (elts = outer_type->elements; (t = *elts) != پوچ; elts++)
    {
      size_t z = t->size;
      صحیح o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      انتخاب (t->type)
	{
	حالت FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  ادامه;
	حالت FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  اگر (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    بشکن;
	  /* FALLTHRU */
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  بشکن;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  بازگشت size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ایستا تهی *
ffi_struct_float_merge (صحیح size_mask, تهی *vi, تهی *vf)
{
  صحیح size = size_mask & 0xff;
  صحیح mask = size_mask >> 8;
  صحیح n = size >> 2;

  اگر (mask == 0)
    بازگشت vi;
  وگرنه اگر (mask == (1 << n) - 1)
    بازگشت vf;
  وگرنه
    {
      unsigned صحیح *wi = vi, *wf = vf;
      صحیح i;

      برای (i = 0; i < n; ++i)
	اگر ((mask >> i) & 1)
	  wi[i] = wf[i];

      بازگشت vi;
    }
}

/* Similar, but place the data into VD in the end.  */

تهی FFI_HIDDEN
ffi_struct_float_copy (صحیح size_mask, تهی *vd, تهی *vi, تهی *vf)
{
  صحیح size = size_mask & 0xff;
  صحیح mask = size_mask >> 8;
  صحیح n = size >> 2;

  اگر (mask == 0)
    ;
  وگرنه اگر (mask == (1 << n) - 1)
    vi = vf;
  وگرنه
    {
      unsigned صحیح *wd = vd, *wi = vi, *wf = vf;
      صحیح i;

      برای (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      بازگشت;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

ایستا ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  صحیح rtt = rtype->type;
  size_t bytes = 0;
  صحیح i, n, flags;

  /* Set the return type flag */
  انتخاب (rtt)
    {
    حالت FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      بشکن;
    حالت FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      بشکن;
    حالت FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      بشکن;
    حالت FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      بشکن;

    حالت FFI_TYPE_COMPLEX:
    حالت FFI_TYPE_STRUCT:
      اگر (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      وگرنه
	{
	  صحیح size_mask = ffi_struct_float_mask (rtype, 0);
	  صحیح word_size = (size_mask >> 2) & 0x3f;
	  صحیح all_mask = (1 << word_size) - 1;
	  صحیح fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  اگر (fp_mask == 0)
	    {
	      اگر (rtype->alignment >= 8)
		{
		  اگر (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  وگرنه اگر (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  وگرنه اگر (fp_mask == all_mask)
	    انتخاب (word_size)
	      {
	      حالت 1: flags = SPARC_RET_F_1; بشکن;
	      حالت 2: flags = SPARC_RET_F_2; بشکن;
	      حالت 3: flags = SP_V9_RET_F_3; بشکن;
	      حالت 4: flags = SPARC_RET_F_4; بشکن;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      حالت 6: flags = SPARC_RET_F_6; بشکن;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      حالت 8: flags = SPARC_RET_F_8; بشکن;
	      }
	}
      بشکن;

    حالت FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      بشکن;
    حالت FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      بشکن;
    حالت FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      بشکن;
    حالت FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      بشکن;
    حالت FFI_TYPE_INT:
    حالت FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      بشکن;
    حالت FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      بشکن;
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      بشکن;

    پیشفرض:
      abort();
    }

  bytes = 0;
  برای (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      انتخاب (ty->type)
	{
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  اگر (z > 16)
	    {
	      a = z = 8;
	      بشکن;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  اگر (bytes >= 16*8)
	    بشکن;
	  اگر ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    بشکن;
	  /* FALLTHRU */
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_DOUBLE:
	حالت FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  بشکن;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  اگر (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  بازگشت FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  بازگشت ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  بازگشت ffi_prep_cif_machdep_core(cif);
}

بیرونی تهی ffi_call_v9(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
			تهی **avalue, size_t bytes, تهی *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

صحیح FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, تهی *rvalue, تهی **avalue)
{
  ffi_type **p_arg;
  صحیح flags = cif->flags;
  صحیح i, nargs;

  اگر (rvalue == پوچ)
    {
      اگر (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (نویسه *)argp + cif->bytes;
	}
      وگرنه
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#ifdef USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#endif

  اگر (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  برای (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      تهی *a = avalue[i];
      size_t z;

      انتخاب (ty->type)
	{
	حالت FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  بشکن;
	حالت FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  بشکن;
	حالت FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  بشکن;
	حالت FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  بشکن;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  بشکن;
	حالت FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  بشکن;
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  بشکن;
	حالت FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  بشکن;
	حالت FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  z = ty->size;
	  اگر (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      بشکن;
	    }
	  اگر (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  بشکن;

	پیشفرض:
	  abort();
	}
    }

  بازگشت flags;
}

ایستا تهی
ffi_call_int(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue_copy = پوچ;

  FFI_ASSERT (cif->abi == FFI_V9);

  اگر (rvalue == پوچ && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برای (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح size = at->size;
      اگر (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          نویسه *argcopy = alloca (size);
          اگر (avalue_copy == پوچ)
            {
              avalue_copy = alloca (nargs * اندازه (تهی *));
              memcpy (avalue_copy, avalue, nargs * اندازه (تهی *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

تهی
ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	    تهی **avalue, تهی *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
ایستا درونخطی تهی
ffi_flush_icache (تهی *p)
{
  __asm__ فرار ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#وگرنه
بیرونی تهی ffi_flush_icache (تهی *) FFI_HIDDEN;
#endif

بیرونی تهی ffi_closure_v9(تهی) FFI_HIDDEN;
بیرونی تهی ffi_go_closure_v9(تهی) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp = (unsigned صحیح *) &closure->tramp[0];
  unsigned long fn;

  اگر (cif->abi != FFI_V9)
    بازگشت FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (unsigned long) ffi_closure_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((unsigned long *) &tramp[4]) = fn;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ffi_flush_icache (closure);

  بازگشت FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*))
{
  اگر (cif->abi != FFI_V9)
    بازگشت FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

صحیح FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
			   تهی *user_data, تهی *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  تهی **avalue;
  صحیح i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * اندازه(تهی *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  اگر (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (تهی *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  وگرنه
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  برای (i = 0; i < nargs; i++, argn = argx)
    {
      صحیح named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      تهی *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      انتخاب (ty->type)
	{
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_STRUCT:
	  z = ty->size;
	  اگر (z > 16)
	    a = *(تهی **)a;
	  وگرنه
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      اگر (named && argn < 16)
		{
		  صحیح size_mask = ffi_struct_float_mask (ty, 0);
		  صحیح argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  بشکن;

	حالت FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  بشکن;
	حالت FFI_TYPE_DOUBLE:
	  اگر (named && argn < 16)
	    a = fpr + argn;
	  بشکن;
	حالت FFI_TYPE_FLOAT:
	  اگر (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  بشکن;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
	حالت FFI_TYPE_POINTER:
	  بشکن;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_SINT32:
	  a += 4;
	  بشکن;
        حالت FFI_TYPE_UINT16:
        حالت FFI_TYPE_SINT16:
	  a += 6;
	  بشکن;
        حالت FFI_TYPE_UINT8:
        حالت FFI_TYPE_SINT8:
	  a += 7;
	  بشکن;

	پیشفرض:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  بازگشت flags;
}
#endif /* SPARC64 */

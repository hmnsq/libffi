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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ਜੇ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
#ਨਹੀਂਤਾਂ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ਜੇ_ਅੰਤ

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ਪੂਰਨ rtt = rtype->type;
  size_t ਬਾਈਟਾਂ;
  ਪੂਰਨ i, n, flags;

  /* Set the return type flag */
  ਚੋਣ (rtt)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
    ਹਾਲਤ FFI_TYPE_STRUCT:
      flags = (rtype->ਮਾਪ & 0xfff) << SPARC_SIZEMASK_ਖਿਸਕਾਓ;
      flags |= SPARC_RET_STRUCT;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = SPARC_RET_UINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      ਚੋਣ (rtt)
	{
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  ਰੋਕੋ;
	ਮੂਲ:
	  abort();
	}
      ਰੋਕੋ;
    ਮੂਲ:
      abort();
    }
  cif->flags = flags;

  ਬਾਈਟਾਂ = 0;
  ਲਈ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->ਮਾਪ;
      ਪੂਰਨ tt = ty->type;

      ਚੋਣ (tt)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  ਜੇ (tt == FFI_TYPE_FLOAT || z > 8)
	    ਜਾਓ by_reference;
	  /* FALLTHRU */

	ਮੂਲ:
	  z = FFI_ALIGN(z, 4);
	}
      ਬਾਈਟਾਂ += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  ਜੇ (ਬਾਈਟਾਂ < 6 * 4)
    ਬਾਈਟਾਂ = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  ਬਾਈਟਾਂ += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, 2 * 4);

  /* Include the call frame to prep_args.  */
  ਬਾਈਟਾਂ += 4*16 + 4*8;
  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_v8(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
			ਖਾਲੀ **avalue, size_t ਬਾਈਟਾਂ, ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

ਪੂਰਨ FFI_HIDDEN
ffi_prep_ਦਲੀਲਾਂ_v8(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *argp, ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_type **p_arg;
  ਪੂਰਨ flags = cif->flags;
  ਪੂਰਨ i, nargs;

  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ ((flags & SPARC_FLAG_RET_ਨਕਾਬ) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (ਅੱਖਰ *)argp + cif->ਬਾਈਟਾਂ;
	}
      ਨਹੀਂਤਾਂ
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)rvalue;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#ਜੇ_ਅੰਤ

  p_arg = cif->arg_types;
  ਲਈ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ਖਾਲੀ *a = avalue[i];
      ਪੂਰਨ tt = ty->type;
      size_t z;

      ਚੋਣ (tt)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)a;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, a, 8);
	  argp += 2;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  *argp++ = *(ਬਿਨਾਨਿਸ਼ਾਨੀ *)a;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->ਮਾਪ;
	  ਜੇ (tt == FFI_TYPE_FLOAT || z > 8)
	    ਜਾਓ by_reference;
	  ਜੇ (z < 4)
	    {
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ((ਅੱਖਰ *)argp + 4 - z, a, z);
	      argp++;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, a, z);
	      argp += z / 4;
	    }
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
    }

  ਵਾਪਸ flags;
}

ਸਥਿਰ ਖਾਲੀ
ffi_call_int (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  size_t ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  ਜੇ (rvalue == ਨਿਰਾ
      && (cif->flags & SPARC_FLAG_RET_ਨਕਾਬ) == SPARC_RET_STRUCT)
    ਬਾਈਟਾਂ += FFI_ALIGN (cif->rtype->ਮਾਪ, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT)
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -ਬਾਈਟਾਂ, ਘੇਰਾ);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GNUC__
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ffi_flush_icache (ਖਾਲੀ *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ ਅਸਥਿਰ ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#ਨਹੀਂਤਾਂ
ਬਾਹਰੀ ਖਾਲੀ ffi_flush_icache (ਖਾਲੀ *) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_v8(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_v8(ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ctx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਘੇਰਾ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_v8;

  ਜੇ (cif->abi != FFI_V8)
    ਵਾਪਸ FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ffi_flush_icache (ਘੇਰਾ);

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
  ਜੇ (cif->abi != FFI_V8)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_v8;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_sparc_inner_v8(ffi_cif *cif, 
			   ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			   ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *rvalue,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *argp)
{
  ffi_type **arg_types;
  ਖਾਲੀ **avalue;
  ਪੂਰਨ i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * ਆਕਾਰ(ਖਾਲੀ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  ਜੇ ((flags & SPARC_FLAG_RET_ਨਕਾਬ) == SPARC_RET_STRUCT)
    {
      ਖਾਲੀ *new_rvalue = (ਖਾਲੀ *)*argp;
      *(ਖਾਲੀ **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      ਪੂਰਨ tt = ty->type;
      ਖਾਲੀ *a = argp;
      size_t z;

      ਚੋਣ (tt)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (ਖਾਲੀ *)*argp;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT16:
        ਹਾਲਤ FFI_TYPE_SINT16:
	  a += 2;
	  ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT8:
        ਹਾਲਤ FFI_TYPE_SINT8:
	  a += 3;
	  ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->ਮਾਪ;
	  ਜੇ (tt == FFI_TYPE_FLOAT || z > 8)
	    ਜਾਓ by_reference;
	  ਜੇ (z < 4)
	    a += 4 - z;
	  ਨਹੀਂਤਾਂ ਜੇ (z > 4)
	    argp++;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ਵਾਪਸ flags;
}
#ਜੇ_ਅੰਤ /* !SPARC64 */

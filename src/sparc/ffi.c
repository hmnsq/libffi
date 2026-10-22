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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ "internal.h"

#ifndef SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ଯଦି FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ଯଦି FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#ଅନ୍ୟଥା
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rtt = rtype->type;
  size_t bytes;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, n, flags;

  /* Set the return type flag */
  ବାଛ (rtt)
    {
    ସ୍ଥିତି FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      flags = (rtype->size & 0xfff) << SPARC_SIZEMASK_SHIFT;
      flags |= SPARC_RET_STRUCT;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_INT:
    ସ୍ଥିତି FFI_TYPE_SINT32:
    ସ୍ଥିତି FFI_TYPE_UINT32:
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = SPARC_RET_UINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      ବାଛ (rtt)
	{
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }
  cif->flags = flags;

  bytes = 0;
  ପାଇଁ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt = ty->type;

      ବାଛ (tt)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  ଯଦି (tt == FFI_TYPE_FLOAT || z > 8)
	    ଯାଅ by_reference;
	  /* FALLTHRU */

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  z = FFI_ALIGN(z, 4);
	}
      bytes += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  ଯଦି (bytes < 6 * 4)
    bytes = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  bytes += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 2 * 4);

  /* Include the call frame to prep_args.  */
  bytes += 4*16 + 4*8;
  cif->bytes = bytes;

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_v8(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
			ଶୂନ୍ୟ **avalue, size_t bytes, ଶୂନ୍ୟ *closure) FFI_HIDDEN;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_prep_args_v8(ffi_cif *cif, unsigned long *argp, ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_type **p_arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = cif->flags;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs;

  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (ଅକ୍ଷର *)argp + cif->bytes;
	}
      ଅନ୍ୟଥା
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (unsigned long)rvalue;

#ifdef USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#endif

  p_arg = cif->arg_types;
  ପାଇଁ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ଶୂନ୍ୟ *a = avalue[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt = ty->type;
      size_t z;

      ବାଛ (tt)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (unsigned long)a;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	  memcpy(argp, a, 8);
	  argp += 2;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  *argp++ = *(unsigned *)a;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  ଯଦି (tt == FFI_TYPE_FLOAT || z > 8)
	    ଯାଅ by_reference;
	  ଯଦି (z < 4)
	    {
	      memcpy((ଅକ୍ଷର *)argp + 4 - z, a, z);
	      argp++;
	    }
	  ଅନ୍ୟଥା
	    {
	      memcpy(argp, a, z);
	      argp += z / 4;
	    }
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}
    }

  ଫେରନ୍ତୁ flags;
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  ଯଦି (rvalue == ଖାଲି
      && (cif->flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    bytes += FFI_ALIGN (cif->rtype->size, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      ଯଦି (at->type == FFI_TYPE_STRUCT)
        {
          ଅକ୍ଷର *argcopy = alloca (size);
          ଯଦି (avalue_copy == ଖାଲି)
            {
              avalue_copy = alloca (nargs * ଆକାର (ଶୂନ୍ୟ *));
              memcpy (avalue_copy, avalue, nargs * ଆକାର (ଶୂନ୍ୟ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -bytes, closure);
}

ଶୂନ୍ୟ
ffi_call (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
ffi_flush_icache (ଶୂନ୍ୟ *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ ଅସ୍ଥିର ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#ଅନ୍ୟଥା
ବାହ୍ୟ ଶୂନ୍ୟ ffi_flush_icache (ଶୂନ୍ୟ *) FFI_HIDDEN;
#endif

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_v8(ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_v8(ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
  unsigned long ctx = (unsigned long) closure;
  unsigned long fn = (unsigned long) ffi_closure_v8;

  ଯଦି (cif->abi != FFI_V8)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ffi_flush_icache (closure);

  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*))
{
  ଯଦି (cif->abi != FFI_V8)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v8;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_sparc_inner_v8(ffi_cif *cif, 
			   ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			   ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *rvalue,
			   unsigned long *argp)
{
  ffi_type **arg_types;
  ଶୂନ୍ୟ **avalue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * ଆକାର(ଶୂନ୍ୟ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  ଯଦି ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    {
      ଶୂନ୍ୟ *new_rvalue = (ଶୂନ୍ୟ *)*argp;
      *(ଶୂନ୍ୟ **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା tt = ty->type;
      ଶୂନ୍ୟ *a = argp;
      size_t z;

      ବାଛ (tt)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (ଶୂନ୍ୟ *)*argp;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	  ଯଦି ((unsigned long)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT16:
        ସ୍ଥିତି FFI_TYPE_SINT16:
	  a += 2;
	  ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT8:
        ସ୍ଥିତି FFI_TYPE_SINT8:
	  a += 3;
	  ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  ଯଦି (tt == FFI_TYPE_FLOAT || z > 8)
	    ଯାଅ by_reference;
	  ଯଦି (z < 4)
	    a += 4 - z;
	  ଅନ୍ୟଥା ଯଦି (z > 4)
	    argp++;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ଫେରନ୍ତୁ flags;
}
#endif /* !SPARC64 */

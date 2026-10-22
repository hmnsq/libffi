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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ "internal.h"

#ifndef SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#જો FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# જો FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#નહીંતર
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  પૂર્ણાંક rtt = rtype->type;
  size_t bytes;
  પૂર્ણાંક i, n, flags;

  /* Set the return type flag */
  પસંદગી (rtt)
    {
    સ્થિતિ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      તોડો;
    સ્થિતિ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      તોડો;
    સ્થિતિ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      તોડો;
    સ્થિતિ FFI_TYPE_LONGDOUBLE:
    સ્થિતિ FFI_TYPE_STRUCT:
      flags = (rtype->size & 0xfff) << SPARC_SIZEMASK_SHIFT;
      flags |= SPARC_RET_STRUCT;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      તોડો;
    સ્થિતિ FFI_TYPE_INT:
    સ્થિતિ FFI_TYPE_SINT32:
    સ્થિતિ FFI_TYPE_UINT32:
    સ્થિતિ FFI_TYPE_POINTER:
      flags = SPARC_RET_UINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      તોડો;
    સ્થિતિ FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      પસંદગી (rtt)
	{
	સ્થિતિ FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  તોડો;
	સ્થિતિ FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  તોડો;
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  તોડો;
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	સ્થિતિ FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT8:
	સ્થિતિ FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  તોડો;
	મૂળભૂત:
	  abort();
	}
      તોડો;
    મૂળભૂત:
      abort();
    }
  cif->flags = flags;

  bytes = 0;
  માટે (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      પૂર્ણાંક tt = ty->type;

      પસંદગી (tt)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  તોડો;

	સ્થિતિ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  જો (tt == FFI_TYPE_FLOAT || z > 8)
	    જાઓ by_reference;
	  /* FALLTHRU */

	મૂળભૂત:
	  z = FFI_ALIGN(z, 4);
	}
      bytes += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  જો (bytes < 6 * 4)
    bytes = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  bytes += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 2 * 4);

  /* Include the call frame to prep_args.  */
  bytes += 4*16 + 4*8;
  cif->bytes = bytes;

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_v8(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
			શૂન્ય **avalue, size_t bytes, શૂન્ય *closure) FFI_HIDDEN;

પૂર્ણાંક FFI_HIDDEN
ffi_prep_args_v8(ffi_cif *cif, unsigned long *argp, શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_type **p_arg;
  પૂર્ણાંક flags = cif->flags;
  પૂર્ણાંક i, nargs;

  જો (rvalue == ખાલી)
    {
      જો ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (અક્ષર *)argp + cif->bytes;
	}
      નહીંતર
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
  માટે (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      શૂન્ય *a = avalue[i];
      પૂર્ણાંક tt = ty->type;
      size_t z;

      પસંદગી (tt)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (unsigned long)a;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	  memcpy(argp, a, 8);
	  argp += 2;
	  તોડો;

	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_POINTER:
	  *argp++ = *(unsigned *)a;
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  તોડો;

        સ્થિતિ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  જો (tt == FFI_TYPE_FLOAT || z > 8)
	    જાઓ by_reference;
	  જો (z < 4)
	    {
	      memcpy((અક્ષર *)argp + 4 - z, a, z);
	      argp++;
	    }
	  નહીંતર
	    {
	      memcpy(argp, a, z);
	      argp += z / 4;
	    }
	  તોડો;

	મૂળભૂત:
	  abort();
	}
    }

  પાછા flags;
}

સ્થિર શૂન્ય
ffi_call_int (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  જો (rvalue == ખાલી
      && (cif->flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    bytes += FFI_ALIGN (cif->rtype->size, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      જો (at->type == FFI_TYPE_STRUCT)
        {
          અક્ષર *argcopy = alloca (size);
          જો (avalue_copy == ખાલી)
            {
              avalue_copy = alloca (nargs * કદ (શૂન્ય *));
              memcpy (avalue_copy, avalue, nargs * કદ (શૂન્ય *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -bytes, closure);
}

શૂન્ય
ffi_call (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
સ્થિર અંતઃરેખ શૂન્ય
ffi_flush_icache (શૂન્ય *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ અસ્થિર ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#નહીંતર
બાહ્ય શૂન્ય ffi_flush_icache (શૂન્ય *) FFI_HIDDEN;
#endif

બાહ્ય શૂન્ય ffi_closure_v8(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_go_closure_v8(શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  unsigned long ctx = (unsigned long) closure;
  unsigned long fn = (unsigned long) ffi_closure_v8;

  જો (cif->abi != FFI_V8)
    પાછા FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ffi_flush_icache (closure);

  પાછા FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
  જો (cif->abi != FFI_V8)
    પાછા FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v8;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

પૂર્ણાંક FFI_HIDDEN
ffi_closure_sparc_inner_v8(ffi_cif *cif, 
			   શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			   શૂન્ય *user_data, શૂન્ય *rvalue,
			   unsigned long *argp)
{
  ffi_type **arg_types;
  શૂન્ય **avalue;
  પૂર્ણાંક i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * કદ(શૂન્ય *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  જો ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    {
      શૂન્ય *new_rvalue = (શૂન્ય *)*argp;
      *(શૂન્ય **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      પૂર્ણાંક tt = ty->type;
      શૂન્ય *a = argp;
      size_t z;

      પસંદગી (tt)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (શૂન્ય *)*argp;
	  તોડો;

	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	  જો ((unsigned long)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  તોડો;

	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	સ્થિતિ FFI_TYPE_POINTER:
	  તોડો;
        સ્થિતિ FFI_TYPE_UINT16:
        સ્થિતિ FFI_TYPE_SINT16:
	  a += 2;
	  તોડો;
        સ્થિતિ FFI_TYPE_UINT8:
        સ્થિતિ FFI_TYPE_SINT8:
	  a += 3;
	  તોડો;

        સ્થિતિ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  જો (tt == FFI_TYPE_FLOAT || z > 8)
	    જાઓ by_reference;
	  જો (z < 4)
	    a += 4 - z;
	  નહીંતર જો (z > 4)
	    argp++;
	  તોડો;

	મૂળભૂત:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  પાછા flags;
}
#endif /* !SPARC64 */

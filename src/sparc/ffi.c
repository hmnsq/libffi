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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব "internal.h"

#ifndef SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# যদি FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#অন্যথা
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  পূর্ণসংখ্যা rtt = rtype->type;
  size_t bytes;
  পূর্ণসংখ্যা i, n, flags;

  /* Set the return type flag */
  নির্বাচন (rtt)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      বিরতি;
    ক্ষেত্র FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      বিরতি;
    ক্ষেত্র FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      বিরতি;
    ক্ষেত্র FFI_TYPE_LONGDOUBLE:
    ক্ষেত্র FFI_TYPE_STRUCT:
      flags = (rtype->আকার_2 & 0xfff) << SPARC_SIZEMASK_SHIFT;
      flags |= SPARC_RET_STRUCT;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      বিরতি;
    ক্ষেত্র FFI_TYPE_INT:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_POINTER:
      flags = SPARC_RET_UINT32;
      বিরতি;
    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      বিরতি;
    ক্ষেত্র FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      নির্বাচন (rtt)
	{
	ক্ষেত্র FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  বিরতি;
	পূর্বনির্ধারিত:
	  abort();
	}
      বিরতি;
    পূর্বনির্ধারিত:
      abort();
    }
  cif->flags = flags;

  bytes = 0;
  জন্য (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->আকার_2;
      পূর্ণসংখ্যা tt = ty->type;

      নির্বাচন (tt)
	{
	ক্ষেত্র FFI_TYPE_STRUCT:
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  যদি (tt == FFI_TYPE_FLOAT || z > 8)
	    যাও by_reference;
	  /* FALLTHRU */

	পূর্বনির্ধারিত:
	  z = FFI_ALIGN(z, 4);
	}
      bytes += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  যদি (bytes < 6 * 4)
    bytes = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  bytes += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 2 * 4);

  /* Include the call frame to prep_args.  */
  bytes += 4*16 + 4*8;
  cif->bytes = bytes;

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_call_v8(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
			শূন্য **avalue, size_t bytes, শূন্য *closure) FFI_HIDDEN;

পূর্ণসংখ্যা FFI_HIDDEN
ffi_prep_args_v8(ffi_cif *cif, unsigned long *argp, শূন্য *rvalue, শূন্য **avalue)
{
  ffi_type **p_arg;
  পূর্ণসংখ্যা flags = cif->flags;
  পূর্ণসংখ্যা i, nargs;

  যদি (rvalue == ফাঁকা)
    {
      যদি ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (অক্ষর *)argp + cif->bytes;
	}
      অন্যথা
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
  জন্য (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      শূন্য *a = avalue[i];
      পূর্ণসংখ্যা tt = ty->type;
      size_t z;

      নির্বাচন (tt)
	{
	ক্ষেত্র FFI_TYPE_STRUCT:
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (unsigned long)a;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
	  memcpy(argp, a, 8);
	  argp += 2;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_POINTER:
	  *argp++ = *(unsigned *)a;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  বিরতি;

        ক্ষেত্র FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->আকার_2;
	  যদি (tt == FFI_TYPE_FLOAT || z > 8)
	    যাও by_reference;
	  যদি (z < 4)
	    {
	      memcpy((অক্ষর *)argp + 4 - z, a, z);
	      argp++;
	    }
	  অন্যথা
	    {
	      memcpy(argp, a, z);
	      argp += z / 4;
	    }
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort();
	}
    }

  ফেরত flags;
}

স্থির শূন্য
ffi_call_int (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	      শূন্য **avalue, শূন্য *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  যদি (rvalue == ফাঁকা
      && (cif->flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    bytes += FFI_ALIGN (cif->rtype->আকার_2, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      যদি (at->type == FFI_TYPE_STRUCT)
        {
          অক্ষর *argcopy = alloca (আকার_2);
          যদি (avalue_copy == ফাঁকা)
            {
              avalue_copy = alloca (nargs * আকার (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], আকার_2);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -bytes, closure);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
স্থির অন্তঃরেখ শূন্য
ffi_flush_icache (শূন্য *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ অস্থির ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#অন্যথা
বাহ্যিক শূন্য ffi_flush_icache (শূন্য *) FFI_HIDDEN;
#endif

বাহ্যিক শূন্য ffi_closure_v8(শূন্য) FFI_HIDDEN;
বাহ্যিক শূন্য ffi_go_closure_v8(শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  unsigned পূর্ণসংখ্যা *tramp = (unsigned পূর্ণসংখ্যা *) &closure->tramp[0];
  unsigned long ctx = (unsigned long) closure;
  unsigned long fn = (unsigned long) ffi_closure_v8;

  যদি (cif->abi != FFI_V8)
    ফেরত FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ffi_flush_icache (closure);

  ফেরত FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
  যদি (cif->abi != FFI_V8)
    ফেরত FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v8;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

পূর্ণসংখ্যা FFI_HIDDEN
ffi_closure_sparc_inner_v8(ffi_cif *cif, 
			   শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			   শূন্য *user_তথ্য, শূন্য *rvalue,
			   unsigned long *argp)
{
  ffi_type **arg_types;
  শূন্য **avalue;
  পূর্ণসংখ্যা i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * আকার(শূন্য *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  যদি ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    {
      শূন্য *new_rvalue = (শূন্য *)*argp;
      *(শূন্য **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      পূর্ণসংখ্যা tt = ty->type;
      শূন্য *a = argp;
      size_t z;

      নির্বাচন (tt)
	{
	ক্ষেত্র FFI_TYPE_STRUCT:
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (শূন্য *)*argp;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	ক্ষেত্র FFI_TYPE_SINT64:
	ক্ষেত্র FFI_TYPE_UINT64:
	  যদি ((unsigned long)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_POINTER:
	  বিরতি;
        ক্ষেত্র FFI_TYPE_UINT16:
        ক্ষেত্র FFI_TYPE_SINT16:
	  a += 2;
	  বিরতি;
        ক্ষেত্র FFI_TYPE_UINT8:
        ক্ষেত্র FFI_TYPE_SINT8:
	  a += 3;
	  বিরতি;

        ক্ষেত্র FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->আকার_2;
	  যদি (tt == FFI_TYPE_FLOAT || z > 8)
	    যাও by_reference;
	  যদি (z < 4)
	    a += 4 - z;
	  অন্যথা যদি (z > 4)
	    argp++;
	  বিরতি;

	পূর্বনির্ধারিত:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_তথ্য);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ফেরত flags;
}
#endif /* !SPARC64 */

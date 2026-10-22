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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव "internal.h"

#ifndef SPARC64

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#जर FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# जर FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अन्यथा
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#endif

/* Perform machine dependent cif processing */
ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  पूर्णांक rtt = rtype->type;
  माप_प्रकार bytes;
  पूर्णांक i, n, flags;

  /* Set the return type flag */
  निवड (rtt)
    {
    स्थिती FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      थांबा;
    स्थिती FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      थांबा;
    स्थिती FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      थांबा;
    स्थिती FFI_TYPE_LONGDOUBLE:
    स्थिती FFI_TYPE_STRUCT:
      flags = (rtype->size & 0xfff) << SPARC_SIZEMASK_SHIFT;
      flags |= SPARC_RET_STRUCT;
      थांबा;
    स्थिती FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      थांबा;
    स्थिती FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      थांबा;
    स्थिती FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      थांबा;
    स्थिती FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      थांबा;
    स्थिती FFI_TYPE_INT:
    स्थिती FFI_TYPE_SINT32:
    स्थिती FFI_TYPE_UINT32:
    स्थिती FFI_TYPE_POINTER:
      flags = SPARC_RET_UINT32;
      थांबा;
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      थांबा;
    स्थिती FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      निवड (rtt)
	{
	स्थिती FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  थांबा;
	स्थिती FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  थांबा;
	स्थिती FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  थांबा;
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  थांबा;
	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  थांबा;
	स्थिती FFI_TYPE_SINT16:
	स्थिती FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  थांबा;
	स्थिती FFI_TYPE_SINT8:
	स्थिती FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  थांबा;
	पूर्वनिर्धारित:
	  पात();
	}
      थांबा;
    पूर्वनिर्धारित:
      पात();
    }
  cif->flags = flags;

  bytes = 0;
  साठी (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      माप_प्रकार z = ty->size;
      पूर्णांक tt = ty->type;

      निवड (tt)
	{
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  थांबा;

	स्थिती FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  जर (tt == FFI_TYPE_FLOAT || z > 8)
	    जा by_reference;
	  /* FALLTHRU */

	पूर्वनिर्धारित:
	  z = FFI_ALIGN(z, 4);
	}
      bytes += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  जर (bytes < 6 * 4)
    bytes = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  bytes += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 2 * 4);

  /* Include the call frame to prep_args.  */
  bytes += 4*16 + 4*8;
  cif->bytes = bytes;

  परत FFI_OK;
}

बाह्य शून्य ffi_call_v8(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
			शून्य **avalue, माप_प्रकार bytes, शून्य *closure) FFI_HIDDEN;

पूर्णांक FFI_HIDDEN
ffi_prep_args_v8(ffi_cif *cif, अचिन्हित दीर्घ *argp, शून्य *rvalue, शून्य **avalue)
{
  ffi_type **p_arg;
  पूर्णांक flags = cif->flags;
  पूर्णांक i, nargs;

  जर (rvalue == रिक्त)
    {
      जर ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (अक्षर *)argp + cif->bytes;
	}
      अन्यथा
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (अचिन्हित दीर्घ)rvalue;

#ifdef USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  स_रखो(argp, 0, 6*4);
#endif

  p_arg = cif->arg_types;
  साठी (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      शून्य *a = avalue[i];
      पूर्णांक tt = ty->type;
      माप_प्रकार z;

      निवड (tt)
	{
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (अचिन्हित दीर्घ)a;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_UINT64:
	स्थिती FFI_TYPE_SINT64:
	  स_नकल(argp, a, 8);
	  argp += 2;
	  थांबा;

	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_POINTER:
	  *argp++ = *(अचिन्हित *)a;
	  थांबा;

	स्थिती FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  थांबा;
	स्थिती FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  थांबा;
	स्थिती FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  थांबा;
	स्थिती FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  थांबा;

        स्थिती FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  जर (tt == FFI_TYPE_FLOAT || z > 8)
	    जा by_reference;
	  जर (z < 4)
	    {
	      स_नकल((अक्षर *)argp + 4 - z, a, z);
	      argp++;
	    }
	  अन्यथा
	    {
	      स_नकल(argp, a, z);
	      argp += z / 4;
	    }
	  थांबा;

	पूर्वनिर्धारित:
	  पात();
	}
    }

  परत flags;
}

स्थिर शून्य
ffi_call_int (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	      शून्य **avalue, शून्य *closure)
{
  माप_प्रकार bytes = cif->bytes;
  माप_प्रकार i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  जर (rvalue == रिक्त
      && (cif->flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    bytes += FFI_ALIGN (cif->rtype->size, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक size = at->size;
      जर (at->type == FFI_TYPE_STRUCT)
        {
          अक्षर *argcopy = alloca (size);
          जर (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * आकार (शून्य *));
              स_नकल (avalue_copy, avalue, nargs * आकार (शून्य *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }

  ffi_call_v8(cif, fn, rvalue, avalue, -bytes, closure);
}

शून्य
ffi_call (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
स्थिर ओळीत शून्य
ffi_flush_icache (शून्य *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ अस्थिर ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#अन्यथा
बाह्य शून्य ffi_flush_icache (शून्य *) FFI_HIDDEN;
#endif

बाह्य शून्य ffi_closure_v8(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_v8(शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  अचिन्हित दीर्घ ctx = (अचिन्हित दीर्घ) closure;
  अचिन्हित दीर्घ fn = (अचिन्हित दीर्घ) ffi_closure_v8;

  जर (cif->abi != FFI_V8)
    परत FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ffi_flush_icache (closure);

  परत FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  जर (cif->abi != FFI_V8)
    परत FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v8;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

पूर्णांक FFI_HIDDEN
ffi_closure_sparc_inner_v8(ffi_cif *cif, 
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_data, शून्य *rvalue,
			   अचिन्हित दीर्घ *argp)
{
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णांक i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * आकार(शून्य *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  जर ((flags & SPARC_FLAG_RET_MASK) == SPARC_RET_STRUCT)
    {
      शून्य *new_rvalue = (शून्य *)*argp;
      *(शून्य **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      पूर्णांक tt = ty->type;
      शून्य *a = argp;
      माप_प्रकार z;

      निवड (tt)
	{
	स्थिती FFI_TYPE_STRUCT:
	स्थिती FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (शून्य *)*argp;
	  थांबा;

	स्थिती FFI_TYPE_DOUBLE:
	स्थिती FFI_TYPE_SINT64:
	स्थिती FFI_TYPE_UINT64:
	  जर ((अचिन्हित दीर्घ)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  थांबा;

	स्थिती FFI_TYPE_INT:
	स्थिती FFI_TYPE_FLOAT:
	स्थिती FFI_TYPE_UINT32:
	स्थिती FFI_TYPE_SINT32:
	स्थिती FFI_TYPE_POINTER:
	  थांबा;
        स्थिती FFI_TYPE_UINT16:
        स्थिती FFI_TYPE_SINT16:
	  a += 2;
	  थांबा;
        स्थिती FFI_TYPE_UINT8:
        स्थिती FFI_TYPE_SINT8:
	  a += 3;
	  थांबा;

        स्थिती FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->size;
	  जर (tt == FFI_TYPE_FLOAT || z > 8)
	    जा by_reference;
	  जर (z < 4)
	    a += 4 - z;
	  अन्यथा जर (z > 4)
	    argp++;
	  थांबा;

	पूर्वनिर्धारित:
	  पात();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  परत flags;
}
#endif /* !SPARC64 */

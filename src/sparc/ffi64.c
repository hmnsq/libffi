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

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#सचे FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# सचे FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#अञ्ञथा
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

स्थिर पूर्णाङ्क
ffi_struct_float_mask (ffi_type *outer_type, पूर्णाङ्क size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  सचे (outer_type->type == FFI_TYPE_COMPLEX)
    {
      पूर्णाङ्क m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      सचे (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      पटिनिवत्त (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  अत्थाय (elts = outer_type->elements; (t = *elts) != रिक्त; elts++)
    {
      size_t z = t->size;
      पूर्णाङ्क o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      विकल्प (t->type)
	{
	पक्ष FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  अनुवर्त;
	पक्ष FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  सचे (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    भङ्ग;
	  /* FALLTHRU */
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  भङ्ग;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  पटिनिवत्त size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

स्थिर शून्य *
ffi_struct_float_merge (पूर्णाङ्क size_mask, शून्य *vi, शून्य *vf)
{
  पूर्णाङ्क size = size_mask & 0xff;
  पूर्णाङ्क mask = size_mask >> 8;
  पूर्णाङ्क n = size >> 2;

  सचे (mask == 0)
    पटिनिवत्त vi;
  अञ्ञथा सचे (mask == (1 << n) - 1)
    पटिनिवत्त vf;
  अञ्ञथा
    {
      unsigned पूर्णाङ्क *wi = vi, *wf = vf;
      पूर्णाङ्क i;

      अत्थाय (i = 0; i < n; ++i)
	सचे ((mask >> i) & 1)
	  wi[i] = wf[i];

      पटिनिवत्त vi;
    }
}

/* Similar, but place the data into VD in the end.  */

शून्य FFI_HIDDEN
ffi_struct_float_copy (पूर्णाङ्क size_mask, शून्य *vd, शून्य *vi, शून्य *vf)
{
  पूर्णाङ्क size = size_mask & 0xff;
  पूर्णाङ्क mask = size_mask >> 8;
  पूर्णाङ्क n = size >> 2;

  सचे (mask == 0)
    ;
  अञ्ञथा सचे (mask == (1 << n) - 1)
    vi = vf;
  अञ्ञथा
    {
      unsigned पूर्णाङ्क *wd = vd, *wi = vi, *wf = vf;
      पूर्णाङ्क i;

      अत्थाय (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      पटिनिवत्त;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

स्थिर ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  पूर्णाङ्क rtt = rtype->type;
  size_t bytes = 0;
  पूर्णाङ्क i, n, flags;

  /* Set the return type flag */
  विकल्प (rtt)
    {
    पक्ष FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      भङ्ग;
    पक्ष FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      भङ्ग;
    पक्ष FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      भङ्ग;
    पक्ष FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      भङ्ग;

    पक्ष FFI_TYPE_COMPLEX:
    पक्ष FFI_TYPE_STRUCT:
      सचे (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      अञ्ञथा
	{
	  पूर्णाङ्क size_mask = ffi_struct_float_mask (rtype, 0);
	  पूर्णाङ्क word_size = (size_mask >> 2) & 0x3f;
	  पूर्णाङ्क all_mask = (1 << word_size) - 1;
	  पूर्णाङ्क fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  सचे (fp_mask == 0)
	    {
	      सचे (rtype->alignment >= 8)
		{
		  सचे (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  अञ्ञथा सचे (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  अञ्ञथा सचे (fp_mask == all_mask)
	    विकल्प (word_size)
	      {
	      पक्ष 1: flags = SPARC_RET_F_1; भङ्ग;
	      पक्ष 2: flags = SPARC_RET_F_2; भङ्ग;
	      पक्ष 3: flags = SP_V9_RET_F_3; भङ्ग;
	      पक्ष 4: flags = SPARC_RET_F_4; भङ्ग;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      पक्ष 6: flags = SPARC_RET_F_6; भङ्ग;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      पक्ष 8: flags = SPARC_RET_F_8; भङ्ग;
	      }
	}
      भङ्ग;

    पक्ष FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      भङ्ग;
    पक्ष FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      भङ्ग;
    पक्ष FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      भङ्ग;
    पक्ष FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      भङ्ग;
    पक्ष FFI_TYPE_INT:
    पक्ष FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      भङ्ग;
    पक्ष FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      भङ्ग;
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      भङ्ग;

    स्वभाव:
      abort();
    }

  bytes = 0;
  अत्थाय (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  सचे (z > 16)
	    {
	      a = z = 8;
	      भङ्ग;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  सचे (bytes >= 16*8)
	    भङ्ग;
	  सचे ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    भङ्ग;
	  /* FALLTHRU */
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_DOUBLE:
	पक्ष FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  भङ्ग;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  सचे (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  पटिनिवत्त FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  पटिनिवत्त ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  पटिनिवत्त ffi_prep_cif_machdep_core(cif);
}

बाह्य शून्य ffi_call_v9(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
			शून्य **avalue, size_t bytes, शून्य *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

पूर्णाङ्क FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, शून्य *rvalue, शून्य **avalue)
{
  ffi_type **p_arg;
  पूर्णाङ्क flags = cif->flags;
  पूर्णाङ्क i, nargs;

  सचे (rvalue == रिक्त)
    {
      सचे (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (वर्ण *)argp + cif->bytes;
	}
      अञ्ञथा
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

  सचे (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  अत्थाय (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      शून्य *a = avalue[i];
      size_t z;

      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  z = ty->size;
	  सचे (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      भङ्ग;
	    }
	  सचे (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
    }

  पटिनिवत्त flags;
}

स्थिर शून्य
ffi_call_int(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;

  FFI_ASSERT (cif->abi == FFI_V9);

  सचे (rvalue == रिक्त && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  अत्थाय (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क size = at->size;
      सचे (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          वर्ण *argcopy = alloca (size);
          सचे (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              memcpy (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	    शून्य **avalue, शून्य *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
स्थिर अन्तःस्थ शून्य
ffi_flush_icache (शून्य *p)
{
  __asm__ चञ्चल ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#अञ्ञथा
बाह्य शून्य ffi_flush_icache (शून्य *) FFI_HIDDEN;
#endif

बाह्य शून्य ffi_closure_v9(शून्य) FFI_HIDDEN;
बाह्य शून्य ffi_go_closure_v9(शून्य) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
  unsigned long fn;

  सचे (cif->abi != FFI_V9)
    पटिनिवत्त FFI_BAD_ABI;

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

  पटिनिवत्त FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*))
{
  सचे (cif->abi != FFI_V9)
    पटिनिवत्त FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}

पूर्णाङ्क FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			   शून्य *user_data, शून्य *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  शून्य **avalue;
  पूर्णाङ्क i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * परिमाण(शून्य *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  सचे (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (शून्य *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  अञ्ञथा
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  अत्थाय (i = 0; i < nargs; i++, argn = argx)
    {
      पूर्णाङ्क named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      शून्य *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      विकल्प (ty->type)
	{
	पक्ष FFI_TYPE_COMPLEX:
	पक्ष FFI_TYPE_STRUCT:
	  z = ty->size;
	  सचे (z > 16)
	    a = *(शून्य **)a;
	  अञ्ञथा
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      सचे (named && argn < 16)
		{
		  पूर्णाङ्क size_mask = ffi_struct_float_mask (ty, 0);
		  पूर्णाङ्क argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  भङ्ग;

	पक्ष FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  भङ्ग;
	पक्ष FFI_TYPE_DOUBLE:
	  सचे (named && argn < 16)
	    a = fpr + argn;
	  भङ्ग;
	पक्ष FFI_TYPE_FLOAT:
	  सचे (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  भङ्ग;

	पक्ष FFI_TYPE_UINT64:
	पक्ष FFI_TYPE_SINT64:
	पक्ष FFI_TYPE_POINTER:
	  भङ्ग;
	पक्ष FFI_TYPE_INT:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_SINT32:
	  a += 4;
	  भङ्ग;
        पक्ष FFI_TYPE_UINT16:
        पक्ष FFI_TYPE_SINT16:
	  a += 6;
	  भङ्ग;
        पक्ष FFI_TYPE_UINT8:
        पक्ष FFI_TYPE_SINT8:
	  a += 7;
	  भङ्ग;

	स्वभाव:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  पटिनिवत्त flags;
}
#endif /* SPARC64 */

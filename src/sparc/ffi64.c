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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# எனில் FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#இல்லையெனில்
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

நிலையான முழு
ffi_struct_float_mask (ffi_type *outer_type, முழு size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  எனில் (outer_type->type == FFI_TYPE_COMPLEX)
    {
      முழு m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      எனில் (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      திரும்பு (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  க்கு (elts = outer_type->elements; (t = *elts) != வெற்று; elts++)
    {
      size_t z = t->size;
      முழு o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      தேர்வு (t->type)
	{
	நிலை FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  தொடர்;
	நிலை FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  எனில் (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    நிறுத்து;
	  /* FALLTHRU */
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  நிறுத்து;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  திரும்பு size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

நிலையான வெறுமை *
ffi_struct_float_merge (முழு size_mask, வெறுமை *vi, வெறுமை *vf)
{
  முழு size = size_mask & 0xff;
  முழு mask = size_mask >> 8;
  முழு n = size >> 2;

  எனில் (mask == 0)
    திரும்பு vi;
  இல்லையெனில் எனில் (mask == (1 << n) - 1)
    திரும்பு vf;
  இல்லையெனில்
    {
      unsigned முழு *wi = vi, *wf = vf;
      முழு i;

      க்கு (i = 0; i < n; ++i)
	எனில் ((mask >> i) & 1)
	  wi[i] = wf[i];

      திரும்பு vi;
    }
}

/* Similar, but place the data into VD in the end.  */

வெறுமை FFI_HIDDEN
ffi_struct_float_copy (முழு size_mask, வெறுமை *vd, வெறுமை *vi, வெறுமை *vf)
{
  முழு size = size_mask & 0xff;
  முழு mask = size_mask >> 8;
  முழு n = size >> 2;

  எனில் (mask == 0)
    ;
  இல்லையெனில் எனில் (mask == (1 << n) - 1)
    vi = vf;
  இல்லையெனில்
    {
      unsigned முழு *wd = vd, *wi = vi, *wf = vf;
      முழு i;

      க்கு (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      திரும்பு;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

நிலையான ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  முழு rtt = rtype->type;
  size_t bytes = 0;
  முழு i, n, flags;

  /* Set the return type flag */
  தேர்வு (rtt)
    {
    நிலை FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      நிறுத்து;
    நிலை FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      நிறுத்து;
    நிலை FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      நிறுத்து;
    நிலை FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      நிறுத்து;

    நிலை FFI_TYPE_COMPLEX:
    நிலை FFI_TYPE_STRUCT:
      எனில் (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      இல்லையெனில்
	{
	  முழு size_mask = ffi_struct_float_mask (rtype, 0);
	  முழு word_size = (size_mask >> 2) & 0x3f;
	  முழு all_mask = (1 << word_size) - 1;
	  முழு fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  எனில் (fp_mask == 0)
	    {
	      எனில் (rtype->alignment >= 8)
		{
		  எனில் (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  இல்லையெனில் எனில் (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  இல்லையெனில் எனில் (fp_mask == all_mask)
	    தேர்வு (word_size)
	      {
	      நிலை 1: flags = SPARC_RET_F_1; நிறுத்து;
	      நிலை 2: flags = SPARC_RET_F_2; நிறுத்து;
	      நிலை 3: flags = SP_V9_RET_F_3; நிறுத்து;
	      நிலை 4: flags = SPARC_RET_F_4; நிறுத்து;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      நிலை 6: flags = SPARC_RET_F_6; நிறுத்து;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      நிலை 8: flags = SPARC_RET_F_8; நிறுத்து;
	      }
	}
      நிறுத்து;

    நிலை FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      நிறுத்து;
    நிலை FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      நிறுத்து;
    நிலை FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      நிறுத்து;
    நிலை FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      நிறுத்து;
    நிலை FFI_TYPE_INT:
    நிலை FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      நிறுத்து;
    நிலை FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      நிறுத்து;
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
    நிலை FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      நிறுத்து;

    இயல்பு:
      abort();
    }

  bytes = 0;
  க்கு (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      தேர்வு (ty->type)
	{
	நிலை FFI_TYPE_COMPLEX:
	நிலை FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  எனில் (z > 16)
	    {
	      a = z = 8;
	      நிறுத்து;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  எனில் (bytes >= 16*8)
	    நிறுத்து;
	  எனில் ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    நிறுத்து;
	  /* FALLTHRU */
	நிலை FFI_TYPE_FLOAT:
	நிலை FFI_TYPE_DOUBLE:
	நிலை FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  நிறுத்து;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  எனில் (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  திரும்பு FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  திரும்பு ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  திரும்பு ffi_prep_cif_machdep_core(cif);
}

வெளி வெறுமை ffi_call_v9(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
			வெறுமை **avalue, size_t bytes, வெறுமை *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

முழு FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_type **p_arg;
  முழு flags = cif->flags;
  முழு i, nargs;

  எனில் (rvalue == வெற்று)
    {
      எனில் (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (எழுத்து *)argp + cif->bytes;
	}
      இல்லையெனில்
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

  எனில் (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  க்கு (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      வெறுமை *a = avalue[i];
      size_t z;

      தேர்வு (ty->type)
	{
	நிலை FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  நிறுத்து;
	நிலை FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	நிலை FFI_TYPE_COMPLEX:
	நிலை FFI_TYPE_STRUCT:
	  z = ty->size;
	  எனில் (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      நிறுத்து;
	    }
	  எனில் (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  நிறுத்து;

	இயல்பு:
	  abort();
	}
    }

  திரும்பு flags;
}

நிலையான வெறுமை
ffi_call_int(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue_copy = வெற்று;

  FFI_ASSERT (cif->abi == FFI_V9);

  எனில் (rvalue == வெற்று && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      முழு size = at->size;
      எனில் (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          எழுத்து *argcopy = alloca (size);
          எனில் (avalue_copy == வெற்று)
            {
              avalue_copy = alloca (nargs * அளவு (வெறுமை *));
              memcpy (avalue_copy, avalue, nargs * அளவு (வெறுமை *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

வெறுமை
ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	    வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
நிலையான உள்வரி வெறுமை
ffi_flush_icache (வெறுமை *p)
{
  __asm__ நிலையற்ற ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#இல்லையெனில்
வெளி வெறுமை ffi_flush_icache (வெறுமை *) FFI_HIDDEN;
#endif

வெளி வெறுமை ffi_closure_v9(வெறுமை) FFI_HIDDEN;
வெளி வெறுமை ffi_go_closure_v9(வெறுமை) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp = (unsigned முழு *) &closure->tramp[0];
  unsigned long fn;

  எனில் (cif->abi != FFI_V9)
    திரும்பு FFI_BAD_ABI;

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

  திரும்பு FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*))
{
  எனில் (cif->abi != FFI_V9)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

முழு FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			   வெறுமை *user_data, வெறுமை *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  வெறுமை **avalue;
  முழு i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * அளவு(வெறுமை *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  எனில் (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (வெறுமை *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  இல்லையெனில்
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  க்கு (i = 0; i < nargs; i++, argn = argx)
    {
      முழு named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      வெறுமை *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      தேர்வு (ty->type)
	{
	நிலை FFI_TYPE_COMPLEX:
	நிலை FFI_TYPE_STRUCT:
	  z = ty->size;
	  எனில் (z > 16)
	    a = *(வெறுமை **)a;
	  இல்லையெனில்
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      எனில் (named && argn < 16)
		{
		  முழு size_mask = ffi_struct_float_mask (ty, 0);
		  முழு argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  நிறுத்து;

	நிலை FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  நிறுத்து;
	நிலை FFI_TYPE_DOUBLE:
	  எனில் (named && argn < 16)
	    a = fpr + argn;
	  நிறுத்து;
	நிலை FFI_TYPE_FLOAT:
	  எனில் (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  நிறுத்து;

	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
	நிலை FFI_TYPE_POINTER:
	  நிறுத்து;
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_SINT32:
	  a += 4;
	  நிறுத்து;
        நிலை FFI_TYPE_UINT16:
        நிலை FFI_TYPE_SINT16:
	  a += 6;
	  நிறுத்து;
        நிலை FFI_TYPE_UINT8:
        நிலை FFI_TYPE_SINT8:
	  a += 7;
	  நிறுத்து;

	இயல்பு:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  திரும்பு flags;
}
#endif /* SPARC64 */

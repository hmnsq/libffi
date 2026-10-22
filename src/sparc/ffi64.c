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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ "internal.h"

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

#ifdef SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

স্থির পূর্ণাঙ্ক
ffi_struct_float_mask (ffi_type *outer_type, পূর্ণাঙ্ক size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  যদি (outer_type->type == FFI_TYPE_COMPLEX)
    {
      পূর্ণাঙ্ক m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      যদি (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      ঘূৰাই (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  বাবে (elts = outer_type->elements; (t = *elts) != রিক্ত; elts++)
    {
      size_t z = t->size;
      পূর্ণাঙ্ক o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      ৱিকল্প (t->type)
	{
	পক্ষ FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  অনুৱর্ত;
	পক্ষ FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  যদি (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    ভঙ্গ;
	  /* FALLTHRU */
	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  ভঙ্গ;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  ঘূৰাই size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

স্থির শূন্য *
ffi_struct_float_merge (পূর্ণাঙ্ক size_mask, শূন্য *vi, শূন্য *vf)
{
  পূর্ণাঙ্ক size = size_mask & 0xff;
  পূর্ণাঙ্ক mask = size_mask >> 8;
  পূর্ণাঙ্ক n = size >> 2;

  যদি (mask == 0)
    ঘূৰাই vi;
  অন্যথা যদি (mask == (1 << n) - 1)
    ঘূৰাই vf;
  অন্যথা
    {
      unsigned পূর্ণাঙ্ক *wi = vi, *wf = vf;
      পূর্ণাঙ্ক i;

      বাবে (i = 0; i < n; ++i)
	যদি ((mask >> i) & 1)
	  wi[i] = wf[i];

      ঘূৰাই vi;
    }
}

/* Similar, but place the data into VD in the end.  */

শূন্য FFI_HIDDEN
ffi_struct_float_copy (পূর্ণাঙ্ক size_mask, শূন্য *vd, শূন্য *vi, শূন্য *vf)
{
  পূর্ণাঙ্ক size = size_mask & 0xff;
  পূর্ণাঙ্ক mask = size_mask >> 8;
  পূর্ণাঙ্ক n = size >> 2;

  যদি (mask == 0)
    ;
  অন্যথা যদি (mask == (1 << n) - 1)
    vi = vf;
  অন্যথা
    {
      unsigned পূর্ণাঙ্ক *wd = vd, *wi = vi, *wf = vf;
      পূর্ণাঙ্ক i;

      বাবে (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      ঘূৰাই;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

স্থির ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  পূর্ণাঙ্ক rtt = rtype->type;
  size_t bytes = 0;
  পূর্ণাঙ্ক i, n, flags;

  /* Set the return type flag */
  ৱিকল্প (rtt)
    {
    পক্ষ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ভঙ্গ;
    পক্ষ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ভঙ্গ;
    পক্ষ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ভঙ্গ;
    পক্ষ FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      ভঙ্গ;

    পক্ষ FFI_TYPE_COMPLEX:
    পক্ষ FFI_TYPE_STRUCT:
      যদি (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      অন্যথা
	{
	  পূর্ণাঙ্ক size_mask = ffi_struct_float_mask (rtype, 0);
	  পূর্ণাঙ্ক word_size = (size_mask >> 2) & 0x3f;
	  পূর্ণাঙ্ক all_mask = (1 << word_size) - 1;
	  পূর্ণাঙ্ক fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  যদি (fp_mask == 0)
	    {
	      যদি (rtype->alignment >= 8)
		{
		  যদি (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  অন্যথা যদি (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  অন্যথা যদি (fp_mask == all_mask)
	    ৱিকল্প (word_size)
	      {
	      পক্ষ 1: flags = SPARC_RET_F_1; ভঙ্গ;
	      পক্ষ 2: flags = SPARC_RET_F_2; ভঙ্গ;
	      পক্ষ 3: flags = SP_V9_RET_F_3; ভঙ্গ;
	      পক্ষ 4: flags = SPARC_RET_F_4; ভঙ্গ;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      পক্ষ 6: flags = SPARC_RET_F_6; ভঙ্গ;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      পক্ষ 8: flags = SPARC_RET_F_8; ভঙ্গ;
	      }
	}
      ভঙ্গ;

    পক্ষ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ভঙ্গ;
    পক্ষ FFI_TYPE_INT:
    পক্ষ FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      ভঙ্গ;
    পক্ষ FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      ভঙ্গ;
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
    পক্ষ FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      ভঙ্গ;

    স্ৱভাৱ:
      abort();
    }

  bytes = 0;
  বাবে (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      ৱিকল্প (ty->type)
	{
	পক্ষ FFI_TYPE_COMPLEX:
	পক্ষ FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  যদি (z > 16)
	    {
	      a = z = 8;
	      ভঙ্গ;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  যদি (bytes >= 16*8)
	    ভঙ্গ;
	  যদি ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    ভঙ্গ;
	  /* FALLTHRU */
	পক্ষ FFI_TYPE_FLOAT:
	পক্ষ FFI_TYPE_DOUBLE:
	পক্ষ FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  ভঙ্গ;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  যদি (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  ঘূৰাই FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ঘূৰাই ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  ঘূৰাই ffi_prep_cif_machdep_core(cif);
}

বাহ্য শূন্য ffi_call_v9(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
			শূন্য **avalue, size_t bytes, শূন্য *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

পূর্ণাঙ্ক FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, শূন্য *rvalue, শূন্য **avalue)
{
  ffi_type **p_arg;
  পূর্ণাঙ্ক flags = cif->flags;
  পূর্ণাঙ্ক i, nargs;

  যদি (rvalue == রিক্ত)
    {
      যদি (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (ৱর্ণ *)argp + cif->bytes;
	}
      অন্যথা
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

  যদি (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  বাবে (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      শূন্য *a = avalue[i];
      size_t z;

      ৱিকল্প (ty->type)
	{
	পক্ষ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_LONGDOUBLE:
	পক্ষ FFI_TYPE_COMPLEX:
	পক্ষ FFI_TYPE_STRUCT:
	  z = ty->size;
	  যদি (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      ভঙ্গ;
	    }
	  যদি (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  ভঙ্গ;

	স্ৱভাৱ:
	  abort();
	}
    }

  ঘূৰাই flags;
}

স্থির শূন্য
ffi_call_int(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = রিক্ত;

  FFI_ASSERT (cif->abi == FFI_V9);

  যদি (rvalue == রিক্ত && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  বাবে (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণাঙ্ক size = at->size;
      যদি (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          ৱর্ণ *argcopy = alloca (size);
          যদি (avalue_copy == রিক্ত)
            {
              avalue_copy = alloca (nargs * পরিমাণ (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * পরিমাণ (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

শূন্য
ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, রিক্ত);
}

শূন্য
ffi_call_go(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	    শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
স্থির অন্তঃস্থ শূন্য
ffi_flush_icache (শূন্য *p)
{
  __asm__ চঞ্চল ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#অন্যথা
বাহ্য শূন্য ffi_flush_icache (শূন্য *) FFI_HIDDEN;
#endif

বাহ্য শূন্য ffi_closure_v9(শূন্য) FFI_HIDDEN;
বাহ্য শূন্য ffi_go_closure_v9(শূন্য) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
  unsigned long fn;

  যদি (cif->abi != FFI_V9)
    ঘূৰাই FFI_BAD_ABI;

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

  ঘূৰাই FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*))
{
  যদি (cif->abi != FFI_V9)
    ঘূৰাই FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  ঘূৰাই FFI_OK;
}

পূর্ণাঙ্ক FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			   শূন্য *user_data, শূন্য *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  শূন্য **avalue;
  পূর্ণাঙ্ক i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * পরিমাণ(শূন্য *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  যদি (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (শূন্য *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  অন্যথা
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  বাবে (i = 0; i < nargs; i++, argn = argx)
    {
      পূর্ণাঙ্ক named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      শূন্য *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      ৱিকল্প (ty->type)
	{
	পক্ষ FFI_TYPE_COMPLEX:
	পক্ষ FFI_TYPE_STRUCT:
	  z = ty->size;
	  যদি (z > 16)
	    a = *(শূন্য **)a;
	  অন্যথা
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      যদি (named && argn < 16)
		{
		  পূর্ণাঙ্ক size_mask = ffi_struct_float_mask (ty, 0);
		  পূর্ণাঙ্ক argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  ভঙ্গ;

	পক্ষ FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_DOUBLE:
	  যদি (named && argn < 16)
	    a = fpr + argn;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_FLOAT:
	  যদি (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  ভঙ্গ;

	পক্ষ FFI_TYPE_UINT64:
	পক্ষ FFI_TYPE_SINT64:
	পক্ষ FFI_TYPE_POINTER:
	  ভঙ্গ;
	পক্ষ FFI_TYPE_INT:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_SINT32:
	  a += 4;
	  ভঙ্গ;
        পক্ষ FFI_TYPE_UINT16:
        পক্ষ FFI_TYPE_SINT16:
	  a += 6;
	  ভঙ্গ;
        পক্ষ FFI_TYPE_UINT8:
        পক্ষ FFI_TYPE_SINT8:
	  a += 7;
	  ভঙ্গ;

	স্ৱভাৱ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ঘূৰাই flags;
}
#endif /* SPARC64 */

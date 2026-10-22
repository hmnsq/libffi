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

#ifdef SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

સ્થિર પૂર્ણાંક
ffi_struct_float_mask (ffi_type *outer_type, પૂર્ણાંક size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  જો (outer_type->type == FFI_TYPE_COMPLEX)
    {
      પૂર્ણાંક m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      જો (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      પાછા (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  માટે (elts = outer_type->elements; (t = *elts) != ખાલી; elts++)
    {
      size_t z = t->size;
      પૂર્ણાંક o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      પસંદગી (t->type)
	{
	સ્થિતિ FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  ચાલુ;
	સ્થિતિ FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  જો (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    તોડો;
	  /* FALLTHRU */
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  તોડો;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  પાછા size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

સ્થિર શૂન્ય *
ffi_struct_float_merge (પૂર્ણાંક size_mask, શૂન્ય *vi, શૂન્ય *vf)
{
  પૂર્ણાંક size = size_mask & 0xff;
  પૂર્ણાંક mask = size_mask >> 8;
  પૂર્ણાંક n = size >> 2;

  જો (mask == 0)
    પાછા vi;
  નહીંતર જો (mask == (1 << n) - 1)
    પાછા vf;
  નહીંતર
    {
      unsigned પૂર્ણાંક *wi = vi, *wf = vf;
      પૂર્ણાંક i;

      માટે (i = 0; i < n; ++i)
	જો ((mask >> i) & 1)
	  wi[i] = wf[i];

      પાછા vi;
    }
}

/* Similar, but place the data into VD in the end.  */

શૂન્ય FFI_HIDDEN
ffi_struct_float_copy (પૂર્ણાંક size_mask, શૂન્ય *vd, શૂન્ય *vi, શૂન્ય *vf)
{
  પૂર્ણાંક size = size_mask & 0xff;
  પૂર્ણાંક mask = size_mask >> 8;
  પૂર્ણાંક n = size >> 2;

  જો (mask == 0)
    ;
  નહીંતર જો (mask == (1 << n) - 1)
    vi = vf;
  નહીંતર
    {
      unsigned પૂર્ણાંક *wd = vd, *wi = vi, *wf = vf;
      પૂર્ણાંક i;

      માટે (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      પાછા;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

સ્થિર ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  પૂર્ણાંક rtt = rtype->type;
  size_t bytes = 0;
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
      flags = SPARC_RET_F_4;
      તોડો;

    સ્થિતિ FFI_TYPE_COMPLEX:
    સ્થિતિ FFI_TYPE_STRUCT:
      જો (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      નહીંતર
	{
	  પૂર્ણાંક size_mask = ffi_struct_float_mask (rtype, 0);
	  પૂર્ણાંક word_size = (size_mask >> 2) & 0x3f;
	  પૂર્ણાંક all_mask = (1 << word_size) - 1;
	  પૂર્ણાંક fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  જો (fp_mask == 0)
	    {
	      જો (rtype->alignment >= 8)
		{
		  જો (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  નહીંતર જો (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  નહીંતર જો (fp_mask == all_mask)
	    પસંદગી (word_size)
	      {
	      સ્થિતિ 1: flags = SPARC_RET_F_1; તોડો;
	      સ્થિતિ 2: flags = SPARC_RET_F_2; તોડો;
	      સ્થિતિ 3: flags = SP_V9_RET_F_3; તોડો;
	      સ્થિતિ 4: flags = SPARC_RET_F_4; તોડો;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      સ્થિતિ 6: flags = SPARC_RET_F_6; તોડો;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      સ્થિતિ 8: flags = SPARC_RET_F_8; તોડો;
	      }
	}
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
      flags = SP_V9_RET_SINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      તોડો;
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
    સ્થિતિ FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      તોડો;

    મૂળભૂત:
      abort();
    }

  bytes = 0;
  માટે (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      પસંદગી (ty->type)
	{
	સ્થિતિ FFI_TYPE_COMPLEX:
	સ્થિતિ FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  જો (z > 16)
	    {
	      a = z = 8;
	      તોડો;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  જો (bytes >= 16*8)
	    તોડો;
	  જો ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    તોડો;
	  /* FALLTHRU */
	સ્થિતિ FFI_TYPE_FLOAT:
	સ્થિતિ FFI_TYPE_DOUBLE:
	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  તોડો;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  જો (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  પાછા FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  પાછા ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  પાછા ffi_prep_cif_machdep_core(cif);
}

બાહ્ય શૂન્ય ffi_call_v9(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
			શૂન્ય **avalue, size_t bytes, શૂન્ય *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

પૂર્ણાંક FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_type **p_arg;
  પૂર્ણાંક flags = cif->flags;
  પૂર્ણાંક i, nargs;

  જો (rvalue == ખાલી)
    {
      જો (flags & SPARC_FLAG_RET_IN_MEM)
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

#ifdef USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#endif

  જો (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  માટે (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      શૂન્ય *a = avalue[i];
      size_t z;

      પસંદગી (ty->type)
	{
	સ્થિતિ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  તોડો;
	સ્થિતિ FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	સ્થિતિ FFI_TYPE_COMPLEX:
	સ્થિતિ FFI_TYPE_STRUCT:
	  z = ty->size;
	  જો (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      તોડો;
	    }
	  જો (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  તોડો;

	મૂળભૂત:
	  abort();
	}
    }

  પાછા flags;
}

સ્થિર શૂન્ય
ffi_call_int(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;

  FFI_ASSERT (cif->abi == FFI_V9);

  જો (rvalue == ખાલી && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      જો (at->type == FFI_TYPE_STRUCT && size > 4)
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
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

શૂન્ય
ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	    શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
સ્થિર અંતઃરેખ શૂન્ય
ffi_flush_icache (શૂન્ય *p)
{
  __asm__ અસ્થિર ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#નહીંતર
બાહ્ય શૂન્ય ffi_flush_icache (શૂન્ય *) FFI_HIDDEN;
#endif

બાહ્ય શૂન્ય ffi_closure_v9(શૂન્ય) FFI_HIDDEN;
બાહ્ય શૂન્ય ffi_go_closure_v9(શૂન્ય) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  unsigned long fn;

  જો (cif->abi != FFI_V9)
    પાછા FFI_BAD_ABI;

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

  પાછા FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*))
{
  જો (cif->abi != FFI_V9)
    પાછા FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

પૂર્ણાંક FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			   શૂન્ય *user_data, શૂન્ય *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  શૂન્ય **avalue;
  પૂર્ણાંક i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * કદ(શૂન્ય *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  જો (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (શૂન્ય *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  નહીંતર
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  માટે (i = 0; i < nargs; i++, argn = argx)
    {
      પૂર્ણાંક named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      શૂન્ય *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      પસંદગી (ty->type)
	{
	સ્થિતિ FFI_TYPE_COMPLEX:
	સ્થિતિ FFI_TYPE_STRUCT:
	  z = ty->size;
	  જો (z > 16)
	    a = *(શૂન્ય **)a;
	  નહીંતર
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      જો (named && argn < 16)
		{
		  પૂર્ણાંક size_mask = ffi_struct_float_mask (ty, 0);
		  પૂર્ણાંક argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  તોડો;

	સ્થિતિ FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  તોડો;
	સ્થિતિ FFI_TYPE_DOUBLE:
	  જો (named && argn < 16)
	    a = fpr + argn;
	  તોડો;
	સ્થિતિ FFI_TYPE_FLOAT:
	  જો (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  તોડો;

	સ્થિતિ FFI_TYPE_UINT64:
	સ્થિતિ FFI_TYPE_SINT64:
	સ્થિતિ FFI_TYPE_POINTER:
	  તોડો;
	સ્થિતિ FFI_TYPE_INT:
	સ્થિતિ FFI_TYPE_UINT32:
	સ્થિતિ FFI_TYPE_SINT32:
	  a += 4;
	  તોડો;
        સ્થિતિ FFI_TYPE_UINT16:
        સ્થિતિ FFI_TYPE_SINT16:
	  a += 6;
	  તોડો;
        સ્થિતિ FFI_TYPE_UINT8:
        સ્થિતિ FFI_TYPE_SINT8:
	  a += 7;
	  તોડો;

	મૂળભૂત:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  પાછા flags;
}
#endif /* SPARC64 */

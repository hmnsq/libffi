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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#എങ്കിൽ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# എങ്കിൽ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE out of date
# endif
#അല്ലെങ്കിൽ
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

സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_struct_float_mask (ffi_type *outer_type, പൂർണ്ണസംഖ്യ size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  എങ്കിൽ (outer_type->type == FFI_TYPE_COMPLEX)
    {
      പൂർണ്ണസംഖ്യ m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      എങ്കിൽ (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      തിരികെ (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  വേണ്ടി (elts = outer_type->elements; (t = *elts) != ശൂന്യ; elts++)
    {
      size_t z = t->size;
      പൂർണ്ണസംഖ്യ o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      തിരഞ്ഞെടുപ്പ് (t->type)
	{
	അവസ്ഥ FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  തുടരുക;
	അവസ്ഥ FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  എങ്കിൽ (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    നിർത്തുക;
	  /* FALLTHRU */
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  നിർത്തുക;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  തിരികെ size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

സ്ഥിതം ശൂന്യം *
ffi_struct_float_merge (പൂർണ്ണസംഖ്യ size_mask, ശൂന്യം *vi, ശൂന്യം *vf)
{
  പൂർണ്ണസംഖ്യ size = size_mask & 0xff;
  പൂർണ്ണസംഖ്യ mask = size_mask >> 8;
  പൂർണ്ണസംഖ്യ n = size >> 2;

  എങ്കിൽ (mask == 0)
    തിരികെ vi;
  അല്ലെങ്കിൽ എങ്കിൽ (mask == (1 << n) - 1)
    തിരികെ vf;
  അല്ലെങ്കിൽ
    {
      unsigned പൂർണ്ണസംഖ്യ *wi = vi, *wf = vf;
      പൂർണ്ണസംഖ്യ i;

      വേണ്ടി (i = 0; i < n; ++i)
	എങ്കിൽ ((mask >> i) & 1)
	  wi[i] = wf[i];

      തിരികെ vi;
    }
}

/* Similar, but place the data into VD in the end.  */

ശൂന്യം FFI_HIDDEN
ffi_struct_float_copy (പൂർണ്ണസംഖ്യ size_mask, ശൂന്യം *vd, ശൂന്യം *vi, ശൂന്യം *vf)
{
  പൂർണ്ണസംഖ്യ size = size_mask & 0xff;
  പൂർണ്ണസംഖ്യ mask = size_mask >> 8;
  പൂർണ്ണസംഖ്യ n = size >> 2;

  എങ്കിൽ (mask == 0)
    ;
  അല്ലെങ്കിൽ എങ്കിൽ (mask == (1 << n) - 1)
    vi = vf;
  അല്ലെങ്കിൽ
    {
      unsigned പൂർണ്ണസംഖ്യ *wd = vd, *wi = vi, *wf = vf;
      പൂർണ്ണസംഖ്യ i;

      വേണ്ടി (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      തിരികെ;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

സ്ഥിതം ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  പൂർണ്ണസംഖ്യ rtt = rtype->type;
  size_t bytes = 0;
  പൂർണ്ണസംഖ്യ i, n, flags;

  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (rtt)
    {
    അവസ്ഥ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_COMPLEX:
    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      അല്ലെങ്കിൽ
	{
	  പൂർണ്ണസംഖ്യ size_mask = ffi_struct_float_mask (rtype, 0);
	  പൂർണ്ണസംഖ്യ word_size = (size_mask >> 2) & 0x3f;
	  പൂർണ്ണസംഖ്യ all_mask = (1 << word_size) - 1;
	  പൂർണ്ണസംഖ്യ fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  എങ്കിൽ (fp_mask == 0)
	    {
	      എങ്കിൽ (rtype->alignment >= 8)
		{
		  എങ്കിൽ (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  അല്ലെങ്കിൽ എങ്കിൽ (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  അല്ലെങ്കിൽ എങ്കിൽ (fp_mask == all_mask)
	    തിരഞ്ഞെടുപ്പ് (word_size)
	      {
	      അവസ്ഥ 1: flags = SPARC_RET_F_1; നിർത്തുക;
	      അവസ്ഥ 2: flags = SPARC_RET_F_2; നിർത്തുക;
	      അവസ്ഥ 3: flags = SP_V9_RET_F_3; നിർത്തുക;
	      അവസ്ഥ 4: flags = SPARC_RET_F_4; നിർത്തുക;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      അവസ്ഥ 6: flags = SPARC_RET_F_6; നിർത്തുക;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      അവസ്ഥ 8: flags = SPARC_RET_F_8; നിർത്തുക;
	      }
	}
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_INT:
    അവസ്ഥ FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      നിർത്തുക;
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      abort();
    }

  bytes = 0;
  വേണ്ടി (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      തിരഞ്ഞെടുപ്പ് (ty->type)
	{
	അവസ്ഥ FFI_TYPE_COMPLEX:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  എങ്കിൽ (z > 16)
	    {
	      a = z = 8;
	      നിർത്തുക;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  എങ്കിൽ (bytes >= 16*8)
	    നിർത്തുക;
	  എങ്കിൽ ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    നിർത്തുക;
	  /* FALLTHRU */
	അവസ്ഥ FFI_TYPE_FLOAT:
	അവസ്ഥ FFI_TYPE_DOUBLE:
	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  നിർത്തുക;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  എങ്കിൽ (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  തിരികെ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  തിരികെ ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  തിരികെ ffi_prep_cif_machdep_core(cif);
}

ബാഹ്യ ശൂന്യം ffi_call_v9(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
			ശൂന്യം **avalue, size_t bytes, ശൂന്യം *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_type **p_arg;
  പൂർണ്ണസംഖ്യ flags = cif->flags;
  പൂർണ്ണസംഖ്യ i, nargs;

  എങ്കിൽ (rvalue == ശൂന്യ)
    {
      എങ്കിൽ (flags & SPARC_FLAG_RET_IN_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (അക്ഷരം *)argp + cif->bytes;
	}
      അല്ലെങ്കിൽ
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

  എങ്കിൽ (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  വേണ്ടി (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ശൂന്യം *a = avalue[i];
      size_t z;

      തിരഞ്ഞെടുപ്പ് (ty->type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	അവസ്ഥ FFI_TYPE_COMPLEX:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  z = ty->size;
	  എങ്കിൽ (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      നിർത്തുക;
	    }
	  എങ്കിൽ (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort();
	}
    }

  തിരികെ flags;
}

സ്ഥിതം ശൂന്യം
ffi_call_int(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue_copy = ശൂന്യ;

  FFI_ASSERT (cif->abi == FFI_V9);

  എങ്കിൽ (rvalue == ശൂന്യ && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  വേണ്ടി (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      പൂർണ്ണസംഖ്യ size = at->size;
      എങ്കിൽ (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          അക്ഷരം *argcopy = alloca (size);
          എങ്കിൽ (avalue_copy == ശൂന്യ)
            {
              avalue_copy = alloca (nargs * വലുപ്പം (ശൂന്യം *));
              memcpy (avalue_copy, avalue, nargs * വലുപ്പം (ശൂന്യം *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

ശൂന്യം
ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	    ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
സ്ഥിതം വരിയിൽ ശൂന്യം
ffi_flush_icache (ശൂന്യം *p)
{
  __asm__ അസ്ഥിര ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#അല്ലെങ്കിൽ
ബാഹ്യ ശൂന്യം ffi_flush_icache (ശൂന്യം *) FFI_HIDDEN;
#endif

ബാഹ്യ ശൂന്യം ffi_closure_v9(ശൂന്യം) FFI_HIDDEN;
ബാഹ്യ ശൂന്യം ffi_go_closure_v9(ശൂന്യം) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
  unsigned long fn;

  എങ്കിൽ (cif->abi != FFI_V9)
    തിരികെ FFI_BAD_ABI;

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

  തിരികെ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*))
{
  എങ്കിൽ (cif->abi != FFI_V9)
    തിരികെ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

പൂർണ്ണസംഖ്യ FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
			   ശൂന്യം *user_data, ശൂന്യം *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  ശൂന്യം **avalue;
  പൂർണ്ണസംഖ്യ i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * വലുപ്പം(ശൂന്യം *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  എങ്കിൽ (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (ശൂന്യം *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  അല്ലെങ്കിൽ
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  വേണ്ടി (i = 0; i < nargs; i++, argn = argx)
    {
      പൂർണ്ണസംഖ്യ named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      ശൂന്യം *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      തിരഞ്ഞെടുപ്പ് (ty->type)
	{
	അവസ്ഥ FFI_TYPE_COMPLEX:
	അവസ്ഥ FFI_TYPE_STRUCT:
	  z = ty->size;
	  എങ്കിൽ (z > 16)
	    a = *(ശൂന്യം **)a;
	  അല്ലെങ്കിൽ
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      എങ്കിൽ (named && argn < 16)
		{
		  പൂർണ്ണസംഖ്യ size_mask = ffi_struct_float_mask (ty, 0);
		  പൂർണ്ണസംഖ്യ argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_DOUBLE:
	  എങ്കിൽ (named && argn < 16)
	    a = fpr + argn;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_FLOAT:
	  എങ്കിൽ (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  നിർത്തുക;

	അവസ്ഥ FFI_TYPE_UINT64:
	അവസ്ഥ FFI_TYPE_SINT64:
	അവസ്ഥ FFI_TYPE_POINTER:
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_INT:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_SINT32:
	  a += 4;
	  നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT16:
        അവസ്ഥ FFI_TYPE_SINT16:
	  a += 6;
	  നിർത്തുക;
        അവസ്ഥ FFI_TYPE_UINT8:
        അവസ്ഥ FFI_TYPE_SINT8:
	  a += 7;
	  നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  തിരികെ flags;
}
#endif /* SPARC64 */

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

#ifdef SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_struct_float_mask (ffi_type *outer_type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_mask)
{
  ffi_type **elts;
  ffi_type *t;

  ଯଦି (outer_type->type == FFI_TYPE_COMPLEX)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->size;

      ଯଦି (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      ଫେରନ୍ତୁ (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  ପାଇଁ (elts = outer_type->elements; (t = *elts) != ଖାଲି; elts++)
    {
      size_t z = t->size;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା o, m, tt;

      size_mask = FFI_ALIGN(size_mask, t->alignment);
      ବାଛ (t->type)
	{
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  size_mask = ffi_struct_float_mask (t, size_mask);
	  ଜାରି;
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  ଯଦି (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    ଭାଙ୍ଗ;
	  /* FALLTHRU */
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (size_mask >> 2) & 0x3f;	/* extract word offset */
	  size_mask |= m << (o + 8);	/* insert mask into place */
	  ଭାଙ୍ଗ;
	}
      size_mask += z;
    }

  size_mask = FFI_ALIGN(size_mask, outer_type->alignment);
  FFI_ASSERT ((size_mask & 0xff) == outer_type->size);

  ଫେରନ୍ତୁ size_mask;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ସ୍ଥିର ଶୂନ୍ୟ *
ffi_struct_float_merge (ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_mask, ଶୂନ୍ୟ *vi, ଶୂନ୍ୟ *vf)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = size_mask & 0xff;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = size_mask >> 8;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = size >> 2;

  ଯଦି (mask == 0)
    ଫେରନ୍ତୁ vi;
  ଅନ୍ୟଥା ଯଦି (mask == (1 << n) - 1)
    ଫେରନ୍ତୁ vf;
  ଅନ୍ୟଥା
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *wi = vi, *wf = vf;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

      ପାଇଁ (i = 0; i < n; ++i)
	ଯଦି ((mask >> i) & 1)
	  wi[i] = wf[i];

      ଫେରନ୍ତୁ vi;
    }
}

/* Similar, but place the data into VD in the end.  */

ଶୂନ୍ୟ FFI_HIDDEN
ffi_struct_float_copy (ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_mask, ଶୂନ୍ୟ *vd, ଶୂନ୍ୟ *vi, ଶୂନ୍ୟ *vf)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = size_mask & 0xff;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask = size_mask >> 8;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = size >> 2;

  ଯଦି (mask == 0)
    ;
  ଅନ୍ୟଥା ଯଦି (mask == (1 << n) - 1)
    vi = vf;
  ଅନ୍ୟଥା
    {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *wd = vd, *wi = vi, *wf = vf;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

      ପାଇଁ (i = 0; i < n; ++i)
	wd[i] = ((mask >> i) & 1 ? wf : wi)[i];
      ଫେରନ୍ତୁ;
    }
  memcpy (vd, vi, size);
}

/* Perform machine dependent cif processing */

ସ୍ଥିର ffi_status
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rtt = rtype->type;
  size_t bytes = 0;
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
      flags = SPARC_RET_F_4;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_COMPLEX:
    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (rtype->size > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_IN_MEM;
	  bytes = 8;
	}
      ଅନ୍ୟଥା
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_mask = ffi_struct_float_mask (rtype, 0);
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା word_size = (size_mask >> 2) & 0x3f;
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା all_mask = (1 << word_size) - 1;
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା fp_mask = size_mask >> 8;

	  flags = (size_mask << SPARC_SIZEMASK_SHIFT) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  ଯଦି (fp_mask == 0)
	    {
	      ଯଦି (rtype->alignment >= 8)
		{
		  ଯଦି (rtype->size == 8)
		    flags = SPARC_RET_INT64;
		  ଅନ୍ୟଥା ଯଦି (rtype->size == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  ଅନ୍ୟଥା ଯଦି (fp_mask == all_mask)
	    ବାଛ (word_size)
	      {
	      ସ୍ଥିତି 1: flags = SPARC_RET_F_1; ଭାଙ୍ଗ;
	      ସ୍ଥିତି 2: flags = SPARC_RET_F_2; ଭାଙ୍ଗ;
	      ସ୍ଥିତି 3: flags = SP_V9_RET_F_3; ଭାଙ୍ଗ;
	      ସ୍ଥିତି 4: flags = SPARC_RET_F_4; ଭାଙ୍ଗ;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      ସ୍ଥିତି 6: flags = SPARC_RET_F_6; ଭାଙ୍ଗ;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      ସ୍ଥିତି 8: flags = SPARC_RET_F_8; ଭାଙ୍ଗ;
	      }
	}
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
      flags = SP_V9_RET_SINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
    ସ୍ଥିତି FFI_TYPE_POINTER:
      flags = SPARC_RET_INT64;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      abort();
    }

  bytes = 0;
  ପାଇଁ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->size;
      size_t a = ty->alignment;

      ବାଛ (ty->type)
	{
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  ଯଦି (z > 16)
	    {
	      a = z = 8;
	      ଭାଙ୍ଗ;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  ଯଦି (bytes >= 16*8)
	    ଭାଙ୍ଗ;
	  ଯଦି ((ffi_struct_float_mask (ty, 0) & 0xff00) == 0)
	    ଭାଙ୍ଗ;
	  /* FALLTHRU */
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  ଭାଙ୍ଗ;
	}
      bytes = FFI_ALIGN(bytes, a);
      bytes += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  ଯଦି (bytes < 6 * 8)
    bytes = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  bytes = FFI_ALIGN(bytes, 16);

  /* Include the call frame to prep_args.  */
  bytes += 8*16 + 8*8;

  cif->bytes = bytes;
  cif->flags = flags;
  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ଫେରନ୍ତୁ ffi_prep_cif_machdep_core(cif);
}

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, unsigned nfixedargs, unsigned ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  ଫେରନ୍ତୁ ffi_prep_cif_machdep_core(cif);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_v9(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
			ଶୂନ୍ୟ **avalue, size_t bytes, ଶୂନ୍ୟ *closure) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_prep_args_v9(ffi_cif *cif, unsigned long *argp, ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_type **p_arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = cif->flags;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs;

  ଯଦି (rvalue == ଖାଲି)
    {
      ଯଦି (flags & SPARC_FLAG_RET_IN_MEM)
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

#ifdef USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#endif

  ଯଦି (flags & SPARC_FLAG_RET_IN_MEM)
    *argp++ = (unsigned long)rvalue;

  p_arg = cif->arg_types;
  ପାଇଁ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ଶୂନ୍ୟ *a = avalue[i];
      size_t z;

      ବାଛ (ty->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  *argp++ = *(UINT64 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT32 *)a;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ARGS;
	  *argp++ = *(UINT64 *)a;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  z = ty->size;
	  ଯଦି (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (unsigned long)a;
	      ଭାଙ୍ଗ;
	    }
	  ଯଦି (((unsigned long)argp & 15) && ty->alignment > 8)
	    argp++;
	  memcpy(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}
    }

  ଫେରନ୍ତୁ flags;
}

ସ୍ଥିର ଶୂନ୍ୟ
ffi_call_int(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  size_t bytes = cif->bytes;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;

  FFI_ASSERT (cif->abi == FFI_V9);

  ଯଦି (rvalue == ଖାଲି && (cif->flags & SPARC_FLAG_RET_IN_MEM))
    bytes += FFI_ALIGN (cif->rtype->size, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      ଯଦି (at->type == FFI_TYPE_STRUCT && size > 4)
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
  
  ffi_call_v9(cif, fn, rvalue, avalue, -bytes, closure);
}

ଶୂନ୍ୟ
ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	    ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

#ifdef __GNUC__
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ
ffi_flush_icache (ଶୂନ୍ୟ *p)
{
  __asm__ ଅସ୍ଥିର ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#ଅନ୍ୟଥା
ବାହ୍ୟ ଶୂନ୍ୟ ffi_flush_icache (ଶୂନ୍ୟ *) FFI_HIDDEN;
#endif

ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_v9(ଶୂନ୍ୟ) FFI_HIDDEN;
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_v9(ଶୂନ୍ୟ) FFI_HIDDEN;

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
  unsigned long fn;

  ଯଦି (cif->abi != FFI_V9)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

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

  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*))
{
  ଯଦି (cif->abi != FFI_V9)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_v9;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା FFI_HIDDEN
ffi_closure_sparc_inner_v9(ffi_cif *cif,
			   ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			   ଶୂନ୍ୟ *user_data, ଶୂନ୍ୟ *rvalue,
			   unsigned long *gpr, unsigned long *fpr)
{
  ffi_type **arg_types;
  ଶୂନ୍ୟ **avalue;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * ଆକାର(ଶୂନ୍ୟ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  ଯଦି (flags & SPARC_FLAG_RET_IN_MEM)
    {
      rvalue = (ଶୂନ୍ୟ *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  ଅନ୍ୟଥା
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  ପାଇଁ (i = 0; i < nargs; i++, argn = argx)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      ଶୂନ୍ୟ *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      ବାଛ (ty->type)
	{
	ସ୍ଥିତି FFI_TYPE_COMPLEX:
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  z = ty->size;
	  ଯଦି (z > 16)
	    a = *(ଶୂନ୍ୟ **)a;
	  ଅନ୍ୟଥା
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      ଯଦି (named && argn < 16)
		{
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size_mask = ffi_struct_float_mask (ty, 0);
		  ପୂର୍ଣ୍ଣସଂଖ୍ୟା argn_mask = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  size_mask = (size_mask & 0xff) | (size_mask & argn_mask);
		  a = ffi_struct_float_merge (size_mask, gpr+argn, fpr+argn);
		}
	    }
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_DOUBLE:
	  ଯଦି (named && argn < 16)
	    a = fpr + argn;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ଯଦି (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  ଭାଙ୍ଗ;

	ସ୍ଥିତି FFI_TYPE_UINT64:
	ସ୍ଥିତି FFI_TYPE_SINT64:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_INT:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_SINT32:
	  a += 4;
	  ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT16:
        ସ୍ଥିତି FFI_TYPE_SINT16:
	  a += 6;
	  ଭାଙ୍ଗ;
        ସ୍ଥିତି FFI_TYPE_UINT8:
        ସ୍ଥିତି FFI_TYPE_SINT8:
	  a += 7;
	  ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_data);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ଫେରନ୍ତୁ flags;
}
#endif /* SPARC64 */

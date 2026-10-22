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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ਜੇ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ਬਾਹਰਲਾ ਦਾ date
# endif
#ਨਹੀਂਤਾਂ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ਸਥਿਰ ਪੂਰਨ
ffi_struct_float_ਨਕਾਬ (ffi_type *outer_type, ਪੂਰਨ ਮਾਪ_ਨਕਾਬ)
{
  ffi_type **elts;
  ffi_type *t;

  ਜੇ (outer_type->type == FFI_TYPE_COMPLEX)
    {
      ਪੂਰਨ m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->ਮਾਪ;

      ਜੇ (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      ਵਾਪਸ (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  ਲਈ (elts = outer_type->elements; (t = *elts) != ਨਿਰਾ; elts++)
    {
      size_t z = t->ਮਾਪ;
      ਪੂਰਨ o, m, tt;

      ਮਾਪ_ਨਕਾਬ = FFI_ALIGN(ਮਾਪ_ਨਕਾਬ, t->alignment);
      ਚੋਣ (t->type)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  ਮਾਪ_ਨਕਾਬ = ffi_struct_float_ਨਕਾਬ (t, ਮਾਪ_ਨਕਾਬ);
	  ਜਾਰੀ;
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  ਜੇ (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    ਰੋਕੋ;
	  /* FALLTHRU */
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (ਮਾਪ_ਨਕਾਬ >> 2) & 0x3f;	/* extract word offset */
	  ਮਾਪ_ਨਕਾਬ |= m << (o + 8);	/* insert mask into place */
	  ਰੋਕੋ;
	}
      ਮਾਪ_ਨਕਾਬ += z;
    }

  ਮਾਪ_ਨਕਾਬ = FFI_ALIGN(ਮਾਪ_ਨਕਾਬ, outer_type->alignment);
  FFI_ASSERT ((ਮਾਪ_ਨਕਾਬ & 0xff) == outer_type->ਮਾਪ);

  ਵਾਪਸ ਮਾਪ_ਨਕਾਬ;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ਸਥਿਰ ਖਾਲੀ *
ffi_struct_float_ਮਿਲਾਓ (ਪੂਰਨ ਮਾਪ_ਨਕਾਬ, ਖਾਲੀ *vi, ਖਾਲੀ *vf)
{
  ਪੂਰਨ ਮਾਪ = ਮਾਪ_ਨਕਾਬ & 0xff;
  ਪੂਰਨ ਨਕਾਬ = ਮਾਪ_ਨਕਾਬ >> 8;
  ਪੂਰਨ n = ਮਾਪ >> 2;

  ਜੇ (ਨਕਾਬ == 0)
    ਵਾਪਸ vi;
  ਨਹੀਂਤਾਂ ਜੇ (ਨਕਾਬ == (1 << n) - 1)
    ਵਾਪਸ vf;
  ਨਹੀਂਤਾਂ
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *wi = vi, *wf = vf;
      ਪੂਰਨ i;

      ਲਈ (i = 0; i < n; ++i)
	ਜੇ ((ਨਕਾਬ >> i) & 1)
	  wi[i] = wf[i];

      ਵਾਪਸ vi;
    }
}

/* Similar, but place the data into VD in the end.  */

ਖਾਲੀ FFI_HIDDEN
ffi_struct_float_copy (ਪੂਰਨ ਮਾਪ_ਨਕਾਬ, ਖਾਲੀ *vd, ਖਾਲੀ *vi, ਖਾਲੀ *vf)
{
  ਪੂਰਨ ਮਾਪ = ਮਾਪ_ਨਕਾਬ & 0xff;
  ਪੂਰਨ ਨਕਾਬ = ਮਾਪ_ਨਕਾਬ >> 8;
  ਪੂਰਨ n = ਮਾਪ >> 2;

  ਜੇ (ਨਕਾਬ == 0)
    ;
  ਨਹੀਂਤਾਂ ਜੇ (ਨਕਾਬ == (1 << n) - 1)
    vi = vf;
  ਨਹੀਂਤਾਂ
    {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *wd = vd, *wi = vi, *wf = vf;
      ਪੂਰਨ i;

      ਲਈ (i = 0; i < n; ++i)
	wd[i] = ((ਨਕਾਬ >> i) & 1 ? wf : wi)[i];
      ਵਾਪਸ;
    }
  ਸਮ੍ਰਿਤੀ_ਨਕਲ (vd, vi, ਮਾਪ);
}

/* Perform machine dependent cif processing */

ਸਥਿਰ ffi_ਸਥਿਤੀ
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ਪੂਰਨ rtt = rtype->type;
  size_t ਬਾਈਟਾਂ = 0;
  ਪੂਰਨ i, n, flags;

  /* Set the return type flag */
  ਚੋਣ (rtt)
    {
    ਹਾਲਤ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_COMPLEX:
    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (rtype->ਮਾਪ > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_ਵਿੱਚ_MEM;
	  ਬਾਈਟਾਂ = 8;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਪੂਰਨ ਮਾਪ_ਨਕਾਬ = ffi_struct_float_ਨਕਾਬ (rtype, 0);
	  ਪੂਰਨ ਸ਼ਬਦ_ਮਾਪ_2 = (ਮਾਪ_ਨਕਾਬ >> 2) & 0x3f;
	  ਪੂਰਨ all_ਨਕਾਬ = (1 << ਸ਼ਬਦ_ਮਾਪ_2) - 1;
	  ਪੂਰਨ fp_ਨਕਾਬ = ਮਾਪ_ਨਕਾਬ >> 8;

	  flags = (ਮਾਪ_ਨਕਾਬ << SPARC_SIZEMASK_ਖਿਸਕਾਓ) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  ਜੇ (fp_ਨਕਾਬ == 0)
	    {
	      ਜੇ (rtype->alignment >= 8)
		{
		  ਜੇ (rtype->ਮਾਪ == 8)
		    flags = SPARC_RET_INT64;
		  ਨਹੀਂਤਾਂ ਜੇ (rtype->ਮਾਪ == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  ਨਹੀਂਤਾਂ ਜੇ (fp_ਨਕਾਬ == all_ਨਕਾਬ)
	    ਚੋਣ (ਸ਼ਬਦ_ਮਾਪ_2)
	      {
	      ਹਾਲਤ 1: flags = SPARC_RET_F_1; ਰੋਕੋ;
	      ਹਾਲਤ 2: flags = SPARC_RET_F_2; ਰੋਕੋ;
	      ਹਾਲਤ 3: flags = SP_V9_RET_F_3; ਰੋਕੋ;
	      ਹਾਲਤ 4: flags = SPARC_RET_F_4; ਰੋਕੋ;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      ਹਾਲਤ 6: flags = SPARC_RET_F_6; ਰੋਕੋ;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      ਹਾਲਤ 8: flags = SPARC_RET_F_8; ਰੋਕੋ;
	      }
	}
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_INT:
    ਹਾਲਤ FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      ਰੋਕੋ;
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      flags = SPARC_RET_INT64;
      ਰੋਕੋ;

    ਮੂਲ:
      abort();
    }

  ਬਾਈਟਾਂ = 0;
  ਲਈ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->ਮਾਪ;
      size_t a = ty->alignment;

      ਚੋਣ (ty->type)
	{
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  ਜੇ (z > 16)
	    {
	      a = z = 8;
	      ਰੋਕੋ;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  ਜੇ (ਬਾਈਟਾਂ >= 16*8)
	    ਰੋਕੋ;
	  ਜੇ ((ffi_struct_float_ਨਕਾਬ (ty, 0) & 0xff00) == 0)
	    ਰੋਕੋ;
	  /* FALLTHRU */
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ਦਲੀਲਾਂ;
	  ਰੋਕੋ;
	}
      ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, a);
      ਬਾਈਟਾਂ += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  ਜੇ (ਬਾਈਟਾਂ < 6 * 8)
    ਬਾਈਟਾਂ = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  ਬਾਈਟਾਂ = FFI_ALIGN(ਬਾਈਟਾਂ, 16);

  /* Include the call frame to prep_args.  */
  ਬਾਈਟਾਂ += 8*16 + 8*8;

  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;
  cif->flags = flags;
  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ਵਾਪਸ ffi_prep_cif_machdep_core(cif);
}

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  ਵਾਪਸ ffi_prep_cif_machdep_core(cif);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_v9(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
			ਖਾਲੀ **avalue, size_t ਬਾਈਟਾਂ, ਖਾਲੀ *ਘੇਰਾ) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਪੂਰਨ FFI_HIDDEN
ffi_prep_ਦਲੀਲਾਂ_v9(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *argp, ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_type **p_arg;
  ਪੂਰਨ flags = cif->flags;
  ਪੂਰਨ i, nargs;

  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ (flags & SPARC_FLAG_RET_ਵਿੱਚ_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (ਅੱਖਰ *)argp + cif->ਬਾਈਟਾਂ;
	}
      ਨਹੀਂਤਾਂ
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#ਜੇ_ਅੰਤ

  ਜੇ (flags & SPARC_FLAG_RET_ਵਿੱਚ_MEM)
    *argp++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)rvalue;

  p_arg = cif->arg_types;
  ਲਈ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ਖਾਲੀ *a = avalue[i];
      size_t z;

      ਚੋਣ (ty->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  *argp++ = *(UINT64 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ਦਲੀਲਾਂ;
	  *argp++ = *(UINT32 *)a;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ਦਲੀਲਾਂ;
	  *argp++ = *(UINT64 *)a;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  z = ty->ਮਾਪ;
	  ਜੇ (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)a;
	      ਰੋਕੋ;
	    }
	  ਜੇ (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)argp & 15) && ty->alignment > 8)
	    argp++;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
    }

  ਵਾਪਸ flags;
}

ਸਥਿਰ ਖਾਲੀ
ffi_call_int(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  size_t ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;

  FFI_ASSERT (cif->abi == FFI_V9);

  ਜੇ (rvalue == ਨਿਰਾ && (cif->flags & SPARC_FLAG_RET_ਵਿੱਚ_MEM))
    ਬਾਈਟਾਂ += FFI_ALIGN (cif->rtype->ਮਾਪ, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT && ਮਾਪ > 4)
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -ਬਾਈਟਾਂ, ਘੇਰਾ);
}

ਖਾਲੀ
ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	    ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਘੇਰਾ);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GNUC__
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ
ffi_flush_icache (ਖਾਲੀ *p)
{
  __asm__ ਅਸਥਿਰ ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#ਨਹੀਂਤਾਂ
ਬਾਹਰੀ ਖਾਲੀ ffi_flush_icache (ਖਾਲੀ *) FFI_HIDDEN;
#ਜੇ_ਅੰਤ

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_v9(ਖਾਲੀ) FFI_HIDDEN;
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_v9(ਖਾਲੀ) FFI_HIDDEN;

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn;

  ਜੇ (cif->abi != FFI_V9)
    ਵਾਪਸ FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) &tramp[4]) = fn;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ffi_flush_icache (ਘੇਰਾ);

  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*))
{
  ਜੇ (cif->abi != FFI_V9)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_v9;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

ਪੂਰਨ FFI_HIDDEN
ffi_ਘੇਰਾ_sparc_inner_v9(ffi_cif *cif,
			   ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			   ਖਾਲੀ *user_ਅੰਕੜੇ, ਖਾਲੀ *rvalue,
			   ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *gpr, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *fpr)
{
  ffi_type **arg_types;
  ਖਾਲੀ **avalue;
  ਪੂਰਨ i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * ਆਕਾਰ(ਖਾਲੀ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  ਜੇ (flags & SPARC_FLAG_RET_ਵਿੱਚ_MEM)
    {
      rvalue = (ਖਾਲੀ *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  ਨਹੀਂਤਾਂ
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  ਲਈ (i = 0; i < nargs; i++, argn = argx)
    {
      ਪੂਰਨ named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      ਖਾਲੀ *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      ਚੋਣ (ty->type)
	{
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  z = ty->ਮਾਪ;
	  ਜੇ (z > 16)
	    a = *(ਖਾਲੀ **)a;
	  ਨਹੀਂਤਾਂ
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      ਜੇ (named && argn < 16)
		{
		  ਪੂਰਨ ਮਾਪ_ਨਕਾਬ = ffi_struct_float_ਨਕਾਬ (ty, 0);
		  ਪੂਰਨ argn_ਨਕਾਬ = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  ਮਾਪ_ਨਕਾਬ = (ਮਾਪ_ਨਕਾਬ & 0xff) | (ਮਾਪ_ਨਕਾਬ & argn_ਨਕਾਬ);
		  a = ffi_struct_float_ਮਿਲਾਓ (ਮਾਪ_ਨਕਾਬ, gpr+argn, fpr+argn);
		}
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (named && argn < 16)
	    a = fpr + argn;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਜੇ (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  a += 4;
	  ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT16:
        ਹਾਲਤ FFI_TYPE_SINT16:
	  a += 6;
	  ਰੋਕੋ;
        ਹਾਲਤ FFI_TYPE_UINT8:
        ਹਾਲਤ FFI_TYPE_SINT8:
	  a += 7;
	  ਰੋਕੋ;

	ਮੂਲ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ਵਾਪਸ flags;
}
#ਜੇ_ਅੰਤ /* SPARC64 */

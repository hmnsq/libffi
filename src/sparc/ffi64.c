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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ "internal.h"

/* Force FFI_TYPE_LONGDOUBLE to be different than FFI_TYPE_DOUBLE;
   all further uses in this file will refer to the 128-bit type.  */
#ಆದರೆ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
# ಆದರೆ FFI_TYPE_LONGDOUBLE != 4
#  error FFI_TYPE_LONGDOUBLE ಹೊರಹೋಗು ದ date
# endif
#ಇಲ್ಲದಿದ್ದರೆ
# undef FFI_TYPE_LONGDOUBLE
# define FFI_TYPE_LONGDOUBLE 4
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SPARC64

/* Flatten the contents of a structure to the parts that are passed in
   floating point registers.  The return is a bit mask wherein bit N
   set means bytes [4*n, 4*n+3] are passed in %fN.

   We encode both the (running) size (maximum 32) and mask (maxumum 255)
   into one integer.  The size is placed in the low byte, so that align
   and addition work correctly.  The mask is placed in the second byte.  */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_struct_float_ಮುಖವಾಡ (ffi_type *outer_type, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ಮುಖವಾಡ)
{
  ffi_type **elts;
  ffi_type *t;

  ಆದರೆ (outer_type->type == FFI_TYPE_COMPLEX)
    {
      ಪೂರ್ಣಾಂಕ m = 0, tt = outer_type->elements[0]->type;
      size_t z = outer_type->ಗಾತ್ರ_2;

      ಆದರೆ (tt == FFI_TYPE_FLOAT
	  || tt == FFI_TYPE_DOUBLE
	  || tt == FFI_TYPE_LONGDOUBLE)
        m = (1 << (z / 4)) - 1;
      ಹಿಂತಿರುಗಿ (m << 8) | z;
    }
  FFI_ASSERT (outer_type->type == FFI_TYPE_STRUCT);

  ಗಾಗಿ (elts = outer_type->elements; (t = *elts) != ಖಾಲಿ; elts++)
    {
      size_t z = t->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ o, m, tt;

      ಗಾತ್ರ_ಮುಖವಾಡ = FFI_ALIGN(ಗಾತ್ರ_ಮುಖವಾಡ, t->alignment);
      ಆಯ್ಕೆ (t->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  ಗಾತ್ರ_ಮುಖವಾಡ = ffi_struct_float_ಮುಖವಾಡ (t, ಗಾತ್ರ_ಮುಖವಾಡ);
	  ಮುಂದುವರಿ;
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  tt = t->elements[0]->type;
	  ಆದರೆ (tt != FFI_TYPE_FLOAT
	      && tt != FFI_TYPE_DOUBLE
	      && tt != FFI_TYPE_LONGDOUBLE)
	    ನಿಲ್ಲಿಸು;
	  /* FALLTHRU */
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  m = (1 << (z / 4)) - 1;	/* compute mask for type */
	  o = (ಗಾತ್ರ_ಮುಖವಾಡ >> 2) & 0x3f;	/* extract word offset */
	  ಗಾತ್ರ_ಮುಖವಾಡ |= m << (o + 8);	/* insert mask into place */
	  ನಿಲ್ಲಿಸು;
	}
      ಗಾತ್ರ_ಮುಖವಾಡ += z;
    }

  ಗಾತ್ರ_ಮುಖವಾಡ = FFI_ALIGN(ಗಾತ್ರ_ಮುಖವಾಡ, outer_type->alignment);
  FFI_ASSERT ((ಗಾತ್ರ_ಮುಖವಾಡ & 0xff) == outer_type->ಗಾತ್ರ_2);

  ಹಿಂತಿರುಗಿ ಗಾತ್ರ_ಮುಖವಾಡ;
}

/* Merge floating point data into integer data.  If the structure is
   entirely floating point, simply return a pointer to the fp data.  */

ಸ್ಥಿರ ಶೂನ್ಯ *
ffi_struct_float_ವಿಲೀನ (ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ಮುಖವಾಡ, ಶೂನ್ಯ *vi, ಶೂನ್ಯ *vf)
{
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ_ಮುಖವಾಡ & 0xff;
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = ಗಾತ್ರ_ಮುಖವಾಡ >> 8;
  ಪೂರ್ಣಾಂಕ n = ಗಾತ್ರ_2 >> 2;

  ಆದರೆ (ಮುಖವಾಡ == 0)
    ಹಿಂತಿರುಗಿ vi;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಖವಾಡ == (1 << n) - 1)
    ಹಿಂತಿರುಗಿ vf;
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *wi = vi, *wf = vf;
      ಪೂರ್ಣಾಂಕ i;

      ಗಾಗಿ (i = 0; i < n; ++i)
	ಆದರೆ ((ಮುಖವಾಡ >> i) & 1)
	  wi[i] = wf[i];

      ಹಿಂತಿರುಗಿ vi;
    }
}

/* Similar, but place the data into VD in the end.  */

ಶೂನ್ಯ FFI_HIDDEN
ffi_struct_float_copy (ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ಮುಖವಾಡ, ಶೂನ್ಯ *vd, ಶೂನ್ಯ *vi, ಶೂನ್ಯ *vf)
{
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = ಗಾತ್ರ_ಮುಖವಾಡ & 0xff;
  ಪೂರ್ಣಾಂಕ ಮುಖವಾಡ = ಗಾತ್ರ_ಮುಖವಾಡ >> 8;
  ಪೂರ್ಣಾಂಕ n = ಗಾತ್ರ_2 >> 2;

  ಆದರೆ (ಮುಖವಾಡ == 0)
    ;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಖವಾಡ == (1 << n) - 1)
    vi = vf;
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *wd = vd, *wi = vi, *wf = vf;
      ಪೂರ್ಣಾಂಕ i;

      ಗಾಗಿ (i = 0; i < n; ++i)
	wd[i] = ((ಮುಖವಾಡ >> i) & 1 ? wf : wi)[i];
      ಹಿಂತಿರುಗಿ;
    }
  ಸ್ಮರಣೆ_ನಕಲು (vd, vi, ಗಾತ್ರ_2);
}

/* Perform machine dependent cif processing */

ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
ffi_prep_cif_machdep_core(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ಪೂರ್ಣಾಂಕ rtt = rtype->type;
  size_t ಬೈಟುಗಳು = 0;
  ಪೂರ್ಣಾಂಕ i, n, flags;

  /* Set the return type flag */
  ಆಯ್ಕೆ (rtt)
    {
    ಸ್ಥಿತಿ FFI_TYPE_VOID:
      flags = SPARC_RET_VOID;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      flags = SPARC_RET_F_1;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
      flags = SPARC_RET_F_2;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
      flags = SPARC_RET_F_4;
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      ಆದರೆ (rtype->ಗಾತ್ರ_2 > 32)
	{
	  flags = SPARC_RET_VOID | SPARC_FLAG_RET_ಒಳ_MEM;
	  ಬೈಟುಗಳು = 8;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ಮುಖವಾಡ = ffi_struct_float_ಮುಖವಾಡ (rtype, 0);
	  ಪೂರ್ಣಾಂಕ ಶಬ್ದ_ಗಾತ್ರ_2 = (ಗಾತ್ರ_ಮುಖವಾಡ >> 2) & 0x3f;
	  ಪೂರ್ಣಾಂಕ all_ಮುಖವಾಡ = (1 << ಶಬ್ದ_ಗಾತ್ರ_2) - 1;
	  ಪೂರ್ಣಾಂಕ fp_ಮುಖವಾಡ = ಗಾತ್ರ_ಮುಖವಾಡ >> 8;

	  flags = (ಗಾತ್ರ_ಮುಖವಾಡ << SPARC_SIZEMASK_ಸರಿಸು) | SPARC_RET_STRUCT;

	  /* For special cases of all-int or all-fp, we can return
	     the value directly without popping through a struct copy.  */
	  ಆದರೆ (fp_ಮುಖವಾಡ == 0)
	    {
	      ಆದರೆ (rtype->alignment >= 8)
		{
		  ಆದರೆ (rtype->ಗಾತ್ರ_2 == 8)
		    flags = SPARC_RET_INT64;
		  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (rtype->ಗಾತ್ರ_2 == 16)
		    flags = SPARC_RET_INT128;
		}
	    }
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (fp_ಮುಖವಾಡ == all_ಮುಖವಾಡ)
	    ಆಯ್ಕೆ (ಶಬ್ದ_ಗಾತ್ರ_2)
	      {
	      ಸ್ಥಿತಿ 1: flags = SPARC_RET_F_1; ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ 2: flags = SPARC_RET_F_2; ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ 3: flags = SP_V9_RET_F_3; ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ 4: flags = SPARC_RET_F_4; ನಿಲ್ಲಿಸು;
	      /* 5 word structures skipped; handled via RET_STRUCT.  */
	      ಸ್ಥಿತಿ 6: flags = SPARC_RET_F_6; ನಿಲ್ಲಿಸು;
	      /* 7 word structures skipped; handled via RET_STRUCT.  */
	      ಸ್ಥಿತಿ 8: flags = SPARC_RET_F_8; ನಿಲ್ಲಿಸು;
	      }
	}
      ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
      flags = SPARC_RET_SINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
      flags = SPARC_RET_UINT8;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
      flags = SPARC_RET_SINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
      flags = SPARC_RET_UINT16;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_INT:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      flags = SP_V9_RET_SINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      flags = SPARC_RET_UINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = SPARC_RET_INT64;
      ನಿಲ್ಲಿಸು;

    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }

  ಬೈಟುಗಳು = 0;
  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->ಗಾತ್ರ_2;
      size_t a = ty->alignment;

      ಆಯ್ಕೆ (ty->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  /* Large structs passed by reference.  */
	  ಆದರೆ (z > 16)
	    {
	      a = z = 8;
	      ನಿಲ್ಲಿಸು;
	    }
	  /* Small structs may be passed in integer or fp regs or both.  */
	  ಆದರೆ (ಬೈಟುಗಳು >= 16*8)
	    ನಿಲ್ಲಿಸು;
	  ಆದರೆ ((ffi_struct_float_ಮುಖವಾಡ (ty, 0) & 0xff00) == 0)
	    ನಿಲ್ಲಿಸು;
	  /* FALLTHRU */
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  flags |= SPARC_FLAG_FP_ವಾದಗಳು;
	  ನಿಲ್ಲಿಸು;
	}
      ಬೈಟುಗಳು = FFI_ALIGN(ಬೈಟುಗಳು, a);
      ಬೈಟುಗಳು += FFI_ALIGN(z, 8);
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary. */
  ಆದರೆ (ಬೈಟುಗಳು < 6 * 8)
    ಬೈಟುಗಳು = 6 * 8;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  ಬೈಟುಗಳು = FFI_ALIGN(ಬೈಟುಗಳು, 16);

  /* Include the call frame to prep_args.  */
  ಬೈಟುಗಳು += 8*16 + 8*8;

  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;
  cif->flags = flags;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  cif->nfixedargs = cif->nargs;
  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_core(cif);
}

ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep_var(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ nfixedargs, ಚಿಹ್ನೆರಹಿತ ntotalargs)
{
  cif->nfixedargs = nfixedargs;
  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_core(cif);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_v9(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
			ಶೂನ್ಯ **avalue, size_t ಬೈಟುಗಳು, ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_prep_ವಾದಗಳು_v9(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *argp, ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_type **p_arg;
  ಪೂರ್ಣಾಂಕ flags = cif->flags;
  ಪೂರ್ಣಾಂಕ i, nargs;

  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ (flags & SPARC_FLAG_RET_ಒಳ_MEM)
	{
	  /* Since we pass the pointer to the callee, we need a value.
	     We allowed for this space in ffi_call, before ffi_call_v8
	     alloca'd the space.  */
	  rvalue = (ಅಕ್ಷರ *)argp + cif->ಬೈಟುಗಳು;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  /* Otherwise, we can ignore the return value.  */
	  flags = SPARC_RET_VOID;
	}
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*8);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (flags & SPARC_FLAG_RET_ಒಳ_MEM)
    *argp++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)rvalue;

  p_arg = cif->arg_types;
  ಗಾಗಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ಶೂನ್ಯ *a = avalue[i];
      size_t z;

      ಆಯ್ಕೆ (ty->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  *argp++ = *(SINT32 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  *argp++ = *(UINT32 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  *argp++ = *(UINT64 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  flags |= SPARC_FLAG_FP_ವಾದಗಳು;
	  *argp++ = *(UINT32 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  flags |= SPARC_FLAG_FP_ವಾದಗಳು;
	  *argp++ = *(UINT64 *)a;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  z = ty->ಗಾತ್ರ_2;
	  ಆದರೆ (z > 16)
	    {
	      /* For structures larger than 16 bytes we pass reference.  */
	      *argp++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)a;
	      ನಿಲ್ಲಿಸು;
	    }
	  ಆದರೆ (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)argp & 15) && ty->alignment > 8)
	    argp++;
	  ಸ್ಮರಣೆ_ನಕಲು(argp, a, z);
	  argp += FFI_ALIGN(z, 8) / 8;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
    }

  ಹಿಂತಿರುಗಿ flags;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  size_t ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;

  FFI_ASSERT (cif->abi == FFI_V9);

  ಆದರೆ (rvalue == ಖಾಲಿ && (cif->flags & SPARC_FLAG_RET_ಒಳ_MEM))
    ಬೈಟುಗಳು += FFI_ALIGN (cif->rtype->ಗಾತ್ರ_2, 16);

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT && ಗಾತ್ರ_2 > 4)
        {
          ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
          ಆದರೆ (avalue_copy == ಖಾಲಿ)
            {
              avalue_copy = alloca (nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
          avalue[i] = argcopy;
        }
    }
  
  ffi_call_v9(cif, fn, rvalue, avalue, -ಬೈಟುಗಳು, ಆವರಣ);
}

ಶೂನ್ಯ
ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	    ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ಆವರಣ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GNUC__
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ffi_flush_icache (ಶೂನ್ಯ *p)
{
  __asm__ ಅಸ್ಥಿರ ("flush	%0; flush %0+8" : : "r" (p) : "memory");
}
#ಇಲ್ಲದಿದ್ದರೆ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_flush_icache (ಶೂನ್ಯ *) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_v9(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_v9(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn;

  ಆದರೆ (cif->abi != FFI_V9)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  /* Trampoline address is equal to the closure address.  We take advantage
     of that to reduce the trampoline size by 8 bytes. */
  fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_v9;
  tramp[0] = 0x83414000;	/* rd	%pc, %g1	*/
  tramp[1] = 0xca586010;	/* ldx	[%g1+16], %g5	*/
  tramp[2] = 0x81c14000;	/* jmp	%g5		*/
  tramp[3] = 0x01000000;	/* nop			*/
  *((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) &tramp[4]) = fn;

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ffi_flush_icache (ಆವರಣ);

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಆದರೆ (cif->abi != FFI_V9)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = ffi_go_ಆವರಣ_v9;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_sparc_inner_v9(ffi_cif *cif,
			   ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *rvalue,
			   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *gpr, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *fpr)
{
  ffi_type **arg_types;
  ಶೂನ್ಯ **avalue;
  ಪೂರ್ಣಾಂಕ i, argn, argx, nargs, flags, nfixedargs;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  nfixedargs = cif->nfixedargs;

  avalue = alloca(nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  */
  ಆದರೆ (flags & SPARC_FLAG_RET_ಒಳ_MEM)
    {
      rvalue = (ಶೂನ್ಯ *) gpr[0];
      /* Skip the structure return address.  */
      argn = 1;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    argn = 0;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಗಾಗಿ (i = 0; i < nargs; i++, argn = argx)
    {
      ಪೂರ್ಣಾಂಕ named = i < nfixedargs;
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *a = &gpr[argn];
      size_t z;

      argx = argn + 1;
      ಆಯ್ಕೆ (ty->type)
	{
	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	  z = ty->ಗಾತ್ರ_2;
	  ಆದರೆ (z > 16)
	    a = *(ಶೂನ್ಯ **)a;
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      argx = argn + FFI_ALIGN (z, 8) / 8;
	      ಆದರೆ (named && argn < 16)
		{
		  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_ಮುಖವಾಡ = ffi_struct_float_ಮುಖವಾಡ (ty, 0);
		  ಪೂರ್ಣಾಂಕ argn_ಮುಖವಾಡ = (0xffff00 >> argn) & 0xff00;

		  /* Eliminate fp registers off the end.  */
		  ಗಾತ್ರ_ಮುಖವಾಡ = (ಗಾತ್ರ_ಮುಖವಾಡ & 0xff) | (ಗಾತ್ರ_ಮುಖವಾಡ & argn_ಮುಖವಾಡ);
		  a = ffi_struct_float_ವಿಲೀನ (ಗಾತ್ರ_ಮುಖವಾಡ, gpr+argn, fpr+argn);
		}
	    }
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  argn = FFI_ALIGN (argn, 2);
	  a = (named && argn < 16 ? fpr : gpr) + argn;
	  argx = argn + 2;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  ಆದರೆ (named && argn < 16)
	    a = fpr + argn;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಆದರೆ (named && argn < 16)
	    a = fpr + argn;
	  a += 4;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	  a += 4;
	  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_UINT16:
        ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  a += 6;
	  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_UINT8:
        ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  a += 7;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ flags;
}
#ಆದರೆ_ಅಂತ್ಯ /* SPARC64 */

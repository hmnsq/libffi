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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SPARC64

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

/* Perform machine dependent cif processing */
ffi_ಸ್ಥಿತಿ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  ffi_type *rtype = cif->rtype;
  ಪೂರ್ಣಾಂಕ rtt = rtype->type;
  size_t ಬೈಟುಗಳು;
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
    ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
      flags = (rtype->ಗಾತ್ರ_2 & 0xfff) << SPARC_SIZEMASK_ಸರಿಸು;
      flags |= SPARC_RET_STRUCT;
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
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
      flags = SPARC_RET_UINT32;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
      flags = SPARC_RET_INT64;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
      rtt = rtype->elements[0]->type;
      ಆಯ್ಕೆ (rtt)
	{
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  flags = SPARC_RET_F_2;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	  flags = SPARC_RET_F_4;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	  flags = SPARC_RET_F_8;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  flags = SPARC_RET_INT128;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	  flags = SPARC_RET_INT64;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  flags = SP_V8_RET_CPLX16;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  flags = SP_V8_RET_CPLX8;
	  ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      abort();
    }
  cif->flags = flags;

  ಬೈಟುಗಳು = 0;
  ಗಾಗಿ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = cif->arg_types[i];
      size_t z = ty->ಗಾತ್ರ_2;
      ಪೂರ್ಣಾಂಕ tt = ty->type;

      ಆಯ್ಕೆ (tt)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Passed by reference.  */
	  z = 4;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  ಆದರೆ (tt == FFI_TYPE_FLOAT || z > 8)
	    ಹೋಗು by_reference;
	  /* FALLTHRU */

	ಪೂರ್ವನಿಯೋಜಿತ:
	  z = FFI_ALIGN(z, 4);
	}
      ಬೈಟುಗಳು += z;
    }

  /* Sparc call frames require that space is allocated for 6 args,
     even if they aren't used. Make that space if necessary.  */
  ಆದರೆ (ಬೈಟುಗಳು < 6 * 4)
    ಬೈಟುಗಳು = 6 * 4;

  /* The ABI always requires space for the struct return pointer.  */
  ಬೈಟುಗಳು += 4;

  /* The stack must be 2 word aligned, so round bytes up appropriately. */
  ಬೈಟುಗಳು = FFI_ALIGN(ಬೈಟುಗಳು, 2 * 4);

  /* Include the call frame to prep_args.  */
  ಬೈಟುಗಳು += 4*16 + 4*8;
  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_v8(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
			ಶೂನ್ಯ **avalue, size_t ಬೈಟುಗಳು, ಶೂನ್ಯ *ಆವರಣ) FFI_HIDDEN;

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_prep_ವಾದಗಳು_v8(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *argp, ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_type **p_arg;
  ಪೂರ್ಣಾಂಕ flags = cif->flags;
  ಪೂರ್ಣಾಂಕ i, nargs;

  ಆದರೆ (rvalue == ಖಾಲಿ)
    {
      ಆದರೆ ((flags & SPARC_FLAG_RET_ಮುಖವಾಡ) == SPARC_RET_STRUCT)
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

  /* This could only really be done when we are returning a structure.
     However, the space is reserved so we can do it unconditionally.  */
  *argp++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)rvalue;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USING_PURIFY
  /* Purify will probably complain in our assembly routine,
     unless we zero out this memory. */
  memset(argp, 0, 6*4);
#ಆದರೆ_ಅಂತ್ಯ

  p_arg = cif->arg_types;
  ಗಾಗಿ (i = 0, nargs = cif->nargs; i < nargs; i++)
    {
      ffi_type *ty = p_arg[i];
      ಶೂನ್ಯ *a = avalue[i];
      ಪೂರ್ಣಾಂಕ tt = ty->type;
      size_t z;

      ಆಯ್ಕೆ (tt)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  *argp++ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)a;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	  ಸ್ಮರಣೆ_ನಕಲು(argp, a, 8);
	  argp += 2;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  *argp++ = *(ಚಿಹ್ನೆರಹಿತ *)a;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_UINT8:
	  *argp++ = *(UINT8 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  *argp++ = *(SINT8 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_UINT16:
	  *argp++ = *(UINT16 *)a;
	  ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  *argp++ = *(SINT16 *)a;
	  ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->ಗಾತ್ರ_2;
	  ಆದರೆ (tt == FFI_TYPE_FLOAT || z > 8)
	    ಹೋಗು by_reference;
	  ಆದರೆ (z < 4)
	    {
	      ಸ್ಮರಣೆ_ನಕಲು((ಅಕ್ಷರ *)argp + 4 - z, a, z);
	      argp++;
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      ಸ್ಮರಣೆ_ನಕಲು(argp, a, z);
	      argp += z / 4;
	    }
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
    }

  ಹಿಂತಿರುಗಿ flags;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  size_t ಬೈಟುಗಳು = cif->ಬೈಟುಗಳು;
  size_t i, nargs = cif->nargs;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;

  FFI_ASSERT (cif->abi == FFI_V8);

  /* If we've not got a return value, we need to create one if we've
     got to pass the return value to the callee.  Otherwise ignore it.  */
  ಆದರೆ (rvalue == ಖಾಲಿ
      && (cif->flags & SPARC_FLAG_RET_ಮುಖವಾಡ) == SPARC_RET_STRUCT)
    ಬೈಟುಗಳು += FFI_ALIGN (cif->rtype->ಗಾತ್ರ_2, 8);

  /* If we have any structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
      ಆದರೆ (at->type == FFI_TYPE_STRUCT)
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

  ffi_call_v8(cif, fn, rvalue, avalue, -ಬೈಟುಗಳು, ಆವರಣ);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GNUC__
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ
ffi_flush_icache (ಶೂನ್ಯ *p)
{
  /* SPARC v8 requires 5 instructions for flush to be visible */
  __asm__ ಅಸ್ಥಿರ ("iflush	%0; iflush %0+8; nop; nop; nop; nop; nop"
		: : "r" (p) : "memory");
}
#ಇಲ್ಲದಿದ್ದರೆ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_flush_icache (ಶೂನ್ಯ *) FFI_HIDDEN;
#ಆದರೆ_ಅಂತ್ಯ

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_v8(ಶೂನ್ಯ) FFI_HIDDEN;
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_v8(ಶೂನ್ಯ) FFI_HIDDEN;

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ctx = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ಆವರಣ;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_v8;

  ಆದರೆ (cif->abi != FFI_V8)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  tramp[0] = 0x03000000 | fn >> 10;	/* sethi %hi(fn), %g1	*/
  tramp[1] = 0x05000000 | ctx >> 10;	/* sethi %hi(ctx), %g2	*/
  tramp[2] = 0x81c06000 | (fn & 0x3ff);	/* jmp   %g1+%lo(fn)	*/
  tramp[3] = 0x8410a000 | (ctx & 0x3ff);/* or    %g2, %lo(ctx)	*/

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

  ffi_flush_icache (ಆವರಣ);

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ *ಆವರಣ, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*))
{
  ಆದರೆ (cif->abi != FFI_V8)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->tramp = ffi_go_ಆವರಣ_v8;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

ಪೂರ್ಣಾಂಕ FFI_HIDDEN
ffi_ಆವರಣ_sparc_inner_v8(ffi_cif *cif, 
			   ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			   ಶೂನ್ಯ *user_ದತ್ತಾಂಶ, ಶೂನ್ಯ *rvalue,
			   ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *argp)
{
  ffi_type **arg_types;
  ಶೂನ್ಯ **avalue;
  ಪೂರ್ಣಾಂಕ i, nargs, flags;

  arg_types = cif->arg_types;
  nargs = cif->nargs;
  flags = cif->flags;
  avalue = alloca(nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));

  /* Copy the caller's structure return address so that the closure
     returns the data directly to the caller.  Also install it so we
     can return the address in %o0.  */
  ಆದರೆ ((flags & SPARC_FLAG_RET_ಮುಖವಾಡ) == SPARC_RET_STRUCT)
    {
      ಶೂನ್ಯ *new_rvalue = (ಶೂನ್ಯ *)*argp;
      *(ಶೂನ್ಯ **)rvalue = new_rvalue;
      rvalue = new_rvalue;
    }

  /* Always skip the structure return address.  */
  argp++;

  /* Grab the addresses of the arguments from the stack frame.  */
  ಗಾಗಿ (i = 0; i < nargs; i++)
    {
      ffi_type *ty = arg_types[i];
      ಪೂರ್ಣಾಂಕ tt = ty->type;
      ಶೂನ್ಯ *a = argp;
      size_t z;

      ಆಯ್ಕೆ (tt)
	{
	ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	by_reference:
	  /* Straight copy of invisible reference.  */
	  a = (ಶೂನ್ಯ *)*argp;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	ಸ್ಥಿತಿ FFI_TYPE_UINT64:
	  ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)a & 7)
	    {
	      /* Align on a 8-byte boundary.  */
	      UINT64 *tmp = alloca(8);
	      *tmp = ((UINT64)argp[0] << 32) | argp[1];
	      a = tmp;
	    }
	  argp++;
	  ನಿಲ್ಲಿಸು;

	ಸ್ಥಿತಿ FFI_TYPE_INT:
	ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_UINT16:
        ಸ್ಥಿತಿ FFI_TYPE_SINT16:
	  a += 2;
	  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ FFI_TYPE_UINT8:
        ಸ್ಥಿತಿ FFI_TYPE_SINT8:
	  a += 3;
	  ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	  tt = ty->elements[0]->type;
	  z = ty->ಗಾತ್ರ_2;
	  ಆದರೆ (tt == FFI_TYPE_FLOAT || z > 8)
	    ಹೋಗು by_reference;
	  ಆದರೆ (z < 4)
	    a += 4 - z;
	  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (z > 4)
	    argp++;
	  ನಿಲ್ಲಿಸು;

	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
	}
      argp++;
      avalue[i] = a;
    }

  /* Invoke the closure.  */
  fun (cif, rvalue, avalue, user_ದತ್ತಾಂಶ);

  /* Tell ffi_closure_sparc how to perform return type promotions.  */
  ಹಿಂತಿರುಗಿ flags;
}
#ಆದರೆ_ಅಂತ್ಯ /* !SPARC64 */

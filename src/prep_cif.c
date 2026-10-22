/* -----------------------------------------------------------------------
   prep_cif.c - Copyright (c) 2011, 2012, 2021, 2025, 2026  Anthony Green
                Copyright (c) 1996, 1998, 2007  Red Hat, Inc.
                Copyright (c) 2022 Oracle and/or its affiliates.

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

/* Round up to FFI_SIZEOF_ARG. */

#ವ್ಯಾಖ್ಯಾನಿಸು ರಾಶಿ_ARG_ಗಾತ್ರ(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

/* Compute the machine-independent layout of a vector (SIMD) type.

   A vector is described exactly like a struct -- arg->elements is a
   NULL-terminated array of pointers -- but every element must point to the
   SAME fundamental scalar type, and the count is the number of lanes.  The
   caller leaves arg->size and arg->alignment as zero; libffi derives them:

     size      = lane_size * lane_count, rounded UP to the next power of two
                 (matching Clang's ext_vector_type storage, e.g. 3 x float
                 -> 16; GCC's vector_size already requires power-of-two totals
                 so the rule is identical there);
     alignment = min(size, 16).

   Only float, double and the fixed-width integer scalars (UINT8..SINT64) are
   valid lane types.  Anything else -- a heterogeneous element list, an
   aggregate lane, long double, or a zero-length vector -- is FFI_BAD_TYPEDEF. */

ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ
initialize_ಸದಿಶ (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t ಎಣಿಕೆ_2 = 0;
  size_t ಒಟ್ಟು, p2;

  ಆದರೆ (UNLIKELY (ptr == ಖಾಲಿ || *ptr == ಖಾಲಿ))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  elem = *ptr;
  ಆಯ್ಕೆ (elem->type)
    {
    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
    ಸ್ಥಿತಿ FFI_TYPE_UINT8:
    ಸ್ಥಿತಿ FFI_TYPE_SINT8:
    ಸ್ಥಿತಿ FFI_TYPE_UINT16:
    ಸ್ಥಿತಿ FFI_TYPE_SINT16:
    ಸ್ಥಿತಿ FFI_TYPE_UINT32:
    ಸ್ಥಿತಿ FFI_TYPE_SINT32:
    ಸ್ಥಿತಿ FFI_TYPE_UINT64:
    ಸ್ಥಿತಿ FFI_TYPE_SINT64:
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  ಗಾಗಿ (; *ptr != ಖಾಲಿ; ptr++)
    {
      ಆದರೆ ((*ptr)->type != elem->type || (*ptr)->ಗಾತ್ರ_2 != elem->ಗಾತ್ರ_2)
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
      ಎಣಿಕೆ_2++;
    }

  ಆದರೆ (UNLIKELY (ಎಣಿಕೆ_2 < 1 || elem->ಗಾತ್ರ_2 == 0))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  ಒಟ್ಟು = elem->ಗಾತ್ರ_2 * ಎಣಿಕೆ_2;
  ಗಾಗಿ (p2 = 1; p2 < ಒಟ್ಟು; p2 <<= 1)
    ;

  arg->ಗಾತ್ರ_2 = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  ಹಿಂತಿರುಗಿ FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  ಆದರೆ (UNLIKELY(arg == ಖಾಲಿ || arg->elements == ಖಾಲಿ))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  ಆದರೆ (arg->type == FFI_TYPE_ಸದಿಶ)
    ಹಿಂತಿರುಗಿ initialize_ಸದಿಶ (arg);

  arg->ಗಾತ್ರ_2 = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  ಆದರೆ (UNLIKELY(ptr == 0))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

  ಆಗ ((*ptr) != ಖಾಲಿ)
    {
      ಆದರೆ (UNLIKELY(((*ptr)->ಗಾತ್ರ_2 == 0)
		    && (initialize_aggregate((*ptr), ಖಾಲಿ) != FFI_OK)))
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->ಗಾತ್ರ_2 = FFI_ALIGN(arg->ಗಾತ್ರ_2, (*ptr)->alignment);
      ಆದರೆ (offsets)
	*offsets++ = arg->ಗಾತ್ರ_2;
      arg->ಗಾತ್ರ_2 += (*ptr)->ಗಾತ್ರ_2;

      arg->alignment = (arg->alignment > (*ptr)->alignment) ?
	arg->alignment : (*ptr)->alignment;

      ptr++;
    }

  /* Structure size includes tail padding.  This is important for
     structures that fit in one register on ABIs like the PowerPC64
     Linux ABI that right justify small structs in a register.
     It's also needed for nested structure layout, for example
     struct A { long a; char b; }; struct B { struct A x; char y; };
     should find y at an offset of 2*sizeof(long) and result in a
     total size of 3*sizeof(long).  */
  arg->ಗಾತ್ರ_2 = FFI_ALIGN (arg->ಗಾತ್ರ_2, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_AGGREGATE_ALIGNMENT
  ಆದರೆ (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (arg->ಗಾತ್ರ_2 == 0)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ FFI_OK;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FFI_ಗುರಿ_HAS_ಸದಿಶ_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
ffi_type_ಒಳಗೊಂಡಿದೆ_ಸದಿಶ (ffi_type *ty)
{
  ffi_type **p;

  ಆದರೆ (ty == ಖಾಲಿ)
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (ty->type == FFI_TYPE_ಸದಿಶ)
    ಹಿಂತಿರುಗಿ 1;
  ಆದರೆ (ty->type == FFI_TYPE_STRUCT && ty->elements != ಖಾಲಿ)
    ಗಾಗಿ (p = ty->elements; *p != ಖಾಲಿ; p++)
      ಆದರೆ (ffi_type_ಒಳಗೊಂಡಿದೆ_ಸದಿಶ (*p))
	ಹಿಂತಿರುಗಿ 1;
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* !FFI_TARGET_HAS_VECTOR_TYPE */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_ಸ್ಥಿತಿ FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ isvariadic,
                             ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
                             ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು = 0;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;
  ffi_type **ptr;

  FFI_ASSERT(cif != ಖಾಲಿ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  ಆದರೆ (! (abi > FFI_ಮೊದಲ_ABI && abi < FFI_ಕೊನೆಯ_ABI))
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FFI_ಗುರಿ_HAS_ಸದಿಶ_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  ಆದರೆ (ffi_type_ಒಳಗೊಂಡಿದೆ_ಸದಿಶ (rtype))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
  ಗಾಗಿ (i = 0; i < ntotalargs; i++)
    ಆದರೆ (ffi_type_ಒಳಗೊಂಡಿದೆ_ಸದಿಶ (atypes[i]))
      ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;
#ಆದರೆ_ಅಂತ್ಯ

  cif->flags = 0;
#ಆದರೆ (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#ಆದರೆ_ಅಂತ್ಯ

  /* Initialize the return type if necessary */
  ಆದರೆ ((cif->rtype->ಗಾತ್ರ_2 == 0)
      && (initialize_aggregate(cif->rtype, ಖಾಲಿ) != FFI_OK))
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
  ಆದರೆ (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#ಆದರೆ_ಅಂತ್ಯ
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#ಆದರೆ !defined FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ
  /* Make space for the return structure pointer */
  ಆದರೆ ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_ಸದಿಶ)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TILE
      && (cif->rtype->ಗಾತ್ರ_2 > 10 * FFI_SIZEOF_ARG)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ XTENSA
      && (cif->rtype->ಗಾತ್ರ_2 > 16)
#ಆದರೆ_ಅಂತ್ಯ
     )
    ಬೈಟುಗಳು = ರಾಶಿ_ARG_ಗಾತ್ರ(ಗಾತ್ರ(ಶೂನ್ಯ*));
#ಆದರೆ_ಅಂತ್ಯ

  ಗಾಗಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      ಆದರೆ (((*ptr)->ಗಾತ್ರ_2 == 0)
	  && (initialize_aggregate((*ptr), ಖಾಲಿ) != FFI_OK))
	ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ FFI_ಗುರಿ_HAS_COMPLEX_TYPE
      ಆದರೆ ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#ಆದರೆ_ಅಂತ್ಯ
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#ಆದರೆ !defined FFI_ಗುರಿ_SPECIFIC_ರಾಶಿ_SPACE_ಹಂಚಿಕೆ
	{
	  /* Add any padding if necessary */
	  ಆದರೆ (((*ptr)->alignment - 1) & ಬೈಟುಗಳು)
	    ಬೈಟುಗಳು = (ಚಿಹ್ನೆರಹಿತ)FFI_ALIGN(ಬೈಟುಗಳು, (*ptr)->alignment);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TILE
	  ಆದರೆ (ಬೈಟುಗಳು < 10 * FFI_SIZEOF_ARG &&
	      ಬೈಟುಗಳು + ರಾಶಿ_ARG_ಗಾತ್ರ((*ptr)->ಗಾತ್ರ_2) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      ಬೈಟುಗಳು = 10 * FFI_SIZEOF_ARG;
	    }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ XTENSA
	  ಆದರೆ (ಬೈಟುಗಳು <= 6*4 && ಬೈಟುಗಳು + ರಾಶಿ_ARG_ಗಾತ್ರ((*ptr)->ಗಾತ್ರ_2) > 6*4)
	    ಬೈಟುಗಳು = 6*4;
#ಆದರೆ_ಅಂತ್ಯ

	  ಬೈಟುಗಳು += (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ರಾಶಿ_ARG_ಗಾತ್ರ((*ptr)->ಗಾತ್ರ_2);
	}
#ಆದರೆ_ಅಂತ್ಯ
    }

  cif->ಬೈಟುಗಳು = ಬೈಟುಗಳು;

  /* Perform machine dependent cif processing */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_ಗುರಿ_SPECIFIC_VARIADIC
  ಆದರೆ (isvariadic)
	ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep(cif);
}
#ಆದರೆ_ಅಂತ್ಯ /* not __CRIS__ */

ffi_ಸ್ಥಿತಿ ffi_prep_cif(ffi_cif *cif, ffi_abi abi, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  ಹಿಂತಿರುಗಿ ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_ಸ್ಥಿತಿ ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs,
                            ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_ಸ್ಥಿತಿ rc;
  size_t int_ಗಾತ್ರ = ffi_type_sint.ಗಾತ್ರ_2;
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  ಆದರೆ (rc != FFI_OK)
    ಹಿಂತಿರುಗಿ rc;

  ಗಾಗಿ (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      ಆದರೆ (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->ಗಾತ್ರ_2 < int_ಗಾತ್ರ))
        ಹಿಂತಿರುಗಿ FFI_BAD_ARGTYPE;
    }

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ FFI_CLOSURES

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ (ffi_ಆವರಣ* ಆವರಣ,
		  ffi_cif* cif,
		  ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		  ಶೂನ್ಯ *user_ದತ್ತಾಂಶ)
{
  ಹಿಂತಿರುಗಿ ffi_prep_ಆವರಣ_loc (ಆವರಣ, cif, fun, user_ದತ್ತಾಂಶ, ಆವರಣ);
}

#ಆದರೆ_ಅಂತ್ಯ

ffi_ಸ್ಥಿತಿ
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  ಆದರೆ (! (abi > FFI_ಮೊದಲ_ABI && abi < FFI_ಕೊನೆಯ_ABI))
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  ಆದರೆ (struct_type->type != FFI_TYPE_STRUCT)
    ಹಿಂತಿರುಗಿ FFI_BAD_TYPEDEF;

#ಆದರೆ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#ಆದರೆ !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

ರಚನೆ ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = ಸ್ಮರಣೆ_ಕೊಡು (ಗಾತ್ರ (ರಚನೆ ffi_call_plan));
  ಆದರೆ (plan != ಖಾಲಿ)
    plan->cif = cif;
  ಹಿಂತಿರುಗಿ plan;
}

ಶೂನ್ಯ
ffi_call_plan_invoke (ffi_call_plan *plan, ಶೂನ್ಯ (*fn) (ಶೂನ್ಯ),
		      ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

ಶೂನ್ಯ
ffi_call_plan_free (ffi_call_plan *plan)
{
  ಬಿಡುಗಡೆ (plan);
}

size_t
ffi_call_plan_ಗಾತ್ರ (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  ಹಿಂತಿರುಗಿ plan != ಖಾಲಿ ? ಗಾತ್ರ (ರಚನೆ ffi_call_plan) : 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* generic ffi_call_plan fallback */

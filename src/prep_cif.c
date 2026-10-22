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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਢੇਰ_ARG_ਮਾਪ(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

ਸਥਿਰ ffi_ਸਥਿਤੀ
initialize_ਸਦਿਸ਼ (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t ਗਿਣਤੀ_2 = 0;
  size_t ਕੁੱਲ, p2;

  ਜੇ (UNLIKELY (ptr == ਨਿਰਾ || *ptr == ਨਿਰਾ))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

  elem = *ptr;
  ਚੋਣ (elem->type)
    {
    ਹਾਲਤ FFI_TYPE_FLOAT:
    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_UINT8:
    ਹਾਲਤ FFI_TYPE_SINT8:
    ਹਾਲਤ FFI_TYPE_UINT16:
    ਹਾਲਤ FFI_TYPE_SINT16:
    ਹਾਲਤ FFI_TYPE_UINT32:
    ਹਾਲਤ FFI_TYPE_SINT32:
    ਹਾਲਤ FFI_TYPE_UINT64:
    ਹਾਲਤ FFI_TYPE_SINT64:
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  ਲਈ (; *ptr != ਨਿਰਾ; ptr++)
    {
      ਜੇ ((*ptr)->type != elem->type || (*ptr)->ਮਾਪ != elem->ਮਾਪ)
	ਵਾਪਸ FFI_BAD_TYPEDEF;
      ਗਿਣਤੀ_2++;
    }

  ਜੇ (UNLIKELY (ਗਿਣਤੀ_2 < 1 || elem->ਮਾਪ == 0))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

  ਕੁੱਲ = elem->ਮਾਪ * ਗਿਣਤੀ_2;
  ਲਈ (p2 = 1; p2 < ਕੁੱਲ; p2 <<= 1)
    ;

  arg->ਮਾਪ = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  ਵਾਪਸ FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

ਸਥਿਰ ffi_ਸਥਿਤੀ initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  ਜੇ (UNLIKELY(arg == ਨਿਰਾ || arg->elements == ਨਿਰਾ))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

  ਜੇ (arg->type == FFI_TYPE_ਸਦਿਸ਼)
    ਵਾਪਸ initialize_ਸਦਿਸ਼ (arg);

  arg->ਮਾਪ = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  ਜੇ (UNLIKELY(ptr == 0))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

  ਜਦੋਂ ((*ptr) != ਨਿਰਾ)
    {
      ਜੇ (UNLIKELY(((*ptr)->ਮਾਪ == 0)
		    && (initialize_aggregate((*ptr), ਨਿਰਾ) != FFI_OK)))
	ਵਾਪਸ FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->ਮਾਪ = FFI_ALIGN(arg->ਮਾਪ, (*ptr)->alignment);
      ਜੇ (offsets)
	*offsets++ = arg->ਮਾਪ;
      arg->ਮਾਪ += (*ptr)->ਮਾਪ;

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
  arg->ਮਾਪ = FFI_ALIGN (arg->ਮਾਪ, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_AGGREGATE_ALIGNMENT
  ਜੇ (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#ਜੇ_ਅੰਤ

  ਜੇ (arg->ਮਾਪ == 0)
    ਵਾਪਸ FFI_BAD_TYPEDEF;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_ਸਦਿਸ਼_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
ਸਥਿਰ ਪੂਰਨ
ffi_type_ਸ਼ਾਮਲ_ਹੈ_ਸਦਿਸ਼ (ffi_type *ty)
{
  ffi_type **p;

  ਜੇ (ty == ਨਿਰਾ)
    ਵਾਪਸ 0;
  ਜੇ (ty->type == FFI_TYPE_ਸਦਿਸ਼)
    ਵਾਪਸ 1;
  ਜੇ (ty->type == FFI_TYPE_STRUCT && ty->elements != ਨਿਰਾ)
    ਲਈ (p = ty->elements; *p != ਨਿਰਾ; p++)
      ਜੇ (ffi_type_ਸ਼ਾਮਲ_ਹੈ_ਸਦਿਸ਼ (*p))
	ਵਾਪਸ 1;
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* !FFI_TARGET_HAS_VECTOR_TYPE */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_ਸਥਿਤੀ FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ isvariadic,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਈਟਾਂ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ffi_type **ptr;

  FFI_ASSERT(cif != ਨਿਰਾ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  ਜੇ (! (abi > FFI_ਪਹਿਲਾ_ABI && abi < FFI_ਆਖਰੀ_ABI))
    ਵਾਪਸ FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_ਸਦਿਸ਼_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  ਜੇ (ffi_type_ਸ਼ਾਮਲ_ਹੈ_ਸਦਿਸ਼ (rtype))
    ਵਾਪਸ FFI_BAD_TYPEDEF;
  ਲਈ (i = 0; i < ntotalargs; i++)
    ਜੇ (ffi_type_ਸ਼ਾਮਲ_ਹੈ_ਸਦਿਸ਼ (atypes[i]))
      ਵਾਪਸ FFI_BAD_TYPEDEF;
#ਜੇ_ਅੰਤ

  cif->flags = 0;
#ਜੇ (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#ਜੇ_ਅੰਤ
#ਜੇ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#ਜੇ_ਅੰਤ

  /* Initialize the return type if necessary */
  ਜੇ ((cif->rtype->ਮਾਪ == 0)
      && (initialize_aggregate(cif->rtype, ਨਿਰਾ) != FFI_OK))
    ਵਾਪਸ FFI_BAD_TYPEDEF;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
  ਜੇ (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#ਜੇ_ਅੰਤ
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#ਜੇ !defined FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_ਢੇਰ_SPACE_ਵੰਡ
  /* Make space for the return structure pointer */
  ਜੇ ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_ਸਦਿਸ਼)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TILE
      && (cif->rtype->ਮਾਪ > 10 * FFI_SIZEOF_ARG)
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ XTENSA
      && (cif->rtype->ਮਾਪ > 16)
#ਜੇ_ਅੰਤ
     )
    ਬਾਈਟਾਂ = ਢੇਰ_ARG_ਮਾਪ(ਆਕਾਰ(ਖਾਲੀ*));
#ਜੇ_ਅੰਤ

  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      ਜੇ (((*ptr)->ਮਾਪ == 0)
	  && (initialize_aggregate((*ptr), ਨਿਰਾ) != FFI_OK))
	ਵਾਪਸ FFI_BAD_TYPEDEF;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_HAS_COMPLEX_TYPE
      ਜੇ ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#ਜੇ_ਅੰਤ
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#ਜੇ !defined FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_ਢੇਰ_SPACE_ਵੰਡ
	{
	  /* Add any padding if necessary */
	  ਜੇ (((*ptr)->alignment - 1) & ਬਾਈਟਾਂ)
	    ਬਾਈਟਾਂ = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_ALIGN(ਬਾਈਟਾਂ, (*ptr)->alignment);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TILE
	  ਜੇ (ਬਾਈਟਾਂ < 10 * FFI_SIZEOF_ARG &&
	      ਬਾਈਟਾਂ + ਢੇਰ_ARG_ਮਾਪ((*ptr)->ਮਾਪ) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      ਬਾਈਟਾਂ = 10 * FFI_SIZEOF_ARG;
	    }
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ XTENSA
	  ਜੇ (ਬਾਈਟਾਂ <= 6*4 && ਬਾਈਟਾਂ + ਢੇਰ_ARG_ਮਾਪ((*ptr)->ਮਾਪ) > 6*4)
	    ਬਾਈਟਾਂ = 6*4;
#ਜੇ_ਅੰਤ

	  ਬਾਈਟਾਂ += (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਢੇਰ_ARG_ਮਾਪ((*ptr)->ਮਾਪ);
	}
#ਜੇ_ਅੰਤ
    }

  cif->ਬਾਈਟਾਂ = ਬਾਈਟਾਂ;

  /* Perform machine dependent cif processing */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_ਨਿਸ਼ਾਨਾ_SPECIFIC_VARIADIC
  ਜੇ (isvariadic)
	ਵਾਪਸ ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ ffi_prep_cif_machdep(cif);
}
#ਜੇ_ਅੰਤ /* not __CRIS__ */

ffi_ਸਥਿਤੀ ffi_prep_cif(ffi_cif *cif, ffi_abi abi, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  ਵਾਪਸ ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_ਸਥਿਤੀ ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
                            ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_ਸਥਿਤੀ rc;
  size_t int_ਮਾਪ = ffi_type_sint.ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  ਜੇ (rc != FFI_OK)
    ਵਾਪਸ rc;

  ਲਈ (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      ਜੇ (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->ਮਾਪ < int_ਮਾਪ))
        ਵਾਪਸ FFI_BAD_ARGTYPE;
    }

  ਵਾਪਸ FFI_OK;
}

#ਜੇ FFI_CLOSURES

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ (ffi_ਘੇਰਾ* ਘੇਰਾ,
		  ffi_cif* cif,
		  ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		  ਖਾਲੀ *user_ਅੰਕੜੇ)
{
  ਵਾਪਸ ffi_prep_ਘੇਰਾ_loc (ਘੇਰਾ, cif, fun, user_ਅੰਕੜੇ, ਘੇਰਾ);
}

#ਜੇ_ਅੰਤ

ffi_ਸਥਿਤੀ
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  ਜੇ (! (abi > FFI_ਪਹਿਲਾ_ABI && abi < FFI_ਆਖਰੀ_ABI))
    ਵਾਪਸ FFI_BAD_ABI;
  ਜੇ (struct_type->type != FFI_TYPE_STRUCT)
    ਵਾਪਸ FFI_BAD_TYPEDEF;

#ਜੇ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#ਜੇ_ਅੰਤ

  ਵਾਪਸ initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#ਜੇ !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

ਬਣਤਰ ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = ਸਮ੍ਰਿਤੀ_ਦਿਓ (ਆਕਾਰ (ਬਣਤਰ ffi_call_plan));
  ਜੇ (plan != ਨਿਰਾ)
    plan->cif = cif;
  ਵਾਪਸ plan;
}

ਖਾਲੀ
ffi_call_plan_invoke (ffi_call_plan *plan, ਖਾਲੀ (*fn) (ਖਾਲੀ),
		      ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

ਖਾਲੀ
ffi_call_plan_free (ffi_call_plan *plan)
{
  ਮੁਕਤ (plan);
}

size_t
ffi_call_plan_ਮਾਪ (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  ਵਾਪਸ plan != ਨਿਰਾ ? ਆਕਾਰ (ਬਣਤਰ ffi_call_plan) : 0;
}

#ਜੇ_ਅੰਤ /* generic ffi_call_plan fallback */

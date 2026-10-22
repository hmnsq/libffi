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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#define تر_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

ساكن ffi_status
initialize_vector (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t كدي = 0;
  size_t total, p2;

  إذا (UNLIKELY (ptr == عدم || *ptr == عدم))
    أرجع FFI_BAD_TYPEDEF;

  elem = *ptr;
  بدل (elem->type)
    {
    حالة FFI_TYPE_FLOAT:
    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_UINT8:
    حالة FFI_TYPE_SINT8:
    حالة FFI_TYPE_UINT16:
    حالة FFI_TYPE_SINT16:
    حالة FFI_TYPE_UINT32:
    حالة FFI_TYPE_SINT32:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_SINT64:
      اكسر;
    افتراضي:
      أرجع FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  لكل (; *ptr != عدم; ptr++)
    {
      إذا ((*ptr)->type != elem->type || (*ptr)->size != elem->size)
	أرجع FFI_BAD_TYPEDEF;
      كدي++;
    }

  إذا (UNLIKELY (كدي < 1 || elem->size == 0))
    أرجع FFI_BAD_TYPEDEF;

  total = elem->size * كدي;
  لكل (p2 = 1; p2 < total; p2 <<= 1)
    ;

  arg->size = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  أرجع FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

ساكن ffi_status initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  إذا (UNLIKELY(arg == عدم || arg->elements == عدم))
    أرجع FFI_BAD_TYPEDEF;

  إذا (arg->type == FFI_TYPE_VECTOR)
    أرجع initialize_vector (arg);

  arg->size = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  إذا (UNLIKELY(ptr == 0))
    أرجع FFI_BAD_TYPEDEF;

  طالما ((*ptr) != عدم)
    {
      إذا (UNLIKELY(((*ptr)->size == 0)
		    && (initialize_aggregate((*ptr), عدم) != FFI_OK)))
	أرجع FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->size = FFI_ALIGN(arg->size, (*ptr)->alignment);
      إذا (offsets)
	*offsets++ = arg->size;
      arg->size += (*ptr)->size;

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
  arg->size = FFI_ALIGN (arg->size, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#ifdef FFI_AGGREGATE_ALIGNMENT
  إذا (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#endif

  إذا (arg->size == 0)
    أرجع FFI_BAD_TYPEDEF;
  وإلا
    أرجع FFI_OK;
}

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
ساكن صحيح
ffi_type_contains_vector (ffi_type *ty)
{
  ffi_type **p;

  إذا (ty == عدم)
    أرجع 0;
  إذا (ty->type == FFI_TYPE_VECTOR)
    أرجع 1;
  إذا (ty->type == FFI_TYPE_STRUCT && ty->elements != عدم)
    لكل (p = ty->elements; *p != عدم; p++)
      إذا (ffi_type_contains_vector (*p))
	أرجع 1;
  أرجع 0;
}
#endif /* !FFI_TARGET_HAS_VECTOR_TYPE */

#ifndef __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_status FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     unsigned صحيح isvariadic,
                             unsigned صحيح nfixedargs,
                             unsigned صحيح ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  unsigned bytes = 0;
  unsigned صحيح i;
  ffi_type **ptr;

  FFI_ASSERT(cif != عدم);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  إذا (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    أرجع FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  إذا (ffi_type_contains_vector (rtype))
    أرجع FFI_BAD_TYPEDEF;
  لكل (i = 0; i < ntotalargs; i++)
    إذا (ffi_type_contains_vector (atypes[i]))
      أرجع FFI_BAD_TYPEDEF;
#endif

  cif->flags = 0;
#إذا (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#endif
#إذا HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  /* Initialize the return type if necessary */
  إذا ((cif->rtype->size == 0)
      && (initialize_aggregate(cif->rtype, عدم) != FFI_OK))
    أرجع FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
  إذا (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#endif
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#إذا !defined FFI_TARGET_SPECIFIC_تر_SPACE_ALLOCATION
  /* Make space for the return structure pointer */
  إذا ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_VECTOR)
#ifdef TILE
      && (cif->rtype->size > 10 * FFI_SIZEOF_ARG)
#endif
#ifdef XTENSA
      && (cif->rtype->size > 16)
#endif
     )
    bytes = تر_ARG_SIZE(حجم(فراغ*));
#endif

  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      إذا (((*ptr)->size == 0)
	  && (initialize_aggregate((*ptr), عدم) != FFI_OK))
	أرجع FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
      إذا ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#endif
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#إذا !defined FFI_TARGET_SPECIFIC_تر_SPACE_ALLOCATION
	{
	  /* Add any padding if necessary */
	  إذا (((*ptr)->alignment - 1) & bytes)
	    bytes = (unsigned)FFI_ALIGN(bytes, (*ptr)->alignment);

#ifdef TILE
	  إذا (bytes < 10 * FFI_SIZEOF_ARG &&
	      bytes + تر_ARG_SIZE((*ptr)->size) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      bytes = 10 * FFI_SIZEOF_ARG;
	    }
#endif
#ifdef XTENSA
	  إذا (bytes <= 6*4 && bytes + تر_ARG_SIZE((*ptr)->size) > 6*4)
	    bytes = 6*4;
#endif

	  bytes += (unsigned صحيح)تر_ARG_SIZE((*ptr)->size);
	}
#endif
    }

  cif->bytes = bytes;

  /* Perform machine dependent cif processing */
#ifdef FFI_TARGET_SPECIFIC_VARIADIC
  إذا (isvariadic)
	أرجع ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#endif

  أرجع ffi_prep_cif_machdep(cif);
}
#endif /* not __CRIS__ */

ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, unsigned صحيح nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  أرجع ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_status ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            unsigned صحيح nfixedargs,
                            unsigned صحيح ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_status rc;
  size_t int_size = ffi_type_sint.size;
  unsigned صحيح i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  إذا (rc != FFI_OK)
    أرجع rc;

  لكل (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      إذا (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->size < int_size))
        أرجع FFI_BAD_ARGTYPE;
    }

  أرجع FFI_OK;
}

#إذا FFI_CLOSURES

ffi_status
ffi_prep_closure (ffi_closure* closure,
		  ffi_cif* cif,
		  فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		  فراغ *user_data)
{
  أرجع ffi_prep_closure_loc (closure, cif, fun, user_data, closure);
}

#endif

ffi_status
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  إذا (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    أرجع FFI_BAD_ABI;
  إذا (struct_type->type != FFI_TYPE_STRUCT)
    أرجع FFI_BAD_TYPEDEF;

#إذا HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  أرجع initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#إذا !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

بنية ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (حجم (بنية ffi_call_plan));
  إذا (plan != عدم)
    plan->cif = cif;
  أرجع plan;
}

فراغ
ffi_call_plan_invoke (ffi_call_plan *plan, فراغ (*fn) (فراغ),
		      فراغ *rvalue, فراغ **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

فراغ
ffi_call_plan_free (ffi_call_plan *plan)
{
  حرر (plan);
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  أرجع plan != عدم ? حجم (بنية ffi_call_plan) : 0;
}

#endif /* generic ffi_call_plan fallback */

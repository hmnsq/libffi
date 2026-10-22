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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#define স্তূপ_ARG_আকার(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

স্থির ffi_status
initialize_vector (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t গুনতি = 0;
  size_t যোগ_2, p2;

  যদি (UNLIKELY (ptr == ফাঁকা || *ptr == ফাঁকা))
    ফেরত FFI_BAD_TYPEDEF;

  elem = *ptr;
  নির্বাচন (elem->type)
    {
    ক্ষেত্র FFI_TYPE_FLOAT:
    ক্ষেত্র FFI_TYPE_DOUBLE:
    ক্ষেত্র FFI_TYPE_UINT8:
    ক্ষেত্র FFI_TYPE_SINT8:
    ক্ষেত্র FFI_TYPE_UINT16:
    ক্ষেত্র FFI_TYPE_SINT16:
    ক্ষেত্র FFI_TYPE_UINT32:
    ক্ষেত্র FFI_TYPE_SINT32:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_SINT64:
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  জন্য (; *ptr != ফাঁকা; ptr++)
    {
      যদি ((*ptr)->type != elem->type || (*ptr)->আকার_2 != elem->আকার_2)
	ফেরত FFI_BAD_TYPEDEF;
      গুনতি++;
    }

  যদি (UNLIKELY (গুনতি < 1 || elem->আকার_2 == 0))
    ফেরত FFI_BAD_TYPEDEF;

  যোগ_2 = elem->আকার_2 * গুনতি;
  জন্য (p2 = 1; p2 < যোগ_2; p2 <<= 1)
    ;

  arg->আকার_2 = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  ফেরত FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

স্থির ffi_status initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  যদি (UNLIKELY(arg == ফাঁকা || arg->elements == ফাঁকা))
    ফেরত FFI_BAD_TYPEDEF;

  যদি (arg->type == FFI_TYPE_VECTOR)
    ফেরত initialize_vector (arg);

  arg->আকার_2 = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  যদি (UNLIKELY(ptr == 0))
    ফেরত FFI_BAD_TYPEDEF;

  যখন ((*ptr) != ফাঁকা)
    {
      যদি (UNLIKELY(((*ptr)->আকার_2 == 0)
		    && (initialize_aggregate((*ptr), ফাঁকা) != FFI_OK)))
	ফেরত FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->আকার_2 = FFI_ALIGN(arg->আকার_2, (*ptr)->alignment);
      যদি (offsets)
	*offsets++ = arg->আকার_2;
      arg->আকার_2 += (*ptr)->আকার_2;

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
  arg->আকার_2 = FFI_ALIGN (arg->আকার_2, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#ifdef FFI_AGGREGATE_ALIGNMENT
  যদি (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#endif

  যদি (arg->আকার_2 == 0)
    ফেরত FFI_BAD_TYPEDEF;
  অন্যথা
    ফেরত FFI_OK;
}

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
স্থির পূর্ণসংখ্যা
ffi_type_contains_vector (ffi_type *ty)
{
  ffi_type **p;

  যদি (ty == ফাঁকা)
    ফেরত 0;
  যদি (ty->type == FFI_TYPE_VECTOR)
    ফেরত 1;
  যদি (ty->type == FFI_TYPE_STRUCT && ty->elements != ফাঁকা)
    জন্য (p = ty->elements; *p != ফাঁকা; p++)
      যদি (ffi_type_contains_vector (*p))
	ফেরত 1;
  ফেরত 0;
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
			     unsigned পূর্ণসংখ্যা isvariadic,
                             unsigned পূর্ণসংখ্যা nfixedargs,
                             unsigned পূর্ণসংখ্যা ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  unsigned bytes = 0;
  unsigned পূর্ণসংখ্যা i;
  ffi_type **ptr;

  FFI_ASSERT(cif != ফাঁকা);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  যদি (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    ফেরত FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  যদি (ffi_type_contains_vector (rtype))
    ফেরত FFI_BAD_TYPEDEF;
  জন্য (i = 0; i < ntotalargs; i++)
    যদি (ffi_type_contains_vector (atypes[i]))
      ফেরত FFI_BAD_TYPEDEF;
#endif

  cif->flags = 0;
#যদি (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#endif
#যদি HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  /* Initialize the return type if necessary */
  যদি ((cif->rtype->আকার_2 == 0)
      && (initialize_aggregate(cif->rtype, ফাঁকা) != FFI_OK))
    ফেরত FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
  যদি (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#endif
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#যদি !defined FFI_TARGET_SPECIFIC_স্তূপ_SPACE_ALLOCATION
  /* Make space for the return structure pointer */
  যদি ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_VECTOR)
#ifdef TILE
      && (cif->rtype->আকার_2 > 10 * FFI_SIZEOF_ARG)
#endif
#ifdef XTENSA
      && (cif->rtype->আকার_2 > 16)
#endif
     )
    bytes = স্তূপ_ARG_আকার(আকার(শূন্য*));
#endif

  জন্য (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      যদি (((*ptr)->আকার_2 == 0)
	  && (initialize_aggregate((*ptr), ফাঁকা) != FFI_OK))
	ফেরত FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
      যদি ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#endif
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#যদি !defined FFI_TARGET_SPECIFIC_স্তূপ_SPACE_ALLOCATION
	{
	  /* Add any padding if necessary */
	  যদি (((*ptr)->alignment - 1) & bytes)
	    bytes = (unsigned)FFI_ALIGN(bytes, (*ptr)->alignment);

#ifdef TILE
	  যদি (bytes < 10 * FFI_SIZEOF_ARG &&
	      bytes + স্তূপ_ARG_আকার((*ptr)->আকার_2) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      bytes = 10 * FFI_SIZEOF_ARG;
	    }
#endif
#ifdef XTENSA
	  যদি (bytes <= 6*4 && bytes + স্তূপ_ARG_আকার((*ptr)->আকার_2) > 6*4)
	    bytes = 6*4;
#endif

	  bytes += (unsigned পূর্ণসংখ্যা)স্তূপ_ARG_আকার((*ptr)->আকার_2);
	}
#endif
    }

  cif->bytes = bytes;

  /* Perform machine dependent cif processing */
#ifdef FFI_TARGET_SPECIFIC_VARIADIC
  যদি (isvariadic)
	ফেরত ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#endif

  ফেরত ffi_prep_cif_machdep(cif);
}
#endif /* not __CRIS__ */

ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, unsigned পূর্ণসংখ্যা nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  ফেরত ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_status ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            unsigned পূর্ণসংখ্যা nfixedargs,
                            unsigned পূর্ণসংখ্যা ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_status rc;
  size_t int_আকার = ffi_type_sint.আকার_2;
  unsigned পূর্ণসংখ্যা i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  যদি (rc != FFI_OK)
    ফেরত rc;

  জন্য (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      যদি (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->আকার_2 < int_আকার))
        ফেরত FFI_BAD_ARGTYPE;
    }

  ফেরত FFI_OK;
}

#যদি FFI_CLOSURES

ffi_status
ffi_prep_closure (ffi_closure* closure,
		  ffi_cif* cif,
		  শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		  শূন্য *user_তথ্য)
{
  ফেরত ffi_prep_closure_loc (closure, cif, fun, user_তথ্য, closure);
}

#endif

ffi_status
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  যদি (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    ফেরত FFI_BAD_ABI;
  যদি (struct_type->type != FFI_TYPE_STRUCT)
    ফেরত FFI_BAD_TYPEDEF;

#যদি HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  ফেরত initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#যদি !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

কাঠামো ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (আকার (কাঠামো ffi_call_plan));
  যদি (plan != ফাঁকা)
    plan->cif = cif;
  ফেরত plan;
}

শূন্য
ffi_call_plan_invoke (ffi_call_plan *plan, শূন্য (*fn) (শূন্য),
		      শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

শূন্য
ffi_call_plan_free (ffi_call_plan *plan)
{
  মুক্ত (plan);
}

size_t
ffi_call_plan_আকার (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  ফেরত plan != ফাঁকা ? আকার (কাঠামো ffi_call_plan) : 0;
}

#endif /* generic ffi_call_plan fallback */

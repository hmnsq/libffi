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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>
#അന്തര്ഭാവ <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#define അട്ടി_ARG_SIZE(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

സ്ഥിതം ffi_status
initialize_vector (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t എണ്ണം = 0;
  size_t total, p2;

  എങ്കിൽ (UNLIKELY (ptr == ശൂന്യ || *ptr == ശൂന്യ))
    തിരികെ FFI_BAD_TYPEDEF;

  elem = *ptr;
  തിരഞ്ഞെടുപ്പ് (elem->type)
    {
    അവസ്ഥ FFI_TYPE_FLOAT:
    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_UINT8:
    അവസ്ഥ FFI_TYPE_SINT8:
    അവസ്ഥ FFI_TYPE_UINT16:
    അവസ്ഥ FFI_TYPE_SINT16:
    അവസ്ഥ FFI_TYPE_UINT32:
    അവസ്ഥ FFI_TYPE_SINT32:
    അവസ്ഥ FFI_TYPE_UINT64:
    അവസ്ഥ FFI_TYPE_SINT64:
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  വേണ്ടി (; *ptr != ശൂന്യ; ptr++)
    {
      എങ്കിൽ ((*ptr)->type != elem->type || (*ptr)->size != elem->size)
	തിരികെ FFI_BAD_TYPEDEF;
      എണ്ണം++;
    }

  എങ്കിൽ (UNLIKELY (എണ്ണം < 1 || elem->size == 0))
    തിരികെ FFI_BAD_TYPEDEF;

  total = elem->size * എണ്ണം;
  വേണ്ടി (p2 = 1; p2 < total; p2 <<= 1)
    ;

  arg->size = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  തിരികെ FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

സ്ഥിതം ffi_status initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  എങ്കിൽ (UNLIKELY(arg == ശൂന്യ || arg->elements == ശൂന്യ))
    തിരികെ FFI_BAD_TYPEDEF;

  എങ്കിൽ (arg->type == FFI_TYPE_VECTOR)
    തിരികെ initialize_vector (arg);

  arg->size = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  എങ്കിൽ (UNLIKELY(ptr == 0))
    തിരികെ FFI_BAD_TYPEDEF;

  ആകുമ്പോൾ ((*ptr) != ശൂന്യ)
    {
      എങ്കിൽ (UNLIKELY(((*ptr)->size == 0)
		    && (initialize_aggregate((*ptr), ശൂന്യ) != FFI_OK)))
	തിരികെ FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->size = FFI_ALIGN(arg->size, (*ptr)->alignment);
      എങ്കിൽ (offsets)
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
  എങ്കിൽ (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#endif

  എങ്കിൽ (arg->size == 0)
    തിരികെ FFI_BAD_TYPEDEF;
  അല്ലെങ്കിൽ
    തിരികെ FFI_OK;
}

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
സ്ഥിതം പൂർണ്ണസംഖ്യ
ffi_type_contains_vector (ffi_type *ty)
{
  ffi_type **p;

  എങ്കിൽ (ty == ശൂന്യ)
    തിരികെ 0;
  എങ്കിൽ (ty->type == FFI_TYPE_VECTOR)
    തിരികെ 1;
  എങ്കിൽ (ty->type == FFI_TYPE_STRUCT && ty->elements != ശൂന്യ)
    വേണ്ടി (p = ty->elements; *p != ശൂന്യ; p++)
      എങ്കിൽ (ffi_type_contains_vector (*p))
	തിരികെ 1;
  തിരികെ 0;
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
			     unsigned പൂർണ്ണസംഖ്യ isvariadic,
                             unsigned പൂർണ്ണസംഖ്യ nfixedargs,
                             unsigned പൂർണ്ണസംഖ്യ ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  unsigned bytes = 0;
  unsigned പൂർണ്ണസംഖ്യ i;
  ffi_type **ptr;

  FFI_ASSERT(cif != ശൂന്യ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  എങ്കിൽ (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    തിരികെ FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#ifndef FFI_TARGET_HAS_VECTOR_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  എങ്കിൽ (ffi_type_contains_vector (rtype))
    തിരികെ FFI_BAD_TYPEDEF;
  വേണ്ടി (i = 0; i < ntotalargs; i++)
    എങ്കിൽ (ffi_type_contains_vector (atypes[i]))
      തിരികെ FFI_BAD_TYPEDEF;
#endif

  cif->flags = 0;
#എങ്കിൽ (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#endif
#എങ്കിൽ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  /* Initialize the return type if necessary */
  എങ്കിൽ ((cif->rtype->size == 0)
      && (initialize_aggregate(cif->rtype, ശൂന്യ) != FFI_OK))
    തിരികെ FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
  എങ്കിൽ (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#endif
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#എങ്കിൽ !defined FFI_TARGET_SPECIFIC_അട്ടി_SPACE_ALLOCATION
  /* Make space for the return structure pointer */
  എങ്കിൽ ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_VECTOR)
#ifdef TILE
      && (cif->rtype->size > 10 * FFI_SIZEOF_ARG)
#endif
#ifdef XTENSA
      && (cif->rtype->size > 16)
#endif
     )
    bytes = അട്ടി_ARG_SIZE(വലുപ്പം(ശൂന്യം*));
#endif

  വേണ്ടി (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      എങ്കിൽ (((*ptr)->size == 0)
	  && (initialize_aggregate((*ptr), ശൂന്യ) != FFI_OK))
	തിരികെ FFI_BAD_TYPEDEF;

#ifndef FFI_TARGET_HAS_COMPLEX_TYPE
      എങ്കിൽ ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#endif
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#എങ്കിൽ !defined FFI_TARGET_SPECIFIC_അട്ടി_SPACE_ALLOCATION
	{
	  /* Add any padding if necessary */
	  എങ്കിൽ (((*ptr)->alignment - 1) & bytes)
	    bytes = (unsigned)FFI_ALIGN(bytes, (*ptr)->alignment);

#ifdef TILE
	  എങ്കിൽ (bytes < 10 * FFI_SIZEOF_ARG &&
	      bytes + അട്ടി_ARG_SIZE((*ptr)->size) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      bytes = 10 * FFI_SIZEOF_ARG;
	    }
#endif
#ifdef XTENSA
	  എങ്കിൽ (bytes <= 6*4 && bytes + അട്ടി_ARG_SIZE((*ptr)->size) > 6*4)
	    bytes = 6*4;
#endif

	  bytes += (unsigned പൂർണ്ണസംഖ്യ)അട്ടി_ARG_SIZE((*ptr)->size);
	}
#endif
    }

  cif->bytes = bytes;

  /* Perform machine dependent cif processing */
#ifdef FFI_TARGET_SPECIFIC_VARIADIC
  എങ്കിൽ (isvariadic)
	തിരികെ ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#endif

  തിരികെ ffi_prep_cif_machdep(cif);
}
#endif /* not __CRIS__ */

ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, unsigned പൂർണ്ണസംഖ്യ nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  തിരികെ ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_status ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            unsigned പൂർണ്ണസംഖ്യ nfixedargs,
                            unsigned പൂർണ്ണസംഖ്യ ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_status rc;
  size_t int_size = ffi_type_sint.size;
  unsigned പൂർണ്ണസംഖ്യ i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  എങ്കിൽ (rc != FFI_OK)
    തിരികെ rc;

  വേണ്ടി (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      എങ്കിൽ (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->size < int_size))
        തിരികെ FFI_BAD_ARGTYPE;
    }

  തിരികെ FFI_OK;
}

#എങ്കിൽ FFI_CLOSURES

ffi_status
ffi_prep_closure (ffi_closure* closure,
		  ffi_cif* cif,
		  ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		  ശൂന്യം *user_data)
{
  തിരികെ ffi_prep_closure_loc (closure, cif, fun, user_data, closure);
}

#endif

ffi_status
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  എങ്കിൽ (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    തിരികെ FFI_BAD_ABI;
  എങ്കിൽ (struct_type->type != FFI_TYPE_STRUCT)
    തിരികെ FFI_BAD_TYPEDEF;

#എങ്കിൽ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#endif

  തിരികെ initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#എങ്കിൽ !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

ഘടന ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = malloc (വലുപ്പം (ഘടന ffi_call_plan));
  എങ്കിൽ (plan != ശൂന്യ)
    plan->cif = cif;
  തിരികെ plan;
}

ശൂന്യം
ffi_call_plan_invoke (ffi_call_plan *plan, ശൂന്യം (*fn) (ശൂന്യം),
		      ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

ശൂന്യം
ffi_call_plan_free (ffi_call_plan *plan)
{
  സ്വതന്ത്രം (plan);
}

size_t
ffi_call_plan_size (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  തിരികെ plan != ശൂന്യ ? വലുപ്പം (ഘടന ffi_call_plan) : 0;
}

#endif /* generic ffi_call_plan fallback */

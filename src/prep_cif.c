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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>
#అన్తర్భావ <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#నిర్వచించు దొంతర_ARG_కొలత(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

స్థిర ffi_స్థితి
initialize_సదిశ (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t లెక్క = 0;
  size_t మొత్తం, p2;

  ఒకవేళ (UNLIKELY (ptr == ఖాళీ || *ptr == ఖాళీ))
    తిరిగి FFI_BAD_TYPEDEF;

  elem = *ptr;
  ఎంపిక (elem->type)
    {
    స్థితి FFI_TYPE_FLOAT:
    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_UINT8:
    స్థితి FFI_TYPE_SINT8:
    స్థితి FFI_TYPE_UINT16:
    స్థితి FFI_TYPE_SINT16:
    స్థితి FFI_TYPE_UINT32:
    స్థితి FFI_TYPE_SINT32:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_SINT64:
      ఆపు;
    అప్రమేయ:
      తిరిగి FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  కొరకు (; *ptr != ఖాళీ; ptr++)
    {
      ఒకవేళ ((*ptr)->type != elem->type || (*ptr)->కొలత != elem->కొలత)
	తిరిగి FFI_BAD_TYPEDEF;
      లెక్క++;
    }

  ఒకవేళ (UNLIKELY (లెక్క < 1 || elem->కొలత == 0))
    తిరిగి FFI_BAD_TYPEDEF;

  మొత్తం = elem->కొలత * లెక్క;
  కొరకు (p2 = 1; p2 < మొత్తం; p2 <<= 1)
    ;

  arg->కొలత = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  తిరిగి FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

స్థిర ffi_స్థితి initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  ఒకవేళ (UNLIKELY(arg == ఖాళీ || arg->elements == ఖాళీ))
    తిరిగి FFI_BAD_TYPEDEF;

  ఒకవేళ (arg->type == FFI_TYPE_సదిశ)
    తిరిగి initialize_సదిశ (arg);

  arg->కొలత = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  ఒకవేళ (UNLIKELY(ptr == 0))
    తిరిగి FFI_BAD_TYPEDEF;

  అయితే ((*ptr) != ఖాళీ)
    {
      ఒకవేళ (UNLIKELY(((*ptr)->కొలత == 0)
		    && (initialize_aggregate((*ptr), ఖాళీ) != FFI_OK)))
	తిరిగి FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->కొలత = FFI_ALIGN(arg->కొలత, (*ptr)->alignment);
      ఒకవేళ (offsets)
	*offsets++ = arg->కొలత;
      arg->కొలత += (*ptr)->కొలత;

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
  arg->కొలత = FFI_ALIGN (arg->కొలత, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#నిర్వచితమైతే FFI_AGGREGATE_ALIGNMENT
  ఒకవేళ (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#అయితే_ముగింపు

  ఒకవేళ (arg->కొలత == 0)
    తిరిగి FFI_BAD_TYPEDEF;
  లేకపోతే
    తిరిగి FFI_OK;
}

#నిర్వచితం_కాకపోతే FFI_లక్ష్యం_HAS_సదిశ_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
స్థిర పూర్ణాంకం
ffi_type_కలిగియుంది_సదిశ (ffi_type *ty)
{
  ffi_type **p;

  ఒకవేళ (ty == ఖాళీ)
    తిరిగి 0;
  ఒకవేళ (ty->type == FFI_TYPE_సదిశ)
    తిరిగి 1;
  ఒకవేళ (ty->type == FFI_TYPE_STRUCT && ty->elements != ఖాళీ)
    కొరకు (p = ty->elements; *p != ఖాళీ; p++)
      ఒకవేళ (ffi_type_కలిగియుంది_సదిశ (*p))
	తిరిగి 1;
  తిరిగి 0;
}
#అయితే_ముగింపు /* !FFI_TARGET_HAS_VECTOR_TYPE */

#నిర్వచితం_కాకపోతే __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_స్థితి FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     గుర్తులేని పూర్ణాంకం isvariadic,
                             గుర్తులేని పూర్ణాంకం nfixedargs,
                             గుర్తులేని పూర్ణాంకం ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  గుర్తులేని బైట్లు = 0;
  గుర్తులేని పూర్ణాంకం i;
  ffi_type **ptr;

  FFI_ASSERT(cif != ఖాళీ);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  ఒకవేళ (! (abi > FFI_మొదటి_ABI && abi < FFI_చివరి_ABI))
    తిరిగి FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#నిర్వచితం_కాకపోతే FFI_లక్ష్యం_HAS_సదిశ_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  ఒకవేళ (ffi_type_కలిగియుంది_సదిశ (rtype))
    తిరిగి FFI_BAD_TYPEDEF;
  కొరకు (i = 0; i < ntotalargs; i++)
    ఒకవేళ (ffi_type_కలిగియుంది_సదిశ (atypes[i]))
      తిరిగి FFI_BAD_TYPEDEF;
#అయితే_ముగింపు

  cif->flags = 0;
#ఒకవేళ (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#అయితే_ముగింపు
#ఒకవేళ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#అయితే_ముగింపు

  /* Initialize the return type if necessary */
  ఒకవేళ ((cif->rtype->కొలత == 0)
      && (initialize_aggregate(cif->rtype, ఖాళీ) != FFI_OK))
    తిరిగి FFI_BAD_TYPEDEF;

#నిర్వచితం_కాకపోతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
  ఒకవేళ (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#అయితే_ముగింపు
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#ఒకవేళ !defined FFI_లక్ష్యం_SPECIFIC_దొంతర_SPACE_కేటాయింపు
  /* Make space for the return structure pointer */
  ఒకవేళ ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_సదిశ)
#నిర్వచితమైతే TILE
      && (cif->rtype->కొలత > 10 * FFI_SIZEOF_ARG)
#అయితే_ముగింపు
#నిర్వచితమైతే XTENSA
      && (cif->rtype->కొలత > 16)
#అయితే_ముగింపు
     )
    బైట్లు = దొంతర_ARG_కొలత(పరిమాణం(శూన్య*));
#అయితే_ముగింపు

  కొరకు (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      ఒకవేళ (((*ptr)->కొలత == 0)
	  && (initialize_aggregate((*ptr), ఖాళీ) != FFI_OK))
	తిరిగి FFI_BAD_TYPEDEF;

#నిర్వచితం_కాకపోతే FFI_లక్ష్యం_HAS_COMPLEX_TYPE
      ఒకవేళ ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#అయితే_ముగింపు
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#ఒకవేళ !defined FFI_లక్ష్యం_SPECIFIC_దొంతర_SPACE_కేటాయింపు
	{
	  /* Add any padding if necessary */
	  ఒకవేళ (((*ptr)->alignment - 1) & బైట్లు)
	    బైట్లు = (గుర్తులేని)FFI_ALIGN(బైట్లు, (*ptr)->alignment);

#నిర్వచితమైతే TILE
	  ఒకవేళ (బైట్లు < 10 * FFI_SIZEOF_ARG &&
	      బైట్లు + దొంతర_ARG_కొలత((*ptr)->కొలత) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      బైట్లు = 10 * FFI_SIZEOF_ARG;
	    }
#అయితే_ముగింపు
#నిర్వచితమైతే XTENSA
	  ఒకవేళ (బైట్లు <= 6*4 && బైట్లు + దొంతర_ARG_కొలత((*ptr)->కొలత) > 6*4)
	    బైట్లు = 6*4;
#అయితే_ముగింపు

	  బైట్లు += (గుర్తులేని పూర్ణాంకం)దొంతర_ARG_కొలత((*ptr)->కొలత);
	}
#అయితే_ముగింపు
    }

  cif->బైట్లు = బైట్లు;

  /* Perform machine dependent cif processing */
#నిర్వచితమైతే FFI_లక్ష్యం_SPECIFIC_VARIADIC
  ఒకవేళ (isvariadic)
	తిరిగి ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#అయితే_ముగింపు

  తిరిగి ffi_prep_cif_machdep(cif);
}
#అయితే_ముగింపు /* not __CRIS__ */

ffi_స్థితి ffi_prep_cif(ffi_cif *cif, ffi_abi abi, గుర్తులేని పూర్ణాంకం nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  తిరిగి ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_స్థితి ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            గుర్తులేని పూర్ణాంకం nfixedargs,
                            గుర్తులేని పూర్ణాంకం ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_స్థితి rc;
  size_t int_కొలత = ffi_type_sint.కొలత;
  గుర్తులేని పూర్ణాంకం i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  ఒకవేళ (rc != FFI_OK)
    తిరిగి rc;

  కొరకు (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      ఒకవేళ (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->కొలత < int_కొలత))
        తిరిగి FFI_BAD_ARGTYPE;
    }

  తిరిగి FFI_OK;
}

#ఒకవేళ FFI_CLOSURES

ffi_స్థితి
ffi_prep_సంవృతం (ffi_సంవృతం* సంవృతం,
		  ffi_cif* cif,
		  శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		  శూన్య *user_దత్తాంశం)
{
  తిరిగి ffi_prep_సంవృతం_loc (సంవృతం, cif, fun, user_దత్తాంశం, సంవృతం);
}

#అయితే_ముగింపు

ffi_స్థితి
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  ఒకవేళ (! (abi > FFI_మొదటి_ABI && abi < FFI_చివరి_ABI))
    తిరిగి FFI_BAD_ABI;
  ఒకవేళ (struct_type->type != FFI_TYPE_STRUCT)
    తిరిగి FFI_BAD_TYPEDEF;

#ఒకవేళ HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#అయితే_ముగింపు

  తిరిగి initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#ఒకవేళ !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

నిర్మాణం ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = జ్ఞాపకం_ఇవ్వు (పరిమాణం (నిర్మాణం ffi_call_plan));
  ఒకవేళ (plan != ఖాళీ)
    plan->cif = cif;
  తిరిగి plan;
}

శూన్య
ffi_call_plan_invoke (ffi_call_plan *plan, శూన్య (*fn) (శూన్య),
		      శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

శూన్య
ffi_call_plan_free (ffi_call_plan *plan)
{
  విడుదల (plan);
}

size_t
ffi_call_plan_కొలత (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  తిరిగి plan != ఖాళీ ? పరిమాణం (నిర్మాణం ffi_call_plan) : 0;
}

#అయితే_ముగింపు /* generic ffi_call_plan fallback */

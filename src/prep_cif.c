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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>
#अन्तर्भाव <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#परिभाषय राशिः_ARG_आकार(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

स्थिर ffi_स्थितिः
initialize_सदिशम् (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t गणनम् = 0;
  size_t योग_2, p2;

  यदि (UNLIKELY (ptr == रिक्त || *ptr == रिक्त))
    प्रत्यागम FFI_BAD_TYPEDEF;

  elem = *ptr;
  विकल्प (elem->type)
    {
    पक्ष FFI_TYPE_FLOAT:
    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_UINT8:
    पक्ष FFI_TYPE_SINT8:
    पक्ष FFI_TYPE_UINT16:
    पक्ष FFI_TYPE_SINT16:
    पक्ष FFI_TYPE_UINT32:
    पक्ष FFI_TYPE_SINT32:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_SINT64:
      भङ्ग;
    स्वभाव:
      प्रत्यागम FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  प्रति (; *ptr != रिक्त; ptr++)
    {
      यदि ((*ptr)->type != elem->type || (*ptr)->आकार != elem->आकार)
	प्रत्यागम FFI_BAD_TYPEDEF;
      गणनम्++;
    }

  यदि (UNLIKELY (गणनम् < 1 || elem->आकार == 0))
    प्रत्यागम FFI_BAD_TYPEDEF;

  योग_2 = elem->आकार * गणनम्;
  प्रति (p2 = 1; p2 < योग_2; p2 <<= 1)
    ;

  arg->आकार = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  प्रत्यागम FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

स्थिर ffi_स्थितिः initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  यदि (UNLIKELY(arg == रिक्त || arg->elements == रिक्त))
    प्रत्यागम FFI_BAD_TYPEDEF;

  यदि (arg->type == FFI_TYPE_सदिशम्)
    प्रत्यागम initialize_सदिशम् (arg);

  arg->आकार = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  यदि (UNLIKELY(ptr == 0))
    प्रत्यागम FFI_BAD_TYPEDEF;

  यावत् ((*ptr) != रिक्त)
    {
      यदि (UNLIKELY(((*ptr)->आकार == 0)
		    && (initialize_aggregate((*ptr), रिक्त) != FFI_OK)))
	प्रत्यागम FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->आकार = FFI_ALIGN(arg->आकार, (*ptr)->alignment);
      यदि (offsets)
	*offsets++ = arg->आकार;
      arg->आकार += (*ptr)->आकार;

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
  arg->आकार = FFI_ALIGN (arg->आकार, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#यदि_परिभाषितम् FFI_AGGREGATE_ALIGNMENT
  यदि (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#यदि_अन्तः

  यदि (arg->आकार == 0)
    प्रत्यागम FFI_BAD_TYPEDEF;
  अन्यथा
    प्रत्यागम FFI_OK;
}

#यदि_अपरिभाषितम् FFI_TARGET_HAS_सदिशम्_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
स्थिर पूर्णाङ्क
ffi_type_अन्तर्भवति_सदिशम् (ffi_type *ty)
{
  ffi_type **p;

  यदि (ty == रिक्त)
    प्रत्यागम 0;
  यदि (ty->type == FFI_TYPE_सदिशम्)
    प्रत्यागम 1;
  यदि (ty->type == FFI_TYPE_STRUCT && ty->elements != रिक्त)
    प्रति (p = ty->elements; *p != रिक्त; p++)
      यदि (ffi_type_अन्तर्भवति_सदिशम् (*p))
	प्रत्यागम 1;
  प्रत्यागम 0;
}
#यदि_अन्तः /* !FFI_TARGET_HAS_VECTOR_TYPE */

#यदि_अपरिभाषितम् __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_स्थितिः FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     अचिह्नित पूर्णाङ्क isvariadic,
                             अचिह्नित पूर्णाङ्क nfixedargs,
                             अचिह्नित पूर्णाङ्क ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  अचिह्नित अष्टकानि = 0;
  अचिह्नित पूर्णाङ्क i;
  ffi_type **ptr;

  FFI_ASSERT(cif != रिक्त);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  यदि (! (abi > FFI_FIRST_ABI && abi < FFI_चरमः_ABI))
    प्रत्यागम FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#यदि_अपरिभाषितम् FFI_TARGET_HAS_सदिशम्_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  यदि (ffi_type_अन्तर्भवति_सदिशम् (rtype))
    प्रत्यागम FFI_BAD_TYPEDEF;
  प्रति (i = 0; i < ntotalargs; i++)
    यदि (ffi_type_अन्तर्भवति_सदिशम् (atypes[i]))
      प्रत्यागम FFI_BAD_TYPEDEF;
#यदि_अन्तः

  cif->flags = 0;
#यदि (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#यदि_अन्तः
#यदि HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#यदि_अन्तः

  /* Initialize the return type if necessary */
  यदि ((cif->rtype->आकार == 0)
      && (initialize_aggregate(cif->rtype, रिक्त) != FFI_OK))
    प्रत्यागम FFI_BAD_TYPEDEF;

#यदि_अपरिभाषितम् FFI_TARGET_HAS_COMPLEX_TYPE
  यदि (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#यदि_अन्तः
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#यदि !defined FFI_TARGET_SPECIFIC_राशिः_SPACE_आवण्टनम्
  /* Make space for the return structure pointer */
  यदि ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_सदिशम्)
#यदि_परिभाषितम् TILE
      && (cif->rtype->आकार > 10 * FFI_SIZEOF_ARG)
#यदि_अन्तः
#यदि_परिभाषितम् XTENSA
      && (cif->rtype->आकार > 16)
#यदि_अन्तः
     )
    अष्टकानि = राशिः_ARG_आकार(परिमाण(शून्य*));
#यदि_अन्तः

  प्रति (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      यदि (((*ptr)->आकार == 0)
	  && (initialize_aggregate((*ptr), रिक्त) != FFI_OK))
	प्रत्यागम FFI_BAD_TYPEDEF;

#यदि_अपरिभाषितम् FFI_TARGET_HAS_COMPLEX_TYPE
      यदि ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#यदि_अन्तः
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#यदि !defined FFI_TARGET_SPECIFIC_राशिः_SPACE_आवण्टनम्
	{
	  /* Add any padding if necessary */
	  यदि (((*ptr)->alignment - 1) & अष्टकानि)
	    अष्टकानि = (अचिह्नित)FFI_ALIGN(अष्टकानि, (*ptr)->alignment);

#यदि_परिभाषितम् TILE
	  यदि (अष्टकानि < 10 * FFI_SIZEOF_ARG &&
	      अष्टकानि + राशिः_ARG_आकार((*ptr)->आकार) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      अष्टकानि = 10 * FFI_SIZEOF_ARG;
	    }
#यदि_अन्तः
#यदि_परिभाषितम् XTENSA
	  यदि (अष्टकानि <= 6*4 && अष्टकानि + राशिः_ARG_आकार((*ptr)->आकार) > 6*4)
	    अष्टकानि = 6*4;
#यदि_अन्तः

	  अष्टकानि += (अचिह्नित पूर्णाङ्क)राशिः_ARG_आकार((*ptr)->आकार);
	}
#यदि_अन्तः
    }

  cif->अष्टकानि = अष्टकानि;

  /* Perform machine dependent cif processing */
#यदि_परिभाषितम् FFI_TARGET_SPECIFIC_VARIADIC
  यदि (isvariadic)
	प्रत्यागम ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#यदि_अन्तः

  प्रत्यागम ffi_prep_cif_machdep(cif);
}
#यदि_अन्तः /* not __CRIS__ */

ffi_स्थितिः ffi_prep_cif(ffi_cif *cif, ffi_abi abi, अचिह्नित पूर्णाङ्क nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  प्रत्यागम ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_स्थितिः ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            अचिह्नित पूर्णाङ्क nfixedargs,
                            अचिह्नित पूर्णाङ्क ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_स्थितिः rc;
  size_t int_आकार = ffi_type_sint.आकार;
  अचिह्नित पूर्णाङ्क i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  यदि (rc != FFI_OK)
    प्रत्यागम rc;

  प्रति (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      यदि (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->आकार < int_आकार))
        प्रत्यागम FFI_BAD_ARGTYPE;
    }

  प्रत्यागम FFI_OK;
}

#यदि FFI_CLOSURES

ffi_स्थितिः
ffi_prep_संवृतिः (ffi_संवृतिः* संवृतिः,
		  ffi_cif* cif,
		  शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		  शून्य *user_दत्त)
{
  प्रत्यागम ffi_prep_संवृतिः_loc (संवृतिः, cif, fun, user_दत्त, संवृतिः);
}

#यदि_अन्तः

ffi_स्थितिः
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  यदि (! (abi > FFI_FIRST_ABI && abi < FFI_चरमः_ABI))
    प्रत्यागम FFI_BAD_ABI;
  यदि (struct_type->type != FFI_TYPE_STRUCT)
    प्रत्यागम FFI_BAD_TYPEDEF;

#यदि HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#यदि_अन्तः

  प्रत्यागम initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#यदि !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

संरचना ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = स्मृतिं_देहि (परिमाण (संरचना ffi_call_plan));
  यदि (plan != रिक्त)
    plan->cif = cif;
  प्रत्यागम plan;
}

शून्य
ffi_call_plan_invoke (ffi_call_plan *plan, शून्य (*fn) (शून्य),
		      शून्य *rvalue, शून्य **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

शून्य
ffi_call_plan_free (ffi_call_plan *plan)
{
  मुक्त (plan);
}

size_t
ffi_call_plan_आकार (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  प्रत्यागम plan != रिक्त ? परिमाण (संरचना ffi_call_plan) : 0;
}

#यदि_अन्तः /* generic ffi_call_plan fallback */

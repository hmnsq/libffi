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

#समावेश <ffi.h>
#समावेश <ffi_common.h>
#समावेश <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#घोषणा ढेर_ARG_आकार(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

अटल ffi_status
initialize_vector (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  माप_प्रकार गिनती = 0;
  माप_प्रकार योग_2, p2;

  अगर (UNLIKELY (ptr == शून्य || *ptr == शून्य))
    वापस FFI_BAD_TYPEDEF;

  elem = *ptr;
  चयन (elem->type)
    {
    हाल FFI_TYPE_FLOAT:
    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_UINT8:
    हाल FFI_TYPE_SINT8:
    हाल FFI_TYPE_UINT16:
    हाल FFI_TYPE_SINT16:
    हाल FFI_TYPE_UINT32:
    हाल FFI_TYPE_SINT32:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_SINT64:
      अवरोध;
    शेष:
      वापस FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  क्रम (; *ptr != शून्य; ptr++)
    {
      अगर ((*ptr)->type != elem->type || (*ptr)->आकार != elem->आकार)
	वापस FFI_BAD_TYPEDEF;
      गिनती++;
    }

  अगर (UNLIKELY (गिनती < 1 || elem->आकार == 0))
    वापस FFI_BAD_TYPEDEF;

  योग_2 = elem->आकार * गिनती;
  क्रम (p2 = 1; p2 < योग_2; p2 <<= 1)
    ;

  arg->आकार = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  वापस FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

अटल ffi_status initialize_aggregate(ffi_type *arg, माप_प्रकार *offsets)
{
  ffi_type **ptr;

  अगर (UNLIKELY(arg == शून्य || arg->elements == शून्य))
    वापस FFI_BAD_TYPEDEF;

  अगर (arg->type == FFI_TYPE_VECTOR)
    वापस initialize_vector (arg);

  arg->आकार = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  अगर (UNLIKELY(ptr == 0))
    वापस FFI_BAD_TYPEDEF;

  जबतक ((*ptr) != शून्य)
    {
      अगर (UNLIKELY(((*ptr)->आकार == 0)
		    && (initialize_aggregate((*ptr), शून्य) != FFI_OK)))
	वापस FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->आकार = FFI_ALIGN(arg->आकार, (*ptr)->alignment);
      अगर (offsets)
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
#अगर_घोषित FFI_AGGREGATE_ALIGNMENT
  अगर (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#पूर्ण_अगर

  अगर (arg->आकार == 0)
    वापस FFI_BAD_TYPEDEF;
  अन्यथा
    वापस FFI_OK;
}

#अगर_अघोषित FFI_TARGET_HAS_VECTOR_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
अटल पूर्णांक
ffi_type_contains_vector (ffi_type *ty)
{
  ffi_type **p;

  अगर (ty == शून्य)
    वापस 0;
  अगर (ty->type == FFI_TYPE_VECTOR)
    वापस 1;
  अगर (ty->type == FFI_TYPE_STRUCT && ty->elements != शून्य)
    क्रम (p = ty->elements; *p != शून्य; p++)
      अगर (ffi_type_contains_vector (*p))
	वापस 1;
  वापस 0;
}
#पूर्ण_अगर /* !FFI_TARGET_HAS_VECTOR_TYPE */

#अगर_अघोषित __CRIS__
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
			     अचिन्हित पूर्णांक isvariadic,
                             अचिन्हित पूर्णांक nfixedargs,
                             अचिन्हित पूर्णांक ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  अचिन्हित bytes = 0;
  अचिन्हित पूर्णांक i;
  ffi_type **ptr;

  FFI_ASSERT(cif != शून्य);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  अगर (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    वापस FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#अगर_अघोषित FFI_TARGET_HAS_VECTOR_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  अगर (ffi_type_contains_vector (rtype))
    वापस FFI_BAD_TYPEDEF;
  क्रम (i = 0; i < ntotalargs; i++)
    अगर (ffi_type_contains_vector (atypes[i]))
      वापस FFI_BAD_TYPEDEF;
#पूर्ण_अगर

  cif->flags = 0;
#अगर (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#पूर्ण_अगर
#अगर HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#पूर्ण_अगर

  /* Initialize the return type if necessary */
  अगर ((cif->rtype->आकार == 0)
      && (initialize_aggregate(cif->rtype, शून्य) != FFI_OK))
    वापस FFI_BAD_TYPEDEF;

#अगर_अघोषित FFI_TARGET_HAS_COMPLEX_TYPE
  अगर (rtype->type == FFI_TYPE_COMPLEX)
    पात();
#पूर्ण_अगर
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#अगर !defined FFI_TARGET_SPECIFIC_ढेर_SPACE_ALLOCATION
  /* Make space for the return structure pointer */
  अगर ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_VECTOR)
#अगर_घोषित TILE
      && (cif->rtype->आकार > 10 * FFI_SIZEOF_ARG)
#पूर्ण_अगर
#अगर_घोषित XTENSA
      && (cif->rtype->आकार > 16)
#पूर्ण_अगर
     )
    bytes = ढेर_ARG_आकार(माप(व्योम*));
#पूर्ण_अगर

  क्रम (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      अगर (((*ptr)->आकार == 0)
	  && (initialize_aggregate((*ptr), शून्य) != FFI_OK))
	वापस FFI_BAD_TYPEDEF;

#अगर_अघोषित FFI_TARGET_HAS_COMPLEX_TYPE
      अगर ((*ptr)->type == FFI_TYPE_COMPLEX)
	पात();
#पूर्ण_अगर
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#अगर !defined FFI_TARGET_SPECIFIC_ढेर_SPACE_ALLOCATION
	{
	  /* Add any padding if necessary */
	  अगर (((*ptr)->alignment - 1) & bytes)
	    bytes = (अचिन्हित)FFI_ALIGN(bytes, (*ptr)->alignment);

#अगर_घोषित TILE
	  अगर (bytes < 10 * FFI_SIZEOF_ARG &&
	      bytes + ढेर_ARG_आकार((*ptr)->आकार) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      bytes = 10 * FFI_SIZEOF_ARG;
	    }
#पूर्ण_अगर
#अगर_घोषित XTENSA
	  अगर (bytes <= 6*4 && bytes + ढेर_ARG_आकार((*ptr)->आकार) > 6*4)
	    bytes = 6*4;
#पूर्ण_अगर

	  bytes += (अचिन्हित पूर्णांक)ढेर_ARG_आकार((*ptr)->आकार);
	}
#पूर्ण_अगर
    }

  cif->bytes = bytes;

  /* Perform machine dependent cif processing */
#अगर_घोषित FFI_TARGET_SPECIFIC_VARIADIC
  अगर (isvariadic)
	वापस ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#पूर्ण_अगर

  वापस ffi_prep_cif_machdep(cif);
}
#पूर्ण_अगर /* not __CRIS__ */

ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, अचिन्हित पूर्णांक nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  वापस ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_status ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            अचिन्हित पूर्णांक nfixedargs,
                            अचिन्हित पूर्णांक ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_status rc;
  माप_प्रकार int_आकार = ffi_type_sint.आकार;
  अचिन्हित पूर्णांक i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  अगर (rc != FFI_OK)
    वापस rc;

  क्रम (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      अगर (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->आकार < int_आकार))
        वापस FFI_BAD_ARGTYPE;
    }

  वापस FFI_OK;
}

#अगर FFI_CLOSURES

ffi_status
ffi_prep_closure (ffi_closure* closure,
		  ffi_cif* cif,
		  व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
		  व्योम *user_तथ्य)
{
  वापस ffi_prep_closure_loc (closure, cif, fun, user_तथ्य, closure);
}

#पूर्ण_अगर

ffi_status
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, माप_प्रकार *offsets)
{
  अगर (! (abi > FFI_FIRST_ABI && abi < FFI_LAST_ABI))
    वापस FFI_BAD_ABI;
  अगर (struct_type->type != FFI_TYPE_STRUCT)
    वापस FFI_BAD_TYPEDEF;

#अगर HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#पूर्ण_अगर

  वापस initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#अगर !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

काष्ठा ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = दो_स्मृति (माप (काष्ठा ffi_call_plan));
  अगर (plan != शून्य)
    plan->cif = cif;
  वापस plan;
}

व्योम
ffi_call_plan_invoke (ffi_call_plan *plan, व्योम (*fn) (व्योम),
		      व्योम *rvalue, व्योम **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

व्योम
ffi_call_plan_free (ffi_call_plan *plan)
{
  मुक्त (plan);
}

माप_प्रकार
ffi_call_plan_आकार (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  वापस plan != शून्य ? माप (काष्ठा ffi_call_plan) : 0;
}

#पूर्ण_अगर /* generic ffi_call_plan fallback */

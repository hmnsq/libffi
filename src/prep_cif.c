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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdlib.h>

/* Round up to FFI_SIZEOF_ARG. */

#تعریف_کرو ڈھیر_ARG_سائز(x) FFI_ALIGN(x, FFI_SIZEOF_ARG)

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

ساکن ffi_حالت
initialize_سمتیہ (ffi_type *arg)
{
  ffi_type **ptr = arg->elements;
  ffi_type *elem;
  size_t گنتی = 0;
  size_t کل, p2;

  اگر (UNLIKELY (ptr == تہی || *ptr == تہی))
    واپس FFI_BAD_TYPEDEF;

  elem = *ptr;
  انتخاب (elem->type)
    {
    حالت FFI_TYPE_FLOAT:
    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_UINT8:
    حالت FFI_TYPE_SINT8:
    حالت FFI_TYPE_UINT16:
    حالت FFI_TYPE_SINT16:
    حالت FFI_TYPE_UINT32:
    حالت FFI_TYPE_SINT32:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_SINT64:
      توڑو;
    طےشدہ:
      واپس FFI_BAD_TYPEDEF;
    }

  /* Every lane must be the identical scalar type.  */
  برائے (; *ptr != تہی; ptr++)
    {
      اگر ((*ptr)->type != elem->type || (*ptr)->سائز != elem->سائز)
	واپس FFI_BAD_TYPEDEF;
      گنتی++;
    }

  اگر (UNLIKELY (گنتی < 1 || elem->سائز == 0))
    واپس FFI_BAD_TYPEDEF;

  کل = elem->سائز * گنتی;
  برائے (p2 = 1; p2 < کل; p2 <<= 1)
    ;

  arg->سائز = p2;
  arg->alignment = p2 < 16 ? p2 : 16;
  واپس FFI_OK;
}

/* Perform machine independent initialization of aggregate type
   specifications. */

ساکن ffi_حالت initialize_aggregate(ffi_type *arg, size_t *offsets)
{
  ffi_type **ptr;

  اگر (UNLIKELY(arg == تہی || arg->elements == تہی))
    واپس FFI_BAD_TYPEDEF;

  اگر (arg->type == FFI_TYPE_سمتیہ)
    واپس initialize_سمتیہ (arg);

  arg->سائز = 0;
  arg->alignment = 0;

  ptr = &(arg->elements[0]);

  اگر (UNLIKELY(ptr == 0))
    واپس FFI_BAD_TYPEDEF;

  جب ((*ptr) != تہی)
    {
      اگر (UNLIKELY(((*ptr)->سائز == 0)
		    && (initialize_aggregate((*ptr), تہی) != FFI_OK)))
	واپس FFI_BAD_TYPEDEF;

      /* Perform a sanity check on the argument type */
      FFI_ASSERT_VALID_TYPE(*ptr);

      arg->سائز = FFI_ALIGN(arg->سائز, (*ptr)->alignment);
      اگر (offsets)
	*offsets++ = arg->سائز;
      arg->سائز += (*ptr)->سائز;

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
  arg->سائز = FFI_ALIGN (arg->سائز, arg->alignment);

  /* On some targets, the ABI defines that structures have an additional
     alignment beyond the "natural" one based on their elements.  */
#اگر_تعریف_شدہ FFI_AGGREGATE_ALIGNMENT
  اگر (FFI_AGGREGATE_ALIGNMENT > arg->alignment)
    arg->alignment = FFI_AGGREGATE_ALIGNMENT;
#اگر_ختم

  اگر (arg->سائز == 0)
    واپس FFI_BAD_TYPEDEF;
  ورنہ
    واپس FFI_OK;
}

#اگر_غیر_تعریف_شدہ FFI_ہدف_HAS_سمتیہ_TYPE
/* Recursively test whether TY is, or contains, a vector (SIMD) type.  Ports
   that do not define FFI_TARGET_HAS_VECTOR_TYPE cannot marshal vectors, so
   ffi_prep_cif_core rejects any signature that mentions one (directly or
   nested inside a struct) with FFI_BAD_TYPEDEF rather than aborting.  */
ساکن صحیح
ffi_type_شامل_ہے_سمتیہ (ffi_type *ty)
{
  ffi_type **p;

  اگر (ty == تہی)
    واپس 0;
  اگر (ty->type == FFI_TYPE_سمتیہ)
    واپس 1;
  اگر (ty->type == FFI_TYPE_STRUCT && ty->elements != تہی)
    برائے (p = ty->elements; *p != تہی; p++)
      اگر (ffi_type_شامل_ہے_سمتیہ (*p))
	واپس 1;
  واپس 0;
}
#اگر_ختم /* !FFI_TARGET_HAS_VECTOR_TYPE */

#اگر_غیر_تعریف_شدہ __CRIS__
/* The CRIS ABI specifies structure elements to have byte
   alignment only, so it completely overrides this functions,
   which assumes "natural" alignment and padding.  */

/* Perform machine independent ffi_cif preparation, then call
   machine dependent routine. */

/* For non variadic functions isvariadic should be 0 and
   nfixedargs==ntotalargs.

   For variadic calls, isvariadic should be 1 and nfixedargs
   and ntotalargs set as appropriate. nfixedargs must always be >=1 */


ffi_حالت FFI_HIDDEN ffi_prep_cif_core(ffi_cif *cif, ffi_abi abi,
			     غیرعلامتی صحیح isvariadic,
                             غیرعلامتی صحیح nfixedargs,
                             غیرعلامتی صحیح ntotalargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  غیرعلامتی بائٹیں = 0;
  غیرعلامتی صحیح i;
  ffi_type **ptr;

  FFI_ASSERT(cif != تہی);
  FFI_ASSERT((!isvariadic) || (nfixedargs >= 1));
  FFI_ASSERT(nfixedargs <= ntotalargs);

  اگر (! (abi > FFI_پہلا_ABI && abi < FFI_آخری_ABI))
    واپس FFI_BAD_ABI;

  cif->abi = abi;
  cif->arg_types = atypes;
  cif->nargs = ntotalargs;
  cif->rtype = rtype;

#اگر_غیر_تعریف_شدہ FFI_ہدف_HAS_سمتیہ_TYPE
  /* Vector (SIMD) types are only marshalled on ports that opt in.  */
  اگر (ffi_type_شامل_ہے_سمتیہ (rtype))
    واپس FFI_BAD_TYPEDEF;
  برائے (i = 0; i < ntotalargs; i++)
    اگر (ffi_type_شامل_ہے_سمتیہ (atypes[i]))
      واپس FFI_BAD_TYPEDEF;
#اگر_ختم

  cif->flags = 0;
#اگر (defined(_M_ARM64) || defined(__aarch64__)) && defined(_WIN32)
  cif->is_variadic = isvariadic;
#اگر_ختم
#اگر HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#اگر_ختم

  /* Initialize the return type if necessary */
  اگر ((cif->rtype->سائز == 0)
      && (initialize_aggregate(cif->rtype, تہی) != FFI_OK))
    واپس FFI_BAD_TYPEDEF;

#اگر_غیر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
  اگر (rtype->type == FFI_TYPE_COMPLEX)
    abort();
#اگر_ختم
  /* Perform a sanity check on the return type */
  FFI_ASSERT_VALID_TYPE(cif->rtype);

  /* x86, x86-64 and s390 stack space allocation is handled in prep_machdep. */
#اگر !defined FFI_ہدف_SPECIFIC_ڈھیر_SPACE_تخصیص
  /* Make space for the return structure pointer */
  اگر ((cif->rtype->type == FFI_TYPE_STRUCT
       || cif->rtype->type == FFI_TYPE_سمتیہ)
#اگر_تعریف_شدہ TILE
      && (cif->rtype->سائز > 10 * FFI_SIZEOF_ARG)
#اگر_ختم
#اگر_تعریف_شدہ XTENSA
      && (cif->rtype->سائز > 16)
#اگر_ختم
     )
    بائٹیں = ڈھیر_ARG_سائز(حجم(خالی*));
#اگر_ختم

  برائے (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, ptr++)
    {

      /* Initialize any uninitialized aggregate type definitions */
      اگر (((*ptr)->سائز == 0)
	  && (initialize_aggregate((*ptr), تہی) != FFI_OK))
	واپس FFI_BAD_TYPEDEF;

#اگر_غیر_تعریف_شدہ FFI_ہدف_HAS_COMPLEX_TYPE
      اگر ((*ptr)->type == FFI_TYPE_COMPLEX)
	abort();
#اگر_ختم
      /* Perform a sanity check on the argument type, do this
	 check after the initialization.  */
      FFI_ASSERT_VALID_TYPE(*ptr);

#اگر !defined FFI_ہدف_SPECIFIC_ڈھیر_SPACE_تخصیص
	{
	  /* Add any padding if necessary */
	  اگر (((*ptr)->alignment - 1) & بائٹیں)
	    بائٹیں = (غیرعلامتی)FFI_ALIGN(بائٹیں, (*ptr)->alignment);

#اگر_تعریف_شدہ TILE
	  اگر (بائٹیں < 10 * FFI_SIZEOF_ARG &&
	      بائٹیں + ڈھیر_ARG_سائز((*ptr)->سائز) > 10 * FFI_SIZEOF_ARG)
	    {
	      /* An argument is never split between the 10 parameter
		 registers and the stack.  */
	      بائٹیں = 10 * FFI_SIZEOF_ARG;
	    }
#اگر_ختم
#اگر_تعریف_شدہ XTENSA
	  اگر (بائٹیں <= 6*4 && بائٹیں + ڈھیر_ARG_سائز((*ptr)->سائز) > 6*4)
	    بائٹیں = 6*4;
#اگر_ختم

	  بائٹیں += (غیرعلامتی صحیح)ڈھیر_ARG_سائز((*ptr)->سائز);
	}
#اگر_ختم
    }

  cif->بائٹیں = بائٹیں;

  /* Perform machine dependent cif processing */
#اگر_تعریف_شدہ FFI_ہدف_SPECIFIC_VARIADIC
  اگر (isvariadic)
	واپس ffi_prep_cif_machdep_var(cif, nfixedargs, ntotalargs);
#اگر_ختم

  واپس ffi_prep_cif_machdep(cif);
}
#اگر_ختم /* not __CRIS__ */

ffi_حالت ffi_prep_cif(ffi_cif *cif, ffi_abi abi, غیرعلامتی صحیح nargs,
			     ffi_type *rtype, ffi_type **atypes)
{
  واپس ffi_prep_cif_core(cif, abi, 0, nargs, nargs, rtype, atypes);
}

ffi_حالت ffi_prep_cif_var(ffi_cif *cif,
                            ffi_abi abi,
                            غیرعلامتی صحیح nfixedargs,
                            غیرعلامتی صحیح ntotalargs,
                            ffi_type *rtype,
                            ffi_type **atypes)
{
  ffi_حالت rc;
  size_t int_سائز = ffi_type_sint.سائز;
  غیرعلامتی صحیح i;

  rc = ffi_prep_cif_core(cif, abi, 1, nfixedargs, ntotalargs, rtype, atypes);

  اگر (rc != FFI_OK)
    واپس rc;

  برائے (i = nfixedargs; i < ntotalargs; i++)
    {
      ffi_type *arg_type = atypes[i];
      اگر (arg_type == &ffi_type_float
          || ((arg_type->type != FFI_TYPE_STRUCT
               && arg_type->type != FFI_TYPE_COMPLEX)
              && arg_type->سائز < int_سائز))
        واپس FFI_BAD_ARGTYPE;
    }

  واپس FFI_OK;
}

#اگر FFI_CLOSURES

ffi_حالت
ffi_prep_احاطہ (ffi_احاطہ* احاطہ,
		  ffi_cif* cif,
		  خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		  خالی *user_ڈیٹا)
{
  واپس ffi_prep_احاطہ_loc (احاطہ, cif, fun, user_ڈیٹا, احاطہ);
}

#اگر_ختم

ffi_حالت
ffi_get_struct_offsets (ffi_abi abi, ffi_type *struct_type, size_t *offsets)
{
  اگر (! (abi > FFI_پہلا_ABI && abi < FFI_آخری_ABI))
    واپس FFI_BAD_ABI;
  اگر (struct_type->type != FFI_TYPE_STRUCT)
    واپس FFI_BAD_TYPEDEF;

#اگر HAVE_LONG_DOUBLE_VARIANT
  ffi_prep_types (abi);
#اگر_ختم

  واپس initialize_aggregate(struct_type, offsets);
}

/* Generic ffi_call_plan: a portable fallback compiled on every target that does
   not provide its own accelerated implementation.  The x86-64 SysV backend
   (ffi64.c) defines these with a fast path under __x86_64__ && !__ILP32__, but
   that file is not built for Windows x86-64 (X86_WIN64), which uses ffiw64.c
   instead -- and clang-cl and MSYS/mingw both define __x86_64__ there.  So
   exclude the fallback only when ffi64.c actually provides it; everywhere else
   this plan just records the cif and invoke calls ffi_call, so the API is
   always present and links on all targets.  The cif must outlive the plan. */
#اگر !(defined(__x86_64__) && !defined(__ILP32__) && !defined(X86_WIN64))

ساخت ffi_call_plan
{
  ffi_cif *cif;
};

ffi_call_plan *
ffi_call_plan_alloc (ffi_cif *cif)
{
  ffi_call_plan *plan = حافظہ_دو (حجم (ساخت ffi_call_plan));
  اگر (plan != تہی)
    plan->cif = cif;
  واپس plan;
}

خالی
ffi_call_plan_invoke (ffi_call_plan *plan, خالی (*fn) (خالی),
		      خالی *rvalue, خالی **avalue)
{
  ffi_call (plan->cif, fn, rvalue, avalue);
}

خالی
ffi_call_plan_free (ffi_call_plan *plan)
{
  آزاد (plan);
}

size_t
ffi_call_plan_سائز (ffi_call_plan *plan)
{
  /* The generic plan is a bare handle; there is no separate move-list.  */
  واپس plan != تہی ? حجم (ساخت ffi_call_plan) : 0;
}

#اگر_ختم /* generic ffi_call_plan fallback */

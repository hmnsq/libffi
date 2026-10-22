/* -----------------------------------------------------------------------
   ffi.c

   CSKY Foreign Function Interface

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

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
تهی ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
 ثبات unsigned صحیح i;
 ثبات تهی **p_argv;
 ثبات نویسه *argp;
 ثبات ffi_type **p_arg;

 argp = پشته;

 اگر ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(تهی **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  size_t z;
  size_t alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#ifdef __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  اگر (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  اگر ((alignment - 1) & (unsigned) argp) {
   argp = (نویسه *) FFI_ALIGN(argp, alignment);
  }

  اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (نویسه *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  اگر (z < اندازه(صحیح))
  {
   z = اندازه(صحیح);
   انتخاب ((*p_arg)->type)
   {
   حالت FFI_TYPE_SINT8:
    *(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
    بشکن;

   حالت FFI_TYPE_UINT8:
    *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
    بشکن;

   حالت FFI_TYPE_SINT16:
    *(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
    بشکن;

   حالت FFI_TYPE_UINT16:
    *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
    بشکن;

   حالت FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#وگرنه
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    بشکن;

   پیشفرض:
    FFI_ASSERT(0);
   }
  }
  وگرنه اگر (z == اندازه(صحیح))
  {
   *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
  }
  وگرنه
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 بازگشت;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {

    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      بشکن;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      وگرنه اگر (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      وگرنه
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      بشکن;

    پیشفرض:
      cif->flags = FFI_TYPE_INT;
      بشکن;
    }

  بازگشت FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned صحیح nfixedargs,
        unsigned صحیح ntotalargs)
{
  بازگشت ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
بیرونی تهی ffi_call_SYSV (تهی (*fn)(تهی), extended_cif *, unsigned, unsigned, unsigned *);

تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  extended_cif ecif;

  صحیح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned صحیح temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  اگر ((rvalue == پوچ) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وگرنه اگر (small_struct)
    ecif.rvalue = &temp;
  وگرنه
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      بشکن;

    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
  اگر (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned نویسه *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#وگرنه
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

ایستا تهی ffi_prep_incoming_args_SYSV (نویسه *پشته, تهی **ret,
      تهی** args, ffi_cif* cif);

تهی ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned صحیح
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     تهی **respp;
     تهی *args;
{
  // our various things...
  ffi_cif       *cif;
  تهی         **arg_area;

  cif         = closure->cif;
  arg_area    = (تهی**) alloca (cif->nargs * اندازه (تهی*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  اگر (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned صحیح tmp = 0;
      tmp = *(unsigned صحیح *)(*respp);
      *(unsigned صحیح *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  بازگشت cif->flags;
}


ایستا تهی
ffi_prep_incoming_args_SYSV(نویسه *پشته, تهی **rvalue,
       تهی **avalue, ffi_cif *cif)
{
  ثبات unsigned صحیح i;
  ثبات تهی **p_argv;
  ثبات نویسه *argp;
  ثبات ffi_type **p_arg;

  argp = پشته;

  اگر ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(تهی **) argp;
    argp += 4;
  }

  p_argv = avalue;

  برای (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      اگر (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      اگر (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      اگر ((alignment - 1) & (unsigned) argp) {
 argp = (نویسه *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned صحیح tmp = 0;
      اگر ((*p_arg)->size < 4) {
        tmp = *(unsigned صحیح *)argp;
        memcpy(argp, ((unsigned نویسه *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#وگرنه
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (تهی*) argp;

      p_argv++;
      argp += z;
    }

  بازگشت;
}

/* How to make a trampoline.  */

بیرونی unsigned نویسه ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#اگر CACHEFLUSH_IN_FFI
بیرونی تهی ffi_csky_cacheflush(unsigned نویسه *__tramp, unsigned صحیح k,
  صحیح i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned نویسه *__tramp = (unsigned نویسه*)(TRAMP);                    \
   unsigned صحیح  __fun = (unsigned صحیح)(FUN);                           \
   unsigned صحیح  __ctx = (unsigned صحیح)(CTX);                           \
   unsigned نویسه *insns = (unsigned نویسه *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned صحیح*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned صحیح*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#وگرنه
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned نویسه *__tramp = (unsigned نویسه*)(TRAMP);                    \
   unsigned صحیح  __fun = (unsigned صحیح)(FUN);                           \
   unsigned صحیح  __ctx = (unsigned صحیح)(CTX);                           \
   unsigned نویسه *insns = (unsigned نویسه *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned صحیح*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned صحیح*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_SIZE-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_SIZE);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#endif

/* the cif must already be prep'ed */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
        ffi_cif* cif,
        تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
        تهی *user_data,
        تهی *codeloc)
{
  تهی (*closure_func)(ffi_closure*) = پوچ;

  اگر (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  وگرنه
    بازگشت FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  بازگشت FFI_OK;
}



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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
 பதிவு unsigned முழு i;
 பதிவு வெறுமை **p_argv;
 பதிவு எழுத்து *argp;
 பதிவு ffi_type **p_arg;

 argp = அடுக்கு;

 எனில் ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(வெறுமை **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  எனில் (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  எனில் ((alignment - 1) & (unsigned) argp) {
   argp = (எழுத்து *) FFI_ALIGN(argp, alignment);
  }

  எனில் ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (எழுத்து *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  எனில் (z < அளவு(முழு))
  {
   z = அளவு(முழு);
   தேர்வு ((*p_arg)->type)
   {
   நிலை FFI_TYPE_SINT8:
    *(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
    நிறுத்து;

   நிலை FFI_TYPE_UINT8:
    *(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
    நிறுத்து;

   நிலை FFI_TYPE_SINT16:
    *(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
    நிறுத்து;

   நிலை FFI_TYPE_UINT16:
    *(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
    நிறுத்து;

   நிலை FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#இல்லையெனில்
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    நிறுத்து;

   இயல்பு:
    FFI_ASSERT(0);
   }
  }
  இல்லையெனில் எனில் (z == அளவு(முழு))
  {
   *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
  }
  இல்லையெனில்
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 திரும்பு;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  தேர்வு (cif->rtype->type)
    {

    நிலை FFI_TYPE_DOUBLE:
    நிலை FFI_TYPE_SINT64:
    நிலை FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      நிறுத்து;

    நிலை FFI_TYPE_STRUCT:
      எனில் (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      இல்லையெனில் எனில் (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      இல்லையெனில்
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      நிறுத்து;

    இயல்பு:
      cif->flags = FFI_TYPE_INT;
      நிறுத்து;
    }

  திரும்பு FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned முழு nfixedargs,
        unsigned முழு ntotalargs)
{
  திரும்பு ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
வெளி வெறுமை ffi_call_SYSV (வெறுமை (*fn)(வெறுமை), extended_cif *, unsigned, unsigned, unsigned *);

வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  extended_cif ecif;

  முழு small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned முழு temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  எனில் ((rvalue == வெற்று) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  இல்லையெனில் எனில் (small_struct)
    ecif.rvalue = &temp;
  இல்லையெனில்
    ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
    {
    நிலை FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      நிறுத்து;

    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
  எனில் (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned எழுத்து *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#இல்லையெனில்
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

நிலையான வெறுமை ffi_prep_incoming_args_SYSV (எழுத்து *அடுக்கு, வெறுமை **ret,
      வெறுமை** args, ffi_cif* cif);

வெறுமை ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned முழு
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     வெறுமை **respp;
     வெறுமை *args;
{
  // our various things...
  ffi_cif       *cif;
  வெறுமை         **arg_area;

  cif         = closure->cif;
  arg_area    = (வெறுமை**) alloca (cif->nargs * அளவு (வெறுமை*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  எனில் (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned முழு tmp = 0;
      tmp = *(unsigned முழு *)(*respp);
      *(unsigned முழு *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  திரும்பு cif->flags;
}


நிலையான வெறுமை
ffi_prep_incoming_args_SYSV(எழுத்து *அடுக்கு, வெறுமை **rvalue,
       வெறுமை **avalue, ffi_cif *cif)
{
  பதிவு unsigned முழு i;
  பதிவு வெறுமை **p_argv;
  பதிவு எழுத்து *argp;
  பதிவு ffi_type **p_arg;

  argp = அடுக்கு;

  எனில் ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(வெறுமை **) argp;
    argp += 4;
  }

  p_argv = avalue;

  க்கு (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      எனில் (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      எனில் (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      எனில் ((alignment - 1) & (unsigned) argp) {
 argp = (எழுத்து *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned முழு tmp = 0;
      எனில் ((*p_arg)->size < 4) {
        tmp = *(unsigned முழு *)argp;
        memcpy(argp, ((unsigned எழுத்து *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#இல்லையெனில்
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (வெறுமை*) argp;

      p_argv++;
      argp += z;
    }

  திரும்பு;
}

/* How to make a trampoline.  */

வெளி unsigned எழுத்து ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#எனில் CACHEFLUSH_IN_FFI
வெளி வெறுமை ffi_csky_cacheflush(unsigned எழுத்து *__tramp, unsigned முழு k,
  முழு i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned எழுத்து *__tramp = (unsigned எழுத்து*)(TRAMP);                    \
   unsigned முழு  __fun = (unsigned முழு)(FUN);                           \
   unsigned முழு  __ctx = (unsigned முழு)(CTX);                           \
   unsigned எழுத்து *insns = (unsigned எழுத்து *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned முழு*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned முழு*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#இல்லையெனில்
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned எழுத்து *__tramp = (unsigned எழுத்து*)(TRAMP);                    \
   unsigned முழு  __fun = (unsigned முழு)(FUN);                           \
   unsigned முழு  __ctx = (unsigned முழு)(CTX);                           \
   unsigned எழுத்து *insns = (unsigned எழுத்து *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned முழு*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned முழு*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
        வெறுமை *user_data,
        வெறுமை *codeloc)
{
  வெறுமை (*closure_func)(ffi_closure*) = வெற்று;

  எனில் (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  இல்லையெனில்
    திரும்பு FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  திரும்பு FFI_OK;
}



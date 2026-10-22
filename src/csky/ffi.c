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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
void ffi_prep_args(var_na *stack, extended_cif *ecif)
{
 pa_njikaa unsigned ຈຳນວນເຕັມ i;
 pa_njikaa void **p_argv;
 pa_njikaa var_na *argp;
 pa_njikaa ffi_type **p_arg;

 argp = stack;

 ຖ້າ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(void **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 ສຳລັບ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  ຖ້າ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  ຖ້າ ((alignment - 1) & (unsigned) argp) {
   argp = (var_na *) FFI_ALIGN(argp, alignment);
  }

  ຖ້າ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (var_na *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  ຖ້າ (z < parimaa_na(ຈຳນວນເຕັມ))
  {
   z = parimaa_na(ຈຳນວນເຕັມ);
   switch ((*p_arg)->type)
   {
   pak_sa FFI_TYPE_SINT8:
    *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ)*(SINT8 *)(* p_argv);
    bha_nga;

   pak_sa FFI_TYPE_UINT8:
    *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT8 *)(* p_argv);
    bha_nga;

   pak_sa FFI_TYPE_SINT16:
    *(signed ຈຳນວນເຕັມ *) argp = (signed ຈຳນວນເຕັມ)*(SINT16 *)(* p_argv);
    bha_nga;

   pak_sa FFI_TYPE_UINT16:
    *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT16 *)(* p_argv);
    bha_nga;

   pak_sa FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#ບໍ່ດັ່ງນັ້ນ
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    bha_nga;

   default:
    FFI_ASSERT(0);
   }
  }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (z == parimaa_na(ຈຳນວນເຕັມ))
  {
   *(unsigned ຈຳນວນເຕັມ *) argp = (unsigned ຈຳນວນເຕັມ)*(UINT32 *)(* p_argv);
  }
  ບໍ່ດັ່ງນັ້ນ
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 ສົ່ງຄືນ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  switch (cif->rtype->type)
    {

    pak_sa FFI_TYPE_DOUBLE:
    pak_sa FFI_TYPE_SINT64:
    pak_sa FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      bha_nga;

    pak_sa FFI_TYPE_STRUCT:
      ຖ້າ (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      ບໍ່ດັ່ງນັ້ນ
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      bha_nga;

    default:
      cif->flags = FFI_TYPE_INT;
      bha_nga;
    }

  ສົ່ງຄືນ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned ຈຳນວນເຕັມ nfixedargs,
        unsigned ຈຳນວນເຕັມ ntotalargs)
{
  ສົ່ງຄືນ ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
extern void ffi_call_SYSV (void (*fn)(void), extended_cif *, unsigned, unsigned, unsigned *);

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;

  ຈຳນວນເຕັມ small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned ຈຳນວນເຕັມ temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  ຖ້າ ((rvalue == NULL) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (small_struct)
    ecif.rvalue = &temp;
  ບໍ່ດັ່ງນັ້ນ
    ecif.rvalue = rvalue;

  switch (cif->abi)
    {
    pak_sa FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      bha_nga;

    default:
      FFI_ASSERT(0);
      bha_nga;
    }
  ຖ້າ (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned var_na *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#ບໍ່ດັ່ງນັ້ນ
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

static void ffi_prep_incoming_args_SYSV (var_na *stack, void **ret,
      void** args, ffi_cif* cif);

void ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned ຈຳນວນເຕັມ
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     void **respp;
     void *args;
{
  // our various things...
  ffi_cif       *cif;
  void         **arg_area;

  cif         = closure->cif;
  arg_area    = (void**) alloca (cif->nargs * parimaa_na (void*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  ຖ້າ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned ຈຳນວນເຕັມ tmp = 0;
      tmp = *(unsigned ຈຳນວນເຕັມ *)(*respp);
      *(unsigned ຈຳນວນເຕັມ *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  ສົ່ງຄືນ cif->flags;
}


static void
ffi_prep_incoming_args_SYSV(var_na *stack, void **rvalue,
       void **avalue, ffi_cif *cif)
{
  pa_njikaa unsigned ຈຳນວນເຕັມ i;
  pa_njikaa void **p_argv;
  pa_njikaa var_na *argp;
  pa_njikaa ffi_type **p_arg;

  argp = stack;

  ຖ້າ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(void **) argp;
    argp += 4;
  }

  p_argv = avalue;

  ສຳລັບ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      ຖ້າ (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      ຖ້າ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      ຖ້າ ((alignment - 1) & (unsigned) argp) {
 argp = (var_na *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned ຈຳນວນເຕັມ tmp = 0;
      ຖ້າ ((*p_arg)->size < 4) {
        tmp = *(unsigned ຈຳນວນເຕັມ *)argp;
        memcpy(argp, ((unsigned var_na *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#ບໍ່ດັ່ງນັ້ນ
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (void*) argp;

      p_argv++;
      argp += z;
    }

  ສົ່ງຄືນ;
}

/* How to make a trampoline.  */

extern unsigned var_na ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#ຖ້າ CACHEFLUSH_IN_FFI
extern void ffi_csky_cacheflush(unsigned var_na *__tramp, unsigned ຈຳນວນເຕັມ k,
  ຈຳນວນເຕັມ i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned var_na *__tramp = (unsigned var_na*)(TRAMP);                    \
   unsigned ຈຳນວນເຕັມ  __fun = (unsigned ຈຳນວນເຕັມ)(FUN);                           \
   unsigned ຈຳນວນເຕັມ  __ctx = (unsigned ຈຳນວນເຕັມ)(CTX);                           \
   unsigned var_na *insns = (unsigned var_na *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ຈຳນວນເຕັມ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ຈຳນວນເຕັມ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ບໍ່ດັ່ງນັ້ນ
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned var_na *__tramp = (unsigned var_na*)(TRAMP);                    \
   unsigned ຈຳນວນເຕັມ  __fun = (unsigned ຈຳນວນເຕັມ)(FUN);                           \
   unsigned ຈຳນວນເຕັມ  __ctx = (unsigned ຈຳນວນເຕັມ)(CTX);                           \
   unsigned var_na *insns = (unsigned var_na *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ຈຳນວນເຕັມ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ຈຳນວນເຕັມ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        void (*fun)(ffi_cif*,void*,void**,void*),
        void *user_data,
        void *codeloc)
{
  void (*closure_func)(ffi_closure*) = NULL;

  ຖ້າ (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  ບໍ່ດັ່ງນັ້ນ
    ສົ່ງຄືນ FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ສົ່ງຄືນ FFI_OK;
}



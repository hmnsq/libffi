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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
فراغ ffi_prep_args(حرف *stack, extended_cif *ecif)
{
 سجل unsigned صحيح i;
 سجل فراغ **p_argv;
 سجل حرف *argp;
 سجل ffi_type **p_arg;

 argp = stack;

 𐡄𐡍 ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(فراغ **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  𐡄𐡍 (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  𐡄𐡍 ((alignment - 1) & (unsigned) argp) {
   argp = (حرف *) FFI_ALIGN(argp, alignment);
  }

  𐡄𐡍 ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (حرف *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  𐡄𐡍 (z < حجم(صحيح))
  {
   z = حجم(صحيح);
   بدل ((*p_arg)->type)
   {
   حالة FFI_TYPE_SINT8:
    *(signed صحيح *) argp = (signed صحيح)*(SINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT8:
    *(unsigned صحيح *) argp = (unsigned صحيح)*(UINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_SINT16:
    *(signed صحيح *) argp = (signed صحيح)*(SINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT16:
    *(unsigned صحيح *) argp = (unsigned صحيح)*(UINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#وإلا
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    اكسر;

   افتراضي:
    FFI_ASSERT(0);
   }
  }
  وإلا 𐡄𐡍 (z == حجم(صحيح))
  {
   *(unsigned صحيح *) argp = (unsigned صحيح)*(UINT32 *)(* p_argv);
  }
  وإلا
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 أرجع;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {

    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      𐡄𐡍 (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      وإلا 𐡄𐡍 (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      وإلا
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned صحيح nfixedargs,
        unsigned صحيح ntotalargs)
{
  أرجع ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
خارجي فراغ ffi_call_SYSV (فراغ (*fn)(فراغ), extended_cif *, unsigned, unsigned, unsigned *);

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  صحيح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned صحيح temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  𐡄𐡍 ((rvalue == عدم) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وإلا 𐡄𐡍 (small_struct)
    ecif.rvalue = &temp;
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
  𐡄𐡍 (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned حرف *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#وإلا
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

ساكن فراغ ffi_prep_incoming_args_SYSV (حرف *stack, فراغ **ret,
      فراغ** args, ffi_cif* cif);

فراغ ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned صحيح
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     فراغ **respp;
     فراغ *args;
{
  // our various things...
  ffi_cif       *cif;
  فراغ         **arg_area;

  cif         = closure->cif;
  arg_area    = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  𐡄𐡍 (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned صحيح tmp = 0;
      tmp = *(unsigned صحيح *)(*respp);
      *(unsigned صحيح *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  أرجع cif->flags;
}


ساكن فراغ
ffi_prep_incoming_args_SYSV(حرف *stack, فراغ **rvalue,
       فراغ **avalue, ffi_cif *cif)
{
  سجل unsigned صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = stack;

  𐡄𐡍 ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(فراغ **) argp;
    argp += 4;
  }

  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      𐡄𐡍 (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      𐡄𐡍 (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      𐡄𐡍 ((alignment - 1) & (unsigned) argp) {
 argp = (حرف *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned صحيح tmp = 0;
      𐡄𐡍 ((*p_arg)->size < 4) {
        tmp = *(unsigned صحيح *)argp;
        memcpy(argp, ((unsigned حرف *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#وإلا
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (فراغ*) argp;

      p_argv++;
      argp += z;
    }

  أرجع;
}

/* How to make a trampoline.  */

خارجي unsigned حرف ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#𐡄𐡍 CACHEFLUSH_IN_FFI
خارجي فراغ ffi_csky_cacheflush(unsigned حرف *__tramp, unsigned صحيح k,
  صحيح i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned حرف *__tramp = (unsigned حرف*)(TRAMP);                    \
   unsigned صحيح  __fun = (unsigned صحيح)(FUN);                           \
   unsigned صحيح  __ctx = (unsigned صحيح)(CTX);                           \
   unsigned حرف *insns = (unsigned حرف *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned صحيح*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned صحيح*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#وإلا
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned حرف *__tramp = (unsigned حرف*)(TRAMP);                    \
   unsigned صحيح  __fun = (unsigned صحيح)(FUN);                           \
   unsigned صحيح  __ctx = (unsigned صحيح)(CTX);                           \
   unsigned حرف *insns = (unsigned حرف *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned صحيح*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned صحيح*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
        فراغ *user_data,
        فراغ *codeloc)
{
  فراغ (*closure_func)(ffi_closure*) = عدم;

  𐡄𐡍 (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  وإلا
    أرجع FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  أرجع FFI_OK;
}



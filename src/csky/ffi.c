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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
 ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
 ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
 ਰਜਿਸਟਰ ਅੱਖਰ *argp;
 ਰਜਿਸਟਰ ffi_type **p_arg;

 argp = ਢੇਰ;

 ਜੇ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(ਖਾਲੀ **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  size_t z;
  size_t alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  ਜੇ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->ਮਾਪ > 8) && (alignment == 8)) {
   alignment = 4;
  }
#ਜੇ_ਅੰਤ

  ਜੇ ((alignment - 1) & (ਬਿਨਾਨਿਸ਼ਾਨੀ) argp) {
   argp = (ਅੱਖਰ *) FFI_ALIGN(argp, alignment);
  }

  ਜੇ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (ਅੱਖਰ *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->ਮਾਪ;
  ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
  {
   z = ਆਕਾਰ(ਪੂਰਨ);
   ਚੋਣ ((*p_arg)->type)
   {
   ਹਾਲਤ FFI_TYPE_SINT8:
    *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
    ਰੋਕੋ;

   ਹਾਲਤ FFI_TYPE_UINT8:
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
    ਰੋਕੋ;

   ਹਾਲਤ FFI_TYPE_SINT16:
    *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
    ਰੋਕੋ;

   ਹਾਲਤ FFI_TYPE_UINT16:
    *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
    ਰੋਕੋ;

   ਹਾਲਤ FFI_TYPE_STRUCT:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYBE__
    ਸਮ੍ਰਿਤੀ_ਨਕਲ((argp + 4 - (*p_arg)->ਮਾਪ), *p_argv, (*p_arg)->ਮਾਪ);
#ਨਹੀਂਤਾਂ
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, (*p_arg)->ਮਾਪ);
#ਜੇ_ਅੰਤ
    ਰੋਕੋ;

   ਮੂਲ:
    FFI_ASSERT(0);
   }
  }
  ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
  {
   *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
  }
  ਨਹੀਂਤਾਂ
  {
   ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 ਵਾਪਸ;
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->ਬਾਈਟਾਂ = (cif->ਬਾਈਟਾਂ + 7) & ~7;

  /* Set the return type flag */
  ਚੋਣ (cif->rtype->type)
    {

    ਹਾਲਤ FFI_TYPE_DOUBLE:
    ਹਾਲਤ FFI_TYPE_SINT64:
    ਹਾਲਤ FFI_TYPE_UINT64:
      cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ) FFI_TYPE_SINT64;
      ਰੋਕੋ;

    ਹਾਲਤ FFI_TYPE_STRUCT:
      ਜੇ (cif->rtype->ਮਾਪ <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_INT;
      ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->ਮਾਪ <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_SINT64;
      ਨਹੀਂਤਾਂ
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (ਬਿਨਾਨਿਸ਼ਾਨੀ)FFI_TYPE_STRUCT;
      ਰੋਕੋ;

    ਮੂਲ:
      cif->flags = FFI_TYPE_INT;
      ਰੋਕੋ;
    }

  ਵਾਪਸ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_var(ffi_cif *cif,
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs,
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  ਵਾਪਸ ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV (ਖਾਲੀ (*fn)(ਖਾਲੀ), extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *);

ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ਪੂਰਨ small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਰਜ਼ੀ;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  ਜੇ ((rvalue == ਨਿਰਾ) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ ਜੇ (small_struct)
    ecif.rvalue = &ਆਰਜ਼ੀ;
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->ਬਾਈਟਾਂ, cif->flags, ecif.rvalue);
      ਰੋਕੋ;

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
  ਜੇ (small_struct)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYBE__
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&ਆਰਜ਼ੀ + (4 - cif->rtype->ਮਾਪ)), cif->rtype->ਮਾਪ);
#ਨਹੀਂਤਾਂ
    ਸਮ੍ਰਿਤੀ_ਨਕਲ (rvalue, &ਆਰਜ਼ੀ, cif->rtype->ਮਾਪ);
#ਜੇ_ਅੰਤ
}

/** private members **/

ਸਥਿਰ ਖਾਲੀ ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV (ਅੱਖਰ *ਢੇਰ, ਖਾਲੀ **ret,
      ਖਾਲੀ** ਦਲੀਲਾਂ, ffi_cif* cif);

ਖਾਲੀ ffi_ਘੇਰਾ_SYSV (ffi_ਘੇਰਾ *);

/* This function is jumped to by the trampoline */

ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ
ffi_ਘੇਰਾ_SYSV_inner (ਘੇਰਾ, respp, ਦਲੀਲਾਂ)
     ffi_ਘੇਰਾ *ਘੇਰਾ;
     ਖਾਲੀ **respp;
     ਖਾਲੀ *ਦਲੀਲਾਂ;
{
  // our various things...
  ffi_cif       *cif;
  ਖਾਲੀ         **arg_area;

  cif         = ਘੇਰਾ->cif;
  arg_area    = (ਖਾਲੀ**) alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਦਲੀਲਾਂ, respp, arg_area, cif);

  (ਘੇਰਾ->fun) (cif, *respp, arg_area, ਘੇਰਾ->user_ਅੰਕੜੇ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYBE__
  ਜੇ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ tmp = 0;
      tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)(*respp);
      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)(*respp) = (tmp >> ((4 - cif->rtype->ਮਾਪ) * 8));
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ cif->flags;
}


ਸਥਿਰ ਖਾਲੀ
ffi_prep_incoming_ਦਲੀਲਾਂ_SYSV(ਅੱਖਰ *ਢੇਰ, ਖਾਲੀ **rvalue,
       ਖਾਲੀ **avalue, ffi_cif *cif)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਰਜਿਸਟਰ ਅੱਖਰ *argp;
  ਰਜਿਸਟਰ ffi_type **p_arg;

  argp = ਢੇਰ;

  ਜੇ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(ਖਾਲੀ **) argp;
    argp += 4;
  }

  p_argv = avalue;

  ਲਈ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      ਜੇ (alignment < 4)
 alignment = 4;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      ਜੇ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->ਮਾਪ > 8) && (alignment == 8)) {
        alignment = 4;
      }
#ਜੇ_ਅੰਤ

      /* Align if necessary */
      ਜੇ ((alignment - 1) & (ਬਿਨਾਨਿਸ਼ਾਨੀ) argp) {
 argp = (ਅੱਖਰ *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->ਮਾਪ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __CSKYBE__
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ tmp = 0;
      ਜੇ ((*p_arg)->ਮਾਪ < 4) {
        tmp = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)argp;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)&tmp + (4 - (*p_arg)->ਮਾਪ)), (*p_arg)->ਮਾਪ);
      }
#ਨਹੀਂਤਾਂ
      /* because we're little endian, this is what it turns into.   */
#ਜੇ_ਅੰਤ
      *p_argv = (ਖਾਲੀ*) argp;

      p_argv++;
      argp += z;
    }

  ਵਾਪਸ;
}

/* How to make a trampoline.  */

ਬਾਹਰੀ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ffi_csky_trampoline[TRAMPOLINE_ਮਾਪ];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CACHEFLUSH_ਵਿੱਚ_FFI 1
#ਜੇ CACHEFLUSH_ਵਿੱਚ_FFI
ਬਾਹਰੀ ਖਾਲੀ ffi_csky_cacheflush(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *__tramp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ k,
  ਪੂਰਨ i);
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *__tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(TRAMP);                    \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __fun = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(FUN);                           \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __ctx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(CTX);                           \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *insns = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)(CTX);                       \
   ਸਮ੍ਰਿਤੀ_ਨਕਲ (__tramp, ffi_csky_trampoline, TRAMPOLINE_ਮਾਪ);              \
   *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[TRAMPOLINE_ਮਾਪ] = __ctx;                  \
   *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[TRAMPOLINE_ਮਾਪ + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_ਮਾਪ, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_ਮਾਪ, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *__tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ*)(TRAMP);                    \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __fun = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(FUN);                           \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  __ctx = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(CTX);                           \
   ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *insns = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)(CTX);                       \
   ਸਮ੍ਰਿਤੀ_ਨਕਲ (__tramp, ffi_csky_trampoline, TRAMPOLINE_ਮਾਪ);              \
   *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[TRAMPOLINE_ਮਾਪ] = __ctx;                  \
   *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ*) &__tramp[TRAMPOLINE_ਮਾਪ + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_ਮਾਪ-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_ਮਾਪ);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ਜੇ_ਅੰਤ

/* the cif must already be prep'ed */

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
        ffi_cif* cif,
        ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
        ਖਾਲੀ *user_ਅੰਕੜੇ,
        ਖਾਲੀ *codeloc)
{
  ਖਾਲੀ (*ਘੇਰਾ_func)(ffi_ਘੇਰਾ*) = ਨਿਰਾ;

  ਜੇ (cif->abi == FFI_SYSV)
    ਘੇਰਾ_func = &ffi_ਘੇਰਾ_SYSV;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&ਘੇਰਾ->tramp[0], \
         ਘੇਰਾ_func,  \
         codeloc);

  ਘੇਰਾ->cif  = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun  = fun;

  ਵਾਪਸ FFI_OK;
}



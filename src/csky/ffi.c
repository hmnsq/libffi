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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
 രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
 രജിസ്റ്റർ ശൂന്യം **p_argv;
 രജിസ്റ്റർ അക്ഷരം *argp;
 രജിസ്റ്റർ ffi_type **p_arg;

 argp = അട്ടി;

 എങ്കിൽ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(ശൂന്യം **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  എങ്കിൽ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  എങ്കിൽ ((alignment - 1) & (unsigned) argp) {
   argp = (അക്ഷരം *) FFI_ALIGN(argp, alignment);
  }

  എങ്കിൽ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (അക്ഷരം *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
  {
   z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
   തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
   {
   അവസ്ഥ FFI_TYPE_SINT8:
    *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* p_argv);
    നിർത്തുക;

   അവസ്ഥ FFI_TYPE_UINT8:
    *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* p_argv);
    നിർത്തുക;

   അവസ്ഥ FFI_TYPE_SINT16:
    *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* p_argv);
    നിർത്തുക;

   അവസ്ഥ FFI_TYPE_UINT16:
    *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* p_argv);
    നിർത്തുക;

   അവസ്ഥ FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#അല്ലെങ്കിൽ
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    നിർത്തുക;

   സ്ഥിരസ്ഥിതി:
    FFI_ASSERT(0);
   }
  }
  അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
  {
   *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
  }
  അല്ലെങ്കിൽ
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 തിരികെ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
    {

    അവസ്ഥ FFI_TYPE_DOUBLE:
    അവസ്ഥ FFI_TYPE_SINT64:
    അവസ്ഥ FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      നിർത്തുക;

    അവസ്ഥ FFI_TYPE_STRUCT:
      എങ്കിൽ (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      അല്ലെങ്കിൽ
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      cif->flags = FFI_TYPE_INT;
      നിർത്തുക;
    }

  തിരികെ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned പൂർണ്ണസംഖ്യ nfixedargs,
        unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  തിരികെ ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
ബാഹ്യ ശൂന്യം ffi_call_SYSV (ശൂന്യം (*fn)(ശൂന്യം), extended_cif *, unsigned, unsigned, unsigned *);

ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  extended_cif ecif;

  പൂർണ്ണസംഖ്യ small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned പൂർണ്ണസംഖ്യ temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  എങ്കിൽ ((rvalue == ശൂന്യ) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ എങ്കിൽ (small_struct)
    ecif.rvalue = &temp;
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
    {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      നിർത്തുക;

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
  എങ്കിൽ (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned അക്ഷരം *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#അല്ലെങ്കിൽ
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

സ്ഥിതം ശൂന്യം ffi_prep_incoming_args_SYSV (അക്ഷരം *അട്ടി, ശൂന്യം **ret,
      ശൂന്യം** args, ffi_cif* cif);

ശൂന്യം ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned പൂർണ്ണസംഖ്യ
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     ശൂന്യം **respp;
     ശൂന്യം *args;
{
  // our various things...
  ffi_cif       *cif;
  ശൂന്യം         **arg_area;

  cif         = closure->cif;
  arg_area    = (ശൂന്യം**) alloca (cif->nargs * വലുപ്പം (ശൂന്യം*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  എങ്കിൽ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned പൂർണ്ണസംഖ്യ tmp = 0;
      tmp = *(unsigned പൂർണ്ണസംഖ്യ *)(*respp);
      *(unsigned പൂർണ്ണസംഖ്യ *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  തിരികെ cif->flags;
}


സ്ഥിതം ശൂന്യം
ffi_prep_incoming_args_SYSV(അക്ഷരം *അട്ടി, ശൂന്യം **rvalue,
       ശൂന്യം **avalue, ffi_cif *cif)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  രജിസ്റ്റർ അക്ഷരം *argp;
  രജിസ്റ്റർ ffi_type **p_arg;

  argp = അട്ടി;

  എങ്കിൽ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(ശൂന്യം **) argp;
    argp += 4;
  }

  p_argv = avalue;

  വേണ്ടി (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      എങ്കിൽ (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      എങ്കിൽ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      എങ്കിൽ ((alignment - 1) & (unsigned) argp) {
 argp = (അക്ഷരം *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned പൂർണ്ണസംഖ്യ tmp = 0;
      എങ്കിൽ ((*p_arg)->size < 4) {
        tmp = *(unsigned പൂർണ്ണസംഖ്യ *)argp;
        memcpy(argp, ((unsigned അക്ഷരം *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#അല്ലെങ്കിൽ
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (ശൂന്യം*) argp;

      p_argv++;
      argp += z;
    }

  തിരികെ;
}

/* How to make a trampoline.  */

ബാഹ്യ unsigned അക്ഷരം ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#എങ്കിൽ CACHEFLUSH_IN_FFI
ബാഹ്യ ശൂന്യം ffi_csky_cacheflush(unsigned അക്ഷരം *__tramp, unsigned പൂർണ്ണസംഖ്യ k,
  പൂർണ്ണസംഖ്യ i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned അക്ഷരം *__tramp = (unsigned അക്ഷരം*)(TRAMP);                    \
   unsigned പൂർണ്ണസംഖ്യ  __fun = (unsigned പൂർണ്ണസംഖ്യ)(FUN);                           \
   unsigned പൂർണ്ണസംഖ്യ  __ctx = (unsigned പൂർണ്ണസംഖ്യ)(CTX);                           \
   unsigned അക്ഷരം *insns = (unsigned അക്ഷരം *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#അല്ലെങ്കിൽ
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned അക്ഷരം *__tramp = (unsigned അക്ഷരം*)(TRAMP);                    \
   unsigned പൂർണ്ണസംഖ്യ  __fun = (unsigned പൂർണ്ണസംഖ്യ)(FUN);                           \
   unsigned പൂർണ്ണസംഖ്യ  __ctx = (unsigned പൂർണ്ണസംഖ്യ)(CTX);                           \
   unsigned അക്ഷരം *insns = (unsigned അക്ഷരം *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned പൂർണ്ണസംഖ്യ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
        ശൂന്യം *user_data,
        ശൂന്യം *codeloc)
{
  ശൂന്യം (*closure_func)(ffi_closure*) = ശൂന്യ;

  എങ്കിൽ (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  അല്ലെങ്കിൽ
    തിരികെ FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  തിരികെ FFI_OK;
}



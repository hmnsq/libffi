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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
 ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
 ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
 ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
 ಪಞ್ಜಿಕಾ ffi_type **p_arg;

 argp = ರಾಶಿ;

 ಯದಿ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(ಶೂನ್ಯ **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  ಯದಿ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  ಯದಿ ((alignment - 1) & (unsigned) argp) {
   argp = (ವರ್ಣ *) FFI_ALIGN(argp, alignment);
  }

  ಯದಿ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (ವರ್ಣ *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
  {
   z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
   ವಿಕಲ್ಪ ((*p_arg)->type)
   {
   ಪಕ್ಷ FFI_TYPE_SINT8:
    *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
    ಭಙ್ಗ;

   ಪಕ್ಷ FFI_TYPE_UINT8:
    *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
    ಭಙ್ಗ;

   ಪಕ್ಷ FFI_TYPE_SINT16:
    *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
    ಭಙ್ಗ;

   ಪಕ್ಷ FFI_TYPE_UINT16:
    *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
    ಭಙ್ಗ;

   ಪಕ್ಷ FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#ಅನ್ಯಥಾ
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    ಭಙ್ಗ;

   ಸ್ವಭಾವ:
    FFI_ASSERT(0);
   }
  }
  ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
  {
   *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
  }
  ಅನ್ಯಥಾ
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 ಪ್ರತ್ಯಾಗಮ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {

    ಪಕ್ಷ FFI_TYPE_DOUBLE:
    ಪಕ್ಷ FFI_TYPE_SINT64:
    ಪಕ್ಷ FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      ಭಙ್ಗ;

    ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಯದಿ (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      ಅನ್ಯಥಾ ಯದಿ (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      ಅನ್ಯಥಾ
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      cif->flags = FFI_TYPE_INT;
      ಭಙ್ಗ;
    }

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs,
        unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV (ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), extended_cif *, unsigned, unsigned, unsigned *);

ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ಪೂರ್ಣಾಙ್ಕ small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned ಪೂರ್ಣಾಙ್ಕ temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  ಯದಿ ((rvalue == ರಿಕ್ತ) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ ಯದಿ (small_struct)
    ecif.rvalue = &temp;
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
    {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      ಭಙ್ಗ;

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
  ಯದಿ (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned ವರ್ಣ *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#ಅನ್ಯಥಾ
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_incoming_args_SYSV (ವರ್ಣ *ರಾಶಿ, ಶೂನ್ಯ **ret,
      ಶೂನ್ಯ** args, ffi_cif* cif);

ಶೂನ್ಯ ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned ಪೂರ್ಣಾಙ್ಕ
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     ಶೂನ್ಯ **respp;
     ಶೂನ್ಯ *args;
{
  // our various things...
  ffi_cif       *cif;
  ಶೂನ್ಯ         **arg_area;

  cif         = closure->cif;
  arg_area    = (ಶೂನ್ಯ**) alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  ಯದಿ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned ಪೂರ್ಣಾಙ್ಕ tmp = 0;
      tmp = *(unsigned ಪೂರ್ಣಾಙ್ಕ *)(*respp);
      *(unsigned ಪೂರ್ಣಾಙ್ಕ *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  ಪ್ರತ್ಯಾಗಮ cif->flags;
}


ಸ್ಥಿರ ಶೂನ್ಯ
ffi_prep_incoming_args_SYSV(ವರ್ಣ *ರಾಶಿ, ಶೂನ್ಯ **rvalue,
       ಶೂನ್ಯ **avalue, ffi_cif *cif)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;

  argp = ರಾಶಿ;

  ಯದಿ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(ಶೂನ್ಯ **) argp;
    argp += 4;
  }

  p_argv = avalue;

  ಪ್ರತಿ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      ಯದಿ (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      ಯದಿ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      ಯದಿ ((alignment - 1) & (unsigned) argp) {
 argp = (ವರ್ಣ *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned ಪೂರ್ಣಾಙ್ಕ tmp = 0;
      ಯದಿ ((*p_arg)->size < 4) {
        tmp = *(unsigned ಪೂರ್ಣಾಙ್ಕ *)argp;
        memcpy(argp, ((unsigned ವರ್ಣ *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#ಅನ್ಯಥಾ
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (ಶೂನ್ಯ*) argp;

      p_argv++;
      argp += z;
    }

  ಪ್ರತ್ಯಾಗಮ;
}

/* How to make a trampoline.  */

ಬಾಹ್ಯ unsigned ವರ್ಣ ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#ಯದಿ CACHEFLUSH_IN_FFI
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_csky_cacheflush(unsigned ವರ್ಣ *__tramp, unsigned ಪೂರ್ಣಾಙ್ಕ k,
  ಪೂರ್ಣಾಙ್ಕ i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned ವರ್ಣ *__tramp = (unsigned ವರ್ಣ*)(TRAMP);                    \
   unsigned ಪೂರ್ಣಾಙ್ಕ  __fun = (unsigned ಪೂರ್ಣಾಙ್ಕ)(FUN);                           \
   unsigned ಪೂರ್ಣಾಙ್ಕ  __ctx = (unsigned ಪೂರ್ಣಾಙ್ಕ)(CTX);                           \
   unsigned ವರ್ಣ *insns = (unsigned ವರ್ಣ *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ಅನ್ಯಥಾ
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned ವರ್ಣ *__tramp = (unsigned ವರ್ಣ*)(TRAMP);                    \
   unsigned ಪೂರ್ಣಾಙ್ಕ  __fun = (unsigned ಪೂರ್ಣಾಙ್ಕ)(FUN);                           \
   unsigned ಪೂರ್ಣಾಙ್ಕ  __ctx = (unsigned ಪೂರ್ಣಾಙ್ಕ)(CTX);                           \
   unsigned ವರ್ಣ *insns = (unsigned ವರ್ಣ *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ಪೂರ್ಣಾಙ್ಕ*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
        ಶೂನ್ಯ *user_data,
        ಶೂನ್ಯ *codeloc)
{
  ಶೂನ್ಯ (*closure_func)(ffi_closure*) = ರಿಕ್ತ;

  ಯದಿ (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}



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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
शून्य ffi_prep_args(वर्ण *राशिः, extended_cif *ecif)
{
 पञ्जिका unsigned पूर्णाङ्क i;
 पञ्जिका शून्य **p_argv;
 पञ्जिका वर्ण *argp;
 पञ्जिका ffi_type **p_arg;

 argp = राशिः;

 जइ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(शून्य **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  जइ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  जइ ((alignment - 1) & (unsigned) argp) {
   argp = (वर्ण *) FFI_ALIGN(argp, alignment);
  }

  जइ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (वर्ण *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  जइ (z < परिमाण(पूर्णाङ्क))
  {
   z = परिमाण(पूर्णाङ्क);
   विकल्प ((*p_arg)->type)
   {
   पक्ष FFI_TYPE_SINT8:
    *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT8 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_UINT8:
    *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT8 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_SINT16:
    *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT16 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_UINT16:
    *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT16 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#अण्णहा
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    भङ्ग;

   स्वभाव:
    FFI_ASSERT(0);
   }
  }
  अण्णहा जइ (z == परिमाण(पूर्णाङ्क))
  {
   *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT32 *)(* p_argv);
  }
  अण्णहा
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 प्रत्यागम;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {

    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      जइ (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      अण्णहा जइ (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      अण्णहा
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      भङ्ग;

    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned पूर्णाङ्क nfixedargs,
        unsigned पूर्णाङ्क ntotalargs)
{
  प्रत्यागम ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
बाह्य शून्य ffi_call_SYSV (शून्य (*fn)(शून्य), extended_cif *, unsigned, unsigned, unsigned *);

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  पूर्णाङ्क small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned पूर्णाङ्क temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  जइ ((rvalue == रिक्त) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अण्णहा जइ (small_struct)
    ecif.rvalue = &temp;
  अण्णहा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
  जइ (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned वर्ण *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#अण्णहा
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

स्थिर शून्य ffi_prep_incoming_args_SYSV (वर्ण *राशिः, शून्य **ret,
      शून्य** args, ffi_cif* cif);

शून्य ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned पूर्णाङ्क
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     शून्य **respp;
     शून्य *args;
{
  // our various things...
  ffi_cif       *cif;
  शून्य         **arg_area;

  cif         = closure->cif;
  arg_area    = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  जइ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned पूर्णाङ्क tmp = 0;
      tmp = *(unsigned पूर्णाङ्क *)(*respp);
      *(unsigned पूर्णाङ्क *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  प्रत्यागम cif->flags;
}


स्थिर शून्य
ffi_prep_incoming_args_SYSV(वर्ण *राशिः, शून्य **rvalue,
       शून्य **avalue, ffi_cif *cif)
{
  पञ्जिका unsigned पूर्णाङ्क i;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;

  argp = राशिः;

  जइ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(शून्य **) argp;
    argp += 4;
  }

  p_argv = avalue;

  प्रति (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      जइ (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      जइ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      जइ ((alignment - 1) & (unsigned) argp) {
 argp = (वर्ण *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned पूर्णाङ्क tmp = 0;
      जइ ((*p_arg)->size < 4) {
        tmp = *(unsigned पूर्णाङ्क *)argp;
        memcpy(argp, ((unsigned वर्ण *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#अण्णहा
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (शून्य*) argp;

      p_argv++;
      argp += z;
    }

  प्रत्यागम;
}

/* How to make a trampoline.  */

बाह्य unsigned वर्ण ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#जइ CACHEFLUSH_IN_FFI
बाह्य शून्य ffi_csky_cacheflush(unsigned वर्ण *__tramp, unsigned पूर्णाङ्क k,
  पूर्णाङ्क i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned वर्ण *__tramp = (unsigned वर्ण*)(TRAMP);                    \
   unsigned पूर्णाङ्क  __fun = (unsigned पूर्णाङ्क)(FUN);                           \
   unsigned पूर्णाङ्क  __ctx = (unsigned पूर्णाङ्क)(CTX);                           \
   unsigned वर्ण *insns = (unsigned वर्ण *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned पूर्णाङ्क*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned पूर्णाङ्क*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#अण्णहा
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned वर्ण *__tramp = (unsigned वर्ण*)(TRAMP);                    \
   unsigned पूर्णाङ्क  __fun = (unsigned पूर्णाङ्क)(FUN);                           \
   unsigned पूर्णाङ्क  __ctx = (unsigned पूर्णाङ्क)(CTX);                           \
   unsigned वर्ण *insns = (unsigned वर्ण *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned पूर्णाङ्क*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned पूर्णाङ्क*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
        शून्य *user_data,
        शून्य *codeloc)
{
  शून्य (*closure_func)(ffi_closure*) = रिक्त;

  जइ (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  अण्णहा
    प्रत्यागम FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  प्रत्यागम FFI_OK;
}



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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
 ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
 ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
 ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
 ପଞ୍ଜିକରଣ ffi_type **p_arg;

 argp = ଗଦା;

 ଯଦି ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(ଶୂନ୍ୟ **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  ଯଦି (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  ଯଦି ((alignment - 1) & (unsigned) argp) {
   argp = (ଅକ୍ଷର *) FFI_ALIGN(argp, alignment);
  }

  ଯଦି ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (ଅକ୍ଷର *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
  {
   z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
   ବାଛ ((*p_arg)->type)
   {
   ସ୍ଥିତି FFI_TYPE_SINT8:
    *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
    ଭାଙ୍ଗ;

   ସ୍ଥିତି FFI_TYPE_UINT8:
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
    ଭାଙ୍ଗ;

   ସ୍ଥିତି FFI_TYPE_SINT16:
    *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
    ଭାଙ୍ଗ;

   ସ୍ଥିତି FFI_TYPE_UINT16:
    *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
    ଭାଙ୍ଗ;

   ସ୍ଥିତି FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#ଅନ୍ୟଥା
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    ଭାଙ୍ଗ;

   ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    FFI_ASSERT(0);
   }
  }
  ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
  {
   *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
  }
  ଅନ୍ୟଥା
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 ଫେରନ୍ତୁ;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  ବାଛ (cif->rtype->type)
    {

    ସ୍ଥିତି FFI_TYPE_DOUBLE:
    ସ୍ଥିତି FFI_TYPE_SINT64:
    ସ୍ଥିତି FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      ଭାଙ୍ଗ;

    ସ୍ଥିତି FFI_TYPE_STRUCT:
      ଯଦି (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      ଅନ୍ୟଥା ଯଦି (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      ଅନ୍ୟଥା
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      cif->flags = FFI_TYPE_INT;
      ଭାଙ୍ଗ;
    }

  ଫେରନ୍ତୁ FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs,
        unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  ଫେରନ୍ତୁ ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV (ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), extended_cif *, unsigned, unsigned, unsigned *);

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  ଯଦି ((rvalue == ଖାଲି) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ଅନ୍ୟଥା ଯଦି (small_struct)
    ecif.rvalue = &temp;
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      ଭାଙ୍ଗ;

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
  ଯଦି (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned ଅକ୍ଷର *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#ଅନ୍ୟଥା
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

ସ୍ଥିର ଶୂନ୍ୟ ffi_prep_incoming_args_SYSV (ଅକ୍ଷର *ଗଦା, ଶୂନ୍ୟ **ret,
      ଶୂନ୍ୟ** args, ffi_cif* cif);

ଶୂନ୍ୟ ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     ଶୂନ୍ୟ **respp;
     ଶୂନ୍ୟ *args;
{
  // our various things...
  ffi_cif       *cif;
  ଶୂନ୍ୟ         **arg_area;

  cif         = closure->cif;
  arg_area    = (ଶୂନ୍ୟ**) alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  ଯଦି (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmp = 0;
      tmp = *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)(*respp);
      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  ଫେରନ୍ତୁ cif->flags;
}


ସ୍ଥିର ଶୂନ୍ୟ
ffi_prep_incoming_args_SYSV(ଅକ୍ଷର *ଗଦା, ଶୂନ୍ୟ **rvalue,
       ଶୂନ୍ୟ **avalue, ffi_cif *cif)
{
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
  ପଞ୍ଜିକରଣ ffi_type **p_arg;

  argp = ଗଦା;

  ଯଦି ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(ଶୂନ୍ୟ **) argp;
    argp += 4;
  }

  p_argv = avalue;

  ପାଇଁ (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      ଯଦି (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      ଯଦି (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      ଯଦି ((alignment - 1) & (unsigned) argp) {
 argp = (ଅକ୍ଷର *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା tmp = 0;
      ଯଦି ((*p_arg)->size < 4) {
        tmp = *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)argp;
        memcpy(argp, ((unsigned ଅକ୍ଷର *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#ଅନ୍ୟଥା
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (ଶୂନ୍ୟ*) argp;

      p_argv++;
      argp += z;
    }

  ଫେରନ୍ତୁ;
}

/* How to make a trampoline.  */

ବାହ୍ୟ unsigned ଅକ୍ଷର ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#ଯଦି CACHEFLUSH_IN_FFI
ବାହ୍ୟ ଶୂନ୍ୟ ffi_csky_cacheflush(unsigned ଅକ୍ଷର *__tramp, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା k,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned ଅକ୍ଷର *__tramp = (unsigned ଅକ୍ଷର*)(TRAMP);                    \
   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __fun = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(FUN);                           \
   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __ctx = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(CTX);                           \
   unsigned ଅକ୍ଷର *insns = (unsigned ଅକ୍ଷର *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ଅନ୍ୟଥା
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned ଅକ୍ଷର *__tramp = (unsigned ଅକ୍ଷର*)(TRAMP);                    \
   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __fun = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(FUN);                           \
   unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  __ctx = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(CTX);                           \
   unsigned ଅକ୍ଷର *insns = (unsigned ଅକ୍ଷର *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
        ଶୂନ୍ୟ *user_data,
        ଶୂନ୍ୟ *codeloc)
{
  ଶୂନ୍ୟ (*closure_func)(ffi_closure*) = ଖାଲି;

  ଯଦି (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  ଫେରନ୍ତୁ FFI_OK;
}



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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
 નોંધણી unsigned પૂર્ણાંક i;
 નોંધણી શૂન્ય **p_argv;
 નોંધણી અક્ષર *argp;
 નોંધણી ffi_type **p_arg;

 argp = ઢગલો;

 જો ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(શૂન્ય **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  જો (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  જો ((alignment - 1) & (unsigned) argp) {
   argp = (અક્ષર *) FFI_ALIGN(argp, alignment);
  }

  જો ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (અક્ષર *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  જો (z < કદ(પૂર્ણાંક))
  {
   z = કદ(પૂર્ણાંક);
   પસંદગી ((*p_arg)->type)
   {
   સ્થિતિ FFI_TYPE_SINT8:
    *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
    તોડો;

   સ્થિતિ FFI_TYPE_UINT8:
    *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
    તોડો;

   સ્થિતિ FFI_TYPE_SINT16:
    *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
    તોડો;

   સ્થિતિ FFI_TYPE_UINT16:
    *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
    તોડો;

   સ્થિતિ FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#નહીંતર
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    તોડો;

   મૂળભૂત:
    FFI_ASSERT(0);
   }
  }
  નહીંતર જો (z == કદ(પૂર્ણાંક))
  {
   *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
  }
  નહીંતર
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 પાછા;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  પસંદગી (cif->rtype->type)
    {

    સ્થિતિ FFI_TYPE_DOUBLE:
    સ્થિતિ FFI_TYPE_SINT64:
    સ્થિતિ FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      તોડો;

    સ્થિતિ FFI_TYPE_STRUCT:
      જો (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      નહીંતર જો (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      નહીંતર
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      તોડો;

    મૂળભૂત:
      cif->flags = FFI_TYPE_INT;
      તોડો;
    }

  પાછા FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned પૂર્ણાંક nfixedargs,
        unsigned પૂર્ણાંક ntotalargs)
{
  પાછા ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
બાહ્ય શૂન્ય ffi_call_SYSV (શૂન્ય (*fn)(શૂન્ય), extended_cif *, unsigned, unsigned, unsigned *);

શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  extended_cif ecif;

  પૂર્ણાંક small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned પૂર્ણાંક temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  જો ((rvalue == ખાલી) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  નહીંતર જો (small_struct)
    ecif.rvalue = &temp;
  નહીંતર
    ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
    {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      તોડો;

    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
  જો (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned અક્ષર *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#નહીંતર
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

સ્થિર શૂન્ય ffi_prep_incoming_args_SYSV (અક્ષર *ઢગલો, શૂન્ય **ret,
      શૂન્ય** args, ffi_cif* cif);

શૂન્ય ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned પૂર્ણાંક
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     શૂન્ય **respp;
     શૂન્ય *args;
{
  // our various things...
  ffi_cif       *cif;
  શૂન્ય         **arg_area;

  cif         = closure->cif;
  arg_area    = (શૂન્ય**) alloca (cif->nargs * કદ (શૂન્ય*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  જો (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned પૂર્ણાંક tmp = 0;
      tmp = *(unsigned પૂર્ણાંક *)(*respp);
      *(unsigned પૂર્ણાંક *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  પાછા cif->flags;
}


સ્થિર શૂન્ય
ffi_prep_incoming_args_SYSV(અક્ષર *ઢગલો, શૂન્ય **rvalue,
       શૂન્ય **avalue, ffi_cif *cif)
{
  નોંધણી unsigned પૂર્ણાંક i;
  નોંધણી શૂન્ય **p_argv;
  નોંધણી અક્ષર *argp;
  નોંધણી ffi_type **p_arg;

  argp = ઢગલો;

  જો ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(શૂન્ય **) argp;
    argp += 4;
  }

  p_argv = avalue;

  માટે (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      જો (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      જો (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      જો ((alignment - 1) & (unsigned) argp) {
 argp = (અક્ષર *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned પૂર્ણાંક tmp = 0;
      જો ((*p_arg)->size < 4) {
        tmp = *(unsigned પૂર્ણાંક *)argp;
        memcpy(argp, ((unsigned અક્ષર *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#નહીંતર
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (શૂન્ય*) argp;

      p_argv++;
      argp += z;
    }

  પાછા;
}

/* How to make a trampoline.  */

બાહ્ય unsigned અક્ષર ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#જો CACHEFLUSH_IN_FFI
બાહ્ય શૂન્ય ffi_csky_cacheflush(unsigned અક્ષર *__tramp, unsigned પૂર્ણાંક k,
  પૂર્ણાંક i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned અક્ષર *__tramp = (unsigned અક્ષર*)(TRAMP);                    \
   unsigned પૂર્ણાંક  __fun = (unsigned પૂર્ણાંક)(FUN);                           \
   unsigned પૂર્ણાંક  __ctx = (unsigned પૂર્ણાંક)(CTX);                           \
   unsigned અક્ષર *insns = (unsigned અક્ષર *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned પૂર્ણાંક*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned પૂર્ણાંક*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#નહીંતર
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned અક્ષર *__tramp = (unsigned અક્ષર*)(TRAMP);                    \
   unsigned પૂર્ણાંક  __fun = (unsigned પૂર્ણાંક)(FUN);                           \
   unsigned પૂર્ણાંક  __ctx = (unsigned પૂર્ણાંક)(CTX);                           \
   unsigned અક્ષર *insns = (unsigned અક્ષર *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned પૂર્ણાંક*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned પૂર્ણાંક*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
        શૂન્ય *user_data,
        શૂન્ય *codeloc)
{
  શૂન્ય (*closure_func)(ffi_closure*) = ખાલી;

  જો (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  નહીંતર
    પાછા FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  પાછા FFI_OK;
}



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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
শূন্য ffi_prep_args(বর্ণ *স্তূপ, extended_cif *ecif)
{
 পঞ্জিকা unsigned পূর্ণাঙ্ক i;
 পঞ্জিকা শূন্য **p_argv;
 পঞ্জিকা বর্ণ *argp;
 পঞ্জিকা ffi_type **p_arg;

 argp = স্তূপ;

 যদি ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(শূন্য **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
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
  যদি (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  যদি ((alignment - 1) & (unsigned) argp) {
   argp = (বর্ণ *) FFI_ALIGN(argp, alignment);
  }

  যদি ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (বর্ণ *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  যদি (z < পরিমাণ(পূর্ণাঙ্ক))
  {
   z = পরিমাণ(পূর্ণাঙ্ক);
   বিকল্প ((*p_arg)->type)
   {
   পক্ষ FFI_TYPE_SINT8:
    *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT8 *)(* p_argv);
    ভঙ্গ;

   পক্ষ FFI_TYPE_UINT8:
    *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT8 *)(* p_argv);
    ভঙ্গ;

   পক্ষ FFI_TYPE_SINT16:
    *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT16 *)(* p_argv);
    ভঙ্গ;

   পক্ষ FFI_TYPE_UINT16:
    *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT16 *)(* p_argv);
    ভঙ্গ;

   পক্ষ FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    memcpy((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#অন্যথা
    memcpy(argp, *p_argv, (*p_arg)->size);
#endif
    ভঙ্গ;

   স্বভাব:
    FFI_ASSERT(0);
   }
  }
  অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
  {
   *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
  }
  অন্যথা
  {
   memcpy(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 প্রত্যাগম;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  বিকল্প (cif->rtype->type)
    {

    পক্ষ FFI_TYPE_DOUBLE:
    পক্ষ FFI_TYPE_SINT64:
    পক্ষ FFI_TYPE_UINT64:
      cif->flags = (unsigned) FFI_TYPE_SINT64;
      ভঙ্গ;

    পক্ষ FFI_TYPE_STRUCT:
      যদি (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (unsigned)FFI_TYPE_INT;
      অন্যথা যদি (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (unsigned)FFI_TYPE_SINT64;
      অন্যথা
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (unsigned)FFI_TYPE_STRUCT;
      ভঙ্গ;

    স্বভাব:
      cif->flags = FFI_TYPE_INT;
      ভঙ্গ;
    }

  প্রত্যাগম FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        unsigned পূর্ণাঙ্ক nfixedargs,
        unsigned পূর্ণাঙ্ক ntotalargs)
{
  প্রত্যাগম ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
বাহ্য শূন্য ffi_call_SYSV (শূন্য (*fn)(শূন্য), extended_cif *, unsigned, unsigned, unsigned *);

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;

  পূর্ণাঙ্ক small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  unsigned পূর্ণাঙ্ক temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  যদি ((rvalue == রিক্ত) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  অন্যথা যদি (small_struct)
    ecif.rvalue = &temp;
  অন্যথা
    ecif.rvalue = rvalue;

  বিকল্প (cif->abi)
    {
    পক্ষ FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      ভঙ্গ;

    স্বভাব:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
  যদি (small_struct)
#ifdef __CSKYBE__
    memcpy (rvalue, ((unsigned বর্ণ *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#অন্যথা
    memcpy (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

স্থির শূন্য ffi_prep_incoming_args_SYSV (বর্ণ *স্তূপ, শূন্য **ret,
      শূন্য** args, ffi_cif* cif);

শূন্য ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

unsigned পূর্ণাঙ্ক
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     শূন্য **respp;
     শূন্য *args;
{
  // our various things...
  ffi_cif       *cif;
  শূন্য         **arg_area;

  cif         = closure->cif;
  arg_area    = (শূন্য**) alloca (cif->nargs * পরিমাণ (শূন্য*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  যদি (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      unsigned পূর্ণাঙ্ক tmp = 0;
      tmp = *(unsigned পূর্ণাঙ্ক *)(*respp);
      *(unsigned পূর্ণাঙ্ক *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  প্রত্যাগম cif->flags;
}


স্থির শূন্য
ffi_prep_incoming_args_SYSV(বর্ণ *স্তূপ, শূন্য **rvalue,
       শূন্য **avalue, ffi_cif *cif)
{
  পঞ্জিকা unsigned পূর্ণাঙ্ক i;
  পঞ্জিকা শূন্য **p_argv;
  পঞ্জিকা বর্ণ *argp;
  পঞ্জিকা ffi_type **p_arg;

  argp = স্তূপ;

  যদি ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(শূন্য **) argp;
    argp += 4;
  }

  p_argv = avalue;

  প্রতি (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      যদি (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      যদি (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      যদি ((alignment - 1) & (unsigned) argp) {
 argp = (বর্ণ *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      unsigned পূর্ণাঙ্ক tmp = 0;
      যদি ((*p_arg)->size < 4) {
        tmp = *(unsigned পূর্ণাঙ্ক *)argp;
        memcpy(argp, ((unsigned বর্ণ *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#অন্যথা
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (শূন্য*) argp;

      p_argv++;
      argp += z;
    }

  প্রত্যাগম;
}

/* How to make a trampoline.  */

বাহ্য unsigned বর্ণ ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#যদি CACHEFLUSH_IN_FFI
বাহ্য শূন্য ffi_csky_cacheflush(unsigned বর্ণ *__tramp, unsigned পূর্ণাঙ্ক k,
  পূর্ণাঙ্ক i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned বর্ণ *__tramp = (unsigned বর্ণ*)(TRAMP);                    \
   unsigned পূর্ণাঙ্ক  __fun = (unsigned পূর্ণাঙ্ক)(FUN);                           \
   unsigned পূর্ণাঙ্ক  __ctx = (unsigned পূর্ণাঙ্ক)(CTX);                           \
   unsigned বর্ণ *insns = (unsigned বর্ণ *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned পূর্ণাঙ্ক*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned পূর্ণাঙ্ক*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#অন্যথা
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ unsigned বর্ণ *__tramp = (unsigned বর্ণ*)(TRAMP);                    \
   unsigned পূর্ণাঙ্ক  __fun = (unsigned পূর্ণাঙ্ক)(FUN);                           \
   unsigned পূর্ণাঙ্ক  __ctx = (unsigned পূর্ণাঙ্ক)(CTX);                           \
   unsigned বর্ণ *insns = (unsigned বর্ণ *)(CTX);                       \
   memcpy (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(unsigned পূর্ণাঙ্ক*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(unsigned পূর্ণাঙ্ক*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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
        শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
        শূন্য *user_data,
        শূন্য *codeloc)
{
  শূন্য (*closure_func)(ffi_closure*) = রিক্ত;

  যদি (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  অন্যথা
    প্রত্যাগম FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  প্রত্যাগম FFI_OK;
}



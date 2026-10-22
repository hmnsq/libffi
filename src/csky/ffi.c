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
शून्य ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
 नोंदणी अचिन्हित पूर्णांक i;
 नोंदणी शून्य **p_argv;
 नोंदणी अक्षर *argp;
 नोंदणी ffi_type **p_arg;

 argp = चळत;

 जर ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(शून्य **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  माप_प्रकार z;
  माप_प्रकार alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#ifdef __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  जर (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
   alignment = 4;
  }
#endif

  जर ((alignment - 1) & (अचिन्हित) argp) {
   argp = (अक्षर *) FFI_ALIGN(argp, alignment);
  }

  जर ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (अक्षर *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->size;
  जर (z < आकार(पूर्णांक))
  {
   z = आकार(पूर्णांक);
   निवड ((*p_arg)->type)
   {
   स्थिती FFI_TYPE_SINT8:
    *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
    थांबा;

   स्थिती FFI_TYPE_UINT8:
    *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
    थांबा;

   स्थिती FFI_TYPE_SINT16:
    *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
    थांबा;

   स्थिती FFI_TYPE_UINT16:
    *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
    थांबा;

   स्थिती FFI_TYPE_STRUCT:
#ifdef __CSKYBE__
    स_नकल((argp + 4 - (*p_arg)->size), *p_argv, (*p_arg)->size);
#अन्यथा
    स_नकल(argp, *p_argv, (*p_arg)->size);
#endif
    थांबा;

   पूर्वनिर्धारित:
    FFI_ASSERT(0);
   }
  }
  अन्यथा जर (z == आकार(पूर्णांक))
  {
   *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
  }
  अन्यथा
  {
   स_नकल(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 परत;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  निवड (cif->rtype->type)
    {

    स्थिती FFI_TYPE_DOUBLE:
    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
      cif->flags = (अचिन्हित) FFI_TYPE_SINT64;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      जर (cif->rtype->size <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (अचिन्हित)FFI_TYPE_INT;
      अन्यथा जर (cif->rtype->size <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (अचिन्हित)FFI_TYPE_SINT64;
      अन्यथा
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (अचिन्हित)FFI_TYPE_STRUCT;
      थांबा;

    पूर्वनिर्धारित:
      cif->flags = FFI_TYPE_INT;
      थांबा;
    }

  परत FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        अचिन्हित पूर्णांक nfixedargs,
        अचिन्हित पूर्णांक ntotalargs)
{
  परत ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
बाह्य शून्य ffi_call_SYSV (शून्य (*fn)(शून्य), extended_cif *, अचिन्हित, अचिन्हित, अचिन्हित *);

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  पूर्णांक small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  अचिन्हित पूर्णांक temp;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  जर ((rvalue == रिक्त) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा जर (small_struct)
    ecif.rvalue = &temp;
  अन्यथा
    ecif.rvalue = rvalue;

  निवड (cif->abi)
    {
    स्थिती FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
  जर (small_struct)
#ifdef __CSKYBE__
    स_नकल (rvalue, ((अचिन्हित अक्षर *)&temp + (4 - cif->rtype->size)), cif->rtype->size);
#अन्यथा
    स_नकल (rvalue, &temp, cif->rtype->size);
#endif
}

/** private members **/

स्थिर शून्य ffi_prep_incoming_args_SYSV (अक्षर *चळत, शून्य **ret,
      शून्य** args, ffi_cif* cif);

शून्य ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

अचिन्हित पूर्णांक
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     शून्य **respp;
     शून्य *args;
{
  // our various things...
  ffi_cif       *cif;
  शून्य         **arg_area;

  cif         = closure->cif;
  arg_area    = (शून्य**) alloca (cif->nargs * आकार (शून्य*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_data);

#ifdef __CSKYBE__
  जर (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      अचिन्हित पूर्णांक tmp = 0;
      tmp = *(अचिन्हित पूर्णांक *)(*respp);
      *(अचिन्हित पूर्णांक *)(*respp) = (tmp >> ((4 - cif->rtype->size) * 8));
  }
#endif

  परत cif->flags;
}


स्थिर शून्य
ffi_prep_incoming_args_SYSV(अक्षर *चळत, शून्य **rvalue,
       शून्य **avalue, ffi_cif *cif)
{
  नोंदणी अचिन्हित पूर्णांक i;
  नोंदणी शून्य **p_argv;
  नोंदणी अक्षर *argp;
  नोंदणी ffi_type **p_arg;

  argp = चळत;

  जर ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(शून्य **) argp;
    argp += 4;
  }

  p_argv = avalue;

  साठी (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;
      माप_प्रकार alignment;

      alignment = (*p_arg)->alignment;
      जर (alignment < 4)
 alignment = 4;

#ifdef __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      जर (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->size > 8) && (alignment == 8)) {
        alignment = 4;
      }
#endif

      /* Align if necessary */
      जर ((alignment - 1) & (अचिन्हित) argp) {
 argp = (अक्षर *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->size;

#ifdef __CSKYBE__
      अचिन्हित पूर्णांक tmp = 0;
      जर ((*p_arg)->size < 4) {
        tmp = *(अचिन्हित पूर्णांक *)argp;
        स_नकल(argp, ((अचिन्हित अक्षर *)&tmp + (4 - (*p_arg)->size)), (*p_arg)->size);
      }
#अन्यथा
      /* because we're little endian, this is what it turns into.   */
#endif
      *p_argv = (शून्य*) argp;

      p_argv++;
      argp += z;
    }

  परत;
}

/* How to make a trampoline.  */

बाह्य अचिन्हित अक्षर ffi_csky_trampoline[TRAMPOLINE_SIZE];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#define CACHEFLUSH_IN_FFI 1
#जर CACHEFLUSH_IN_FFI
बाह्य शून्य ffi_csky_cacheflush(अचिन्हित अक्षर *__tramp, अचिन्हित पूर्णांक k,
  पूर्णांक i);
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिन्हित अक्षर *__tramp = (अचिन्हित अक्षर*)(TRAMP);                    \
   अचिन्हित पूर्णांक  __fun = (अचिन्हित पूर्णांक)(FUN);                           \
   अचिन्हित पूर्णांक  __ctx = (अचिन्हित पूर्णांक)(CTX);                           \
   अचिन्हित अक्षर *insns = (अचिन्हित अक्षर *)(CTX);                       \
   स_नकल (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_SIZE, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_SIZE, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#अन्यथा
#define FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिन्हित अक्षर *__tramp = (अचिन्हित अक्षर*)(TRAMP);                    \
   अचिन्हित पूर्णांक  __fun = (अचिन्हित पूर्णांक)(FUN);                           \
   अचिन्हित पूर्णांक  __ctx = (अचिन्हित पूर्णांक)(CTX);                           \
   अचिन्हित अक्षर *insns = (अचिन्हित अक्षर *)(CTX);                       \
   स_नकल (__tramp, ffi_csky_trampoline, TRAMPOLINE_SIZE);              \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_SIZE] = __ctx;                  \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_SIZE + 4] = __fun;              \
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

  जर (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  अन्यथा
    परत FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_data = user_data;
  closure->fun  = fun;

  परत FFI_OK;
}



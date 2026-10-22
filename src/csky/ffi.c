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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
व्योम ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
 रेजिस्टर अचिन्हित पूर्णांक i;
 रेजिस्टर व्योम **p_argv;
 रेजिस्टर अक्षर *argp;
 रेजिस्टर ffi_type **p_arg;

 argp = ढेर;

 अगर ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(व्योम **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  माप_प्रकार z;
  माप_प्रकार alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#अगर_घोषित __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  अगर (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->आकार > 8) && (alignment == 8)) {
   alignment = 4;
  }
#पूर्ण_अगर

  अगर ((alignment - 1) & (अचिन्हित) argp) {
   argp = (अक्षर *) FFI_ALIGN(argp, alignment);
  }

  अगर ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (अक्षर *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->आकार;
  अगर (z < माप(पूर्णांक))
  {
   z = माप(पूर्णांक);
   चयन ((*p_arg)->type)
   {
   हाल FFI_TYPE_SINT8:
    *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
    अवरोध;

   हाल FFI_TYPE_UINT8:
    *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
    अवरोध;

   हाल FFI_TYPE_SINT16:
    *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
    अवरोध;

   हाल FFI_TYPE_UINT16:
    *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
    अवरोध;

   हाल FFI_TYPE_STRUCT:
#अगर_घोषित __CSKYBE__
    स_नकल((argp + 4 - (*p_arg)->आकार), *p_argv, (*p_arg)->आकार);
#अन्यथा
    स_नकल(argp, *p_argv, (*p_arg)->आकार);
#पूर्ण_अगर
    अवरोध;

   शेष:
    FFI_ASSERT(0);
   }
  }
  अन्यथा अगर (z == माप(पूर्णांक))
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

 वापस;
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->bytes = (cif->bytes + 7) & ~7;

  /* Set the return type flag */
  चयन (cif->rtype->type)
    {

    हाल FFI_TYPE_DOUBLE:
    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
      cif->flags = (अचिन्हित) FFI_TYPE_SINT64;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      अगर (cif->rtype->आकार <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (अचिन्हित)FFI_TYPE_INT;
      अन्यथा अगर (cif->rtype->आकार <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (अचिन्हित)FFI_TYPE_SINT64;
      अन्यथा
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (अचिन्हित)FFI_TYPE_STRUCT;
      अवरोध;

    शेष:
      cif->flags = FFI_TYPE_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
        अचिन्हित पूर्णांक nfixedargs,
        अचिन्हित पूर्णांक ntotalargs)
{
  वापस ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
बाह्य व्योम ffi_call_SYSV (व्योम (*fn)(व्योम), extended_cif *, अचिन्हित, अचिन्हित, अचिन्हित *);

व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;

  पूर्णांक small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  अचिन्हित पूर्णांक अस्थायी;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  अगर ((rvalue == शून्य) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा अगर (small_struct)
    ecif.rvalue = &अस्थायी;
  अन्यथा
    ecif.rvalue = rvalue;

  चयन (cif->abi)
    {
    हाल FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->bytes, cif->flags, ecif.rvalue);
      अवरोध;

    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }
  अगर (small_struct)
#अगर_घोषित __CSKYBE__
    स_नकल (rvalue, ((अचिन्हित अक्षर *)&अस्थायी + (4 - cif->rtype->आकार)), cif->rtype->आकार);
#अन्यथा
    स_नकल (rvalue, &अस्थायी, cif->rtype->आकार);
#पूर्ण_अगर
}

/** private members **/

अटल व्योम ffi_prep_incoming_args_SYSV (अक्षर *ढेर, व्योम **ret,
      व्योम** args, ffi_cif* cif);

व्योम ffi_closure_SYSV (ffi_closure *);

/* This function is jumped to by the trampoline */

अचिन्हित पूर्णांक
ffi_closure_SYSV_inner (closure, respp, args)
     ffi_closure *closure;
     व्योम **respp;
     व्योम *args;
{
  // our various things...
  ffi_cif       *cif;
  व्योम         **arg_area;

  cif         = closure->cif;
  arg_area    = (व्योम**) alloca (cif->nargs * माप (व्योम*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_args_SYSV(args, respp, arg_area, cif);

  (closure->fun) (cif, *respp, arg_area, closure->user_तथ्य);

#अगर_घोषित __CSKYBE__
  अगर (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      अचिन्हित पूर्णांक tmp = 0;
      tmp = *(अचिन्हित पूर्णांक *)(*respp);
      *(अचिन्हित पूर्णांक *)(*respp) = (tmp >> ((4 - cif->rtype->आकार) * 8));
  }
#पूर्ण_अगर

  वापस cif->flags;
}


अटल व्योम
ffi_prep_incoming_args_SYSV(अक्षर *ढेर, व्योम **rvalue,
       व्योम **avalue, ffi_cif *cif)
{
  रेजिस्टर अचिन्हित पूर्णांक i;
  रेजिस्टर व्योम **p_argv;
  रेजिस्टर अक्षर *argp;
  रेजिस्टर ffi_type **p_arg;

  argp = ढेर;

  अगर ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(व्योम **) argp;
    argp += 4;
  }

  p_argv = avalue;

  क्रम (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      माप_प्रकार z;
      माप_प्रकार alignment;

      alignment = (*p_arg)->alignment;
      अगर (alignment < 4)
 alignment = 4;

#अगर_घोषित __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      अगर (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->आकार > 8) && (alignment == 8)) {
        alignment = 4;
      }
#पूर्ण_अगर

      /* Align if necessary */
      अगर ((alignment - 1) & (अचिन्हित) argp) {
 argp = (अक्षर *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->आकार;

#अगर_घोषित __CSKYBE__
      अचिन्हित पूर्णांक tmp = 0;
      अगर ((*p_arg)->आकार < 4) {
        tmp = *(अचिन्हित पूर्णांक *)argp;
        स_नकल(argp, ((अचिन्हित अक्षर *)&tmp + (4 - (*p_arg)->आकार)), (*p_arg)->आकार);
      }
#अन्यथा
      /* because we're little endian, this is what it turns into.   */
#पूर्ण_अगर
      *p_argv = (व्योम*) argp;

      p_argv++;
      argp += z;
    }

  वापस;
}

/* How to make a trampoline.  */

बाह्य अचिन्हित अक्षर ffi_csky_trampoline[TRAMPOLINE_आकार];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#घोषणा CACHEFLUSH_IN_FFI 1
#अगर CACHEFLUSH_IN_FFI
बाह्य व्योम ffi_csky_cacheflush(अचिन्हित अक्षर *__tramp, अचिन्हित पूर्णांक k,
  पूर्णांक i);
#घोषणा FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिन्हित अक्षर *__tramp = (अचिन्हित अक्षर*)(TRAMP);                    \
   अचिन्हित पूर्णांक  __fun = (अचिन्हित पूर्णांक)(FUN);                           \
   अचिन्हित पूर्णांक  __ctx = (अचिन्हित पूर्णांक)(CTX);                           \
   अचिन्हित अक्षर *insns = (अचिन्हित अक्षर *)(CTX);                       \
   स_नकल (__tramp, ffi_csky_trampoline, TRAMPOLINE_आकार);              \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_आकार] = __ctx;                  \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_आकार + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_आकार, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_आकार, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#अन्यथा
#घोषणा FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिन्हित अक्षर *__tramp = (अचिन्हित अक्षर*)(TRAMP);                    \
   अचिन्हित पूर्णांक  __fun = (अचिन्हित पूर्णांक)(FUN);                           \
   अचिन्हित पूर्णांक  __ctx = (अचिन्हित पूर्णांक)(CTX);                           \
   अचिन्हित अक्षर *insns = (अचिन्हित अक्षर *)(CTX);                       \
   स_नकल (__tramp, ffi_csky_trampoline, TRAMPOLINE_आकार);              \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_आकार] = __ctx;                  \
   *(अचिन्हित पूर्णांक*) &__tramp[TRAMPOLINE_आकार + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_आकार-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_आकार);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#पूर्ण_अगर

/* the cif must already be prep'ed */

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
        ffi_cif* cif,
        व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
        व्योम *user_तथ्य,
        व्योम *codeloc)
{
  व्योम (*closure_func)(ffi_closure*) = शून्य;

  अगर (cif->abi == FFI_SYSV)
    closure_func = &ffi_closure_SYSV;
  अन्यथा
    वापस FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&closure->tramp[0], \
         closure_func,  \
         codeloc);

  closure->cif  = cif;
  closure->user_तथ्य = user_तथ्य;
  closure->fun  = fun;

  वापस FFI_OK;
}



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
शून्य ffi_prep_तर्काः(वर्ण *राशिः, extended_cif *ecif)
{
 पञ्जिका अचिह्नित पूर्णाङ्क i;
 पञ्जिका शून्य **p_argv;
 पञ्जिका वर्ण *argp;
 पञ्जिका ffi_type **p_arg;

 argp = राशिः;

 यदि ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
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
#यदि_परिभाषितम् __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  यदि (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->आकार > 8) && (alignment == 8)) {
   alignment = 4;
  }
#यदि_अन्तः

  यदि ((alignment - 1) & (अचिह्नित) argp) {
   argp = (वर्ण *) FFI_ALIGN(argp, alignment);
  }

  यदि ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (वर्ण *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->आकार;
  यदि (z < परिमाण(पूर्णाङ्क))
  {
   z = परिमाण(पूर्णाङ्क);
   विकल्प ((*p_arg)->type)
   {
   पक्ष FFI_TYPE_SINT8:
    *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT8 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_UINT8:
    *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT8 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_SINT16:
    *(चिह्नित पूर्णाङ्क *) argp = (चिह्नित पूर्णाङ्क)*(SINT16 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_UINT16:
    *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT16 *)(* p_argv);
    भङ्ग;

   पक्ष FFI_TYPE_STRUCT:
#यदि_परिभाषितम् __CSKYBE__
    स्मृतिं_प्रतिलिख((argp + 4 - (*p_arg)->आकार), *p_argv, (*p_arg)->आकार);
#अन्यथा
    स्मृतिं_प्रतिलिख(argp, *p_argv, (*p_arg)->आकार);
#यदि_अन्तः
    भङ्ग;

   स्वभाव:
    FFI_ASSERT(0);
   }
  }
  अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
  {
   *(अचिह्नित पूर्णाङ्क *) argp = (अचिह्नित पूर्णाङ्क)*(UINT32 *)(* p_argv);
  }
  अन्यथा
  {
   स्मृतिं_प्रतिलिख(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 प्रत्यागम;
}

/* Perform machine dependent cif processing */
ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->अष्टकानि = (cif->अष्टकानि + 7) & ~7;

  /* Set the return type flag */
  विकल्प (cif->rtype->type)
    {

    पक्ष FFI_TYPE_DOUBLE:
    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
      cif->flags = (अचिह्नित) FFI_TYPE_SINT64;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      यदि (cif->rtype->आकार <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (अचिह्नित)FFI_TYPE_INT;
      अन्यथा यदि (cif->rtype->आकार <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (अचिह्नित)FFI_TYPE_SINT64;
      अन्यथा
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (अचिह्नित)FFI_TYPE_STRUCT;
      भङ्ग;

    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_स्थितिः ffi_prep_cif_machdep_var(ffi_cif *cif,
        अचिह्नित पूर्णाङ्क nfixedargs,
        अचिह्नित पूर्णाङ्क ntotalargs)
{
  प्रत्यागम ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
बाह्य शून्य ffi_call_SYSV (शून्य (*fn)(शून्य), extended_cif *, अचिह्नित, अचिह्नित, अचिह्नित *);

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;

  पूर्णाङ्क small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  अचिह्नित पूर्णाङ्क अस्थायी;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  यदि ((rvalue == रिक्त) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा यदि (small_struct)
    ecif.rvalue = &अस्थायी;
  अन्यथा
    ecif.rvalue = rvalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->अष्टकानि, cif->flags, ecif.rvalue);
      भङ्ग;

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
  यदि (small_struct)
#यदि_परिभाषितम् __CSKYBE__
    स्मृतिं_प्रतिलिख (rvalue, ((अचिह्नित वर्ण *)&अस्थायी + (4 - cif->rtype->आकार)), cif->rtype->आकार);
#अन्यथा
    स्मृतिं_प्रतिलिख (rvalue, &अस्थायी, cif->rtype->आकार);
#यदि_अन्तः
}

/** private members **/

स्थिर शून्य ffi_prep_incoming_तर्काः_SYSV (वर्ण *राशिः, शून्य **ret,
      शून्य** तर्काः, ffi_cif* cif);

शून्य ffi_संवृतिः_SYSV (ffi_संवृतिः *);

/* This function is jumped to by the trampoline */

अचिह्नित पूर्णाङ्क
ffi_संवृतिः_SYSV_inner (संवृतिः, respp, तर्काः)
     ffi_संवृतिः *संवृतिः;
     शून्य **respp;
     शून्य *तर्काः;
{
  // our various things...
  ffi_cif       *cif;
  शून्य         **arg_area;

  cif         = संवृतिः->cif;
  arg_area    = (शून्य**) alloca (cif->nargs * परिमाण (शून्य*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_तर्काः_SYSV(तर्काः, respp, arg_area, cif);

  (संवृतिः->fun) (cif, *respp, arg_area, संवृतिः->user_दत्त);

#यदि_परिभाषितम् __CSKYBE__
  यदि (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      अचिह्नित पूर्णाङ्क tmp = 0;
      tmp = *(अचिह्नित पूर्णाङ्क *)(*respp);
      *(अचिह्नित पूर्णाङ्क *)(*respp) = (tmp >> ((4 - cif->rtype->आकार) * 8));
  }
#यदि_अन्तः

  प्रत्यागम cif->flags;
}


स्थिर शून्य
ffi_prep_incoming_तर्काः_SYSV(वर्ण *राशिः, शून्य **rvalue,
       शून्य **avalue, ffi_cif *cif)
{
  पञ्जिका अचिह्नित पूर्णाङ्क i;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;

  argp = राशिः;

  यदि ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(शून्य **) argp;
    argp += 4;
  }

  p_argv = avalue;

  प्रति (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      यदि (alignment < 4)
 alignment = 4;

#यदि_परिभाषितम् __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      यदि (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->आकार > 8) && (alignment == 8)) {
        alignment = 4;
      }
#यदि_अन्तः

      /* Align if necessary */
      यदि ((alignment - 1) & (अचिह्नित) argp) {
 argp = (वर्ण *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->आकार;

#यदि_परिभाषितम् __CSKYBE__
      अचिह्नित पूर्णाङ्क tmp = 0;
      यदि ((*p_arg)->आकार < 4) {
        tmp = *(अचिह्नित पूर्णाङ्क *)argp;
        स्मृतिं_प्रतिलिख(argp, ((अचिह्नित वर्ण *)&tmp + (4 - (*p_arg)->आकार)), (*p_arg)->आकार);
      }
#अन्यथा
      /* because we're little endian, this is what it turns into.   */
#यदि_अन्तः
      *p_argv = (शून्य*) argp;

      p_argv++;
      argp += z;
    }

  प्रत्यागम;
}

/* How to make a trampoline.  */

बाह्य अचिह्नित वर्ण ffi_csky_trampoline[TRAMPOLINE_आकार];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#परिभाषय CACHEFLUSH_अन्तर्_FFI 1
#यदि CACHEFLUSH_अन्तर्_FFI
बाह्य शून्य ffi_csky_cacheflush(अचिह्नित वर्ण *__tramp, अचिह्नित पूर्णाङ्क k,
  पूर्णाङ्क i);
#परिभाषय FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिह्नित वर्ण *__tramp = (अचिह्नित वर्ण*)(TRAMP);                    \
   अचिह्नित पूर्णाङ्क  __fun = (अचिह्नित पूर्णाङ्क)(FUN);                           \
   अचिह्नित पूर्णाङ्क  __ctx = (अचिह्नित पूर्णाङ्क)(CTX);                           \
   अचिह्नित वर्ण *insns = (अचिह्नित वर्ण *)(CTX);                       \
   स्मृतिं_प्रतिलिख (__tramp, ffi_csky_trampoline, TRAMPOLINE_आकार);              \
   *(अचिह्नित पूर्णाङ्क*) &__tramp[TRAMPOLINE_आकार] = __ctx;                  \
   *(अचिह्नित पूर्णाङ्क*) &__tramp[TRAMPOLINE_आकार + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_आकार, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_आकार, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#अन्यथा
#परिभाषय FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ अचिह्नित वर्ण *__tramp = (अचिह्नित वर्ण*)(TRAMP);                    \
   अचिह्नित पूर्णाङ्क  __fun = (अचिह्नित पूर्णाङ्क)(FUN);                           \
   अचिह्नित पूर्णाङ्क  __ctx = (अचिह्नित पूर्णाङ्क)(CTX);                           \
   अचिह्नित वर्ण *insns = (अचिह्नित वर्ण *)(CTX);                       \
   स्मृतिं_प्रतिलिख (__tramp, ffi_csky_trampoline, TRAMPOLINE_आकार);              \
   *(अचिह्नित पूर्णाङ्क*) &__tramp[TRAMPOLINE_आकार] = __ctx;                  \
   *(अचिह्नित पूर्णाङ्क*) &__tramp[TRAMPOLINE_आकार + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_आकार-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_आकार);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#यदि_अन्तः

/* the cif must already be prep'ed */

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः* संवृतिः,
        ffi_cif* cif,
        शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
        शून्य *user_दत्त,
        शून्य *codeloc)
{
  शून्य (*संवृतिः_func)(ffi_संवृतिः*) = रिक्त;

  यदि (cif->abi == FFI_SYSV)
    संवृतिः_func = &ffi_संवृतिः_SYSV;
  अन्यथा
    प्रत्यागम FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&संवृतिः->tramp[0], \
         संवृतिः_func,  \
         codeloc);

  संवृतिः->cif  = cif;
  संवृतिः->user_दत्त = user_दत्त;
  संवृतिः->fun  = fun;

  प्रत्यागम FFI_OK;
}



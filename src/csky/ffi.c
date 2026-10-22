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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
فراغ ffi_prep_آرگومانها(حرف *پشته, extended_cif *ecif)
{
 سجل بیعلامت صحيح i;
 سجل فراغ **p_argv;
 سجل حرف *argp;
 سجل ffi_type **p_arg;

 argp = پشته;

 إذا ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(فراغ **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  size_t z;
  size_t alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#اگر_تعریف_شده __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->اندازه > 8) && (alignment == 8)) {
   alignment = 4;
  }
#پایان_اگر

  إذا ((alignment - 1) & (بیعلامت) argp) {
   argp = (حرف *) FFI_ALIGN(argp, alignment);
  }

  إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (حرف *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->اندازه;
  إذا (z < حجم(صحيح))
  {
   z = حجم(صحيح);
   بدل ((*p_arg)->type)
   {
   حالة FFI_TYPE_SINT8:
    *(علامتدار صحيح *) argp = (علامتدار صحيح)*(SINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT8:
    *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_SINT16:
    *(علامتدار صحيح *) argp = (علامتدار صحيح)*(SINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT16:
    *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_STRUCT:
#اگر_تعریف_شده __CSKYBE__
    کپی_حافظه((argp + 4 - (*p_arg)->اندازه), *p_argv, (*p_arg)->اندازه);
#وإلا
    کپی_حافظه(argp, *p_argv, (*p_arg)->اندازه);
#پایان_اگر
    اكسر;

   افتراضي:
    FFI_ASSERT(0);
   }
  }
  وإلا إذا (z == حجم(صحيح))
  {
   *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT32 *)(* p_argv);
  }
  وإلا
  {
   کپی_حافظه(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 أرجع;
}

/* Perform machine dependent cif processing */
ffi_وضعیت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->بایتها = (cif->بایتها + 7) & ~7;

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {

    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (بیعلامت) FFI_TYPE_SINT64;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->اندازه <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (بیعلامت)FFI_TYPE_INT;
      وإلا إذا (cif->rtype->اندازه <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (بیعلامت)FFI_TYPE_SINT64;
      وإلا
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (بیعلامت)FFI_TYPE_STRUCT;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_وضعیت ffi_prep_cif_machdep_var(ffi_cif *cif,
        بیعلامت صحيح nfixedargs,
        بیعلامت صحيح ntotalargs)
{
  أرجع ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
خارجي فراغ ffi_call_SYSV (فراغ (*fn)(فراغ), extended_cif *, بیعلامت, بیعلامت, بیعلامت *);

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  صحيح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  بیعلامت صحيح موقت;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  إذا ((rvalue == عدم) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->اندازه);
    }
  وإلا إذا (small_struct)
    ecif.rvalue = &موقت;
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->بایتها, cif->flags, ecif.rvalue);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
  إذا (small_struct)
#اگر_تعریف_شده __CSKYBE__
    کپی_حافظه (rvalue, ((بیعلامت حرف *)&موقت + (4 - cif->rtype->اندازه)), cif->rtype->اندازه);
#وإلا
    کپی_حافظه (rvalue, &موقت, cif->rtype->اندازه);
#پایان_اگر
}

/** private members **/

ساكن فراغ ffi_prep_incoming_آرگومانها_SYSV (حرف *پشته, فراغ **ret,
      فراغ** آرگومانها, ffi_cif* cif);

فراغ ffi_بستگی_SYSV (ffi_بستگی *);

/* This function is jumped to by the trampoline */

بیعلامت صحيح
ffi_بستگی_SYSV_inner (بستگی, respp, آرگومانها)
     ffi_بستگی *بستگی;
     فراغ **respp;
     فراغ *آرگومانها;
{
  // our various things...
  ffi_cif       *cif;
  فراغ         **arg_area;

  cif         = بستگی->cif;
  arg_area    = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_آرگومانها_SYSV(آرگومانها, respp, arg_area, cif);

  (بستگی->fun) (cif, *respp, arg_area, بستگی->user_داده);

#اگر_تعریف_شده __CSKYBE__
  إذا (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      بیعلامت صحيح tmp = 0;
      tmp = *(بیعلامت صحيح *)(*respp);
      *(بیعلامت صحيح *)(*respp) = (tmp >> ((4 - cif->rtype->اندازه) * 8));
  }
#پایان_اگر

  أرجع cif->flags;
}


ساكن فراغ
ffi_prep_incoming_آرگومانها_SYSV(حرف *پشته, فراغ **rvalue,
       فراغ **avalue, ffi_cif *cif)
{
  سجل بیعلامت صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = پشته;

  إذا ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(فراغ **) argp;
    argp += 4;
  }

  p_argv = avalue;

  لكل (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      إذا (alignment < 4)
 alignment = 4;

#اگر_تعریف_شده __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->اندازه > 8) && (alignment == 8)) {
        alignment = 4;
      }
#پایان_اگر

      /* Align if necessary */
      إذا ((alignment - 1) & (بیعلامت) argp) {
 argp = (حرف *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->اندازه;

#اگر_تعریف_شده __CSKYBE__
      بیعلامت صحيح tmp = 0;
      إذا ((*p_arg)->اندازه < 4) {
        tmp = *(بیعلامت صحيح *)argp;
        کپی_حافظه(argp, ((بیعلامت حرف *)&tmp + (4 - (*p_arg)->اندازه)), (*p_arg)->اندازه);
      }
#وإلا
      /* because we're little endian, this is what it turns into.   */
#پایان_اگر
      *p_argv = (فراغ*) argp;

      p_argv++;
      argp += z;
    }

  أرجع;
}

/* How to make a trampoline.  */

خارجي بیعلامت حرف ffi_csky_trampoline[TRAMPOLINE_اندازه];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#تعریف_کن CACHEFLUSH_درون_FFI 1
#إذا CACHEFLUSH_درون_FFI
خارجي فراغ ffi_csky_cacheflush(بیعلامت حرف *__tramp, بیعلامت صحيح k,
  صحيح i);
#تعریف_کن FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بیعلامت حرف *__tramp = (بیعلامت حرف*)(TRAMP);                    \
   بیعلامت صحيح  __fun = (بیعلامت صحيح)(FUN);                           \
   بیعلامت صحيح  __ctx = (بیعلامت صحيح)(CTX);                           \
   بیعلامت حرف *insns = (بیعلامت حرف *)(CTX);                       \
   کپی_حافظه (__tramp, ffi_csky_trampoline, TRAMPOLINE_اندازه);              \
   *(بیعلامت صحيح*) &__tramp[TRAMPOLINE_اندازه] = __ctx;                  \
   *(بیعلامت صحيح*) &__tramp[TRAMPOLINE_اندازه + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_اندازه, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_اندازه, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#وإلا
#تعریف_کن FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بیعلامت حرف *__tramp = (بیعلامت حرف*)(TRAMP);                    \
   بیعلامت صحيح  __fun = (بیعلامت صحيح)(FUN);                           \
   بیعلامت صحيح  __ctx = (بیعلامت صحيح)(CTX);                           \
   بیعلامت حرف *insns = (بیعلامت حرف *)(CTX);                       \
   کپی_حافظه (__tramp, ffi_csky_trampoline, TRAMPOLINE_اندازه);              \
   *(بیعلامت صحيح*) &__tramp[TRAMPOLINE_اندازه] = __ctx;                  \
   *(بیعلامت صحيح*) &__tramp[TRAMPOLINE_اندازه + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_اندازه-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_اندازه);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#پایان_اگر

/* the cif must already be prep'ed */

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
        ffi_cif* cif,
        فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
        فراغ *user_داده,
        فراغ *codeloc)
{
  فراغ (*بستگی_func)(ffi_بستگی*) = عدم;

  إذا (cif->abi == FFI_SYSV)
    بستگی_func = &ffi_بستگی_SYSV;
  وإلا
    أرجع FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&بستگی->tramp[0], \
         بستگی_func,  \
         codeloc);

  بستگی->cif  = cif;
  بستگی->user_داده = user_داده;
  بستگی->fun  = fun;

  أرجع FFI_OK;
}



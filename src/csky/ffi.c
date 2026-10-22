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
فراغ ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
 سجل بنانشانی صحيح i;
 سجل فراغ **p_argv;
 سجل حرف *argp;
 سجل ffi_type **p_arg;

 argp = ڈھیر;

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
#جے_تعریف_شدہ __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->ناپ > 8) && (alignment == 8)) {
   alignment = 4;
  }
#جے_انت

  إذا ((alignment - 1) & (بنانشانی) argp) {
   argp = (حرف *) FFI_ALIGN(argp, alignment);
  }

  إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (حرف *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->ناپ;
  إذا (z < حجم(صحيح))
  {
   z = حجم(صحيح);
   بدل ((*p_arg)->type)
   {
   حالة FFI_TYPE_SINT8:
    *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT8:
    *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_SINT16:
    *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT16:
    *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_STRUCT:
#جے_تعریف_شدہ __CSKYBE__
    حافظہ_نقل((argp + 4 - (*p_arg)->ناپ), *p_argv, (*p_arg)->ناپ);
#وإلا
    حافظہ_نقل(argp, *p_argv, (*p_arg)->ناپ);
#جے_انت
    اكسر;

   افتراضي:
    FFI_ASSERT(0);
   }
  }
  وإلا إذا (z == حجم(صحيح))
  {
   *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
  }
  وإلا
  {
   حافظہ_نقل(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 أرجع;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->بائٹاں = (cif->بائٹاں + 7) & ~7;

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {

    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (بنانشانی) FFI_TYPE_SINT64;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->ناپ <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (بنانشانی)FFI_TYPE_INT;
      وإلا إذا (cif->rtype->ناپ <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (بنانشانی)FFI_TYPE_SINT64;
      وإلا
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (بنانشانی)FFI_TYPE_STRUCT;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
        بنانشانی صحيح nfixedargs,
        بنانشانی صحيح ntotalargs)
{
  أرجع ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
خارجي فراغ ffi_call_SYSV (فراغ (*fn)(فراغ), extended_cif *, بنانشانی, بنانشانی, بنانشانی *);

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  صحيح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  بنانشانی صحيح عارضی;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  إذا ((rvalue == عدم) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ناپ);
    }
  وإلا إذا (small_struct)
    ecif.rvalue = &عارضی;
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->بائٹاں, cif->flags, ecif.rvalue);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
  إذا (small_struct)
#جے_تعریف_شدہ __CSKYBE__
    حافظہ_نقل (rvalue, ((بنانشانی حرف *)&عارضی + (4 - cif->rtype->ناپ)), cif->rtype->ناپ);
#وإلا
    حافظہ_نقل (rvalue, &عارضی, cif->rtype->ناپ);
#جے_انت
}

/** private members **/

ساكن فراغ ffi_prep_incoming_دلیلاں_SYSV (حرف *ڈھیر, فراغ **ret,
      فراغ** دلیلاں, ffi_cif* cif);

فراغ ffi_گھیرا_SYSV (ffi_گھیرا *);

/* This function is jumped to by the trampoline */

بنانشانی صحيح
ffi_گھیرا_SYSV_inner (گھیرا, respp, دلیلاں)
     ffi_گھیرا *گھیرا;
     فراغ **respp;
     فراغ *دلیلاں;
{
  // our various things...
  ffi_cif       *cif;
  فراغ         **arg_area;

  cif         = گھیرا->cif;
  arg_area    = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_دلیلاں_SYSV(دلیلاں, respp, arg_area, cif);

  (گھیرا->fun) (cif, *respp, arg_area, گھیرا->user_انکڑے);

#جے_تعریف_شدہ __CSKYBE__
  إذا (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      بنانشانی صحيح tmp = 0;
      tmp = *(بنانشانی صحيح *)(*respp);
      *(بنانشانی صحيح *)(*respp) = (tmp >> ((4 - cif->rtype->ناپ) * 8));
  }
#جے_انت

  أرجع cif->flags;
}


ساكن فراغ
ffi_prep_incoming_دلیلاں_SYSV(حرف *ڈھیر, فراغ **rvalue,
       فراغ **avalue, ffi_cif *cif)
{
  سجل بنانشانی صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = ڈھیر;

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

#جے_تعریف_شدہ __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->ناپ > 8) && (alignment == 8)) {
        alignment = 4;
      }
#جے_انت

      /* Align if necessary */
      إذا ((alignment - 1) & (بنانشانی) argp) {
 argp = (حرف *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->ناپ;

#جے_تعریف_شدہ __CSKYBE__
      بنانشانی صحيح tmp = 0;
      إذا ((*p_arg)->ناپ < 4) {
        tmp = *(بنانشانی صحيح *)argp;
        حافظہ_نقل(argp, ((بنانشانی حرف *)&tmp + (4 - (*p_arg)->ناپ)), (*p_arg)->ناپ);
      }
#وإلا
      /* because we're little endian, this is what it turns into.   */
#جے_انت
      *p_argv = (فراغ*) argp;

      p_argv++;
      argp += z;
    }

  أرجع;
}

/* How to make a trampoline.  */

خارجي بنانشانی حرف ffi_csky_trampoline[TRAMPOLINE_ناپ];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#تعریف_کرو CACHEFLUSH_وچ_FFI 1
#إذا CACHEFLUSH_وچ_FFI
خارجي فراغ ffi_csky_cacheflush(بنانشانی حرف *__tramp, بنانشانی صحيح k,
  صحيح i);
#تعریف_کرو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بنانشانی حرف *__tramp = (بنانشانی حرف*)(TRAMP);                    \
   بنانشانی صحيح  __fun = (بنانشانی صحيح)(FUN);                           \
   بنانشانی صحيح  __ctx = (بنانشانی صحيح)(CTX);                           \
   بنانشانی حرف *insns = (بنانشانی حرف *)(CTX);                       \
   حافظہ_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_ناپ);              \
   *(بنانشانی صحيح*) &__tramp[TRAMPOLINE_ناپ] = __ctx;                  \
   *(بنانشانی صحيح*) &__tramp[TRAMPOLINE_ناپ + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_ناپ, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_ناپ, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#وإلا
#تعریف_کرو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بنانشانی حرف *__tramp = (بنانشانی حرف*)(TRAMP);                    \
   بنانشانی صحيح  __fun = (بنانشانی صحيح)(FUN);                           \
   بنانشانی صحيح  __ctx = (بنانشانی صحيح)(CTX);                           \
   بنانشانی حرف *insns = (بنانشانی حرف *)(CTX);                       \
   حافظہ_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_ناپ);              \
   *(بنانشانی صحيح*) &__tramp[TRAMPOLINE_ناپ] = __ctx;                  \
   *(بنانشانی صحيح*) &__tramp[TRAMPOLINE_ناپ + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_ناپ-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_ناپ);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#جے_انت

/* the cif must already be prep'ed */

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
        ffi_cif* cif,
        فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
        فراغ *user_انکڑے,
        فراغ *codeloc)
{
  فراغ (*گھیرا_func)(ffi_گھیرا*) = عدم;

  إذا (cif->abi == FFI_SYSV)
    گھیرا_func = &ffi_گھیرا_SYSV;
  وإلا
    أرجع FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&گھیرا->tramp[0], \
         گھیرا_func,  \
         codeloc);

  گھیرا->cif  = cif;
  گھیرا->user_انکڑے = user_انکڑے;
  گھیرا->fun  = fun;

  أرجع FFI_OK;
}



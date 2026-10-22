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
فراغ ffi_prep_دليلون(حرف *ڍير, extended_cif *ecif)
{
 سجل بنانشان صحيح i;
 سجل فراغ **p_argv;
 سجل حرف *argp;
 سجل ffi_type **p_arg;

 argp = ڍير;

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
#جيڪڏهن_وصف_ٿيل __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->سائيز > 8) && (alignment == 8)) {
   alignment = 4;
  }
#جيڪڏهن_پورو

  إذا ((alignment - 1) & (بنانشان) argp) {
   argp = (حرف *) FFI_ALIGN(argp, alignment);
  }

  إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (حرف *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->سائيز;
  إذا (z < حجم(صحيح))
  {
   z = حجم(صحيح);
   بدل ((*p_arg)->type)
   {
   حالة FFI_TYPE_SINT8:
    *(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT8:
    *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT8 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_SINT16:
    *(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_UINT16:
    *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT16 *)(* p_argv);
    اكسر;

   حالة FFI_TYPE_STRUCT:
#جيڪڏهن_وصف_ٿيل __CSKYBE__
    يادگيري_نقل((argp + 4 - (*p_arg)->سائيز), *p_argv, (*p_arg)->سائيز);
#وإلا
    يادگيري_نقل(argp, *p_argv, (*p_arg)->سائيز);
#جيڪڏهن_پورو
    اكسر;

   افتراضي:
    FFI_ASSERT(0);
   }
  }
  وإلا إذا (z == حجم(صحيح))
  {
   *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT32 *)(* p_argv);
  }
  وإلا
  {
   يادگيري_نقل(argp, *p_argv, z);
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
  cif->بائيٽون = (cif->بائيٽون + 7) & ~7;

  /* Set the return type flag */
  بدل (cif->rtype->type)
    {

    حالة FFI_TYPE_DOUBLE:
    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
      cif->flags = (بنانشان) FFI_TYPE_SINT64;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->سائيز <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (بنانشان)FFI_TYPE_INT;
      وإلا إذا (cif->rtype->سائيز <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (بنانشان)FFI_TYPE_SINT64;
      وإلا
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (بنانشان)FFI_TYPE_STRUCT;
      اكسر;

    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
        بنانشان صحيح nfixedargs,
        بنانشان صحيح ntotalargs)
{
  أرجع ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
خارجي فراغ ffi_call_SYSV (فراغ (*fn)(فراغ), extended_cif *, بنانشان, بنانشان, بنانشان *);

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;

  صحيح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  بنانشان صحيح عارضي;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  إذا ((rvalue == عدم) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائيز);
    }
  وإلا إذا (small_struct)
    ecif.rvalue = &عارضي;
  وإلا
    ecif.rvalue = rvalue;

  بدل (cif->abi)
    {
    حالة FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->بائيٽون, cif->flags, ecif.rvalue);
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
  إذا (small_struct)
#جيڪڏهن_وصف_ٿيل __CSKYBE__
    يادگيري_نقل (rvalue, ((بنانشان حرف *)&عارضي + (4 - cif->rtype->سائيز)), cif->rtype->سائيز);
#وإلا
    يادگيري_نقل (rvalue, &عارضي, cif->rtype->سائيز);
#جيڪڏهن_پورو
}

/** private members **/

ساكن فراغ ffi_prep_incoming_دليلون_SYSV (حرف *ڍير, فراغ **ret,
      فراغ** دليلون, ffi_cif* cif);

فراغ ffi_گهيرو_SYSV (ffi_گهيرو *);

/* This function is jumped to by the trampoline */

بنانشان صحيح
ffi_گهيرو_SYSV_inner (گهيرو, respp, دليلون)
     ffi_گهيرو *گهيرو;
     فراغ **respp;
     فراغ *دليلون;
{
  // our various things...
  ffi_cif       *cif;
  فراغ         **arg_area;

  cif         = گهيرو->cif;
  arg_area    = (فراغ**) alloca (cif->nargs * حجم (فراغ*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_دليلون_SYSV(دليلون, respp, arg_area, cif);

  (گهيرو->fun) (cif, *respp, arg_area, گهيرو->user_معلومات);

#جيڪڏهن_وصف_ٿيل __CSKYBE__
  إذا (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      بنانشان صحيح tmp = 0;
      tmp = *(بنانشان صحيح *)(*respp);
      *(بنانشان صحيح *)(*respp) = (tmp >> ((4 - cif->rtype->سائيز) * 8));
  }
#جيڪڏهن_پورو

  أرجع cif->flags;
}


ساكن فراغ
ffi_prep_incoming_دليلون_SYSV(حرف *ڍير, فراغ **rvalue,
       فراغ **avalue, ffi_cif *cif)
{
  سجل بنانشان صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;

  argp = ڍير;

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

#جيڪڏهن_وصف_ٿيل __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      إذا (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->سائيز > 8) && (alignment == 8)) {
        alignment = 4;
      }
#جيڪڏهن_پورو

      /* Align if necessary */
      إذا ((alignment - 1) & (بنانشان) argp) {
 argp = (حرف *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->سائيز;

#جيڪڏهن_وصف_ٿيل __CSKYBE__
      بنانشان صحيح tmp = 0;
      إذا ((*p_arg)->سائيز < 4) {
        tmp = *(بنانشان صحيح *)argp;
        يادگيري_نقل(argp, ((بنانشان حرف *)&tmp + (4 - (*p_arg)->سائيز)), (*p_arg)->سائيز);
      }
#وإلا
      /* because we're little endian, this is what it turns into.   */
#جيڪڏهن_پورو
      *p_argv = (فراغ*) argp;

      p_argv++;
      argp += z;
    }

  أرجع;
}

/* How to make a trampoline.  */

خارجي بنانشان حرف ffi_csky_trampoline[TRAMPOLINE_سائيز];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#وصف_ڪريو CACHEFLUSH_اندريون_FFI 1
#إذا CACHEFLUSH_اندريون_FFI
خارجي فراغ ffi_csky_cacheflush(بنانشان حرف *__tramp, بنانشان صحيح k,
  صحيح i);
#وصف_ڪريو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بنانشان حرف *__tramp = (بنانشان حرف*)(TRAMP);                    \
   بنانشان صحيح  __fun = (بنانشان صحيح)(FUN);                           \
   بنانشان صحيح  __ctx = (بنانشان صحيح)(CTX);                           \
   بنانشان حرف *insns = (بنانشان حرف *)(CTX);                       \
   يادگيري_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_سائيز);              \
   *(بنانشان صحيح*) &__tramp[TRAMPOLINE_سائيز] = __ctx;                  \
   *(بنانشان صحيح*) &__tramp[TRAMPOLINE_سائيز + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_سائيز, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_سائيز, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#وإلا
#وصف_ڪريو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ بنانشان حرف *__tramp = (بنانشان حرف*)(TRAMP);                    \
   بنانشان صحيح  __fun = (بنانشان صحيح)(FUN);                           \
   بنانشان صحيح  __ctx = (بنانشان صحيح)(CTX);                           \
   بنانشان حرف *insns = (بنانشان حرف *)(CTX);                       \
   يادگيري_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_سائيز);              \
   *(بنانشان صحيح*) &__tramp[TRAMPOLINE_سائيز] = __ctx;                  \
   *(بنانشان صحيح*) &__tramp[TRAMPOLINE_سائيز + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_سائيز-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_سائيز);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#جيڪڏهن_پورو

/* the cif must already be prep'ed */

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
        ffi_cif* cif,
        فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
        فراغ *user_معلومات,
        فراغ *codeloc)
{
  فراغ (*گهيرو_func)(ffi_گهيرو*) = عدم;

  إذا (cif->abi == FFI_SYSV)
    گهيرو_func = &ffi_گهيرو_SYSV;
  وإلا
    أرجع FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&گهيرو->tramp[0], \
         گهيرو_func,  \
         codeloc);

  گهيرو->cif  = cif;
  گهيرو->user_معلومات = user_معلومات;
  گهيرو->fun  = fun;

  أرجع FFI_OK;
}



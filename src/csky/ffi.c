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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
خالی ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
 اندراج غیرعلامتی صحیح i;
 اندراج خالی **p_argv;
 اندراج حرف *argp;
 اندراج ffi_type **p_arg;

 argp = ڈھیر;

 اگر ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(خالی **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  size_t z;
  size_t alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#اگر_تعریف_شدہ __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  اگر (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->سائز > 8) && (alignment == 8)) {
   alignment = 4;
  }
#اگر_ختم

  اگر ((alignment - 1) & (غیرعلامتی) argp) {
   argp = (حرف *) FFI_ALIGN(argp, alignment);
  }

  اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (حرف *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->سائز;
  اگر (z < حجم(صحیح))
  {
   z = حجم(صحیح);
   انتخاب ((*p_arg)->type)
   {
   حالت FFI_TYPE_SINT8:
    *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
    توڑو;

   حالت FFI_TYPE_UINT8:
    *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
    توڑو;

   حالت FFI_TYPE_SINT16:
    *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
    توڑو;

   حالت FFI_TYPE_UINT16:
    *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
    توڑو;

   حالت FFI_TYPE_STRUCT:
#اگر_تعریف_شدہ __CSKYBE__
    حافظہ_نقل((argp + 4 - (*p_arg)->سائز), *p_argv, (*p_arg)->سائز);
#ورنہ
    حافظہ_نقل(argp, *p_argv, (*p_arg)->سائز);
#اگر_ختم
    توڑو;

   طےشدہ:
    FFI_ASSERT(0);
   }
  }
  ورنہ اگر (z == حجم(صحیح))
  {
   *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
  }
  ورنہ
  {
   حافظہ_نقل(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 واپس;
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->بائٹیں = (cif->بائٹیں + 7) & ~7;

  /* Set the return type flag */
  انتخاب (cif->rtype->type)
    {

    حالت FFI_TYPE_DOUBLE:
    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
      cif->flags = (غیرعلامتی) FFI_TYPE_SINT64;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->سائز <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (غیرعلامتی)FFI_TYPE_INT;
      ورنہ اگر (cif->rtype->سائز <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (غیرعلامتی)FFI_TYPE_SINT64;
      ورنہ
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (غیرعلامتی)FFI_TYPE_STRUCT;
      توڑو;

    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  واپس FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
        غیرعلامتی صحیح nfixedargs,
        غیرعلامتی صحیح ntotalargs)
{
  واپس ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
خارجی خالی ffi_call_SYSV (خالی (*fn)(خالی), extended_cif *, غیرعلامتی, غیرعلامتی, غیرعلامتی *);

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;

  صحیح small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  غیرعلامتی صحیح عارضی;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  اگر ((rvalue == تہی) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ اگر (small_struct)
    ecif.rvalue = &عارضی;
  ورنہ
    ecif.rvalue = rvalue;

  انتخاب (cif->abi)
    {
    حالت FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->بائٹیں, cif->flags, ecif.rvalue);
      توڑو;

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
  اگر (small_struct)
#اگر_تعریف_شدہ __CSKYBE__
    حافظہ_نقل (rvalue, ((غیرعلامتی حرف *)&عارضی + (4 - cif->rtype->سائز)), cif->rtype->سائز);
#ورنہ
    حافظہ_نقل (rvalue, &عارضی, cif->rtype->سائز);
#اگر_ختم
}

/** private members **/

ساکن خالی ffi_prep_incoming_دلائل_SYSV (حرف *ڈھیر, خالی **ret,
      خالی** دلائل, ffi_cif* cif);

خالی ffi_احاطہ_SYSV (ffi_احاطہ *);

/* This function is jumped to by the trampoline */

غیرعلامتی صحیح
ffi_احاطہ_SYSV_inner (احاطہ, respp, دلائل)
     ffi_احاطہ *احاطہ;
     خالی **respp;
     خالی *دلائل;
{
  // our various things...
  ffi_cif       *cif;
  خالی         **arg_area;

  cif         = احاطہ->cif;
  arg_area    = (خالی**) alloca (cif->nargs * حجم (خالی*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_دلائل_SYSV(دلائل, respp, arg_area, cif);

  (احاطہ->fun) (cif, *respp, arg_area, احاطہ->user_ڈیٹا);

#اگر_تعریف_شدہ __CSKYBE__
  اگر (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      غیرعلامتی صحیح tmp = 0;
      tmp = *(غیرعلامتی صحیح *)(*respp);
      *(غیرعلامتی صحیح *)(*respp) = (tmp >> ((4 - cif->rtype->سائز) * 8));
  }
#اگر_ختم

  واپس cif->flags;
}


ساکن خالی
ffi_prep_incoming_دلائل_SYSV(حرف *ڈھیر, خالی **rvalue,
       خالی **avalue, ffi_cif *cif)
{
  اندراج غیرعلامتی صحیح i;
  اندراج خالی **p_argv;
  اندراج حرف *argp;
  اندراج ffi_type **p_arg;

  argp = ڈھیر;

  اگر ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(خالی **) argp;
    argp += 4;
  }

  p_argv = avalue;

  برائے (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      اگر (alignment < 4)
 alignment = 4;

#اگر_تعریف_شدہ __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      اگر (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->سائز > 8) && (alignment == 8)) {
        alignment = 4;
      }
#اگر_ختم

      /* Align if necessary */
      اگر ((alignment - 1) & (غیرعلامتی) argp) {
 argp = (حرف *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->سائز;

#اگر_تعریف_شدہ __CSKYBE__
      غیرعلامتی صحیح tmp = 0;
      اگر ((*p_arg)->سائز < 4) {
        tmp = *(غیرعلامتی صحیح *)argp;
        حافظہ_نقل(argp, ((غیرعلامتی حرف *)&tmp + (4 - (*p_arg)->سائز)), (*p_arg)->سائز);
      }
#ورنہ
      /* because we're little endian, this is what it turns into.   */
#اگر_ختم
      *p_argv = (خالی*) argp;

      p_argv++;
      argp += z;
    }

  واپس;
}

/* How to make a trampoline.  */

خارجی غیرعلامتی حرف ffi_csky_trampoline[TRAMPOLINE_سائز];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#تعریف_کرو CACHEFLUSH_اندر_FFI 1
#اگر CACHEFLUSH_اندر_FFI
خارجی خالی ffi_csky_cacheflush(غیرعلامتی حرف *__tramp, غیرعلامتی صحیح k,
  صحیح i);
#تعریف_کرو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ غیرعلامتی حرف *__tramp = (غیرعلامتی حرف*)(TRAMP);                    \
   غیرعلامتی صحیح  __fun = (غیرعلامتی صحیح)(FUN);                           \
   غیرعلامتی صحیح  __ctx = (غیرعلامتی صحیح)(CTX);                           \
   غیرعلامتی حرف *insns = (غیرعلامتی حرف *)(CTX);                       \
   حافظہ_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_سائز);              \
   *(غیرعلامتی صحیح*) &__tramp[TRAMPOLINE_سائز] = __ctx;                  \
   *(غیرعلامتی صحیح*) &__tramp[TRAMPOLINE_سائز + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_سائز, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_سائز, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#ورنہ
#تعریف_کرو FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ غیرعلامتی حرف *__tramp = (غیرعلامتی حرف*)(TRAMP);                    \
   غیرعلامتی صحیح  __fun = (غیرعلامتی صحیح)(FUN);                           \
   غیرعلامتی صحیح  __ctx = (غیرعلامتی صحیح)(CTX);                           \
   غیرعلامتی حرف *insns = (غیرعلامتی حرف *)(CTX);                       \
   حافظہ_نقل (__tramp, ffi_csky_trampoline, TRAMPOLINE_سائز);              \
   *(غیرعلامتی صحیح*) &__tramp[TRAMPOLINE_سائز] = __ctx;                  \
   *(غیرعلامتی صحیح*) &__tramp[TRAMPOLINE_سائز + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_سائز-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_سائز);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#اگر_ختم

/* the cif must already be prep'ed */

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
        ffi_cif* cif,
        خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
        خالی *user_ڈیٹا,
        خالی *codeloc)
{
  خالی (*احاطہ_func)(ffi_احاطہ*) = تہی;

  اگر (cif->abi == FFI_SYSV)
    احاطہ_func = &ffi_احاطہ_SYSV;
  ورنہ
    واپس FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&احاطہ->tramp[0], \
         احاطہ_func,  \
         codeloc);

  احاطہ->cif  = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun  = fun;

  واپس FFI_OK;
}



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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments
*/
శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
 నమోదు గుర్తులేని పూర్ణాంకం i;
 నమోదు శూన్య **p_argv;
 నమోదు అక్షరం *argp;
 నమోదు ffi_type **p_arg;

 argp = దొంతర;

 ఒకవేళ ( ecif->cif->flags == FFI_TYPE_STRUCT ) {
  *(శూన్య **) argp = ecif->rvalue;
  argp += 4;
 }

 p_argv = ecif->avalue;

 కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
   (i != 0);
   i--, p_arg++)
 {
  size_t z;
  size_t alignment;

  /* Align if necessary */
  alignment = (*p_arg)->alignment;
#నిర్వచితమైతే __CSKYABIV1__
  /*
   * Adapt ABIV1 bug.
   * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
   */
  ఒకవేళ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->కొలత > 8) && (alignment == 8)) {
   alignment = 4;
  }
#అయితే_ముగింపు

  ఒకవేళ ((alignment - 1) & (గుర్తులేని) argp) {
   argp = (అక్షరం *) FFI_ALIGN(argp, alignment);
  }

  ఒకవేళ ((*p_arg)->type == FFI_TYPE_STRUCT)
   argp = (అక్షరం *) FFI_ALIGN(argp, 4);

  z = (*p_arg)->కొలత;
  ఒకవేళ (z < పరిమాణం(పూర్ణాంకం))
  {
   z = పరిమాణం(పూర్ణాంకం);
   ఎంపిక ((*p_arg)->type)
   {
   స్థితి FFI_TYPE_SINT8:
    *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
    ఆపు;

   స్థితి FFI_TYPE_UINT8:
    *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
    ఆపు;

   స్థితి FFI_TYPE_SINT16:
    *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
    ఆపు;

   స్థితి FFI_TYPE_UINT16:
    *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
    ఆపు;

   స్థితి FFI_TYPE_STRUCT:
#నిర్వచితమైతే __CSKYBE__
    జ్ఞాపకం_నకలు((argp + 4 - (*p_arg)->కొలత), *p_argv, (*p_arg)->కొలత);
#లేకపోతే
    జ్ఞాపకం_నకలు(argp, *p_argv, (*p_arg)->కొలత);
#అయితే_ముగింపు
    ఆపు;

   అప్రమేయ:
    FFI_ASSERT(0);
   }
  }
  లేకపోతే ఒకవేళ (z == పరిమాణం(పూర్ణాంకం))
  {
   *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
  }
  లేకపోతే
  {
   జ్ఞాపకం_నకలు(argp, *p_argv, z);
  }
  p_argv++;
  argp += z;
 }

 తిరిగి;
}

/* Perform machine dependent cif processing */
ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Round the stack up to a multiple of 8 bytes.  This isn't needed
     everywhere, but it is on some platforms, and it doesn't hcsky anything
     when it isn't needed.  */
  cif->బైట్లు = (cif->బైట్లు + 7) & ~7;

  /* Set the return type flag */
  ఎంపిక (cif->rtype->type)
    {

    స్థితి FFI_TYPE_DOUBLE:
    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
      cif->flags = (గుర్తులేని) FFI_TYPE_SINT64;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ (cif->rtype->కొలత <= 4)
 /* A Composite Type not larger than 4 bytes is returned in r0.  */
 cif->flags = (గుర్తులేని)FFI_TYPE_INT;
      లేకపోతే ఒకవేళ (cif->rtype->కొలత <= 8)
 /* A Composite Type not larger than 8 bytes is returned in r0, r1.  */
 cif->flags = (గుర్తులేని)FFI_TYPE_SINT64;
      లేకపోతే
 /* A Composite Type larger than 8 bytes, or whose size cannot
    be determined statically ... is stored in memory at an
    address passed [in r0].  */
 cif->flags = (గుర్తులేని)FFI_TYPE_STRUCT;
      ఆపు;

    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

/* Perform machine dependent cif processing for variadic calls */
ffi_స్థితి ffi_prep_cif_machdep_var(ffi_cif *cif,
        గుర్తులేని పూర్ణాంకం nfixedargs,
        గుర్తులేని పూర్ణాంకం ntotalargs)
{
  తిరిగి ffi_prep_cif_machdep(cif);
}

/* Prototypes for assembly functions, in sysv.S */
బాహ్య శూన్య ffi_call_SYSV (శూన్య (*fn)(శూన్య), extended_cif *, గుర్తులేని, గుర్తులేని, గుర్తులేని *);

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;

  పూర్ణాంకం small_struct = (cif->flags == FFI_TYPE_INT
        && cif->rtype->type == FFI_TYPE_STRUCT);

  ecif.cif = cif;
  ecif.avalue = avalue;

  గుర్తులేని పూర్ణాంకం తాత్కాలిక;

  /* If the return value is a struct and we don't have a return */
  /* value address then we need to make one          */

  ఒకవేళ ((rvalue == ఖాళీ) &&
      (cif->flags == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->కొలత);
    }
  లేకపోతే ఒకవేళ (small_struct)
    ecif.rvalue = &తాత్కాలిక;
  లేకపోతే
    ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV (fn, &ecif, cif->బైట్లు, cif->flags, ecif.rvalue);
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
  ఒకవేళ (small_struct)
#నిర్వచితమైతే __CSKYBE__
    జ్ఞాపకం_నకలు (rvalue, ((గుర్తులేని అక్షరం *)&తాత్కాలిక + (4 - cif->rtype->కొలత)), cif->rtype->కొలత);
#లేకపోతే
    జ్ఞాపకం_నకలు (rvalue, &తాత్కాలిక, cif->rtype->కొలత);
#అయితే_ముగింపు
}

/** private members **/

స్థిర శూన్య ffi_prep_incoming_వాదనలు_SYSV (అక్షరం *దొంతర, శూన్య **ret,
      శూన్య** వాదనలు, ffi_cif* cif);

శూన్య ffi_సంవృతం_SYSV (ffi_సంవృతం *);

/* This function is jumped to by the trampoline */

గుర్తులేని పూర్ణాంకం
ffi_సంవృతం_SYSV_inner (సంవృతం, respp, వాదనలు)
     ffi_సంవృతం *సంవృతం;
     శూన్య **respp;
     శూన్య *వాదనలు;
{
  // our various things...
  ffi_cif       *cif;
  శూన్య         **arg_area;

  cif         = సంవృతం->cif;
  arg_area    = (శూన్య**) alloca (cif->nargs * పరిమాణం (శూన్య*));

  /* this call will initialize ARG_AREA, such that each
   * element in that array points to the corresponding
   * value on the stack; and if the function returns
   * a structure, it will re-set RESP to point to the
   * structure return address.  */

  ffi_prep_incoming_వాదనలు_SYSV(వాదనలు, respp, arg_area, cif);

  (సంవృతం->fun) (cif, *respp, arg_area, సంవృతం->user_దత్తాంశం);

#నిర్వచితమైతే __CSKYBE__
  ఒకవేళ (cif->flags == FFI_TYPE_INT && cif->rtype->type == FFI_TYPE_STRUCT) {
      గుర్తులేని పూర్ణాంకం tmp = 0;
      tmp = *(గుర్తులేని పూర్ణాంకం *)(*respp);
      *(గుర్తులేని పూర్ణాంకం *)(*respp) = (tmp >> ((4 - cif->rtype->కొలత) * 8));
  }
#అయితే_ముగింపు

  తిరిగి cif->flags;
}


స్థిర శూన్య
ffi_prep_incoming_వాదనలు_SYSV(అక్షరం *దొంతర, శూన్య **rvalue,
       శూన్య **avalue, ffi_cif *cif)
{
  నమోదు గుర్తులేని పూర్ణాంకం i;
  నమోదు శూన్య **p_argv;
  నమోదు అక్షరం *argp;
  నమోదు ffi_type **p_arg;

  argp = దొంతర;

  ఒకవేళ ( cif->flags == FFI_TYPE_STRUCT ) {
    *rvalue = *(శూన్య **) argp;
    argp += 4;
  }

  p_argv = avalue;

  కొరకు (i = cif->nargs, p_arg = cif->arg_types; (i != 0); i--, p_arg++)
    {
      size_t z;
      size_t alignment;

      alignment = (*p_arg)->alignment;
      ఒకవేళ (alignment < 4)
 alignment = 4;

#నిర్వచితమైతే __CSKYABIV1__
      /*
       * Adapt ABIV1 bug.
       * If struct's size is larger than 8 bytes, then it always alignment as 4 bytes.
       */
      ఒకవేళ (((*p_arg)->type == FFI_TYPE_STRUCT) && ((*p_arg)->కొలత > 8) && (alignment == 8)) {
        alignment = 4;
      }
#అయితే_ముగింపు

      /* Align if necessary */
      ఒకవేళ ((alignment - 1) & (గుర్తులేని) argp) {
 argp = (అక్షరం *) FFI_ALIGN(argp, alignment);
      }

      z = (*p_arg)->కొలత;

#నిర్వచితమైతే __CSKYBE__
      గుర్తులేని పూర్ణాంకం tmp = 0;
      ఒకవేళ ((*p_arg)->కొలత < 4) {
        tmp = *(గుర్తులేని పూర్ణాంకం *)argp;
        జ్ఞాపకం_నకలు(argp, ((గుర్తులేని అక్షరం *)&tmp + (4 - (*p_arg)->కొలత)), (*p_arg)->కొలత);
      }
#లేకపోతే
      /* because we're little endian, this is what it turns into.   */
#అయితే_ముగింపు
      *p_argv = (శూన్య*) argp;

      p_argv++;
      argp += z;
    }

  తిరిగి;
}

/* How to make a trampoline.  */

బాహ్య గుర్తులేని అక్షరం ffi_csky_trampoline[TRAMPOLINE_కొలత];

/*
 * Since there is no __clear_cache in libgcc in csky toolchain.
 * define ffi_csky_cacheflush in sysv.S.
 * void ffi_csky_cacheflush(uint32 start_addr, uint32 size, int cache)
 */
#నిర్వచించు CACHEFLUSH_లోపల_FFI 1
#ఒకవేళ CACHEFLUSH_లోపల_FFI
బాహ్య శూన్య ffi_csky_cacheflush(గుర్తులేని అక్షరం *__tramp, గుర్తులేని పూర్ణాంకం k,
  పూర్ణాంకం i);
#నిర్వచించు FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ గుర్తులేని అక్షరం *__tramp = (గుర్తులేని అక్షరం*)(TRAMP);                    \
   గుర్తులేని పూర్ణాంకం  __fun = (గుర్తులేని పూర్ణాంకం)(FUN);                           \
   గుర్తులేని పూర్ణాంకం  __ctx = (గుర్తులేని పూర్ణాంకం)(CTX);                           \
   గుర్తులేని అక్షరం *insns = (గుర్తులేని అక్షరం *)(CTX);                       \
   జ్ఞాపకం_నకలు (__tramp, ffi_csky_trampoline, TRAMPOLINE_కొలత);              \
   *(గుర్తులేని పూర్ణాంకం*) &__tramp[TRAMPOLINE_కొలత] = __ctx;                  \
   *(గుర్తులేని పూర్ణాంకం*) &__tramp[TRAMPOLINE_కొలత + 4] = __fun;              \
   ffi_csky_cacheflush(&__tramp[0], TRAMPOLINE_కొలత, 3); /* Clear data mapping.  */ \
   ffi_csky_cacheflush(insns, TRAMPOLINE_కొలత, 3);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#లేకపోతే
#నిర్వచించు FFI_INIT_TRAMPOLINE(TRAMP,FUN,CTX)                              \
({ గుర్తులేని అక్షరం *__tramp = (గుర్తులేని అక్షరం*)(TRAMP);                    \
   గుర్తులేని పూర్ణాంకం  __fun = (గుర్తులేని పూర్ణాంకం)(FUN);                           \
   గుర్తులేని పూర్ణాంకం  __ctx = (గుర్తులేని పూర్ణాంకం)(CTX);                           \
   గుర్తులేని అక్షరం *insns = (గుర్తులేని అక్షరం *)(CTX);                       \
   జ్ఞాపకం_నకలు (__tramp, ffi_csky_trampoline, TRAMPOLINE_కొలత);              \
   *(గుర్తులేని పూర్ణాంకం*) &__tramp[TRAMPOLINE_కొలత] = __ctx;                  \
   *(గుర్తులేని పూర్ణాంకం*) &__tramp[TRAMPOLINE_కొలత + 4] = __fun;              \
   __clear_cache((&__tramp[0]), (&__tramp[TRAMPOLINE_కొలత-1])); /* Clear data mapping.  */ \
   __clear_cache(insns, insns + TRAMPOLINE_కొలత);                       \
                                                 /* Clear instruction   \
                                                    mapping.  */        \
 })
#అయితే_ముగింపు

/* the cif must already be prep'ed */

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
        ffi_cif* cif,
        శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
        శూన్య *user_దత్తాంశం,
        శూన్య *codeloc)
{
  శూన్య (*సంవృతం_func)(ffi_సంవృతం*) = ఖాళీ;

  ఒకవేళ (cif->abi == FFI_SYSV)
    సంవృతం_func = &ffi_సంవృతం_SYSV;
  లేకపోతే
    తిరిగి FFI_BAD_ABI;

  FFI_INIT_TRAMPOLINE (&సంవృతం->tramp[0], \
         సంవృతం_func,  \
         codeloc);

  సంవృతం->cif  = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun  = fun;

  తిరిగి FFI_OK;
}



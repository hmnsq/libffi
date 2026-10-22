/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2014 Sebastian Macke <sebastian@macke.de>

   OpenRISC Foreign Function Interface

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
#అన్తర్భావ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

శూన్య* ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
  అక్షరం *stacktemp = దొంతర;
  పూర్ణాంకం i, s;
  ffi_type **arg;
  పూర్ణాంకం లెక్క = 0;
  పూర్ణాంకం nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  శూన్య **argv = ecif->avalue;

  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(శూన్య **) దొంతర = ecif->rvalue;
      దొంతర += 4;
      లెక్క = 4;
    }
  కొరకు(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    ఒకవేళ ((nfixedargs == 0) && (లెక్క < 24))
      {
        లెక్క = 24;
        దొంతర = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    ఎంపిక((*arg)->type)
      {
      స్థితి FFI_TYPE_STRUCT:
        *(శూన్య **)దొంతర = *argv;
        ఆపు;

      స్థితి FFI_TYPE_SINT8:
        *(గుర్తుగల పూర్ణాంకం *) దొంతర = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* argv);
        ఆపు;

      స్థితి FFI_TYPE_UINT8:
        *(గుర్తులేని పూర్ణాంకం *) దొంతర = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* argv);
        ఆపు;

      స్థితి FFI_TYPE_SINT16:
        *(గుర్తుగల పూర్ణాంకం *) దొంతర = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* argv);
        ఆపు;

      స్థితి FFI_TYPE_UINT16:
        *(గుర్తులేని పూర్ణాంకం *) దొంతర = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* argv);
        ఆపు;

      స్థితి FFI_TYPE_SINT32:
      స్థితి FFI_TYPE_UINT32:
      స్థితి FFI_TYPE_FLOAT:
      స్థితి FFI_TYPE_సూచిక:
        *(పూర్ణాంకం *)దొంతర = *(పూర్ణాంకం*)(*argv);
        ఆపు;

      అప్రమేయ: /* 8 byte types */
        ఒకవేళ (లెక్క == 20) /* never split arguments */
          {
            దొంతర += 4;
            లెక్క += 4;
          }
        s = (*arg)->కొలత;
        జ్ఞాపకం_నకలు(దొంతర, *argv, s);
        ఆపు;
      }

    దొంతర += s;
    లెక్క += s;
    argv++;
    arg++;
  }
  తిరిగి stacktemp + ((లెక్క>24)?24:0);
}

బాహ్య శూన్య ffi_call_SYSV(గుర్తులేని,
                          extended_cif *,
                          శూన్య *(*)(అక్షరం *, extended_cif *),
                          గుర్తులేని *,
                          శూన్య (*fn)(శూన్య),
                          గుర్తులేని);


శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  పూర్ణాంకం i;
  పూర్ణాంకం కొలత;
  ffi_type **arg;
  శూన్య **avalue_copy = ఖాళీ;

  /* Calculate size to allocate on stack */

  కొరకు(i = 0, arg = cif->arg_types, కొలత=0; i < cif->nargs; i++, arg++)
    {
      ఒకవేళ ((*arg)->type == FFI_TYPE_STRUCT)
        కొలత += 4;
      లేకపోతే
      ఒకవేళ ((*arg)->కొలత <= 4)
        కొలత += 4;
      లేకపోతే
        కొలత += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        పూర్ణాంకం కొలత = at->కొలత;
        ఒకవేళ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            అక్షరం *argcopy = alloca (కొలత);
            ఒకవేళ (avalue_copy == ఖాళీ)
              {
                avalue_copy = alloca (cif->nargs * పరిమాణం (శూన్య *));
                జ్ఞాపకం_నకలు (avalue_copy, avalue, cif->nargs * పరిమాణం (శూన్య *));
                avalue = avalue_copy;
              }
            జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  ఒకవేళ (cif->nargs != cif->nfixedargs)
    కొలత += 24;

  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
    కొలత += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  ఎంపిక (cif->abi)
  {
    స్థితి FFI_SYSV:
      ffi_call_SYSV(కొలత, &ecif, ffi_prep_వాదనలు, rvalue, fn, cif->flags);
      ఆపు;
    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
  }
}


శూన్య ffi_సంవృతం_SYSV(గుర్తులేని దీర్ఘ r3, గుర్తులేని దీర్ఘ r4, గుర్తులేని దీర్ఘ r5,
                      గుర్తులేని దీర్ఘ r6, గుర్తులేని దీర్ఘ r7, గుర్తులేని దీర్ఘ r8)
{
  నమోదు పూర్ణాంకం *sp __asm__ ("r17");
  నమోదు పూర్ణాంకం *r13 __asm__ ("r13");

  ffi_సంవృతం* సంవృతం = (ffi_సంవృతం*) r13;
  అక్షరం *దొంతర_వాదనలు = (అక్షరం*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  గుర్తులేని రిజిస్టరు_వాదనలు[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  శూన్య *struct_rvalue = (శూన్య *) r3;

  ffi_cif *cif = సంవృతం->cif;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue = alloca (cif->nargs * పరిమాణం(శూన్య *));
  అక్షరం *ptr = (అక్షరం *) రిజిస్టరు_వాదనలు;
  పూర్ణాంకం లెక్క = 0;
  పూర్ణాంకం nfixedargs = cif->nfixedargs;
  పూర్ణాంకం i;

  /* preserve struct type return pointer passing */

  ఒకవేళ ((cif->rtype != ఖాళీ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    లెక్క = 4;
  }

  /* Find the address of each argument.  */
  కొరకు (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      ఒకవేళ ((nfixedargs == 0) && (లెక్క < 24))
        {
          ptr = దొంతర_వాదనలు;
          లెక్క = 24;
        }
      nfixedargs--;

      ఎంపిక (arg_types[i]->type)
        {
        స్థితి FFI_TYPE_SINT8:
        స్థితి FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          ఆపు;

        స్థితి FFI_TYPE_SINT16:
        స్థితి FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          ఆపు;

        స్థితి FFI_TYPE_SINT32:
        స్థితి FFI_TYPE_UINT32:
        స్థితి FFI_TYPE_FLOAT:
        స్థితి FFI_TYPE_సూచిక:
          avalue[i] = ptr;
          ఆపు;

        స్థితి FFI_TYPE_STRUCT:
          avalue[i] = *(శూన్య**)ptr;
          ఆపు;

        అప్రమేయ:
          /* 8-byte values  */

          /* arguments are never splitted */
          ఒకవేళ (ptr == &రిజిస్టరు_వాదనలు[5])
            ptr = దొంతర_వాదనలు;
          avalue[i] = ptr;
          ptr += 4;
          లెక్క += 4;
          ఆపు;
        }
      ptr += 4;
      లెక్క += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      ఒకవేళ (లెక్క == 24)
        ptr = దొంతర_వాదనలు;
    }

  ఒకవేళ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (సంవృతం->fun) (cif, struct_rvalue, avalue, సంవృతం->user_దత్తాంశం);
    } లేకపోతే
    {
      దీర్ఘ దీర్ఘ rvalue;
      (సంవృతం->fun) (cif, &rvalue, avalue, సంవృతం->user_దత్తాంశం);
      ఒకవేళ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం* సంవృతం,
                      ffi_cif* cif,
                      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
                      శూన్య *user_దత్తాంశం,
                      శూన్య *codeloc)
{
  గుర్తులేని పొట్టి *tramp = (గుర్తులేని పొట్టి *) సంవృతం->tramp;
  గుర్తులేని దీర్ఘ fn = (గుర్తులేని దీర్ఘ) ffi_సంవృతం_SYSV;
  గుర్తులేని దీర్ఘ cls = (గుర్తులేని దీర్ఘ) codeloc;

  ఒకవేళ (cif->abi != FFI_SYSV)
    తిరిగి FFI_BAD_ABI;

  సంవృతం->cif = cif;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;
  సంవృతం->fun = fun;

  /* write pointers to temporary registers */
  tramp[0] = (0x6 << 10) | (13 << 5); /* l.movhi r13, ... */
  tramp[1] = cls >> 16;
  tramp[2] = (0x2a << 10) | (13 << 5) | 13; /* l.ori r13, r13, ... */
  tramp[3] = cls & 0xFFFF;

  tramp[4] = (0x6 << 10) | (15 << 5); /* l.movhi r15, ... */
  tramp[5] = fn >> 16;
  tramp[6] = (0x2a << 10) | (15 << 5) | 15; /* l.ori r15, r15 ... */
  tramp[7] = fn & 0xFFFF;

  tramp[8] = (0x11 << 10); /* l.jr r15 */
  tramp[9] = 15 << 11;

  tramp[10] = (0x2a << 10) | (17 << 5) | 1; /* l.ori r17, r1, ... */
  tramp[11] = 0x0;

  తిరిగి FFI_OK;
}


ffi_స్థితి ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  లేకపోతే
  ఒకవేళ (cif->rtype->కొలత > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  తిరిగి FFI_OK;
}


ffi_స్థితి ffi_prep_cif_machdep_var(ffi_cif *cif,
         గుర్తులేని పూర్ణాంకం nfixedargs, గుర్తులేని పూర్ణాంకం ntotalargs)
{
  ffi_స్థితి status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  తిరిగి status;
}

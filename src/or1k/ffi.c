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

#ضمن <ffi.h>
#ضمن "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ* ffi_prep_آرگومانها(حرف *پشته, extended_cif *ecif)
{
  حرف *stacktemp = پشته;
  صحيح i, s;
  ffi_type **arg;
  صحيح شمار = 0;
  صحيح nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  فراغ **argv = ecif->avalue;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(فراغ **) پشته = ecif->rvalue;
      پشته += 4;
      شمار = 4;
    }
  لكل(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    إذا ((nfixedargs == 0) && (شمار < 24))
      {
        شمار = 24;
        پشته = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    بدل((*arg)->type)
      {
      حالة FFI_TYPE_STRUCT:
        *(فراغ **)پشته = *argv;
        اكسر;

      حالة FFI_TYPE_SINT8:
        *(علامتدار صحيح *) پشته = (علامتدار صحيح)*(SINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT8:
        *(بیعلامت صحيح *) پشته = (بیعلامت صحيح)*(UINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT16:
        *(علامتدار صحيح *) پشته = (علامتدار صحيح)*(SINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT16:
        *(بیعلامت صحيح *) پشته = (بیعلامت صحيح)*(UINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_اشاره_گر:
        *(صحيح *)پشته = *(صحيح*)(*argv);
        اكسر;

      افتراضي: /* 8 byte types */
        إذا (شمار == 20) /* never split arguments */
          {
            پشته += 4;
            شمار += 4;
          }
        s = (*arg)->اندازه;
        کپی_حافظه(پشته, *argv, s);
        اكسر;
      }

    پشته += s;
    شمار += s;
    argv++;
    arg++;
  }
  أرجع stacktemp + ((شمار>24)?24:0);
}

خارجي فراغ ffi_call_SYSV(بیعلامت,
                          extended_cif *,
                          فراغ *(*)(حرف *, extended_cif *),
                          بیعلامت *,
                          فراغ (*fn)(فراغ),
                          بیعلامت);


فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  صحيح اندازه;
  ffi_type **arg;
  فراغ **avalue_copy = عدم;

  /* Calculate size to allocate on stack */

  لكل(i = 0, arg = cif->arg_types, اندازه=0; i < cif->nargs; i++, arg++)
    {
      إذا ((*arg)->type == FFI_TYPE_STRUCT)
        اندازه += 4;
      وإلا
      إذا ((*arg)->اندازه <= 4)
        اندازه += 4;
      وإلا
        اندازه += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        صحيح اندازه = at->اندازه;
        إذا (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            حرف *argcopy = alloca (اندازه);
            إذا (avalue_copy == عدم)
              {
                avalue_copy = alloca (cif->nargs * حجم (فراغ *));
                کپی_حافظه (avalue_copy, avalue, cif->nargs * حجم (فراغ *));
                avalue = avalue_copy;
              }
            کپی_حافظه (argcopy, avalue[i], اندازه);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  إذا (cif->nargs != cif->nfixedargs)
    اندازه += 24;

  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    اندازه += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  بدل (cif->abi)
  {
    حالة FFI_SYSV:
      ffi_call_SYSV(اندازه, &ecif, ffi_prep_آرگومانها, rvalue, fn, cif->flags);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
  }
}


فراغ ffi_بستگی_SYSV(بیعلامت دراز r3, بیعلامت دراز r4, بیعلامت دراز r5,
                      بیعلامت دراز r6, بیعلامت دراز r7, بیعلامت دراز r8)
{
  سجل صحيح *sp __asm__ ("r17");
  سجل صحيح *r13 __asm__ ("r13");

  ffi_بستگی* بستگی = (ffi_بستگی*) r13;
  حرف *پشته_آرگومانها = (حرف*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بیعلامت ثبات_آرگومانها[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) r3;

  ffi_cif *cif = بستگی->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) ثبات_آرگومانها;
  صحيح شمار = 0;
  صحيح nfixedargs = cif->nfixedargs;
  صحيح i;

  /* preserve struct type return pointer passing */

  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    شمار = 4;
  }

  /* Find the address of each argument.  */
  لكل (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      إذا ((nfixedargs == 0) && (شمار < 24))
        {
          ptr = پشته_آرگومانها;
          شمار = 24;
        }
      nfixedargs--;

      بدل (arg_types[i]->type)
        {
        حالة FFI_TYPE_SINT8:
        حالة FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          اكسر;

        حالة FFI_TYPE_SINT16:
        حالة FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          اكسر;

        حالة FFI_TYPE_SINT32:
        حالة FFI_TYPE_UINT32:
        حالة FFI_TYPE_FLOAT:
        حالة FFI_TYPE_اشاره_گر:
          avalue[i] = ptr;
          اكسر;

        حالة FFI_TYPE_STRUCT:
          avalue[i] = *(فراغ**)ptr;
          اكسر;

        افتراضي:
          /* 8-byte values  */

          /* arguments are never splitted */
          إذا (ptr == &ثبات_آرگومانها[5])
            ptr = پشته_آرگومانها;
          avalue[i] = ptr;
          ptr += 4;
          شمار += 4;
          اكسر;
        }
      ptr += 4;
      شمار += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      إذا (شمار == 24)
        ptr = پشته_آرگومانها;
    }

  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (بستگی->fun) (cif, struct_rvalue, avalue, بستگی->user_داده);
    } وإلا
    {
      دراز دراز rvalue;
      (بستگی->fun) (cif, &rvalue, avalue, بستگی->user_داده);
      إذا (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_داده,
                      فراغ *codeloc)
{
  بیعلامت کوتاه *tramp = (بیعلامت کوتاه *) بستگی->tramp;
  بیعلامت دراز fn = (بیعلامت دراز) ffi_بستگی_SYSV;
  بیعلامت دراز cls = (بیعلامت دراز) codeloc;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  بستگی->cif = cif;
  بستگی->user_داده = user_داده;
  بستگی->fun = fun;

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

  أرجع FFI_OK;
}


ffi_وضعیت ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
  إذا (cif->rtype->اندازه > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  أرجع FFI_OK;
}


ffi_وضعیت ffi_prep_cif_machdep_var(ffi_cif *cif,
         بیعلامت صحيح nfixedargs, بیعلامت صحيح ntotalargs)
{
  ffi_وضعیت status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  أرجع status;
}

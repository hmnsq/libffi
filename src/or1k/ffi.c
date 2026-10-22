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

فراغ* ffi_prep_دليلون(حرف *ڍير, extended_cif *ecif)
{
  حرف *stacktemp = ڍير;
  صحيح i, s;
  ffi_type **arg;
  صحيح ڳڻپ = 0;
  صحيح nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  فراغ **argv = ecif->avalue;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(فراغ **) ڍير = ecif->rvalue;
      ڍير += 4;
      ڳڻپ = 4;
    }
  لكل(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    إذا ((nfixedargs == 0) && (ڳڻپ < 24))
      {
        ڳڻپ = 24;
        ڍير = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    بدل((*arg)->type)
      {
      حالة FFI_TYPE_STRUCT:
        *(فراغ **)ڍير = *argv;
        اكسر;

      حالة FFI_TYPE_SINT8:
        *(نشانوارو صحيح *) ڍير = (نشانوارو صحيح)*(SINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT8:
        *(بنانشان صحيح *) ڍير = (بنانشان صحيح)*(UINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT16:
        *(نشانوارو صحيح *) ڍير = (نشانوارو صحيح)*(SINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT16:
        *(بنانشان صحيح *) ڍير = (بنانشان صحيح)*(UINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_اشارو:
        *(صحيح *)ڍير = *(صحيح*)(*argv);
        اكسر;

      افتراضي: /* 8 byte types */
        إذا (ڳڻپ == 20) /* never split arguments */
          {
            ڍير += 4;
            ڳڻپ += 4;
          }
        s = (*arg)->سائيز;
        يادگيري_نقل(ڍير, *argv, s);
        اكسر;
      }

    ڍير += s;
    ڳڻپ += s;
    argv++;
    arg++;
  }
  أرجع stacktemp + ((ڳڻپ>24)?24:0);
}

خارجي فراغ ffi_call_SYSV(بنانشان,
                          extended_cif *,
                          فراغ *(*)(حرف *, extended_cif *),
                          بنانشان *,
                          فراغ (*fn)(فراغ),
                          بنانشان);


فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  صحيح سائيز;
  ffi_type **arg;
  فراغ **avalue_copy = عدم;

  /* Calculate size to allocate on stack */

  لكل(i = 0, arg = cif->arg_types, سائيز=0; i < cif->nargs; i++, arg++)
    {
      إذا ((*arg)->type == FFI_TYPE_STRUCT)
        سائيز += 4;
      وإلا
      إذا ((*arg)->سائيز <= 4)
        سائيز += 4;
      وإلا
        سائيز += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        صحيح سائيز = at->سائيز;
        إذا (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            حرف *argcopy = alloca (سائيز);
            إذا (avalue_copy == عدم)
              {
                avalue_copy = alloca (cif->nargs * حجم (فراغ *));
                يادگيري_نقل (avalue_copy, avalue, cif->nargs * حجم (فراغ *));
                avalue = avalue_copy;
              }
            يادگيري_نقل (argcopy, avalue[i], سائيز);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  إذا (cif->nargs != cif->nfixedargs)
    سائيز += 24;

  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    سائيز += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  بدل (cif->abi)
  {
    حالة FFI_SYSV:
      ffi_call_SYSV(سائيز, &ecif, ffi_prep_دليلون, rvalue, fn, cif->flags);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
  }
}


فراغ ffi_گهيرو_SYSV(بنانشان ڊگهو r3, بنانشان ڊگهو r4, بنانشان ڊگهو r5,
                      بنانشان ڊگهو r6, بنانشان ڊگهو r7, بنانشان ڊگهو r8)
{
  سجل صحيح *sp __asm__ ("r17");
  سجل صحيح *r13 __asm__ ("r13");

  ffi_گهيرو* گهيرو = (ffi_گهيرو*) r13;
  حرف *ڍير_دليلون = (حرف*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بنانشان رجسٽر_دليلون[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) r3;

  ffi_cif *cif = گهيرو->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) رجسٽر_دليلون;
  صحيح ڳڻپ = 0;
  صحيح nfixedargs = cif->nfixedargs;
  صحيح i;

  /* preserve struct type return pointer passing */

  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ڳڻپ = 4;
  }

  /* Find the address of each argument.  */
  لكل (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      إذا ((nfixedargs == 0) && (ڳڻپ < 24))
        {
          ptr = ڍير_دليلون;
          ڳڻپ = 24;
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
        حالة FFI_TYPE_اشارو:
          avalue[i] = ptr;
          اكسر;

        حالة FFI_TYPE_STRUCT:
          avalue[i] = *(فراغ**)ptr;
          اكسر;

        افتراضي:
          /* 8-byte values  */

          /* arguments are never splitted */
          إذا (ptr == &رجسٽر_دليلون[5])
            ptr = ڍير_دليلون;
          avalue[i] = ptr;
          ptr += 4;
          ڳڻپ += 4;
          اكسر;
        }
      ptr += 4;
      ڳڻپ += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      إذا (ڳڻپ == 24)
        ptr = ڍير_دليلون;
    }

  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (گهيرو->fun) (cif, struct_rvalue, avalue, گهيرو->user_معلومات);
    } وإلا
    {
      ڊگهو ڊگهو rvalue;
      (گهيرو->fun) (cif, &rvalue, avalue, گهيرو->user_معلومات);
      إذا (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_معلومات,
                      فراغ *codeloc)
{
  بنانشان ننڍو *tramp = (بنانشان ننڍو *) گهيرو->tramp;
  بنانشان ڊگهو fn = (بنانشان ڊگهو) ffi_گهيرو_SYSV;
  بنانشان ڊگهو cls = (بنانشان ڊگهو) codeloc;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  گهيرو->cif = cif;
  گهيرو->user_معلومات = user_معلومات;
  گهيرو->fun = fun;

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


ffi_حالت ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
  إذا (cif->rtype->سائيز > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  أرجع FFI_OK;
}


ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
         بنانشان صحيح nfixedargs, بنانشان صحيح ntotalargs)
{
  ffi_حالت status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  أرجع status;
}

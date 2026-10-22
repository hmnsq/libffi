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

#شامل <ffi.h>
#شامل "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

خالی* ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  حرف *stacktemp = ڈھیر;
  صحیح i, s;
  ffi_type **arg;
  صحیح گنتی = 0;
  صحیح nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  خالی **argv = ecif->avalue;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(خالی **) ڈھیر = ecif->rvalue;
      ڈھیر += 4;
      گنتی = 4;
    }
  برائے(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    اگر ((nfixedargs == 0) && (گنتی < 24))
      {
        گنتی = 24;
        ڈھیر = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    انتخاب((*arg)->type)
      {
      حالت FFI_TYPE_STRUCT:
        *(خالی **)ڈھیر = *argv;
        توڑو;

      حالت FFI_TYPE_SINT8:
        *(علامتی صحیح *) ڈھیر = (علامتی صحیح)*(SINT8 *)(* argv);
        توڑو;

      حالت FFI_TYPE_UINT8:
        *(غیرعلامتی صحیح *) ڈھیر = (غیرعلامتی صحیح)*(UINT8 *)(* argv);
        توڑو;

      حالت FFI_TYPE_SINT16:
        *(علامتی صحیح *) ڈھیر = (علامتی صحیح)*(SINT16 *)(* argv);
        توڑو;

      حالت FFI_TYPE_UINT16:
        *(غیرعلامتی صحیح *) ڈھیر = (غیرعلامتی صحیح)*(UINT16 *)(* argv);
        توڑو;

      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_اشارہ_گر:
        *(صحیح *)ڈھیر = *(صحیح*)(*argv);
        توڑو;

      طےشدہ: /* 8 byte types */
        اگر (گنتی == 20) /* never split arguments */
          {
            ڈھیر += 4;
            گنتی += 4;
          }
        s = (*arg)->سائز;
        حافظہ_نقل(ڈھیر, *argv, s);
        توڑو;
      }

    ڈھیر += s;
    گنتی += s;
    argv++;
    arg++;
  }
  واپس stacktemp + ((گنتی>24)?24:0);
}

خارجی خالی ffi_call_SYSV(غیرعلامتی,
                          extended_cif *,
                          خالی *(*)(حرف *, extended_cif *),
                          غیرعلامتی *,
                          خالی (*fn)(خالی),
                          غیرعلامتی);


خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  صحیح i;
  صحیح سائز;
  ffi_type **arg;
  خالی **avalue_copy = تہی;

  /* Calculate size to allocate on stack */

  برائے(i = 0, arg = cif->arg_types, سائز=0; i < cif->nargs; i++, arg++)
    {
      اگر ((*arg)->type == FFI_TYPE_STRUCT)
        سائز += 4;
      ورنہ
      اگر ((*arg)->سائز <= 4)
        سائز += 4;
      ورنہ
        سائز += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        صحیح سائز = at->سائز;
        اگر (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            حرف *argcopy = alloca (سائز);
            اگر (avalue_copy == تہی)
              {
                avalue_copy = alloca (cif->nargs * حجم (خالی *));
                حافظہ_نقل (avalue_copy, avalue, cif->nargs * حجم (خالی *));
                avalue = avalue_copy;
              }
            حافظہ_نقل (argcopy, avalue[i], سائز);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  اگر (cif->nargs != cif->nfixedargs)
    سائز += 24;

  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    سائز += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  انتخاب (cif->abi)
  {
    حالت FFI_SYSV:
      ffi_call_SYSV(سائز, &ecif, ffi_prep_دلائل, rvalue, fn, cif->flags);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
  }
}


خالی ffi_احاطہ_SYSV(غیرعلامتی طویل r3, غیرعلامتی طویل r4, غیرعلامتی طویل r5,
                      غیرعلامتی طویل r6, غیرعلامتی طویل r7, غیرعلامتی طویل r8)
{
  اندراج صحیح *sp __asm__ ("r17");
  اندراج صحیح *r13 __asm__ ("r13");

  ffi_احاطہ* احاطہ = (ffi_احاطہ*) r13;
  حرف *ڈھیر_دلائل = (حرف*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  غیرعلامتی رجسٹر_دلائل[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  خالی *struct_rvalue = (خالی *) r3;

  ffi_cif *cif = احاطہ->cif;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue = alloca (cif->nargs * حجم(خالی *));
  حرف *ptr = (حرف *) رجسٹر_دلائل;
  صحیح گنتی = 0;
  صحیح nfixedargs = cif->nfixedargs;
  صحیح i;

  /* preserve struct type return pointer passing */

  اگر ((cif->rtype != تہی) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    گنتی = 4;
  }

  /* Find the address of each argument.  */
  برائے (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      اگر ((nfixedargs == 0) && (گنتی < 24))
        {
          ptr = ڈھیر_دلائل;
          گنتی = 24;
        }
      nfixedargs--;

      انتخاب (arg_types[i]->type)
        {
        حالت FFI_TYPE_SINT8:
        حالت FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          توڑو;

        حالت FFI_TYPE_SINT16:
        حالت FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          توڑو;

        حالت FFI_TYPE_SINT32:
        حالت FFI_TYPE_UINT32:
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_اشارہ_گر:
          avalue[i] = ptr;
          توڑو;

        حالت FFI_TYPE_STRUCT:
          avalue[i] = *(خالی**)ptr;
          توڑو;

        طےشدہ:
          /* 8-byte values  */

          /* arguments are never splitted */
          اگر (ptr == &رجسٹر_دلائل[5])
            ptr = ڈھیر_دلائل;
          avalue[i] = ptr;
          ptr += 4;
          گنتی += 4;
          توڑو;
        }
      ptr += 4;
      گنتی += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      اگر (گنتی == 24)
        ptr = ڈھیر_دلائل;
    }

  اگر (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (احاطہ->fun) (cif, struct_rvalue, avalue, احاطہ->user_ڈیٹا);
    } ورنہ
    {
      طویل طویل rvalue;
      (احاطہ->fun) (cif, &rvalue, avalue, احاطہ->user_ڈیٹا);
      اگر (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
                      ffi_cif* cif,
                      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
                      خالی *user_ڈیٹا,
                      خالی *codeloc)
{
  غیرعلامتی مختصر *tramp = (غیرعلامتی مختصر *) احاطہ->tramp;
  غیرعلامتی طویل fn = (غیرعلامتی طویل) ffi_احاطہ_SYSV;
  غیرعلامتی طویل cls = (غیرعلامتی طویل) codeloc;

  اگر (cif->abi != FFI_SYSV)
    واپس FFI_BAD_ABI;

  احاطہ->cif = cif;
  احاطہ->user_ڈیٹا = user_ڈیٹا;
  احاطہ->fun = fun;

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

  واپس FFI_OK;
}


ffi_حالت ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  ورنہ
  اگر (cif->rtype->سائز > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  واپس FFI_OK;
}


ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
         غیرعلامتی صحیح nfixedargs, غیرعلامتی صحیح ntotalargs)
{
  ffi_حالت status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  واپس status;
}

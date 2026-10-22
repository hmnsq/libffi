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

تهی* ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
  نویسه *stacktemp = پشته;
  صحیح i, s;
  ffi_type **arg;
  صحیح شمار = 0;
  صحیح nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  تهی **argv = ecif->avalue;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(تهی **) پشته = ecif->rvalue;
      پشته += 4;
      شمار = 4;
    }
  برای(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    اگر ((nfixedargs == 0) && (شمار < 24))
      {
        شمار = 24;
        پشته = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    انتخاب((*arg)->type)
      {
      حالت FFI_TYPE_STRUCT:
        *(تهی **)پشته = *argv;
        بشکن;

      حالت FFI_TYPE_SINT8:
        *(signed صحیح *) پشته = (signed صحیح)*(SINT8 *)(* argv);
        بشکن;

      حالت FFI_TYPE_UINT8:
        *(unsigned صحیح *) پشته = (unsigned صحیح)*(UINT8 *)(* argv);
        بشکن;

      حالت FFI_TYPE_SINT16:
        *(signed صحیح *) پشته = (signed صحیح)*(SINT16 *)(* argv);
        بشکن;

      حالت FFI_TYPE_UINT16:
        *(unsigned صحیح *) پشته = (unsigned صحیح)*(UINT16 *)(* argv);
        بشکن;

      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_FLOAT:
      حالت FFI_TYPE_POINTER:
        *(صحیح *)پشته = *(صحیح*)(*argv);
        بشکن;

      پیشفرض: /* 8 byte types */
        اگر (شمار == 20) /* never split arguments */
          {
            پشته += 4;
            شمار += 4;
          }
        s = (*arg)->size;
        memcpy(پشته, *argv, s);
        بشکن;
      }

    پشته += s;
    شمار += s;
    argv++;
    arg++;
  }
  بازگشت stacktemp + ((شمار>24)?24:0);
}

بیرونی تهی ffi_call_SYSV(unsigned,
                          extended_cif *,
                          تهی *(*)(نویسه *, extended_cif *),
                          unsigned *,
                          تهی (*fn)(تهی),
                          unsigned);


تهی ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  صحیح i;
  صحیح size;
  ffi_type **arg;
  تهی **avalue_copy = پوچ;

  /* Calculate size to allocate on stack */

  برای(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      اگر ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      وگرنه
      اگر ((*arg)->size <= 4)
        size += 4;
      وگرنه
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        صحیح size = at->size;
        اگر (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            نویسه *argcopy = alloca (size);
            اگر (avalue_copy == پوچ)
              {
                avalue_copy = alloca (cif->nargs * اندازه (تهی *));
                memcpy (avalue_copy, avalue, cif->nargs * اندازه (تهی *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  اگر (cif->nargs != cif->nfixedargs)
    size += 24;

  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  انتخاب (cif->abi)
  {
    حالت FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
  }
}


تهی ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  ثبات صحیح *sp __asm__ ("r17");
  ثبات صحیح *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  نویسه *پشته_args = (نویسه*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  تهی *struct_rvalue = (تهی *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue = alloca (cif->nargs * اندازه(تهی *));
  نویسه *ptr = (نویسه *) register_args;
  صحیح شمار = 0;
  صحیح nfixedargs = cif->nfixedargs;
  صحیح i;

  /* preserve struct type return pointer passing */

  اگر ((cif->rtype != پوچ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    شمار = 4;
  }

  /* Find the address of each argument.  */
  برای (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      اگر ((nfixedargs == 0) && (شمار < 24))
        {
          ptr = پشته_args;
          شمار = 24;
        }
      nfixedargs--;

      انتخاب (arg_types[i]->type)
        {
        حالت FFI_TYPE_SINT8:
        حالت FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          بشکن;

        حالت FFI_TYPE_SINT16:
        حالت FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          بشکن;

        حالت FFI_TYPE_SINT32:
        حالت FFI_TYPE_UINT32:
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_POINTER:
          avalue[i] = ptr;
          بشکن;

        حالت FFI_TYPE_STRUCT:
          avalue[i] = *(تهی**)ptr;
          بشکن;

        پیشفرض:
          /* 8-byte values  */

          /* arguments are never splitted */
          اگر (ptr == &register_args[5])
            ptr = پشته_args;
          avalue[i] = ptr;
          ptr += 4;
          شمار += 4;
          بشکن;
        }
      ptr += 4;
      شمار += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      اگر (شمار == 24)
        ptr = پشته_args;
    }

  اگر (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } وگرنه
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      اگر (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
                      تهی *user_data,
                      تهی *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  closure->cif = cif;
  closure->user_data = user_data;
  closure->fun = fun;

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

  بازگشت FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  وگرنه
  اگر (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  بازگشت FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned صحیح nfixedargs, unsigned صحیح ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  بازگشت status;
}

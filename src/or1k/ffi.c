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

فراغ* ffi_prep_args(حرف *stack, extended_cif *ecif)
{
  حرف *stacktemp = stack;
  صحيح i, s;
  ffi_type **arg;
  صحيح count = 0;
  صحيح nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  فراغ **argv = ecif->avalue;

  𐤀𐤌 (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(فراغ **) stack = ecif->rvalue;
      stack += 4;
      count = 4;
    }
  لكل(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    𐤀𐤌 ((nfixedargs == 0) && (count < 24))
      {
        count = 24;
        stack = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    بدل((*arg)->type)
      {
      حالة FFI_TYPE_STRUCT:
        *(فراغ **)stack = *argv;
        اكسر;

      حالة FFI_TYPE_SINT8:
        *(signed صحيح *) stack = (signed صحيح)*(SINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT8:
        *(unsigned صحيح *) stack = (unsigned صحيح)*(UINT8 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT16:
        *(signed صحيح *) stack = (signed صحيح)*(SINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_UINT16:
        *(unsigned صحيح *) stack = (unsigned صحيح)*(UINT16 *)(* argv);
        اكسر;

      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_FLOAT:
      حالة FFI_TYPE_POINTER:
        *(صحيح *)stack = *(صحيح*)(*argv);
        اكسر;

      افتراضي: /* 8 byte types */
        𐤀𐤌 (count == 20) /* never split arguments */
          {
            stack += 4;
            count += 4;
          }
        s = (*arg)->size;
        memcpy(stack, *argv, s);
        اكسر;
      }

    stack += s;
    count += s;
    argv++;
    arg++;
  }
  أرجع stacktemp + ((count>24)?24:0);
}

خارجي فراغ ffi_call_SYSV(unsigned,
                          extended_cif *,
                          فراغ *(*)(حرف *, extended_cif *),
                          unsigned *,
                          فراغ (*fn)(فراغ),
                          unsigned);


فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  صحيح i;
  صحيح size;
  ffi_type **arg;
  فراغ **avalue_copy = عدم;

  /* Calculate size to allocate on stack */

  لكل(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      𐤀𐤌 ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      وإلا
      𐤀𐤌 ((*arg)->size <= 4)
        size += 4;
      وإلا
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        صحيح size = at->size;
        𐤀𐤌 (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            حرف *argcopy = alloca (size);
            𐤀𐤌 (avalue_copy == عدم)
              {
                avalue_copy = alloca (cif->nargs * حجم (فراغ *));
                memcpy (avalue_copy, avalue, cif->nargs * حجم (فراغ *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  𐤀𐤌 (cif->nargs != cif->nfixedargs)
    size += 24;

  𐤀𐤌 (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  بدل (cif->abi)
  {
    حالة FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
  }
}


فراغ ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  سجل صحيح *sp __asm__ ("r17");
  سجل صحيح *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  حرف *stack_args = (حرف*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) register_args;
  صحيح count = 0;
  صحيح nfixedargs = cif->nfixedargs;
  صحيح i;

  /* preserve struct type return pointer passing */

  𐤀𐤌 ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    count = 4;
  }

  /* Find the address of each argument.  */
  لكل (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      𐤀𐤌 ((nfixedargs == 0) && (count < 24))
        {
          ptr = stack_args;
          count = 24;
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
        حالة FFI_TYPE_POINTER:
          avalue[i] = ptr;
          اكسر;

        حالة FFI_TYPE_STRUCT:
          avalue[i] = *(فراغ**)ptr;
          اكسر;

        افتراضي:
          /* 8-byte values  */

          /* arguments are never splitted */
          𐤀𐤌 (ptr == &register_args[5])
            ptr = stack_args;
          avalue[i] = ptr;
          ptr += 4;
          count += 4;
          اكسر;
        }
      ptr += 4;
      count += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      𐤀𐤌 (count == 24)
        ptr = stack_args;
    }

  𐤀𐤌 (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } وإلا
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      𐤀𐤌 (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
                      فراغ *user_data,
                      فراغ *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  𐤀𐤌 (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

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

  أرجع FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  𐤀𐤌 (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  وإلا
  𐤀𐤌 (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  أرجع FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned صحيح nfixedargs, unsigned صحيح ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  أرجع status;
}

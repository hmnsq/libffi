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

#include <ffi.h>
#include "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void* ffi_prep_args(var_na *stack, extended_cif *ecif)
{
  var_na *stacktemp = stack;
  පූර්ණ_සංඛ්‍යා i, s;
  ffi_type **arg;
  පූර්ණ_සංඛ්‍යා count = 0;
  පූර්ණ_සංඛ්‍යා nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  void **argv = ecif->avalue;

  නම් (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(void **) stack = ecif->rvalue;
      stack += 4;
      count = 4;
    }
  සඳහා(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    නම් ((nfixedargs == 0) && (count < 24))
      {
        count = 24;
        stack = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    switch((*arg)->type)
      {
      pak_sa FFI_TYPE_STRUCT:
        *(void **)stack = *argv;
        bha_nga;

      pak_sa FFI_TYPE_SINT8:
        *(signed පූර්ණ_සංඛ්‍යා *) stack = (signed පූර්ණ_සංඛ්‍යා)*(SINT8 *)(* argv);
        bha_nga;

      pak_sa FFI_TYPE_UINT8:
        *(unsigned පූර්ණ_සංඛ්‍යා *) stack = (unsigned පූර්ණ_සංඛ්‍යා)*(UINT8 *)(* argv);
        bha_nga;

      pak_sa FFI_TYPE_SINT16:
        *(signed පූර්ණ_සංඛ්‍යා *) stack = (signed පූර්ණ_සංඛ්‍යා)*(SINT16 *)(* argv);
        bha_nga;

      pak_sa FFI_TYPE_UINT16:
        *(unsigned පූර්ණ_සංඛ්‍යා *) stack = (unsigned පූර්ණ_සංඛ්‍යා)*(UINT16 *)(* argv);
        bha_nga;

      pak_sa FFI_TYPE_SINT32:
      pak_sa FFI_TYPE_UINT32:
      pak_sa FFI_TYPE_FLOAT:
      pak_sa FFI_TYPE_POINTER:
        *(පූර්ණ_සංඛ්‍යා *)stack = *(පූර්ණ_සංඛ්‍යා*)(*argv);
        bha_nga;

      default: /* 8 byte types */
        නම් (count == 20) /* never split arguments */
          {
            stack += 4;
            count += 4;
          }
        s = (*arg)->size;
        memcpy(stack, *argv, s);
        bha_nga;
      }

    stack += s;
    count += s;
    argv++;
    arg++;
  }
  ආපසු stacktemp + ((count>24)?24:0);
}

extern void ffi_call_SYSV(unsigned,
                          extended_cif *,
                          void *(*)(var_na *, extended_cif *),
                          unsigned *,
                          void (*fn)(void),
                          unsigned);


void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  පූර්ණ_සංඛ්‍යා i;
  පූර්ණ_සංඛ්‍යා size;
  ffi_type **arg;
  void **avalue_copy = NULL;

  /* Calculate size to allocate on stack */

  සඳහා(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      නම් ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      නැතහොත්
      නම් ((*arg)->size <= 4)
        size += 4;
      නැතහොත්
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        පූර්ණ_සංඛ්‍යා size = at->size;
        නම් (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            var_na *argcopy = alloca (size);
            නම් (avalue_copy == NULL)
              {
                avalue_copy = alloca (cif->nargs * parimaa_na (void *));
                memcpy (avalue_copy, avalue, cif->nargs * parimaa_na (void *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  නම් (cif->nargs != cif->nfixedargs)
    size += 24;

  නම් (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  switch (cif->abi)
  {
    pak_sa FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      bha_nga;
    default:
      FFI_ASSERT(0);
      bha_nga;
  }
}


void ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  pa_njikaa පූර්ණ_සංඛ්‍යා *sp __asm__ ("r17");
  pa_njikaa පූර්ණ_සංඛ්‍යා *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  var_na *stack_args = (var_na*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  void *struct_rvalue = (void *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue = alloca (cif->nargs * parimaa_na(void *));
  var_na *ptr = (var_na *) register_args;
  පූර්ණ_සංඛ්‍යා count = 0;
  පූර්ණ_සංඛ්‍යා nfixedargs = cif->nfixedargs;
  පූර්ණ_සංඛ්‍යා i;

  /* preserve struct type return pointer passing */

  නම් ((cif->rtype != NULL) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    count = 4;
  }

  /* Find the address of each argument.  */
  සඳහා (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      නම් ((nfixedargs == 0) && (count < 24))
        {
          ptr = stack_args;
          count = 24;
        }
      nfixedargs--;

      switch (arg_types[i]->type)
        {
        pak_sa FFI_TYPE_SINT8:
        pak_sa FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          bha_nga;

        pak_sa FFI_TYPE_SINT16:
        pak_sa FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          bha_nga;

        pak_sa FFI_TYPE_SINT32:
        pak_sa FFI_TYPE_UINT32:
        pak_sa FFI_TYPE_FLOAT:
        pak_sa FFI_TYPE_POINTER:
          avalue[i] = ptr;
          bha_nga;

        pak_sa FFI_TYPE_STRUCT:
          avalue[i] = *(void**)ptr;
          bha_nga;

        default:
          /* 8-byte values  */

          /* arguments are never splitted */
          නම් (ptr == &register_args[5])
            ptr = stack_args;
          avalue[i] = ptr;
          ptr += 4;
          count += 4;
          bha_nga;
        }
      ptr += 4;
      count += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      නම් (count == 24)
        ptr = stack_args;
    }

  නම් (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } නැතහොත්
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      නම් (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      void (*fun)(ffi_cif*,void*,void**,void*),
                      void *user_data,
                      void *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  නම් (cif->abi != FFI_SYSV)
    ආපසු FFI_BAD_ABI;

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

  ආපසු FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  නම් (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  නැතහොත්
  නම් (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ආපසු FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned පූර්ණ_සංඛ්‍යා nfixedargs, unsigned පූර්ණ_සංඛ්‍යා ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ආපසු status;
}

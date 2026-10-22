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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ശൂന്യം* ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
  അക്ഷരം *stacktemp = അട്ടി;
  പൂർണ്ണസംഖ്യ i, s;
  ffi_type **arg;
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  ശൂന്യം **argv = ecif->avalue;

  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ശൂന്യം **) അട്ടി = ecif->rvalue;
      അട്ടി += 4;
      എണ്ണം = 4;
    }
  വേണ്ടി(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    എങ്കിൽ ((nfixedargs == 0) && (എണ്ണം < 24))
      {
        എണ്ണം = 24;
        അട്ടി = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    തിരഞ്ഞെടുപ്പ്((*arg)->type)
      {
      അവസ്ഥ FFI_TYPE_STRUCT:
        *(ശൂന്യം **)അട്ടി = *argv;
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_SINT8:
        *(signed പൂർണ്ണസംഖ്യ *) അട്ടി = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* argv);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_UINT8:
        *(unsigned പൂർണ്ണസംഖ്യ *) അട്ടി = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* argv);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_SINT16:
        *(signed പൂർണ്ണസംഖ്യ *) അട്ടി = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* argv);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_UINT16:
        *(unsigned പൂർണ്ണസംഖ്യ *) അട്ടി = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* argv);
        നിർത്തുക;

      അവസ്ഥ FFI_TYPE_SINT32:
      അവസ്ഥ FFI_TYPE_UINT32:
      അവസ്ഥ FFI_TYPE_FLOAT:
      അവസ്ഥ FFI_TYPE_POINTER:
        *(പൂർണ്ണസംഖ്യ *)അട്ടി = *(പൂർണ്ണസംഖ്യ*)(*argv);
        നിർത്തുക;

      സ്ഥിരസ്ഥിതി: /* 8 byte types */
        എങ്കിൽ (എണ്ണം == 20) /* never split arguments */
          {
            അട്ടി += 4;
            എണ്ണം += 4;
          }
        s = (*arg)->size;
        memcpy(അട്ടി, *argv, s);
        നിർത്തുക;
      }

    അട്ടി += s;
    എണ്ണം += s;
    argv++;
    arg++;
  }
  തിരികെ stacktemp + ((എണ്ണം>24)?24:0);
}

ബാഹ്യ ശൂന്യം ffi_call_SYSV(unsigned,
                          extended_cif *,
                          ശൂന്യം *(*)(അക്ഷരം *, extended_cif *),
                          unsigned *,
                          ശൂന്യം (*fn)(ശൂന്യം),
                          unsigned);


ശൂന്യം ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ size;
  ffi_type **arg;
  ശൂന്യം **avalue_copy = ശൂന്യ;

  /* Calculate size to allocate on stack */

  വേണ്ടി(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      എങ്കിൽ ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      അല്ലെങ്കിൽ
      എങ്കിൽ ((*arg)->size <= 4)
        size += 4;
      അല്ലെങ്കിൽ
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        പൂർണ്ണസംഖ്യ size = at->size;
        എങ്കിൽ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            അക്ഷരം *argcopy = alloca (size);
            എങ്കിൽ (avalue_copy == ശൂന്യ)
              {
                avalue_copy = alloca (cif->nargs * വലുപ്പം (ശൂന്യം *));
                memcpy (avalue_copy, avalue, cif->nargs * വലുപ്പം (ശൂന്യം *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  എങ്കിൽ (cif->nargs != cif->nfixedargs)
    size += 24;

  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  തിരഞ്ഞെടുപ്പ് (cif->abi)
  {
    അവസ്ഥ FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
  }
}


ശൂന്യം ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  രജിസ്റ്റർ പൂർണ്ണസംഖ്യ *sp __asm__ ("r17");
  രജിസ്റ്റർ പൂർണ്ണസംഖ്യ *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  അക്ഷരം *അട്ടി_args = (അക്ഷരം*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  ശൂന്യം *struct_rvalue = (ശൂന്യം *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue = alloca (cif->nargs * വലുപ്പം(ശൂന്യം *));
  അക്ഷരം *ptr = (അക്ഷരം *) register_args;
  പൂർണ്ണസംഖ്യ എണ്ണം = 0;
  പൂർണ്ണസംഖ്യ nfixedargs = cif->nfixedargs;
  പൂർണ്ണസംഖ്യ i;

  /* preserve struct type return pointer passing */

  എങ്കിൽ ((cif->rtype != ശൂന്യ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    എണ്ണം = 4;
  }

  /* Find the address of each argument.  */
  വേണ്ടി (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      എങ്കിൽ ((nfixedargs == 0) && (എണ്ണം < 24))
        {
          ptr = അട്ടി_args;
          എണ്ണം = 24;
        }
      nfixedargs--;

      തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
        {
        അവസ്ഥ FFI_TYPE_SINT8:
        അവസ്ഥ FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          നിർത്തുക;

        അവസ്ഥ FFI_TYPE_SINT16:
        അവസ്ഥ FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          നിർത്തുക;

        അവസ്ഥ FFI_TYPE_SINT32:
        അവസ്ഥ FFI_TYPE_UINT32:
        അവസ്ഥ FFI_TYPE_FLOAT:
        അവസ്ഥ FFI_TYPE_POINTER:
          avalue[i] = ptr;
          നിർത്തുക;

        അവസ്ഥ FFI_TYPE_STRUCT:
          avalue[i] = *(ശൂന്യം**)ptr;
          നിർത്തുക;

        സ്ഥിരസ്ഥിതി:
          /* 8-byte values  */

          /* arguments are never splitted */
          എങ്കിൽ (ptr == &register_args[5])
            ptr = അട്ടി_args;
          avalue[i] = ptr;
          ptr += 4;
          എണ്ണം += 4;
          നിർത്തുക;
        }
      ptr += 4;
      എണ്ണം += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      എങ്കിൽ (എണ്ണം == 24)
        ptr = അട്ടി_args;
    }

  എങ്കിൽ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } അല്ലെങ്കിൽ
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      എങ്കിൽ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
                      ശൂന്യം *user_data,
                      ശൂന്യം *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  എങ്കിൽ (cif->abi != FFI_SYSV)
    തിരികെ FFI_BAD_ABI;

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

  തിരികെ FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  അല്ലെങ്കിൽ
  എങ്കിൽ (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  തിരികെ FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned പൂർണ്ണസംഖ്യ nfixedargs, unsigned പൂർണ്ണസംഖ്യ ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  തിരികെ status;
}

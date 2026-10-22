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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ* ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਅੱਖਰ *stacktemp = ਢੇਰ;
  ਪੂਰਨ i, s;
  ffi_type **arg;
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਪੂਰਨ nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  ਖਾਲੀ **argv = ecif->avalue;

  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) ਢੇਰ = ecif->rvalue;
      ਢੇਰ += 4;
      ਗਿਣਤੀ_2 = 4;
    }
  ਲਈ(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    ਜੇ ((nfixedargs == 0) && (ਗਿਣਤੀ_2 < 24))
      {
        ਗਿਣਤੀ_2 = 24;
        ਢੇਰ = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    ਚੋਣ((*arg)->type)
      {
      ਹਾਲਤ FFI_TYPE_STRUCT:
        *(ਖਾਲੀ **)ਢੇਰ = *argv;
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_SINT8:
        *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) ਢੇਰ = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* argv);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_UINT8:
        *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) ਢੇਰ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* argv);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_SINT16:
        *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) ਢੇਰ = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* argv);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_UINT16:
        *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) ਢੇਰ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* argv);
        ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_FLOAT:
      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
        *(ਪੂਰਨ *)ਢੇਰ = *(ਪੂਰਨ*)(*argv);
        ਰੋਕੋ;

      ਮੂਲ: /* 8 byte types */
        ਜੇ (ਗਿਣਤੀ_2 == 20) /* never split arguments */
          {
            ਢੇਰ += 4;
            ਗਿਣਤੀ_2 += 4;
          }
        s = (*arg)->ਮਾਪ;
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਢੇਰ, *argv, s);
        ਰੋਕੋ;
      }

    ਢੇਰ += s;
    ਗਿਣਤੀ_2 += s;
    argv++;
    arg++;
  }
  ਵਾਪਸ stacktemp + ((ਗਿਣਤੀ_2>24)?24:0);
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_SYSV(ਬਿਨਾਨਿਸ਼ਾਨੀ,
                          extended_cif *,
                          ਖਾਲੀ *(*)(ਅੱਖਰ *, extended_cif *),
                          ਬਿਨਾਨਿਸ਼ਾਨੀ *,
                          ਖਾਲੀ (*fn)(ਖਾਲੀ),
                          ਬਿਨਾਨਿਸ਼ਾਨੀ);


ਖਾਲੀ ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ਪੂਰਨ i;
  ਪੂਰਨ ਮਾਪ;
  ffi_type **arg;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;

  /* Calculate size to allocate on stack */

  ਲਈ(i = 0, arg = cif->arg_types, ਮਾਪ=0; i < cif->nargs; i++, arg++)
    {
      ਜੇ ((*arg)->type == FFI_TYPE_STRUCT)
        ਮਾਪ += 4;
      ਨਹੀਂਤਾਂ
      ਜੇ ((*arg)->ਮਾਪ <= 4)
        ਮਾਪ += 4;
      ਨਹੀਂਤਾਂ
        ਮਾਪ += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
        ਜੇ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
            ਜੇ (avalue_copy == ਨਿਰਾ)
              {
                avalue_copy = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));
                ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));
                avalue = avalue_copy;
              }
            ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  ਜੇ (cif->nargs != cif->nfixedargs)
    ਮਾਪ += 24;

  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    ਮਾਪ += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  ਚੋਣ (cif->abi)
  {
    ਹਾਲਤ FFI_SYSV:
      ffi_call_SYSV(ਮਾਪ, &ecif, ffi_prep_ਦਲੀਲਾਂ, rvalue, fn, cif->flags);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
  }
}


ਖਾਲੀ ffi_ਘੇਰਾ_SYSV(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r3, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r4, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r5,
                      ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r6, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r7, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ r8)
{
  ਰਜਿਸਟਰ ਪੂਰਨ *sp __asm__ ("r17");
  ਰਜਿਸਟਰ ਪੂਰਨ *r13 __asm__ ("r13");

  ffi_ਘੇਰਾ* ਘੇਰਾ = (ffi_ਘੇਰਾ*) r13;
  ਅੱਖਰ *ਢੇਰ_ਦਲੀਲਾਂ = (ਅੱਖਰ*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  ਖਾਲੀ *struct_rvalue = (ਖਾਲੀ *) r3;

  ffi_cif *cif = ਘੇਰਾ->cif;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue = alloca (cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  ਅੱਖਰ *ptr = (ਅੱਖਰ *) ਰਜਿਸਟਰ_ਦਲੀਲਾਂ;
  ਪੂਰਨ ਗਿਣਤੀ_2 = 0;
  ਪੂਰਨ nfixedargs = cif->nfixedargs;
  ਪੂਰਨ i;

  /* preserve struct type return pointer passing */

  ਜੇ ((cif->rtype != ਨਿਰਾ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ਗਿਣਤੀ_2 = 4;
  }

  /* Find the address of each argument.  */
  ਲਈ (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      ਜੇ ((nfixedargs == 0) && (ਗਿਣਤੀ_2 < 24))
        {
          ptr = ਢੇਰ_ਦਲੀਲਾਂ;
          ਗਿਣਤੀ_2 = 24;
        }
      nfixedargs--;

      ਚੋਣ (arg_types[i]->type)
        {
        ਹਾਲਤ FFI_TYPE_SINT8:
        ਹਾਲਤ FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_SINT16:
        ਹਾਲਤ FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_SINT32:
        ਹਾਲਤ FFI_TYPE_UINT32:
        ਹਾਲਤ FFI_TYPE_FLOAT:
        ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
          avalue[i] = ptr;
          ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_STRUCT:
          avalue[i] = *(ਖਾਲੀ**)ptr;
          ਰੋਕੋ;

        ਮੂਲ:
          /* 8-byte values  */

          /* arguments are never splitted */
          ਜੇ (ptr == &ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[5])
            ptr = ਢੇਰ_ਦਲੀਲਾਂ;
          avalue[i] = ptr;
          ptr += 4;
          ਗਿਣਤੀ_2 += 4;
          ਰੋਕੋ;
        }
      ptr += 4;
      ਗਿਣਤੀ_2 += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      ਜੇ (ਗਿਣਤੀ_2 == 24)
        ptr = ਢੇਰ_ਦਲੀਲਾਂ;
    }

  ਜੇ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (ਘੇਰਾ->fun) (cif, struct_rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
    } ਨਹੀਂਤਾਂ
    {
      ਲੰਮਾ ਲੰਮਾ rvalue;
      (ਘੇਰਾ->fun) (cif, &rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
      ਜੇ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
                      ffi_cif* cif,
                      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
                      ਖਾਲੀ *user_ਅੰਕੜੇ,
                      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) ਘੇਰਾ->tramp;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_SYSV;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cls = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) codeloc;

  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;
  ਘੇਰਾ->fun = fun;

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

  ਵਾਪਸ FFI_OK;
}


ffi_ਸਥਿਤੀ ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  ਨਹੀਂਤਾਂ
  ਜੇ (cif->rtype->ਮਾਪ > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ਵਾਪਸ FFI_OK;
}


ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_var(ffi_cif *cif,
         ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ nfixedargs, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ntotalargs)
{
  ffi_ਸਥਿਤੀ status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ਵਾਪਸ status;
}

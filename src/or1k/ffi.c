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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ* ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
  ವರ್ಣ *stacktemp = ರಾಶಿ;
  ಪೂರ್ಣಾಙ್ಕ i, s;
  ffi_type **arg;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಙ್ಕ nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  ಶೂನ್ಯ **argv = ecif->avalue;

  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) ರಾಶಿ = ecif->rvalue;
      ರಾಶಿ += 4;
      ಎಣಿಕೆ = 4;
    }
  ಪ್ರತಿ(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    ಯದಿ ((nfixedargs == 0) && (ಎಣಿಕೆ < 24))
      {
        ಎಣಿಕೆ = 24;
        ರಾಶಿ = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    ವಿಕಲ್ಪ((*arg)->type)
      {
      ಪಕ್ಷ FFI_TYPE_STRUCT:
        *(ಶೂನ್ಯ **)ರಾಶಿ = *argv;
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_SINT8:
        *(signed ಪೂರ್ಣಾಙ್ಕ *) ರಾಶಿ = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* argv);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_UINT8:
        *(unsigned ಪೂರ್ಣಾಙ್ಕ *) ರಾಶಿ = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* argv);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_SINT16:
        *(signed ಪೂರ್ಣಾಙ್ಕ *) ರಾಶಿ = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* argv);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_UINT16:
        *(unsigned ಪೂರ್ಣಾಙ್ಕ *) ರಾಶಿ = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* argv);
        ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_FLOAT:
      ಪಕ್ಷ FFI_TYPE_POINTER:
        *(ಪೂರ್ಣಾಙ್ಕ *)ರಾಶಿ = *(ಪೂರ್ಣಾಙ್ಕ*)(*argv);
        ಭಙ್ಗ;

      ಸ್ವಭಾವ: /* 8 byte types */
        ಯದಿ (ಎಣಿಕೆ == 20) /* never split arguments */
          {
            ರಾಶಿ += 4;
            ಎಣಿಕೆ += 4;
          }
        s = (*arg)->size;
        memcpy(ರಾಶಿ, *argv, s);
        ಭಙ್ಗ;
      }

    ರಾಶಿ += s;
    ಎಣಿಕೆ += s;
    argv++;
    arg++;
  }
  ಪ್ರತ್ಯಾಗಮ stacktemp + ((ಎಣಿಕೆ>24)?24:0);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(unsigned,
                          extended_cif *,
                          ಶೂನ್ಯ *(*)(ವರ್ಣ *, extended_cif *),
                          unsigned *,
                          ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
                          unsigned);


ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ size;
  ffi_type **arg;
  ಶೂನ್ಯ **avalue_copy = ರಿಕ್ತ;

  /* Calculate size to allocate on stack */

  ಪ್ರತಿ(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      ಯದಿ ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      ಅನ್ಯಥಾ
      ಯದಿ ((*arg)->size <= 4)
        size += 4;
      ಅನ್ಯಥಾ
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        ಪೂರ್ಣಾಙ್ಕ size = at->size;
        ಯದಿ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            ವರ್ಣ *argcopy = alloca (size);
            ಯದಿ (avalue_copy == ರಿಕ್ತ)
              {
                avalue_copy = alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
                memcpy (avalue_copy, avalue, cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  ಯದಿ (cif->nargs != cif->nfixedargs)
    size += 24;

  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  ವಿಕಲ್ಪ (cif->abi)
  {
    ಪಕ್ಷ FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
  }
}


ಶೂನ್ಯ ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  ಪಞ್ಜಿಕಾ ಪೂರ್ಣಾಙ್ಕ *sp __asm__ ("r17");
  ಪಞ್ಜಿಕಾ ಪೂರ್ಣಾಙ್ಕ *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  ವರ್ಣ *ರಾಶಿ_args = (ವರ್ಣ*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  ಶೂನ್ಯ *struct_rvalue = (ಶೂನ್ಯ *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  ವರ್ಣ *ptr = (ವರ್ಣ *) register_args;
  ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;
  ಪೂರ್ಣಾಙ್ಕ nfixedargs = cif->nfixedargs;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* preserve struct type return pointer passing */

  ಯದಿ ((cif->rtype != ರಿಕ್ತ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ಎಣಿಕೆ = 4;
  }

  /* Find the address of each argument.  */
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      ಯದಿ ((nfixedargs == 0) && (ಎಣಿಕೆ < 24))
        {
          ptr = ರಾಶಿ_args;
          ಎಣಿಕೆ = 24;
        }
      nfixedargs--;

      ವಿಕಲ್ಪ (arg_types[i]->type)
        {
        ಪಕ್ಷ FFI_TYPE_SINT8:
        ಪಕ್ಷ FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          ಭಙ್ಗ;

        ಪಕ್ಷ FFI_TYPE_SINT16:
        ಪಕ್ಷ FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          ಭಙ್ಗ;

        ಪಕ್ಷ FFI_TYPE_SINT32:
        ಪಕ್ಷ FFI_TYPE_UINT32:
        ಪಕ್ಷ FFI_TYPE_FLOAT:
        ಪಕ್ಷ FFI_TYPE_POINTER:
          avalue[i] = ptr;
          ಭಙ್ಗ;

        ಪಕ್ಷ FFI_TYPE_STRUCT:
          avalue[i] = *(ಶೂನ್ಯ**)ptr;
          ಭಙ್ಗ;

        ಸ್ವಭಾವ:
          /* 8-byte values  */

          /* arguments are never splitted */
          ಯದಿ (ptr == &register_args[5])
            ptr = ರಾಶಿ_args;
          avalue[i] = ptr;
          ptr += 4;
          ಎಣಿಕೆ += 4;
          ಭಙ್ಗ;
        }
      ptr += 4;
      ಎಣಿಕೆ += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      ಯದಿ (ಎಣಿಕೆ == 24)
        ptr = ರಾಶಿ_args;
    }

  ಯದಿ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } ಅನ್ಯಥಾ
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      ಯದಿ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_data,
                      ಶೂನ್ಯ *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

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

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  ಅನ್ಯಥಾ
  ಯದಿ (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned ಪೂರ್ಣಾಙ್ಕ nfixedargs, unsigned ಪೂರ್ಣಾಙ್ಕ ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ಪ್ರತ್ಯಾಗಮ status;
}

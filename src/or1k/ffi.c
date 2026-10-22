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

ಶೂನ್ಯ* ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, extended_cif *ecif)
{
  ಅಕ್ಷರ *stacktemp = ರಾಶಿ;
  ಪೂರ್ಣಾಂಕ i, s;
  ffi_type **arg;
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಪೂರ್ಣಾಂಕ nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  ಶೂನ್ಯ **argv = ecif->avalue;

  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) ರಾಶಿ = ecif->rvalue;
      ರಾಶಿ += 4;
      ಎಣಿಕೆ_2 = 4;
    }
  ಗಾಗಿ(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    ಆದರೆ ((nfixedargs == 0) && (ಎಣಿಕೆ_2 < 24))
      {
        ಎಣಿಕೆ_2 = 24;
        ರಾಶಿ = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    ಆಯ್ಕೆ((*arg)->type)
      {
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
        *(ಶೂನ್ಯ **)ರಾಶಿ = *argv;
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
        *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) ರಾಶಿ = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT8 *)(* argv);
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
        *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) ರಾಶಿ = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT8 *)(* argv);
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
        *(ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ *) ರಾಶಿ = (ಚಿಹ್ನೆಯುಳ್ಳ ಪೂರ್ಣಾಂಕ)*(SINT16 *)(* argv);
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
        *(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) ರಾಶಿ = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)*(UINT16 *)(* argv);
        ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
        *(ಪೂರ್ಣಾಂಕ *)ರಾಶಿ = *(ಪೂರ್ಣಾಂಕ*)(*argv);
        ನಿಲ್ಲಿಸು;

      ಪೂರ್ವನಿಯೋಜಿತ: /* 8 byte types */
        ಆದರೆ (ಎಣಿಕೆ_2 == 20) /* never split arguments */
          {
            ರಾಶಿ += 4;
            ಎಣಿಕೆ_2 += 4;
          }
        s = (*arg)->ಗಾತ್ರ_2;
        ಸ್ಮರಣೆ_ನಕಲು(ರಾಶಿ, *argv, s);
        ನಿಲ್ಲಿಸು;
      }

    ರಾಶಿ += s;
    ಎಣಿಕೆ_2 += s;
    argv++;
    arg++;
  }
  ಹಿಂತಿರುಗಿ stacktemp + ((ಎಣಿಕೆ_2>24)?24:0);
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_SYSV(ಚಿಹ್ನೆರಹಿತ,
                          extended_cif *,
                          ಶೂನ್ಯ *(*)(ಅಕ್ಷರ *, extended_cif *),
                          ಚಿಹ್ನೆರಹಿತ *,
                          ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
                          ಚಿಹ್ನೆರಹಿತ);


ಶೂನ್ಯ ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
  ffi_type **arg;
  ಶೂನ್ಯ **avalue_copy = ಖಾಲಿ;

  /* Calculate size to allocate on stack */

  ಗಾಗಿ(i = 0, arg = cif->arg_types, ಗಾತ್ರ_2=0; i < cif->nargs; i++, arg++)
    {
      ಆದರೆ ((*arg)->type == FFI_TYPE_STRUCT)
        ಗಾತ್ರ_2 += 4;
      ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ ((*arg)->ಗಾತ್ರ_2 <= 4)
        ಗಾತ್ರ_2 += 4;
      ಇಲ್ಲದಿದ್ದರೆ
        ಗಾತ್ರ_2 += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = at->ಗಾತ್ರ_2;
        ಆದರೆ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            ಅಕ್ಷರ *argcopy = alloca (ಗಾತ್ರ_2);
            ಆದರೆ (avalue_copy == ಖಾಲಿ)
              {
                avalue_copy = alloca (cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
                ಸ್ಮರಣೆ_ನಕಲು (avalue_copy, avalue, cif->nargs * ಗಾತ್ರ (ಶೂನ್ಯ *));
                avalue = avalue_copy;
              }
            ಸ್ಮರಣೆ_ನಕಲು (argcopy, avalue[i], ಗಾತ್ರ_2);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  ಆದರೆ (cif->nargs != cif->nfixedargs)
    ಗಾತ್ರ_2 += 24;

  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
    ಗಾತ್ರ_2 += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  ಆಯ್ಕೆ (cif->abi)
  {
    ಸ್ಥಿತಿ FFI_SYSV:
      ffi_call_SYSV(ಗಾತ್ರ_2, &ecif, ffi_prep_ವಾದಗಳು, rvalue, fn, cif->flags);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
  }
}


ಶೂನ್ಯ ffi_ಆವರಣ_SYSV(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r3, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r4, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r5,
                      ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r6, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r7, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ r8)
{
  ನೋಂದಣಿ ಪೂರ್ಣಾಂಕ *sp __asm__ ("r17");
  ನೋಂದಣಿ ಪೂರ್ಣಾಂಕ *r13 __asm__ ("r13");

  ffi_ಆವರಣ* ಆವರಣ = (ffi_ಆವರಣ*) r13;
  ಅಕ್ಷರ *ರಾಶಿ_ವಾದಗಳು = (ಅಕ್ಷರ*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ಚಿಹ್ನೆರಹಿತ ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  ಶೂನ್ಯ *struct_rvalue = (ಶೂನ್ಯ *) r3;

  ffi_cif *cif = ಆವರಣ->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಗಾತ್ರ(ಶೂನ್ಯ *));
  ಅಕ್ಷರ *ptr = (ಅಕ್ಷರ *) ರಿಜಿಸ್ಟರ್_ವಾದಗಳು;
  ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2 = 0;
  ಪೂರ್ಣಾಂಕ nfixedargs = cif->nfixedargs;
  ಪೂರ್ಣಾಂಕ i;

  /* preserve struct type return pointer passing */

  ಆದರೆ ((cif->rtype != ಖಾಲಿ) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ಎಣಿಕೆ_2 = 4;
  }

  /* Find the address of each argument.  */
  ಗಾಗಿ (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      ಆದರೆ ((nfixedargs == 0) && (ಎಣಿಕೆ_2 < 24))
        {
          ptr = ರಾಶಿ_ವಾದಗಳು;
          ಎಣಿಕೆ_2 = 24;
        }
      nfixedargs--;

      ಆಯ್ಕೆ (arg_types[i]->type)
        {
        ಸ್ಥಿತಿ FFI_TYPE_SINT8:
        ಸ್ಥಿತಿ FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_SINT16:
        ಸ್ಥಿತಿ FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_SINT32:
        ಸ್ಥಿತಿ FFI_TYPE_UINT32:
        ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
          avalue[i] = ptr;
          ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
          avalue[i] = *(ಶೂನ್ಯ**)ptr;
          ನಿಲ್ಲಿಸು;

        ಪೂರ್ವನಿಯೋಜಿತ:
          /* 8-byte values  */

          /* arguments are never splitted */
          ಆದರೆ (ptr == &ರಿಜಿಸ್ಟರ್_ವಾದಗಳು[5])
            ptr = ರಾಶಿ_ವಾದಗಳು;
          avalue[i] = ptr;
          ptr += 4;
          ಎಣಿಕೆ_2 += 4;
          ನಿಲ್ಲಿಸು;
        }
      ptr += 4;
      ಎಣಿಕೆ_2 += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      ಆದರೆ (ಎಣಿಕೆ_2 == 24)
        ptr = ರಾಶಿ_ವಾದಗಳು;
    }

  ಆದರೆ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (ಆವರಣ->fun) (cif, struct_rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
    } ಇಲ್ಲದಿದ್ದರೆ
    {
      ದೀರ್ಘ ದೀರ್ಘ rvalue;
      (ಆವರಣ->fun) (cif, &rvalue, avalue, ಆವರಣ->user_ದತ್ತಾಂಶ);
      ಆದರೆ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ* ಆವರಣ,
                      ffi_cif* cif,
                      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
                      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
                      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *tramp = (ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ *) ಆವರಣ->tramp;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ fn = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ffi_ಆವರಣ_SYSV;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cls = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) codeloc;

  ಆದರೆ (cif->abi != FFI_SYSV)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;

  ಆವರಣ->cif = cif;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;
  ಆವರಣ->fun = fun;

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

  ಹಿಂತಿರುಗಿ FFI_OK;
}


ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (cif->rtype->ಗಾತ್ರ_2 > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ಹಿಂತಿರುಗಿ FFI_OK;
}


ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep_var(ffi_cif *cif,
         ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ nfixedargs, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ntotalargs)
{
  ffi_ಸ್ಥಿತಿ status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ಹಿಂತಿರುಗಿ status;
}

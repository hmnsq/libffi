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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य* ffi_prep_args(वर्ण *राशिः, extended_cif *ecif)
{
  वर्ण *stacktemp = राशिः;
  पूर्णाङ्क i, s;
  ffi_type **arg;
  पूर्णाङ्क गणनम् = 0;
  पूर्णाङ्क nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  शून्य **argv = ecif->avalue;

  जइ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(शून्य **) राशिः = ecif->rvalue;
      राशिः += 4;
      गणनम् = 4;
    }
  प्रति(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    जइ ((nfixedargs == 0) && (गणनम् < 24))
      {
        गणनम् = 24;
        राशिः = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    विकल्प((*arg)->type)
      {
      पक्ष FFI_TYPE_STRUCT:
        *(शून्य **)राशिः = *argv;
        भङ्ग;

      पक्ष FFI_TYPE_SINT8:
        *(signed पूर्णाङ्क *) राशिः = (signed पूर्णाङ्क)*(SINT8 *)(* argv);
        भङ्ग;

      पक्ष FFI_TYPE_UINT8:
        *(unsigned पूर्णाङ्क *) राशिः = (unsigned पूर्णाङ्क)*(UINT8 *)(* argv);
        भङ्ग;

      पक्ष FFI_TYPE_SINT16:
        *(signed पूर्णाङ्क *) राशिः = (signed पूर्णाङ्क)*(SINT16 *)(* argv);
        भङ्ग;

      पक्ष FFI_TYPE_UINT16:
        *(unsigned पूर्णाङ्क *) राशिः = (unsigned पूर्णाङ्क)*(UINT16 *)(* argv);
        भङ्ग;

      पक्ष FFI_TYPE_SINT32:
      पक्ष FFI_TYPE_UINT32:
      पक्ष FFI_TYPE_FLOAT:
      पक्ष FFI_TYPE_POINTER:
        *(पूर्णाङ्क *)राशिः = *(पूर्णाङ्क*)(*argv);
        भङ्ग;

      स्वभाव: /* 8 byte types */
        जइ (गणनम् == 20) /* never split arguments */
          {
            राशिः += 4;
            गणनम् += 4;
          }
        s = (*arg)->size;
        memcpy(राशिः, *argv, s);
        भङ्ग;
      }

    राशिः += s;
    गणनम् += s;
    argv++;
    arg++;
  }
  प्रत्यागम stacktemp + ((गणनम्>24)?24:0);
}

बाह्य शून्य ffi_call_SYSV(unsigned,
                          extended_cif *,
                          शून्य *(*)(वर्ण *, extended_cif *),
                          unsigned *,
                          शून्य (*fn)(शून्य),
                          unsigned);


शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  पूर्णाङ्क i;
  पूर्णाङ्क size;
  ffi_type **arg;
  शून्य **avalue_copy = रिक्त;

  /* Calculate size to allocate on stack */

  प्रति(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      जइ ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      अण्णहा
      जइ ((*arg)->size <= 4)
        size += 4;
      अण्णहा
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        पूर्णाङ्क size = at->size;
        जइ (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            वर्ण *argcopy = alloca (size);
            जइ (avalue_copy == रिक्त)
              {
                avalue_copy = alloca (cif->nargs * परिमाण (शून्य *));
                memcpy (avalue_copy, avalue, cif->nargs * परिमाण (शून्य *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  जइ (cif->nargs != cif->nfixedargs)
    size += 24;

  जइ (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  विकल्प (cif->abi)
  {
    पक्ष FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
  }
}


शून्य ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  पञ्जिका पूर्णाङ्क *sp __asm__ ("r17");
  पञ्जिका पूर्णाङ्क *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  वर्ण *राशिः_args = (वर्ण*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  शून्य *struct_rvalue = (शून्य *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * परिमाण(शून्य *));
  वर्ण *ptr = (वर्ण *) register_args;
  पूर्णाङ्क गणनम् = 0;
  पूर्णाङ्क nfixedargs = cif->nfixedargs;
  पूर्णाङ्क i;

  /* preserve struct type return pointer passing */

  जइ ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    गणनम् = 4;
  }

  /* Find the address of each argument.  */
  प्रति (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      जइ ((nfixedargs == 0) && (गणनम् < 24))
        {
          ptr = राशिः_args;
          गणनम् = 24;
        }
      nfixedargs--;

      विकल्प (arg_types[i]->type)
        {
        पक्ष FFI_TYPE_SINT8:
        पक्ष FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          भङ्ग;

        पक्ष FFI_TYPE_SINT16:
        पक्ष FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          भङ्ग;

        पक्ष FFI_TYPE_SINT32:
        पक्ष FFI_TYPE_UINT32:
        पक्ष FFI_TYPE_FLOAT:
        पक्ष FFI_TYPE_POINTER:
          avalue[i] = ptr;
          भङ्ग;

        पक्ष FFI_TYPE_STRUCT:
          avalue[i] = *(शून्य**)ptr;
          भङ्ग;

        स्वभाव:
          /* 8-byte values  */

          /* arguments are never splitted */
          जइ (ptr == &register_args[5])
            ptr = राशिः_args;
          avalue[i] = ptr;
          ptr += 4;
          गणनम् += 4;
          भङ्ग;
        }
      ptr += 4;
      गणनम् += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      जइ (गणनम् == 24)
        ptr = राशिः_args;
    }

  जइ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } अण्णहा
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      जइ (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
                      शून्य *user_data,
                      शून्य *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  जइ (cif->abi != FFI_SYSV)
    प्रत्यागम FFI_BAD_ABI;

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

  प्रत्यागम FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  जइ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  अण्णहा
  जइ (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  प्रत्यागम FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned पूर्णाङ्क nfixedargs, unsigned पूर्णाङ्क ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  प्रत्यागम status;
}

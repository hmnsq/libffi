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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ* ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  ଅକ୍ଷର *stacktemp = ଗଦା;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, s;
  ffi_type **arg;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  ଶୂନ୍ୟ **argv = ecif->avalue;

  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) ଗଦା = ecif->rvalue;
      ଗଦା += 4;
      ଗଣନା_2 = 4;
    }
  ପାଇଁ(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    ଯଦି ((nfixedargs == 0) && (ଗଣନା_2 < 24))
      {
        ଗଣନା_2 = 24;
        ଗଦା = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    ବାଛ((*arg)->type)
      {
      ସ୍ଥିତି FFI_TYPE_STRUCT:
        *(ଶୂନ୍ୟ **)ଗଦା = *argv;
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_SINT8:
        *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) ଗଦା = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* argv);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_UINT8:
        *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) ଗଦା = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* argv);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_SINT16:
        *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) ଗଦା = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* argv);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_UINT16:
        *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) ଗଦା = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* argv);
        ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_SINT32:
      ସ୍ଥିତି FFI_TYPE_UINT32:
      ସ୍ଥିତି FFI_TYPE_FLOAT:
      ସ୍ଥିତି FFI_TYPE_POINTER:
        *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)ଗଦା = *(ପୂର୍ଣ୍ଣସଂଖ୍ୟା*)(*argv);
        ଭାଙ୍ଗ;

      ପୂର୍ବନିର୍ଦ୍ଧାରିତ: /* 8 byte types */
        ଯଦି (ଗଣନା_2 == 20) /* never split arguments */
          {
            ଗଦା += 4;
            ଗଣନା_2 += 4;
          }
        s = (*arg)->size;
        memcpy(ଗଦା, *argv, s);
        ଭାଙ୍ଗ;
      }

    ଗଦା += s;
    ଗଣନା_2 += s;
    argv++;
    arg++;
  }
  ଫେରନ୍ତୁ stacktemp + ((ଗଣନା_2>24)?24:0);
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_SYSV(unsigned,
                          extended_cif *,
                          ଶୂନ୍ୟ *(*)(ଅକ୍ଷର *, extended_cif *),
                          unsigned *,
                          ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ),
                          unsigned);


ଶୂନ୍ୟ ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
  ffi_type **arg;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;

  /* Calculate size to allocate on stack */

  ପାଇଁ(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      ଯଦି ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      ଅନ୍ୟଥା
      ଯଦି ((*arg)->size <= 4)
        size += 4;
      ଅନ୍ୟଥା
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
        ଯଦି (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            ଅକ୍ଷର *argcopy = alloca (size);
            ଯଦି (avalue_copy == ଖାଲି)
              {
                avalue_copy = alloca (cif->nargs * ଆକାର (ଶୂନ୍ୟ *));
                memcpy (avalue_copy, avalue, cif->nargs * ଆକାର (ଶୂନ୍ୟ *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  ଯଦି (cif->nargs != cif->nfixedargs)
    size += 24;

  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  ବାଛ (cif->abi)
  {
    ସ୍ଥିତି FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
  }
}


ଶୂନ୍ୟ ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  ପଞ୍ଜିକରଣ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *sp __asm__ ("r17");
  ପଞ୍ଜିକରଣ ପୂର୍ଣ୍ଣସଂଖ୍ୟା *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  ଅକ୍ଷର *ଗଦା_args = (ଅକ୍ଷର*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  ଶୂନ୍ୟ *struct_rvalue = (ଶୂନ୍ୟ *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue = alloca (cif->nargs * ଆକାର(ଶୂନ୍ୟ *));
  ଅକ୍ଷର *ptr = (ଅକ୍ଷର *) register_args;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs = cif->nfixedargs;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* preserve struct type return pointer passing */

  ଯଦି ((cif->rtype != ଖାଲି) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ଗଣନା_2 = 4;
  }

  /* Find the address of each argument.  */
  ପାଇଁ (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      ଯଦି ((nfixedargs == 0) && (ଗଣନା_2 < 24))
        {
          ptr = ଗଦା_args;
          ଗଣନା_2 = 24;
        }
      nfixedargs--;

      ବାଛ (arg_types[i]->type)
        {
        ସ୍ଥିତି FFI_TYPE_SINT8:
        ସ୍ଥିତି FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_SINT16:
        ସ୍ଥିତି FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_SINT32:
        ସ୍ଥିତି FFI_TYPE_UINT32:
        ସ୍ଥିତି FFI_TYPE_FLOAT:
        ସ୍ଥିତି FFI_TYPE_POINTER:
          avalue[i] = ptr;
          ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_STRUCT:
          avalue[i] = *(ଶୂନ୍ୟ**)ptr;
          ଭାଙ୍ଗ;

        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          /* 8-byte values  */

          /* arguments are never splitted */
          ଯଦି (ptr == &register_args[5])
            ptr = ଗଦା_args;
          avalue[i] = ptr;
          ptr += 4;
          ଗଣନା_2 += 4;
          ଭାଙ୍ଗ;
        }
      ptr += 4;
      ଗଣନା_2 += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      ଯଦି (ଗଣନା_2 == 24)
        ptr = ଗଦା_args;
    }

  ଯଦି (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } ଅନ୍ୟଥା
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      ଯଦି (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
                      ଶୂନ୍ୟ *user_data,
                      ଶୂନ୍ୟ *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  ଯଦି (cif->abi != FFI_SYSV)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

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

  ଫେରନ୍ତୁ FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  ଅନ୍ୟଥା
  ଯଦି (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ଫେରନ୍ତୁ FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା nfixedargs, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ଫେରନ୍ତୁ status;
}

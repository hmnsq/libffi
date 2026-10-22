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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

வெறுமை* ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
  எழுத்து *stacktemp = அடுக்கு;
  முழு i, s;
  ffi_type **arg;
  முழு மொத்தம் = 0;
  முழு nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  வெறுமை **argv = ecif->avalue;

  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(வெறுமை **) அடுக்கு = ecif->rvalue;
      அடுக்கு += 4;
      மொத்தம் = 4;
    }
  க்கு(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    எனில் ((nfixedargs == 0) && (மொத்தம் < 24))
      {
        மொத்தம் = 24;
        அடுக்கு = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    தேர்வு((*arg)->type)
      {
      நிலை FFI_TYPE_STRUCT:
        *(வெறுமை **)அடுக்கு = *argv;
        நிறுத்து;

      நிலை FFI_TYPE_SINT8:
        *(signed முழு *) அடுக்கு = (signed முழு)*(SINT8 *)(* argv);
        நிறுத்து;

      நிலை FFI_TYPE_UINT8:
        *(unsigned முழு *) அடுக்கு = (unsigned முழு)*(UINT8 *)(* argv);
        நிறுத்து;

      நிலை FFI_TYPE_SINT16:
        *(signed முழு *) அடுக்கு = (signed முழு)*(SINT16 *)(* argv);
        நிறுத்து;

      நிலை FFI_TYPE_UINT16:
        *(unsigned முழு *) அடுக்கு = (unsigned முழு)*(UINT16 *)(* argv);
        நிறுத்து;

      நிலை FFI_TYPE_SINT32:
      நிலை FFI_TYPE_UINT32:
      நிலை FFI_TYPE_FLOAT:
      நிலை FFI_TYPE_POINTER:
        *(முழு *)அடுக்கு = *(முழு*)(*argv);
        நிறுத்து;

      இயல்பு: /* 8 byte types */
        எனில் (மொத்தம் == 20) /* never split arguments */
          {
            அடுக்கு += 4;
            மொத்தம் += 4;
          }
        s = (*arg)->size;
        memcpy(அடுக்கு, *argv, s);
        நிறுத்து;
      }

    அடுக்கு += s;
    மொத்தம் += s;
    argv++;
    arg++;
  }
  திரும்பு stacktemp + ((மொத்தம்>24)?24:0);
}

வெளி வெறுமை ffi_call_SYSV(unsigned,
                          extended_cif *,
                          வெறுமை *(*)(எழுத்து *, extended_cif *),
                          unsigned *,
                          வெறுமை (*fn)(வெறுமை),
                          unsigned);


வெறுமை ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  முழு i;
  முழு size;
  ffi_type **arg;
  வெறுமை **avalue_copy = வெற்று;

  /* Calculate size to allocate on stack */

  க்கு(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      எனில் ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      இல்லையெனில்
      எனில் ((*arg)->size <= 4)
        size += 4;
      இல்லையெனில்
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        முழு size = at->size;
        எனில் (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            எழுத்து *argcopy = alloca (size);
            எனில் (avalue_copy == வெற்று)
              {
                avalue_copy = alloca (cif->nargs * அளவு (வெறுமை *));
                memcpy (avalue_copy, avalue, cif->nargs * அளவு (வெறுமை *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  எனில் (cif->nargs != cif->nfixedargs)
    size += 24;

  எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  தேர்வு (cif->abi)
  {
    நிலை FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
  }
}


வெறுமை ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  பதிவு முழு *sp __asm__ ("r17");
  பதிவு முழு *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  எழுத்து *அடுக்கு_args = (எழுத்து*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  வெறுமை *struct_rvalue = (வெறுமை *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue = alloca (cif->nargs * அளவு(வெறுமை *));
  எழுத்து *ptr = (எழுத்து *) register_args;
  முழு மொத்தம் = 0;
  முழு nfixedargs = cif->nfixedargs;
  முழு i;

  /* preserve struct type return pointer passing */

  எனில் ((cif->rtype != வெற்று) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    மொத்தம் = 4;
  }

  /* Find the address of each argument.  */
  க்கு (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      எனில் ((nfixedargs == 0) && (மொத்தம் < 24))
        {
          ptr = அடுக்கு_args;
          மொத்தம் = 24;
        }
      nfixedargs--;

      தேர்வு (arg_types[i]->type)
        {
        நிலை FFI_TYPE_SINT8:
        நிலை FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          நிறுத்து;

        நிலை FFI_TYPE_SINT16:
        நிலை FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          நிறுத்து;

        நிலை FFI_TYPE_SINT32:
        நிலை FFI_TYPE_UINT32:
        நிலை FFI_TYPE_FLOAT:
        நிலை FFI_TYPE_POINTER:
          avalue[i] = ptr;
          நிறுத்து;

        நிலை FFI_TYPE_STRUCT:
          avalue[i] = *(வெறுமை**)ptr;
          நிறுத்து;

        இயல்பு:
          /* 8-byte values  */

          /* arguments are never splitted */
          எனில் (ptr == &register_args[5])
            ptr = அடுக்கு_args;
          avalue[i] = ptr;
          ptr += 4;
          மொத்தம் += 4;
          நிறுத்து;
        }
      ptr += 4;
      மொத்தம் += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      எனில் (மொத்தம் == 24)
        ptr = அடுக்கு_args;
    }

  எனில் (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } இல்லையெனில்
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      எனில் (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
                      வெறுமை *user_data,
                      வெறுமை *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

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

  திரும்பு FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  இல்லையெனில்
  எனில் (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  திரும்பு FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned முழு nfixedargs, unsigned முழு ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  திரும்பு status;
}

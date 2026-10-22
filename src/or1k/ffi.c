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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

શૂન્ય* ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
  અક્ષર *stacktemp = ઢગલો;
  પૂર્ણાંક i, s;
  ffi_type **arg;
  પૂર્ણાંક ગણતરી = 0;
  પૂર્ણાંક nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  શૂન્ય **argv = ecif->avalue;

  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(શૂન્ય **) ઢગલો = ecif->rvalue;
      ઢગલો += 4;
      ગણતરી = 4;
    }
  માટે(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    જો ((nfixedargs == 0) && (ગણતરી < 24))
      {
        ગણતરી = 24;
        ઢગલો = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    પસંદગી((*arg)->type)
      {
      સ્થિતિ FFI_TYPE_STRUCT:
        *(શૂન્ય **)ઢગલો = *argv;
        તોડો;

      સ્થિતિ FFI_TYPE_SINT8:
        *(signed પૂર્ણાંક *) ઢગલો = (signed પૂર્ણાંક)*(SINT8 *)(* argv);
        તોડો;

      સ્થિતિ FFI_TYPE_UINT8:
        *(unsigned પૂર્ણાંક *) ઢગલો = (unsigned પૂર્ણાંક)*(UINT8 *)(* argv);
        તોડો;

      સ્થિતિ FFI_TYPE_SINT16:
        *(signed પૂર્ણાંક *) ઢગલો = (signed પૂર્ણાંક)*(SINT16 *)(* argv);
        તોડો;

      સ્થિતિ FFI_TYPE_UINT16:
        *(unsigned પૂર્ણાંક *) ઢગલો = (unsigned પૂર્ણાંક)*(UINT16 *)(* argv);
        તોડો;

      સ્થિતિ FFI_TYPE_SINT32:
      સ્થિતિ FFI_TYPE_UINT32:
      સ્થિતિ FFI_TYPE_FLOAT:
      સ્થિતિ FFI_TYPE_POINTER:
        *(પૂર્ણાંક *)ઢગલો = *(પૂર્ણાંક*)(*argv);
        તોડો;

      મૂળભૂત: /* 8 byte types */
        જો (ગણતરી == 20) /* never split arguments */
          {
            ઢગલો += 4;
            ગણતરી += 4;
          }
        s = (*arg)->size;
        memcpy(ઢગલો, *argv, s);
        તોડો;
      }

    ઢગલો += s;
    ગણતરી += s;
    argv++;
    arg++;
  }
  પાછા stacktemp + ((ગણતરી>24)?24:0);
}

બાહ્ય શૂન્ય ffi_call_SYSV(unsigned,
                          extended_cif *,
                          શૂન્ય *(*)(અક્ષર *, extended_cif *),
                          unsigned *,
                          શૂન્ય (*fn)(શૂન્ય),
                          unsigned);


શૂન્ય ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  પૂર્ણાંક i;
  પૂર્ણાંક size;
  ffi_type **arg;
  શૂન્ય **avalue_copy = ખાલી;

  /* Calculate size to allocate on stack */

  માટે(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      જો ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      નહીંતર
      જો ((*arg)->size <= 4)
        size += 4;
      નહીંતર
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        પૂર્ણાંક size = at->size;
        જો (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            અક્ષર *argcopy = alloca (size);
            જો (avalue_copy == ખાલી)
              {
                avalue_copy = alloca (cif->nargs * કદ (શૂન્ય *));
                memcpy (avalue_copy, avalue, cif->nargs * કદ (શૂન્ય *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  જો (cif->nargs != cif->nfixedargs)
    size += 24;

  જો (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  પસંદગી (cif->abi)
  {
    સ્થિતિ FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
  }
}


શૂન્ય ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  નોંધણી પૂર્ણાંક *sp __asm__ ("r17");
  નોંધણી પૂર્ણાંક *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  અક્ષર *ઢગલો_args = (અક્ષર*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  શૂન્ય *struct_rvalue = (શૂન્ય *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue = alloca (cif->nargs * કદ(શૂન્ય *));
  અક્ષર *ptr = (અક્ષર *) register_args;
  પૂર્ણાંક ગણતરી = 0;
  પૂર્ણાંક nfixedargs = cif->nfixedargs;
  પૂર્ણાંક i;

  /* preserve struct type return pointer passing */

  જો ((cif->rtype != ખાલી) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    ગણતરી = 4;
  }

  /* Find the address of each argument.  */
  માટે (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      જો ((nfixedargs == 0) && (ગણતરી < 24))
        {
          ptr = ઢગલો_args;
          ગણતરી = 24;
        }
      nfixedargs--;

      પસંદગી (arg_types[i]->type)
        {
        સ્થિતિ FFI_TYPE_SINT8:
        સ્થિતિ FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          તોડો;

        સ્થિતિ FFI_TYPE_SINT16:
        સ્થિતિ FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          તોડો;

        સ્થિતિ FFI_TYPE_SINT32:
        સ્થિતિ FFI_TYPE_UINT32:
        સ્થિતિ FFI_TYPE_FLOAT:
        સ્થિતિ FFI_TYPE_POINTER:
          avalue[i] = ptr;
          તોડો;

        સ્થિતિ FFI_TYPE_STRUCT:
          avalue[i] = *(શૂન્ય**)ptr;
          તોડો;

        મૂળભૂત:
          /* 8-byte values  */

          /* arguments are never splitted */
          જો (ptr == &register_args[5])
            ptr = ઢગલો_args;
          avalue[i] = ptr;
          ptr += 4;
          ગણતરી += 4;
          તોડો;
        }
      ptr += 4;
      ગણતરી += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      જો (ગણતરી == 24)
        ptr = ઢગલો_args;
    }

  જો (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } નહીંતર
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      જો (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
                      શૂન્ય *user_data,
                      શૂન્ય *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  જો (cif->abi != FFI_SYSV)
    પાછા FFI_BAD_ABI;

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

  પાછા FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  જો (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  નહીંતર
  જો (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  પાછા FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned પૂર્ણાંક nfixedargs, unsigned પૂર્ણાંક ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  પાછા status;
}

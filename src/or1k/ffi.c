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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য* ffi_prep_args(অক্ষর *স্তূপ, extended_cif *ecif)
{
  অক্ষর *stacktemp = স্তূপ;
  পূর্ণসংখ্যা i, s;
  ffi_type **arg;
  পূর্ণসংখ্যা গুনতি = 0;
  পূর্ণসংখ্যা nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  শূন্য **argv = ecif->avalue;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(শূন্য **) স্তূপ = ecif->rvalue;
      স্তূপ += 4;
      গুনতি = 4;
    }
  জন্য(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    যদি ((nfixedargs == 0) && (গুনতি < 24))
      {
        গুনতি = 24;
        স্তূপ = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    নির্বাচন((*arg)->type)
      {
      ক্ষেত্র FFI_TYPE_STRUCT:
        *(শূন্য **)স্তূপ = *argv;
        বিরতি;

      ক্ষেত্র FFI_TYPE_SINT8:
        *(signed পূর্ণসংখ্যা *) স্তূপ = (signed পূর্ণসংখ্যা)*(SINT8 *)(* argv);
        বিরতি;

      ক্ষেত্র FFI_TYPE_UINT8:
        *(unsigned পূর্ণসংখ্যা *) স্তূপ = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(* argv);
        বিরতি;

      ক্ষেত্র FFI_TYPE_SINT16:
        *(signed পূর্ণসংখ্যা *) স্তূপ = (signed পূর্ণসংখ্যা)*(SINT16 *)(* argv);
        বিরতি;

      ক্ষেত্র FFI_TYPE_UINT16:
        *(unsigned পূর্ণসংখ্যা *) স্তূপ = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(* argv);
        বিরতি;

      ক্ষেত্র FFI_TYPE_SINT32:
      ক্ষেত্র FFI_TYPE_UINT32:
      ক্ষেত্র FFI_TYPE_FLOAT:
      ক্ষেত্র FFI_TYPE_POINTER:
        *(পূর্ণসংখ্যা *)স্তূপ = *(পূর্ণসংখ্যা*)(*argv);
        বিরতি;

      পূর্বনির্ধারিত: /* 8 byte types */
        যদি (গুনতি == 20) /* never split arguments */
          {
            স্তূপ += 4;
            গুনতি += 4;
          }
        s = (*arg)->আকার_2;
        memcpy(স্তূপ, *argv, s);
        বিরতি;
      }

    স্তূপ += s;
    গুনতি += s;
    argv++;
    arg++;
  }
  ফেরত stacktemp + ((গুনতি>24)?24:0);
}

বাহ্যিক শূন্য ffi_call_SYSV(unsigned,
                          extended_cif *,
                          শূন্য *(*)(অক্ষর *, extended_cif *),
                          unsigned *,
                          শূন্য (*fn)(শূন্য),
                          unsigned);


শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা আকার_2;
  ffi_type **arg;
  শূন্য **avalue_copy = ফাঁকা;

  /* Calculate size to allocate on stack */

  জন্য(i = 0, arg = cif->arg_types, আকার_2=0; i < cif->nargs; i++, arg++)
    {
      যদি ((*arg)->type == FFI_TYPE_STRUCT)
        আকার_2 += 4;
      অন্যথা
      যদি ((*arg)->আকার_2 <= 4)
        আকার_2 += 4;
      অন্যথা
        আকার_2 += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        পূর্ণসংখ্যা আকার_2 = at->আকার_2;
        যদি (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            অক্ষর *argcopy = alloca (আকার_2);
            যদি (avalue_copy == ফাঁকা)
              {
                avalue_copy = alloca (cif->nargs * আকার (শূন্য *));
                memcpy (avalue_copy, avalue, cif->nargs * আকার (শূন্য *));
                avalue = avalue_copy;
              }
            memcpy (argcopy, avalue[i], আকার_2);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  যদি (cif->nargs != cif->nfixedargs)
    আকার_2 += 24;

  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    আকার_2 += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  নির্বাচন (cif->abi)
  {
    ক্ষেত্র FFI_SYSV:
      ffi_call_SYSV(আকার_2, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      বিরতি;
    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
  }
}


শূন্য ffi_closure_SYSV(unsigned long r3, unsigned long r4, unsigned long r5,
                      unsigned long r6, unsigned long r7, unsigned long r8)
{
  নিবন্ধ পূর্ণসংখ্যা *sp __asm__ ("r17");
  নিবন্ধ পূর্ণসংখ্যা *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  অক্ষর *স্তূপ_args = (অক্ষর*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  শূন্য *struct_rvalue = (শূন্য *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue = alloca (cif->nargs * আকার(শূন্য *));
  অক্ষর *ptr = (অক্ষর *) register_args;
  পূর্ণসংখ্যা গুনতি = 0;
  পূর্ণসংখ্যা nfixedargs = cif->nfixedargs;
  পূর্ণসংখ্যা i;

  /* preserve struct type return pointer passing */

  যদি ((cif->rtype != ফাঁকা) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    গুনতি = 4;
  }

  /* Find the address of each argument.  */
  জন্য (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      যদি ((nfixedargs == 0) && (গুনতি < 24))
        {
          ptr = স্তূপ_args;
          গুনতি = 24;
        }
      nfixedargs--;

      নির্বাচন (arg_types[i]->type)
        {
        ক্ষেত্র FFI_TYPE_SINT8:
        ক্ষেত্র FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          বিরতি;

        ক্ষেত্র FFI_TYPE_SINT16:
        ক্ষেত্র FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          বিরতি;

        ক্ষেত্র FFI_TYPE_SINT32:
        ক্ষেত্র FFI_TYPE_UINT32:
        ক্ষেত্র FFI_TYPE_FLOAT:
        ক্ষেত্র FFI_TYPE_POINTER:
          avalue[i] = ptr;
          বিরতি;

        ক্ষেত্র FFI_TYPE_STRUCT:
          avalue[i] = *(শূন্য**)ptr;
          বিরতি;

        পূর্বনির্ধারিত:
          /* 8-byte values  */

          /* arguments are never splitted */
          যদি (ptr == &register_args[5])
            ptr = স্তূপ_args;
          avalue[i] = ptr;
          ptr += 4;
          গুনতি += 4;
          বিরতি;
        }
      ptr += 4;
      গুনতি += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      যদি (গুনতি == 24)
        ptr = স্তূপ_args;
    }

  যদি (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_তথ্য);
    } অন্যথা
    {
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_তথ্য);
      যদি (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
                      শূন্য *user_তথ্য,
                      শূন্য *codeloc)
{
  unsigned short *tramp = (unsigned short *) closure->tramp;
  unsigned long fn = (unsigned long) ffi_closure_SYSV;
  unsigned long cls = (unsigned long) codeloc;

  যদি (cif->abi != FFI_SYSV)
    ফেরত FFI_BAD_ABI;

  closure->cif = cif;
  closure->user_তথ্য = user_তথ্য;
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

  ফেরত FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  অন্যথা
  যদি (cif->rtype->আকার_2 > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  ফেরত FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         unsigned পূর্ণসংখ্যা nfixedargs, unsigned পূর্ণসংখ্যা ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  ফেরত status;
}

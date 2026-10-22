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

#समावेश <ffi.h>
#समावेश "ffi_common.h"

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

व्योम* ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
  अक्षर *stacktemp = ढेर;
  पूर्णांक i, s;
  ffi_type **arg;
  पूर्णांक गिनती = 0;
  पूर्णांक nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  व्योम **argv = ecif->avalue;

  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(व्योम **) ढेर = ecif->rvalue;
      ढेर += 4;
      गिनती = 4;
    }
  क्रम(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    अगर ((nfixedargs == 0) && (गिनती < 24))
      {
        गिनती = 24;
        ढेर = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    चयन((*arg)->type)
      {
      हाल FFI_TYPE_STRUCT:
        *(व्योम **)ढेर = *argv;
        अवरोध;

      हाल FFI_TYPE_SINT8:
        *(चिन्हित पूर्णांक *) ढेर = (चिन्हित पूर्णांक)*(SINT8 *)(* argv);
        अवरोध;

      हाल FFI_TYPE_UINT8:
        *(अचिन्हित पूर्णांक *) ढेर = (अचिन्हित पूर्णांक)*(UINT8 *)(* argv);
        अवरोध;

      हाल FFI_TYPE_SINT16:
        *(चिन्हित पूर्णांक *) ढेर = (चिन्हित पूर्णांक)*(SINT16 *)(* argv);
        अवरोध;

      हाल FFI_TYPE_UINT16:
        *(अचिन्हित पूर्णांक *) ढेर = (अचिन्हित पूर्णांक)*(UINT16 *)(* argv);
        अवरोध;

      हाल FFI_TYPE_SINT32:
      हाल FFI_TYPE_UINT32:
      हाल FFI_TYPE_FLOAT:
      हाल FFI_TYPE_POINTER:
        *(पूर्णांक *)ढेर = *(पूर्णांक*)(*argv);
        अवरोध;

      शेष: /* 8 byte types */
        अगर (गिनती == 20) /* never split arguments */
          {
            ढेर += 4;
            गिनती += 4;
          }
        s = (*arg)->आकार;
        स_नकल(ढेर, *argv, s);
        अवरोध;
      }

    ढेर += s;
    गिनती += s;
    argv++;
    arg++;
  }
  वापस stacktemp + ((गिनती>24)?24:0);
}

बाह्य व्योम ffi_call_SYSV(अचिन्हित,
                          extended_cif *,
                          व्योम *(*)(अक्षर *, extended_cif *),
                          अचिन्हित *,
                          व्योम (*fn)(व्योम),
                          अचिन्हित);


व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  पूर्णांक i;
  पूर्णांक आकार;
  ffi_type **arg;
  व्योम **avalue_copy = शून्य;

  /* Calculate size to allocate on stack */

  क्रम(i = 0, arg = cif->arg_types, आकार=0; i < cif->nargs; i++, arg++)
    {
      अगर ((*arg)->type == FFI_TYPE_STRUCT)
        आकार += 4;
      अन्यथा
      अगर ((*arg)->आकार <= 4)
        आकार += 4;
      अन्यथा
        आकार += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        पूर्णांक आकार = at->आकार;
        अगर (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            अक्षर *argcopy = alloca (आकार);
            अगर (avalue_copy == शून्य)
              {
                avalue_copy = alloca (cif->nargs * माप (व्योम *));
                स_नकल (avalue_copy, avalue, cif->nargs * माप (व्योम *));
                avalue = avalue_copy;
              }
            स_नकल (argcopy, avalue[i], आकार);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  अगर (cif->nargs != cif->nfixedargs)
    आकार += 24;

  अगर (cif->rtype->type == FFI_TYPE_STRUCT)
    आकार += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  चयन (cif->abi)
  {
    हाल FFI_SYSV:
      ffi_call_SYSV(आकार, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
  }
}


व्योम ffi_closure_SYSV(अचिन्हित दीर्घ r3, अचिन्हित दीर्घ r4, अचिन्हित दीर्घ r5,
                      अचिन्हित दीर्घ r6, अचिन्हित दीर्घ r7, अचिन्हित दीर्घ r8)
{
  रेजिस्टर पूर्णांक *sp __asm__ ("r17");
  रेजिस्टर पूर्णांक *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  अक्षर *ढेर_args = (अक्षर*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  व्योम *struct_rvalue = (व्योम *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue = alloca (cif->nargs * माप(व्योम *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक गिनती = 0;
  पूर्णांक nfixedargs = cif->nfixedargs;
  पूर्णांक i;

  /* preserve struct type return pointer passing */

  अगर ((cif->rtype != शून्य) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    गिनती = 4;
  }

  /* Find the address of each argument.  */
  क्रम (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      अगर ((nfixedargs == 0) && (गिनती < 24))
        {
          ptr = ढेर_args;
          गिनती = 24;
        }
      nfixedargs--;

      चयन (arg_types[i]->type)
        {
        हाल FFI_TYPE_SINT8:
        हाल FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          अवरोध;

        हाल FFI_TYPE_SINT16:
        हाल FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          अवरोध;

        हाल FFI_TYPE_SINT32:
        हाल FFI_TYPE_UINT32:
        हाल FFI_TYPE_FLOAT:
        हाल FFI_TYPE_POINTER:
          avalue[i] = ptr;
          अवरोध;

        हाल FFI_TYPE_STRUCT:
          avalue[i] = *(व्योम**)ptr;
          अवरोध;

        शेष:
          /* 8-byte values  */

          /* arguments are never splitted */
          अगर (ptr == &register_args[5])
            ptr = ढेर_args;
          avalue[i] = ptr;
          ptr += 4;
          गिनती += 4;
          अवरोध;
        }
      ptr += 4;
      गिनती += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      अगर (गिनती == 24)
        ptr = ढेर_args;
    }

  अगर (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_तथ्य);
    } अन्यथा
    {
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_तथ्य);
      अगर (cif->rtype)
        __asm__ ("l.ori r12, %0, 0x0\n l.lwz r11, 0(r12)\n l.lwz r12, 4(r12)" : : "r" (&rvalue));
    }
}


ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
                      ffi_cif* cif,
                      व्योम (*fun)(ffi_cif*,व्योम*,व्योम**,व्योम*),
                      व्योम *user_तथ्य,
                      व्योम *codeloc)
{
  अचिन्हित लघु *tramp = (अचिन्हित लघु *) closure->tramp;
  अचिन्हित दीर्घ fn = (अचिन्हित दीर्घ) ffi_closure_SYSV;
  अचिन्हित दीर्घ cls = (अचिन्हित दीर्घ) codeloc;

  अगर (cif->abi != FFI_SYSV)
    वापस FFI_BAD_ABI;

  closure->cif = cif;
  closure->user_तथ्य = user_तथ्य;
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

  वापस FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  अगर (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  अन्यथा
  अगर (cif->rtype->आकार > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  वापस FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  वापस status;
}

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

शून्य* ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
  अक्षर *stacktemp = चळत;
  पूर्णांक i, s;
  ffi_type **arg;
  पूर्णांक मोजणी = 0;
  पूर्णांक nfixedargs;

  nfixedargs = ecif->cif->nfixedargs;
  arg = ecif->cif->arg_types;
  शून्य **argv = ecif->avalue;

  जर (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(शून्य **) चळत = ecif->rvalue;
      चळत += 4;
      मोजणी = 4;
    }
  साठी(i=0; i<ecif->cif->nargs; i++)
  {

    /* variadic args are saved on stack */
    जर ((nfixedargs == 0) && (मोजणी < 24))
      {
        मोजणी = 24;
        चळत = stacktemp + 24;
      }
    nfixedargs--;

    s = 4;
    निवड((*arg)->type)
      {
      स्थिती FFI_TYPE_STRUCT:
        *(शून्य **)चळत = *argv;
        थांबा;

      स्थिती FFI_TYPE_SINT8:
        *(चिन्हित पूर्णांक *) चळत = (चिन्हित पूर्णांक)*(SINT8 *)(* argv);
        थांबा;

      स्थिती FFI_TYPE_UINT8:
        *(अचिन्हित पूर्णांक *) चळत = (अचिन्हित पूर्णांक)*(UINT8 *)(* argv);
        थांबा;

      स्थिती FFI_TYPE_SINT16:
        *(चिन्हित पूर्णांक *) चळत = (चिन्हित पूर्णांक)*(SINT16 *)(* argv);
        थांबा;

      स्थिती FFI_TYPE_UINT16:
        *(अचिन्हित पूर्णांक *) चळत = (अचिन्हित पूर्णांक)*(UINT16 *)(* argv);
        थांबा;

      स्थिती FFI_TYPE_SINT32:
      स्थिती FFI_TYPE_UINT32:
      स्थिती FFI_TYPE_FLOAT:
      स्थिती FFI_TYPE_POINTER:
        *(पूर्णांक *)चळत = *(पूर्णांक*)(*argv);
        थांबा;

      पूर्वनिर्धारित: /* 8 byte types */
        जर (मोजणी == 20) /* never split arguments */
          {
            चळत += 4;
            मोजणी += 4;
          }
        s = (*arg)->size;
        स_नकल(चळत, *argv, s);
        थांबा;
      }

    चळत += s;
    मोजणी += s;
    argv++;
    arg++;
  }
  परत stacktemp + ((मोजणी>24)?24:0);
}

बाह्य शून्य ffi_call_SYSV(अचिन्हित,
                          extended_cif *,
                          शून्य *(*)(अक्षर *, extended_cif *),
                          अचिन्हित *,
                          शून्य (*fn)(शून्य),
                          अचिन्हित);


शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  पूर्णांक i;
  पूर्णांक size;
  ffi_type **arg;
  शून्य **avalue_copy = रिक्त;

  /* Calculate size to allocate on stack */

  साठी(i = 0, arg = cif->arg_types, size=0; i < cif->nargs; i++, arg++)
    {
      जर ((*arg)->type == FFI_TYPE_STRUCT)
        size += 4;
      अन्यथा
      जर ((*arg)->size <= 4)
        size += 4;
      अन्यथा
        size += 8;

      /* If we have any large structure arguments, make a copy so we are passing
         by value.  The pointer array is cloned first: the caller owns
         avalue[] and may reuse it for another call, so it must not be
         modified.  */
      {
        ffi_type *at = cif->arg_types[i];
        पूर्णांक size = at->size;
        जर (at->type == FFI_TYPE_STRUCT) /* && size > 4) All struct args? */
          {
            अक्षर *argcopy = alloca (size);
            जर (avalue_copy == रिक्त)
              {
                avalue_copy = alloca (cif->nargs * आकार (शून्य *));
                स_नकल (avalue_copy, avalue, cif->nargs * आकार (शून्य *));
                avalue = avalue_copy;
              }
            स_नकल (argcopy, avalue[i], size);
            avalue[i] = argcopy;
          }
      }
    }

  /* for variadic functions more space is needed on the stack */
  जर (cif->nargs != cif->nfixedargs)
    size += 24;

  जर (cif->rtype->type == FFI_TYPE_STRUCT)
    size += 4;


  extended_cif ecif;
  ecif.cif = cif;
  ecif.avalue = avalue;
  ecif.rvalue = rvalue;

  निवड (cif->abi)
  {
    स्थिती FFI_SYSV:
      ffi_call_SYSV(size, &ecif, ffi_prep_args, rvalue, fn, cif->flags);
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
  }
}


शून्य ffi_closure_SYSV(अचिन्हित दीर्घ r3, अचिन्हित दीर्घ r4, अचिन्हित दीर्घ r5,
                      अचिन्हित दीर्घ r6, अचिन्हित दीर्घ r7, अचिन्हित दीर्घ r8)
{
  नोंदणी पूर्णांक *sp __asm__ ("r17");
  नोंदणी पूर्णांक *r13 __asm__ ("r13");

  ffi_closure* closure = (ffi_closure*) r13;
  अक्षर *चळत_args = (अक्षर*) sp;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { r3, r4, r5, r6, r7, r8 };

  /* Pointer to a struct return value.  */
  शून्य *struct_rvalue = (शून्य *) r3;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * आकार(शून्य *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक मोजणी = 0;
  पूर्णांक nfixedargs = cif->nfixedargs;
  पूर्णांक i;

  /* preserve struct type return pointer passing */

  जर ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT))
  {
    ptr += 4;
    मोजणी = 4;
  }

  /* Find the address of each argument.  */
  साठी (i = 0; i < cif->nargs; i++)
    {

      /* variadic args are saved on stack */
      जर ((nfixedargs == 0) && (मोजणी < 24))
        {
          ptr = चळत_args;
          मोजणी = 24;
        }
      nfixedargs--;

      निवड (arg_types[i]->type)
        {
        स्थिती FFI_TYPE_SINT8:
        स्थिती FFI_TYPE_UINT8:
          avalue[i] = ptr + 3;
          थांबा;

        स्थिती FFI_TYPE_SINT16:
        स्थिती FFI_TYPE_UINT16:
          avalue[i] = ptr + 2;
          थांबा;

        स्थिती FFI_TYPE_SINT32:
        स्थिती FFI_TYPE_UINT32:
        स्थिती FFI_TYPE_FLOAT:
        स्थिती FFI_TYPE_POINTER:
          avalue[i] = ptr;
          थांबा;

        स्थिती FFI_TYPE_STRUCT:
          avalue[i] = *(शून्य**)ptr;
          थांबा;

        पूर्वनिर्धारित:
          /* 8-byte values  */

          /* arguments are never splitted */
          जर (ptr == &register_args[5])
            ptr = चळत_args;
          avalue[i] = ptr;
          ptr += 4;
          मोजणी += 4;
          थांबा;
        }
      ptr += 4;
      मोजणी += 4;

      /* If we've handled more arguments than fit in registers,
         start looking at the those passed on the stack.  */

      जर (मोजणी == 24)
        ptr = चळत_args;
    }

  जर (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    } अन्यथा
    {
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      जर (cif->rtype)
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
  अचिन्हित लघु *tramp = (अचिन्हित लघु *) closure->tramp;
  अचिन्हित दीर्घ fn = (अचिन्हित दीर्घ) ffi_closure_SYSV;
  अचिन्हित दीर्घ cls = (अचिन्हित दीर्घ) codeloc;

  जर (cif->abi != FFI_SYSV)
    परत FFI_BAD_ABI;

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

  परत FFI_OK;
}


ffi_status ffi_prep_cif_machdep (ffi_cif *cif)
{
  cif->flags = 0;

  /* structures are returned as pointers */
  जर (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = FFI_TYPE_STRUCT;
  अन्यथा
  जर (cif->rtype->size > 4)
    cif->flags = FFI_TYPE_UINT64;

  cif->nfixedargs = cif->nargs;

  परत FFI_OK;
}


ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
         अचिन्हित पूर्णांक nfixedargs, अचिन्हित पूर्णांक ntotalargs)
{
  ffi_status status;

  status = ffi_prep_cif_machdep (cif);
  cif->nfixedargs = nfixedargs;
  परत status;
}

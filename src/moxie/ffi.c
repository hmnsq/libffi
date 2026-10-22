/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2012, 2013, 2018, 2021, 2022, 2026  Anthony Green

   Moxie Foreign Function Interface

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
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

व्योम *ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
  रेजिस्टर अचिन्हित पूर्णांक i;
  रेजिस्टर व्योम **p_argv;
  रेजिस्टर अक्षर *argp;
  रेजिस्टर ffi_type **p_arg;
  रेजिस्टर पूर्णांक गिनती = 0;

  p_argv = ecif->avalue;
  argp = ढेर;

  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(व्योम **) argp = ecif->rvalue;
      argp += 4;
    }

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->आकार;

      अगर ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = माप(व्योम*);
	  *(व्योम **) argp = *p_argv;
	}
      अन्यथा अगर (z < माप(पूर्णांक))
	{
	  z = माप(पूर्णांक);
	  चयन ((*p_arg)->type)
	    {
	    हाल FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      अवरोध;

	    हाल FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      अवरोध;

	    हाल FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      अवरोध;

	    हाल FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      अवरोध;

	    शेष:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा अगर (z == माप(पूर्णांक))
	{
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	}
      अन्यथा
	{
	  स_नकल(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      गिनती += z;
    }

  वापस (ढेर + ((गिनती > 24) ? 24 : FFI_ALIGN_DOWN(गिनती, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  अगर (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  अन्यथा
    cif->flags = cif->rtype->आकार;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  वापस FFI_OK;
}

बाह्य व्योम ffi_call_EABI(व्योम *(*)(अक्षर *, extended_cif *),
			  extended_cif *,
			  अचिन्हित, अचिन्हित,
			  अचिन्हित *,
			  व्योम (*fn)(व्योम));

व्योम ffi_call(ffi_cif *cif,
	      व्योम (*fn)(व्योम),
	      व्योम *rvalue,
	      व्योम **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue_copy = शून्य;
  पूर्णांक i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  अगर ((rvalue == शून्य) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक आकार = at->आकार;
      अगर (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          अक्षर *argcopy = alloca (आकार);
          अगर (avalue_copy == शून्य)
            {
              avalue_copy = alloca (nargs * माप (व्योम *));
              स_नकल (avalue_copy, avalue, nargs * माप (व्योम *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  चयन (cif->abi)
    {
    हाल FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      अवरोध;
    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }
}

व्योम ffi_closure_eabi (अचिन्हित arg1, अचिन्हित arg2, अचिन्हित arg3,
		       अचिन्हित arg4, अचिन्हित arg5, अचिन्हित arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  रेजिस्टर ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  रेजिस्टर अक्षर *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  व्योम *struct_rvalue = (व्योम *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  अक्षर *ढेर_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  अक्षर *register_args_ptr = (अक्षर *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue = alloca (cif->nargs * माप(व्योम *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक i;

  /* preserve struct type return pointer passing */
  अगर ((cif->rtype != शून्य) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (अक्षर *)&register_args[1];
  }

  /* Find the address of each argument.  */
  क्रम (i = 0; i < cif->nargs; i++)
    {
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
          {
            अगर (arg_types[i]->आकार > 4)
              {
                व्योम *copy = alloca(arg_types[i]->आकार);
                स_नकल(copy, *(व्योम**)ptr, arg_types[i]->आकार);
                avalue[i] = copy;
              }
            अन्यथा
              avalue[i] = *(व्योम**)ptr;
          }
	  अवरोध;
	शेष:
	  /* This is an 8-byte value.  */
	  अगर (ptr == (अक्षर *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      अचिन्हित *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(अचिन्हित *) ptr;
	      ip[1] = *(अचिन्हित *) ढेर_args;
	    }
	  अन्यथा
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  अवरोध;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      अगर (ptr == (अक्षर *) &register_args[6])
	ptr = ढेर_args;
      अन्यथा अगर (ptr == (अक्षर *) &register_args[7])
	ptr = ढेर_args + 4;
    }

  /* Invoke the closure.  */
  अगर (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_तथ्य);
    }
  अन्यथा
    {
      /* Allocate space for the return value and call the function.  */
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_तथ्य);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित लघु *tramp = (अचिन्हित लघु *) &closure->tramp[0];
  अचिन्हित दीर्घ fn = (दीर्घ) ffi_closure_eabi;
  अचिन्हित दीर्घ cls = (दीर्घ) codeloc;

  अगर (cif->abi != FFI_EABI)
    वापस FFI_BAD_ABI;

  fn = (अचिन्हित दीर्घ) ffi_closure_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  वापस FFI_OK;
}

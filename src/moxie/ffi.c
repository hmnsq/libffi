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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य *ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
  नोंदणी अचिन्हित पूर्णांक i;
  नोंदणी शून्य **p_argv;
  नोंदणी अक्षर *argp;
  नोंदणी ffi_type **p_arg;
  नोंदणी पूर्णांक मोजणी = 0;

  p_argv = ecif->avalue;
  argp = चळत;

  जर (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
    }

  साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      माप_प्रकार z;

      z = (*p_arg)->size;

      जर ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = आकार(शून्य*);
	  *(शून्य **) argp = *p_argv;
	}
      अन्यथा जर (z < आकार(पूर्णांक))
	{
	  z = आकार(पूर्णांक);
	  निवड ((*p_arg)->type)
	    {
	    स्थिती FFI_TYPE_SINT8:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
	      थांबा;

	    स्थिती FFI_TYPE_UINT8:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
	      थांबा;

	    स्थिती FFI_TYPE_SINT16:
	      *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
	      थांबा;

	    स्थिती FFI_TYPE_UINT16:
	      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
	      थांबा;

	    पूर्वनिर्धारित:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा जर (z == आकार(पूर्णांक))
	{
	  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	}
      अन्यथा
	{
	  स_नकल(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      मोजणी += z;
    }

  परत (चळत + ((मोजणी > 24) ? 24 : FFI_ALIGN_DOWN(मोजणी, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  जर (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  अन्यथा
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  परत FFI_OK;
}

बाह्य शून्य ffi_call_EABI(शून्य *(*)(अक्षर *, extended_cif *),
			  extended_cif *,
			  अचिन्हित, अचिन्हित,
			  अचिन्हित *,
			  शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif,
	      शून्य (*fn)(शून्य),
	      शून्य *rvalue,
	      शून्य **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णांक i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  जर ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक size = at->size;
      जर (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          अक्षर *argcopy = alloca (size);
          जर (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * आकार (शून्य *));
              स_नकल (avalue_copy, avalue, nargs * आकार (शून्य *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  निवड (cif->abi)
    {
    स्थिती FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      थांबा;
    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}

शून्य ffi_closure_eabi (अचिन्हित arg1, अचिन्हित arg2, अचिन्हित arg3,
		       अचिन्हित arg4, अचिन्हित arg5, अचिन्हित arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  नोंदणी ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  नोंदणी अक्षर *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  शून्य *struct_rvalue = (शून्य *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  अक्षर *चळत_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  अक्षर *register_args_ptr = (अक्षर *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * आकार(शून्य *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक i;

  /* preserve struct type return pointer passing */
  जर ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (अक्षर *)&register_args[1];
  }

  /* Find the address of each argument.  */
  साठी (i = 0; i < cif->nargs; i++)
    {
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
          {
            जर (arg_types[i]->size > 4)
              {
                शून्य *copy = alloca(arg_types[i]->size);
                स_नकल(copy, *(शून्य**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            अन्यथा
              avalue[i] = *(शून्य**)ptr;
          }
	  थांबा;
	पूर्वनिर्धारित:
	  /* This is an 8-byte value.  */
	  जर (ptr == (अक्षर *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      अचिन्हित *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(अचिन्हित *) ptr;
	      ip[1] = *(अचिन्हित *) चळत_args;
	    }
	  अन्यथा
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  थांबा;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      जर (ptr == (अक्षर *) &register_args[6])
	ptr = चळत_args;
      अन्यथा जर (ptr == (अक्षर *) &register_args[7])
	ptr = चळत_args + 4;
    }

  /* Invoke the closure.  */
  जर (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  अन्यथा
    {
      /* Allocate space for the return value and call the function.  */
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  अचिन्हित लघु *tramp = (अचिन्हित लघु *) &closure->tramp[0];
  अचिन्हित दीर्घ fn = (दीर्घ) ffi_closure_eabi;
  अचिन्हित दीर्घ cls = (दीर्घ) codeloc;

  जर (cif->abi != FFI_EABI)
    परत FFI_BAD_ABI;

  fn = (अचिन्हित दीर्घ) ffi_closure_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  परत FFI_OK;
}

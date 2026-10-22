/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2004, 2026  Anthony Green
   Copyright (C) 2007  Free Software Foundation, Inc.
	   Copyright (C) 2008  Red Hat, Inc.
   
   FR-V Foreign Function Interface 

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
      /*      if ((*p_arg)->type == FFI_TYPE_FLOAT)
	{
	  if (count > 24)
	    {
	      // This is going on the stack.  Turn it into a double.  
	      *(double *) argp = (double) *(float*)(* p_argv);
	      z = sizeof(double);
	    }
	  else
	    *(void **) argp = *(void **)(* p_argv);
	}  */
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

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  जर ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;
    
  
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
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  नोंदणी ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  नोंदणी अक्षर *frame_pointer __asm__ ("fp");
  अक्षर *चळत_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * आकार(शून्य *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक i;

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
	  avalue[i] = ptr;
	  थांबा;
	स्थिती FFI_TYPE_STRUCT:
	  avalue[i] = *(शून्य**)ptr;
	  थांबा;
	पूर्वनिर्धारित:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  थांबा;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      जर (ptr == ((अक्षर *)register_args + (6*4)))
	ptr = चळत_args;
    }

  /* Invoke the closure.  */
  जर (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      नोंदणी शून्य *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  अन्यथा
    {
      /* Allocate space for the return value and call the function.  */
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((पूर्णांक *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  अचिन्हित दीर्घ fn = (दीर्घ) ffi_closure_eabi;
  अचिन्हित दीर्घ cls = (दीर्घ) codeloc;
#ifdef __FRV_FDPIC__
  नोंदणी शून्य *got __asm__("gr15");
#endif
  पूर्णांक i;

  fn = (अचिन्हित दीर्घ) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((अचिन्हित पूर्णांक *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#अन्यथा
  tramp[0] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[1] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[2] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[3] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[4] = 0x80300006;                  /* jmpl @(gr0, gr6)      */
#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  /* Cache flushing.  */
  साठी (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ अस्थिर ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  परत FFI_OK;
}

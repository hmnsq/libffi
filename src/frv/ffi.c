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

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  अगर ((rvalue == शून्य) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;
    
  
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
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  रेजिस्टर ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  रेजिस्टर अक्षर *frame_pointer __asm__ ("fp");
  अक्षर *ढेर_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  अचिन्हित register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue = alloca (cif->nargs * माप(व्योम *));
  अक्षर *ptr = (अक्षर *) register_args;
  पूर्णांक i;

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
	  avalue[i] = ptr;
	  अवरोध;
	हाल FFI_TYPE_STRUCT:
	  avalue[i] = *(व्योम**)ptr;
	  अवरोध;
	शेष:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  अवरोध;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      अगर (ptr == ((अक्षर *)register_args + (6*4)))
	ptr = ढेर_args;
    }

  /* Invoke the closure.  */
  अगर (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      रेजिस्टर व्योम *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_तथ्य);
    }
  अन्यथा
    {
      /* Allocate space for the return value and call the function.  */
      दीर्घ दीर्घ rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_तथ्य);

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
		      व्योम (*fun)(ffi_cif*, व्योम*, व्योम**, व्योम*),
		      व्योम *user_तथ्य,
		      व्योम *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  अचिन्हित दीर्घ fn = (दीर्घ) ffi_closure_eabi;
  अचिन्हित दीर्घ cls = (दीर्घ) codeloc;
#अगर_घोषित __FRV_FDPIC__
  रेजिस्टर व्योम *got __asm__("gr15");
#पूर्ण_अगर
  पूर्णांक i;

  fn = (अचिन्हित दीर्घ) ffi_closure_eabi;

#अगर_घोषित __FRV_FDPIC__
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
#पूर्ण_अगर

  closure->cif = cif;
  closure->fun = fun;
  closure->user_तथ्य = user_तथ्य;

  /* Cache flushing.  */
  क्रम (i = 0; i < FFI_TRAMPOLINE_आकार; i++)
    __asm__ अस्थिर ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  वापस FFI_OK;
}

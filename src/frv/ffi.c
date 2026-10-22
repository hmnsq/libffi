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

शून्य *ffi_prep_args(वर्ण *ढेर, extended_cif *ecif)
{
  पञ्जिका unsigned पूर्णाङ्क i;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;
  पञ्जिका पूर्णाङ्क गिनती = 0;

  p_argv = ecif->avalue;
  argp = ढेर;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      यदि ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = परिमाण(शून्य*);
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
      अन्यथा यदि (z < परिमाण(पूर्णाङ्क))
	{
	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT8 *)(* p_argv);
	      भङ्ग;
	      
	    पक्ष FFI_TYPE_UINT8:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT8 *)(* p_argv);
	      भङ्ग;
	      
	    पक्ष FFI_TYPE_SINT16:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT16 *)(* p_argv);
	      भङ्ग;
		  
	    पक्ष FFI_TYPE_UINT16:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT16 *)(* p_argv);
	      भङ्ग;
		  
	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
	  *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT32 *)(* p_argv);
	}
      अन्यथा
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      गिनती += z;
    }

  प्रत्यागम (ढेर + ((गिनती > 24) ? 24 : FFI_ALIGN_DOWN(गिनती, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  यदि (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  अन्यथा
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_EABI(शून्य *(*)(वर्ण *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
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

  यदि ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;
    
  
  विकल्प (cif->abi) 
    {
    पक्ष FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

शून्य ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  पञ्जिका ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  पञ्जिका वर्ण *frame_pointer __asm__ ("fp");
  वर्ण *ढेर_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * परिमाण(शून्य *));
  वर्ण *ptr = (वर्ण *) register_args;
  पूर्णाङ्क i;

  /* Find the address of each argument.  */
  प्रति (i = 0; i < cif->nargs; i++)
    {
      विकल्प (arg_types[i]->type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
	  avalue[i] = *(शून्य**)ptr;
	  भङ्ग;
	स्वभाव:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  भङ्ग;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      यदि (ptr == ((वर्ण *)register_args + (6*4)))
	ptr = ढेर_args;
    }

  /* Invoke the closure.  */
  यदि (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      पञ्जिका शून्य *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  अन्यथा
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((पूर्णाङ्क *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  पञ्जिका शून्य *got __asm__("gr15");
#endif
  पूर्णाङ्क i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned पूर्णाङ्क *)codeloc)[2];
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
  प्रति (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ चञ्चल ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  प्रत्यागम FFI_OK;
}

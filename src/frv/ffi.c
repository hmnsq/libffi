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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ശൂന്യം *ffi_prep_args(അക്ഷരം *അട്ടി, extended_cif *ecif)
{
  രജിസ്റ്റർ unsigned പൂർണ്ണസംഖ്യ i;
  രജിസ്റ്റർ ശൂന്യം **p_argv;
  രജിസ്റ്റർ അക്ഷരം *argp;
  രജിസ്റ്റർ ffi_type **p_arg;
  രജിസ്റ്റർ പൂർണ്ണസംഖ്യ എണ്ണം = 0;

  p_argv = ecif->avalue;
  argp = അട്ടി;

  വേണ്ടി (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      എങ്കിൽ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = വലുപ്പം(ശൂന്യം*);
	  *(ശൂന്യം **) argp = *p_argv;
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
      അല്ലെങ്കിൽ എങ്കിൽ (z < വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  z = വലുപ്പം(പൂർണ്ണസംഖ്യ);
	  തിരഞ്ഞെടുപ്പ് ((*p_arg)->type)
	    {
	    അവസ്ഥ FFI_TYPE_SINT8:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT8 *)(* p_argv);
	      നിർത്തുക;
	      
	    അവസ്ഥ FFI_TYPE_UINT8:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT8 *)(* p_argv);
	      നിർത്തുക;
	      
	    അവസ്ഥ FFI_TYPE_SINT16:
	      *(signed പൂർണ്ണസംഖ്യ *) argp = (signed പൂർണ്ണസംഖ്യ)*(SINT16 *)(* p_argv);
	      നിർത്തുക;
		  
	    അവസ്ഥ FFI_TYPE_UINT16:
	      *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT16 *)(* p_argv);
	      നിർത്തുക;
		  
	    സ്ഥിരസ്ഥിതി:
	      FFI_ASSERT(0);
	    }
	}
      അല്ലെങ്കിൽ എങ്കിൽ (z == വലുപ്പം(പൂർണ്ണസംഖ്യ))
	{
	  *(unsigned പൂർണ്ണസംഖ്യ *) argp = (unsigned പൂർണ്ണസംഖ്യ)*(UINT32 *)(* p_argv);
	}
      അല്ലെങ്കിൽ
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      എണ്ണം += z;
    }

  തിരികെ (അട്ടി + ((എണ്ണം > 24) ? 24 : FFI_ALIGN_DOWN(എണ്ണം, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  അല്ലെങ്കിൽ
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  തിരികെ FFI_OK;
}

ബാഹ്യ ശൂന്യം ffi_call_EABI(ശൂന്യം *(*)(അക്ഷരം *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  ശൂന്യം (*fn)(ശൂന്യം));

ശൂന്യം ffi_call(ffi_cif *cif, 
	      ശൂന്യം (*fn)(ശൂന്യം), 
	      ശൂന്യം *rvalue, 
	      ശൂന്യം **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  എങ്കിൽ ((rvalue == ശൂന്യ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;
    
  
  തിരഞ്ഞെടുപ്പ് (cif->abi) 
    {
    അവസ്ഥ FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}

ശൂന്യം ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  രജിസ്റ്റർ ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  രജിസ്റ്റർ അക്ഷരം *frame_pointer __asm__ ("fp");
  അക്ഷരം *അട്ടി_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue = alloca (cif->nargs * വലുപ്പം(ശൂന്യം *));
  അക്ഷരം *ptr = (അക്ഷരം *) register_args;
  പൂർണ്ണസംഖ്യ i;

  /* Find the address of each argument.  */
  വേണ്ടി (i = 0; i < cif->nargs; i++)
    {
      തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
	{
	അവസ്ഥ FFI_TYPE_SINT8:
	അവസ്ഥ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT16:
	അവസ്ഥ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_SINT32:
	അവസ്ഥ FFI_TYPE_UINT32:
	അവസ്ഥ FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_STRUCT:
	  avalue[i] = *(ശൂന്യം**)ptr;
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  നിർത്തുക;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      എങ്കിൽ (ptr == ((അക്ഷരം *)register_args + (6*4)))
	ptr = അട്ടി_args;
    }

  /* Invoke the closure.  */
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      രജിസ്റ്റർ ശൂന്യം *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  അല്ലെങ്കിൽ
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((പൂർണ്ണസംഖ്യ *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  രജിസ്റ്റർ ശൂന്യം *got __asm__("gr15");
#endif
  പൂർണ്ണസംഖ്യ i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned പൂർണ്ണസംഖ്യ *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#അല്ലെങ്കിൽ
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
  വേണ്ടി (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ അസ്ഥിര ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  തിരികെ FFI_OK;
}

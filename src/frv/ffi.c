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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

શૂન્ય *ffi_prep_args(અક્ષર *ઢગલો, extended_cif *ecif)
{
  નોંધણી unsigned પૂર્ણાંક i;
  નોંધણી શૂન્ય **p_argv;
  નોંધણી અક્ષર *argp;
  નોંધણી ffi_type **p_arg;
  નોંધણી પૂર્ણાંક ગણતરી = 0;

  p_argv = ecif->avalue;
  argp = ઢગલો;

  માટે (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      જો ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = કદ(શૂન્ય*);
	  *(શૂન્ય **) argp = *p_argv;
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
      નહીંતર જો (z < કદ(પૂર્ણાંક))
	{
	  z = કદ(પૂર્ણાંક);
	  પસંદગી ((*p_arg)->type)
	    {
	    સ્થિતિ FFI_TYPE_SINT8:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT8 *)(* p_argv);
	      તોડો;
	      
	    સ્થિતિ FFI_TYPE_UINT8:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT8 *)(* p_argv);
	      તોડો;
	      
	    સ્થિતિ FFI_TYPE_SINT16:
	      *(signed પૂર્ણાંક *) argp = (signed પૂર્ણાંક)*(SINT16 *)(* p_argv);
	      તોડો;
		  
	    સ્થિતિ FFI_TYPE_UINT16:
	      *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT16 *)(* p_argv);
	      તોડો;
		  
	    મૂળભૂત:
	      FFI_ASSERT(0);
	    }
	}
      નહીંતર જો (z == કદ(પૂર્ણાંક))
	{
	  *(unsigned પૂર્ણાંક *) argp = (unsigned પૂર્ણાંક)*(UINT32 *)(* p_argv);
	}
      નહીંતર
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ગણતરી += z;
    }

  પાછા (ઢગલો + ((ગણતરી > 24) ? 24 : FFI_ALIGN_DOWN(ગણતરી, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  જો (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  નહીંતર
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  પાછા FFI_OK;
}

બાહ્ય શૂન્ય ffi_call_EABI(શૂન્ય *(*)(અક્ષર *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  શૂન્ય (*fn)(શૂન્ય));

શૂન્ય ffi_call(ffi_cif *cif, 
	      શૂન્ય (*fn)(શૂન્ય), 
	      શૂન્ય *rvalue, 
	      શૂન્ય **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  જો ((rvalue == ખાલી) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;
    
  
  પસંદગી (cif->abi) 
    {
    સ્થિતિ FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      તોડો;
    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}

શૂન્ય ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  નોંધણી ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  નોંધણી અક્ષર *frame_pointer __asm__ ("fp");
  અક્ષર *ઢગલો_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue = alloca (cif->nargs * કદ(શૂન્ય *));
  અક્ષર *ptr = (અક્ષર *) register_args;
  પૂર્ણાંક i;

  /* Find the address of each argument.  */
  માટે (i = 0; i < cif->nargs; i++)
    {
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
	  avalue[i] = ptr;
	  તોડો;
	સ્થિતિ FFI_TYPE_STRUCT:
	  avalue[i] = *(શૂન્ય**)ptr;
	  તોડો;
	મૂળભૂત:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  તોડો;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      જો (ptr == ((અક્ષર *)register_args + (6*4)))
	ptr = ઢગલો_args;
    }

  /* Invoke the closure.  */
  જો (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      નોંધણી શૂન્ય *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  નહીંતર
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((પૂર્ણાંક *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  નોંધણી શૂન્ય *got __asm__("gr15");
#endif
  પૂર્ણાંક i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned પૂર્ણાંક *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#નહીંતર
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
  માટે (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ અસ્થિર ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  પાછા FFI_OK;
}

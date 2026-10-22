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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

வெறுமை *ffi_prep_args(எழுத்து *அடுக்கு, extended_cif *ecif)
{
  பதிவு unsigned முழு i;
  பதிவு வெறுமை **p_argv;
  பதிவு எழுத்து *argp;
  பதிவு ffi_type **p_arg;
  பதிவு முழு மொத்தம் = 0;

  p_argv = ecif->avalue;
  argp = அடுக்கு;

  க்கு (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      எனில் ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = அளவு(வெறுமை*);
	  *(வெறுமை **) argp = *p_argv;
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
      இல்லையெனில் எனில் (z < அளவு(முழு))
	{
	  z = அளவு(முழு);
	  தேர்வு ((*p_arg)->type)
	    {
	    நிலை FFI_TYPE_SINT8:
	      *(signed முழு *) argp = (signed முழு)*(SINT8 *)(* p_argv);
	      நிறுத்து;
	      
	    நிலை FFI_TYPE_UINT8:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT8 *)(* p_argv);
	      நிறுத்து;
	      
	    நிலை FFI_TYPE_SINT16:
	      *(signed முழு *) argp = (signed முழு)*(SINT16 *)(* p_argv);
	      நிறுத்து;
		  
	    நிலை FFI_TYPE_UINT16:
	      *(unsigned முழு *) argp = (unsigned முழு)*(UINT16 *)(* p_argv);
	      நிறுத்து;
		  
	    இயல்பு:
	      FFI_ASSERT(0);
	    }
	}
      இல்லையெனில் எனில் (z == அளவு(முழு))
	{
	  *(unsigned முழு *) argp = (unsigned முழு)*(UINT32 *)(* p_argv);
	}
      இல்லையெனில்
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      மொத்தம் += z;
    }

  திரும்பு (அடுக்கு + ((மொத்தம் > 24) ? 24 : FFI_ALIGN_DOWN(மொத்தம், 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  இல்லையெனில்
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  திரும்பு FFI_OK;
}

வெளி வெறுமை ffi_call_EABI(வெறுமை *(*)(எழுத்து *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  வெறுமை (*fn)(வெறுமை));

வெறுமை ffi_call(ffi_cif *cif, 
	      வெறுமை (*fn)(வெறுமை), 
	      வெறுமை *rvalue, 
	      வெறுமை **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  எனில் ((rvalue == வெற்று) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;
    
  
  தேர்வு (cif->abi) 
    {
    நிலை FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      நிறுத்து;
    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}

வெறுமை ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  பதிவு ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  பதிவு எழுத்து *frame_pointer __asm__ ("fp");
  எழுத்து *அடுக்கு_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue = alloca (cif->nargs * அளவு(வெறுமை *));
  எழுத்து *ptr = (எழுத்து *) register_args;
  முழு i;

  /* Find the address of each argument.  */
  க்கு (i = 0; i < cif->nargs; i++)
    {
      தேர்வு (arg_types[i]->type)
	{
	நிலை FFI_TYPE_SINT8:
	நிலை FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT16:
	நிலை FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  நிறுத்து;
	நிலை FFI_TYPE_SINT32:
	நிலை FFI_TYPE_UINT32:
	நிலை FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  நிறுத்து;
	நிலை FFI_TYPE_STRUCT:
	  avalue[i] = *(வெறுமை**)ptr;
	  நிறுத்து;
	இயல்பு:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  நிறுத்து;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      எனில் (ptr == ((எழுத்து *)register_args + (6*4)))
	ptr = அடுக்கு_args;
    }

  /* Invoke the closure.  */
  எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      பதிவு வெறுமை *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  இல்லையெனில்
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((முழு *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp = (unsigned முழு *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  பதிவு வெறுமை *got __asm__("gr15");
#endif
  முழு i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned முழு *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#இல்லையெனில்
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
  க்கு (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ நிலையற்ற ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  திரும்பு FFI_OK;
}

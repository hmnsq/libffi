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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ *ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
  ପଞ୍ଜିକରଣ ffi_type **p_arg;
  ପଞ୍ଜିକରଣ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;

  p_argv = ecif->avalue;
  argp = ଗଦା;

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      ଯଦି ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ଆକାର(ଶୂନ୍ୟ*);
	  *(ଶୂନ୍ୟ **) argp = *p_argv;
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
      ଅନ୍ୟଥା ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
	      
	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;
	      
	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
		  
	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;
		  
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	}
      ଅନ୍ୟଥା
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ଗଣନା_2 += z;
    }

  ଫେରନ୍ତୁ (ଗଦା + ((ଗଣନା_2 > 24) ? 24 : FFI_ALIGN_DOWN(ଗଣନା_2, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ଅନ୍ୟଥା
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_EABI(ଶୂନ୍ୟ *(*)(ଅକ୍ଷର *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));

ଶୂନ୍ୟ ffi_call(ffi_cif *cif, 
	      ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), 
	      ଶୂନ୍ୟ *rvalue, 
	      ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ଯଦି ((rvalue == ଖାଲି) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;
    
  
  ବାଛ (cif->abi) 
    {
    ସ୍ଥିତି FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  ପଞ୍ଜିକରଣ ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *frame_pointer __asm__ ("fp");
  ଅକ୍ଷର *ଗଦା_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue = alloca (cif->nargs * ଆକାର(ଶୂନ୍ୟ *));
  ଅକ୍ଷର *ptr = (ଅକ୍ଷର *) register_args;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* Find the address of each argument.  */
  ପାଇଁ (i = 0; i < cif->nargs; i++)
    {
      ବାଛ (arg_types[i]->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_STRUCT:
	  avalue[i] = *(ଶୂନ୍ୟ**)ptr;
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  ଭାଙ୍ଗ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ଯଦି (ptr == ((ଅକ୍ଷର *)register_args + (6*4)))
	ptr = ଗଦା_args;
    }

  /* Invoke the closure.  */
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      ପଞ୍ଜିକରଣ ଶୂନ୍ୟ *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  ଅନ୍ୟଥା
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ *got __asm__("gr15");
#endif
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ଅନ୍ୟଥା
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
  ପାଇଁ (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ ଅସ୍ଥିର ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  ଫେରନ୍ତୁ FFI_OK;
}

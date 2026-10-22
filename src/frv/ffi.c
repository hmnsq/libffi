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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

تهی *ffi_prep_args(نویسه *پشته, extended_cif *ecif)
{
  ثبات unsigned صحیح i;
  ثبات تهی **p_argv;
  ثبات نویسه *argp;
  ثبات ffi_type **p_arg;
  ثبات صحیح شمار = 0;

  p_argv = ecif->avalue;
  argp = پشته;

  برای (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = اندازه(تهی*);
	  *(تهی **) argp = *p_argv;
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
      وگرنه اگر (z < اندازه(صحیح))
	{
	  z = اندازه(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT8 *)(* p_argv);
	      بشکن;
	      
	    حالت FFI_TYPE_UINT8:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT8 *)(* p_argv);
	      بشکن;
	      
	    حالت FFI_TYPE_SINT16:
	      *(signed صحیح *) argp = (signed صحیح)*(SINT16 *)(* p_argv);
	      بشکن;
		  
	    حالت FFI_TYPE_UINT16:
	      *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT16 *)(* p_argv);
	      بشکن;
		  
	    پیشفرض:
	      FFI_ASSERT(0);
	    }
	}
      وگرنه اگر (z == اندازه(صحیح))
	{
	  *(unsigned صحیح *) argp = (unsigned صحیح)*(UINT32 *)(* p_argv);
	}
      وگرنه
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      شمار += z;
    }

  بازگشت (پشته + ((شمار > 24) ? 24 : FFI_ALIGN_DOWN(شمار, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وگرنه
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  بازگشت FFI_OK;
}

بیرونی تهی ffi_call_EABI(تهی *(*)(نویسه *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  تهی (*fn)(تهی));

تهی ffi_call(ffi_cif *cif, 
	      تهی (*fn)(تهی), 
	      تهی *rvalue, 
	      تهی **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  اگر ((rvalue == پوچ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;
    
  
  انتخاب (cif->abi) 
    {
    حالت FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      بشکن;
    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
}

تهی ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  ثبات ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ثبات نویسه *frame_pointer __asm__ ("fp");
  نویسه *پشته_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue = alloca (cif->nargs * اندازه(تهی *));
  نویسه *ptr = (نویسه *) register_args;
  صحیح i;

  /* Find the address of each argument.  */
  برای (i = 0; i < cif->nargs; i++)
    {
      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  بشکن;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  بشکن;
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  بشکن;
	حالت FFI_TYPE_STRUCT:
	  avalue[i] = *(تهی**)ptr;
	  بشکن;
	پیشفرض:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  بشکن;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      اگر (ptr == ((نویسه *)register_args + (6*4)))
	ptr = پشته_args;
    }

  /* Invoke the closure.  */
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      ثبات تهی *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  وگرنه
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((صحیح *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp = (unsigned صحیح *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  ثبات تهی *got __asm__("gr15");
#endif
  صحیح i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned صحیح *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#وگرنه
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
  برای (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ فرار ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  بازگشت FFI_OK;
}

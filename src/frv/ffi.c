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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য *ffi_prep_args(বর্ণ *স্তূপ, extended_cif *ecif)
{
  পঞ্জিকা unsigned পূর্ণাঙ্ক i;
  পঞ্জিকা শূন্য **p_argv;
  পঞ্জিকা বর্ণ *argp;
  পঞ্জিকা ffi_type **p_arg;
  পঞ্জিকা পূর্ণাঙ্ক গুনতি = 0;

  p_argv = ecif->avalue;
  argp = স্তূপ;

  প্রতি (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      যদি ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = পরিমাণ(শূন্য*);
	  *(শূন্য **) argp = *p_argv;
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
      অন্যথা যদি (z < পরিমাণ(পূর্ণাঙ্ক))
	{
	  z = পরিমাণ(পূর্ণাঙ্ক);
	  বিকল্প ((*p_arg)->type)
	    {
	    পক্ষ FFI_TYPE_SINT8:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT8 *)(* p_argv);
	      ভঙ্গ;
	      
	    পক্ষ FFI_TYPE_UINT8:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT8 *)(* p_argv);
	      ভঙ্গ;
	      
	    পক্ষ FFI_TYPE_SINT16:
	      *(signed পূর্ণাঙ্ক *) argp = (signed পূর্ণাঙ্ক)*(SINT16 *)(* p_argv);
	      ভঙ্গ;
		  
	    পক্ষ FFI_TYPE_UINT16:
	      *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT16 *)(* p_argv);
	      ভঙ্গ;
		  
	    স্বভাব:
	      FFI_ASSERT(0);
	    }
	}
      অন্যথা যদি (z == পরিমাণ(পূর্ণাঙ্ক))
	{
	  *(unsigned পূর্ণাঙ্ক *) argp = (unsigned পূর্ণাঙ্ক)*(UINT32 *)(* p_argv);
	}
      অন্যথা
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      গুনতি += z;
    }

  প্রত্যাগম (স্তূপ + ((গুনতি > 24) ? 24 : FFI_ALIGN_DOWN(গুনতি, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  অন্যথা
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  প্রত্যাগম FFI_OK;
}

বাহ্য শূন্য ffi_call_EABI(শূন্য *(*)(বর্ণ *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif, 
	      শূন্য (*fn)(শূন্য), 
	      শূন্য *rvalue, 
	      শূন্য **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  যদি ((rvalue == রিক্ত) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  অন্যথা
    ecif.rvalue = rvalue;
    
  
  বিকল্প (cif->abi) 
    {
    পক্ষ FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      ভঙ্গ;
    স্বভাব:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
}

শূন্য ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  পঞ্জিকা ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  পঞ্জিকা বর্ণ *frame_pointer __asm__ ("fp");
  বর্ণ *স্তূপ_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue = alloca (cif->nargs * পরিমাণ(শূন্য *));
  বর্ণ *ptr = (বর্ণ *) register_args;
  পূর্ণাঙ্ক i;

  /* Find the address of each argument.  */
  প্রতি (i = 0; i < cif->nargs; i++)
    {
      বিকল্প (arg_types[i]->type)
	{
	পক্ষ FFI_TYPE_SINT8:
	পক্ষ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_SINT16:
	পক্ষ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_SINT32:
	পক্ষ FFI_TYPE_UINT32:
	পক্ষ FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  ভঙ্গ;
	পক্ষ FFI_TYPE_STRUCT:
	  avalue[i] = *(শূন্য**)ptr;
	  ভঙ্গ;
	স্বভাব:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  ভঙ্গ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      যদি (ptr == ((বর্ণ *)register_args + (6*4)))
	ptr = স্তূপ_args;
    }

  /* Invoke the closure.  */
  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      পঞ্জিকা শূন্য *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  অন্যথা
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((পূর্ণাঙ্ক *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  পঞ্জিকা শূন্য *got __asm__("gr15");
#endif
  পূর্ণাঙ্ক i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned পূর্ণাঙ্ক *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#অন্যথা
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
  প্রতি (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ চঞ্চল ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  প্রত্যাগম FFI_OK;
}

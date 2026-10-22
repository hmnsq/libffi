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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಶೂನ್ಯ *ffi_prep_args(ವರ್ಣ *ರಾಶಿ, extended_cif *ecif)
{
  ಪಞ್ಜಿಕಾ unsigned ಪೂರ್ಣಾಙ್ಕ i;
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ **p_argv;
  ಪಞ್ಜಿಕಾ ವರ್ಣ *argp;
  ಪಞ್ಜಿಕಾ ffi_type **p_arg;
  ಪಞ್ಜಿಕಾ ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ = 0;

  p_argv = ecif->avalue;
  argp = ರಾಶಿ;

  ಪ್ರತಿ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      ಯದಿ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ಪರಿಮಾಣ(ಶೂನ್ಯ*);
	  *(ಶೂನ್ಯ **) argp = *p_argv;
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
      ಅನ್ಯಥಾ ಯದಿ (z < ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  z = ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ);
	  ವಿಕಲ್ಪ ((*p_arg)->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_SINT8:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT8 *)(* p_argv);
	      ಭಙ್ಗ;
	      
	    ಪಕ್ಷ FFI_TYPE_UINT8:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT8 *)(* p_argv);
	      ಭಙ್ಗ;
	      
	    ಪಕ್ಷ FFI_TYPE_SINT16:
	      *(signed ಪೂರ್ಣಾಙ್ಕ *) argp = (signed ಪೂರ್ಣಾಙ್ಕ)*(SINT16 *)(* p_argv);
	      ಭಙ್ಗ;
		  
	    ಪಕ್ಷ FFI_TYPE_UINT16:
	      *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT16 *)(* p_argv);
	      ಭಙ್ಗ;
		  
	    ಸ್ವಭಾವ:
	      FFI_ASSERT(0);
	    }
	}
      ಅನ್ಯಥಾ ಯದಿ (z == ಪರಿಮಾಣ(ಪೂರ್ಣಾಙ್ಕ))
	{
	  *(unsigned ಪೂರ್ಣಾಙ್ಕ *) argp = (unsigned ಪೂರ್ಣಾಙ್ಕ)*(UINT32 *)(* p_argv);
	}
      ಅನ್ಯಥಾ
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ಎಣಿಕೆ += z;
    }

  ಪ್ರತ್ಯಾಗಮ (ರಾಶಿ + ((ಎಣಿಕೆ > 24) ? 24 : FFI_ALIGN_DOWN(ಎಣಿಕೆ, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ಅನ್ಯಥಾ
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_call_EABI(ಶೂನ್ಯ *(*)(ವರ್ಣ *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ));

ಶೂನ್ಯ ffi_call(ffi_cif *cif, 
	      ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), 
	      ಶೂನ್ಯ *rvalue, 
	      ಶೂನ್ಯ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಯದಿ ((rvalue == ರಿಕ್ತ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;
    
  
  ವಿಕಲ್ಪ (cif->abi) 
    {
    ಪಕ್ಷ FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}

ಶೂನ್ಯ ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  ಪಞ್ಜಿಕಾ ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ಪಞ್ಜಿಕಾ ವರ್ಣ *frame_pointer __asm__ ("fp");
  ವರ್ಣ *ರಾಶಿ_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  ವರ್ಣ *ptr = (ವರ್ಣ *) register_args;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* Find the address of each argument.  */
  ಪ್ರತಿ (i = 0; i < cif->nargs; i++)
    {
      ವಿಕಲ್ಪ (arg_types[i]->type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_SINT32:
	ಪಕ್ಷ FFI_TYPE_UINT32:
	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  avalue[i] = *(ಶೂನ್ಯ**)ptr;
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  ಭಙ್ಗ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ಯದಿ (ptr == ((ವರ್ಣ *)register_args + (6*4)))
	ptr = ರಾಶಿ_args;
    }

  /* Invoke the closure.  */
  ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      ಪಞ್ಜಿಕಾ ಶೂನ್ಯ *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  ಅನ್ಯಥಾ
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((ಪೂರ್ಣಾಙ್ಕ *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  ಪಞ್ಜಿಕಾ ಶೂನ್ಯ *got __asm__("gr15");
#endif
  ಪೂರ್ಣಾಙ್ಕ i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned ಪೂರ್ಣಾಙ್ಕ *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ಅನ್ಯಥಾ
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
  ಪ್ರತಿ (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ ಚಞ್ಚಲ ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

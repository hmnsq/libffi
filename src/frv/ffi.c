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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਰਜਿਸਟਰ ਅੱਖਰ *argp;
  ਰਜਿਸਟਰ ffi_type **p_arg;
  ਰਜਿਸਟਰ ਪੂਰਨ ਗਿਣਤੀ_2 = 0;

  p_argv = ecif->avalue;
  argp = ਢੇਰ;

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->ਮਾਪ;

      ਜੇ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ਆਕਾਰ(ਖਾਲੀ*);
	  *(ਖਾਲੀ **) argp = *p_argv;
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
      ਨਹੀਂਤਾਂ ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
	      ਰੋਕੋ;
	      
	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
	      ਰੋਕੋ;
	      
	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
	      ਰੋਕੋ;
		  
	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
	      ਰੋਕੋ;
		  
	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	}
      ਨਹੀਂਤਾਂ
	{
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ਗਿਣਤੀ_2 += z;
    }

  ਵਾਪਸ (ਢੇਰ + ((ਗਿਣਤੀ_2 > 24) ? 24 : FFI_ALIGN_ਥੱਲੇ(ਗਿਣਤੀ_2, 8)));
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ਨਹੀਂਤਾਂ
    cif->flags = cif->rtype->ਮਾਪ;

  cif->ਬਾਈਟਾਂ = FFI_ALIGN (cif->ਬਾਈਟਾਂ, 8);

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_EABI(ਖਾਲੀ *(*)(ਅੱਖਰ *, extended_cif *), 
			  extended_cif *, 
			  ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ, 
			  ਬਿਨਾਨਿਸ਼ਾਨੀ *, 
			  ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif, 
	      ਖਾਲੀ (*fn)(ਖਾਲੀ), 
	      ਖਾਲੀ *rvalue, 
	      ਖਾਲੀ **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ਜੇ ((rvalue == ਨਿਰਾ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;
    
  
  ਚੋਣ (cif->abi) 
    {
    ਹਾਲਤ FFI_EABI:
      ffi_call_EABI(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ, 
		    cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}

ਖਾਲੀ ffi_ਘੇਰਾ_eabi (ਬਿਨਾਨਿਸ਼ਾਨੀ arg1, ਬਿਨਾਨਿਸ਼ਾਨੀ arg2, ਬਿਨਾਨਿਸ਼ਾਨੀ arg3,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ arg4, ਬਿਨਾਨਿਸ਼ਾਨੀ arg5, ਬਿਨਾਨਿਸ਼ਾਨੀ arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  ਰਜਿਸਟਰ ffi_ਘੇਰਾ *creg __asm__ ("gr7");
  ffi_ਘੇਰਾ *ਘੇਰਾ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ਰਜਿਸਟਰ ਅੱਖਰ *ਚੌਖਟਾ_ਸੂਚਕ __asm__ ("fp");
  ਅੱਖਰ *ਢੇਰ_ਦਲੀਲਾਂ = ਚੌਖਟਾ_ਸੂਚਕ + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = ਘੇਰਾ->cif;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue = alloca (cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  ਅੱਖਰ *ptr = (ਅੱਖਰ *) ਰਜਿਸਟਰ_ਦਲੀਲਾਂ;
  ਪੂਰਨ i;

  /* Find the address of each argument.  */
  ਲਈ (i = 0; i < cif->nargs; i++)
    {
      ਚੋਣ (arg_types[i]->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  avalue[i] = *(ਖਾਲੀ**)ptr;
	  ਰੋਕੋ;
	ਮੂਲ:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  ਰੋਕੋ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ਜੇ (ptr == ((ਅੱਖਰ *)ਰਜਿਸਟਰ_ਦਲੀਲਾਂ + (6*4)))
	ptr = ਢੇਰ_ਦਲੀਲਾਂ;
    }

  /* Invoke the closure.  */
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      ਰਜਿਸਟਰ ਖਾਲੀ *return_struct_ptr __asm__("gr3");
      (ਘੇਰਾ->fun) (cif, return_struct_ptr, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
    }
  ਨਹੀਂਤਾਂ
    {
      /* Allocate space for the return value and call the function.  */
      ਲੰਮਾ ਲੰਮਾ rvalue;
      (ਘੇਰਾ->fun) (cif, &rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((ਪੂਰਨ *) &rvalue)[1]));
    }
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn = (ਲੰਮਾ) ffi_ਘੇਰਾ_eabi;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cls = (ਲੰਮਾ) codeloc;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FRV_FDPIC__
  ਰਜਿਸਟਰ ਖਾਲੀ *got __asm__("gr15");
#ਜੇ_ਅੰਤ
  ਪੂਰਨ i;

  fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_eabi;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __FRV_FDPIC__
  tramp[0] = &((ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ਨਹੀਂਤਾਂ
  tramp[0] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[1] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[2] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[3] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[4] = 0x80300006;                  /* jmpl @(gr0, gr6)      */
#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  /* Cache flushing.  */
  ਲਈ (i = 0; i < FFI_TRAMPOLINE_ਮਾਪ; i++)
    __asm__ ਅਸਥਿਰ ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  ਵਾਪਸ FFI_OK;
}

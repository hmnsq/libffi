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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

void *ffi_prep_args(var_na *stack, extended_cif *ecif)
{
  pa_njikaa unsigned puur_naa_nka i;
  pa_njikaa void **p_argv;
  pa_njikaa var_na *argp;
  pa_njikaa ffi_type **p_arg;
  pa_njikaa puur_naa_nka count = 0;

  p_argv = ecif->avalue;
  argp = stack;

  for (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;
      
      z = (*p_arg)->size;

      ᮚᮦᮔ᮪ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = parimaa_na(void*);
	  *(void **) argp = *p_argv;
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
      ᮜᮤᮚ ᮚᮦᮔ᮪ (z < parimaa_na(puur_naa_nka))
	{
	  z = parimaa_na(puur_naa_nka);
	  switch ((*p_arg)->type)
	    {
	    pak_sa FFI_TYPE_SINT8:
	      *(signed puur_naa_nka *) argp = (signed puur_naa_nka)*(SINT8 *)(* p_argv);
	      bha_nga;
	      
	    pak_sa FFI_TYPE_UINT8:
	      *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT8 *)(* p_argv);
	      bha_nga;
	      
	    pak_sa FFI_TYPE_SINT16:
	      *(signed puur_naa_nka *) argp = (signed puur_naa_nka)*(SINT16 *)(* p_argv);
	      bha_nga;
		  
	    pak_sa FFI_TYPE_UINT16:
	      *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT16 *)(* p_argv);
	      bha_nga;
		  
	    default:
	      FFI_ASSERT(0);
	    }
	}
      ᮜᮤᮚ ᮚᮦᮔ᮪ (z == parimaa_na(puur_naa_nka))
	{
	  *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT32 *)(* p_argv);
	}
      ᮜᮤᮚ
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      count += z;
    }

  ᮝᮜᮤ (stack + ((count > 24) ? 24 : FFI_ALIGN_DOWN(count, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ᮚᮦᮔ᮪ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ᮜᮤᮚ
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ᮝᮜᮤ FFI_OK;
}

extern void ffi_call_EABI(void *(*)(var_na *, extended_cif *), 
			  extended_cif *, 
			  unsigned, unsigned, 
			  unsigned *, 
			  void (*fn)(void));

void ffi_call(ffi_cif *cif, 
	      void (*fn)(void), 
	      void *rvalue, 
	      void **avalue)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ᮚᮦᮔ᮪ ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ᮜᮤᮚ
    ecif.rvalue = rvalue;
    
  
  switch (cif->abi) 
    {
    pak_sa FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes, 
		    cif->flags, ecif.rvalue, fn);
      bha_nga;
    default:
      FFI_ASSERT(0);
      bha_nga;
    }
}

void ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in gr7.  We must save this
     pointer in a place that will persist while we do our work.  */
  pa_njikaa ffi_closure *creg __asm__ ("gr7");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  pa_njikaa var_na *frame_pointer __asm__ ("fp");
  var_na *stack_args = frame_pointer + 16;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue = alloca (cif->nargs * parimaa_na(void *));
  var_na *ptr = (var_na *) register_args;
  puur_naa_nka i;

  /* Find the address of each argument.  */
  for (i = 0; i < cif->nargs; i++)
    {
      switch (arg_types[i]->type)
	{
	pak_sa FFI_TYPE_SINT8:
	pak_sa FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  bha_nga;
	pak_sa FFI_TYPE_SINT16:
	pak_sa FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  bha_nga;
	pak_sa FFI_TYPE_SINT32:
	pak_sa FFI_TYPE_UINT32:
	pak_sa FFI_TYPE_FLOAT:
	  avalue[i] = ptr;
	  bha_nga;
	pak_sa FFI_TYPE_STRUCT:
	  avalue[i] = *(void**)ptr;
	  bha_nga;
	default:
	  /* This is an 8-byte value.  */
	  avalue[i] = ptr;
	  ptr += 4;
	  bha_nga;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ᮚᮦᮔ᮪ (ptr == ((var_na *)register_args + (6*4)))
	ptr = stack_args;
    }

  /* Invoke the closure.  */
  ᮚᮦᮔ᮪ (cif->rtype->type == FFI_TYPE_STRUCT)
    {
      /* The caller allocates space for the return structure, and
       passes a pointer to this space in gr3.  Use this value directly
       as the return value.  */
      pa_njikaa void *return_struct_ptr __asm__("gr3");
      (closure->fun) (cif, return_struct_ptr, avalue, closure->user_data);
    }
  ᮜᮤᮚ
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);

      /* Functions return 4-byte or smaller results in gr8.  8-byte
	 values also use gr9.  We fill the both, even for small return
	 values, just to avoid a branch.  */ 
      __asm__ ("ldi  @(%0, #0), gr8" : : "r" (&rvalue));
      __asm__ ("ldi  @(%0, #0), gr9" : : "r" (&((puur_naa_nka *) &rvalue)[1]));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned puur_naa_nka *tramp = (unsigned puur_naa_nka *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;
#ifdef __FRV_FDPIC__
  pa_njikaa void *got __asm__("gr15");
#endif
  puur_naa_nka i;

  fn = (unsigned long) ffi_closure_eabi;

#ifdef __FRV_FDPIC__
  tramp[0] = &((unsigned puur_naa_nka *)codeloc)[2];
  tramp[1] = got;
  tramp[2] = 0x8cfc0000 + (fn  & 0xffff); /* setlos lo(fn), gr6    */
  tramp[3] = 0x8efc0000 + (cls & 0xffff); /* setlos lo(cls), gr7   */
  tramp[4] = 0x8cf80000 + (fn  >> 16);	  /* sethi hi(fn), gr6     */
  tramp[5] = 0x8ef80000 + (cls >> 16);    /* sethi hi(cls), gr7    */
  tramp[6] = 0x9cc86000;                  /* ldi @(gr6, #0), gr14  */
  tramp[7] = 0x8030e000;                  /* jmpl @(gr14, gr0)     */
#ᮜᮤᮚ
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
  for (i = 0; i < FFI_TRAMPOLINE_SIZE; i++)
    __asm__ ca_ncala ("dcf @(%0,%1)\n\tici @(%2,%1)" :: "r" (tramp), "r" (i),
		      "r" (codeloc));

  ᮝᮜᮤ FFI_OK;
}

/* -----------------------------------------------------------------------
   ffi.c - Copyright (C) 2012, 2013, 2018, 2021, 2022, 2026  Anthony Green

   Moxie Foreign Function Interface

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

  જો (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(શૂન્ય **) argp = ecif->rvalue;
      argp += 4;
    }

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
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue_copy = ખાલી;
  પૂર્ણાંક i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  જો ((rvalue == ખાલી) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  નહીંતર
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  માટે (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      પૂર્ણાંક size = at->size;
      જો (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          અક્ષર *argcopy = alloca (size);
          જો (avalue_copy == ખાલી)
            {
              avalue_copy = alloca (nargs * કદ (શૂન્ય *));
              memcpy (avalue_copy, avalue, nargs * કદ (શૂન્ય *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  નોંધણી ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  નોંધણી અક્ષર *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  શૂન્ય *struct_rvalue = (શૂન્ય *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  અક્ષર *ઢગલો_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  અક્ષર *register_args_ptr = (અક્ષર *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  શૂન્ય **avalue = alloca (cif->nargs * કદ(શૂન્ય *));
  અક્ષર *ptr = (અક્ષર *) register_args;
  પૂર્ણાંક i;

  /* preserve struct type return pointer passing */
  જો ((cif->rtype != ખાલી) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (અક્ષર *)&register_args[1];
  }

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
	સ્થિતિ FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  તોડો;
	સ્થિતિ FFI_TYPE_STRUCT:
          {
            જો (arg_types[i]->size > 4)
              {
                શૂન્ય *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(શૂન્ય**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            નહીંતર
              avalue[i] = *(શૂન્ય**)ptr;
          }
	  તોડો;
	મૂળભૂત:
	  /* This is an 8-byte value.  */
	  જો (ptr == (અક્ષર *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) ઢગલો_args;
	    }
	  નહીંતર
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  તોડો;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      જો (ptr == (અક્ષર *) &register_args[6])
	ptr = ઢગલો_args;
      નહીંતર જો (ptr == (અક્ષર *) &register_args[7])
	ptr = ઢગલો_args + 4;
    }

  /* Invoke the closure.  */
  જો (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  નહીંતર
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_data);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  જો (cif->abi != FFI_EABI)
    પાછા FFI_BAD_ABI;

  fn = (unsigned long) ffi_closure_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  પાછા FFI_OK;
}

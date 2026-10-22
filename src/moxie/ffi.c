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

  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ശൂന്യം **) argp = ecif->rvalue;
      argp += 4;
    }

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
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue_copy = ശൂന്യ;
  പൂർണ്ണസംഖ്യ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  എങ്കിൽ ((rvalue == ശൂന്യ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  വേണ്ടി (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      പൂർണ്ണസംഖ്യ size = at->size;
      എങ്കിൽ (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          അക്ഷരം *argcopy = alloca (size);
          എങ്കിൽ (avalue_copy == ശൂന്യ)
            {
              avalue_copy = alloca (nargs * വലുപ്പം (ശൂന്യം *));
              memcpy (avalue_copy, avalue, nargs * വലുപ്പം (ശൂന്യം *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  രജിസ്റ്റർ ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  രജിസ്റ്റർ അക്ഷരം *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  ശൂന്യം *struct_rvalue = (ശൂന്യം *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  അക്ഷരം *അട്ടി_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  അക്ഷരം *register_args_ptr = (അക്ഷരം *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ശൂന്യം **avalue = alloca (cif->nargs * വലുപ്പം(ശൂന്യം *));
  അക്ഷരം *ptr = (അക്ഷരം *) register_args;
  പൂർണ്ണസംഖ്യ i;

  /* preserve struct type return pointer passing */
  എങ്കിൽ ((cif->rtype != ശൂന്യ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (അക്ഷരം *)&register_args[1];
  }

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
	അവസ്ഥ FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  നിർത്തുക;
	അവസ്ഥ FFI_TYPE_STRUCT:
          {
            എങ്കിൽ (arg_types[i]->size > 4)
              {
                ശൂന്യം *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(ശൂന്യം**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            അല്ലെങ്കിൽ
              avalue[i] = *(ശൂന്യം**)ptr;
          }
	  നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
	  /* This is an 8-byte value.  */
	  എങ്കിൽ (ptr == (അക്ഷരം *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) അട്ടി_args;
	    }
	  അല്ലെങ്കിൽ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  നിർത്തുക;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      എങ്കിൽ (ptr == (അക്ഷരം *) &register_args[6])
	ptr = അട്ടി_args;
      അല്ലെങ്കിൽ എങ്കിൽ (ptr == (അക്ഷരം *) &register_args[7])
	ptr = അട്ടി_args + 4;
    }

  /* Invoke the closure.  */
  എങ്കിൽ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  അല്ലെങ്കിൽ
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
		      ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  എങ്കിൽ (cif->abi != FFI_EABI)
    തിരികെ FFI_BAD_ABI;

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

  തിരികെ FFI_OK;
}

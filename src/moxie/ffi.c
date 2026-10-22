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

  எனில் (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(வெறுமை **) argp = ecif->rvalue;
      argp += 4;
    }

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
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue_copy = வெற்று;
  முழு i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  எனில் ((rvalue == வெற்று) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  இல்லையெனில்
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  க்கு (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      முழு size = at->size;
      எனில் (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          எழுத்து *argcopy = alloca (size);
          எனில் (avalue_copy == வெற்று)
            {
              avalue_copy = alloca (nargs * அளவு (வெறுமை *));
              memcpy (avalue_copy, avalue, nargs * அளவு (வெறுமை *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  பதிவு ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  பதிவு எழுத்து *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  வெறுமை *struct_rvalue = (வெறுமை *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  எழுத்து *அடுக்கு_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  எழுத்து *register_args_ptr = (எழுத்து *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  வெறுமை **avalue = alloca (cif->nargs * அளவு(வெறுமை *));
  எழுத்து *ptr = (எழுத்து *) register_args;
  முழு i;

  /* preserve struct type return pointer passing */
  எனில் ((cif->rtype != வெற்று) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (எழுத்து *)&register_args[1];
  }

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
	நிலை FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  நிறுத்து;
	நிலை FFI_TYPE_STRUCT:
          {
            எனில் (arg_types[i]->size > 4)
              {
                வெறுமை *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(வெறுமை**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            இல்லையெனில்
              avalue[i] = *(வெறுமை**)ptr;
          }
	  நிறுத்து;
	இயல்பு:
	  /* This is an 8-byte value.  */
	  எனில் (ptr == (எழுத்து *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) அடுக்கு_args;
	    }
	  இல்லையெனில்
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  நிறுத்து;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      எனில் (ptr == (எழுத்து *) &register_args[6])
	ptr = அடுக்கு_args;
      இல்லையெனில் எனில் (ptr == (எழுத்து *) &register_args[7])
	ptr = அடுக்கு_args + 4;
    }

  /* Invoke the closure.  */
  எனில் (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  இல்லையெனில்
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
		      வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  எனில் (cif->abi != FFI_EABI)
    திரும்பு FFI_BAD_ABI;

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

  திரும்பு FFI_OK;
}

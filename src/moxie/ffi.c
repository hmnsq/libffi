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

  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ಶೂನ್ಯ **) argp = ecif->rvalue;
      argp += 4;
    }

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
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue_copy = ರಿಕ್ತ;
  ಪೂರ್ಣಾಙ್ಕ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ಯದಿ ((rvalue == ರಿಕ್ತ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ಪ್ರತಿ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ಪೂರ್ಣಾಙ್ಕ size = at->size;
      ಯದಿ (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          ವರ್ಣ *argcopy = alloca (size);
          ಯದಿ (avalue_copy == ರಿಕ್ತ)
            {
              avalue_copy = alloca (nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
              memcpy (avalue_copy, avalue, nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  ಪಞ್ಜಿಕಾ ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ಪಞ್ಜಿಕಾ ವರ್ಣ *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  ಶೂನ್ಯ *struct_rvalue = (ಶೂನ್ಯ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  ವರ್ಣ *ರಾಶಿ_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  ವರ್ಣ *register_args_ptr = (ವರ್ಣ *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ಶೂನ್ಯ **avalue = alloca (cif->nargs * ಪರಿಮಾಣ(ಶೂನ್ಯ *));
  ವರ್ಣ *ptr = (ವರ್ಣ *) register_args;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* preserve struct type return pointer passing */
  ಯದಿ ((cif->rtype != ರಿಕ್ತ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (ವರ್ಣ *)&register_args[1];
  }

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
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  ಭಙ್ಗ;
	ಪಕ್ಷ FFI_TYPE_STRUCT:
          {
            ಯದಿ (arg_types[i]->size > 4)
              {
                ಶೂನ್ಯ *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(ಶೂನ್ಯ**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            ಅನ್ಯಥಾ
              avalue[i] = *(ಶೂನ್ಯ**)ptr;
          }
	  ಭಙ್ಗ;
	ಸ್ವಭಾವ:
	  /* This is an 8-byte value.  */
	  ಯದಿ (ptr == (ವರ್ಣ *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) ರಾಶಿ_args;
	    }
	  ಅನ್ಯಥಾ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  ಭಙ್ಗ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ಯದಿ (ptr == (ವರ್ಣ *) &register_args[6])
	ptr = ರಾಶಿ_args;
      ಅನ್ಯಥಾ ಯದಿ (ptr == (ವರ್ಣ *) &register_args[7])
	ptr = ರಾಶಿ_args + 4;
    }

  /* Invoke the closure.  */
  ಯದಿ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  ಅನ್ಯಥಾ
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
		      ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  ಯದಿ (cif->abi != FFI_EABI)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

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

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

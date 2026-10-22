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

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(تهی **) argp = ecif->rvalue;
      argp += 4;
    }

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
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue_copy = پوچ;
  صحیح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  اگر ((rvalue == پوچ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  وگرنه
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برای (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح size = at->size;
      اگر (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          نویسه *argcopy = alloca (size);
          اگر (avalue_copy == پوچ)
            {
              avalue_copy = alloca (nargs * اندازه (تهی *));
              memcpy (avalue_copy, avalue, nargs * اندازه (تهی *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  ثبات ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ثبات نویسه *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  تهی *struct_rvalue = (تهی *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  نویسه *پشته_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  نویسه *register_args_ptr = (نویسه *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  تهی **avalue = alloca (cif->nargs * اندازه(تهی *));
  نویسه *ptr = (نویسه *) register_args;
  صحیح i;

  /* preserve struct type return pointer passing */
  اگر ((cif->rtype != پوچ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (نویسه *)&register_args[1];
  }

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
	حالت FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  بشکن;
	حالت FFI_TYPE_STRUCT:
          {
            اگر (arg_types[i]->size > 4)
              {
                تهی *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(تهی**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            وگرنه
              avalue[i] = *(تهی**)ptr;
          }
	  بشکن;
	پیشفرض:
	  /* This is an 8-byte value.  */
	  اگر (ptr == (نویسه *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) پشته_args;
	    }
	  وگرنه
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  بشکن;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      اگر (ptr == (نویسه *) &register_args[6])
	ptr = پشته_args;
      وگرنه اگر (ptr == (نویسه *) &register_args[7])
	ptr = پشته_args + 4;
    }

  /* Invoke the closure.  */
  اگر (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  وگرنه
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
		      تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  اگر (cif->abi != FFI_EABI)
    بازگشت FFI_BAD_ABI;

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

  بازگشت FFI_OK;
}

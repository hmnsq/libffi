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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

शून्य *ffi_prep_args(वर्ण *ढेर, extended_cif *ecif)
{
  पञ्जिका unsigned पूर्णाङ्क i;
  पञ्जिका शून्य **p_argv;
  पञ्जिका वर्ण *argp;
  पञ्जिका ffi_type **p_arg;
  पञ्जिका पूर्णाङ्क गिनती = 0;

  p_argv = ecif->avalue;
  argp = ढेर;

  यदि (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
    }

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;

      यदि ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = परिमाण(शून्य*);
	  *(शून्य **) argp = *p_argv;
	}
      अन्यथा यदि (z < परिमाण(पूर्णाङ्क))
	{
	  z = परिमाण(पूर्णाङ्क);
	  विकल्प ((*p_arg)->type)
	    {
	    पक्ष FFI_TYPE_SINT8:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT8 *)(* p_argv);
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT8:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT8 *)(* p_argv);
	      भङ्ग;

	    पक्ष FFI_TYPE_SINT16:
	      *(signed पूर्णाङ्क *) argp = (signed पूर्णाङ्क)*(SINT16 *)(* p_argv);
	      भङ्ग;

	    पक्ष FFI_TYPE_UINT16:
	      *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT16 *)(* p_argv);
	      भङ्ग;

	    स्वभाव:
	      FFI_ASSERT(0);
	    }
	}
      अन्यथा यदि (z == परिमाण(पूर्णाङ्क))
	{
	  *(unsigned पूर्णाङ्क *) argp = (unsigned पूर्णाङ्क)*(UINT32 *)(* p_argv);
	}
      अन्यथा
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      गिनती += z;
    }

  प्रत्यागम (ढेर + ((गिनती > 24) ? 24 : FFI_ALIGN_DOWN(गिनती, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  यदि (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  अन्यथा
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_EABI(शून्य *(*)(वर्ण *, extended_cif *),
			  extended_cif *,
			  unsigned, unsigned,
			  unsigned *,
			  शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif,
	      शून्य (*fn)(शून्य),
	      शून्य *rvalue,
	      शून्य **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णाङ्क i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  यदि ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क size = at->size;
      यदि (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          वर्ण *argcopy = alloca (size);
          यदि (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              memcpy (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      भङ्ग;
    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

शून्य ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  पञ्जिका ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  पञ्जिका वर्ण *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  शून्य *struct_rvalue = (शून्य *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  वर्ण *ढेर_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  वर्ण *register_args_ptr = (वर्ण *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue = alloca (cif->nargs * परिमाण(शून्य *));
  वर्ण *ptr = (वर्ण *) register_args;
  पूर्णाङ्क i;

  /* preserve struct type return pointer passing */
  यदि ((cif->rtype != रिक्त) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (वर्ण *)&register_args[1];
  }

  /* Find the address of each argument.  */
  प्रति (i = 0; i < cif->nargs; i++)
    {
      विकल्प (arg_types[i]->type)
	{
	पक्ष FFI_TYPE_SINT8:
	पक्ष FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT16:
	पक्ष FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  भङ्ग;
	पक्ष FFI_TYPE_SINT32:
	पक्ष FFI_TYPE_UINT32:
	पक्ष FFI_TYPE_FLOAT:
	पक्ष FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  भङ्ग;
	पक्ष FFI_TYPE_STRUCT:
          {
            यदि (arg_types[i]->size > 4)
              {
                शून्य *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(शून्य**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            अन्यथा
              avalue[i] = *(शून्य**)ptr;
          }
	  भङ्ग;
	स्वभाव:
	  /* This is an 8-byte value.  */
	  यदि (ptr == (वर्ण *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) ढेर_args;
	    }
	  अन्यथा
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  भङ्ग;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      यदि (ptr == (वर्ण *) &register_args[6])
	ptr = ढेर_args;
      अन्यथा यदि (ptr == (वर्ण *) &register_args[7])
	ptr = ढेर_args + 4;
    }

  /* Invoke the closure.  */
  यदि (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  अन्यथा
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
		      शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  यदि (cif->abi != FFI_EABI)
    प्रत्यागम FFI_BAD_ABI;

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

  प्रत्यागम FFI_OK;
}

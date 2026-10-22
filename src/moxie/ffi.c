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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ଶୂନ୍ୟ *ffi_prep_args(ଅକ୍ଷର *ଗଦା, extended_cif *ecif)
{
  ପଞ୍ଜିକରଣ unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଞ୍ଜିକରଣ ଶୂନ୍ୟ **p_argv;
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *argp;
  ପଞ୍ଜିକରଣ ffi_type **p_arg;
  ପଞ୍ଜିକରଣ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2 = 0;

  p_argv = ecif->avalue;
  argp = ଗଦା;

  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ଶୂନ୍ୟ **) argp = ecif->rvalue;
      argp += 4;
    }

  ପାଇଁ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;

      ଯଦି ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ଆକାର(ଶୂନ୍ୟ*);
	  *(ଶୂନ୍ୟ **) argp = *p_argv;
	}
      ଅନ୍ୟଥା ଯଦି (z < ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  z = ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା);
	  ବାଛ ((*p_arg)->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_SINT8:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT8:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT8 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_SINT16:
	      *(signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (signed ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(SINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ସ୍ଥିତି FFI_TYPE_UINT16:
	      *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT16 *)(* p_argv);
	      ଭାଙ୍ଗ;

	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      FFI_ASSERT(0);
	    }
	}
      ଅନ୍ୟଥା ଯଦି (z == ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା))
	{
	  *(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) argp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*(UINT32 *)(* p_argv);
	}
      ଅନ୍ୟଥା
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ଗଣନା_2 += z;
    }

  ଫେରନ୍ତୁ (ଗଦା + ((ଗଣନା_2 > 24) ? 24 : FFI_ALIGN_DOWN(ଗଣନା_2, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ଅନ୍ୟଥା
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ଫେରନ୍ତୁ FFI_OK;
}

ବାହ୍ୟ ଶୂନ୍ୟ ffi_call_EABI(ଶୂନ୍ୟ *(*)(ଅକ୍ଷର *, extended_cif *),
			  extended_cif *,
			  unsigned, unsigned,
			  unsigned *,
			  ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ));

ଶୂନ୍ୟ ffi_call(ffi_cif *cif,
	      ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ),
	      ଶୂନ୍ୟ *rvalue,
	      ଶୂନ୍ୟ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue_copy = ଖାଲି;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ଯଦି ((rvalue == ଖାଲି) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ପାଇଁ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = at->size;
      ଯଦି (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          ଅକ୍ଷର *argcopy = alloca (size);
          ଯଦି (avalue_copy == ଖାଲି)
            {
              avalue_copy = alloca (nargs * ଆକାର (ଶୂନ୍ୟ *));
              memcpy (avalue_copy, avalue, nargs * ଆକାର (ଶୂନ୍ୟ *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ବାଛ (cif->abi)
    {
    ସ୍ଥିତି FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  ପଞ୍ଜିକରଣ ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ପଞ୍ଜିକରଣ ଅକ୍ଷର *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  ଶୂନ୍ୟ *struct_rvalue = (ଶୂନ୍ୟ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  ଅକ୍ଷର *ଗଦା_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  ଅକ୍ଷର *register_args_ptr = (ଅକ୍ଷର *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  ଶୂନ୍ୟ **avalue = alloca (cif->nargs * ଆକାର(ଶୂନ୍ୟ *));
  ଅକ୍ଷର *ptr = (ଅକ୍ଷର *) register_args;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

  /* preserve struct type return pointer passing */
  ଯଦି ((cif->rtype != ଖାଲି) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (ଅକ୍ଷର *)&register_args[1];
  }

  /* Find the address of each argument.  */
  ପାଇଁ (i = 0; i < cif->nargs; i++)
    {
      ବାଛ (arg_types[i]->type)
	{
	ସ୍ଥିତି FFI_TYPE_SINT8:
	ସ୍ଥିତି FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT16:
	ସ୍ଥିତି FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_SINT32:
	ସ୍ଥିତି FFI_TYPE_UINT32:
	ସ୍ଥିତି FFI_TYPE_FLOAT:
	ସ୍ଥିତି FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  ଭାଙ୍ଗ;
	ସ୍ଥିତି FFI_TYPE_STRUCT:
          {
            ଯଦି (arg_types[i]->size > 4)
              {
                ଶୂନ୍ୟ *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(ଶୂନ୍ୟ**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            ଅନ୍ୟଥା
              avalue[i] = *(ଶୂନ୍ୟ**)ptr;
          }
	  ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	  /* This is an 8-byte value.  */
	  ଯଦି (ptr == (ଅକ୍ଷର *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) ଗଦା_args;
	    }
	  ଅନ୍ୟଥା
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  ଭାଙ୍ଗ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ଯଦି (ptr == (ଅକ୍ଷର *) &register_args[6])
	ptr = ଗଦା_args;
      ଅନ୍ୟଥା ଯଦି (ptr == (ଅକ୍ଷର *) &register_args[7])
	ptr = ଗଦା_args + 4;
    }

  /* Invoke the closure.  */
  ଯଦି (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  ଅନ୍ୟଥା
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
		      ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  ଯଦି (cif->abi != FFI_EABI)
    ଫେରନ୍ତୁ FFI_BAD_ABI;

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

  ଫେରନ୍ତୁ FFI_OK;
}

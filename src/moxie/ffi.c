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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

শূন্য *ffi_prep_args(অক্ষর *স্তূপ, extended_cif *ecif)
{
  নিবন্ধ unsigned পূর্ণসংখ্যা i;
  নিবন্ধ শূন্য **p_argv;
  নিবন্ধ অক্ষর *argp;
  নিবন্ধ ffi_type **p_arg;
  নিবন্ধ পূর্ণসংখ্যা গুনতি = 0;

  p_argv = ecif->avalue;
  argp = স্তূপ;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(শূন্য **) argp = ecif->rvalue;
      argp += 4;
    }

  জন্য (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->আকার_2;

      যদি ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = আকার(শূন্য*);
	  *(শূন্য **) argp = *p_argv;
	}
      অন্যথা যদি (z < আকার(পূর্ণসংখ্যা))
	{
	  z = আকার(পূর্ণসংখ্যা);
	  নির্বাচন ((*p_arg)->type)
	    {
	    ক্ষেত্র FFI_TYPE_SINT8:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT8 *)(* p_argv);
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT8:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(* p_argv);
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_SINT16:
	      *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT16 *)(* p_argv);
	      বিরতি;

	    ক্ষেত্র FFI_TYPE_UINT16:
	      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(* p_argv);
	      বিরতি;

	    পূর্বনির্ধারিত:
	      FFI_ASSERT(0);
	    }
	}
      অন্যথা যদি (z == আকার(পূর্ণসংখ্যা))
	{
	  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	}
      অন্যথা
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      গুনতি += z;
    }

  ফেরত (স্তূপ + ((গুনতি > 24) ? 24 : FFI_ALIGN_DOWN(গুনতি, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  যদি (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  অন্যথা
    cif->flags = cif->rtype->আকার_2;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_call_EABI(শূন্য *(*)(অক্ষর *, extended_cif *),
			  extended_cif *,
			  unsigned, unsigned,
			  unsigned *,
			  শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif,
	      শূন্য (*fn)(শূন্য),
	      শূন্য *rvalue,
	      শূন্য **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;
  পূর্ণসংখ্যা i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  যদি ((rvalue == ফাঁকা) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->আকার_2);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      যদি (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          অক্ষর *argcopy = alloca (আকার_2);
          যদি (avalue_copy == ফাঁকা)
            {
              avalue_copy = alloca (nargs * আকার (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], আকার_2);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_EABI:
      ffi_call_EABI(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      বিরতি;
    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }
}

শূন্য ffi_closure_eabi (unsigned arg1, unsigned arg2, unsigned arg3,
		       unsigned arg4, unsigned arg5, unsigned arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  নিবন্ধ ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  নিবন্ধ অক্ষর *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  শূন্য *struct_rvalue = (শূন্য *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  অক্ষর *স্তূপ_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  অক্ষর *register_args_ptr = (অক্ষর *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue = alloca (cif->nargs * আকার(শূন্য *));
  অক্ষর *ptr = (অক্ষর *) register_args;
  পূর্ণসংখ্যা i;

  /* preserve struct type return pointer passing */
  যদি ((cif->rtype != ফাঁকা) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (অক্ষর *)&register_args[1];
  }

  /* Find the address of each argument.  */
  জন্য (i = 0; i < cif->nargs; i++)
    {
      নির্বাচন (arg_types[i]->type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	ক্ষেত্র FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT16:
	ক্ষেত্র FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_SINT32:
	ক্ষেত্র FFI_TYPE_UINT32:
	ক্ষেত্র FFI_TYPE_FLOAT:
	ক্ষেত্র FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  বিরতি;
	ক্ষেত্র FFI_TYPE_STRUCT:
          {
            যদি (arg_types[i]->আকার_2 > 4)
              {
                শূন্য *copy = alloca(arg_types[i]->আকার_2);
                memcpy(copy, *(শূন্য**)ptr, arg_types[i]->আকার_2);
                avalue[i] = copy;
              }
            অন্যথা
              avalue[i] = *(শূন্য**)ptr;
          }
	  বিরতি;
	পূর্বনির্ধারিত:
	  /* This is an 8-byte value.  */
	  যদি (ptr == (অক্ষর *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) স্তূপ_args;
	    }
	  অন্যথা
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  বিরতি;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      যদি (ptr == (অক্ষর *) &register_args[6])
	ptr = স্তূপ_args;
      অন্যথা যদি (ptr == (অক্ষর *) &register_args[7])
	ptr = স্তূপ_args + 4;
    }

  /* Invoke the closure.  */
  যদি (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_তথ্য);
    }
  অন্যথা
    {
      /* Allocate space for the return value and call the function.  */
      long long rvalue;
      (closure->fun) (cif, &rvalue, avalue, closure->user_তথ্য);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_status
ffi_prep_closure_loc (ffi_closure* closure,
		      ffi_cif* cif,
		      শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  যদি (cif->abi != FFI_EABI)
    ফেরত FFI_BAD_ABI;

  fn = (unsigned long) ffi_closure_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

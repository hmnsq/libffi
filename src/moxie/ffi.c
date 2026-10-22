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

  ပေႃး (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
    }

  for (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->size;

      ပေႃး ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = parimaa_na(void*);
	  *(void **) argp = *p_argv;
	}
      ပေႃးဢမ်ႇ ပေႃး (z < parimaa_na(puur_naa_nka))
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
      ပေႃးဢမ်ႇ ပေႃး (z == parimaa_na(puur_naa_nka))
	{
	  *(unsigned puur_naa_nka *) argp = (unsigned puur_naa_nka)*(UINT32 *)(* p_argv);
	}
      ပေႃးဢမ်ႇ
	{
	  memcpy(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      count += z;
    }

  ပွၵ်ႈ (stack + ((count > 24) ? 24 : FFI_ALIGN_DOWN(count, 8)));
}

/* Perform machine dependent cif processing */
ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
  ပေႃး (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ပေႃးဢမ်ႇ
    cif->flags = cif->rtype->size;

  cif->bytes = FFI_ALIGN (cif->bytes, 8);

  ပွၵ်ႈ FFI_OK;
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
  ffi_type **arg_types = cif->arg_types;
  void **avalue_copy = NULL;
  puur_naa_nka i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ပေႃး ((rvalue == NULL) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->size);
    }
  ပေႃးဢမ်ႇ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  for (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      puur_naa_nka size = at->size;
      ပေႃး (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          var_na *argcopy = alloca (size);
          ပေႃး (avalue_copy == NULL)
            {
              avalue_copy = alloca (nargs * parimaa_na (void *));
              memcpy (avalue_copy, avalue, nargs * parimaa_na (void *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

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
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  pa_njikaa ffi_closure *creg __asm__ ("$r12");
  ffi_closure *closure = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  pa_njikaa var_na *frame_pointer __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  void *struct_rvalue = (void *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  var_na *stack_args = frame_pointer + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  unsigned register_args[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  var_na *register_args_ptr = (var_na *) register_args;

  ffi_cif *cif = closure->cif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue = alloca (cif->nargs * parimaa_na(void *));
  var_na *ptr = (var_na *) register_args;
  puur_naa_nka i;

  /* preserve struct type return pointer passing */
  ပေႃး ((cif->rtype != NULL) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    register_args_ptr = (var_na *)&register_args[1];
  }

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
	pak_sa FFI_TYPE_POINTER:
	  avalue[i] = ptr;
	  bha_nga;
	pak_sa FFI_TYPE_STRUCT:
          {
            ပေႃး (arg_types[i]->size > 4)
              {
                void *copy = alloca(arg_types[i]->size);
                memcpy(copy, *(void**)ptr, arg_types[i]->size);
                avalue[i] = copy;
              }
            ပေႃးဢမ်ႇ
              avalue[i] = *(void**)ptr;
          }
	  bha_nga;
	default:
	  /* This is an 8-byte value.  */
	  ပေႃး (ptr == (var_na *) &register_args[5])
	    {
	      /* The value is split across two locations */
	      unsigned *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(unsigned *) ptr;
	      ip[1] = *(unsigned *) stack_args;
	    }
	  ပေႃးဢမ်ႇ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  bha_nga;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ပေႃး (ptr == (var_na *) &register_args[6])
	ptr = stack_args;
      ပေႃးဢမ်ႇ ပေႃး (ptr == (var_na *) &register_args[7])
	ptr = stack_args + 4;
    }

  /* Invoke the closure.  */
  ပေႃး (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (closure->fun) (cif, struct_rvalue, avalue, closure->user_data);
    }
  ပေႃးဢမ်ႇ
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
		      void (*fun)(ffi_cif*, void*, void**, void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned short *tramp = (unsigned short *) &closure->tramp[0];
  unsigned long fn = (long) ffi_closure_eabi;
  unsigned long cls = (long) codeloc;

  ပေႃး (cif->abi != FFI_EABI)
    ပွၵ်ႈ FFI_BAD_ABI;

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

  ပွၵ်ႈ FFI_OK;
}

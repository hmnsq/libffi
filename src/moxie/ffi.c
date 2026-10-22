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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਖਾਲੀ *ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, extended_cif *ecif)
{
  ਰਜਿਸਟਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i;
  ਰਜਿਸਟਰ ਖਾਲੀ **p_argv;
  ਰਜਿਸਟਰ ਅੱਖਰ *argp;
  ਰਜਿਸਟਰ ffi_type **p_arg;
  ਰਜਿਸਟਰ ਪੂਰਨ ਗਿਣਤੀ_2 = 0;

  p_argv = ecif->avalue;
  argp = ਢੇਰ;

  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(ਖਾਲੀ **) argp = ecif->rvalue;
      argp += 4;
    }

  ਲਈ (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ਮਾਪ;

      ਜੇ ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = ਆਕਾਰ(ਖਾਲੀ*);
	  *(ਖਾਲੀ **) argp = *p_argv;
	}
      ਨਹੀਂਤਾਂ ਜੇ (z < ਆਕਾਰ(ਪੂਰਨ))
	{
	  z = ਆਕਾਰ(ਪੂਰਨ);
	  ਚੋਣ ((*p_arg)->type)
	    {
	    ਹਾਲਤ FFI_TYPE_SINT8:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT8 *)(* p_argv);
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT8:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT8 *)(* p_argv);
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_SINT16:
	      *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ *) argp = (ਨਿਸ਼ਾਨੀਵਾਲਾ ਪੂਰਨ)*(SINT16 *)(* p_argv);
	      ਰੋਕੋ;

	    ਹਾਲਤ FFI_TYPE_UINT16:
	      *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT16 *)(* p_argv);
	      ਰੋਕੋ;

	    ਮੂਲ:
	      FFI_ASSERT(0);
	    }
	}
      ਨਹੀਂਤਾਂ ਜੇ (z == ਆਕਾਰ(ਪੂਰਨ))
	{
	  *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)*(UINT32 *)(* p_argv);
	}
      ਨਹੀਂਤਾਂ
	{
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ਗਿਣਤੀ_2 += z;
    }

  ਵਾਪਸ (ਢੇਰ + ((ਗਿਣਤੀ_2 > 24) ? 24 : FFI_ALIGN_ਥੱਲੇ(ਗਿਣਤੀ_2, 8)));
}

/* Perform machine dependent cif processing */
ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
  ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ਨਹੀਂਤਾਂ
    cif->flags = cif->rtype->ਮਾਪ;

  cif->ਬਾਈਟਾਂ = FFI_ALIGN (cif->ਬਾਈਟਾਂ, 8);

  ਵਾਪਸ FFI_OK;
}

ਬਾਹਰੀ ਖਾਲੀ ffi_call_EABI(ਖਾਲੀ *(*)(ਅੱਖਰ *, extended_cif *),
			  extended_cif *,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ,
			  ਬਿਨਾਨਿਸ਼ਾਨੀ *,
			  ਖਾਲੀ (*fn)(ਖਾਲੀ));

ਖਾਲੀ ffi_call(ffi_cif *cif,
	      ਖਾਲੀ (*fn)(ਖਾਲੀ),
	      ਖਾਲੀ *rvalue,
	      ਖਾਲੀ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue_copy = ਨਿਰਾ;
  ਪੂਰਨ i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  ਜੇ ((rvalue == ਨਿਰਾ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
    }
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  ਲਈ (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      ਪੂਰਨ ਮਾਪ = at->ਮਾਪ;
      ਜੇ (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          ਅੱਖਰ *argcopy = alloca (ਮਾਪ);
          ਜੇ (avalue_copy == ਨਿਰਾ)
            {
              avalue_copy = alloca (nargs * ਆਕਾਰ (ਖਾਲੀ *));
              ਸਮ੍ਰਿਤੀ_ਨਕਲ (avalue_copy, avalue, nargs * ਆਕਾਰ (ਖਾਲੀ *));
              avalue = avalue_copy;
            }
          ਸਮ੍ਰਿਤੀ_ਨਕਲ (argcopy, avalue[i], ਮਾਪ);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ਚੋਣ (cif->abi)
    {
    ਹਾਲਤ FFI_EABI:
      ffi_call_EABI(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ,
		    cif->flags, ecif.rvalue, fn);
      ਰੋਕੋ;
    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}

ਖਾਲੀ ffi_ਘੇਰਾ_eabi (ਬਿਨਾਨਿਸ਼ਾਨੀ arg1, ਬਿਨਾਨਿਸ਼ਾਨੀ arg2, ਬਿਨਾਨਿਸ਼ਾਨੀ arg3,
		       ਬਿਨਾਨਿਸ਼ਾਨੀ arg4, ਬਿਨਾਨਿਸ਼ਾਨੀ arg5, ਬਿਨਾਨਿਸ਼ਾਨੀ arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  ਰਜਿਸਟਰ ffi_ਘੇਰਾ *creg __asm__ ("$r12");
  ffi_ਘੇਰਾ *ਘੇਰਾ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  ਰਜਿਸਟਰ ਅੱਖਰ *ਚੌਖਟਾ_ਸੂਚਕ __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  ਖਾਲੀ *struct_rvalue = (ਖਾਲੀ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  ਅੱਖਰ *ਢੇਰ_ਦਲੀਲਾਂ = ਚੌਖਟਾ_ਸੂਚਕ + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  ਅੱਖਰ *ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_ptr = (ਅੱਖਰ *) ਰਜਿਸਟਰ_ਦਲੀਲਾਂ;

  ffi_cif *cif = ਘੇਰਾ->cif;
  ffi_type **arg_types = cif->arg_types;
  ਖਾਲੀ **avalue = alloca (cif->nargs * ਆਕਾਰ(ਖਾਲੀ *));
  ਅੱਖਰ *ptr = (ਅੱਖਰ *) ਰਜਿਸਟਰ_ਦਲੀਲਾਂ;
  ਪੂਰਨ i;

  /* preserve struct type return pointer passing */
  ਜੇ ((cif->rtype != ਨਿਰਾ) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    ਰਜਿਸਟਰ_ਦਲੀਲਾਂ_ptr = (ਅੱਖਰ *)&ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[1];
  }

  /* Find the address of each argument.  */
  ਲਈ (i = 0; i < cif->nargs; i++)
    {
      ਚੋਣ (arg_types[i]->type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	ਹਾਲਤ FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT16:
	ਹਾਲਤ FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_SINT32:
	ਹਾਲਤ FFI_TYPE_UINT32:
	ਹਾਲਤ FFI_TYPE_FLOAT:
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  avalue[i] = ptr;
	  ਰੋਕੋ;
	ਹਾਲਤ FFI_TYPE_STRUCT:
          {
            ਜੇ (arg_types[i]->ਮਾਪ > 4)
              {
                ਖਾਲੀ *copy = alloca(arg_types[i]->ਮਾਪ);
                ਸਮ੍ਰਿਤੀ_ਨਕਲ(copy, *(ਖਾਲੀ**)ptr, arg_types[i]->ਮਾਪ);
                avalue[i] = copy;
              }
            ਨਹੀਂਤਾਂ
              avalue[i] = *(ਖਾਲੀ**)ptr;
          }
	  ਰੋਕੋ;
	ਮੂਲ:
	  /* This is an 8-byte value.  */
	  ਜੇ (ptr == (ਅੱਖਰ *) &ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[5])
	    {
	      /* The value is split across two locations */
	      ਬਿਨਾਨਿਸ਼ਾਨੀ *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(ਬਿਨਾਨਿਸ਼ਾਨੀ *) ptr;
	      ip[1] = *(ਬਿਨਾਨਿਸ਼ਾਨੀ *) ਢੇਰ_ਦਲੀਲਾਂ;
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  ਰੋਕੋ;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      ਜੇ (ptr == (ਅੱਖਰ *) &ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[6])
	ptr = ਢੇਰ_ਦਲੀਲਾਂ;
      ਨਹੀਂਤਾਂ ਜੇ (ptr == (ਅੱਖਰ *) &ਰਜਿਸਟਰ_ਦਲੀਲਾਂ[7])
	ptr = ਢੇਰ_ਦਲੀਲਾਂ + 4;
    }

  /* Invoke the closure.  */
  ਜੇ (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (ਘੇਰਾ->fun) (cif, struct_rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
    }
  ਨਹੀਂਤਾਂ
    {
      /* Allocate space for the return value and call the function.  */
      ਲੰਮਾ ਲੰਮਾ rvalue;
      (ਘੇਰਾ->fun) (cif, &rvalue, avalue, ਘੇਰਾ->user_ਅੰਕੜੇ);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ* ਘੇਰਾ,
		      ffi_cif* cif,
		      ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ *) &ਘੇਰਾ->tramp[0];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ fn = (ਲੰਮਾ) ffi_ਘੇਰਾ_eabi;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cls = (ਲੰਮਾ) codeloc;

  ਜੇ (cif->abi != FFI_EABI)
    ਵਾਪਸ FFI_BAD_ABI;

  fn = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ffi_ਘੇਰਾ_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

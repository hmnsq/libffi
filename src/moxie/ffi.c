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

خالی *ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  اندراج غیرعلامتی صحیح i;
  اندراج خالی **p_argv;
  اندراج حرف *argp;
  اندراج ffi_type **p_arg;
  اندراج صحیح گنتی = 0;

  p_argv = ecif->avalue;
  argp = ڈھیر;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(خالی **) argp = ecif->rvalue;
      argp += 4;
    }

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->سائز;

      اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = حجم(خالی*);
	  *(خالی **) argp = *p_argv;
	}
      ورنہ اگر (z < حجم(صحیح))
	{
	  z = حجم(صحیح);
	  انتخاب ((*p_arg)->type)
	    {
	    حالت FFI_TYPE_SINT8:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT8 *)(* p_argv);
	      توڑو;

	    حالت FFI_TYPE_UINT8:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT8 *)(* p_argv);
	      توڑو;

	    حالت FFI_TYPE_SINT16:
	      *(علامتی صحیح *) argp = (علامتی صحیح)*(SINT16 *)(* p_argv);
	      توڑو;

	    حالت FFI_TYPE_UINT16:
	      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT16 *)(* p_argv);
	      توڑو;

	    طےشدہ:
	      FFI_ASSERT(0);
	    }
	}
      ورنہ اگر (z == حجم(صحیح))
	{
	  *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	}
      ورنہ
	{
	  حافظہ_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      گنتی += z;
    }

  واپس (ڈھیر + ((گنتی > 24) ? 24 : FFI_ALIGN_نیچے_کو(گنتی, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  اگر (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  ورنہ
    cif->flags = cif->rtype->سائز;

  cif->بائٹیں = FFI_ALIGN (cif->بائٹیں, 8);

  واپس FFI_OK;
}

خارجی خالی ffi_call_EABI(خالی *(*)(حرف *, extended_cif *),
			  extended_cif *,
			  غیرعلامتی, غیرعلامتی,
			  غیرعلامتی *,
			  خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif,
	      خالی (*fn)(خالی),
	      خالی *rvalue,
	      خالی **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;
  صحیح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  اگر ((rvalue == تہی) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->سائز);
    }
  ورنہ
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  برائے (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحیح سائز = at->سائز;
      اگر (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          حرف *argcopy = alloca (سائز);
          اگر (avalue_copy == تہی)
            {
              avalue_copy = alloca (nargs * حجم (خالی *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (خالی *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], سائز);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  انتخاب (cif->abi)
    {
    حالت FFI_EABI:
      ffi_call_EABI(ffi_prep_دلائل, &ecif, cif->بائٹیں,
		    cif->flags, ecif.rvalue, fn);
      توڑو;
    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

خالی ffi_احاطہ_eabi (غیرعلامتی arg1, غیرعلامتی arg2, غیرعلامتی arg3,
		       غیرعلامتی arg4, غیرعلامتی arg5, غیرعلامتی arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  اندراج ffi_احاطہ *creg __asm__ ("$r12");
  ffi_احاطہ *احاطہ = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  اندراج حرف *چوکھٹا_اشارہ_گر __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  خالی *struct_rvalue = (خالی *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  حرف *ڈھیر_دلائل = چوکھٹا_اشارہ_گر + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  غیرعلامتی رجسٹر_دلائل[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  حرف *رجسٹر_دلائل_ptr = (حرف *) رجسٹر_دلائل;

  ffi_cif *cif = احاطہ->cif;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue = alloca (cif->nargs * حجم(خالی *));
  حرف *ptr = (حرف *) رجسٹر_دلائل;
  صحیح i;

  /* preserve struct type return pointer passing */
  اگر ((cif->rtype != تہی) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    رجسٹر_دلائل_ptr = (حرف *)&رجسٹر_دلائل[1];
  }

  /* Find the address of each argument.  */
  برائے (i = 0; i < cif->nargs; i++)
    {
      انتخاب (arg_types[i]->type)
	{
	حالت FFI_TYPE_SINT8:
	حالت FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  توڑو;
	حالت FFI_TYPE_SINT16:
	حالت FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  توڑو;
	حالت FFI_TYPE_SINT32:
	حالت FFI_TYPE_UINT32:
	حالت FFI_TYPE_FLOAT:
	حالت FFI_TYPE_اشارہ_گر:
	  avalue[i] = ptr;
	  توڑو;
	حالت FFI_TYPE_STRUCT:
          {
            اگر (arg_types[i]->سائز > 4)
              {
                خالی *copy = alloca(arg_types[i]->سائز);
                حافظہ_نقل(copy, *(خالی**)ptr, arg_types[i]->سائز);
                avalue[i] = copy;
              }
            ورنہ
              avalue[i] = *(خالی**)ptr;
          }
	  توڑو;
	طےشدہ:
	  /* This is an 8-byte value.  */
	  اگر (ptr == (حرف *) &رجسٹر_دلائل[5])
	    {
	      /* The value is split across two locations */
	      غیرعلامتی *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(غیرعلامتی *) ptr;
	      ip[1] = *(غیرعلامتی *) ڈھیر_دلائل;
	    }
	  ورنہ
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  توڑو;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      اگر (ptr == (حرف *) &رجسٹر_دلائل[6])
	ptr = ڈھیر_دلائل;
      ورنہ اگر (ptr == (حرف *) &رجسٹر_دلائل[7])
	ptr = ڈھیر_دلائل + 4;
    }

  /* Invoke the closure.  */
  اگر (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (احاطہ->fun) (cif, struct_rvalue, avalue, احاطہ->user_ڈیٹا);
    }
  ورنہ
    {
      /* Allocate space for the return value and call the function.  */
      طویل طویل rvalue;
      (احاطہ->fun) (cif, &rvalue, avalue, احاطہ->user_ڈیٹا);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ* احاطہ,
		      ffi_cif* cif,
		      خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی مختصر *tramp = (غیرعلامتی مختصر *) &احاطہ->tramp[0];
  غیرعلامتی طویل fn = (طویل) ffi_احاطہ_eabi;
  غیرعلامتی طویل cls = (طویل) codeloc;

  اگر (cif->abi != FFI_EABI)
    واپس FFI_BAD_ABI;

  fn = (غیرعلامتی طویل) ffi_احاطہ_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

  واپس FFI_OK;
}

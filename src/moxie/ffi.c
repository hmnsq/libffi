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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

فراغ *ffi_prep_دلیلاں(حرف *ڈھیر, extended_cif *ecif)
{
  سجل بنانشانی صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  سجل صحيح گنتی = 0;

  p_argv = ecif->avalue;
  argp = ڈھیر;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT)
    {
      *(فراغ **) argp = ecif->rvalue;
      argp += 4;
    }

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0);
       i--, p_arg++)
    {
      size_t z;

      z = (*p_arg)->ناپ;

      إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
	{
	  z = حجم(فراغ*);
	  *(فراغ **) argp = *p_argv;
	}
      وإلا إذا (z < حجم(صحيح))
	{
	  z = حجم(صحيح);
	  بدل ((*p_arg)->type)
	    {
	    حالة FFI_TYPE_SINT8:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نشانیوالا صحيح *) argp = (نشانیوالا صحيح)*(SINT16 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT16 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم(صحيح))
	{
	  *(بنانشانی صحيح *) argp = (بنانشانی صحيح)*(UINT32 *)(* p_argv);
	}
      وإلا
	{
	  حافظہ_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      گنتی += z;
    }

  أرجع (ڈھیر + ((گنتی > 24) ? 24 : FFI_ALIGN_تھلے(گنتی, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وإلا
    cif->flags = cif->rtype->ناپ;

  cif->بائٹاں = FFI_ALIGN (cif->بائٹاں, 8);

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_EABI(فراغ *(*)(حرف *, extended_cif *),
			  extended_cif *,
			  بنانشانی, بنانشانی,
			  بنانشانی *,
			  فراغ (*fn)(فراغ));

فراغ ffi_call(ffi_cif *cif,
	      فراغ (*fn)(فراغ),
	      فراغ *rvalue,
	      فراغ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;
  صحيح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */

  إذا ((rvalue == عدم) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca(cif->rtype->ناپ);
    }
  وإلا
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح ناپ = at->ناپ;
      إذا (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          حرف *argcopy = alloca (ناپ);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              حافظہ_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          حافظہ_نقل (argcopy, avalue[i], ناپ);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  بدل (cif->abi)
    {
    حالة FFI_EABI:
      ffi_call_EABI(ffi_prep_دلیلاں, &ecif, cif->بائٹاں,
		    cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ ffi_گھیرا_eabi (بنانشانی arg1, بنانشانی arg2, بنانشانی arg3,
		       بنانشانی arg4, بنانشانی arg5, بنانشانی arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  سجل ffi_گھیرا *creg __asm__ ("$r12");
  ffi_گھیرا *گھیرا = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  سجل حرف *چوکھٹا_اشارہ_گر __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  حرف *ڈھیر_دلیلاں = چوکھٹا_اشارہ_گر + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بنانشانی رجسٹر_دلیلاں[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  حرف *رجسٹر_دلیلاں_ptr = (حرف *) رجسٹر_دلیلاں;

  ffi_cif *cif = گھیرا->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) رجسٹر_دلیلاں;
  صحيح i;

  /* preserve struct type return pointer passing */
  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    رجسٹر_دلیلاں_ptr = (حرف *)&رجسٹر_دلیلاں[1];
  }

  /* Find the address of each argument.  */
  لكل (i = 0; i < cif->nargs; i++)
    {
      بدل (arg_types[i]->type)
	{
	حالة FFI_TYPE_SINT8:
	حالة FFI_TYPE_UINT8:
	  avalue[i] = ptr + 3;
	  اكسر;
	حالة FFI_TYPE_SINT16:
	حالة FFI_TYPE_UINT16:
	  avalue[i] = ptr + 2;
	  اكسر;
	حالة FFI_TYPE_SINT32:
	حالة FFI_TYPE_UINT32:
	حالة FFI_TYPE_FLOAT:
	حالة FFI_TYPE_اشارہ_گر:
	  avalue[i] = ptr;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
          {
            إذا (arg_types[i]->ناپ > 4)
              {
                فراغ *copy = alloca(arg_types[i]->ناپ);
                حافظہ_نقل(copy, *(فراغ**)ptr, arg_types[i]->ناپ);
                avalue[i] = copy;
              }
            وإلا
              avalue[i] = *(فراغ**)ptr;
          }
	  اكسر;
	افتراضي:
	  /* This is an 8-byte value.  */
	  إذا (ptr == (حرف *) &رجسٹر_دلیلاں[5])
	    {
	      /* The value is split across two locations */
	      بنانشانی *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(بنانشانی *) ptr;
	      ip[1] = *(بنانشانی *) ڈھیر_دلیلاں;
	    }
	  وإلا
	    {
	      avalue[i] = ptr;
	    }
	  ptr += 4;
	  اكسر;
	}
      ptr += 4;

      /* If we've handled more arguments than fit in registers,
	 start looking at the those passed on the stack.  */
      إذا (ptr == (حرف *) &رجسٹر_دلیلاں[6])
	ptr = ڈھیر_دلیلاں;
      وإلا إذا (ptr == (حرف *) &رجسٹر_دلیلاں[7])
	ptr = ڈھیر_دلیلاں + 4;
    }

  /* Invoke the closure.  */
  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (گھیرا->fun) (cif, struct_rvalue, avalue, گھیرا->user_انکڑے);
    }
  وإلا
    {
      /* Allocate space for the return value and call the function.  */
      لما لما rvalue;
      (گھیرا->fun) (cif, &rvalue, avalue, گھیرا->user_انکڑے);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_حالت
ffi_prep_گھیرا_loc (ffi_گھیرا* گھیرا,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_انکڑے,
		      فراغ *codeloc)
{
  بنانشانی چھوٹا *tramp = (بنانشانی چھوٹا *) &گھیرا->tramp[0];
  بنانشانی لما fn = (لما) ffi_گھیرا_eabi;
  بنانشانی لما cls = (لما) codeloc;

  إذا (cif->abi != FFI_EABI)
    أرجع FFI_BAD_ABI;

  fn = (بنانشانی لما) ffi_گھیرا_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  گھیرا->cif = cif;
  گھیرا->fun = fun;
  گھیرا->user_انکڑے = user_انکڑے;

  أرجع FFI_OK;
}

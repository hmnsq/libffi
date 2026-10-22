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

فراغ *ffi_prep_دلیلونه(حرف *ډېری, extended_cif *ecif)
{
  سجل بینښه صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  سجل صحيح شمېر = 0;

  p_argv = ecif->avalue;
  argp = ډېری;

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

      z = (*p_arg)->اندازه;

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
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نښهدار صحيح *) argp = (نښهدار صحيح)*(SINT16 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT16 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم(صحيح))
	{
	  *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	}
      وإلا
	{
	  حافظه_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      شمېر += z;
    }

  أرجع (ډېری + ((شمېر > 24) ? 24 : FFI_ALIGN_ښکته(شمېر, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وإلا
    cif->flags = cif->rtype->اندازه;

  cif->بایټونه = FFI_ALIGN (cif->بایټونه, 8);

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_EABI(فراغ *(*)(حرف *, extended_cif *),
			  extended_cif *,
			  بینښه, بینښه,
			  بینښه *,
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
      ecif.rvalue = alloca(cif->rtype->اندازه);
    }
  وإلا
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح اندازه = at->اندازه;
      إذا (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          حرف *argcopy = alloca (اندازه);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              حافظه_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          حافظه_نقل (argcopy, avalue[i], اندازه);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  بدل (cif->abi)
    {
    حالة FFI_EABI:
      ffi_call_EABI(ffi_prep_دلیلونه, &ecif, cif->بایټونه,
		    cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ ffi_تړون_eabi (بینښه arg1, بینښه arg2, بینښه arg3,
		       بینښه arg4, بینښه arg5, بینښه arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  سجل ffi_تړون *creg __asm__ ("$r12");
  ffi_تړون *تړون = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  سجل حرف *چوکاټ_اشاره_ګر __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  حرف *ډېری_دلیلونه = چوکاټ_اشاره_ګر + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بینښه ثبتوونکی_دلیلونه[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  حرف *ثبتوونکی_دلیلونه_ptr = (حرف *) ثبتوونکی_دلیلونه;

  ffi_cif *cif = تړون->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) ثبتوونکی_دلیلونه;
  صحيح i;

  /* preserve struct type return pointer passing */
  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    ثبتوونکی_دلیلونه_ptr = (حرف *)&ثبتوونکی_دلیلونه[1];
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
	حالة FFI_TYPE_اشاره_ګر:
	  avalue[i] = ptr;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
          {
            إذا (arg_types[i]->اندازه > 4)
              {
                فراغ *copy = alloca(arg_types[i]->اندازه);
                حافظه_نقل(copy, *(فراغ**)ptr, arg_types[i]->اندازه);
                avalue[i] = copy;
              }
            وإلا
              avalue[i] = *(فراغ**)ptr;
          }
	  اكسر;
	افتراضي:
	  /* This is an 8-byte value.  */
	  إذا (ptr == (حرف *) &ثبتوونکی_دلیلونه[5])
	    {
	      /* The value is split across two locations */
	      بینښه *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(بینښه *) ptr;
	      ip[1] = *(بینښه *) ډېری_دلیلونه;
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
      إذا (ptr == (حرف *) &ثبتوونکی_دلیلونه[6])
	ptr = ډېری_دلیلونه;
      وإلا إذا (ptr == (حرف *) &ثبتوونکی_دلیلونه[7])
	ptr = ډېری_دلیلونه + 4;
    }

  /* Invoke the closure.  */
  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (تړون->fun) (cif, struct_rvalue, avalue, تړون->user_معلومات);
    }
  وإلا
    {
      /* Allocate space for the return value and call the function.  */
      اوږد اوږد rvalue;
      (تړون->fun) (cif, &rvalue, avalue, تړون->user_معلومات);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_حالت
ffi_prep_تړون_loc (ffi_تړون* تړون,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بینښه لنډ *tramp = (بینښه لنډ *) &تړون->tramp[0];
  بینښه اوږد fn = (اوږد) ffi_تړون_eabi;
  بینښه اوږد cls = (اوږد) codeloc;

  إذا (cif->abi != FFI_EABI)
    أرجع FFI_BAD_ABI;

  fn = (بینښه اوږد) ffi_تړون_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  تړون->cif = cif;
  تړون->fun = fun;
  تړون->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

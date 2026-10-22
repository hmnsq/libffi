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

فراغ *ffi_prep_آرگومانها(حرف *پشته, extended_cif *ecif)
{
  سجل بیعلامت صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  سجل صحيح شمار = 0;

  p_argv = ecif->avalue;
  argp = پشته;

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
	      *(علامتدار صحيح *) argp = (علامتدار صحيح)*(SINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(علامتدار صحيح *) argp = (علامتدار صحيح)*(SINT16 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT16 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم(صحيح))
	{
	  *(بیعلامت صحيح *) argp = (بیعلامت صحيح)*(UINT32 *)(* p_argv);
	}
      وإلا
	{
	  کپی_حافظه(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      شمار += z;
    }

  أرجع (پشته + ((شمار > 24) ? 24 : FFI_ALIGN_پایین(شمار, 8)));
}

/* Perform machine dependent cif processing */
ffi_وضعیت ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وإلا
    cif->flags = cif->rtype->اندازه;

  cif->بایتها = FFI_ALIGN (cif->بایتها, 8);

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_EABI(فراغ *(*)(حرف *, extended_cif *),
			  extended_cif *,
			  بیعلامت, بیعلامت,
			  بیعلامت *,
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
              کپی_حافظه (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          کپی_حافظه (argcopy, avalue[i], اندازه);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  بدل (cif->abi)
    {
    حالة FFI_EABI:
      ffi_call_EABI(ffi_prep_آرگومانها, &ecif, cif->بایتها,
		    cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ ffi_بستگی_eabi (بیعلامت arg1, بیعلامت arg2, بیعلامت arg3,
		       بیعلامت arg4, بیعلامت arg5, بیعلامت arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  سجل ffi_بستگی *creg __asm__ ("$r12");
  ffi_بستگی *بستگی = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  سجل حرف *قاب_اشاره_گر __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  حرف *پشته_آرگومانها = قاب_اشاره_گر + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بیعلامت ثبات_آرگومانها[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  حرف *ثبات_آرگومانها_ptr = (حرف *) ثبات_آرگومانها;

  ffi_cif *cif = بستگی->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) ثبات_آرگومانها;
  صحيح i;

  /* preserve struct type return pointer passing */
  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    ثبات_آرگومانها_ptr = (حرف *)&ثبات_آرگومانها[1];
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
	حالة FFI_TYPE_اشاره_گر:
	  avalue[i] = ptr;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
          {
            إذا (arg_types[i]->اندازه > 4)
              {
                فراغ *copy = alloca(arg_types[i]->اندازه);
                کپی_حافظه(copy, *(فراغ**)ptr, arg_types[i]->اندازه);
                avalue[i] = copy;
              }
            وإلا
              avalue[i] = *(فراغ**)ptr;
          }
	  اكسر;
	افتراضي:
	  /* This is an 8-byte value.  */
	  إذا (ptr == (حرف *) &ثبات_آرگومانها[5])
	    {
	      /* The value is split across two locations */
	      بیعلامت *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(بیعلامت *) ptr;
	      ip[1] = *(بیعلامت *) پشته_آرگومانها;
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
      إذا (ptr == (حرف *) &ثبات_آرگومانها[6])
	ptr = پشته_آرگومانها;
      وإلا إذا (ptr == (حرف *) &ثبات_آرگومانها[7])
	ptr = پشته_آرگومانها + 4;
    }

  /* Invoke the closure.  */
  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (بستگی->fun) (cif, struct_rvalue, avalue, بستگی->user_داده);
    }
  وإلا
    {
      /* Allocate space for the return value and call the function.  */
      دراز دراز rvalue;
      (بستگی->fun) (cif, &rvalue, avalue, بستگی->user_داده);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_وضعیت
ffi_prep_بستگی_loc (ffi_بستگی* بستگی,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_داده,
		      فراغ *codeloc)
{
  بیعلامت کوتاه *tramp = (بیعلامت کوتاه *) &بستگی->tramp[0];
  بیعلامت دراز fn = (دراز) ffi_بستگی_eabi;
  بیعلامت دراز cls = (دراز) codeloc;

  إذا (cif->abi != FFI_EABI)
    أرجع FFI_BAD_ABI;

  fn = (بیعلامت دراز) ffi_بستگی_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  بستگی->cif = cif;
  بستگی->fun = fun;
  بستگی->user_داده = user_داده;

  أرجع FFI_OK;
}

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

فراغ *ffi_prep_دليلون(حرف *ڍير, extended_cif *ecif)
{
  سجل بنانشان صحيح i;
  سجل فراغ **p_argv;
  سجل حرف *argp;
  سجل ffi_type **p_arg;
  سجل صحيح ڳڻپ = 0;

  p_argv = ecif->avalue;
  argp = ڍير;

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

      z = (*p_arg)->سائيز;

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
	      *(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT8:
	      *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT8 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_SINT16:
	      *(نشانوارو صحيح *) argp = (نشانوارو صحيح)*(SINT16 *)(* p_argv);
	      اكسر;

	    حالة FFI_TYPE_UINT16:
	      *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT16 *)(* p_argv);
	      اكسر;

	    افتراضي:
	      FFI_ASSERT(0);
	    }
	}
      وإلا إذا (z == حجم(صحيح))
	{
	  *(بنانشان صحيح *) argp = (بنانشان صحيح)*(UINT32 *)(* p_argv);
	}
      وإلا
	{
	  يادگيري_نقل(argp, *p_argv, z);
	}
      p_argv++;
      argp += z;
      ڳڻپ += z;
    }

  أرجع (ڍير + ((ڳڻپ > 24) ? 24 : FFI_ALIGN_هيٺ_ڏانهن(ڳڻپ, 8)));
}

/* Perform machine dependent cif processing */
ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
  إذا (cif->rtype->type == FFI_TYPE_STRUCT)
    cif->flags = -1;
  وإلا
    cif->flags = cif->rtype->سائيز;

  cif->بائيٽون = FFI_ALIGN (cif->بائيٽون, 8);

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_EABI(فراغ *(*)(حرف *, extended_cif *),
			  extended_cif *,
			  بنانشان, بنانشان,
			  بنانشان *,
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
      ecif.rvalue = alloca(cif->rtype->سائيز);
    }
  وإلا
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  لكل (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      صحيح سائيز = at->سائيز;
      إذا (at->type == FFI_TYPE_STRUCT) /*  && size > 4) All struct args?? */
        {
          حرف *argcopy = alloca (سائيز);
          إذا (avalue_copy == عدم)
            {
              avalue_copy = alloca (nargs * حجم (فراغ *));
              يادگيري_نقل (avalue_copy, avalue, nargs * حجم (فراغ *));
              avalue = avalue_copy;
            }
          يادگيري_نقل (argcopy, avalue[i], سائيز);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  بدل (cif->abi)
    {
    حالة FFI_EABI:
      ffi_call_EABI(ffi_prep_دليلون, &ecif, cif->بائيٽون,
		    cif->flags, ecif.rvalue, fn);
      اكسر;
    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ ffi_گهيرو_eabi (بنانشان arg1, بنانشان arg2, بنانشان arg3,
		       بنانشان arg4, بنانشان arg5, بنانشان arg6)
{
  /* This function is called by a trampoline.  The trampoline stows a
     pointer to the ffi_closure object in $r12.  We must save this
     pointer in a place that will persist while we do our work.  */
  سجل ffi_گهيرو *creg __asm__ ("$r12");
  ffi_گهيرو *گهيرو = creg;

  /* Arguments that don't fit in registers are found on the stack
     at a fixed offset above the current frame pointer.  */
  سجل حرف *چوکٺو_اشارو __asm__ ("$fp");

  /* Pointer to a struct return value.  */
  فراغ *struct_rvalue = (فراغ *) arg1;

  /* 6 words reserved for register args + 3 words from jsr */
  حرف *ڍير_دليلون = چوکٺو_اشارو + 9*4;

  /* Lay the register arguments down in a continuous chunk of memory.  */
  بنانشان رجسٽر_دليلون[6] =
    { arg1, arg2, arg3, arg4, arg5, arg6 };
  حرف *رجسٽر_دليلون_ptr = (حرف *) رجسٽر_دليلون;

  ffi_cif *cif = گهيرو->cif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue = alloca (cif->nargs * حجم(فراغ *));
  حرف *ptr = (حرف *) رجسٽر_دليلون;
  صحيح i;

  /* preserve struct type return pointer passing */
  إذا ((cif->rtype != عدم) && (cif->rtype->type == FFI_TYPE_STRUCT)) {
    ptr += 4;
    رجسٽر_دليلون_ptr = (حرف *)&رجسٽر_دليلون[1];
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
	حالة FFI_TYPE_اشارو:
	  avalue[i] = ptr;
	  اكسر;
	حالة FFI_TYPE_STRUCT:
          {
            إذا (arg_types[i]->سائيز > 4)
              {
                فراغ *copy = alloca(arg_types[i]->سائيز);
                يادگيري_نقل(copy, *(فراغ**)ptr, arg_types[i]->سائيز);
                avalue[i] = copy;
              }
            وإلا
              avalue[i] = *(فراغ**)ptr;
          }
	  اكسر;
	افتراضي:
	  /* This is an 8-byte value.  */
	  إذا (ptr == (حرف *) &رجسٽر_دليلون[5])
	    {
	      /* The value is split across two locations */
	      بنانشان *ip = alloca(8);
	      avalue[i] = ip;
	      ip[0] = *(بنانشان *) ptr;
	      ip[1] = *(بنانشان *) ڍير_دليلون;
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
      إذا (ptr == (حرف *) &رجسٽر_دليلون[6])
	ptr = ڍير_دليلون;
      وإلا إذا (ptr == (حرف *) &رجسٽر_دليلون[7])
	ptr = ڍير_دليلون + 4;
    }

  /* Invoke the closure.  */
  إذا (cif->rtype && (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      (گهيرو->fun) (cif, struct_rvalue, avalue, گهيرو->user_معلومات);
    }
  وإلا
    {
      /* Allocate space for the return value and call the function.  */
      ڊگهو ڊگهو rvalue;
      (گهيرو->fun) (cif, &rvalue, avalue, گهيرو->user_معلومات);
      __asm__ ("mov $r12, %0\n ld.l $r0, ($r12)\n ldo.l $r1, 4($r12)" : : "r" (&rvalue));
    }
}

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو* گهيرو,
		      ffi_cif* cif,
		      فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بنانشان ننڍو *tramp = (بنانشان ننڍو *) &گهيرو->tramp[0];
  بنانشان ڊگهو fn = (ڊگهو) ffi_گهيرو_eabi;
  بنانشان ڊگهو cls = (ڊگهو) codeloc;

  إذا (cif->abi != FFI_EABI)
    أرجع FFI_BAD_ABI;

  fn = (بنانشان ڊگهو) ffi_گهيرو_eabi;

  tramp[0] = 0x01e0; /* ldi.l $r12, .... */
  tramp[1] = cls >> 16;
  tramp[2] = cls & 0xffff;
  tramp[3] = 0x1a00; /* jmpa .... */
  tramp[4] = fn >> 16;
  tramp[5] = fn & 0xffff;

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

  أرجع FFI_OK;
}

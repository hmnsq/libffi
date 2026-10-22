/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2004  Renesas Technology
           Copyright (c) 2008  Red Hat, Inc.
           Copyright (c) 2022, 2026  Anthony Green

   M32R Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL RENESAS TECHNOLOGY BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

فراغ ffi_prep_دلیلونه(حرف *ډېری, extended_cif *ecif)
{
  بینښه صحيح i;
  صحيح tmp;
  بینښه صحيح avn;
  فراغ **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ډېری;

  إذا (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->اندازه > 8)
    {
      *(فراغ **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  لكل (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      إذا (((*p_arg)->alignment - 1) & (بینښه) argp)
	argp = (حرف *) FFI_ALIGN (argp, (*p_arg)->alignment);

      إذا (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->اندازه;
	  إذا (z < حجم (صحيح))
	    {
	      z = حجم (صحيح);

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

		حالة FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->اندازه;
	  	  إذا ((*p_arg)->alignment != 1)
		    حافظه_نقل (argp, *p_argv, z);
		  وإلا
		    حافظه_نقل (argp + 4 - z, *p_argv, z);
	  	  z = حجم (صحيح);
		  اكسر;

		افتراضي:
		  FFI_ASSERT(0);
		}
	    }
	  وإلا إذا (z == حجم (صحيح))
	    {
	       *(بینښه صحيح *) argp = (بینښه صحيح)*(UINT32 *)(* p_argv);
	    }
	  وإلا
	    {
	      إذا ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  إذا (z > 8)
		    {
		      *(بینښه صحيح *) argp = (بینښه صحيح)(فراغ *)(* p_argv);
		      z = حجم(فراغ *);
		    }
		  وإلا
		    {
	              حافظه_نقل(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      وإلا
	        {
		  /* Double or long long 64bit.  */
	          حافظه_نقل (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  أرجع;
}

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  بدل (cif->rtype->type)
    {
    حالة FFI_TYPE_VOID:
      cif->flags = (بینښه) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_STRUCT:
      إذا (cif->rtype->اندازه <= 4)
	cif->flags = FFI_TYPE_INT;

      وإلا إذا (cif->rtype->اندازه <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      وإلا
	cif->flags = (بینښه) cif->rtype->type;
      اكسر;

    حالة FFI_TYPE_SINT64:
    حالة FFI_TYPE_UINT64:
    حالة FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      اكسر;

    حالة FFI_TYPE_FLOAT:
    افتراضي:
      cif->flags = FFI_TYPE_INT;
      اكسر;
    }

  أرجع FFI_OK;
}

خارجي فراغ ffi_call_SYSV(فراغ (*)(حرف *, extended_cif *), extended_cif *,
			  بینښه, بینښه, بینښه *, فراغ (*fn)(فراغ));

فراغ ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  فراغ **avalue_copy = عدم;
  صحيح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  إذا ((rvalue == عدم) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->اندازه);
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
      إذا (at->type == FFI_TYPE_STRUCT && اندازه > 4)
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
    حالة FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلیلونه, &ecif, cif->بایټونه,
		    cif->flags, ecif.rvalue, fn);
      إذا (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  صحيح اندازه = cif->rtype->اندازه;
	  صحيح align = cif->rtype->alignment;

	  إذا (اندازه < 4)
	    {
	      إذا (align == 1)
	        *(بینښه اوږد *)(ecif.rvalue) <<= (4 - اندازه) * 8;
	    }
	  وإلا إذا (4 < اندازه && اندازه < 8)
	    {
	      إذا (align == 1)
		{
		  حافظه_نقل (ecif.rvalue, ecif.rvalue + 8-اندازه, اندازه);
		}
	      وإلا إذا (align == 2)
		{
		  إذا (اندازه & 1)
		    اندازه += 1;

		  إذا (اندازه != 8)
		    حافظه_نقل (ecif.rvalue, ecif.rvalue + 8-اندازه, اندازه);
		}
	    }
	}
      اكسر;

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

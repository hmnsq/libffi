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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

خالی ffi_prep_دلائل(حرف *ڈھیر, extended_cif *ecif)
{
  غیرعلامتی صحیح i;
  صحیح tmp;
  غیرعلامتی صحیح avn;
  خالی **p_argv;
  حرف *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ڈھیر;

  اگر (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->سائز > 8)
    {
      *(خالی **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  برائے (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      اگر (((*p_arg)->alignment - 1) & (غیرعلامتی) argp)
	argp = (حرف *) FFI_ALIGN (argp, (*p_arg)->alignment);

      اگر (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->سائز;
	  اگر (z < حجم (صحیح))
	    {
	      z = حجم (صحیح);

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

		حالت FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->سائز;
	  	  اگر ((*p_arg)->alignment != 1)
		    حافظہ_نقل (argp, *p_argv, z);
		  ورنہ
		    حافظہ_نقل (argp + 4 - z, *p_argv, z);
	  	  z = حجم (صحیح);
		  توڑو;

		طےشدہ:
		  FFI_ASSERT(0);
		}
	    }
	  ورنہ اگر (z == حجم (صحیح))
	    {
	       *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)*(UINT32 *)(* p_argv);
	    }
	  ورنہ
	    {
	      اگر ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  اگر (z > 8)
		    {
		      *(غیرعلامتی صحیح *) argp = (غیرعلامتی صحیح)(خالی *)(* p_argv);
		      z = حجم(خالی *);
		    }
		  ورنہ
		    {
	              حافظہ_نقل(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      ورنہ
	        {
		  /* Double or long long 64bit.  */
	          حافظہ_نقل (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  واپس;
}

/* Perform machine dependent cif processing.  */
ffi_حالت
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  انتخاب (cif->rtype->type)
    {
    حالت FFI_TYPE_VOID:
      cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

    حالت FFI_TYPE_STRUCT:
      اگر (cif->rtype->سائز <= 4)
	cif->flags = FFI_TYPE_INT;

      ورنہ اگر (cif->rtype->سائز <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      ورنہ
	cif->flags = (غیرعلامتی) cif->rtype->type;
      توڑو;

    حالت FFI_TYPE_SINT64:
    حالت FFI_TYPE_UINT64:
    حالت FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      توڑو;

    حالت FFI_TYPE_FLOAT:
    طےشدہ:
      cif->flags = FFI_TYPE_INT;
      توڑو;
    }

  واپس FFI_OK;
}

خارجی خالی ffi_call_SYSV(خالی (*)(حرف *, extended_cif *), extended_cif *,
			  غیرعلامتی, غیرعلامتی, غیرعلامتی *, خالی (*fn)(خالی));

خالی ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  خالی **avalue_copy = تہی;
  صحیح i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  اگر ((rvalue == تہی) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->سائز);
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
      اگر (at->type == FFI_TYPE_STRUCT && سائز > 4)
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
    حالت FFI_SYSV:
      ffi_call_SYSV(ffi_prep_دلائل, &ecif, cif->بائٹیں,
		    cif->flags, ecif.rvalue, fn);
      اگر (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  صحیح سائز = cif->rtype->سائز;
	  صحیح align = cif->rtype->alignment;

	  اگر (سائز < 4)
	    {
	      اگر (align == 1)
	        *(غیرعلامتی طویل *)(ecif.rvalue) <<= (4 - سائز) * 8;
	    }
	  ورنہ اگر (4 < سائز && سائز < 8)
	    {
	      اگر (align == 1)
		{
		  حافظہ_نقل (ecif.rvalue, ecif.rvalue + 8-سائز, سائز);
		}
	      ورنہ اگر (align == 2)
		{
		  اگر (سائز & 1)
		    سائز += 1;

		  اگر (سائز != 8)
		    حافظہ_نقل (ecif.rvalue, ecif.rvalue + 8-سائز, سائز);
		}
	    }
	}
      توڑو;

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

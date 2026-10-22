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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

शून्य ffi_prep_args(वर्ण *ढेर, extended_cif *ecif)
{
  अचिन्हित पूर्णाङ्क i;
  पूर्णाङ्क tmp;
  अचिन्हित पूर्णाङ्क avn;
  शून्य **p_argv;
  वर्ण *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ढेर;

  यदि (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  प्रति (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      माप_प्रकार z;

      /* Align if necessary.  */
      यदि (((*p_arg)->alignment - 1) & (अचिन्हित) argp)
	argp = (वर्ण *) FFI_ALIGN (argp, (*p_arg)->alignment);

      यदि (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  यदि (z < परिमाण (पूर्णाङ्क))
	    {
	      z = परिमाण (पूर्णाङ्क);

	      विकल्प ((*p_arg)->type)
		{
		पक्ष FFI_TYPE_SINT8:
		  *(चिन्हित पूर्णाङ्क *) argp = (चिन्हित पूर्णाङ्क)*(SINT8 *)(* p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_UINT8:
		  *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क)*(UINT8 *)(* p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_SINT16:
		  *(चिन्हित पूर्णाङ्क *) argp = (चिन्हित पूर्णाङ्क)*(SINT16 *)(* p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_UINT16:
		  *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क)*(UINT16 *)(* p_argv);
		  भङ्ग;

		पक्ष FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  यदि ((*p_arg)->alignment != 1)
		    स_नकल (argp, *p_argv, z);
		  अन्यथा
		    स_नकल (argp + 4 - z, *p_argv, z);
	  	  z = परिमाण (पूर्णाङ्क);
		  भङ्ग;

		स्वभाव:
		  FFI_ASSERT(0);
		}
	    }
	  अन्यथा यदि (z == परिमाण (पूर्णाङ्क))
	    {
	       *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क)*(UINT32 *)(* p_argv);
	    }
	  अन्यथा
	    {
	      यदि ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  यदि (z > 8)
		    {
		      *(अचिन्हित पूर्णाङ्क *) argp = (अचिन्हित पूर्णाङ्क)(शून्य *)(* p_argv);
		      z = परिमाण(शून्य *);
		    }
		  अन्यथा
		    {
	              स_नकल(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      अन्यथा
	        {
		  /* Double or long long 64bit.  */
	          स_नकल (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  प्रत्यागम;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  विकल्प (cif->rtype->type)
    {
    पक्ष FFI_TYPE_VOID:
      cif->flags = (अचिन्हित) cif->rtype->type;
      भङ्ग;

    पक्ष FFI_TYPE_STRUCT:
      यदि (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      अन्यथा यदि (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      अन्यथा
	cif->flags = (अचिन्हित) cif->rtype->type;
      भङ्ग;

    पक्ष FFI_TYPE_SINT64:
    पक्ष FFI_TYPE_UINT64:
    पक्ष FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      भङ्ग;

    पक्ष FFI_TYPE_FLOAT:
    स्वभाव:
      cif->flags = FFI_TYPE_INT;
      भङ्ग;
    }

  प्रत्यागम FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*)(वर्ण *, extended_cif *), extended_cif *,
			  अचिन्हित, अचिन्हित, अचिन्हित *, शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णाङ्क i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  यदि ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  प्रति (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णाङ्क size = at->size;
      यदि (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          वर्ण *argcopy = alloca (size);
          यदि (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * परिमाण (शून्य *));
              स_नकल (avalue_copy, avalue, nargs * परिमाण (शून्य *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  विकल्प (cif->abi)
    {
    पक्ष FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      यदि (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  पूर्णाङ्क size = cif->rtype->size;
	  पूर्णाङ्क align = cif->rtype->alignment;

	  यदि (size < 4)
	    {
	      यदि (align == 1)
	        *(अचिन्हित दीर्घ *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  अन्यथा यदि (4 < size && size < 8)
	    {
	      यदि (align == 1)
		{
		  स_नकल (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      अन्यथा यदि (align == 2)
		{
		  यदि (size & 1)
		    size += 1;

		  यदि (size != 8)
		    स_नकल (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      भङ्ग;

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

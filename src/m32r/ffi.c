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

शून्य ffi_prep_args(अक्षर *चळत, extended_cif *ecif)
{
  अचिन्हित पूर्णांक i;
  पूर्णांक tmp;
  अचिन्हित पूर्णांक avn;
  शून्य **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = चळत;

  जर (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(शून्य **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  साठी (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      माप_प्रकार z;

      /* Align if necessary.  */
      जर (((*p_arg)->alignment - 1) & (अचिन्हित) argp)
	argp = (अक्षर *) FFI_ALIGN (argp, (*p_arg)->alignment);

      जर (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  जर (z < आकार (पूर्णांक))
	    {
	      z = आकार (पूर्णांक);

	      निवड ((*p_arg)->type)
		{
		स्थिती FFI_TYPE_SINT8:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
		  थांबा;

		स्थिती FFI_TYPE_UINT8:
		  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
		  थांबा;

		स्थिती FFI_TYPE_SINT16:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
		  थांबा;

		स्थिती FFI_TYPE_UINT16:
		  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
		  थांबा;

		स्थिती FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  जर ((*p_arg)->alignment != 1)
		    स_नकल (argp, *p_argv, z);
		  अन्यथा
		    स_नकल (argp + 4 - z, *p_argv, z);
	  	  z = आकार (पूर्णांक);
		  थांबा;

		पूर्वनिर्धारित:
		  FFI_ASSERT(0);
		}
	    }
	  अन्यथा जर (z == आकार (पूर्णांक))
	    {
	       *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	    }
	  अन्यथा
	    {
	      जर ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  जर (z > 8)
		    {
		      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)(शून्य *)(* p_argv);
		      z = आकार(शून्य *);
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

  परत;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  निवड (cif->rtype->type)
    {
    स्थिती FFI_TYPE_VOID:
      cif->flags = (अचिन्हित) cif->rtype->type;
      थांबा;

    स्थिती FFI_TYPE_STRUCT:
      जर (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      अन्यथा जर (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      अन्यथा
	cif->flags = (अचिन्हित) cif->rtype->type;
      थांबा;

    स्थिती FFI_TYPE_SINT64:
    स्थिती FFI_TYPE_UINT64:
    स्थिती FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      थांबा;

    स्थिती FFI_TYPE_FLOAT:
    पूर्वनिर्धारित:
      cif->flags = FFI_TYPE_INT;
      थांबा;
    }

  परत FFI_OK;
}

बाह्य शून्य ffi_call_SYSV(शून्य (*)(अक्षर *, extended_cif *), extended_cif *,
			  अचिन्हित, अचिन्हित, अचिन्हित *, शून्य (*fn)(शून्य));

शून्य ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  शून्य **avalue_copy = रिक्त;
  पूर्णांक i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  जर ((rvalue == रिक्त) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  साठी (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक size = at->size;
      जर (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          अक्षर *argcopy = alloca (size);
          जर (avalue_copy == रिक्त)
            {
              avalue_copy = alloca (nargs * आकार (शून्य *));
              स_नकल (avalue_copy, avalue, nargs * आकार (शून्य *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  निवड (cif->abi)
    {
    स्थिती FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      जर (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  पूर्णांक size = cif->rtype->size;
	  पूर्णांक align = cif->rtype->alignment;

	  जर (size < 4)
	    {
	      जर (align == 1)
	        *(अचिन्हित दीर्घ *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  अन्यथा जर (4 < size && size < 8)
	    {
	      जर (align == 1)
		{
		  स_नकल (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      अन्यथा जर (align == 2)
		{
		  जर (size & 1)
		    size += 1;

		  जर (size != 8)
		    स_नकल (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      थांबा;

    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}

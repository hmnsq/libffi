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

#समावेश <ffi.h>
#समावेश <ffi_common.h>

#समावेश <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

व्योम ffi_prep_args(अक्षर *ढेर, extended_cif *ecif)
{
  अचिन्हित पूर्णांक i;
  पूर्णांक tmp;
  अचिन्हित पूर्णांक avn;
  व्योम **p_argv;
  अक्षर *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = ढेर;

  अगर (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->आकार > 8)
    {
      *(व्योम **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  क्रम (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      माप_प्रकार z;

      /* Align if necessary.  */
      अगर (((*p_arg)->alignment - 1) & (अचिन्हित) argp)
	argp = (अक्षर *) FFI_ALIGN (argp, (*p_arg)->alignment);

      अगर (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->आकार;
	  अगर (z < माप (पूर्णांक))
	    {
	      z = माप (पूर्णांक);

	      चयन ((*p_arg)->type)
		{
		हाल FFI_TYPE_SINT8:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT8 *)(* p_argv);
		  अवरोध;

		हाल FFI_TYPE_UINT8:
		  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT8 *)(* p_argv);
		  अवरोध;

		हाल FFI_TYPE_SINT16:
		  *(चिन्हित पूर्णांक *) argp = (चिन्हित पूर्णांक)*(SINT16 *)(* p_argv);
		  अवरोध;

		हाल FFI_TYPE_UINT16:
		  *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT16 *)(* p_argv);
		  अवरोध;

		हाल FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->आकार;
	  	  अगर ((*p_arg)->alignment != 1)
		    स_नकल (argp, *p_argv, z);
		  अन्यथा
		    स_नकल (argp + 4 - z, *p_argv, z);
	  	  z = माप (पूर्णांक);
		  अवरोध;

		शेष:
		  FFI_ASSERT(0);
		}
	    }
	  अन्यथा अगर (z == माप (पूर्णांक))
	    {
	       *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)*(UINT32 *)(* p_argv);
	    }
	  अन्यथा
	    {
	      अगर ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  अगर (z > 8)
		    {
		      *(अचिन्हित पूर्णांक *) argp = (अचिन्हित पूर्णांक)(व्योम *)(* p_argv);
		      z = माप(व्योम *);
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

  वापस;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  चयन (cif->rtype->type)
    {
    हाल FFI_TYPE_VOID:
      cif->flags = (अचिन्हित) cif->rtype->type;
      अवरोध;

    हाल FFI_TYPE_STRUCT:
      अगर (cif->rtype->आकार <= 4)
	cif->flags = FFI_TYPE_INT;

      अन्यथा अगर (cif->rtype->आकार <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      अन्यथा
	cif->flags = (अचिन्हित) cif->rtype->type;
      अवरोध;

    हाल FFI_TYPE_SINT64:
    हाल FFI_TYPE_UINT64:
    हाल FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      अवरोध;

    हाल FFI_TYPE_FLOAT:
    शेष:
      cif->flags = FFI_TYPE_INT;
      अवरोध;
    }

  वापस FFI_OK;
}

बाह्य व्योम ffi_call_SYSV(व्योम (*)(अक्षर *, extended_cif *), extended_cif *,
			  अचिन्हित, अचिन्हित, अचिन्हित *, व्योम (*fn)(व्योम));

व्योम ffi_call(ffi_cif *cif, व्योम (*fn)(व्योम), व्योम *rvalue, व्योम **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  व्योम **avalue_copy = शून्य;
  पूर्णांक i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  अगर ((rvalue == शून्य) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->आकार);
    }
  अन्यथा
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  क्रम (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      पूर्णांक आकार = at->आकार;
      अगर (at->type == FFI_TYPE_STRUCT && आकार > 4)
        {
          अक्षर *argcopy = alloca (आकार);
          अगर (avalue_copy == शून्य)
            {
              avalue_copy = alloca (nargs * माप (व्योम *));
              स_नकल (avalue_copy, avalue, nargs * माप (व्योम *));
              avalue = avalue_copy;
            }
          स_नकल (argcopy, avalue[i], आकार);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  चयन (cif->abi)
    {
    हाल FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      अगर (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  पूर्णांक आकार = cif->rtype->आकार;
	  पूर्णांक align = cif->rtype->alignment;

	  अगर (आकार < 4)
	    {
	      अगर (align == 1)
	        *(अचिन्हित दीर्घ *)(ecif.rvalue) <<= (4 - आकार) * 8;
	    }
	  अन्यथा अगर (4 < आकार && आकार < 8)
	    {
	      अगर (align == 1)
		{
		  स_नकल (ecif.rvalue, ecif.rvalue + 8-आकार, आकार);
		}
	      अन्यथा अगर (align == 2)
		{
		  अगर (आकार & 1)
		    आकार += 1;

		  अगर (आकार != 8)
		    स_नकल (ecif.rvalue, ecif.rvalue + 8-आकार, आकार);
		}
	    }
	}
      अवरोध;

    शेष:
      FFI_ASSERT(0);
      अवरोध;
    }
}

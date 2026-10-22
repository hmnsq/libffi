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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

শূন্য ffi_prep_args(অক্ষর *স্তূপ, extended_cif *ecif)
{
  unsigned পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা tmp;
  unsigned পূর্ণসংখ্যা avn;
  শূন্য **p_argv;
  অক্ষর *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = স্তূপ;

  যদি (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->আকার_2 > 8)
    {
      *(শূন্য **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  জন্য (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      যদি (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (অক্ষর *) FFI_ALIGN (argp, (*p_arg)->alignment);

      যদি (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->আকার_2;
	  যদি (z < আকার (পূর্ণসংখ্যা))
	    {
	      z = আকার (পূর্ণসংখ্যা);

	      নির্বাচন ((*p_arg)->type)
		{
		ক্ষেত্র FFI_TYPE_SINT8:
		  *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT8 *)(* p_argv);
		  বিরতি;

		ক্ষেত্র FFI_TYPE_UINT8:
		  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT8 *)(* p_argv);
		  বিরতি;

		ক্ষেত্র FFI_TYPE_SINT16:
		  *(signed পূর্ণসংখ্যা *) argp = (signed পূর্ণসংখ্যা)*(SINT16 *)(* p_argv);
		  বিরতি;

		ক্ষেত্র FFI_TYPE_UINT16:
		  *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT16 *)(* p_argv);
		  বিরতি;

		ক্ষেত্র FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->আকার_2;
	  	  যদি ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  অন্যথা
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = আকার (পূর্ণসংখ্যা);
		  বিরতি;

		পূর্বনির্ধারিত:
		  FFI_ASSERT(0);
		}
	    }
	  অন্যথা যদি (z == আকার (পূর্ণসংখ্যা))
	    {
	       *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)*(UINT32 *)(* p_argv);
	    }
	  অন্যথা
	    {
	      যদি ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  যদি (z > 8)
		    {
		      *(unsigned পূর্ণসংখ্যা *) argp = (unsigned পূর্ণসংখ্যা)(শূন্য *)(* p_argv);
		      z = আকার(শূন্য *);
		    }
		  অন্যথা
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      অন্যথা
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  ফেরত;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  নির্বাচন (cif->rtype->type)
    {
    ক্ষেত্র FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      বিরতি;

    ক্ষেত্র FFI_TYPE_STRUCT:
      যদি (cif->rtype->আকার_2 <= 4)
	cif->flags = FFI_TYPE_INT;

      অন্যথা যদি (cif->rtype->আকার_2 <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      অন্যথা
	cif->flags = (unsigned) cif->rtype->type;
      বিরতি;

    ক্ষেত্র FFI_TYPE_SINT64:
    ক্ষেত্র FFI_TYPE_UINT64:
    ক্ষেত্র FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      বিরতি;

    ক্ষেত্র FFI_TYPE_FLOAT:
    পূর্বনির্ধারিত:
      cif->flags = FFI_TYPE_INT;
      বিরতি;
    }

  ফেরত FFI_OK;
}

বাহ্যিক শূন্য ffi_call_SYSV(শূন্য (*)(অক্ষর *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, শূন্য (*fn)(শূন্য));

শূন্য ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  শূন্য **avalue_copy = ফাঁকা;
  পূর্ণসংখ্যা i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  যদি ((rvalue == ফাঁকা) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->আকার_2);
    }
  অন্যথা
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  জন্য (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      পূর্ণসংখ্যা আকার_2 = at->আকার_2;
      যদি (at->type == FFI_TYPE_STRUCT && আকার_2 > 4)
        {
          অক্ষর *argcopy = alloca (আকার_2);
          যদি (avalue_copy == ফাঁকা)
            {
              avalue_copy = alloca (nargs * আকার (শূন্য *));
              memcpy (avalue_copy, avalue, nargs * আকার (শূন্য *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], আকার_2);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  নির্বাচন (cif->abi)
    {
    ক্ষেত্র FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      যদি (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  পূর্ণসংখ্যা আকার_2 = cif->rtype->আকার_2;
	  পূর্ণসংখ্যা align = cif->rtype->alignment;

	  যদি (আকার_2 < 4)
	    {
	      যদি (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - আকার_2) * 8;
	    }
	  অন্যথা যদি (4 < আকার_2 && আকার_2 < 8)
	    {
	      যদি (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-আকার_2, আকার_2);
		}
	      অন্যথা যদি (align == 2)
		{
		  যদি (আকার_2 & 1)
		    আকার_2 += 1;

		  যদি (আকার_2 != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-আকার_2, আকার_2);
		}
	    }
	}
      বিরতি;

    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }
}

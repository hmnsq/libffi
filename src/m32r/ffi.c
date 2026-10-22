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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, extended_cif *ecif)
{
  గుర్తులేని పూర్ణాంకం i;
  పూర్ణాంకం tmp;
  గుర్తులేని పూర్ణాంకం avn;
  శూన్య **p_argv;
  అక్షరం *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = దొంతర;

  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->కొలత > 8)
    {
      *(శూన్య **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  కొరకు (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ఒకవేళ (((*p_arg)->alignment - 1) & (గుర్తులేని) argp)
	argp = (అక్షరం *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ఒకవేళ (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->కొలత;
	  ఒకవేళ (z < పరిమాణం (పూర్ణాంకం))
	    {
	      z = పరిమాణం (పూర్ణాంకం);

	      ఎంపిక ((*p_arg)->type)
		{
		స్థితి FFI_TYPE_SINT8:
		  *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT8 *)(* p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_UINT8:
		  *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT8 *)(* p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_SINT16:
		  *(గుర్తుగల పూర్ణాంకం *) argp = (గుర్తుగల పూర్ణాంకం)*(SINT16 *)(* p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_UINT16:
		  *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT16 *)(* p_argv);
		  ఆపు;

		స్థితి FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->కొలత;
	  	  ఒకవేళ ((*p_arg)->alignment != 1)
		    జ్ఞాపకం_నకలు (argp, *p_argv, z);
		  లేకపోతే
		    జ్ఞాపకం_నకలు (argp + 4 - z, *p_argv, z);
	  	  z = పరిమాణం (పూర్ణాంకం);
		  ఆపు;

		అప్రమేయ:
		  FFI_ASSERT(0);
		}
	    }
	  లేకపోతే ఒకవేళ (z == పరిమాణం (పూర్ణాంకం))
	    {
	       *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)*(UINT32 *)(* p_argv);
	    }
	  లేకపోతే
	    {
	      ఒకవేళ ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ఒకవేళ (z > 8)
		    {
		      *(గుర్తులేని పూర్ణాంకం *) argp = (గుర్తులేని పూర్ణాంకం)(శూన్య *)(* p_argv);
		      z = పరిమాణం(శూన్య *);
		    }
		  లేకపోతే
		    {
	              జ్ఞాపకం_నకలు(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      లేకపోతే
	        {
		  /* Double or long long 64bit.  */
	          జ్ఞాపకం_నకలు (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  తిరిగి;
}

/* Perform machine dependent cif processing.  */
ffi_స్థితి
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  ఎంపిక (cif->rtype->type)
    {
    స్థితి FFI_TYPE_VOID:
      cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

    స్థితి FFI_TYPE_STRUCT:
      ఒకవేళ (cif->rtype->కొలత <= 4)
	cif->flags = FFI_TYPE_INT;

      లేకపోతే ఒకవేళ (cif->rtype->కొలత <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      లేకపోతే
	cif->flags = (గుర్తులేని) cif->rtype->type;
      ఆపు;

    స్థితి FFI_TYPE_SINT64:
    స్థితి FFI_TYPE_UINT64:
    స్థితి FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      ఆపు;

    స్థితి FFI_TYPE_FLOAT:
    అప్రమేయ:
      cif->flags = FFI_TYPE_INT;
      ఆపు;
    }

  తిరిగి FFI_OK;
}

బాహ్య శూన్య ffi_call_SYSV(శూన్య (*)(అక్షరం *, extended_cif *), extended_cif *,
			  గుర్తులేని, గుర్తులేని, గుర్తులేని *, శూన్య (*fn)(శూన్య));

శూన్య ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  శూన్య **avalue_copy = ఖాళీ;
  పూర్ణాంకం i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ఒకవేళ ((rvalue == ఖాళీ) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->కొలత);
    }
  లేకపోతే
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  కొరకు (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      పూర్ణాంకం కొలత = at->కొలత;
      ఒకవేళ (at->type == FFI_TYPE_STRUCT && కొలత > 4)
        {
          అక్షరం *argcopy = alloca (కొలత);
          ఒకవేళ (avalue_copy == ఖాళీ)
            {
              avalue_copy = alloca (nargs * పరిమాణం (శూన్య *));
              జ్ఞాపకం_నకలు (avalue_copy, avalue, nargs * పరిమాణం (శూన్య *));
              avalue = avalue_copy;
            }
          జ్ఞాపకం_నకలు (argcopy, avalue[i], కొలత);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  ఎంపిక (cif->abi)
    {
    స్థితి FFI_SYSV:
      ffi_call_SYSV(ffi_prep_వాదనలు, &ecif, cif->బైట్లు,
		    cif->flags, ecif.rvalue, fn);
      ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  పూర్ణాంకం కొలత = cif->rtype->కొలత;
	  పూర్ణాంకం align = cif->rtype->alignment;

	  ఒకవేళ (కొలత < 4)
	    {
	      ఒకవేళ (align == 1)
	        *(గుర్తులేని దీర్ఘ *)(ecif.rvalue) <<= (4 - కొలత) * 8;
	    }
	  లేకపోతే ఒకవేళ (4 < కొలత && కొలత < 8)
	    {
	      ఒకవేళ (align == 1)
		{
		  జ్ఞాపకం_నకలు (ecif.rvalue, ecif.rvalue + 8-కొలత, కొలత);
		}
	      లేకపోతే ఒకవేళ (align == 2)
		{
		  ఒకవేళ (కొలత & 1)
		    కొలత += 1;

		  ఒకవేళ (కొలత != 8)
		    జ్ఞాపకం_నకలు (ecif.rvalue, ecif.rvalue + 8-కొలత, కొలత);
		}
	    }
	}
      ఆపు;

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}

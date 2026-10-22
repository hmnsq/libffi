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

#include <ffi.h>
#include <ffi_common.h>

#include <stdlib.h>

/* ffi_prep_args is called by the assembly routine once stack
   space has been allocated for the function's arguments.  */

void ffi_prep_args(char *stack, extended_cif *ecif)
{
  unsigned цео i;
  цео tmp;
  unsigned цео avn;
  void **p_argv;
  char *argp;
  ffi_type **p_arg;

  tmp = 0;
  argp = stack;

  ако (ecif->cif->rtype->type == FFI_TYPE_STRUCT && ecif->cif->rtype->size > 8)
    {
      *(void **) argp = ecif->rvalue;
      argp += 4;
    }

  avn = ecif->cif->nargs;
  p_argv = ecif->avalue;

  за (i = ecif->cif->nargs, p_arg = ecif->cif->arg_types;
       (i != 0) && (avn != 0);
       i--, p_arg++)
    {
      size_t z;

      /* Align if necessary.  */
      ако (((*p_arg)->alignment - 1) & (unsigned) argp)
	argp = (char *) FFI_ALIGN (argp, (*p_arg)->alignment);

      ако (avn != 0)
	{
	  avn--;
	  z = (*p_arg)->size;
	  ако (z < sizeof (цео))
	    {
	      z = sizeof (цео);

	      switch ((*p_arg)->type)
		{
		case FFI_TYPE_SINT8:
		  *(signed цео *) argp = (signed цео)*(SINT8 *)(* p_argv);
		  break;

		case FFI_TYPE_UINT8:
		  *(unsigned цео *) argp = (unsigned цео)*(UINT8 *)(* p_argv);
		  break;

		case FFI_TYPE_SINT16:
		  *(signed цео *) argp = (signed цео)*(SINT16 *)(* p_argv);
		  break;

		case FFI_TYPE_UINT16:
		  *(unsigned цео *) argp = (unsigned цео)*(UINT16 *)(* p_argv);
		  break;

		case FFI_TYPE_STRUCT:
	  	  z = (*p_arg)->size;
	  	  ако ((*p_arg)->alignment != 1)
		    memcpy (argp, *p_argv, z);
		  инако
		    memcpy (argp + 4 - z, *p_argv, z);
	  	  z = sizeof (цео);
		  break;

		default:
		  FFI_ASSERT(0);
		}
	    }
	  инако ако (z == sizeof (цео))
	    {
	       *(unsigned цео *) argp = (unsigned цео)*(UINT32 *)(* p_argv);
	    }
	  инако
	    {
	      ако ((*p_arg)->type == FFI_TYPE_STRUCT)
	        {
		  ако (z > 8)
		    {
		      *(unsigned цео *) argp = (unsigned цео)(void *)(* p_argv);
		      z = sizeof(void *);
		    }
		  инако
		    {
	              memcpy(argp, *p_argv, z);
		      z = 8;
		    }
	        }
	      инако
	        {
		  /* Double or long long 64bit.  */
	          memcpy (argp, *p_argv, z);
	        }
	    }
	  p_argv++;
	  argp += z;
	}
    }

  врати;
}

/* Perform machine dependent cif processing.  */
ffi_status
ffi_prep_cif_machdep(ffi_cif *cif)
{
  /* Set the return type flag.  */
  switch (cif->rtype->type)
    {
    case FFI_TYPE_VOID:
      cif->flags = (unsigned) cif->rtype->type;
      break;

    case FFI_TYPE_STRUCT:
      ако (cif->rtype->size <= 4)
	cif->flags = FFI_TYPE_INT;

      инако ако (cif->rtype->size <= 8)
	cif->flags = FFI_TYPE_DOUBLE;

      инако
	cif->flags = (unsigned) cif->rtype->type;
      break;

    case FFI_TYPE_SINT64:
    case FFI_TYPE_UINT64:
    case FFI_TYPE_DOUBLE:
      cif->flags = FFI_TYPE_DOUBLE;
      break;

    case FFI_TYPE_FLOAT:
    default:
      cif->flags = FFI_TYPE_INT;
      break;
    }

  врати FFI_OK;
}

extern void ffi_call_SYSV(void (*)(char *, extended_cif *), extended_cif *,
			  unsigned, unsigned, unsigned *, void (*fn)(void));

void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  extended_cif ecif;
  ffi_type **arg_types = cif->arg_types;
  void **avalue_copy = NULL;
  цео i, nargs = cif->nargs;

  ecif.cif = cif;

  /* If the return value is a struct and we don't have
     a return value address then we need to make one.  */
  ако ((rvalue == NULL) &&
      (cif->rtype->type == FFI_TYPE_STRUCT))
    {
      ecif.rvalue = alloca (cif->rtype->size);
    }
  инако
    ecif.rvalue = rvalue;

  /* If we have any large structure arguments, make a copy so we are passing
     by value.  The pointer array is cloned first: the caller owns avalue[]
     and may reuse it for another call, so it must not be modified.  */
  за (i = 0; i < nargs; i++)
    {
      ffi_type *at = arg_types[i];
      цео size = at->size;
      ако (at->type == FFI_TYPE_STRUCT && size > 4)
        {
          char *argcopy = alloca (size);
          ако (avalue_copy == NULL)
            {
              avalue_copy = alloca (nargs * sizeof (void *));
              memcpy (avalue_copy, avalue, nargs * sizeof (void *));
              avalue = avalue_copy;
            }
          memcpy (argcopy, avalue[i], size);
          avalue[i] = argcopy;
        }
    }
  ecif.avalue = avalue;

  switch (cif->abi)
    {
    case FFI_SYSV:
      ffi_call_SYSV(ffi_prep_args, &ecif, cif->bytes,
		    cif->flags, ecif.rvalue, fn);
      ако (cif->rtype->type == FFI_TYPE_STRUCT)
	{
	  цео size = cif->rtype->size;
	  цео align = cif->rtype->alignment;

	  ако (size < 4)
	    {
	      ако (align == 1)
	        *(unsigned long *)(ecif.rvalue) <<= (4 - size) * 8;
	    }
	  инако ако (4 < size && size < 8)
	    {
	      ако (align == 1)
		{
		  memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	      инако ако (align == 2)
		{
		  ако (size & 1)
		    size += 1;

		  ако (size != 8)
		    memcpy (ecif.rvalue, ecif.rvalue + 8-size, size);
		}
	    }
	}
      break;

    default:
      FFI_ASSERT(0);
      break;
    }
}

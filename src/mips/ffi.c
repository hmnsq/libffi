/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2011  Anthony Green
           Copyright (c) 2008  David Daney
           Copyright (c) 1996, 2007, 2008, 2011  Red Hat, Inc.
   
   MIPS Foreign Function Interface 

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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

#ifdef __GNUC__
#  सचे (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  सचे defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  अञ्ञथा
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#अञ्ञथा
# define FFI_MIPS_STOP_HERE() करोहि {} याव(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &राशिः[bytes]); \
सचे (argp == &राशिः[bytes]) \
{ \
  argp = राशिः; \
  FFI_MIPS_STOP_HERE(); \
}
#अञ्ञथा
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

स्थिर शून्य ffi_prep_args(वर्ण *राशिः, 
			  extended_cif *ecif,
			  पूर्णाङ्क bytes,
			  पूर्णाङ्क flags)
{
  पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp, *argp_f;
  ffi_type **p_arg;

  memset(राशिः, 0, bytes);

#ifdef FFI_MIPS_N32
  पूर्णाङ्क soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  सचे (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      सचे (bytes + 16 > 8 * परिमाण(ffi_arg))
        argp = &राशिः[bytes - (8 * परिमाण(ffi_arg))];
      अञ्ञथा
        argp = राशिः;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  अञ्ञथा
    {
      सचे (bytes > 8 * परिमाण(ffi_arg))
        argp = &राशिः[bytes - (8 * परिमाण(ffi_arg))];
      अञ्ञथा
        argp = राशिः;
    }
#अञ्ञथा
  argp = राशिः;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  सचे ( ecif->cif->rstruct_flag != 0 )
#अञ्ञथा
  सचे ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += परिमाण(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  अत्थाय (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned पूर्णाङ्क a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      सचे (a < परिमाण(ffi_arg))
        a = परिमाण(ffi_arg);
      
      सचे ((a - 1) & (unsigned long) argp)
	{
	  argp = (वर्ण *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      सचे (z <= परिमाण(ffi_arg))
	{
          पूर्णाङ्क type = (*p_arg)->type;
	  z = परिमाण(ffi_arg);

          /* The size of a pointer depends on the ABI */
          सचे (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	सचे (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    विकल्प (type)
	      {
	      पक्ष FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		भङ्ग;
	      पक्ष FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		भङ्ग;
	      स्वभाव:
		भङ्ग;
	      }
	  }
	  विकल्प (type)
	    {
	      पक्ष FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		भङ्ग;

	      पक्ष FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#अञ्ञथा
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		भङ्ग;

#ifdef FFI_MIPS_N32
	      पक्ष FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        सचे(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(भिन्न *) argp = *(भिन्न *)(* p_argv);
		    argp += z;
		    वर्ण *tmp = (शून्य *) (*p_argv);
		    *(भिन्न *) argp = *(भिन्न *)(tmp+4);
		  }
		अञ्ञथा
		  memcpy(argp, *p_argv, (*p_arg)->size);
		भङ्ग;
#endif
	      /* This can only happen with 64bit slots.  */
	      पक्ष FFI_TYPE_FLOAT:
		*(भिन्न *) argp = *(भिन्न *)(* p_argv);
		भङ्ग;

	      /* Handle structures.  */
	      स्वभाव:
		memcpy(argp, *p_argv, (*p_arg)->size);
		भङ्ग;
	    }
	}
      अञ्ञथा
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#अञ्ञथा
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) राशिः + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    सचे (end <= cap)
	      memcpy(argp, *p_argv, z);
	    अञ्ञथा
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = राशिः;
                z -= portion;
		memcpy(argp, (शून्य*)((unsigned long)(*p_argv) + portion),
                       z);
	      }
	  }
#endif
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#ifdef FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

स्थिर पूर्णाङ्क
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  सचे (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      सचे (*arg_reg > 7)
	पटिनिवत्त 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  पटिनिवत्त 0;
}

स्थिर unsigned
calc_n32_struct_flags(पूर्णाङ्क soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  सचे (soft_float)
    पटिनिवत्त 0;

  याव ((e = arg->elements[index]))
    {
      सचे (e->type == FFI_TYPE_COMPLEX)
	{
	  सचे (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    भङ्ग;
	  सचे (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    भङ्ग;
	}
      अञ्ञथा
	सचे (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  भङ्ग;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  पटिनिवत्त flags;
}

स्थिर unsigned
calc_n32_return_struct_flags(पूर्णाङ्क soft_float, ffi_type *arg)
{
  unsigned flags;
  unsigned small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  सचे (arg->size > 16)
    पटिनिवत्त 0;

  सचे (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  सचे (e->type == FFI_TYPE_COMPLEX)
    {
      पूर्णाङ्क type = e->elements[0]->type;

      सचे (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	पटिनिवत्त small;

      सचे (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  पटिनिवत्त small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  अञ्ञथा
    {
      सचे (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	पटिनिवत्त small;

      flags = e->type;

      सचे (arg->elements[1])
	{
	  e = arg->elements[1];
	  सचे (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    पटिनिवत्त small;

	  सचे (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      पटिनिवत्त small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  सचे (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  पटिनिवत्त flags;
}

#endif

/* Perform machine dependent cif processing */
स्थिर ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  सचे (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      सचे (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  विकल्प ((cif->arg_types)[0]->type)
	    {
	    पक्ष FFI_TYPE_FLOAT:
	    पक्ष FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      भङ्ग;
	      
	    स्वभाव:
	      भङ्ग;
	    }

	  सचे (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      सचे (cif->flags)
		{
		  विकल्प ((cif->arg_types)[1]->type)
		    {
		    पक्ष FFI_TYPE_FLOAT:
		    पक्ष FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      भङ्ग;
		      
		    स्वभाव:
		      भङ्ग;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  सचे (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_VOID:
        पक्ष FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          भङ्ग;

        पक्ष FFI_TYPE_SINT64:
        पक्ष FFI_TYPE_UINT64:
        पक्ष FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          भङ्ग;
      
        पक्ष FFI_TYPE_FLOAT:
        स्वभाव:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          भङ्ग;
        }
    }
  अञ्ञथा
    {
      /* FFI_O32 */      
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_VOID:
        पक्ष FFI_TYPE_STRUCT:
        पक्ष FFI_TYPE_FLOAT:
        पक्ष FFI_TYPE_DOUBLE:
        पक्ष FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  सचे (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          भङ्ग;

        पक्ष FFI_TYPE_SINT64:
        पक्ष FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          भङ्ग;
      
        स्वभाव:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          भङ्ग;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned गणनम् = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    पूर्णाङ्क soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    सचे (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	सचे (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    गणनम् = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	अञ्ञथा
	    cif->rstruct_flag = 0;
      }
    अञ्ञथा
      cif->rstruct_flag = 0;

    याव (गणनम्-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	विकल्प (t->type)
	  {
	  पक्ष FFI_TYPE_FLOAT:
	  पक्ष FFI_TYPE_DOUBLE:
	    सचे (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    भङ्ग;
          पक्ष FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    सचे (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    अञ्ञथा
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		सचे (arg_reg >= 8)
		  अनुवर्त;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            भङ्ग;

	  पक्ष FFI_TYPE_COMPLEX:
	    विकल्प (t->elements[0]->type)
	      {
	      पक्ष FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		सचे (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अञ्ञथा
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    सचे (arg_reg >= 8)
		        अनुवर्त;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    सचे (arg_reg >= 8)
		        अनुवर्त;
		  }
		/* passthrough */
	      पक्ष FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      पक्ष FFI_TYPE_SINT32:
	      पक्ष FFI_TYPE_UINT32:
	      पक्ष FFI_TYPE_DOUBLE:
		सचे (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अञ्ञथा
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    सचे (arg_reg >= 8)
		        अनुवर्त;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		भङ्ग;
	      स्वभाव:
		arg_reg += 2;
		भङ्ग;
	      }
	    भङ्ग;

	  पक्ष FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    भङ्ग;

	  स्वभाव:
	    arg_reg++;
            भङ्ग;
	  }

	index++;
      }

  /* Set the return type flag */
    विकल्प (cif->rtype->type)
      {
      पक्ष FFI_TYPE_STRUCT:
	{
	  सचे (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  अञ्ञथा
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  भङ्ग;
	}
      
      पक्ष FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	भङ्ग;

      पक्ष FFI_TYPE_POINTER:
	सचे (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	अञ्ञथा
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	भङ्ग;

      पक्ष FFI_TYPE_FLOAT:
	सचे (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    भङ्ग;
	  }
	/* else fall through */
      पक्ष FFI_TYPE_DOUBLE:
	सचे (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	अञ्ञथा
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	भङ्ग;

      पक्ष FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	सचे (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	अञ्ञथा
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	भङ्ग;
      पक्ष FFI_TYPE_COMPLEX:
	{
	  पूर्णाङ्क type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  सचे (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      विकल्प (type)
		{
		पक्ष FFI_TYPE_DOUBLE:
		पक्ष FFI_TYPE_SINT64:
		पक्ष FFI_TYPE_UINT64:
		पक्ष FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  भङ्ग;
		पक्ष FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  भङ्ग;
		पक्ष FFI_TYPE_FLOAT:
		स्वभाव:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  अञ्ञथा
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  भङ्ग;
	}
      पक्ष FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	भङ्ग;
      पक्ष FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	भङ्ग;
      स्वभाव:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	भङ्ग;
      }
  }
#endif
  पटिनिवत्त FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    पटिनिवत्त ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    पटिनिवत्त ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
बाह्य पूर्णाङ्क ffi_call_O32(शून्य (*)(वर्ण *, extended_cif *, पूर्णाङ्क, पूर्णाङ्क), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, शून्य (*)(शून्य), शून्य *closure);

/* Low level routine for calling N32 functions */
बाह्य पूर्णाङ्क ffi_call_N32(शून्य (*)(वर्ण *, extended_cif *, पूर्णाङ्क, पूर्णाङ्क), 
			extended_cif *, unsigned, 
			unsigned, शून्य *, शून्य (*)(शून्य), शून्य *closure);

शून्य ffi_call_int(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, 
	      शून्य **avalue, शून्य *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  सचे ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  अञ्ञथा
    ecif.rvalue = rvalue;
    
  विकल्प (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    पक्ष FFI_O32:
    पक्ष FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      भङ्ग;
#endif

#ifdef FFI_MIPS_N32
    पक्ष FFI_N32:
    पक्ष FFI_N32_SOFT_FLOAT:
    पक्ष FFI_N64:
    पक्ष FFI_N64_SOFT_FLOAT:
      {
        पूर्णाङ्क copy_rvalue = 0;
	पूर्णाङ्क copy_offset = 0;
        वर्ण *rvalue_copy = ecif.rvalue;
        सचे (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	अञ्ञथा सचे (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#सचे defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        सचे (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      भङ्ग;
#endif

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#सचे FFI_CLOSURES
#सचे defined(FFI_MIPS_O32)
बाह्य शून्य ffi_closure_O32(शून्य);
बाह्य शून्य ffi_go_closure_O32(शून्य);
#अञ्ञथा
बाह्य शून्य ffi_closure_N32(शून्य);
बाह्य शून्य ffi_go_closure_N32(शून्य);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  unsigned पूर्णाङ्क *tramp = (unsigned पूर्णाङ्क *) &closure->tramp[0];
  शून्य * fn;
  वर्ण *clear_location = (वर्ण *) codeloc;

#सचे defined(FFI_MIPS_O32)
  सचे (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
  fn = ffi_closure_O32;
#अञ्ञथा
#सचे _MIPS_SIM ==_ABIN32
  सचे (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
#अञ्ञथा
  सचे (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#सचे defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#सचे !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#अञ्ञथा
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#अञ्ञथा
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned long)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((unsigned long)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((unsigned long)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((unsigned long)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((unsigned long)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((unsigned long)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((unsigned long)fn  & 0xffff);
  /* jr   $25          */
#सचे !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#अञ्ञथा
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#सचे !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#अञ्ञथा
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  पटिनिवत्त FFI_OK;
}

/*
 * Decodes the arguments to a function, which will be stored on the
 * stack. AR is the pointer to the beginning of the integer arguments
 * (and, depending upon the arguments, some floating-point arguments
 * as well). FPR is a pointer to the area where floating point
 * registers have been saved, if any.
 *
 * RVALUE is the location where the function return value will be
 * stored. CLOSURE is the prepared closure to invoke.
 *
 * This function should only be called from assembly, which is in
 * turn called from a trampoline.
 *
 * Returns the function return type.
 *
 * Based on the similar routine for sparc.
 */
पूर्णाङ्क
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			    शून्य *user_data,
			    शून्य *rvalue, ffi_arg *ar,
			    द्विगुण *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णाङ्क i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * परिमाण (ffi_arg));
  avaluep = alloca (cif->nargs * परिमाण (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  सचे ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (शून्य *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  सचे ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  याव (i < avn)
    {
      सचे (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      सचे (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#सचे defined(__MIPSEB__) || defined(_MIPSEB)
	  सचे (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((वर्ण *) &fpr[i]) + परिमाण (भिन्न);
	  अञ्ञथा
#endif
	    avaluep[i] = (वर्ण *) &fpr[i];
	}
      अञ्ञथा
	{
	  विकल्प (arg_types[i]->type)
	    {
	      पक्ष FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		भङ्ग;

	      पक्ष FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		भङ्ग;

	      स्वभाव:
		avaluep[i] = (वर्ण *) &ar[argn];
		भङ्ग;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  सचे (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_FLOAT:
          पटिनिवत्त FFI_TYPE_INT;
        पक्ष FFI_TYPE_DOUBLE:
          पटिनिवत्त FFI_TYPE_UINT64;
        स्वभाव:
          पटिनिवत्त cif->rtype->type;
        }
    }
  अञ्ञथा
    {
      सचे (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ चञ्चल ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      पटिनिवत्त cif->rtype->type;
    }
}

#सचे defined(FFI_MIPS_N32)

स्थिर शून्य
copy_struct_N32(वर्ण *target, unsigned offset, ffi_abi abi, ffi_type *type,
                पूर्णाङ्क argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, पूर्णाङ्क soft_float)
{
  ffi_type **elt_typep = type->elements;
  याव(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      वर्ण *tp;
      वर्ण *argp;
      वर्ण *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / परिमाण(ffi_arg);
      arg_offset = arg_offset % परिमाण(ffi_arg);

      argp = (वर्ण *)(ar + argn);
      fpp = (वर्ण *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      सचे (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(द्विगुण *)tp = *(द्विगुण *)fpp;
      अञ्ञथा
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / परिमाण(ffi_arg);
      arg_offset = arg_offset % परिमाण(ffi_arg);
    }
}

/*
 * Decodes the arguments to a function, which will be stored on the
 * stack. AR is the pointer to the beginning of the integer
 * arguments. FPR is a pointer to the area where floating point
 * registers have been saved.
 *
 * RVALUE is the location where the function return value will be
 * stored. CLOSURE is the prepared closure to invoke.
 *
 * This function should only be called from assembly, which is in
 * turn called from a trampoline.
 *
 * Returns the function return flags.
 *
 */
पूर्णाङ्क
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                            शून्य *user_data,
			    शून्य *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णाङ्क i, avn, argn;
  पूर्णाङ्क soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * परिमाण (ffi_arg));
  avaluep = alloca (cif->nargs * परिमाण (ffi_arg));

  argn = 0;

  सचे (cif->rstruct_flag)
    {
#सचे _MIPS_SIM==_ABIN32
      rvalue = (शून्य *)(UINT32)ar[0];
#अञ्ञथा /* N64 */
      rvalue = (शून्य *)ar[0];
#endif
      argn = 1;
    }
  सचे (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  याव (i < avn)
    {
      सचे (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          सचे ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#सचे defined(__MIPSEB__) || defined(_MIPSEB)
          सचे (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((वर्ण *) argp) + परिमाण (भिन्न);
          अञ्ञथा
#endif
            avaluep[i] = (वर्ण *) argp;
        }
      अञ्ञथा सचे (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (वर्ण *) argp;
        }
      अञ्ञथा सचे (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (वर्ण *) argp;
        }
      अञ्ञथा सचे (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          सचे (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  अञ्ञथा
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (वर्ण *) argp;
        }
      अञ्ञथा
        {
          unsigned type = arg_types[i]->type;

          सचे (arg_types[i]->alignment > परिमाण(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / परिमाण(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          सचे (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  सचे (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          विकल्प (type)
            {
            पक्ष FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_STRUCT:
              सचे (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  भङ्ग;
                }
              /* Else fall through.  */
            स्वभाव:
              avaluep[i] = (वर्ण *) argp;
              भङ्ग;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, परिमाण(ffi_arg)) / परिमाण(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  पटिनिवत्त cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#सचे defined(FFI_MIPS_O32)
बाह्य शून्य ffi_closure_O32(शून्य);
बाह्य शून्य ffi_go_closure_O32(शून्य);
#अञ्ञथा
बाह्य शून्य ffi_closure_N32(शून्य);
बाह्य शून्य ffi_go_closure_N32(शून्य);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य * fn;

#सचे defined(FFI_MIPS_O32)
  सचे (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#अञ्ञथा
#सचे _MIPS_SIM ==_ABIN32
  सचे (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
#अञ्ञथा
  सचे (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    पटिनिवत्त FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (शून्य *)fn;
  closure->cif = cif;
  closure->fun = fun;

  पटिनिवत्त FFI_OK;
}

#endif /* FFI_CLOSURES */

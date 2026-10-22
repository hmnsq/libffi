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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>

#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <stdio.h>

#ifdef __GNUC__
#  எனில் (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  எனில் defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  இல்லையெனில்
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#இல்லையெனில்
# define FFI_MIPS_STOP_HERE() செய் {} போது(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &அடுக்கு[bytes]); \
எனில் (argp == &அடுக்கு[bytes]) \
{ \
  argp = அடுக்கு; \
  FFI_MIPS_STOP_HERE(); \
}
#இல்லையெனில்
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

நிலையான வெறுமை ffi_prep_args(எழுத்து *அடுக்கு, 
			  extended_cif *ecif,
			  முழு bytes,
			  முழு flags)
{
  முழு i;
  வெறுமை **p_argv;
  எழுத்து *argp, *argp_f;
  ffi_type **p_arg;

  memset(அடுக்கு, 0, bytes);

#ifdef FFI_MIPS_N32
  முழு soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  எனில் (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      எனில் (bytes + 16 > 8 * அளவு(ffi_arg))
        argp = &அடுக்கு[bytes - (8 * அளவு(ffi_arg))];
      இல்லையெனில்
        argp = அடுக்கு;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  இல்லையெனில்
    {
      எனில் (bytes > 8 * அளவு(ffi_arg))
        argp = &அடுக்கு[bytes - (8 * அளவு(ffi_arg))];
      இல்லையெனில்
        argp = அடுக்கு;
    }
#இல்லையெனில்
  argp = அடுக்கு;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  எனில் ( ecif->cif->rstruct_flag != 0 )
#இல்லையெனில்
  எனில் ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += அளவு(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  க்கு (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned முழு a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      எனில் (a < அளவு(ffi_arg))
        a = அளவு(ffi_arg);
      
      எனில் ((a - 1) & (unsigned long) argp)
	{
	  argp = (எழுத்து *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      எனில் (z <= அளவு(ffi_arg))
	{
          முழு type = (*p_arg)->type;
	  z = அளவு(ffi_arg);

          /* The size of a pointer depends on the ABI */
          எனில் (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	எனில் (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    தேர்வு (type)
	      {
	      நிலை FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		நிறுத்து;
	      நிலை FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		நிறுத்து;
	      இயல்பு:
		நிறுத்து;
	      }
	  }
	  தேர்வு (type)
	    {
	      நிலை FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		நிறுத்து;

	      நிலை FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		நிறுத்து;
		  
	      நிலை FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		நிறுத்து;
		  
	      நிலை FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		நிறுத்து;
		  
	      நிலை FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		நிறுத்து;
		  
	      நிலை FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#இல்லையெனில்
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		நிறுத்து;

#ifdef FFI_MIPS_N32
	      நிலை FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        எனில்(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(தசம *) argp = *(தசம *)(* p_argv);
		    argp += z;
		    எழுத்து *tmp = (வெறுமை *) (*p_argv);
		    *(தசம *) argp = *(தசம *)(tmp+4);
		  }
		இல்லையெனில்
		  memcpy(argp, *p_argv, (*p_arg)->size);
		நிறுத்து;
#endif
	      /* This can only happen with 64bit slots.  */
	      நிலை FFI_TYPE_FLOAT:
		*(தசம *) argp = *(தசம *)(* p_argv);
		நிறுத்து;

	      /* Handle structures.  */
	      இயல்பு:
		memcpy(argp, *p_argv, (*p_arg)->size);
		நிறுத்து;
	    }
	}
      இல்லையெனில்
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#இல்லையெனில்
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) அடுக்கு + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    எனில் (end <= cap)
	      memcpy(argp, *p_argv, z);
	    இல்லையெனில்
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = அடுக்கு;
                z -= portion;
		memcpy(argp, (வெறுமை*)((unsigned long)(*p_argv) + portion),
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

நிலையான முழு
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  எனில் (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      எனில் (*arg_reg > 7)
	திரும்பு 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  திரும்பு 0;
}

நிலையான unsigned
calc_n32_struct_flags(முழு soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  எனில் (soft_float)
    திரும்பு 0;

  போது ((e = arg->elements[index]))
    {
      எனில் (e->type == FFI_TYPE_COMPLEX)
	{
	  எனில் (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    நிறுத்து;
	  எனில் (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    நிறுத்து;
	}
      இல்லையெனில்
	எனில் (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  நிறுத்து;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  திரும்பு flags;
}

நிலையான unsigned
calc_n32_return_struct_flags(முழு soft_float, ffi_type *arg)
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
  
  எனில் (arg->size > 16)
    திரும்பு 0;

  எனில் (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  எனில் (e->type == FFI_TYPE_COMPLEX)
    {
      முழு type = e->elements[0]->type;

      எனில் (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	திரும்பு small;

      எனில் (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  திரும்பு small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  இல்லையெனில்
    {
      எனில் (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	திரும்பு small;

      flags = e->type;

      எனில் (arg->elements[1])
	{
	  e = arg->elements[1];
	  எனில் (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    திரும்பு small;

	  எனில் (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      திரும்பு small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  எனில் (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  திரும்பு flags;
}

#endif

/* Perform machine dependent cif processing */
நிலையான ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  எனில் (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      எனில் (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  தேர்வு ((cif->arg_types)[0]->type)
	    {
	    நிலை FFI_TYPE_FLOAT:
	    நிலை FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      நிறுத்து;
	      
	    இயல்பு:
	      நிறுத்து;
	    }

	  எனில் (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      எனில் (cif->flags)
		{
		  தேர்வு ((cif->arg_types)[1]->type)
		    {
		    நிலை FFI_TYPE_FLOAT:
		    நிலை FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      நிறுத்து;
		      
		    இயல்பு:
		      நிறுத்து;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  எனில் (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      தேர்வு (cif->rtype->type)
        {
        நிலை FFI_TYPE_VOID:
        நிலை FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          நிறுத்து;

        நிலை FFI_TYPE_SINT64:
        நிலை FFI_TYPE_UINT64:
        நிலை FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          நிறுத்து;
      
        நிலை FFI_TYPE_FLOAT:
        இயல்பு:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          நிறுத்து;
        }
    }
  இல்லையெனில்
    {
      /* FFI_O32 */      
      தேர்வு (cif->rtype->type)
        {
        நிலை FFI_TYPE_VOID:
        நிலை FFI_TYPE_STRUCT:
        நிலை FFI_TYPE_FLOAT:
        நிலை FFI_TYPE_DOUBLE:
        நிலை FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  எனில் (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          நிறுத்து;

        நிலை FFI_TYPE_SINT64:
        நிலை FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          நிறுத்து;
      
        இயல்பு:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          நிறுத்து;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned மொத்தம் = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    முழு soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    எனில் (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	எனில் (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    மொத்தம் = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	இல்லையெனில்
	    cif->rstruct_flag = 0;
      }
    இல்லையெனில்
      cif->rstruct_flag = 0;

    போது (மொத்தம்-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	தேர்வு (t->type)
	  {
	  நிலை FFI_TYPE_FLOAT:
	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    நிறுத்து;
          நிலை FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    எனில் (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    இல்லையெனில்
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		எனில் (arg_reg >= 8)
		  தொடர்;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            நிறுத்து;

	  நிலை FFI_TYPE_COMPLEX:
	    தேர்வு (t->elements[0]->type)
	      {
	      நிலை FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		எனில் (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		இல்லையெனில்
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    எனில் (arg_reg >= 8)
		        தொடர்;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    எனில் (arg_reg >= 8)
		        தொடர்;
		  }
		/* passthrough */
	      நிலை FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      நிலை FFI_TYPE_SINT32:
	      நிலை FFI_TYPE_UINT32:
	      நிலை FFI_TYPE_DOUBLE:
		எனில் (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		இல்லையெனில்
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    எனில் (arg_reg >= 8)
		        தொடர்;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		நிறுத்து;
	      இயல்பு:
		arg_reg += 2;
		நிறுத்து;
	      }
	    நிறுத்து;

	  நிலை FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    நிறுத்து;

	  இயல்பு:
	    arg_reg++;
            நிறுத்து;
	  }

	index++;
      }

  /* Set the return type flag */
    தேர்வு (cif->rtype->type)
      {
      நிலை FFI_TYPE_STRUCT:
	{
	  எனில் (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  இல்லையெனில்
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  நிறுத்து;
	}
      
      நிலை FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	நிறுத்து;

      நிலை FFI_TYPE_POINTER:
	எனில் (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	இல்லையெனில்
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	நிறுத்து;

      நிலை FFI_TYPE_FLOAT:
	எனில் (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    நிறுத்து;
	  }
	/* else fall through */
      நிலை FFI_TYPE_DOUBLE:
	எனில் (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	இல்லையெனில்
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	நிறுத்து;

      நிலை FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	எனில் (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	இல்லையெனில்
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	நிறுத்து;
      நிலை FFI_TYPE_COMPLEX:
	{
	  முழு type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  எனில் (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      தேர்வு (type)
		{
		நிலை FFI_TYPE_DOUBLE:
		நிலை FFI_TYPE_SINT64:
		நிலை FFI_TYPE_UINT64:
		நிலை FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  நிறுத்து;
		நிலை FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  நிறுத்து;
		நிலை FFI_TYPE_FLOAT:
		இயல்பு:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  இல்லையெனில்
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  நிறுத்து;
	}
      நிலை FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	நிறுத்து;
      நிலை FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	நிறுத்து;
      இயல்பு:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	நிறுத்து;
      }
  }
#endif
  திரும்பு FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    திரும்பு ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    திரும்பு ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
வெளி முழு ffi_call_O32(வெறுமை (*)(எழுத்து *, extended_cif *, முழு, முழு), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, வெறுமை (*)(வெறுமை), வெறுமை *closure);

/* Low level routine for calling N32 functions */
வெளி முழு ffi_call_N32(வெறுமை (*)(எழுத்து *, extended_cif *, முழு, முழு), 
			extended_cif *, unsigned, 
			unsigned, வெறுமை *, வெறுமை (*)(வெறுமை), வெறுமை *closure);

வெறுமை ffi_call_int(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, 
	      வெறுமை **avalue, வெறுமை *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  எனில் ((rvalue == வெற்று) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  இல்லையெனில்
    ecif.rvalue = rvalue;
    
  தேர்வு (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    நிலை FFI_O32:
    நிலை FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      நிறுத்து;
#endif

#ifdef FFI_MIPS_N32
    நிலை FFI_N32:
    நிலை FFI_N32_SOFT_FLOAT:
    நிலை FFI_N64:
    நிலை FFI_N64_SOFT_FLOAT:
      {
        முழு copy_rvalue = 0;
	முழு copy_offset = 0;
        எழுத்து *rvalue_copy = ecif.rvalue;
        எனில் (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	இல்லையெனில் எனில் (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#எனில் defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        எனில் (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      நிறுத்து;
#endif

    இயல்பு:
      FFI_ASSERT(0);
      நிறுத்து;
    }
}

வெறுமை
ffi_call(ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#எனில் FFI_CLOSURES
#எனில் defined(FFI_MIPS_O32)
வெளி வெறுமை ffi_closure_O32(வெறுமை);
வெளி வெறுமை ffi_go_closure_O32(வெறுமை);
#இல்லையெனில்
வெளி வெறுமை ffi_closure_N32(வெறுமை);
வெளி வெறுமை ffi_go_closure_N32(வெறுமை);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  unsigned முழு *tramp = (unsigned முழு *) &closure->tramp[0];
  வெறுமை * fn;
  எழுத்து *clear_location = (எழுத்து *) codeloc;

#எனில் defined(FFI_MIPS_O32)
  எனில் (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
  fn = ffi_closure_O32;
#இல்லையெனில்
#எனில் _MIPS_SIM ==_ABIN32
  எனில் (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
#இல்லையெனில்
  எனில் (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#எனில் defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#எனில் !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#இல்லையெனில்
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#இல்லையெனில்
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
#எனில் !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#இல்லையெனில்
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#எனில் !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#இல்லையெனில்
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  திரும்பு FFI_OK;
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
முழு
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
			    வெறுமை *user_data,
			    வெறுமை *rvalue, ffi_arg *ar,
			    இரட்டை *fpr)
{
  வெறுமை **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  முழு i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * அளவு (ffi_arg));
  avaluep = alloca (cif->nargs * அளவு (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  எனில் ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (வெறுமை *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  எனில் ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  போது (i < avn)
    {
      எனில் (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      எனில் (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#எனில் defined(__MIPSEB__) || defined(_MIPSEB)
	  எனில் (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((எழுத்து *) &fpr[i]) + அளவு (தசம);
	  இல்லையெனில்
#endif
	    avaluep[i] = (எழுத்து *) &fpr[i];
	}
      இல்லையெனில்
	{
	  தேர்வு (arg_types[i]->type)
	    {
	      நிலை FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		நிறுத்து;

	      நிலை FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		நிறுத்து;
		  
	      நிலை FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		நிறுத்து;
		  
	      நிலை FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		நிறுத்து;

	      இயல்பு:
		avaluep[i] = (எழுத்து *) &ar[argn];
		நிறுத்து;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  எனில் (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      தேர்வு (cif->rtype->type)
        {
        நிலை FFI_TYPE_FLOAT:
          திரும்பு FFI_TYPE_INT;
        நிலை FFI_TYPE_DOUBLE:
          திரும்பு FFI_TYPE_UINT64;
        இயல்பு:
          திரும்பு cif->rtype->type;
        }
    }
  இல்லையெனில்
    {
      எனில் (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ நிலையற்ற ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      திரும்பு cif->rtype->type;
    }
}

#எனில் defined(FFI_MIPS_N32)

நிலையான வெறுமை
copy_struct_N32(எழுத்து *target, unsigned offset, ffi_abi abi, ffi_type *type,
                முழு argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, முழு soft_float)
{
  ffi_type **elt_typep = type->elements;
  போது(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      எழுத்து *tp;
      எழுத்து *argp;
      எழுத்து *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / அளவு(ffi_arg);
      arg_offset = arg_offset % அளவு(ffi_arg);

      argp = (எழுத்து *)(ar + argn);
      fpp = (எழுத்து *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      எனில் (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(இரட்டை *)tp = *(இரட்டை *)fpp;
      இல்லையெனில்
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / அளவு(ffi_arg);
      arg_offset = arg_offset % அளவு(ffi_arg);
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
முழு
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    வெறுமை (*fun)(ffi_cif*, வெறுமை*, வெறுமை**, வெறுமை*),
                            வெறுமை *user_data,
			    வெறுமை *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  வெறுமை **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  முழு i, avn, argn;
  முழு soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * அளவு (ffi_arg));
  avaluep = alloca (cif->nargs * அளவு (ffi_arg));

  argn = 0;

  எனில் (cif->rstruct_flag)
    {
#எனில் _MIPS_SIM==_ABIN32
      rvalue = (வெறுமை *)(UINT32)ar[0];
#இல்லையெனில் /* N64 */
      rvalue = (வெறுமை *)ar[0];
#endif
      argn = 1;
    }
  எனில் (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  போது (i < avn)
    {
      எனில் (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          எனில் ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#எனில் defined(__MIPSEB__) || defined(_MIPSEB)
          எனில் (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((எழுத்து *) argp) + அளவு (தசம);
          இல்லையெனில்
#endif
            avaluep[i] = (எழுத்து *) argp;
        }
      இல்லையெனில் எனில் (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (எழுத்து *) argp;
        }
      இல்லையெனில் எனில் (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (எழுத்து *) argp;
        }
      இல்லையெனில் எனில் (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          எனில் (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  இல்லையெனில்
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (எழுத்து *) argp;
        }
      இல்லையெனில்
        {
          unsigned type = arg_types[i]->type;

          எனில் (arg_types[i]->alignment > அளவு(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / அளவு(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          எனில் (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  எனில் (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          தேர்வு (type)
            {
            நிலை FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              நிறுத்து;

            நிலை FFI_TYPE_STRUCT:
              எனில் (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  நிறுத்து;
                }
              /* Else fall through.  */
            இயல்பு:
              avaluep[i] = (எழுத்து *) argp;
              நிறுத்து;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, அளவு(ffi_arg)) / அளவு(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  திரும்பு cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#எனில் defined(FFI_MIPS_O32)
வெளி வெறுமை ffi_closure_O32(வெறுமை);
வெளி வெறுமை ffi_go_closure_O32(வெறுமை);
#இல்லையெனில்
வெளி வெறுமை ffi_closure_N32(வெறுமை);
வெளி வெறுமை ffi_go_closure_N32(வெறுமை);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*))
{
  வெறுமை * fn;

#எனில் defined(FFI_MIPS_O32)
  எனில் (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#இல்லையெனில்
#எனில் _MIPS_SIM ==_ABIN32
  எனில் (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
#இல்லையெனில்
  எனில் (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    திரும்பு FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (வெறுமை *)fn;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

#endif /* FFI_CLOSURES */

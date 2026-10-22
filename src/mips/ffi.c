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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <stdio.h>

#ifdef __GNUC__
#  اگر (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  اگر defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  وگرنه
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#وگرنه
# define FFI_MIPS_STOP_HERE() انجامبده {} وقتی(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &پشته[bytes]); \
اگر (argp == &پشته[bytes]) \
{ \
  argp = پشته; \
  FFI_MIPS_STOP_HERE(); \
}
#وگرنه
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ایستا تهی ffi_prep_args(نویسه *پشته, 
			  extended_cif *ecif,
			  صحیح bytes,
			  صحیح flags)
{
  صحیح i;
  تهی **p_argv;
  نویسه *argp, *argp_f;
  ffi_type **p_arg;

  memset(پشته, 0, bytes);

#ifdef FFI_MIPS_N32
  صحیح soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  اگر (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      اگر (bytes + 16 > 8 * اندازه(ffi_arg))
        argp = &پشته[bytes - (8 * اندازه(ffi_arg))];
      وگرنه
        argp = پشته;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  وگرنه
    {
      اگر (bytes > 8 * اندازه(ffi_arg))
        argp = &پشته[bytes - (8 * اندازه(ffi_arg))];
      وگرنه
        argp = پشته;
    }
#وگرنه
  argp = پشته;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  اگر ( ecif->cif->rstruct_flag != 0 )
#وگرنه
  اگر ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += اندازه(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  برای (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned صحیح a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      اگر (a < اندازه(ffi_arg))
        a = اندازه(ffi_arg);
      
      اگر ((a - 1) & (unsigned long) argp)
	{
	  argp = (نویسه *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      اگر (z <= اندازه(ffi_arg))
	{
          صحیح type = (*p_arg)->type;
	  z = اندازه(ffi_arg);

          /* The size of a pointer depends on the ABI */
          اگر (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	اگر (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    انتخاب (type)
	      {
	      حالت FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		بشکن;
	      حالت FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		بشکن;
	      پیشفرض:
		بشکن;
	      }
	  }
	  انتخاب (type)
	    {
	      حالت FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		بشکن;

	      حالت FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		بشکن;
		  
	      حالت FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		بشکن;
		  
	      حالت FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		بشکن;
		  
	      حالت FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		بشکن;
		  
	      حالت FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#وگرنه
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		بشکن;

#ifdef FFI_MIPS_N32
	      حالت FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        اگر(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(اعشاری *) argp = *(اعشاری *)(* p_argv);
		    argp += z;
		    نویسه *tmp = (تهی *) (*p_argv);
		    *(اعشاری *) argp = *(اعشاری *)(tmp+4);
		  }
		وگرنه
		  memcpy(argp, *p_argv, (*p_arg)->size);
		بشکن;
#endif
	      /* This can only happen with 64bit slots.  */
	      حالت FFI_TYPE_FLOAT:
		*(اعشاری *) argp = *(اعشاری *)(* p_argv);
		بشکن;

	      /* Handle structures.  */
	      پیشفرض:
		memcpy(argp, *p_argv, (*p_arg)->size);
		بشکن;
	    }
	}
      وگرنه
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#وگرنه
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) پشته + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    اگر (end <= cap)
	      memcpy(argp, *p_argv, z);
	    وگرنه
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = پشته;
                z -= portion;
		memcpy(argp, (تهی*)((unsigned long)(*p_argv) + portion),
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

ایستا صحیح
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  اگر (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      اگر (*arg_reg > 7)
	بازگشت 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  بازگشت 0;
}

ایستا unsigned
calc_n32_struct_flags(صحیح soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  اگر (soft_float)
    بازگشت 0;

  وقتی ((e = arg->elements[index]))
    {
      اگر (e->type == FFI_TYPE_COMPLEX)
	{
	  اگر (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    بشکن;
	  اگر (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    بشکن;
	}
      وگرنه
	اگر (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  بشکن;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  بازگشت flags;
}

ایستا unsigned
calc_n32_return_struct_flags(صحیح soft_float, ffi_type *arg)
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
  
  اگر (arg->size > 16)
    بازگشت 0;

  اگر (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  اگر (e->type == FFI_TYPE_COMPLEX)
    {
      صحیح type = e->elements[0]->type;

      اگر (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	بازگشت small;

      اگر (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  بازگشت small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  وگرنه
    {
      اگر (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	بازگشت small;

      flags = e->type;

      اگر (arg->elements[1])
	{
	  e = arg->elements[1];
	  اگر (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    بازگشت small;

	  اگر (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      بازگشت small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  اگر (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  بازگشت flags;
}

#endif

/* Perform machine dependent cif processing */
ایستا ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  اگر (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      اگر (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  انتخاب ((cif->arg_types)[0]->type)
	    {
	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      بشکن;
	      
	    پیشفرض:
	      بشکن;
	    }

	  اگر (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      اگر (cif->flags)
		{
		  انتخاب ((cif->arg_types)[1]->type)
		    {
		    حالت FFI_TYPE_FLOAT:
		    حالت FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      بشکن;
		      
		    پیشفرض:
		      بشکن;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  اگر (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_VOID:
        حالت FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          بشکن;

        حالت FFI_TYPE_SINT64:
        حالت FFI_TYPE_UINT64:
        حالت FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          بشکن;
      
        حالت FFI_TYPE_FLOAT:
        پیشفرض:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          بشکن;
        }
    }
  وگرنه
    {
      /* FFI_O32 */      
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_VOID:
        حالت FFI_TYPE_STRUCT:
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_DOUBLE:
        حالت FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  اگر (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          بشکن;

        حالت FFI_TYPE_SINT64:
        حالت FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          بشکن;
      
        پیشفرض:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          بشکن;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned شمار = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    صحیح soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    اگر (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	اگر (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    شمار = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	وگرنه
	    cif->rstruct_flag = 0;
      }
    وگرنه
      cif->rstruct_flag = 0;

    وقتی (شمار-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	انتخاب (t->type)
	  {
	  حالت FFI_TYPE_FLOAT:
	  حالت FFI_TYPE_DOUBLE:
	    اگر (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    بشکن;
          حالت FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    اگر (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    وگرنه
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		اگر (arg_reg >= 8)
		  ادامه;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            بشکن;

	  حالت FFI_TYPE_COMPLEX:
	    انتخاب (t->elements[0]->type)
	      {
	      حالت FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		اگر (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		وگرنه
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        ادامه;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        ادامه;
		  }
		/* passthrough */
	      حالت FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      حالت FFI_TYPE_SINT32:
	      حالت FFI_TYPE_UINT32:
	      حالت FFI_TYPE_DOUBLE:
		اگر (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		وگرنه
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        ادامه;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		بشکن;
	      پیشفرض:
		arg_reg += 2;
		بشکن;
	      }
	    بشکن;

	  حالت FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    بشکن;

	  پیشفرض:
	    arg_reg++;
            بشکن;
	  }

	index++;
      }

  /* Set the return type flag */
    انتخاب (cif->rtype->type)
      {
      حالت FFI_TYPE_STRUCT:
	{
	  اگر (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  وگرنه
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  بشکن;
	}
      
      حالت FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	بشکن;

      حالت FFI_TYPE_POINTER:
	اگر (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	وگرنه
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	بشکن;

      حالت FFI_TYPE_FLOAT:
	اگر (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    بشکن;
	  }
	/* else fall through */
      حالت FFI_TYPE_DOUBLE:
	اگر (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	وگرنه
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	بشکن;

      حالت FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	اگر (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	وگرنه
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	بشکن;
      حالت FFI_TYPE_COMPLEX:
	{
	  صحیح type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  اگر (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      انتخاب (type)
		{
		حالت FFI_TYPE_DOUBLE:
		حالت FFI_TYPE_SINT64:
		حالت FFI_TYPE_UINT64:
		حالت FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  بشکن;
		حالت FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  بشکن;
		حالت FFI_TYPE_FLOAT:
		پیشفرض:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  وگرنه
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  بشکن;
	}
      حالت FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	بشکن;
      حالت FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	بشکن;
      پیشفرض:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	بشکن;
      }
  }
#endif
  بازگشت FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    بازگشت ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    بازگشت ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
بیرونی صحیح ffi_call_O32(تهی (*)(نویسه *, extended_cif *, صحیح, صحیح), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, تهی (*)(تهی), تهی *closure);

/* Low level routine for calling N32 functions */
بیرونی صحیح ffi_call_N32(تهی (*)(نویسه *, extended_cif *, صحیح, صحیح), 
			extended_cif *, unsigned, 
			unsigned, تهی *, تهی (*)(تهی), تهی *closure);

تهی ffi_call_int(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, 
	      تهی **avalue, تهی *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  اگر ((rvalue == پوچ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  وگرنه
    ecif.rvalue = rvalue;
    
  انتخاب (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    حالت FFI_O32:
    حالت FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      بشکن;
#endif

#ifdef FFI_MIPS_N32
    حالت FFI_N32:
    حالت FFI_N32_SOFT_FLOAT:
    حالت FFI_N64:
    حالت FFI_N64_SOFT_FLOAT:
      {
        صحیح copy_rvalue = 0;
	صحیح copy_offset = 0;
        نویسه *rvalue_copy = ecif.rvalue;
        اگر (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	وگرنه اگر (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        اگر (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      بشکن;
#endif

    پیشفرض:
      FFI_ASSERT(0);
      بشکن;
    }
}

تهی
ffi_call(ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#اگر FFI_CLOSURES
#اگر defined(FFI_MIPS_O32)
بیرونی تهی ffi_closure_O32(تهی);
بیرونی تهی ffi_go_closure_O32(تهی);
#وگرنه
بیرونی تهی ffi_closure_N32(تهی);
بیرونی تهی ffi_go_closure_N32(تهی);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
		      تهی *user_data,
		      تهی *codeloc)
{
  unsigned صحیح *tramp = (unsigned صحیح *) &closure->tramp[0];
  تهی * fn;
  نویسه *clear_location = (نویسه *) codeloc;

#اگر defined(FFI_MIPS_O32)
  اگر (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
  fn = ffi_closure_O32;
#وگرنه
#اگر _MIPS_SIM ==_ABIN32
  اگر (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
#وگرنه
  اگر (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#اگر defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#اگر !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#وگرنه
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#وگرنه
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
#اگر !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#وگرنه
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#اگر !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#وگرنه
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  بازگشت FFI_OK;
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
صحیح
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
			    تهی *user_data,
			    تهی *rvalue, ffi_arg *ar,
			    دوگانه *fpr)
{
  تهی **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحیح i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * اندازه (ffi_arg));
  avaluep = alloca (cif->nargs * اندازه (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  اگر ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (تهی *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  اگر ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  وقتی (i < avn)
    {
      اگر (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      اگر (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
	  اگر (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((نویسه *) &fpr[i]) + اندازه (اعشاری);
	  وگرنه
#endif
	    avaluep[i] = (نویسه *) &fpr[i];
	}
      وگرنه
	{
	  انتخاب (arg_types[i]->type)
	    {
	      حالت FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		بشکن;

	      حالت FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		بشکن;
		  
	      حالت FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		بشکن;
		  
	      حالت FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		بشکن;

	      پیشفرض:
		avaluep[i] = (نویسه *) &ar[argn];
		بشکن;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  اگر (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_FLOAT:
          بازگشت FFI_TYPE_INT;
        حالت FFI_TYPE_DOUBLE:
          بازگشت FFI_TYPE_UINT64;
        پیشفرض:
          بازگشت cif->rtype->type;
        }
    }
  وگرنه
    {
      اگر (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ فرار ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      بازگشت cif->rtype->type;
    }
}

#اگر defined(FFI_MIPS_N32)

ایستا تهی
copy_struct_N32(نویسه *target, unsigned offset, ffi_abi abi, ffi_type *type,
                صحیح argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, صحیح soft_float)
{
  ffi_type **elt_typep = type->elements;
  وقتی(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      نویسه *tp;
      نویسه *argp;
      نویسه *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / اندازه(ffi_arg);
      arg_offset = arg_offset % اندازه(ffi_arg);

      argp = (نویسه *)(ar + argn);
      fpp = (نویسه *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      اگر (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(دوگانه *)tp = *(دوگانه *)fpp;
      وگرنه
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / اندازه(ffi_arg);
      arg_offset = arg_offset % اندازه(ffi_arg);
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
صحیح
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    تهی (*fun)(ffi_cif*, تهی*, تهی**, تهی*),
                            تهی *user_data,
			    تهی *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  تهی **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحیح i, avn, argn;
  صحیح soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * اندازه (ffi_arg));
  avaluep = alloca (cif->nargs * اندازه (ffi_arg));

  argn = 0;

  اگر (cif->rstruct_flag)
    {
#اگر _MIPS_SIM==_ABIN32
      rvalue = (تهی *)(UINT32)ar[0];
#وگرنه /* N64 */
      rvalue = (تهی *)ar[0];
#endif
      argn = 1;
    }
  اگر (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  وقتی (i < avn)
    {
      اگر (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          اگر ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
          اگر (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((نویسه *) argp) + اندازه (اعشاری);
          وگرنه
#endif
            avaluep[i] = (نویسه *) argp;
        }
      وگرنه اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (نویسه *) argp;
        }
      وگرنه اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (نویسه *) argp;
        }
      وگرنه اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          اگر (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  وگرنه
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (نویسه *) argp;
        }
      وگرنه
        {
          unsigned type = arg_types[i]->type;

          اگر (arg_types[i]->alignment > اندازه(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / اندازه(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          اگر (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  اگر (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          انتخاب (type)
            {
            حالت FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              بشکن;

            حالت FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              بشکن;

            حالت FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              بشکن;

            حالت FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              بشکن;

            حالت FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              بشکن;

            حالت FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              بشکن;

            حالت FFI_TYPE_STRUCT:
              اگر (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  بشکن;
                }
              /* Else fall through.  */
            پیشفرض:
              avaluep[i] = (نویسه *) argp;
              بشکن;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, اندازه(ffi_arg)) / اندازه(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  بازگشت cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#اگر defined(FFI_MIPS_O32)
بیرونی تهی ffi_closure_O32(تهی);
بیرونی تهی ffi_go_closure_O32(تهی);
#وگرنه
بیرونی تهی ffi_closure_N32(تهی);
بیرونی تهی ffi_go_closure_N32(تهی);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*))
{
  تهی * fn;

#اگر defined(FFI_MIPS_O32)
  اگر (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#وگرنه
#اگر _MIPS_SIM ==_ABIN32
  اگر (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
#وگرنه
  اگر (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    بازگشت FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (تهی *)fn;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

#endif /* FFI_CLOSURES */

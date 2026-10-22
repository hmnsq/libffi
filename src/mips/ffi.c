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

#অন্তর্ভাৱ <ffi.h>
#অন্তর্ভাৱ <ffi_common.h>

#অন্তর্ভাৱ <stdint.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdio.h>

#ifdef __GNUC__
#  যদি (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  যদি defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  অন্যথা
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#অন্যথা
# define FFI_MIPS_STOP_HERE() কৰা {} যেতিয়ালৈ(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &স্তূপ[bytes]); \
যদি (argp == &স্তূপ[bytes]) \
{ \
  argp = স্তূপ; \
  FFI_MIPS_STOP_HERE(); \
}
#অন্যথা
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

স্থির শূন্য ffi_prep_args(ৱর্ণ *স্তূপ, 
			  extended_cif *ecif,
			  পূর্ণাঙ্ক bytes,
			  পূর্ণাঙ্ক flags)
{
  পূর্ণাঙ্ক i;
  শূন্য **p_argv;
  ৱর্ণ *argp, *argp_f;
  ffi_type **p_arg;

  memset(স্তূপ, 0, bytes);

#ifdef FFI_MIPS_N32
  পূর্ণাঙ্ক soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  যদি (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      যদি (bytes + 16 > 8 * পরিমাণ(ffi_arg))
        argp = &স্তূপ[bytes - (8 * পরিমাণ(ffi_arg))];
      অন্যথা
        argp = স্তূপ;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  অন্যথা
    {
      যদি (bytes > 8 * পরিমাণ(ffi_arg))
        argp = &স্তূপ[bytes - (8 * পরিমাণ(ffi_arg))];
      অন্যথা
        argp = স্তূপ;
    }
#অন্যথা
  argp = স্তূপ;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  যদি ( ecif->cif->rstruct_flag != 0 )
#অন্যথা
  যদি ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += পরিমাণ(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  বাবে (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned পূর্ণাঙ্ক a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      যদি (a < পরিমাণ(ffi_arg))
        a = পরিমাণ(ffi_arg);
      
      যদি ((a - 1) & (unsigned long) argp)
	{
	  argp = (ৱর্ণ *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      যদি (z <= পরিমাণ(ffi_arg))
	{
          পূর্ণাঙ্ক type = (*p_arg)->type;
	  z = পরিমাণ(ffi_arg);

          /* The size of a pointer depends on the ABI */
          যদি (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	যদি (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ৱিকল্প (type)
	      {
	      পক্ষ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ভঙ্গ;
	      পক্ষ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ভঙ্গ;
	      স্ৱভাৱ:
		ভঙ্গ;
	      }
	  }
	  ৱিকল্প (type)
	    {
	      পক্ষ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ভঙ্গ;

	      পক্ষ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#অন্যথা
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		ভঙ্গ;

#ifdef FFI_MIPS_N32
	      পক্ষ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        যদি(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ভিন্ন *) argp = *(ভিন্ন *)(* p_argv);
		    argp += z;
		    ৱর্ণ *tmp = (শূন্য *) (*p_argv);
		    *(ভিন্ন *) argp = *(ভিন্ন *)(tmp+4);
		  }
		অন্যথা
		  memcpy(argp, *p_argv, (*p_arg)->size);
		ভঙ্গ;
#endif
	      /* This can only happen with 64bit slots.  */
	      পক্ষ FFI_TYPE_FLOAT:
		*(ভিন্ন *) argp = *(ভিন্ন *)(* p_argv);
		ভঙ্গ;

	      /* Handle structures.  */
	      স্ৱভাৱ:
		memcpy(argp, *p_argv, (*p_arg)->size);
		ভঙ্গ;
	    }
	}
      অন্যথা
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#অন্যথা
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) স্তূপ + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    যদি (end <= cap)
	      memcpy(argp, *p_argv, z);
	    অন্যথা
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = স্তূপ;
                z -= portion;
		memcpy(argp, (শূন্য*)((unsigned long)(*p_argv) + portion),
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

স্থির পূর্ণাঙ্ক
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  যদি (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      যদি (*arg_reg > 7)
	ঘূৰাই 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  ঘূৰাই 0;
}

স্থির unsigned
calc_n32_struct_flags(পূর্ণাঙ্ক soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  যদি (soft_float)
    ঘূৰাই 0;

  যেতিয়ালৈ ((e = arg->elements[index]))
    {
      যদি (e->type == FFI_TYPE_COMPLEX)
	{
	  যদি (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ভঙ্গ;
	  যদি (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ভঙ্গ;
	}
      অন্যথা
	যদি (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ভঙ্গ;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ঘূৰাই flags;
}

স্থির unsigned
calc_n32_return_struct_flags(পূর্ণাঙ্ক soft_float, ffi_type *arg)
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
  
  যদি (arg->size > 16)
    ঘূৰাই 0;

  যদি (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  যদি (e->type == FFI_TYPE_COMPLEX)
    {
      পূর্ণাঙ্ক type = e->elements[0]->type;

      যদি (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ঘূৰাই small;

      যদি (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ঘূৰাই small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  অন্যথা
    {
      যদি (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ঘূৰাই small;

      flags = e->type;

      যদি (arg->elements[1])
	{
	  e = arg->elements[1];
	  যদি (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ঘূৰাই small;

	  যদি (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ঘূৰাই small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  যদি (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ঘূৰাই flags;
}

#endif

/* Perform machine dependent cif processing */
স্থির ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  যদি (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      যদি (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ৱিকল্প ((cif->arg_types)[0]->type)
	    {
	    পক্ষ FFI_TYPE_FLOAT:
	    পক্ষ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ভঙ্গ;
	      
	    স্ৱভাৱ:
	      ভঙ্গ;
	    }

	  যদি (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      যদি (cif->flags)
		{
		  ৱিকল্প ((cif->arg_types)[1]->type)
		    {
		    পক্ষ FFI_TYPE_FLOAT:
		    পক্ষ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      ভঙ্গ;
		      
		    স্ৱভাৱ:
		      ভঙ্গ;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  যদি (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ৱিকল্প (cif->rtype->type)
        {
        পক্ষ FFI_TYPE_VOID:
        পক্ষ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          ভঙ্গ;

        পক্ষ FFI_TYPE_SINT64:
        পক্ষ FFI_TYPE_UINT64:
        পক্ষ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ভঙ্গ;
      
        পক্ষ FFI_TYPE_FLOAT:
        স্ৱভাৱ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ভঙ্গ;
        }
    }
  অন্যথা
    {
      /* FFI_O32 */      
      ৱিকল্প (cif->rtype->type)
        {
        পক্ষ FFI_TYPE_VOID:
        পক্ষ FFI_TYPE_STRUCT:
        পক্ষ FFI_TYPE_FLOAT:
        পক্ষ FFI_TYPE_DOUBLE:
        পক্ষ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  যদি (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          ভঙ্গ;

        পক্ষ FFI_TYPE_SINT64:
        পক্ষ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ভঙ্গ;
      
        স্ৱভাৱ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ভঙ্গ;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned গুনতি = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    পূর্ণাঙ্ক soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    যদি (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	যদি (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    গুনতি = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	অন্যথা
	    cif->rstruct_flag = 0;
      }
    অন্যথা
      cif->rstruct_flag = 0;

    যেতিয়ালৈ (গুনতি-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	ৱিকল্প (t->type)
	  {
	  পক্ষ FFI_TYPE_FLOAT:
	  পক্ষ FFI_TYPE_DOUBLE:
	    যদি (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    ভঙ্গ;
          পক্ষ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    যদি (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    অন্যথা
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		যদি (arg_reg >= 8)
		  অনুৱর্ত;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            ভঙ্গ;

	  পক্ষ FFI_TYPE_COMPLEX:
	    ৱিকল্প (t->elements[0]->type)
	      {
	      পক্ষ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		যদি (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		অন্যথা
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    যদি (arg_reg >= 8)
		        অনুৱর্ত;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    যদি (arg_reg >= 8)
		        অনুৱর্ত;
		  }
		/* passthrough */
	      পক্ষ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      পক্ষ FFI_TYPE_SINT32:
	      পক্ষ FFI_TYPE_UINT32:
	      পক্ষ FFI_TYPE_DOUBLE:
		যদি (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		অন্যথা
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    যদি (arg_reg >= 8)
		        অনুৱর্ত;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		ভঙ্গ;
	      স্ৱভাৱ:
		arg_reg += 2;
		ভঙ্গ;
	      }
	    ভঙ্গ;

	  পক্ষ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    ভঙ্গ;

	  স্ৱভাৱ:
	    arg_reg++;
            ভঙ্গ;
	  }

	index++;
      }

  /* Set the return type flag */
    ৱিকল্প (cif->rtype->type)
      {
      পক্ষ FFI_TYPE_STRUCT:
	{
	  যদি (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  অন্যথা
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ভঙ্গ;
	}
      
      পক্ষ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ভঙ্গ;

      পক্ষ FFI_TYPE_POINTER:
	যদি (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	অন্যথা
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ভঙ্গ;

      পক্ষ FFI_TYPE_FLOAT:
	যদি (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    ভঙ্গ;
	  }
	/* else fall through */
      পক্ষ FFI_TYPE_DOUBLE:
	যদি (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	অন্যথা
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ভঙ্গ;

      পক্ষ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	যদি (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	অন্যথা
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	ভঙ্গ;
      পক্ষ FFI_TYPE_COMPLEX:
	{
	  পূর্ণাঙ্ক type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  যদি (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ৱিকল্প (type)
		{
		পক্ষ FFI_TYPE_DOUBLE:
		পক্ষ FFI_TYPE_SINT64:
		পক্ষ FFI_TYPE_UINT64:
		পক্ষ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ভঙ্গ;
		পক্ষ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ভঙ্গ;
		পক্ষ FFI_TYPE_FLOAT:
		স্ৱভাৱ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  অন্যথা
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ভঙ্গ;
	}
      পক্ষ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ভঙ্গ;
      পক্ষ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ভঙ্গ;
      স্ৱভাৱ:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ভঙ্গ;
      }
  }
#endif
  ঘূৰাই FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    ঘূৰাই ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    ঘূৰাই ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
বাহ্য পূর্ণাঙ্ক ffi_call_O32(শূন্য (*)(ৱর্ণ *, extended_cif *, পূর্ণাঙ্ক, পূর্ণাঙ্ক), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, শূন্য (*)(শূন্য), শূন্য *closure);

/* Low level routine for calling N32 functions */
বাহ্য পূর্ণাঙ্ক ffi_call_N32(শূন্য (*)(ৱর্ণ *, extended_cif *, পূর্ণাঙ্ক, পূর্ণাঙ্ক), 
			extended_cif *, unsigned, 
			unsigned, শূন্য *, শূন্য (*)(শূন্য), শূন্য *closure);

শূন্য ffi_call_int(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, 
	      শূন্য **avalue, শূন্য *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  যদি ((rvalue == রিক্ত) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  অন্যথা
    ecif.rvalue = rvalue;
    
  ৱিকল্প (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    পক্ষ FFI_O32:
    পক্ষ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      ভঙ্গ;
#endif

#ifdef FFI_MIPS_N32
    পক্ষ FFI_N32:
    পক্ষ FFI_N32_SOFT_FLOAT:
    পক্ষ FFI_N64:
    পক্ষ FFI_N64_SOFT_FLOAT:
      {
        পূর্ণাঙ্ক copy_rvalue = 0;
	পূর্ণাঙ্ক copy_offset = 0;
        ৱর্ণ *rvalue_copy = ecif.rvalue;
        যদি (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	অন্যথা যদি (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#যদি defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        যদি (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      ভঙ্গ;
#endif

    স্ৱভাৱ:
      FFI_ASSERT(0);
      ভঙ্গ;
    }
}

শূন্য
ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, রিক্ত);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#যদি FFI_CLOSURES
#যদি defined(FFI_MIPS_O32)
বাহ্য শূন্য ffi_closure_O32(শূন্য);
বাহ্য শূন্য ffi_go_closure_O32(শূন্য);
#অন্যথা
বাহ্য শূন্য ffi_closure_N32(শূন্য);
বাহ্য শূন্য ffi_go_closure_N32(শূন্য);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_data,
		      শূন্য *codeloc)
{
  unsigned পূর্ণাঙ্ক *tramp = (unsigned পূর্ণাঙ্ক *) &closure->tramp[0];
  শূন্য * fn;
  ৱর্ণ *clear_location = (ৱর্ণ *) codeloc;

#যদি defined(FFI_MIPS_O32)
  যদি (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
  fn = ffi_closure_O32;
#অন্যথা
#যদি _MIPS_SIM ==_ABIN32
  যদি (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
#অন্যথা
  যদি (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#যদি defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#যদি !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#অন্যথা
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#অন্যথা
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
#যদি !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#অন্যথা
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#যদি !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#অন্যথা
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  ঘূৰাই FFI_OK;
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
পূর্ণাঙ্ক
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			    শূন্য *user_data,
			    শূন্য *rvalue, ffi_arg *ar,
			    দ্ৱিগুণ *fpr)
{
  শূন্য **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  পূর্ণাঙ্ক i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * পরিমাণ (ffi_arg));
  avaluep = alloca (cif->nargs * পরিমাণ (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  যদি ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (শূন্য *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  যদি ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  যেতিয়ালৈ (i < avn)
    {
      যদি (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      যদি (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#যদি defined(__MIPSEB__) || defined(_MIPSEB)
	  যদি (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((ৱর্ণ *) &fpr[i]) + পরিমাণ (ভিন্ন);
	  অন্যথা
#endif
	    avaluep[i] = (ৱর্ণ *) &fpr[i];
	}
      অন্যথা
	{
	  ৱিকল্প (arg_types[i]->type)
	    {
	      পক্ষ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ভঙ্গ;

	      পক্ষ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ভঙ্গ;
		  
	      পক্ষ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ভঙ্গ;

	      স্ৱভাৱ:
		avaluep[i] = (ৱর্ণ *) &ar[argn];
		ভঙ্গ;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  যদি (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ৱিকল্প (cif->rtype->type)
        {
        পক্ষ FFI_TYPE_FLOAT:
          ঘূৰাই FFI_TYPE_INT;
        পক্ষ FFI_TYPE_DOUBLE:
          ঘূৰাই FFI_TYPE_UINT64;
        স্ৱভাৱ:
          ঘূৰাই cif->rtype->type;
        }
    }
  অন্যথা
    {
      যদি (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ চঞ্চল ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      ঘূৰাই cif->rtype->type;
    }
}

#যদি defined(FFI_MIPS_N32)

স্থির শূন্য
copy_struct_N32(ৱর্ণ *target, unsigned offset, ffi_abi abi, ffi_type *type,
                পূর্ণাঙ্ক argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, পূর্ণাঙ্ক soft_float)
{
  ffi_type **elt_typep = type->elements;
  যেতিয়ালৈ(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      ৱর্ণ *tp;
      ৱর্ণ *argp;
      ৱর্ণ *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / পরিমাণ(ffi_arg);
      arg_offset = arg_offset % পরিমাণ(ffi_arg);

      argp = (ৱর্ণ *)(ar + argn);
      fpp = (ৱর্ণ *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      যদি (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(দ্ৱিগুণ *)tp = *(দ্ৱিগুণ *)fpp;
      অন্যথা
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / পরিমাণ(ffi_arg);
      arg_offset = arg_offset % পরিমাণ(ffi_arg);
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
পূর্ণাঙ্ক
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
                            শূন্য *user_data,
			    শূন্য *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  শূন্য **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  পূর্ণাঙ্ক i, avn, argn;
  পূর্ণাঙ্ক soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * পরিমাণ (ffi_arg));
  avaluep = alloca (cif->nargs * পরিমাণ (ffi_arg));

  argn = 0;

  যদি (cif->rstruct_flag)
    {
#যদি _MIPS_SIM==_ABIN32
      rvalue = (শূন্য *)(UINT32)ar[0];
#অন্যথা /* N64 */
      rvalue = (শূন্য *)ar[0];
#endif
      argn = 1;
    }
  যদি (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  যেতিয়ালৈ (i < avn)
    {
      যদি (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          যদি ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#যদি defined(__MIPSEB__) || defined(_MIPSEB)
          যদি (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((ৱর্ণ *) argp) + পরিমাণ (ভিন্ন);
          অন্যথা
#endif
            avaluep[i] = (ৱর্ণ *) argp;
        }
      অন্যথা যদি (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ৱর্ণ *) argp;
        }
      অন্যথা যদি (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ৱর্ণ *) argp;
        }
      অন্যথা যদি (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          যদি (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  অন্যথা
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (ৱর্ণ *) argp;
        }
      অন্যথা
        {
          unsigned type = arg_types[i]->type;

          যদি (arg_types[i]->alignment > পরিমাণ(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / পরিমাণ(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          যদি (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  যদি (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ৱিকল্প (type)
            {
            পক্ষ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ভঙ্গ;

            পক্ষ FFI_TYPE_STRUCT:
              যদি (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ভঙ্গ;
                }
              /* Else fall through.  */
            স্ৱভাৱ:
              avaluep[i] = (ৱর্ণ *) argp;
              ভঙ্গ;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, পরিমাণ(ffi_arg)) / পরিমাণ(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  ঘূৰাই cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#যদি defined(FFI_MIPS_O32)
বাহ্য শূন্য ffi_closure_O32(শূন্য);
বাহ্য শূন্য ffi_go_closure_O32(শূন্য);
#অন্যথা
বাহ্য শূন্য ffi_closure_N32(শূন্য);
বাহ্য শূন্য ffi_go_closure_N32(শূন্য);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*))
{
  শূন্য * fn;

#যদি defined(FFI_MIPS_O32)
  যদি (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#অন্যথা
#যদি _MIPS_SIM ==_ABIN32
  যদি (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
#অন্যথা
  যদি (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ঘূৰাই FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (শূন্য *)fn;
  closure->cif = cif;
  closure->fun = fun;

  ঘূৰাই FFI_OK;
}

#endif /* FFI_CLOSURES */

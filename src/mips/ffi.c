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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>

#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdio.h>

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
# define FFI_MIPS_STOP_HERE() করো {} যখন(0)
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

স্থির শূন্য ffi_prep_args(অক্ষর *স্তূপ, 
			  extended_cif *ecif,
			  পূর্ণসংখ্যা bytes,
			  পূর্ণসংখ্যা flags)
{
  পূর্ণসংখ্যা i;
  শূন্য **p_argv;
  অক্ষর *argp, *argp_f;
  ffi_type **p_arg;

  memset(স্তূপ, 0, bytes);

#ifdef FFI_MIPS_N32
  পূর্ণসংখ্যা soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  যদি (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      যদি (bytes + 16 > 8 * আকার(ffi_arg))
        argp = &স্তূপ[bytes - (8 * আকার(ffi_arg))];
      অন্যথা
        argp = স্তূপ;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  অন্যথা
    {
      যদি (bytes > 8 * আকার(ffi_arg))
        argp = &স্তূপ[bytes - (8 * আকার(ffi_arg))];
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
      argp += আকার(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  জন্য (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned পূর্ণসংখ্যা a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      যদি (a < আকার(ffi_arg))
        a = আকার(ffi_arg);
      
      যদি ((a - 1) & (unsigned long) argp)
	{
	  argp = (অক্ষর *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->আকার_2;
      যদি (z <= আকার(ffi_arg))
	{
          পূর্ণসংখ্যা type = (*p_arg)->type;
	  z = আকার(ffi_arg);

          /* The size of a pointer depends on the ABI */
          যদি (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	যদি (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    নির্বাচন (type)
	      {
	      ক্ষেত্র FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		বিরতি;
	      ক্ষেত্র FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		বিরতি;
	      পূর্বনির্ধারিত:
		বিরতি;
	      }
	  }
	  নির্বাচন (type)
	    {
	      ক্ষেত্র FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		বিরতি;

	      ক্ষেত্র FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#অন্যথা
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		বিরতি;

#ifdef FFI_MIPS_N32
	      ক্ষেত্র FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        যদি(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(দশমিক *) argp = *(দশমিক *)(* p_argv);
		    argp += z;
		    অক্ষর *tmp = (শূন্য *) (*p_argv);
		    *(দশমিক *) argp = *(দশমিক *)(tmp+4);
		  }
		অন্যথা
		  memcpy(argp, *p_argv, (*p_arg)->আকার_2);
		বিরতি;
#endif
	      /* This can only happen with 64bit slots.  */
	      ক্ষেত্র FFI_TYPE_FLOAT:
		*(দশমিক *) argp = *(দশমিক *)(* p_argv);
		বিরতি;

	      /* Handle structures.  */
	      পূর্বনির্ধারিত:
		memcpy(argp, *p_argv, (*p_arg)->আকার_2);
		বিরতি;
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

স্থির পূর্ণসংখ্যা
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
	ফেরত 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->আকার_2;
  ফেরত 0;
}

স্থির unsigned
calc_n32_struct_flags(পূর্ণসংখ্যা soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned সূচক = 0;

  ffi_type *e;

  যদি (soft_float)
    ফেরত 0;

  যখন ((e = arg->elements[সূচক]))
    {
      যদি (e->type == FFI_TYPE_COMPLEX)
	{
	  যদি (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    বিরতি;
	  যদি (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    বিরতি;
	}
      অন্যথা
	যদি (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  বিরতি;
      সূচক++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ফেরত flags;
}

স্থির unsigned
calc_n32_return_struct_flags(পূর্ণসংখ্যা soft_float, ffi_type *arg)
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
  
  যদি (arg->আকার_2 > 16)
    ফেরত 0;

  যদি (arg->আকার_2 > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  যদি (e->type == FFI_TYPE_COMPLEX)
    {
      পূর্ণসংখ্যা type = e->elements[0]->type;

      যদি (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ফেরত small;

      যদি (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ফেরত small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  অন্যথা
    {
      যদি (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ফেরত small;

      flags = e->type;

      যদি (arg->elements[1])
	{
	  e = arg->elements[1];
	  যদি (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ফেরত small;

	  যদি (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ফেরত small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  যদি (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ফেরত flags;
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
	  নির্বাচন ((cif->arg_types)[0]->type)
	    {
	    ক্ষেত্র FFI_TYPE_FLOAT:
	    ক্ষেত্র FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      বিরতি;
	      
	    পূর্বনির্ধারিত:
	      বিরতি;
	    }

	  যদি (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      যদি (cif->flags)
		{
		  নির্বাচন ((cif->arg_types)[1]->type)
		    {
		    ক্ষেত্র FFI_TYPE_FLOAT:
		    ক্ষেত্র FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      বিরতি;
		      
		    পূর্বনির্ধারিত:
		      বিরতি;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  যদি (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      নির্বাচন (cif->rtype->type)
        {
        ক্ষেত্র FFI_TYPE_VOID:
        ক্ষেত্র FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          বিরতি;

        ক্ষেত্র FFI_TYPE_SINT64:
        ক্ষেত্র FFI_TYPE_UINT64:
        ক্ষেত্র FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          বিরতি;
      
        ক্ষেত্র FFI_TYPE_FLOAT:
        পূর্বনির্ধারিত:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          বিরতি;
        }
    }
  অন্যথা
    {
      /* FFI_O32 */      
      নির্বাচন (cif->rtype->type)
        {
        ক্ষেত্র FFI_TYPE_VOID:
        ক্ষেত্র FFI_TYPE_STRUCT:
        ক্ষেত্র FFI_TYPE_FLOAT:
        ক্ষেত্র FFI_TYPE_DOUBLE:
        ক্ষেত্র FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  যদি (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          বিরতি;

        ক্ষেত্র FFI_TYPE_SINT64:
        ক্ষেত্র FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          বিরতি;
      
        পূর্বনির্ধারিত:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          বিরতি;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned গুনতি = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned সূচক = 0;

    unsigned struct_flags = 0;
    পূর্ণসংখ্যা soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
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

    যখন (গুনতি-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[সূচক];

	নির্বাচন (t->type)
	  {
	  ক্ষেত্র FFI_TYPE_FLOAT:
	  ক্ষেত্র FFI_TYPE_DOUBLE:
	    যদি (!soft_float && সূচক < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    বিরতি;
          ক্ষেত্র FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    যদি (soft_float || সূচক >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    অন্যথা
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		যদি (arg_reg >= 8)
		  চালু;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            বিরতি;

	  ক্ষেত্র FFI_TYPE_COMPLEX:
	    নির্বাচন (t->elements[0]->type)
	      {
	      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		যদি (soft_float || সূচক >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		অন্যথা
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    যদি (arg_reg >= 8)
		        চালু;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    যদি (arg_reg >= 8)
		        চালু;
		  }
		/* passthrough */
	      ক্ষেত্র FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      ক্ষেত্র FFI_TYPE_SINT32:
	      ক্ষেত্র FFI_TYPE_UINT32:
	      ক্ষেত্র FFI_TYPE_DOUBLE:
		যদি (soft_float || সূচক >= nfixedargs)
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
		        চালু;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		বিরতি;
	      পূর্বনির্ধারিত:
		arg_reg += 2;
		বিরতি;
	      }
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || সূচক >= nfixedargs,
						t, &loc, &arg_reg);
	    বিরতি;

	  পূর্বনির্ধারিত:
	    arg_reg++;
            বিরতি;
	  }

	সূচক++;
      }

  /* Set the return type flag */
    নির্বাচন (cif->rtype->type)
      {
      ক্ষেত্র FFI_TYPE_STRUCT:
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
	  বিরতি;
	}
      
      ক্ষেত্র FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	বিরতি;

      ক্ষেত্র FFI_TYPE_POINTER:
	যদি (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	অন্যথা
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	বিরতি;

      ক্ষেত্র FFI_TYPE_FLOAT:
	যদি (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    বিরতি;
	  }
	/* else fall through */
      ক্ষেত্র FFI_TYPE_DOUBLE:
	যদি (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	অন্যথা
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	বিরতি;

      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
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
	বিরতি;
      ক্ষেত্র FFI_TYPE_COMPLEX:
	{
	  পূর্ণসংখ্যা type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  যদি (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      নির্বাচন (type)
		{
		ক্ষেত্র FFI_TYPE_DOUBLE:
		ক্ষেত্র FFI_TYPE_SINT64:
		ক্ষেত্র FFI_TYPE_UINT64:
		ক্ষেত্র FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  বিরতি;
		ক্ষেত্র FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  বিরতি;
		ক্ষেত্র FFI_TYPE_FLOAT:
		পূর্বনির্ধারিত:
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
	  বিরতি;
	}
      ক্ষেত্র FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	বিরতি;
      ক্ষেত্র FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	বিরতি;
      পূর্বনির্ধারিত:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	বিরতি;
      }
  }
#endif
  ফেরত FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    ফেরত ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    ফেরত ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
বাহ্যিক পূর্ণসংখ্যা ffi_call_O32(শূন্য (*)(অক্ষর *, extended_cif *, পূর্ণসংখ্যা, পূর্ণসংখ্যা), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, শূন্য (*)(শূন্য), শূন্য *closure);

/* Low level routine for calling N32 functions */
বাহ্যিক পূর্ণসংখ্যা ffi_call_N32(শূন্য (*)(অক্ষর *, extended_cif *, পূর্ণসংখ্যা, পূর্ণসংখ্যা), 
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
  
  যদি ((rvalue == ফাঁকা) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->আকার_2);
  অন্যথা
    ecif.rvalue = rvalue;
    
  নির্বাচন (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    ক্ষেত্র FFI_O32:
    ক্ষেত্র FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      বিরতি;
#endif

#ifdef FFI_MIPS_N32
    ক্ষেত্র FFI_N32:
    ক্ষেত্র FFI_N32_SOFT_FLOAT:
    ক্ষেত্র FFI_N64:
    ক্ষেত্র FFI_N64_SOFT_FLOAT:
      {
        পূর্ণসংখ্যা copy_rvalue = 0;
	পূর্ণসংখ্যা copy_offset = 0;
        অক্ষর *rvalue_copy = ecif.rvalue;
        যদি (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->আকার_2 < 16)
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
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->আকার_2);
      }
      বিরতি;
#endif

    পূর্বনির্ধারিত:
      FFI_ASSERT(0);
      বিরতি;
    }
}

শূন্য
ffi_call(ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#যদি FFI_CLOSURES
#যদি defined(FFI_MIPS_O32)
বাহ্যিক শূন্য ffi_closure_O32(শূন্য);
বাহ্যিক শূন্য ffi_go_closure_O32(শূন্য);
#অন্যথা
বাহ্যিক শূন্য ffi_closure_N32(শূন্য);
বাহ্যিক শূন্য ffi_go_closure_N32(শূন্য);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  unsigned পূর্ণসংখ্যা *tramp = (unsigned পূর্ণসংখ্যা *) &closure->tramp[0];
  শূন্য * fn;
  অক্ষর *clear_location = (অক্ষর *) codeloc;

#যদি defined(FFI_MIPS_O32)
  যদি (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
  fn = ffi_closure_O32;
#অন্যথা
#যদি _MIPS_SIM ==_ABIN32
  যদি (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
#অন্যথা
  যদি (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
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
  closure->user_তথ্য = user_তথ্য;

#যদি !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_আকার);
#অন্যথা
  cacheflush (clear_location, FFI_TRAMPOLINE_আকার, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  ফেরত FFI_OK;
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
পূর্ণসংখ্যা
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
			    শূন্য *user_তথ্য,
			    শূন্য *rvalue, ffi_arg *ar,
			    দ্বিগুণ *fpr)
{
  শূন্য **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  পূর্ণসংখ্যা i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * আকার (ffi_arg));
  avaluep = alloca (cif->nargs * আকার (ffi_arg));

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

  যখন (i < avn)
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
	    avaluep[i] = ((অক্ষর *) &fpr[i]) + আকার (দশমিক);
	  অন্যথা
#endif
	    avaluep[i] = (অক্ষর *) &fpr[i];
	}
      অন্যথা
	{
	  নির্বাচন (arg_types[i]->type)
	    {
	      ক্ষেত্র FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		বিরতি;

	      ক্ষেত্র FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		বিরতি;
		  
	      ক্ষেত্র FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		বিরতি;

	      পূর্বনির্ধারিত:
		avaluep[i] = (অক্ষর *) &ar[argn];
		বিরতি;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->আকার_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_তথ্য);

  যদি (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      নির্বাচন (cif->rtype->type)
        {
        ক্ষেত্র FFI_TYPE_FLOAT:
          ফেরত FFI_TYPE_INT;
        ক্ষেত্র FFI_TYPE_DOUBLE:
          ফেরত FFI_TYPE_UINT64;
        পূর্বনির্ধারিত:
          ফেরত cif->rtype->type;
        }
    }
  অন্যথা
    {
      যদি (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ অস্থির ("move $v1, %0" : : "r"(cif->rtype->আকার_2));
      }
      ফেরত cif->rtype->type;
    }
}

#যদি defined(FFI_MIPS_N32)

স্থির শূন্য
copy_struct_N32(অক্ষর *target, unsigned offset, ffi_abi abi, ffi_type *type,
                পূর্ণসংখ্যা argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, পূর্ণসংখ্যা soft_float)
{
  ffi_type **elt_typep = type->elements;
  যখন(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      অক্ষর *tp;
      অক্ষর *argp;
      অক্ষর *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / আকার(ffi_arg);
      arg_offset = arg_offset % আকার(ffi_arg);

      argp = (অক্ষর *)(ar + argn);
      fpp = (অক্ষর *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      যদি (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(দ্বিগুণ *)tp = *(দ্বিগুণ *)fpp;
      অন্যথা
        memcpy(tp, argp + arg_offset, elt_type->আকার_2);

      offset += elt_type->আকার_2;
      arg_offset += elt_type->আকার_2;
      elt_typep++;
      argn += arg_offset / আকার(ffi_arg);
      arg_offset = arg_offset % আকার(ffi_arg);
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
পূর্ণসংখ্যা
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    শূন্য (*fun)(ffi_cif*, শূন্য*, শূন্য**, শূন্য*),
                            শূন্য *user_তথ্য,
			    শূন্য *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  শূন্য **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  পূর্ণসংখ্যা i, avn, argn;
  পূর্ণসংখ্যা soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * আকার (ffi_arg));
  avaluep = alloca (cif->nargs * আকার (ffi_arg));

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

  যখন (i < avn)
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
            avaluep[i] = ((অক্ষর *) argp) + আকার (দশমিক);
          অন্যথা
#endif
            avaluep[i] = (অক্ষর *) argp;
        }
      অন্যথা যদি (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (অক্ষর *) argp;
        }
      অন্যথা যদি (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (অক্ষর *) argp;
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
          avaluep[i] = (অক্ষর *) argp;
        }
      অন্যথা
        {
          unsigned type = arg_types[i]->type;

          যদি (arg_types[i]->alignment > আকার(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / আকার(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          যদি (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  যদি (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          নির্বাচন (type)
            {
            ক্ষেত্র FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              বিরতি;

            ক্ষেত্র FFI_TYPE_STRUCT:
              যদি (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->আকার_2);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  বিরতি;
                }
              /* Else fall through.  */
            পূর্বনির্ধারিত:
              avaluep[i] = (অক্ষর *) argp;
              বিরতি;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->আকার_2, আকার(ffi_arg)) / আকার(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_তথ্য);

  ফেরত cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#যদি defined(FFI_MIPS_O32)
বাহ্যিক শূন্য ffi_closure_O32(শূন্য);
বাহ্যিক শূন্য ffi_go_closure_O32(শূন্য);
#অন্যথা
বাহ্যিক শূন্য ffi_closure_N32(শূন্য);
বাহ্যিক শূন্য ffi_go_closure_N32(শূন্য);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*))
{
  শূন্য * fn;

#যদি defined(FFI_MIPS_O32)
  যদি (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#অন্যথা
#যদি _MIPS_SIM ==_ABIN32
  যদি (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
#অন্যথা
  যদি (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ফেরত FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (শূন্য *)fn;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

#endif /* FFI_CLOSURES */

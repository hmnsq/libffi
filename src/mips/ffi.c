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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>

#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ifdef __GNUC__
#  ಯದಿ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  ಯದಿ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ಅನ್ಯಥಾ
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#ಅನ್ಯಥಾ
# define FFI_MIPS_STOP_HERE() ಕುರು {} ಯಾವತ್(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &ರಾಶಿ[bytes]); \
ಯದಿ (argp == &ರಾಶಿ[bytes]) \
{ \
  argp = ರಾಶಿ; \
  FFI_MIPS_STOP_HERE(); \
}
#ಅನ್ಯಥಾ
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_args(ವರ್ಣ *ರಾಶಿ, 
			  extended_cif *ecif,
			  ಪೂರ್ಣಾಙ್ಕ bytes,
			  ಪೂರ್ಣಾಙ್ಕ flags)
{
  ಪೂರ್ಣಾಙ್ಕ i;
  ಶೂನ್ಯ **p_argv;
  ವರ್ಣ *argp, *argp_f;
  ffi_type **p_arg;

  memset(ರಾಶಿ, 0, bytes);

#ifdef FFI_MIPS_N32
  ಪೂರ್ಣಾಙ್ಕ soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ಯದಿ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ಯದಿ (bytes + 16 > 8 * ಪರಿಮಾಣ(ffi_arg))
        argp = &ರಾಶಿ[bytes - (8 * ಪರಿಮಾಣ(ffi_arg))];
      ಅನ್ಯಥಾ
        argp = ರಾಶಿ;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  ಅನ್ಯಥಾ
    {
      ಯದಿ (bytes > 8 * ಪರಿಮಾಣ(ffi_arg))
        argp = &ರಾಶಿ[bytes - (8 * ಪರಿಮಾಣ(ffi_arg))];
      ಅನ್ಯಥಾ
        argp = ರಾಶಿ;
    }
#ಅನ್ಯಥಾ
  argp = ರಾಶಿ;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  ಯದಿ ( ecif->cif->rstruct_flag != 0 )
#ಅನ್ಯಥಾ
  ಯದಿ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += ಪರಿಮಾಣ(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  ಪ್ರತಿ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned ಪೂರ್ಣಾಙ್ಕ a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ಯದಿ (a < ಪರಿಮಾಣ(ffi_arg))
        a = ಪರಿಮಾಣ(ffi_arg);
      
      ಯದಿ ((a - 1) & (unsigned long) argp)
	{
	  argp = (ವರ್ಣ *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      ಯದಿ (z <= ಪರಿಮಾಣ(ffi_arg))
	{
          ಪೂರ್ಣಾಙ್ಕ type = (*p_arg)->type;
	  z = ಪರಿಮಾಣ(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ಯದಿ (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ಯದಿ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ವಿಕಲ್ಪ (type)
	      {
	      ಪಕ್ಷ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ಭಙ್ಗ;
	      ಪಕ್ಷ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ಭಙ್ಗ;
	      ಸ್ವಭಾವ:
		ಭಙ್ಗ;
	      }
	  }
	  ವಿಕಲ್ಪ (type)
	    {
	      ಪಕ್ಷ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ಭಙ್ಗ;

	      ಪಕ್ಷ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ಅನ್ಯಥಾ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		ಭಙ್ಗ;

#ifdef FFI_MIPS_N32
	      ಪಕ್ಷ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ಯದಿ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ಭಿನ್ನ *) argp = *(ಭಿನ್ನ *)(* p_argv);
		    argp += z;
		    ವರ್ಣ *tmp = (ಶೂನ್ಯ *) (*p_argv);
		    *(ಭಿನ್ನ *) argp = *(ಭಿನ್ನ *)(tmp+4);
		  }
		ಅನ್ಯಥಾ
		  memcpy(argp, *p_argv, (*p_arg)->size);
		ಭಙ್ಗ;
#endif
	      /* This can only happen with 64bit slots.  */
	      ಪಕ್ಷ FFI_TYPE_FLOAT:
		*(ಭಿನ್ನ *) argp = *(ಭಿನ್ನ *)(* p_argv);
		ಭಙ್ಗ;

	      /* Handle structures.  */
	      ಸ್ವಭಾವ:
		memcpy(argp, *p_argv, (*p_arg)->size);
		ಭಙ್ಗ;
	    }
	}
      ಅನ್ಯಥಾ
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#ಅನ್ಯಥಾ
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) ರಾಶಿ + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ಯದಿ (end <= cap)
	      memcpy(argp, *p_argv, z);
	    ಅನ್ಯಥಾ
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = ರಾಶಿ;
                z -= portion;
		memcpy(argp, (ಶೂನ್ಯ*)((unsigned long)(*p_argv) + portion),
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

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ಯದಿ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ಯದಿ (*arg_reg > 7)
	ಪ್ರತ್ಯಾಗಮ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ unsigned
calc_n32_struct_flags(ಪೂರ್ಣಾಙ್ಕ soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  ಯದಿ (soft_float)
    ಪ್ರತ್ಯಾಗಮ 0;

  ಯಾವತ್ ((e = arg->elements[index]))
    {
      ಯದಿ (e->type == FFI_TYPE_COMPLEX)
	{
	  ಯದಿ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ಭಙ್ಗ;
	  ಯದಿ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ಭಙ್ಗ;
	}
      ಅನ್ಯಥಾ
	ಯದಿ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ಭಙ್ಗ;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ಪ್ರತ್ಯಾಗಮ flags;
}

ಸ್ಥಿರ unsigned
calc_n32_return_struct_flags(ಪೂರ್ಣಾಙ್ಕ soft_float, ffi_type *arg)
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
  
  ಯದಿ (arg->size > 16)
    ಪ್ರತ್ಯಾಗಮ 0;

  ಯದಿ (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ಯದಿ (e->type == FFI_TYPE_COMPLEX)
    {
      ಪೂರ್ಣಾಙ್ಕ type = e->elements[0]->type;

      ಯದಿ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ಪ್ರತ್ಯಾಗಮ small;

      ಯದಿ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ಪ್ರತ್ಯಾಗಮ small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  ಅನ್ಯಥಾ
    {
      ಯದಿ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ಪ್ರತ್ಯಾಗಮ small;

      flags = e->type;

      ಯದಿ (arg->elements[1])
	{
	  e = arg->elements[1];
	  ಯದಿ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ಪ್ರತ್ಯಾಗಮ small;

	  ಯದಿ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ಪ್ರತ್ಯಾಗಮ small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  ಯದಿ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ಪ್ರತ್ಯಾಗಮ flags;
}

#endif

/* Perform machine dependent cif processing */
ಸ್ಥಿರ ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  ಯದಿ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ಯದಿ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ವಿಕಲ್ಪ ((cif->arg_types)[0]->type)
	    {
	    ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಪಕ್ಷ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ಭಙ್ಗ;
	      
	    ಸ್ವಭಾವ:
	      ಭಙ್ಗ;
	    }

	  ಯದಿ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ಯದಿ (cif->flags)
		{
		  ವಿಕಲ್ಪ ((cif->arg_types)[1]->type)
		    {
		    ಪಕ್ಷ FFI_TYPE_FLOAT:
		    ಪಕ್ಷ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      ಭಙ್ಗ;
		      
		    ಸ್ವಭಾವ:
		      ಭಙ್ಗ;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ಯದಿ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ವಿಕಲ್ಪ (cif->rtype->type)
        {
        ಪಕ್ಷ FFI_TYPE_VOID:
        ಪಕ್ಷ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          ಭಙ್ಗ;

        ಪಕ್ಷ FFI_TYPE_SINT64:
        ಪಕ್ಷ FFI_TYPE_UINT64:
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ಭಙ್ಗ;
      
        ಪಕ್ಷ FFI_TYPE_FLOAT:
        ಸ್ವಭಾವ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ಭಙ್ಗ;
        }
    }
  ಅನ್ಯಥಾ
    {
      /* FFI_O32 */      
      ವಿಕಲ್ಪ (cif->rtype->type)
        {
        ಪಕ್ಷ FFI_TYPE_VOID:
        ಪಕ್ಷ FFI_TYPE_STRUCT:
        ಪಕ್ಷ FFI_TYPE_FLOAT:
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
        ಪಕ್ಷ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  ಯದಿ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          ಭಙ್ಗ;

        ಪಕ್ಷ FFI_TYPE_SINT64:
        ಪಕ್ಷ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ಭಙ್ಗ;
      
        ಸ್ವಭಾವ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ಭಙ್ಗ;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned ಎಣಿಕೆ = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    ಪೂರ್ಣಾಙ್ಕ soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ಯದಿ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ಎಣಿಕೆ = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ಅನ್ಯಥಾ
	    cif->rstruct_flag = 0;
      }
    ಅನ್ಯಥಾ
      cif->rstruct_flag = 0;

    ಯಾವತ್ (ಎಣಿಕೆ-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	ವಿಕಲ್ಪ (t->type)
	  {
	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    ಭಙ್ಗ;
          ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ಯದಿ (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ಅನ್ಯಥಾ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		ಯದಿ (arg_reg >= 8)
		  ಅನುವರ್ತ;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_COMPLEX:
	    ವಿಕಲ್ಪ (t->elements[0]->type)
	      {
	      ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ಯದಿ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ಅನ್ಯಥಾ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ಯದಿ (arg_reg >= 8)
		        ಅನುವರ್ತ;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ಯದಿ (arg_reg >= 8)
		        ಅನುವರ್ತ;
		  }
		/* passthrough */
	      ಪಕ್ಷ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      ಪಕ್ಷ FFI_TYPE_SINT32:
	      ಪಕ್ಷ FFI_TYPE_UINT32:
	      ಪಕ್ಷ FFI_TYPE_DOUBLE:
		ಯದಿ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ಅನ್ಯಥಾ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ಯದಿ (arg_reg >= 8)
		        ಅನುವರ್ತ;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		ಭಙ್ಗ;
	      ಸ್ವಭಾವ:
		arg_reg += 2;
		ಭಙ್ಗ;
	      }
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    ಭಙ್ಗ;

	  ಸ್ವಭಾವ:
	    arg_reg++;
            ಭಙ್ಗ;
	  }

	index++;
      }

  /* Set the return type flag */
    ವಿಕಲ್ಪ (cif->rtype->type)
      {
      ಪಕ್ಷ FFI_TYPE_STRUCT:
	{
	  ಯದಿ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ಅನ್ಯಥಾ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ಭಙ್ಗ;
	}
      
      ಪಕ್ಷ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_POINTER:
	ಯದಿ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ಅನ್ಯಥಾ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_FLOAT:
	ಯದಿ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    ಭಙ್ಗ;
	  }
	/* else fall through */
      ಪಕ್ಷ FFI_TYPE_DOUBLE:
	ಯದಿ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ಅನ್ಯಥಾ
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ಯದಿ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	ಅನ್ಯಥಾ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_COMPLEX:
	{
	  ಪೂರ್ಣಾಙ್ಕ type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  ಯದಿ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ವಿಕಲ್ಪ (type)
		{
		ಪಕ್ಷ FFI_TYPE_DOUBLE:
		ಪಕ್ಷ FFI_TYPE_SINT64:
		ಪಕ್ಷ FFI_TYPE_UINT64:
		ಪಕ್ಷ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ಭಙ್ಗ;
		ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ಭಙ್ಗ;
		ಪಕ್ಷ FFI_TYPE_FLOAT:
		ಸ್ವಭಾವ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ಅನ್ಯಥಾ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ಭಙ್ಗ;
	}
      ಪಕ್ಷ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ಭಙ್ಗ;
      ಪಕ್ಷ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ಭಙ್ಗ;
      ಸ್ವಭಾವ:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ಭಙ್ಗ;
      }
  }
#endif
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    ಪ್ರತ್ಯಾಗಮ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ffi_call_O32(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ *closure);

/* Low level routine for calling N32 functions */
ಬಾಹ್ಯ ಪೂರ್ಣಾಙ್ಕ ffi_call_N32(ಶೂನ್ಯ (*)(ವರ್ಣ *, extended_cif *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ), 
			extended_cif *, unsigned, 
			unsigned, ಶೂನ್ಯ *, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ *closure);

ಶೂನ್ಯ ffi_call_int(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, 
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  ಯದಿ ((rvalue == ರಿಕ್ತ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  ಅನ್ಯಥಾ
    ecif.rvalue = rvalue;
    
  ವಿಕಲ್ಪ (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    ಪಕ್ಷ FFI_O32:
    ಪಕ್ಷ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      ಭಙ್ಗ;
#endif

#ifdef FFI_MIPS_N32
    ಪಕ್ಷ FFI_N32:
    ಪಕ್ಷ FFI_N32_SOFT_FLOAT:
    ಪಕ್ಷ FFI_N64:
    ಪಕ್ಷ FFI_N64_SOFT_FLOAT:
      {
        ಪೂರ್ಣಾಙ್ಕ copy_rvalue = 0;
	ಪೂರ್ಣಾಙ್ಕ copy_offset = 0;
        ವರ್ಣ *rvalue_copy = ecif.rvalue;
        ಯದಿ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ಅನ್ಯಥಾ ಯದಿ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ಯದಿ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        ಯದಿ (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      ಭಙ್ಗ;
#endif

    ಸ್ವಭಾವ:
      FFI_ASSERT(0);
      ಭಙ್ಗ;
    }
}

ಶೂನ್ಯ
ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#ಯದಿ FFI_CLOSURES
#ಯದಿ defined(FFI_MIPS_O32)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_O32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_O32(ಶೂನ್ಯ);
#ಅನ್ಯಥಾ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_N32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_N32(ಶೂನ್ಯ);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ *tramp = (unsigned ಪೂರ್ಣಾಙ್ಕ *) &closure->tramp[0];
  ಶೂನ್ಯ * fn;
  ವರ್ಣ *clear_location = (ವರ್ಣ *) codeloc;

#ಯದಿ defined(FFI_MIPS_O32)
  ಯದಿ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
  fn = ffi_closure_O32;
#ಅನ್ಯಥಾ
#ಯದಿ _MIPS_SIM ==_ABIN32
  ಯದಿ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#ಅನ್ಯಥಾ
  ಯದಿ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#ಯದಿ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#ಯದಿ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ಅನ್ಯಥಾ
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#ಅನ್ಯಥಾ
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
#ಯದಿ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ಅನ್ಯಥಾ
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ಯದಿ !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#ಅನ್ಯಥಾ
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  ಪ್ರತ್ಯಾಗಮ FFI_OK;
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
ಪೂರ್ಣಾಙ್ಕ
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			    ಶೂನ್ಯ *user_data,
			    ಶೂನ್ಯ *rvalue, ffi_arg *ar,
			    ದ್ವಿಗುಣ *fpr)
{
  ಶೂನ್ಯ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ffi_arg));
  avaluep = alloca (cif->nargs * ಪರಿಮಾಣ (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ಯದಿ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (ಶೂನ್ಯ *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  ಯದಿ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಯಾವತ್ (i < avn)
    {
      ಯದಿ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ಯದಿ (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ಯದಿ defined(__MIPSEB__) || defined(_MIPSEB)
	  ಯದಿ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((ವರ್ಣ *) &fpr[i]) + ಪರಿಮಾಣ (ಭಿನ್ನ);
	  ಅನ್ಯಥಾ
#endif
	    avaluep[i] = (ವರ್ಣ *) &fpr[i];
	}
      ಅನ್ಯಥಾ
	{
	  ವಿಕಲ್ಪ (arg_types[i]->type)
	    {
	      ಪಕ್ಷ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ಭಙ್ಗ;

	      ಪಕ್ಷ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ಭಙ್ಗ;
		  
	      ಪಕ್ಷ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ಭಙ್ಗ;

	      ಸ್ವಭಾವ:
		avaluep[i] = (ವರ್ಣ *) &ar[argn];
		ಭಙ್ಗ;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  ಯದಿ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ವಿಕಲ್ಪ (cif->rtype->type)
        {
        ಪಕ್ಷ FFI_TYPE_FLOAT:
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_INT;
        ಪಕ್ಷ FFI_TYPE_DOUBLE:
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT64;
        ಸ್ವಭಾವ:
          ಪ್ರತ್ಯಾಗಮ cif->rtype->type;
        }
    }
  ಅನ್ಯಥಾ
    {
      ಯದಿ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ ಚಞ್ಚಲ ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      ಪ್ರತ್ಯಾಗಮ cif->rtype->type;
    }
}

#ಯದಿ defined(FFI_MIPS_N32)

ಸ್ಥಿರ ಶೂನ್ಯ
copy_struct_N32(ವರ್ಣ *target, unsigned offset, ffi_abi abi, ffi_type *type,
                ಪೂರ್ಣಾಙ್ಕ argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, ಪೂರ್ಣಾಙ್ಕ soft_float)
{
  ffi_type **elt_typep = type->elements;
  ಯಾವತ್(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      ವರ್ಣ *tp;
      ವರ್ಣ *argp;
      ವರ್ಣ *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / ಪರಿಮಾಣ(ffi_arg);
      arg_offset = arg_offset % ಪರಿಮಾಣ(ffi_arg);

      argp = (ವರ್ಣ *)(ar + argn);
      fpp = (ವರ್ಣ *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      ಯದಿ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ದ್ವಿಗುಣ *)tp = *(ದ್ವಿಗುಣ *)fpp;
      ಅನ್ಯಥಾ
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / ಪರಿಮಾಣ(ffi_arg);
      arg_offset = arg_offset % ಪರಿಮಾಣ(ffi_arg);
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
ಪೂರ್ಣಾಙ್ಕ
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
                            ಶೂನ್ಯ *user_data,
			    ಶೂನ್ಯ *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  ಶೂನ್ಯ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಙ್ಕ i, avn, argn;
  ಪೂರ್ಣಾಙ್ಕ soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ffi_arg));
  avaluep = alloca (cif->nargs * ಪರಿಮಾಣ (ffi_arg));

  argn = 0;

  ಯದಿ (cif->rstruct_flag)
    {
#ಯದಿ _MIPS_SIM==_ABIN32
      rvalue = (ಶೂನ್ಯ *)(UINT32)ar[0];
#ಅನ್ಯಥಾ /* N64 */
      rvalue = (ಶೂನ್ಯ *)ar[0];
#endif
      argn = 1;
    }
  ಯದಿ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಯಾವತ್ (i < avn)
    {
      ಯದಿ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ಯದಿ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ಯದಿ defined(__MIPSEB__) || defined(_MIPSEB)
          ಯದಿ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((ವರ್ಣ *) argp) + ಪರಿಮಾಣ (ಭಿನ್ನ);
          ಅನ್ಯಥಾ
#endif
            avaluep[i] = (ವರ್ಣ *) argp;
        }
      ಅನ್ಯಥಾ ಯದಿ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ವರ್ಣ *) argp;
        }
      ಅನ್ಯಥಾ ಯದಿ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ವರ್ಣ *) argp;
        }
      ಅನ್ಯಥಾ ಯದಿ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ಯದಿ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ಅನ್ಯಥಾ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (ವರ್ಣ *) argp;
        }
      ಅನ್ಯಥಾ
        {
          unsigned type = arg_types[i]->type;

          ಯದಿ (arg_types[i]->alignment > ಪರಿಮಾಣ(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / ಪರಿಮಾಣ(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ಯದಿ (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ಯದಿ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ವಿಕಲ್ಪ (type)
            {
            ಪಕ್ಷ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ಭಙ್ಗ;

            ಪಕ್ಷ FFI_TYPE_STRUCT:
              ಯದಿ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ಭಙ್ಗ;
                }
              /* Else fall through.  */
            ಸ್ವಭಾವ:
              avaluep[i] = (ವರ್ಣ *) argp;
              ಭಙ್ಗ;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, ಪರಿಮಾಣ(ffi_arg)) / ಪರಿಮಾಣ(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  ಪ್ರತ್ಯಾಗಮ cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#ಯದಿ defined(FFI_MIPS_O32)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_O32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_O32(ಶೂನ್ಯ);
#ಅನ್ಯಥಾ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_N32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_N32(ಶೂನ್ಯ);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ * fn;

#ಯದಿ defined(FFI_MIPS_O32)
  ಯದಿ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#ಅನ್ಯಥಾ
#ಯದಿ _MIPS_SIM ==_ABIN32
  ಯದಿ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#ಅನ್ಯಥಾ
  ಯದಿ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (ಶೂನ್ಯ *)fn;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#endif /* FFI_CLOSURES */

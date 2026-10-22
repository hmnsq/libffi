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

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __GNUC__
#  ਜੇ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ USE__BUILTIN___CLEAR_CACHE
#  ਜੇ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ਨਹੀਂਤਾਂ
#    include <sys/cachectl.h>
#  endif
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_DEBUG
# define FFI_MIPS_STOP_ਇੱਥੇ() ffi_stop_ਇੱਥੇ()
#ਨਹੀਂਤਾਂ
# define FFI_MIPS_STOP_ਇੱਥੇ() ਕਰੋ {} ਜਦੋਂ(0)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FIX_ARGP \
FFI_ASSERT(argp <= &ਢੇਰ[ਬਾਈਟਾਂ]); \
ਜੇ (argp == &ਢੇਰ[ਬਾਈਟਾਂ]) \
{ \
  argp = ਢੇਰ; \
  FFI_MIPS_STOP_ਇੱਥੇ(); \
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FIX_ARGP 
#ਜੇ_ਅੰਤ


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ਸਥਿਰ ਖਾਲੀ ffi_prep_ਦਲੀਲਾਂ(ਅੱਖਰ *ਢੇਰ, 
			  extended_cif *ecif,
			  ਪੂਰਨ ਬਾਈਟਾਂ,
			  ਪੂਰਨ flags)
{
  ਪੂਰਨ i;
  ਖਾਲੀ **p_argv;
  ਅੱਖਰ *argp, *argp_f;
  ffi_type **p_arg;

  memset(ਢੇਰ, 0, ਬਾਈਟਾਂ);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
  ਪੂਰਨ soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ਜੇ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ਜੇ (ਬਾਈਟਾਂ + 16 > 8 * ਆਕਾਰ(ffi_arg))
        argp = &ਢੇਰ[ਬਾਈਟਾਂ - (8 * ਆਕਾਰ(ffi_arg))];
      ਨਹੀਂਤਾਂ
        argp = ਢੇਰ;
      * (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) argp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ecif->rvalue;
      argp += 16;
    }
  ਨਹੀਂਤਾਂ
    {
      ਜੇ (ਬਾਈਟਾਂ > 8 * ਆਕਾਰ(ffi_arg))
        argp = &ਢੇਰ[ਬਾਈਟਾਂ - (8 * ਆਕਾਰ(ffi_arg))];
      ਨਹੀਂਤਾਂ
        argp = ਢੇਰ;
    }
#ਨਹੀਂਤਾਂ
  argp = ਢੇਰ;
#ਜੇ_ਅੰਤ

  argp_f = argp;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
  ਜੇ ( ecif->cif->rstruct_flag != 0 )
#ਨਹੀਂਤਾਂ
  ਜੇ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#ਜੇ_ਅੰਤ  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += ਆਕਾਰ(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  ਲਈ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ਜੇ (a < ਆਕਾਰ(ffi_arg))
        a = ਆਕਾਰ(ffi_arg);
      
      ਜੇ ((a - 1) & (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) argp)
	{
	  argp = (ਅੱਖਰ *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->ਮਾਪ;
      ਜੇ (z <= ਆਕਾਰ(ffi_arg))
	{
          ਪੂਰਨ type = (*p_arg)->type;
	  z = ਆਕਾਰ(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ਜੇ (type == FFI_TYPE_ਸੂਚਕ)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ਜੇ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ਚੋਣ (type)
	      {
	      ਹਾਲਤ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ਰੋਕੋ;
	      ਹਾਲਤ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ਰੋਕੋ;
	      ਮੂਲ:
		ਰੋਕੋ;
	      }
	  }
	  ਚੋਣ (type)
	    {
	      ਹਾਲਤ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ਰੋਕੋ;

	      ਹਾਲਤ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_UINT32:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ਨਹੀਂਤਾਂ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#ਜੇ_ਅੰਤ
		ਰੋਕੋ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
	      ਹਾਲਤ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ਜੇ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ਦਸ਼ਮਲਵ *) argp = *(ਦਸ਼ਮਲਵ *)(* p_argv);
		    argp += z;
		    ਅੱਖਰ *tmp = (ਖਾਲੀ *) (*p_argv);
		    *(ਦਸ਼ਮਲਵ *) argp = *(ਦਸ਼ਮਲਵ *)(tmp+4);
		  }
		ਨਹੀਂਤਾਂ
		  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, (*p_arg)->ਮਾਪ);
		ਰੋਕੋ;
#ਜੇ_ਅੰਤ
	      /* This can only happen with 64bit slots.  */
	      ਹਾਲਤ FFI_TYPE_FLOAT:
		*(ਦਸ਼ਮਲਵ *) argp = *(ਦਸ਼ਮਲਵ *)(* p_argv);
		ਰੋਕੋ;

	      /* Handle structures.  */
	      ਮੂਲ:
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, (*p_arg)->ਮਾਪ);
		ਰੋਕੋ;
	    }
	}
      ਨਹੀਂਤਾਂ
	{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
#ਨਹੀਂਤਾਂ
	  {
	    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਿਰਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) argp + z;
	    ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ cap = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਢੇਰ + ਬਾਈਟਾਂ;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ਜੇ (ਸਿਰਾ <= cap)
	      ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, z);
	    ਨਹੀਂਤਾਂ
	      {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ portion = cap - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)argp;

		ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, *p_argv, portion);
		argp = ਢੇਰ;
                z -= portion;
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(argp, (ਖਾਲੀ*)((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(*p_argv) + portion),
                       z);
	      }
	  }
#ਜੇ_ਅੰਤ
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

ਸਥਿਰ ਪੂਰਨ
calc_n32_struct_flags_element(ਬਿਨਾਨਿਸ਼ਾਨੀ *flags, ffi_type *e,
			      ਬਿਨਾਨਿਸ਼ਾਨੀ *loc, ਬਿਨਾਨਿਸ਼ਾਨੀ *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ਜੇ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ਜੇ (*arg_reg > 7)
	ਵਾਪਸ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_ਬਿੱਟ));
    }
  *loc += e->ਮਾਪ;
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ
calc_n32_struct_flags(ਪੂਰਨ soft_float, ffi_type *arg,
		      ਬਿਨਾਨਿਸ਼ਾਨੀ *loc, ਬਿਨਾਨਿਸ਼ਾਨੀ *arg_reg)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ = 0;

  ffi_type *e;

  ਜੇ (soft_float)
    ਵਾਪਸ 0;

  ਜਦੋਂ ((e = arg->elements[ਸੂਚਕਾਂਕ]))
    {
      ਜੇ (e->type == FFI_TYPE_COMPLEX)
	{
	  ਜੇ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ਰੋਕੋ;
	  ਜੇ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ਰੋਕੋ;
	}
      ਨਹੀਂਤਾਂ
	ਜੇ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ਰੋਕੋ;
      ਸੂਚਕਾਂਕ++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ਵਾਪਸ flags;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ
calc_n32_return_struct_flags(ਪੂਰਨ soft_float, ffi_type *arg)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
  ਬਿਨਾਨਿਸ਼ਾਨੀ small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  ਜੇ (arg->ਮਾਪ > 16)
    ਵਾਪਸ 0;

  ਜੇ (arg->ਮਾਪ > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ਜੇ (e->type == FFI_TYPE_COMPLEX)
    {
      ਪੂਰਨ type = e->elements[0]->type;

      ਜੇ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ਵਾਪਸ small;

      ਜੇ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ਵਾਪਸ small;
	}

      flags = (type << FFI_FLAG_ਬਿੱਟ) + type;
    }
  ਨਹੀਂਤਾਂ
    {
      ਜੇ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ਵਾਪਸ small;

      flags = e->type;

      ਜੇ (arg->elements[1])
	{
	  e = arg->elements[1];
	  ਜੇ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ਵਾਪਸ small;

	  ਜੇ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ਵਾਪਸ small;
	    }

	  flags += e->type << FFI_FLAG_ਬਿੱਟ;
	}
    }

  ਜੇ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ਵਾਪਸ flags;
}

#ਜੇ_ਅੰਤ

/* Perform machine dependent cif processing */
ਸਥਿਰ ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_int(ffi_cif *cif, ਬਿਨਾਨਿਸ਼ਾਨੀ nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  ਜੇ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ਜੇ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ਚੋਣ ((cif->arg_types)[0]->type)
	    {
	    ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਹਾਲਤ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ਰੋਕੋ;
	      
	    ਮੂਲ:
	      ਰੋਕੋ;
	    }

	  ਜੇ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ਜੇ (cif->flags)
		{
		  ਚੋਣ ((cif->arg_types)[1]->type)
		    {
		    ਹਾਲਤ FFI_TYPE_FLOAT:
		    ਹਾਲਤ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_ਬਿੱਟ;
		      ਰੋਕੋ;
		      
		    ਮੂਲ:
		      ਰੋਕੋ;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ਜੇ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ਚੋਣ (cif->rtype->type)
        {
        ਹਾਲਤ FFI_TYPE_VOID:
        ਹਾਲਤ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_ਬਿੱਟ * 2);
          ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_SINT64:
        ਹਾਲਤ FFI_TYPE_UINT64:
        ਹਾਲਤ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ਬਿੱਟ * 2);
          ਰੋਕੋ;
      
        ਹਾਲਤ FFI_TYPE_FLOAT:
        ਮੂਲ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_ਬਿੱਟ * 2);
          ਰੋਕੋ;
        }
    }
  ਨਹੀਂਤਾਂ
    {
      /* FFI_O32 */      
      ਚੋਣ (cif->rtype->type)
        {
        ਹਾਲਤ FFI_TYPE_VOID:
        ਹਾਲਤ FFI_TYPE_STRUCT:
        ਹਾਲਤ FFI_TYPE_FLOAT:
        ਹਾਲਤ FFI_TYPE_DOUBLE:
        ਹਾਲਤ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_ਬਿੱਟ * 2);
	  ਜੇ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_ਬਿੱਟ * 4);
          ਰੋਕੋ;

        ਹਾਲਤ FFI_TYPE_SINT64:
        ਹਾਲਤ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ਬਿੱਟ * 2);
          ਰੋਕੋ;
      
        ਮੂਲ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_ਬਿੱਟ * 2);
          ਰੋਕੋ;
        }
    }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    ਬਿਨਾਨਿਸ਼ਾਨੀ arg_reg = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ loc = 0;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2 = (cif->nargs < 8) ? cif->nargs : 8;
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੂਚਕਾਂਕ = 0;

    ਬਿਨਾਨਿਸ਼ਾਨੀ struct_flags = 0;
    ਪੂਰਨ soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ਜੇ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ਗਿਣਤੀ_2 = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ਨਹੀਂਤਾਂ
	    cif->rstruct_flag = 0;
      }
    ਨਹੀਂਤਾਂ
      cif->rstruct_flag = 0;

    ਜਦੋਂ (ਗਿਣਤੀ_2-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[ਸੂਚਕਾਂਕ];

	ਚੋਣ (t->type)
	  {
	  ਹਾਲਤ FFI_TYPE_FLOAT:
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (!soft_float && ਸੂਚਕਾਂਕ < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_ਬਿੱਟ);
	    arg_reg++;
	    ਰੋਕੋ;
          ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ਜੇ (soft_float || ਸੂਚਕਾਂਕ >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ਨਹੀਂਤਾਂ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		arg_reg++;
		ਜੇ (arg_reg >= 8)
		  ਜਾਰੀ;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		arg_reg++;
	      }
            ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_COMPLEX:
	    ਚੋਣ (t->elements[0]->type)
	      {
	      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ਜੇ (soft_float || ਸੂਚਕਾਂਕ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ਨਹੀਂਤਾਂ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		    arg_reg++;
		    ਜੇ (arg_reg >= 8)
		        ਜਾਰੀ;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		    arg_reg++;
		    ਜੇ (arg_reg >= 8)
		        ਜਾਰੀ;
		  }
		/* passthrough */
	      ਹਾਲਤ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->ਬਾਈਟਾਂ += 16;
		/* passthrough */
	      ਹਾਲਤ FFI_TYPE_SINT32:
	      ਹਾਲਤ FFI_TYPE_UINT32:
	      ਹਾਲਤ FFI_TYPE_DOUBLE:
		ਜੇ (soft_float || ਸੂਚਕਾਂਕ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ਨਹੀਂਤਾਂ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		    arg_reg++;
		    ਜੇ (arg_reg >= 8)
		        ਜਾਰੀ;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_ਬਿੱਟ));
		    arg_reg++;
		  }
		ਰੋਕੋ;
	      ਮੂਲ:
		arg_reg += 2;
		ਰੋਕੋ;
	      }
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || ਸੂਚਕਾਂਕ >= nfixedargs,
						t, &loc, &arg_reg);
	    ਰੋਕੋ;

	  ਮੂਲ:
	    arg_reg++;
            ਰੋਕੋ;
	  }

	ਸੂਚਕਾਂਕ++;
      }

  /* Set the return type flag */
    ਚੋਣ (cif->rtype->type)
      {
      ਹਾਲਤ FFI_TYPE_STRUCT:
	{
	  ਜੇ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_ਬਿੱਟ * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_ਬਿੱਟ * 8));
	    }
	  ਰੋਕੋ;
	}
      
      ਹਾਲਤ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	ਜੇ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ਬਿੱਟ * 8);
	ਨਹੀਂਤਾਂ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ਬਿੱਟ * 8);
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_FLOAT:
	ਜੇ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ਬਿੱਟ * 8);
	    ਰੋਕੋ;
	  }
	/* else fall through */
      ਹਾਲਤ FFI_TYPE_DOUBLE:
	ਜੇ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ਬਿੱਟ * 8);
	ਨਹੀਂਤਾਂ
	  cif->flags += cif->rtype->type << (FFI_FLAG_ਬਿੱਟ * 8);
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ਜੇ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_ਬਿੱਟ * 8);
 	  }
	ਨਹੀਂਤਾਂ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_ਬਿੱਟ * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_ਬਿੱਟ))
					      << (4 + (FFI_FLAG_ਬਿੱਟ * 8));
	  }
	ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_COMPLEX:
	{
	  ਪੂਰਨ type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_ਬਿੱਟ * 8));
	  ਜੇ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ਚੋਣ (type)
		{
		ਹਾਲਤ FFI_TYPE_DOUBLE:
		ਹਾਲਤ FFI_TYPE_SINT64:
		ਹਾਲਤ FFI_TYPE_UINT64:
		ਹਾਲਤ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ਰੋਕੋ;
		ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ਰੋਕੋ;
		ਹਾਲਤ FFI_TYPE_FLOAT:
		ਮੂਲ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_ਬਿੱਟ * 8));
	    }
	  ਨਹੀਂਤਾਂ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_ਬਿੱਟ * 8));
	    }
	  ਰੋਕੋ;
	}
      ਹਾਲਤ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ਬਿੱਟ * 8);
	ਰੋਕੋ;
      ਹਾਲਤ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ਬਿੱਟ * 8);
	ਰੋਕੋ;
      ਮੂਲ:
	cif->flags += cif->rtype->type << (FFI_FLAG_ਬਿੱਟ * 8);
	ਰੋਕੋ;
      }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ FFI_OK;
}

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep(ffi_cif *cif)
{
    ਵਾਪਸ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_ਸਥਿਤੀ ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ nfixedargs,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ ntotalargs MAYBE_ਅਣਵਰਤਿਆ)
{
    ਵਾਪਸ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
ਬਾਹਰੀ ਪੂਰਨ ffi_call_O32(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *, ਪੂਰਨ, ਪੂਰਨ), 
			extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, 
			ਬਿਨਾਨਿਸ਼ਾਨੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ *, ਖਾਲੀ (*)(ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ);

/* Low level routine for calling N32 functions */
ਬਾਹਰੀ ਪੂਰਨ ffi_call_N32(ਖਾਲੀ (*)(ਅੱਖਰ *, extended_cif *, ਪੂਰਨ, ਪੂਰਨ), 
			extended_cif *, ਬਿਨਾਨਿਸ਼ਾਨੀ, 
			ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *, ਖਾਲੀ (*)(ਖਾਲੀ), ਖਾਲੀ *ਘੇਰਾ);

ਖਾਲੀ ffi_call_int(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, 
	      ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  ਜੇ ((rvalue == ਨਿਰਾ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->ਮਾਪ);
  ਨਹੀਂਤਾਂ
    ecif.rvalue = rvalue;
    
  ਚੋਣ (cif->abi) 
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_O32
    ਹਾਲਤ FFI_O32:
    ਹਾਲਤ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ, 
		   cif->flags, ecif.rvalue, fn, ਘੇਰਾ);
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ FFI_MIPS_N32
    ਹਾਲਤ FFI_N32:
    ਹਾਲਤ FFI_N32_SOFT_FLOAT:
    ਹਾਲਤ FFI_N64:
    ਹਾਲਤ FFI_N64_SOFT_FLOAT:
      {
        ਪੂਰਨ copy_rvalue = 0;
	ਪੂਰਨ copy_ਵਿੱਥ = 0;
        ਅੱਖਰ *rvalue_copy = ecif.rvalue;
        ਜੇ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->ਮਾਪ < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ਨਹੀਂਤਾਂ ਜੇ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ਜੇ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_ਵਿੱਥ = 4;
#ਜੇ_ਅੰਤ
	  }
        ffi_call_N32(ffi_prep_ਦਲੀਲਾਂ, &ecif, cif->ਬਾਈਟਾਂ,
                     cif->flags, rvalue_copy, fn, ਘੇਰਾ);
        ਜੇ (copy_rvalue)
          ਸਮ੍ਰਿਤੀ_ਨਕਲ(ecif.rvalue, rvalue_copy + copy_ਵਿੱਥ, cif->rtype->ਮਾਪ);
      }
      ਰੋਕੋ;
#ਜੇ_ਅੰਤ

    ਮੂਲ:
      FFI_ASSERT(0);
      ਰੋਕੋ;
    }
}

ਖਾਲੀ
ffi_call(ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ਘੇਰਾ);
}


#ਜੇ FFI_CLOSURES
#ਜੇ defined(FFI_MIPS_O32)
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_O32(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_O32(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_N32(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_N32(ਖਾਲੀ);
#ਜੇ_ਅੰਤ /* FFI_MIPS_O32 */

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) &ਘੇਰਾ->tramp[0];
  ਖਾਲੀ * fn;
  ਅੱਖਰ *clear_location = (ਅੱਖਰ *) codeloc;

#ਜੇ defined(FFI_MIPS_O32)
  ਜੇ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
  fn = ffi_ਘੇਰਾ_O32;
#ਨਹੀਂਤਾਂ
#ਜੇ _MIPS_SIM ==_ABIN32
  ਜੇ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
#ਨਹੀਂਤਾਂ
  ਜੇ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
  fn = ffi_ਘੇਰਾ_N32;
#ਜੇ_ਅੰਤ /* FFI_MIPS_O32 */

#ਜੇ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ)codeloc >> 16);
  /* jr   $25          */
#ਜੇ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ਨਹੀਂਤਾਂ
  tramp[3] = 0x03200009;
#ਜੇ_ਅੰਤ
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ)codeloc & 0xffff);
#ਨਹੀਂਤਾਂ
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)fn  & 0xffff);
  /* jr   $25          */
#ਜੇ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ਨਹੀਂਤਾਂ
  tramp[11] = 0x03200009;
#ਜੇ_ਅੰਤ
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc & 0xffff);

#ਜੇ_ਅੰਤ

  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

#ਜੇ !defined(__FreeBSD__)
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_ਮਾਪ);
#ਨਹੀਂਤਾਂ
  cacheflush (clear_location, FFI_TRAMPOLINE_ਮਾਪ, ICACHE);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* ! __FreeBSD__ */
  ਵਾਪਸ FFI_OK;
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
ਪੂਰਨ
ffi_ਘੇਰਾ_mips_inner_O32 (ffi_cif *cif,
                            ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
			    ਖਾਲੀ *user_ਅੰਕੜੇ,
			    ਖਾਲੀ *rvalue, ffi_arg *ar,
			    ਦੁੱਗਣਾ *fpr)
{
  ਖਾਲੀ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ਪੂਰਨ i, avn, argn, ਵੇਖਿਆ_int;

  avalue = alloca (cif->nargs * ਆਕਾਰ (ffi_arg));
  avaluep = alloca (cif->nargs * ਆਕਾਰ (ffi_arg));

  ਵੇਖਿਆ_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ਜੇ ((cif->flags >> (FFI_FLAG_ਬਿੱਟ * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (ਖਾਲੀ *)(uintptr_t)ar[0];
      argn = 1;
      ਵੇਖਿਆ_int = 1;
    }
  ਜੇ ((cif->flags >> (FFI_FLAG_ਬਿੱਟ * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ਜਦੋਂ (i < avn)
    {
      ਜੇ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ਜੇ (i < 2 && !ਵੇਖਿਆ_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ਜੇ defined(__MIPSEB__) || defined(_MIPSEB)
	  ਜੇ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((ਅੱਖਰ *) &fpr[i]) + ਆਕਾਰ (ਦਸ਼ਮਲਵ);
	  ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
	    avaluep[i] = (ਅੱਖਰ *) &fpr[i];
	}
      ਨਹੀਂਤਾਂ
	{
	  ਚੋਣ (arg_types[i]->type)
	    {
	      ਹਾਲਤ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ਰੋਕੋ;

	      ਹਾਲਤ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ਰੋਕੋ;
		  
	      ਹਾਲਤ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ਰੋਕੋ;

	      ਮੂਲ:
		avaluep[i] = (ਅੱਖਰ *) &ar[argn];
		ਰੋਕੋ;
	    }
	  ਵੇਖਿਆ_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->ਮਾਪ, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_ਅੰਕੜੇ);

  ਜੇ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ਚੋਣ (cif->rtype->type)
        {
        ਹਾਲਤ FFI_TYPE_FLOAT:
          ਵਾਪਸ FFI_TYPE_INT;
        ਹਾਲਤ FFI_TYPE_DOUBLE:
          ਵਾਪਸ FFI_TYPE_UINT64;
        ਮੂਲ:
          ਵਾਪਸ cif->rtype->type;
        }
    }
  ਨਹੀਂਤਾਂ
    {
      ਜੇ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ ਅਸਥਿਰ ("move $v1, %0" : : "r"(cif->rtype->ਮਾਪ));
      }
      ਵਾਪਸ cif->rtype->type;
    }
}

#ਜੇ defined(FFI_MIPS_N32)

ਸਥਿਰ ਖਾਲੀ
copy_struct_N32(ਅੱਖਰ *ਨਿਸ਼ਾਨਾ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ offset, ffi_abi abi, ffi_type *type,
                ਪੂਰਨ argn, ਬਿਨਾਨਿਸ਼ਾਨੀ arg_ਵਿੱਥ, ffi_arg *ar,
                ffi_arg *fpr, ਪੂਰਨ soft_float)
{
  ffi_type **elt_typep = type->elements;
  ਜਦੋਂ(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      ਬਿਨਾਨਿਸ਼ਾਨੀ o;
      ਅੱਖਰ *tp;
      ਅੱਖਰ *argp;
      ਅੱਖਰ *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_ਵਿੱਥ += o - offset;
      offset = o;
      argn += arg_ਵਿੱਥ / ਆਕਾਰ(ffi_arg);
      arg_ਵਿੱਥ = arg_ਵਿੱਥ % ਆਕਾਰ(ffi_arg);

      argp = (ਅੱਖਰ *)(ar + argn);
      fpp = (ਅੱਖਰ *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = ਨਿਸ਼ਾਨਾ_2 + offset;

      ਜੇ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ਦੁੱਗਣਾ *)tp = *(ਦੁੱਗਣਾ *)fpp;
      ਨਹੀਂਤਾਂ
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(tp, argp + arg_ਵਿੱਥ, elt_type->ਮਾਪ);

      offset += elt_type->ਮਾਪ;
      arg_ਵਿੱਥ += elt_type->ਮਾਪ;
      elt_typep++;
      argn += arg_ਵਿੱਥ / ਆਕਾਰ(ffi_arg);
      arg_ਵਿੱਥ = arg_ਵਿੱਥ % ਆਕਾਰ(ffi_arg);
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
ਪੂਰਨ
ffi_ਘੇਰਾ_mips_inner_N32 (ffi_cif *cif, 
			    ਖਾਲੀ (*fun)(ffi_cif*, ਖਾਲੀ*, ਖਾਲੀ**, ਖਾਲੀ*),
                            ਖਾਲੀ *user_ਅੰਕੜੇ,
			    ਖਾਲੀ *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  ਖਾਲੀ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ਪੂਰਨ i, avn, argn;
  ਪੂਰਨ soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * ਆਕਾਰ (ffi_arg));
  avaluep = alloca (cif->nargs * ਆਕਾਰ (ffi_arg));

  argn = 0;

  ਜੇ (cif->rstruct_flag)
    {
#ਜੇ _MIPS_SIM==_ABIN32
      rvalue = (ਖਾਲੀ *)(UINT32)ar[0];
#ਨਹੀਂਤਾਂ /* N64 */
      rvalue = (ਖਾਲੀ *)ar[0];
#ਜੇ_ਅੰਤ
      argn = 1;
    }
  ਜੇ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ਜਦੋਂ (i < avn)
    {
      ਜੇ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ਜੇ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ਜੇ defined(__MIPSEB__) || defined(_MIPSEB)
          ਜੇ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((ਅੱਖਰ *) argp) + ਆਕਾਰ (ਦਸ਼ਮਲਵ);
          ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
            avaluep[i] = (ਅੱਖਰ *) argp;
        }
      ਨਹੀਂਤਾਂ ਜੇ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ਅੱਖਰ *) argp;
        }
      ਨਹੀਂਤਾਂ ਜੇ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ਅੱਖਰ *) argp;
        }
      ਨਹੀਂਤਾਂ ਜੇ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ਜੇ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ਨਹੀਂਤਾਂ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (ਅੱਖਰ *) argp;
        }
      ਨਹੀਂਤਾਂ
        {
          ਬਿਨਾਨਿਸ਼ਾਨੀ type = arg_types[i]->type;

          ਜੇ (arg_types[i]->alignment > ਆਕਾਰ(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / ਆਕਾਰ(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ਜੇ (type == FFI_TYPE_ਸੂਚਕ)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ਜੇ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ਚੋਣ (type)
            {
            ਹਾਲਤ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ਰੋਕੋ;

            ਹਾਲਤ FFI_TYPE_STRUCT:
              ਜੇ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->ਮਾਪ);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ਰੋਕੋ;
                }
              /* Else fall through.  */
            ਮੂਲ:
              avaluep[i] = (ਅੱਖਰ *) argp;
              ਰੋਕੋ;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->ਮਾਪ, ਆਕਾਰ(ffi_arg)) / ਆਕਾਰ(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_ਅੰਕੜੇ);

  ਵਾਪਸ cif->flags >> (FFI_FLAG_ਬਿੱਟ * 8);
}

#ਜੇ_ਅੰਤ /* FFI_MIPS_N32 */

#ਜੇ defined(FFI_MIPS_O32)
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_O32(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_O32(ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_N32(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_N32(ਖਾਲੀ);
#ਜੇ_ਅੰਤ /* FFI_MIPS_O32 */

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ* ਘੇਰਾ, ffi_cif* cif,
		     ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*))
{
  ਖਾਲੀ * fn;

#ਜੇ defined(FFI_MIPS_O32)
  ਜੇ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
  fn = ffi_go_ਘੇਰਾ_O32;
#ਨਹੀਂਤਾਂ
#ਜੇ _MIPS_SIM ==_ABIN32
  ਜੇ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
#ਨਹੀਂਤਾਂ
  ਜੇ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ਵਾਪਸ FFI_BAD_ABI;
#ਜੇ_ਅੰਤ
  fn = ffi_go_ਘੇਰਾ_N32;
#ਜੇ_ਅੰਤ /* FFI_MIPS_O32 */

  ਘੇਰਾ->tramp = (ਖਾਲੀ *)fn;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ_ਅੰਤ /* FFI_CLOSURES */

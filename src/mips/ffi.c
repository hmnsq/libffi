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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __GNUC__
#  ಆದರೆ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ USE__BUILTIN___CLEAR_CACHE
#  ಆದರೆ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ಇಲ್ಲದಿದ್ದರೆ
#    include <sys/cachectl.h>
#  endif
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_DEBUG
# define FFI_MIPS_STOP_ಇಲ್ಲಿ() ffi_stop_ಇಲ್ಲಿ()
#ಇಲ್ಲದಿದ್ದರೆ
# define FFI_MIPS_STOP_ಇಲ್ಲಿ() ಮಾಡು {} ಆಗ(0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
#ವ್ಯಾಖ್ಯಾನಿಸು FIX_ARGP \
FFI_ASSERT(argp <= &ರಾಶಿ[ಬೈಟುಗಳು]); \
ಆದರೆ (argp == &ರಾಶಿ[ಬೈಟುಗಳು]) \
{ \
  argp = ರಾಶಿ; \
  FFI_MIPS_STOP_ಇಲ್ಲಿ(); \
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು FIX_ARGP 
#ಆದರೆ_ಅಂತ್ಯ


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ಸ್ಥಿರ ಶೂನ್ಯ ffi_prep_ವಾದಗಳು(ಅಕ್ಷರ *ರಾಶಿ, 
			  extended_cif *ecif,
			  ಪೂರ್ಣಾಂಕ ಬೈಟುಗಳು,
			  ಪೂರ್ಣಾಂಕ flags)
{
  ಪೂರ್ಣಾಂಕ i;
  ಶೂನ್ಯ **p_argv;
  ಅಕ್ಷರ *argp, *argp_f;
  ffi_type **p_arg;

  memset(ರಾಶಿ, 0, ಬೈಟುಗಳು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
  ಪೂರ್ಣಾಂಕ soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ಆದರೆ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ಆದರೆ (ಬೈಟುಗಳು + 16 > 8 * ಗಾತ್ರ(ffi_arg))
        argp = &ರಾಶಿ[ಬೈಟುಗಳು - (8 * ಗಾತ್ರ(ffi_arg))];
      ಇಲ್ಲದಿದ್ದರೆ
        argp = ರಾಶಿ;
      * (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *) argp = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ecif->rvalue;
      argp += 16;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಆದರೆ (ಬೈಟುಗಳು > 8 * ಗಾತ್ರ(ffi_arg))
        argp = &ರಾಶಿ[ಬೈಟುಗಳು - (8 * ಗಾತ್ರ(ffi_arg))];
      ಇಲ್ಲದಿದ್ದರೆ
        argp = ರಾಶಿ;
    }
#ಇಲ್ಲದಿದ್ದರೆ
  argp = ರಾಶಿ;
#ಆದರೆ_ಅಂತ್ಯ

  argp_f = argp;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
  ಆದರೆ ( ecif->cif->rstruct_flag != 0 )
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#ಆದರೆ_ಅಂತ್ಯ  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += ಗಾತ್ರ(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  ಗಾಗಿ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ಆದರೆ (a < ಗಾತ್ರ(ffi_arg))
        a = ಗಾತ್ರ(ffi_arg);
      
      ಆದರೆ ((a - 1) & (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) argp)
	{
	  argp = (ಅಕ್ಷರ *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->ಗಾತ್ರ_2;
      ಆದರೆ (z <= ಗಾತ್ರ(ffi_arg))
	{
          ಪೂರ್ಣಾಂಕ type = (*p_arg)->type;
	  z = ಗಾತ್ರ(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ಆದರೆ (type == FFI_TYPE_ಸೂಚಕ)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ಆದರೆ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ಆಯ್ಕೆ (type)
	      {
	      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ನಿಲ್ಲಿಸು;
	      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ನಿಲ್ಲಿಸು;
	      ಪೂರ್ವನಿಯೋಜಿತ:
		ನಿಲ್ಲಿಸು;
	      }
	  }
	  ಆಯ್ಕೆ (type)
	    {
	      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ನಿಲ್ಲಿಸು;

	      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ಇಲ್ಲದಿದ್ದರೆ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#ಆದರೆ_ಅಂತ್ಯ
		ನಿಲ್ಲಿಸು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
	      ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ಆದರೆ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ದಶಮಾಂಶ *) argp = *(ದಶಮಾಂಶ *)(* p_argv);
		    argp += z;
		    ಅಕ್ಷರ *tmp = (ಶೂನ್ಯ *) (*p_argv);
		    *(ದಶಮಾಂಶ *) argp = *(ದಶಮಾಂಶ *)(tmp+4);
		  }
		ಇಲ್ಲದಿದ್ದರೆ
		  ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, (*p_arg)->ಗಾತ್ರ_2);
		ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
	      /* This can only happen with 64bit slots.  */
	      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
		*(ದಶಮಾಂಶ *) argp = *(ದಶಮಾಂಶ *)(* p_argv);
		ನಿಲ್ಲಿಸು;

	      /* Handle structures.  */
	      ಪೂರ್ವನಿಯೋಜಿತ:
		ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, (*p_arg)->ಗಾತ್ರ_2);
		ನಿಲ್ಲಿಸು;
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
	  ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, z);
#ಇಲ್ಲದಿದ್ದರೆ
	  {
	    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಕೊನೆ = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) argp + z;
	    ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ cap = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ) ರಾಶಿ + ಬೈಟುಗಳು;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ಆದರೆ (ಕೊನೆ <= cap)
	      ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, z);
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ portion = cap - (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)argp;

		ಸ್ಮರಣೆ_ನಕಲು(argp, *p_argv, portion);
		argp = ರಾಶಿ;
                z -= portion;
		ಸ್ಮರಣೆ_ನಕಲು(argp, (ಶೂನ್ಯ*)((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)(*p_argv) + portion),
                       z);
	      }
	  }
#ಆದರೆ_ಅಂತ್ಯ
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
calc_n32_struct_flags_element(ಚಿಹ್ನೆರಹಿತ *flags, ffi_type *e,
			      ಚಿಹ್ನೆರಹಿತ *loc, ಚಿಹ್ನೆರಹಿತ *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ಆದರೆ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ಆದರೆ (*arg_reg > 7)
	ಹಿಂತಿರುಗಿ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
    }
  *loc += e->ಗಾತ್ರ_2;
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ
calc_n32_struct_flags(ಪೂರ್ಣಾಂಕ soft_float, ffi_type *arg,
		      ಚಿಹ್ನೆರಹಿತ *loc, ಚಿಹ್ನೆರಹಿತ *arg_reg)
{
  ಚಿಹ್ನೆರಹಿತ flags = 0;
  ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ = 0;

  ffi_type *e;

  ಆದರೆ (soft_float)
    ಹಿಂತಿರುಗಿ 0;

  ಆಗ ((e = arg->elements[ಸೂಚ್ಯಂಕ]))
    {
      ಆದರೆ (e->type == FFI_TYPE_COMPLEX)
	{
	  ಆದರೆ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ನಿಲ್ಲಿಸು;
	  ಆದರೆ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ನಿಲ್ಲಿಸು;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ನಿಲ್ಲಿಸು;
      ಸೂಚ್ಯಂಕ++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ಹಿಂತಿರುಗಿ flags;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ
calc_n32_return_struct_flags(ಪೂರ್ಣಾಂಕ soft_float, ffi_type *arg)
{
  ಚಿಹ್ನೆರಹಿತ flags;
  ಚಿಹ್ನೆರಹಿತ small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  ಆದರೆ (arg->ಗಾತ್ರ_2 > 16)
    ಹಿಂತಿರುಗಿ 0;

  ಆದರೆ (arg->ಗಾತ್ರ_2 > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ಆದರೆ (e->type == FFI_TYPE_COMPLEX)
    {
      ಪೂರ್ಣಾಂಕ type = e->elements[0]->type;

      ಆದರೆ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ಹಿಂತಿರುಗಿ small;

      ಆದರೆ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ಹಿಂತಿರುಗಿ small;
	}

      flags = (type << FFI_FLAG_ಬಿಟ್ಟುಗಳು) + type;
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಆದರೆ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ಹಿಂತಿರುಗಿ small;

      flags = e->type;

      ಆದರೆ (arg->elements[1])
	{
	  e = arg->elements[1];
	  ಆದರೆ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ಹಿಂತಿರುಗಿ small;

	  ಆದರೆ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ಹಿಂತಿರುಗಿ small;
	    }

	  flags += e->type << FFI_FLAG_ಬಿಟ್ಟುಗಳು;
	}
    }

  ಆದರೆ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ಹಿಂತಿರುಗಿ flags;
}

#ಆದರೆ_ಅಂತ್ಯ

/* Perform machine dependent cif processing */
ಸ್ಥಿರ ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep_int(ffi_cif *cif, ಚಿಹ್ನೆರಹಿತ nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  ಆದರೆ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ಆದರೆ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ಆಯ್ಕೆ ((cif->arg_types)[0]->type)
	    {
	    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ನಿಲ್ಲಿಸು;
	      
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      ನಿಲ್ಲಿಸು;
	    }

	  ಆದರೆ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ಆದರೆ (cif->flags)
		{
		  ಆಯ್ಕೆ ((cif->arg_types)[1]->type)
		    {
		    ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
		    ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_ಬಿಟ್ಟುಗಳು;
		      ನಿಲ್ಲಿಸು;
		      
		    ಪೂರ್ವನಿಯೋಜಿತ:
		      ನಿಲ್ಲಿಸು;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ಆದರೆ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ಆಯ್ಕೆ (cif->rtype->type)
        {
        ಸ್ಥಿತಿ FFI_TYPE_VOID:
        ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
          ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_SINT64:
        ಸ್ಥಿತಿ FFI_TYPE_UINT64:
        ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
          ನಿಲ್ಲಿಸು;
      
        ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ಪೂರ್ವನಿಯೋಜಿತ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
          ನಿಲ್ಲಿಸು;
        }
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      /* FFI_O32 */      
      ಆಯ್ಕೆ (cif->rtype->type)
        {
        ಸ್ಥಿತಿ FFI_TYPE_VOID:
        ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
        ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
        ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
        ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
	  ಆದರೆ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 4);
          ನಿಲ್ಲಿಸು;

        ಸ್ಥಿತಿ FFI_TYPE_SINT64:
        ಸ್ಥಿತಿ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
          ನಿಲ್ಲಿಸು;
      
        ಪೂರ್ವನಿಯೋಜಿತ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2);
          ನಿಲ್ಲಿಸು;
        }
    }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    ಚಿಹ್ನೆರಹಿತ arg_reg = 0;
    ಚಿಹ್ನೆರಹಿತ loc = 0;
    ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2 = (cif->nargs < 8) ? cif->nargs : 8;
    ಚಿಹ್ನೆರಹಿತ ಸೂಚ್ಯಂಕ = 0;

    ಚಿಹ್ನೆರಹಿತ struct_flags = 0;
    ಪೂರ್ಣಾಂಕ soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ಆದರೆ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ಎಣಿಕೆ_2 = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ಇಲ್ಲದಿದ್ದರೆ
	    cif->rstruct_flag = 0;
      }
    ಇಲ್ಲದಿದ್ದರೆ
      cif->rstruct_flag = 0;

    ಆಗ (ಎಣಿಕೆ_2-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[ಸೂಚ್ಯಂಕ];

	ಆಯ್ಕೆ (t->type)
	  {
	  ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	  ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	    ಆದರೆ (!soft_float && ಸೂಚ್ಯಂಕ < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು);
	    arg_reg++;
	    ನಿಲ್ಲಿಸು;
          ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ಆದರೆ (soft_float || ಸೂಚ್ಯಂಕ >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ಇಲ್ಲದಿದ್ದರೆ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		arg_reg++;
		ಆದರೆ (arg_reg >= 8)
		  ಮುಂದುವರಿ;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		arg_reg++;
	      }
            ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	    ಆಯ್ಕೆ (t->elements[0]->type)
	      {
	      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ಆದರೆ (soft_float || ಸೂಚ್ಯಂಕ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ಇಲ್ಲದಿದ್ದರೆ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		    arg_reg++;
		    ಆದರೆ (arg_reg >= 8)
		        ಮುಂದುವರಿ;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		    arg_reg++;
		    ಆದರೆ (arg_reg >= 8)
		        ಮುಂದುವರಿ;
		  }
		/* passthrough */
	      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->ಬೈಟುಗಳು += 16;
		/* passthrough */
	      ಸ್ಥಿತಿ FFI_TYPE_SINT32:
	      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
		ಆದರೆ (soft_float || ಸೂಚ್ಯಂಕ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ಇಲ್ಲದಿದ್ದರೆ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		    arg_reg++;
		    ಆದರೆ (arg_reg >= 8)
		        ಮುಂದುವರಿ;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_ಬಿಟ್ಟುಗಳು));
		    arg_reg++;
		  }
		ನಿಲ್ಲಿಸು;
	      ಪೂರ್ವನಿಯೋಜಿತ:
		arg_reg += 2;
		ನಿಲ್ಲಿಸು;
	      }
	    ನಿಲ್ಲಿಸು;

	  ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || ಸೂಚ್ಯಂಕ >= nfixedargs,
						t, &loc, &arg_reg);
	    ನಿಲ್ಲಿಸು;

	  ಪೂರ್ವನಿಯೋಜಿತ:
	    arg_reg++;
            ನಿಲ್ಲಿಸು;
	  }

	ಸೂಚ್ಯಂಕ++;
      }

  /* Set the return type flag */
    ಆಯ್ಕೆ (cif->rtype->type)
      {
      ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
	{
	  ಆದರೆ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8));
	    }
	  ನಿಲ್ಲಿಸು;
	}
      
      ಸ್ಥಿತಿ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_ಸೂಚಕ:
	ಆದರೆ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ಇಲ್ಲದಿದ್ದರೆ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
	ಆದರೆ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	    ನಿಲ್ಲಿಸು;
	  }
	/* else fall through */
      ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
	ಆದರೆ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ಇಲ್ಲದಿದ್ದರೆ
	  cif->flags += cif->rtype->type << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ನಿಲ್ಲಿಸು;

      ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ಆದರೆ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
 	  }
	ಇಲ್ಲದಿದ್ದರೆ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_ಬಿಟ್ಟುಗಳು))
					      << (4 + (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8));
	  }
	ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_COMPLEX:
	{
	  ಪೂರ್ಣಾಂಕ type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8));
	  ಆದರೆ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ಆಯ್ಕೆ (type)
		{
		ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
		ಸ್ಥಿತಿ FFI_TYPE_SINT64:
		ಸ್ಥಿತಿ FFI_TYPE_UINT64:
		ಸ್ಥಿತಿ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
		ಪೂರ್ವನಿಯೋಜಿತ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8));
	    }
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8));
	    }
	  ನಿಲ್ಲಿಸು;
	}
      ಸ್ಥಿತಿ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
	cif->flags += cif->rtype->type << (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
	ನಿಲ್ಲಿಸು;
      }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ FFI_OK;
}

ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep(ffi_cif *cif)
{
    ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_ಸ್ಥಿತಿ ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    ಚಿಹ್ನೆರಹಿತ nfixedargs,
                                    ಚಿಹ್ನೆರಹಿತ ntotalargs MAYBE_ಬಳಸದ)
{
    ಹಿಂತಿರುಗಿ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ffi_call_O32(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ), 
			extended_cif *, ಚಿಹ್ನೆರಹಿತ, 
			ಚಿಹ್ನೆರಹಿತ, ಚಿಹ್ನೆರಹಿತ *, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ);

/* Low level routine for calling N32 functions */
ಬಾಹ್ಯ ಪೂರ್ಣಾಂಕ ffi_call_N32(ಶೂನ್ಯ (*)(ಅಕ್ಷರ *, extended_cif *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ), 
			extended_cif *, ಚಿಹ್ನೆರಹಿತ, 
			ಚಿಹ್ನೆರಹಿತ, ಶೂನ್ಯ *, ಶೂನ್ಯ (*)(ಶೂನ್ಯ), ಶೂನ್ಯ *ಆವರಣ);

ಶೂನ್ಯ ffi_call_int(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, 
	      ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  ಆದರೆ ((rvalue == ಖಾಲಿ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ
    ecif.rvalue = rvalue;
    
  ಆಯ್ಕೆ (cif->abi) 
    {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_O32
    ಸ್ಥಿತಿ FFI_O32:
    ಸ್ಥಿತಿ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು, 
		   cif->flags, ecif.rvalue, fn, ಆವರಣ);
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ FFI_MIPS_N32
    ಸ್ಥಿತಿ FFI_N32:
    ಸ್ಥಿತಿ FFI_N32_SOFT_FLOAT:
    ಸ್ಥಿತಿ FFI_N64:
    ಸ್ಥಿತಿ FFI_N64_SOFT_FLOAT:
      {
        ಪೂರ್ಣಾಂಕ copy_rvalue = 0;
	ಪೂರ್ಣಾಂಕ copy_ಸ್ಥಾನಾಂತರ = 0;
        ಅಕ್ಷರ *rvalue_copy = ecif.rvalue;
        ಆದರೆ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->ಗಾತ್ರ_2 < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ಆದರೆ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_ಸ್ಥಾನಾಂತರ = 4;
#ಆದರೆ_ಅಂತ್ಯ
	  }
        ffi_call_N32(ffi_prep_ವಾದಗಳು, &ecif, cif->ಬೈಟುಗಳು,
                     cif->flags, rvalue_copy, fn, ಆವರಣ);
        ಆದರೆ (copy_rvalue)
          ಸ್ಮರಣೆ_ನಕಲು(ecif.rvalue, rvalue_copy + copy_ಸ್ಥಾನಾಂತರ, cif->rtype->ಗಾತ್ರ_2);
      }
      ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ

    ಪೂರ್ವನಿಯೋಜಿತ:
      FFI_ASSERT(0);
      ನಿಲ್ಲಿಸು;
    }
}

ಶೂನ್ಯ
ffi_call(ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಖಾಲಿ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *ಆವರಣ)
{
  ffi_call_int (cif, fn, rvalue, avalue, ಆವರಣ);
}


#ಆದರೆ FFI_CLOSURES
#ಆದರೆ defined(FFI_MIPS_O32)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_O32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_O32(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_N32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_N32(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ /* FFI_MIPS_O32 */

ffi_ಸ್ಥಿತಿ
ffi_prep_ಆವರಣ_loc (ffi_ಆವರಣ *ಆವರಣ,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
		      ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
		      ಶೂನ್ಯ *codeloc)
{
  ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *tramp = (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *) &ಆವರಣ->tramp[0];
  ಶೂನ್ಯ * fn;
  ಅಕ್ಷರ *clear_location = (ಅಕ್ಷರ *) codeloc;

#ಆದರೆ defined(FFI_MIPS_O32)
  ಆದರೆ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  fn = ffi_ಆವರಣ_O32;
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ _MIPS_SIM ==_ABIN32
  ಆದರೆ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
  fn = ffi_ಆವರಣ_N32;
#ಆದರೆ_ಅಂತ್ಯ /* FFI_MIPS_O32 */

#ಆದರೆ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((ಚಿಹ್ನೆರಹಿತ)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((ಚಿಹ್ನೆರಹಿತ)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((ಚಿಹ್ನೆರಹಿತ)codeloc >> 16);
  /* jr   $25          */
#ಆದರೆ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ಇಲ್ಲದಿದ್ದರೆ
  tramp[3] = 0x03200009;
#ಆದರೆ_ಅಂತ್ಯ
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((ಚಿಹ್ನೆರಹಿತ)codeloc & 0xffff);
#ಇಲ್ಲದಿದ್ದರೆ
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)fn  & 0xffff);
  /* jr   $25          */
#ಆದರೆ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ಇಲ್ಲದಿದ್ದರೆ
  tramp[11] = 0x03200009;
#ಆದರೆ_ಅಂತ್ಯ
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)codeloc & 0xffff);

#ಆದರೆ_ಅಂತ್ಯ

  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;
  ಆವರಣ->user_ದತ್ತಾಂಶ = user_ದತ್ತಾಂಶ;

#ಆದರೆ !defined(__FreeBSD__)
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_ಗಾತ್ರ);
#ಇಲ್ಲದಿದ್ದರೆ
  cacheflush (clear_location, FFI_TRAMPOLINE_ಗಾತ್ರ, ICACHE);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* ! __FreeBSD__ */
  ಹಿಂತಿರುಗಿ FFI_OK;
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
ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_mips_inner_O32 (ffi_cif *cif,
                            ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
			    ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			    ಶೂನ್ಯ *rvalue, ffi_arg *ar,
			    ದ್ವಿಗುಣ *fpr)
{
  ಶೂನ್ಯ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಂಕ i, avn, argn, ನೋಡಿದ_int;

  avalue = alloca (cif->nargs * ಗಾತ್ರ (ffi_arg));
  avaluep = alloca (cif->nargs * ಗಾತ್ರ (ffi_arg));

  ನೋಡಿದ_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ಆದರೆ ((cif->flags >> (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (ಶೂನ್ಯ *)(uintptr_t)ar[0];
      argn = 1;
      ನೋಡಿದ_int = 1;
    }
  ಆದರೆ ((cif->flags >> (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಆಗ (i < avn)
    {
      ಆದರೆ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ಆದರೆ (i < 2 && !ನೋಡಿದ_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ಆದರೆ defined(__MIPSEB__) || defined(_MIPSEB)
	  ಆದರೆ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((ಅಕ್ಷರ *) &fpr[i]) + ಗಾತ್ರ (ದಶಮಾಂಶ);
	  ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
	    avaluep[i] = (ಅಕ್ಷರ *) &fpr[i];
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಆಯ್ಕೆ (arg_types[i]->type)
	    {
	      ಸ್ಥಿತಿ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ನಿಲ್ಲಿಸು;

	      ಸ್ಥಿತಿ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ನಿಲ್ಲಿಸು;
		  
	      ಸ್ಥಿತಿ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ನಿಲ್ಲಿಸು;

	      ಪೂರ್ವನಿಯೋಜಿತ:
		avaluep[i] = (ಅಕ್ಷರ *) &ar[argn];
		ನಿಲ್ಲಿಸು;
	    }
	  ನೋಡಿದ_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->ಗಾತ್ರ_2, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_ದತ್ತಾಂಶ);

  ಆದರೆ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ಆಯ್ಕೆ (cif->rtype->type)
        {
        ಸ್ಥಿತಿ FFI_TYPE_FLOAT:
          ಹಿಂತಿರುಗಿ FFI_TYPE_INT;
        ಸ್ಥಿತಿ FFI_TYPE_DOUBLE:
          ಹಿಂತಿರುಗಿ FFI_TYPE_UINT64;
        ಪೂರ್ವನಿಯೋಜಿತ:
          ಹಿಂತಿರುಗಿ cif->rtype->type;
        }
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ಆದರೆ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ ಅಸ್ಥಿರ ("move $v1, %0" : : "r"(cif->rtype->ಗಾತ್ರ_2));
      }
      ಹಿಂತಿರುಗಿ cif->rtype->type;
    }
}

#ಆದರೆ defined(FFI_MIPS_N32)

ಸ್ಥಿರ ಶೂನ್ಯ
copy_struct_N32(ಅಕ್ಷರ *ಗುರಿ_2, ಚಿಹ್ನೆರಹಿತ offset, ffi_abi abi, ffi_type *type,
                ಪೂರ್ಣಾಂಕ argn, ಚಿಹ್ನೆರಹಿತ arg_ಸ್ಥಾನಾಂತರ, ffi_arg *ar,
                ffi_arg *fpr, ಪೂರ್ಣಾಂಕ soft_float)
{
  ffi_type **elt_typep = type->elements;
  ಆಗ(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      ಚಿಹ್ನೆರಹಿತ o;
      ಅಕ್ಷರ *tp;
      ಅಕ್ಷರ *argp;
      ಅಕ್ಷರ *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_ಸ್ಥಾನಾಂತರ += o - offset;
      offset = o;
      argn += arg_ಸ್ಥಾನಾಂತರ / ಗಾತ್ರ(ffi_arg);
      arg_ಸ್ಥಾನಾಂತರ = arg_ಸ್ಥಾನಾಂತರ % ಗಾತ್ರ(ffi_arg);

      argp = (ಅಕ್ಷರ *)(ar + argn);
      fpp = (ಅಕ್ಷರ *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = ಗುರಿ_2 + offset;

      ಆದರೆ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ದ್ವಿಗುಣ *)tp = *(ದ್ವಿಗುಣ *)fpp;
      ಇಲ್ಲದಿದ್ದರೆ
        ಸ್ಮರಣೆ_ನಕಲು(tp, argp + arg_ಸ್ಥಾನಾಂತರ, elt_type->ಗಾತ್ರ_2);

      offset += elt_type->ಗಾತ್ರ_2;
      arg_ಸ್ಥಾನಾಂತರ += elt_type->ಗಾತ್ರ_2;
      elt_typep++;
      argn += arg_ಸ್ಥಾನಾಂತರ / ಗಾತ್ರ(ffi_arg);
      arg_ಸ್ಥಾನಾಂತರ = arg_ಸ್ಥಾನಾಂತರ % ಗಾತ್ರ(ffi_arg);
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
ಪೂರ್ಣಾಂಕ
ffi_ಆವರಣ_mips_inner_N32 (ffi_cif *cif, 
			    ಶೂನ್ಯ (*fun)(ffi_cif*, ಶೂನ್ಯ*, ಶೂನ್ಯ**, ಶೂನ್ಯ*),
                            ಶೂನ್ಯ *user_ದತ್ತಾಂಶ,
			    ಶೂನ್ಯ *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  ಶೂನ್ಯ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ಪೂರ್ಣಾಂಕ i, avn, argn;
  ಪೂರ್ಣಾಂಕ soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * ಗಾತ್ರ (ffi_arg));
  avaluep = alloca (cif->nargs * ಗಾತ್ರ (ffi_arg));

  argn = 0;

  ಆದರೆ (cif->rstruct_flag)
    {
#ಆದರೆ _MIPS_SIM==_ABIN32
      rvalue = (ಶೂನ್ಯ *)(UINT32)ar[0];
#ಇಲ್ಲದಿದ್ದರೆ /* N64 */
      rvalue = (ಶೂನ್ಯ *)ar[0];
#ಆದರೆ_ಅಂತ್ಯ
      argn = 1;
    }
  ಆದರೆ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ಆಗ (i < avn)
    {
      ಆದರೆ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ಆದರೆ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ಆದರೆ defined(__MIPSEB__) || defined(_MIPSEB)
          ಆದರೆ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((ಅಕ್ಷರ *) argp) + ಗಾತ್ರ (ದಶಮಾಂಶ);
          ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
            avaluep[i] = (ಅಕ್ಷರ *) argp;
        }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ಅಕ್ಷರ *) argp;
        }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ಅಕ್ಷರ *) argp;
        }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ಆದರೆ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ಇಲ್ಲದಿದ್ದರೆ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (ಅಕ್ಷರ *) argp;
        }
      ಇಲ್ಲದಿದ್ದರೆ
        {
          ಚಿಹ್ನೆರಹಿತ type = arg_types[i]->type;

          ಆದರೆ (arg_types[i]->alignment > ಗಾತ್ರ(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / ಗಾತ್ರ(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ಆದರೆ (type == FFI_TYPE_ಸೂಚಕ)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ಆದರೆ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ಆಯ್ಕೆ (type)
            {
            ಸ್ಥಿತಿ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ನಿಲ್ಲಿಸು;

            ಸ್ಥಿತಿ FFI_TYPE_STRUCT:
              ಆದರೆ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->ಗಾತ್ರ_2);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ನಿಲ್ಲಿಸು;
                }
              /* Else fall through.  */
            ಪೂರ್ವನಿಯೋಜಿತ:
              avaluep[i] = (ಅಕ್ಷರ *) argp;
              ನಿಲ್ಲಿಸು;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->ಗಾತ್ರ_2, ಗಾತ್ರ(ffi_arg)) / ಗಾತ್ರ(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_ದತ್ತಾಂಶ);

  ಹಿಂತಿರುಗಿ cif->flags >> (FFI_FLAG_ಬಿಟ್ಟುಗಳು * 8);
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_MIPS_N32 */

#ಆದರೆ defined(FFI_MIPS_O32)
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_O32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_O32(ಶೂನ್ಯ);
#ಇಲ್ಲದಿದ್ದರೆ
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_ಆವರಣ_N32(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_ಆವರಣ_N32(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ /* FFI_MIPS_O32 */

ffi_ಸ್ಥಿತಿ
ffi_prep_go_ಆವರಣ (ffi_go_ಆವರಣ* ಆವರಣ, ffi_cif* cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಶೂನ್ಯ * fn;

#ಆದರೆ defined(FFI_MIPS_O32)
  ಆದರೆ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
  fn = ffi_go_ಆವರಣ_O32;
#ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ _MIPS_SIM ==_ABIN32
  ಆದರೆ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಇಲ್ಲದಿದ್ದರೆ
  ಆದರೆ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ಹಿಂತಿರುಗಿ FFI_BAD_ABI;
#ಆದರೆ_ಅಂತ್ಯ
  fn = ffi_go_ಆವರಣ_N32;
#ಆದರೆ_ಅಂತ್ಯ /* FFI_MIPS_O32 */

  ಆವರಣ->tramp = (ಶೂನ್ಯ *)fn;
  ಆವರಣ->cif = cif;
  ಆವರಣ->fun = fun;

  ಹಿಂತಿರುಗಿ FFI_OK;
}

#ಆದರೆ_ಅಂತ್ಯ /* FFI_CLOSURES */

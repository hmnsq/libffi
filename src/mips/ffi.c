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

#അന്തര്ഭാവ <ffi.h>
#അന്തര്ഭാവ <ffi_common.h>

#അന്തര്ഭാവ <stdint.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <stdio.h>

#ifdef __GNUC__
#  എങ്കിൽ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  എങ്കിൽ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  അല്ലെങ്കിൽ
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#അല്ലെങ്കിൽ
# define FFI_MIPS_STOP_HERE() ചെയ്യുക {} ആകുമ്പോൾ(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &അട്ടി[bytes]); \
എങ്കിൽ (argp == &അട്ടി[bytes]) \
{ \
  argp = അട്ടി; \
  FFI_MIPS_STOP_HERE(); \
}
#അല്ലെങ്കിൽ
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

സ്ഥിതം ശൂന്യം ffi_prep_args(അക്ഷരം *അട്ടി, 
			  extended_cif *ecif,
			  പൂർണ്ണസംഖ്യ bytes,
			  പൂർണ്ണസംഖ്യ flags)
{
  പൂർണ്ണസംഖ്യ i;
  ശൂന്യം **p_argv;
  അക്ഷരം *argp, *argp_f;
  ffi_type **p_arg;

  memset(അട്ടി, 0, bytes);

#ifdef FFI_MIPS_N32
  പൂർണ്ണസംഖ്യ soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  എങ്കിൽ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      എങ്കിൽ (bytes + 16 > 8 * വലുപ്പം(ffi_arg))
        argp = &അട്ടി[bytes - (8 * വലുപ്പം(ffi_arg))];
      അല്ലെങ്കിൽ
        argp = അട്ടി;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  അല്ലെങ്കിൽ
    {
      എങ്കിൽ (bytes > 8 * വലുപ്പം(ffi_arg))
        argp = &അട്ടി[bytes - (8 * വലുപ്പം(ffi_arg))];
      അല്ലെങ്കിൽ
        argp = അട്ടി;
    }
#അല്ലെങ്കിൽ
  argp = അട്ടി;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  എങ്കിൽ ( ecif->cif->rstruct_flag != 0 )
#അല്ലെങ്കിൽ
  എങ്കിൽ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += വലുപ്പം(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  വേണ്ടി (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned പൂർണ്ണസംഖ്യ a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      എങ്കിൽ (a < വലുപ്പം(ffi_arg))
        a = വലുപ്പം(ffi_arg);
      
      എങ്കിൽ ((a - 1) & (unsigned long) argp)
	{
	  argp = (അക്ഷരം *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      എങ്കിൽ (z <= വലുപ്പം(ffi_arg))
	{
          പൂർണ്ണസംഖ്യ type = (*p_arg)->type;
	  z = വലുപ്പം(ffi_arg);

          /* The size of a pointer depends on the ABI */
          എങ്കിൽ (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	എങ്കിൽ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    തിരഞ്ഞെടുപ്പ് (type)
	      {
	      അവസ്ഥ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		നിർത്തുക;
	      അവസ്ഥ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		നിർത്തുക;
	      സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	      }
	  }
	  തിരഞ്ഞെടുപ്പ് (type)
	    {
	      അവസ്ഥ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		നിർത്തുക;

	      അവസ്ഥ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#അല്ലെങ്കിൽ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		നിർത്തുക;

#ifdef FFI_MIPS_N32
	      അവസ്ഥ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        എങ്കിൽ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ദശാംശം *) argp = *(ദശാംശം *)(* p_argv);
		    argp += z;
		    അക്ഷരം *tmp = (ശൂന്യം *) (*p_argv);
		    *(ദശാംശം *) argp = *(ദശാംശം *)(tmp+4);
		  }
		അല്ലെങ്കിൽ
		  memcpy(argp, *p_argv, (*p_arg)->size);
		നിർത്തുക;
#endif
	      /* This can only happen with 64bit slots.  */
	      അവസ്ഥ FFI_TYPE_FLOAT:
		*(ദശാംശം *) argp = *(ദശാംശം *)(* p_argv);
		നിർത്തുക;

	      /* Handle structures.  */
	      സ്ഥിരസ്ഥിതി:
		memcpy(argp, *p_argv, (*p_arg)->size);
		നിർത്തുക;
	    }
	}
      അല്ലെങ്കിൽ
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#അല്ലെങ്കിൽ
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) അട്ടി + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    എങ്കിൽ (end <= cap)
	      memcpy(argp, *p_argv, z);
	    അല്ലെങ്കിൽ
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = അട്ടി;
                z -= portion;
		memcpy(argp, (ശൂന്യം*)((unsigned long)(*p_argv) + portion),
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

സ്ഥിതം പൂർണ്ണസംഖ്യ
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  എങ്കിൽ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      എങ്കിൽ (*arg_reg > 7)
	തിരികെ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  തിരികെ 0;
}

സ്ഥിതം unsigned
calc_n32_struct_flags(പൂർണ്ണസംഖ്യ soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  എങ്കിൽ (soft_float)
    തിരികെ 0;

  ആകുമ്പോൾ ((e = arg->elements[index]))
    {
      എങ്കിൽ (e->type == FFI_TYPE_COMPLEX)
	{
	  എങ്കിൽ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    നിർത്തുക;
	  എങ്കിൽ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    നിർത്തുക;
	}
      അല്ലെങ്കിൽ
	എങ്കിൽ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  നിർത്തുക;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  തിരികെ flags;
}

സ്ഥിതം unsigned
calc_n32_return_struct_flags(പൂർണ്ണസംഖ്യ soft_float, ffi_type *arg)
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
  
  എങ്കിൽ (arg->size > 16)
    തിരികെ 0;

  എങ്കിൽ (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  എങ്കിൽ (e->type == FFI_TYPE_COMPLEX)
    {
      പൂർണ്ണസംഖ്യ type = e->elements[0]->type;

      എങ്കിൽ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	തിരികെ small;

      എങ്കിൽ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  തിരികെ small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  അല്ലെങ്കിൽ
    {
      എങ്കിൽ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	തിരികെ small;

      flags = e->type;

      എങ്കിൽ (arg->elements[1])
	{
	  e = arg->elements[1];
	  എങ്കിൽ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    തിരികെ small;

	  എങ്കിൽ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      തിരികെ small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  എങ്കിൽ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  തിരികെ flags;
}

#endif

/* Perform machine dependent cif processing */
സ്ഥിതം ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  എങ്കിൽ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      എങ്കിൽ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  തിരഞ്ഞെടുപ്പ് ((cif->arg_types)[0]->type)
	    {
	    അവസ്ഥ FFI_TYPE_FLOAT:
	    അവസ്ഥ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      നിർത്തുക;
	      
	    സ്ഥിരസ്ഥിതി:
	      നിർത്തുക;
	    }

	  എങ്കിൽ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      എങ്കിൽ (cif->flags)
		{
		  തിരഞ്ഞെടുപ്പ് ((cif->arg_types)[1]->type)
		    {
		    അവസ്ഥ FFI_TYPE_FLOAT:
		    അവസ്ഥ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      നിർത്തുക;
		      
		    സ്ഥിരസ്ഥിതി:
		      നിർത്തുക;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  എങ്കിൽ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
        {
        അവസ്ഥ FFI_TYPE_VOID:
        അവസ്ഥ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          നിർത്തുക;

        അവസ്ഥ FFI_TYPE_SINT64:
        അവസ്ഥ FFI_TYPE_UINT64:
        അവസ്ഥ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          നിർത്തുക;
      
        അവസ്ഥ FFI_TYPE_FLOAT:
        സ്ഥിരസ്ഥിതി:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          നിർത്തുക;
        }
    }
  അല്ലെങ്കിൽ
    {
      /* FFI_O32 */      
      തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
        {
        അവസ്ഥ FFI_TYPE_VOID:
        അവസ്ഥ FFI_TYPE_STRUCT:
        അവസ്ഥ FFI_TYPE_FLOAT:
        അവസ്ഥ FFI_TYPE_DOUBLE:
        അവസ്ഥ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  എങ്കിൽ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          നിർത്തുക;

        അവസ്ഥ FFI_TYPE_SINT64:
        അവസ്ഥ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          നിർത്തുക;
      
        സ്ഥിരസ്ഥിതി:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          നിർത്തുക;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned എണ്ണം = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    പൂർണ്ണസംഖ്യ soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	എങ്കിൽ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    എണ്ണം = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	അല്ലെങ്കിൽ
	    cif->rstruct_flag = 0;
      }
    അല്ലെങ്കിൽ
      cif->rstruct_flag = 0;

    ആകുമ്പോൾ (എണ്ണം-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	തിരഞ്ഞെടുപ്പ് (t->type)
	  {
	  അവസ്ഥ FFI_TYPE_FLOAT:
	  അവസ്ഥ FFI_TYPE_DOUBLE:
	    എങ്കിൽ (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    നിർത്തുക;
          അവസ്ഥ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    എങ്കിൽ (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    അല്ലെങ്കിൽ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		എങ്കിൽ (arg_reg >= 8)
		  തുടരുക;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_COMPLEX:
	    തിരഞ്ഞെടുപ്പ് (t->elements[0]->type)
	      {
	      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		എങ്കിൽ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		അല്ലെങ്കിൽ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    എങ്കിൽ (arg_reg >= 8)
		        തുടരുക;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    എങ്കിൽ (arg_reg >= 8)
		        തുടരുക;
		  }
		/* passthrough */
	      അവസ്ഥ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      അവസ്ഥ FFI_TYPE_SINT32:
	      അവസ്ഥ FFI_TYPE_UINT32:
	      അവസ്ഥ FFI_TYPE_DOUBLE:
		എങ്കിൽ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		അല്ലെങ്കിൽ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    എങ്കിൽ (arg_reg >= 8)
		        തുടരുക;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		നിർത്തുക;
	      സ്ഥിരസ്ഥിതി:
		arg_reg += 2;
		നിർത്തുക;
	      }
	    നിർത്തുക;

	  അവസ്ഥ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    നിർത്തുക;

	  സ്ഥിരസ്ഥിതി:
	    arg_reg++;
            നിർത്തുക;
	  }

	index++;
      }

  /* Set the return type flag */
    തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
      {
      അവസ്ഥ FFI_TYPE_STRUCT:
	{
	  എങ്കിൽ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  അല്ലെങ്കിൽ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  നിർത്തുക;
	}
      
      അവസ്ഥ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_POINTER:
	എങ്കിൽ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	അല്ലെങ്കിൽ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_FLOAT:
	എങ്കിൽ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    നിർത്തുക;
	  }
	/* else fall through */
      അവസ്ഥ FFI_TYPE_DOUBLE:
	എങ്കിൽ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	അല്ലെങ്കിൽ
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	നിർത്തുക;

      അവസ്ഥ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	എങ്കിൽ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	അല്ലെങ്കിൽ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	നിർത്തുക;
      അവസ്ഥ FFI_TYPE_COMPLEX:
	{
	  പൂർണ്ണസംഖ്യ type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  എങ്കിൽ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      തിരഞ്ഞെടുപ്പ് (type)
		{
		അവസ്ഥ FFI_TYPE_DOUBLE:
		അവസ്ഥ FFI_TYPE_SINT64:
		അവസ്ഥ FFI_TYPE_UINT64:
		അവസ്ഥ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  നിർത്തുക;
		അവസ്ഥ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  നിർത്തുക;
		അവസ്ഥ FFI_TYPE_FLOAT:
		സ്ഥിരസ്ഥിതി:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  അല്ലെങ്കിൽ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  നിർത്തുക;
	}
      അവസ്ഥ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	നിർത്തുക;
      അവസ്ഥ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	നിർത്തുക;
      }
  }
#endif
  തിരികെ FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    തിരികെ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    തിരികെ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
ബാഹ്യ പൂർണ്ണസംഖ്യ ffi_call_O32(ശൂന്യം (*)(അക്ഷരം *, extended_cif *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, ശൂന്യം (*)(ശൂന്യം), ശൂന്യം *closure);

/* Low level routine for calling N32 functions */
ബാഹ്യ പൂർണ്ണസംഖ്യ ffi_call_N32(ശൂന്യം (*)(അക്ഷരം *, extended_cif *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ), 
			extended_cif *, unsigned, 
			unsigned, ശൂന്യം *, ശൂന്യം (*)(ശൂന്യം), ശൂന്യം *closure);

ശൂന്യം ffi_call_int(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, 
	      ശൂന്യം **avalue, ശൂന്യം *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  എങ്കിൽ ((rvalue == ശൂന്യ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  അല്ലെങ്കിൽ
    ecif.rvalue = rvalue;
    
  തിരഞ്ഞെടുപ്പ് (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    അവസ്ഥ FFI_O32:
    അവസ്ഥ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      നിർത്തുക;
#endif

#ifdef FFI_MIPS_N32
    അവസ്ഥ FFI_N32:
    അവസ്ഥ FFI_N32_SOFT_FLOAT:
    അവസ്ഥ FFI_N64:
    അവസ്ഥ FFI_N64_SOFT_FLOAT:
      {
        പൂർണ്ണസംഖ്യ copy_rvalue = 0;
	പൂർണ്ണസംഖ്യ copy_offset = 0;
        അക്ഷരം *rvalue_copy = ecif.rvalue;
        എങ്കിൽ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	അല്ലെങ്കിൽ എങ്കിൽ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#എങ്കിൽ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        എങ്കിൽ (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      നിർത്തുക;
#endif

    സ്ഥിരസ്ഥിതി:
      FFI_ASSERT(0);
      നിർത്തുക;
    }
}

ശൂന്യം
ffi_call(ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue, ശൂന്യം **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ശൂന്യ);
}

ശൂന്യം
ffi_call_go (ffi_cif *cif, ശൂന്യം (*fn)(ശൂന്യം), ശൂന്യം *rvalue,
	     ശൂന്യം **avalue, ശൂന്യം *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#എങ്കിൽ FFI_CLOSURES
#എങ്കിൽ defined(FFI_MIPS_O32)
ബാഹ്യ ശൂന്യം ffi_closure_O32(ശൂന്യം);
ബാഹ്യ ശൂന്യം ffi_go_closure_O32(ശൂന്യം);
#അല്ലെങ്കിൽ
ബാഹ്യ ശൂന്യം ffi_closure_N32(ശൂന്യം);
ബാഹ്യ ശൂന്യം ffi_go_closure_N32(ശൂന്യം);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*),
		      ശൂന്യം *user_data,
		      ശൂന്യം *codeloc)
{
  unsigned പൂർണ്ണസംഖ്യ *tramp = (unsigned പൂർണ്ണസംഖ്യ *) &closure->tramp[0];
  ശൂന്യം * fn;
  അക്ഷരം *clear_location = (അക്ഷരം *) codeloc;

#എങ്കിൽ defined(FFI_MIPS_O32)
  എങ്കിൽ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
  fn = ffi_closure_O32;
#അല്ലെങ്കിൽ
#എങ്കിൽ _MIPS_SIM ==_ABIN32
  എങ്കിൽ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
#അല്ലെങ്കിൽ
  എങ്കിൽ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#എങ്കിൽ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#എങ്കിൽ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#അല്ലെങ്കിൽ
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#അല്ലെങ്കിൽ
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
#എങ്കിൽ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#അല്ലെങ്കിൽ
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#എങ്കിൽ !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#അല്ലെങ്കിൽ
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  തിരികെ FFI_OK;
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
പൂർണ്ണസംഖ്യ
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
			    ശൂന്യം *user_data,
			    ശൂന്യം *rvalue, ffi_arg *ar,
			    ഇരട്ട *fpr)
{
  ശൂന്യം **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  പൂർണ്ണസംഖ്യ i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * വലുപ്പം (ffi_arg));
  avaluep = alloca (cif->nargs * വലുപ്പം (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  എങ്കിൽ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (ശൂന്യം *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  എങ്കിൽ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ആകുമ്പോൾ (i < avn)
    {
      എങ്കിൽ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      എങ്കിൽ (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#എങ്കിൽ defined(__MIPSEB__) || defined(_MIPSEB)
	  എങ്കിൽ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((അക്ഷരം *) &fpr[i]) + വലുപ്പം (ദശാംശം);
	  അല്ലെങ്കിൽ
#endif
	    avaluep[i] = (അക്ഷരം *) &fpr[i];
	}
      അല്ലെങ്കിൽ
	{
	  തിരഞ്ഞെടുപ്പ് (arg_types[i]->type)
	    {
	      അവസ്ഥ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		നിർത്തുക;

	      അവസ്ഥ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		നിർത്തുക;
		  
	      അവസ്ഥ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		നിർത്തുക;

	      സ്ഥിരസ്ഥിതി:
		avaluep[i] = (അക്ഷരം *) &ar[argn];
		നിർത്തുക;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  എങ്കിൽ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      തിരഞ്ഞെടുപ്പ് (cif->rtype->type)
        {
        അവസ്ഥ FFI_TYPE_FLOAT:
          തിരികെ FFI_TYPE_INT;
        അവസ്ഥ FFI_TYPE_DOUBLE:
          തിരികെ FFI_TYPE_UINT64;
        സ്ഥിരസ്ഥിതി:
          തിരികെ cif->rtype->type;
        }
    }
  അല്ലെങ്കിൽ
    {
      എങ്കിൽ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ അസ്ഥിര ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      തിരികെ cif->rtype->type;
    }
}

#എങ്കിൽ defined(FFI_MIPS_N32)

സ്ഥിതം ശൂന്യം
copy_struct_N32(അക്ഷരം *target, unsigned offset, ffi_abi abi, ffi_type *type,
                പൂർണ്ണസംഖ്യ argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, പൂർണ്ണസംഖ്യ soft_float)
{
  ffi_type **elt_typep = type->elements;
  ആകുമ്പോൾ(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      അക്ഷരം *tp;
      അക്ഷരം *argp;
      അക്ഷരം *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / വലുപ്പം(ffi_arg);
      arg_offset = arg_offset % വലുപ്പം(ffi_arg);

      argp = (അക്ഷരം *)(ar + argn);
      fpp = (അക്ഷരം *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      എങ്കിൽ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ഇരട്ട *)tp = *(ഇരട്ട *)fpp;
      അല്ലെങ്കിൽ
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / വലുപ്പം(ffi_arg);
      arg_offset = arg_offset % വലുപ്പം(ffi_arg);
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
പൂർണ്ണസംഖ്യ
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    ശൂന്യം (*fun)(ffi_cif*, ശൂന്യം*, ശൂന്യം**, ശൂന്യം*),
                            ശൂന്യം *user_data,
			    ശൂന്യം *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  ശൂന്യം **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  പൂർണ്ണസംഖ്യ i, avn, argn;
  പൂർണ്ണസംഖ്യ soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * വലുപ്പം (ffi_arg));
  avaluep = alloca (cif->nargs * വലുപ്പം (ffi_arg));

  argn = 0;

  എങ്കിൽ (cif->rstruct_flag)
    {
#എങ്കിൽ _MIPS_SIM==_ABIN32
      rvalue = (ശൂന്യം *)(UINT32)ar[0];
#അല്ലെങ്കിൽ /* N64 */
      rvalue = (ശൂന്യം *)ar[0];
#endif
      argn = 1;
    }
  എങ്കിൽ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ആകുമ്പോൾ (i < avn)
    {
      എങ്കിൽ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          എങ്കിൽ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#എങ്കിൽ defined(__MIPSEB__) || defined(_MIPSEB)
          എങ്കിൽ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((അക്ഷരം *) argp) + വലുപ്പം (ദശാംശം);
          അല്ലെങ്കിൽ
#endif
            avaluep[i] = (അക്ഷരം *) argp;
        }
      അല്ലെങ്കിൽ എങ്കിൽ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (അക്ഷരം *) argp;
        }
      അല്ലെങ്കിൽ എങ്കിൽ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (അക്ഷരം *) argp;
        }
      അല്ലെങ്കിൽ എങ്കിൽ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          എങ്കിൽ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  അല്ലെങ്കിൽ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (അക്ഷരം *) argp;
        }
      അല്ലെങ്കിൽ
        {
          unsigned type = arg_types[i]->type;

          എങ്കിൽ (arg_types[i]->alignment > വലുപ്പം(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / വലുപ്പം(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          എങ്കിൽ (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  എങ്കിൽ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          തിരഞ്ഞെടുപ്പ് (type)
            {
            അവസ്ഥ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              നിർത്തുക;

            അവസ്ഥ FFI_TYPE_STRUCT:
              എങ്കിൽ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  നിർത്തുക;
                }
              /* Else fall through.  */
            സ്ഥിരസ്ഥിതി:
              avaluep[i] = (അക്ഷരം *) argp;
              നിർത്തുക;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, വലുപ്പം(ffi_arg)) / വലുപ്പം(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  തിരികെ cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#എങ്കിൽ defined(FFI_MIPS_O32)
ബാഹ്യ ശൂന്യം ffi_closure_O32(ശൂന്യം);
ബാഹ്യ ശൂന്യം ffi_go_closure_O32(ശൂന്യം);
#അല്ലെങ്കിൽ
ബാഹ്യ ശൂന്യം ffi_closure_N32(ശൂന്യം);
ബാഹ്യ ശൂന്യം ffi_go_closure_N32(ശൂന്യം);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ശൂന്യം (*fun)(ffi_cif*,ശൂന്യം*,ശൂന്യം**,ശൂന്യം*))
{
  ശൂന്യം * fn;

#എങ്കിൽ defined(FFI_MIPS_O32)
  എങ്കിൽ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#അല്ലെങ്കിൽ
#എങ്കിൽ _MIPS_SIM ==_ABIN32
  എങ്കിൽ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
#അല്ലെങ്കിൽ
  എങ്കിൽ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    തിരികെ FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (ശൂന്യം *)fn;
  closure->cif = cif;
  closure->fun = fun;

  തിരികെ FFI_OK;
}

#endif /* FFI_CLOSURES */

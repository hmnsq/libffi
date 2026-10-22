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

#અન્તર્ભાવ <ffi.h>
#અન્તર્ભાવ <ffi_common.h>

#અન્તર્ભાવ <stdint.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <stdio.h>

#ifdef __GNUC__
#  જો (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  જો defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  નહીંતર
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#નહીંતર
# define FFI_MIPS_STOP_HERE() કરો {} જ્યારે(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &ઢગલો[bytes]); \
જો (argp == &ઢગલો[bytes]) \
{ \
  argp = ઢગલો; \
  FFI_MIPS_STOP_HERE(); \
}
#નહીંતર
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

સ્થિર શૂન્ય ffi_prep_args(અક્ષર *ઢગલો, 
			  extended_cif *ecif,
			  પૂર્ણાંક bytes,
			  પૂર્ણાંક flags)
{
  પૂર્ણાંક i;
  શૂન્ય **p_argv;
  અક્ષર *argp, *argp_f;
  ffi_type **p_arg;

  memset(ઢગલો, 0, bytes);

#ifdef FFI_MIPS_N32
  પૂર્ણાંક soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  જો (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      જો (bytes + 16 > 8 * કદ(ffi_arg))
        argp = &ઢગલો[bytes - (8 * કદ(ffi_arg))];
      નહીંતર
        argp = ઢગલો;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  નહીંતર
    {
      જો (bytes > 8 * કદ(ffi_arg))
        argp = &ઢગલો[bytes - (8 * કદ(ffi_arg))];
      નહીંતર
        argp = ઢગલો;
    }
#નહીંતર
  argp = ઢગલો;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  જો ( ecif->cif->rstruct_flag != 0 )
#નહીંતર
  જો ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += કદ(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  માટે (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned પૂર્ણાંક a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      જો (a < કદ(ffi_arg))
        a = કદ(ffi_arg);
      
      જો ((a - 1) & (unsigned long) argp)
	{
	  argp = (અક્ષર *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      જો (z <= કદ(ffi_arg))
	{
          પૂર્ણાંક type = (*p_arg)->type;
	  z = કદ(ffi_arg);

          /* The size of a pointer depends on the ABI */
          જો (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	જો (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    પસંદગી (type)
	      {
	      સ્થિતિ FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		તોડો;
	      સ્થિતિ FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		તોડો;
	      મૂળભૂત:
		તોડો;
	      }
	  }
	  પસંદગી (type)
	    {
	      સ્થિતિ FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		તોડો;

	      સ્થિતિ FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#નહીંતર
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		તોડો;

#ifdef FFI_MIPS_N32
	      સ્થિતિ FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        જો(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(દશાંશ *) argp = *(દશાંશ *)(* p_argv);
		    argp += z;
		    અક્ષર *tmp = (શૂન્ય *) (*p_argv);
		    *(દશાંશ *) argp = *(દશાંશ *)(tmp+4);
		  }
		નહીંતર
		  memcpy(argp, *p_argv, (*p_arg)->size);
		તોડો;
#endif
	      /* This can only happen with 64bit slots.  */
	      સ્થિતિ FFI_TYPE_FLOAT:
		*(દશાંશ *) argp = *(દશાંશ *)(* p_argv);
		તોડો;

	      /* Handle structures.  */
	      મૂળભૂત:
		memcpy(argp, *p_argv, (*p_arg)->size);
		તોડો;
	    }
	}
      નહીંતર
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#નહીંતર
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) ઢગલો + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    જો (end <= cap)
	      memcpy(argp, *p_argv, z);
	    નહીંતર
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = ઢગલો;
                z -= portion;
		memcpy(argp, (શૂન્ય*)((unsigned long)(*p_argv) + portion),
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

સ્થિર પૂર્ણાંક
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  જો (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      જો (*arg_reg > 7)
	પાછા 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  પાછા 0;
}

સ્થિર unsigned
calc_n32_struct_flags(પૂર્ણાંક soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  જો (soft_float)
    પાછા 0;

  જ્યારે ((e = arg->elements[index]))
    {
      જો (e->type == FFI_TYPE_COMPLEX)
	{
	  જો (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    તોડો;
	  જો (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    તોડો;
	}
      નહીંતર
	જો (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  તોડો;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  પાછા flags;
}

સ્થિર unsigned
calc_n32_return_struct_flags(પૂર્ણાંક soft_float, ffi_type *arg)
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
  
  જો (arg->size > 16)
    પાછા 0;

  જો (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  જો (e->type == FFI_TYPE_COMPLEX)
    {
      પૂર્ણાંક type = e->elements[0]->type;

      જો (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	પાછા small;

      જો (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  પાછા small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  નહીંતર
    {
      જો (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	પાછા small;

      flags = e->type;

      જો (arg->elements[1])
	{
	  e = arg->elements[1];
	  જો (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    પાછા small;

	  જો (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      પાછા small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  જો (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  પાછા flags;
}

#endif

/* Perform machine dependent cif processing */
સ્થિર ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  જો (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      જો (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  પસંદગી ((cif->arg_types)[0]->type)
	    {
	    સ્થિતિ FFI_TYPE_FLOAT:
	    સ્થિતિ FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      તોડો;
	      
	    મૂળભૂત:
	      તોડો;
	    }

	  જો (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      જો (cif->flags)
		{
		  પસંદગી ((cif->arg_types)[1]->type)
		    {
		    સ્થિતિ FFI_TYPE_FLOAT:
		    સ્થિતિ FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      તોડો;
		      
		    મૂળભૂત:
		      તોડો;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  જો (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      પસંદગી (cif->rtype->type)
        {
        સ્થિતિ FFI_TYPE_VOID:
        સ્થિતિ FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          તોડો;

        સ્થિતિ FFI_TYPE_SINT64:
        સ્થિતિ FFI_TYPE_UINT64:
        સ્થિતિ FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          તોડો;
      
        સ્થિતિ FFI_TYPE_FLOAT:
        મૂળભૂત:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          તોડો;
        }
    }
  નહીંતર
    {
      /* FFI_O32 */      
      પસંદગી (cif->rtype->type)
        {
        સ્થિતિ FFI_TYPE_VOID:
        સ્થિતિ FFI_TYPE_STRUCT:
        સ્થિતિ FFI_TYPE_FLOAT:
        સ્થિતિ FFI_TYPE_DOUBLE:
        સ્થિતિ FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  જો (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          તોડો;

        સ્થિતિ FFI_TYPE_SINT64:
        સ્થિતિ FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          તોડો;
      
        મૂળભૂત:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          તોડો;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned ગણતરી = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    પૂર્ણાંક soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    જો (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	જો (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ગણતરી = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	નહીંતર
	    cif->rstruct_flag = 0;
      }
    નહીંતર
      cif->rstruct_flag = 0;

    જ્યારે (ગણતરી-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	પસંદગી (t->type)
	  {
	  સ્થિતિ FFI_TYPE_FLOAT:
	  સ્થિતિ FFI_TYPE_DOUBLE:
	    જો (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    તોડો;
          સ્થિતિ FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    જો (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    નહીંતર
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		જો (arg_reg >= 8)
		  ચાલુ;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            તોડો;

	  સ્થિતિ FFI_TYPE_COMPLEX:
	    પસંદગી (t->elements[0]->type)
	      {
	      સ્થિતિ FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		જો (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		નહીંતર
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    જો (arg_reg >= 8)
		        ચાલુ;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    જો (arg_reg >= 8)
		        ચાલુ;
		  }
		/* passthrough */
	      સ્થિતિ FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      સ્થિતિ FFI_TYPE_SINT32:
	      સ્થિતિ FFI_TYPE_UINT32:
	      સ્થિતિ FFI_TYPE_DOUBLE:
		જો (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		નહીંતર
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    જો (arg_reg >= 8)
		        ચાલુ;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		તોડો;
	      મૂળભૂત:
		arg_reg += 2;
		તોડો;
	      }
	    તોડો;

	  સ્થિતિ FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    તોડો;

	  મૂળભૂત:
	    arg_reg++;
            તોડો;
	  }

	index++;
      }

  /* Set the return type flag */
    પસંદગી (cif->rtype->type)
      {
      સ્થિતિ FFI_TYPE_STRUCT:
	{
	  જો (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  નહીંતર
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  તોડો;
	}
      
      સ્થિતિ FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	તોડો;

      સ્થિતિ FFI_TYPE_POINTER:
	જો (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	નહીંતર
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	તોડો;

      સ્થિતિ FFI_TYPE_FLOAT:
	જો (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    તોડો;
	  }
	/* else fall through */
      સ્થિતિ FFI_TYPE_DOUBLE:
	જો (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	નહીંતર
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	તોડો;

      સ્થિતિ FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	જો (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	નહીંતર
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	તોડો;
      સ્થિતિ FFI_TYPE_COMPLEX:
	{
	  પૂર્ણાંક type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  જો (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      પસંદગી (type)
		{
		સ્થિતિ FFI_TYPE_DOUBLE:
		સ્થિતિ FFI_TYPE_SINT64:
		સ્થિતિ FFI_TYPE_UINT64:
		સ્થિતિ FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  તોડો;
		સ્થિતિ FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  તોડો;
		સ્થિતિ FFI_TYPE_FLOAT:
		મૂળભૂત:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  નહીંતર
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  તોડો;
	}
      સ્થિતિ FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	તોડો;
      સ્થિતિ FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	તોડો;
      મૂળભૂત:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	તોડો;
      }
  }
#endif
  પાછા FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    પાછા ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    પાછા ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
બાહ્ય પૂર્ણાંક ffi_call_O32(શૂન્ય (*)(અક્ષર *, extended_cif *, પૂર્ણાંક, પૂર્ણાંક), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, શૂન્ય (*)(શૂન્ય), શૂન્ય *closure);

/* Low level routine for calling N32 functions */
બાહ્ય પૂર્ણાંક ffi_call_N32(શૂન્ય (*)(અક્ષર *, extended_cif *, પૂર્ણાંક, પૂર્ણાંક), 
			extended_cif *, unsigned, 
			unsigned, શૂન્ય *, શૂન્ય (*)(શૂન્ય), શૂન્ય *closure);

શૂન્ય ffi_call_int(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, 
	      શૂન્ય **avalue, શૂન્ય *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  જો ((rvalue == ખાલી) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  નહીંતર
    ecif.rvalue = rvalue;
    
  પસંદગી (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    સ્થિતિ FFI_O32:
    સ્થિતિ FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      તોડો;
#endif

#ifdef FFI_MIPS_N32
    સ્થિતિ FFI_N32:
    સ્થિતિ FFI_N32_SOFT_FLOAT:
    સ્થિતિ FFI_N64:
    સ્થિતિ FFI_N64_SOFT_FLOAT:
      {
        પૂર્ણાંક copy_rvalue = 0;
	પૂર્ણાંક copy_offset = 0;
        અક્ષર *rvalue_copy = ecif.rvalue;
        જો (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	નહીંતર જો (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#જો defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        જો (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      તોડો;
#endif

    મૂળભૂત:
      FFI_ASSERT(0);
      તોડો;
    }
}

શૂન્ય
ffi_call(ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue, શૂન્ય **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ખાલી);
}

શૂન્ય
ffi_call_go (ffi_cif *cif, શૂન્ય (*fn)(શૂન્ય), શૂન્ય *rvalue,
	     શૂન્ય **avalue, શૂન્ય *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#જો FFI_CLOSURES
#જો defined(FFI_MIPS_O32)
બાહ્ય શૂન્ય ffi_closure_O32(શૂન્ય);
બાહ્ય શૂન્ય ffi_go_closure_O32(શૂન્ય);
#નહીંતર
બાહ્ય શૂન્ય ffi_closure_N32(શૂન્ય);
બાહ્ય શૂન્ય ffi_go_closure_N32(શૂન્ય);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*),
		      શૂન્ય *user_data,
		      શૂન્ય *codeloc)
{
  unsigned પૂર્ણાંક *tramp = (unsigned પૂર્ણાંક *) &closure->tramp[0];
  શૂન્ય * fn;
  અક્ષર *clear_location = (અક્ષર *) codeloc;

#જો defined(FFI_MIPS_O32)
  જો (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
  fn = ffi_closure_O32;
#નહીંતર
#જો _MIPS_SIM ==_ABIN32
  જો (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
#નહીંતર
  જો (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#જો defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#જો !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#નહીંતર
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#નહીંતર
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
#જો !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#નહીંતર
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#જો !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#નહીંતર
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  પાછા FFI_OK;
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
પૂર્ણાંક
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
			    શૂન્ય *user_data,
			    શૂન્ય *rvalue, ffi_arg *ar,
			    દ્વિગુણ *fpr)
{
  શૂન્ય **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  પૂર્ણાંક i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * કદ (ffi_arg));
  avaluep = alloca (cif->nargs * કદ (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  જો ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (શૂન્ય *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  જો ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  જ્યારે (i < avn)
    {
      જો (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      જો (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#જો defined(__MIPSEB__) || defined(_MIPSEB)
	  જો (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((અક્ષર *) &fpr[i]) + કદ (દશાંશ);
	  નહીંતર
#endif
	    avaluep[i] = (અક્ષર *) &fpr[i];
	}
      નહીંતર
	{
	  પસંદગી (arg_types[i]->type)
	    {
	      સ્થિતિ FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		તોડો;

	      સ્થિતિ FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		તોડો;
		  
	      સ્થિતિ FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		તોડો;

	      મૂળભૂત:
		avaluep[i] = (અક્ષર *) &ar[argn];
		તોડો;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  જો (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      પસંદગી (cif->rtype->type)
        {
        સ્થિતિ FFI_TYPE_FLOAT:
          પાછા FFI_TYPE_INT;
        સ્થિતિ FFI_TYPE_DOUBLE:
          પાછા FFI_TYPE_UINT64;
        મૂળભૂત:
          પાછા cif->rtype->type;
        }
    }
  નહીંતર
    {
      જો (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ અસ્થિર ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      પાછા cif->rtype->type;
    }
}

#જો defined(FFI_MIPS_N32)

સ્થિર શૂન્ય
copy_struct_N32(અક્ષર *target, unsigned offset, ffi_abi abi, ffi_type *type,
                પૂર્ણાંક argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, પૂર્ણાંક soft_float)
{
  ffi_type **elt_typep = type->elements;
  જ્યારે(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      અક્ષર *tp;
      અક્ષર *argp;
      અક્ષર *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / કદ(ffi_arg);
      arg_offset = arg_offset % કદ(ffi_arg);

      argp = (અક્ષર *)(ar + argn);
      fpp = (અક્ષર *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      જો (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(દ્વિગુણ *)tp = *(દ્વિગુણ *)fpp;
      નહીંતર
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / કદ(ffi_arg);
      arg_offset = arg_offset % કદ(ffi_arg);
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
પૂર્ણાંક
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    શૂન્ય (*fun)(ffi_cif*, શૂન્ય*, શૂન્ય**, શૂન્ય*),
                            શૂન્ય *user_data,
			    શૂન્ય *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  શૂન્ય **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  પૂર્ણાંક i, avn, argn;
  પૂર્ણાંક soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * કદ (ffi_arg));
  avaluep = alloca (cif->nargs * કદ (ffi_arg));

  argn = 0;

  જો (cif->rstruct_flag)
    {
#જો _MIPS_SIM==_ABIN32
      rvalue = (શૂન્ય *)(UINT32)ar[0];
#નહીંતર /* N64 */
      rvalue = (શૂન્ય *)ar[0];
#endif
      argn = 1;
    }
  જો (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  જ્યારે (i < avn)
    {
      જો (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          જો ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#જો defined(__MIPSEB__) || defined(_MIPSEB)
          જો (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((અક્ષર *) argp) + કદ (દશાંશ);
          નહીંતર
#endif
            avaluep[i] = (અક્ષર *) argp;
        }
      નહીંતર જો (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (અક્ષર *) argp;
        }
      નહીંતર જો (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (અક્ષર *) argp;
        }
      નહીંતર જો (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          જો (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  નહીંતર
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (અક્ષર *) argp;
        }
      નહીંતર
        {
          unsigned type = arg_types[i]->type;

          જો (arg_types[i]->alignment > કદ(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / કદ(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          જો (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  જો (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          પસંદગી (type)
            {
            સ્થિતિ FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              તોડો;

            સ્થિતિ FFI_TYPE_STRUCT:
              જો (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  તોડો;
                }
              /* Else fall through.  */
            મૂળભૂત:
              avaluep[i] = (અક્ષર *) argp;
              તોડો;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, કદ(ffi_arg)) / કદ(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  પાછા cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#જો defined(FFI_MIPS_O32)
બાહ્ય શૂન્ય ffi_closure_O32(શૂન્ય);
બાહ્ય શૂન્ય ffi_go_closure_O32(શૂન્ય);
#નહીંતર
બાહ્ય શૂન્ય ffi_closure_N32(શૂન્ય);
બાહ્ય શૂન્ય ffi_go_closure_N32(શૂન્ય);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     શૂન્ય (*fun)(ffi_cif*,શૂન્ય*,શૂન્ય**,શૂન્ય*))
{
  શૂન્ય * fn;

#જો defined(FFI_MIPS_O32)
  જો (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#નહીંતર
#જો _MIPS_SIM ==_ABIN32
  જો (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
#નહીંતર
  જો (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    પાછા FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (શૂન્ય *)fn;
  closure->cif = cif;
  closure->fun = fun;

  પાછા FFI_OK;
}

#endif /* FFI_CLOSURES */

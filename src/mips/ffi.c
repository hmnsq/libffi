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

#include <ffi.h>
#include <ffi_common.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __GNUC__
#  nếu (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  nếu defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  không_thì
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#không_thì
# define FFI_MIPS_STOP_HERE() do {} trong_khi(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &stack[bytes]); \
nếu (argp == &stack[bytes]) \
{ \
  argp = stack; \
  FFI_MIPS_STOP_HERE(); \
}
#không_thì
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

static void ffi_prep_args(char *stack, 
			  extended_cif *ecif,
			  số_nguyên bytes,
			  số_nguyên flags)
{
  số_nguyên i;
  void **p_argv;
  char *argp, *argp_f;
  ffi_type **p_arg;

  memset(stack, 0, bytes);

#ifdef FFI_MIPS_N32
  số_nguyên soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  nếu (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      nếu (bytes + 16 > 8 * sizeof(ffi_arg))
        argp = &stack[bytes - (8 * sizeof(ffi_arg))];
      không_thì
        argp = stack;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  không_thì
    {
      nếu (bytes > 8 * sizeof(ffi_arg))
        argp = &stack[bytes - (8 * sizeof(ffi_arg))];
      không_thì
        argp = stack;
    }
#không_thì
  argp = stack;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  nếu ( ecif->cif->rstruct_flag != 0 )
#không_thì
  nếu ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += sizeof(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  cho (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned số_nguyên a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      nếu (a < sizeof(ffi_arg))
        a = sizeof(ffi_arg);
      
      nếu ((a - 1) & (unsigned long) argp)
	{
	  argp = (char *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      nếu (z <= sizeof(ffi_arg))
	{
          số_nguyên type = (*p_arg)->type;
	  z = sizeof(ffi_arg);

          /* The size of a pointer depends on the ABI */
          nếu (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	nếu (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    switch (type)
	      {
	      case FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		break;
	      case FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		break;
	      default:
		break;
	      }
	  }
	  switch (type)
	    {
	      case FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		break;

	      case FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		break;
		  
	      case FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		break;
		  
	      case FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		break;
		  
	      case FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		break;
		  
	      case FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#không_thì
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		break;

#ifdef FFI_MIPS_N32
	      case FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        nếu(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(float *) argp = *(float *)(* p_argv);
		    argp += z;
		    char *tmp = (void *) (*p_argv);
		    *(float *) argp = *(float *)(tmp+4);
		  }
		không_thì
		  memcpy(argp, *p_argv, (*p_arg)->size);
		break;
#endif
	      /* This can only happen with 64bit slots.  */
	      case FFI_TYPE_FLOAT:
		*(float *) argp = *(float *)(* p_argv);
		break;

	      /* Handle structures.  */
	      default:
		memcpy(argp, *p_argv, (*p_arg)->size);
		break;
	    }
	}
      không_thì
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#không_thì
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) stack + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    nếu (end <= cap)
	      memcpy(argp, *p_argv, z);
	    không_thì
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = stack;
                z -= portion;
		memcpy(argp, (void*)((unsigned long)(*p_argv) + portion),
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

static số_nguyên
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  nếu (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      nếu (*arg_reg > 7)
	trả_về 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  trả_về 0;
}

static unsigned
calc_n32_struct_flags(số_nguyên soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  nếu (soft_float)
    trả_về 0;

  trong_khi ((e = arg->elements[index]))
    {
      nếu (e->type == FFI_TYPE_COMPLEX)
	{
	  nếu (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    break;
	  nếu (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    break;
	}
      không_thì
	nếu (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  break;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  trả_về flags;
}

static unsigned
calc_n32_return_struct_flags(số_nguyên soft_float, ffi_type *arg)
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
  
  nếu (arg->size > 16)
    trả_về 0;

  nếu (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  nếu (e->type == FFI_TYPE_COMPLEX)
    {
      số_nguyên type = e->elements[0]->type;

      nếu (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	trả_về small;

      nếu (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  trả_về small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  không_thì
    {
      nếu (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	trả_về small;

      flags = e->type;

      nếu (arg->elements[1])
	{
	  e = arg->elements[1];
	  nếu (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    trả_về small;

	  nếu (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      trả_về small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  nếu (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  trả_về flags;
}

#endif

/* Perform machine dependent cif processing */
static ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  nếu (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      nếu (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  switch ((cif->arg_types)[0]->type)
	    {
	    case FFI_TYPE_FLOAT:
	    case FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      break;
	      
	    default:
	      break;
	    }

	  nếu (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      nếu (cif->flags)
		{
		  switch ((cif->arg_types)[1]->type)
		    {
		    case FFI_TYPE_FLOAT:
		    case FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      break;
		      
		    default:
		      break;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  nếu (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      switch (cif->rtype->type)
        {
        case FFI_TYPE_VOID:
        case FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          break;

        case FFI_TYPE_SINT64:
        case FFI_TYPE_UINT64:
        case FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          break;
      
        case FFI_TYPE_FLOAT:
        default:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          break;
        }
    }
  không_thì
    {
      /* FFI_O32 */      
      switch (cif->rtype->type)
        {
        case FFI_TYPE_VOID:
        case FFI_TYPE_STRUCT:
        case FFI_TYPE_FLOAT:
        case FFI_TYPE_DOUBLE:
        case FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  nếu (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          break;

        case FFI_TYPE_SINT64:
        case FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          break;
      
        default:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          break;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned count = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    số_nguyên soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    nếu (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	nếu (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    count = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	không_thì
	    cif->rstruct_flag = 0;
      }
    không_thì
      cif->rstruct_flag = 0;

    trong_khi (count-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	switch (t->type)
	  {
	  case FFI_TYPE_FLOAT:
	  case FFI_TYPE_DOUBLE:
	    nếu (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    break;
          case FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    nếu (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    không_thì
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		nếu (arg_reg >= 8)
		  continue;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            break;

	  case FFI_TYPE_COMPLEX:
	    switch (t->elements[0]->type)
	      {
	      case FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		nếu (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		không_thì
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    nếu (arg_reg >= 8)
		        continue;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    nếu (arg_reg >= 8)
		        continue;
		  }
		/* passthrough */
	      case FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      case FFI_TYPE_SINT32:
	      case FFI_TYPE_UINT32:
	      case FFI_TYPE_DOUBLE:
		nếu (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		không_thì
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    nếu (arg_reg >= 8)
		        continue;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		break;
	      default:
		arg_reg += 2;
		break;
	      }
	    break;

	  case FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    break;

	  default:
	    arg_reg++;
            break;
	  }

	index++;
      }

  /* Set the return type flag */
    switch (cif->rtype->type)
      {
      case FFI_TYPE_STRUCT:
	{
	  nếu (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  không_thì
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  break;
	}
      
      case FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	break;

      case FFI_TYPE_POINTER:
	nếu (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	không_thì
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	break;

      case FFI_TYPE_FLOAT:
	nếu (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    break;
	  }
	/* else fall through */
      case FFI_TYPE_DOUBLE:
	nếu (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	không_thì
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	break;

      case FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	nếu (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	không_thì
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	break;
      case FFI_TYPE_COMPLEX:
	{
	  số_nguyên type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  nếu (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      switch (type)
		{
		case FFI_TYPE_DOUBLE:
		case FFI_TYPE_SINT64:
		case FFI_TYPE_UINT64:
		case FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  break;
		case FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  break;
		case FFI_TYPE_FLOAT:
		default:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  không_thì
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  break;
	}
      case FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	break;
      case FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	break;
      default:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	break;
      }
  }
#endif
  trả_về FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    trả_về ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    trả_về ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
extern số_nguyên ffi_call_O32(void (*)(char *, extended_cif *, số_nguyên, số_nguyên), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, void (*)(void), void *closure);

/* Low level routine for calling N32 functions */
extern số_nguyên ffi_call_N32(void (*)(char *, extended_cif *, số_nguyên, số_nguyên), 
			extended_cif *, unsigned, 
			unsigned, void *, void (*)(void), void *closure);

void ffi_call_int(ffi_cif *cif, void (*fn)(void), void *rvalue, 
	      void **avalue, void *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  nếu ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  không_thì
    ecif.rvalue = rvalue;
    
  switch (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    case FFI_O32:
    case FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      break;
#endif

#ifdef FFI_MIPS_N32
    case FFI_N32:
    case FFI_N32_SOFT_FLOAT:
    case FFI_N64:
    case FFI_N64_SOFT_FLOAT:
      {
        số_nguyên copy_rvalue = 0;
	số_nguyên copy_offset = 0;
        char *rvalue_copy = ecif.rvalue;
        nếu (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	không_thì nếu (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#nếu defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        nếu (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      break;
#endif

    default:
      FFI_ASSERT(0);
      break;
    }
}

void
ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, NULL);
}

void
ffi_call_go (ffi_cif *cif, void (*fn)(void), void *rvalue,
	     void **avalue, void *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#nếu FFI_CLOSURES
#nếu defined(FFI_MIPS_O32)
extern void ffi_closure_O32(void);
extern void ffi_go_closure_O32(void);
#không_thì
extern void ffi_closure_N32(void);
extern void ffi_go_closure_N32(void);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      void (*fun)(ffi_cif*,void*,void**,void*),
		      void *user_data,
		      void *codeloc)
{
  unsigned số_nguyên *tramp = (unsigned số_nguyên *) &closure->tramp[0];
  void * fn;
  char *clear_location = (char *) codeloc;

#nếu defined(FFI_MIPS_O32)
  nếu (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
  fn = ffi_closure_O32;
#không_thì
#nếu _MIPS_SIM ==_ABIN32
  nếu (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
#không_thì
  nếu (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#nếu defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#nếu !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#không_thì
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#không_thì
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
#nếu !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#không_thì
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#nếu !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#không_thì
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  trả_về FFI_OK;
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
số_nguyên
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            void (*fun)(ffi_cif*, void*, void**, void*),
			    void *user_data,
			    void *rvalue, ffi_arg *ar,
			    double *fpr)
{
  void **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  số_nguyên i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * sizeof (ffi_arg));
  avaluep = alloca (cif->nargs * sizeof (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  nếu ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (void *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  nếu ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  trong_khi (i < avn)
    {
      nếu (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      nếu (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#nếu defined(__MIPSEB__) || defined(_MIPSEB)
	  nếu (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((char *) &fpr[i]) + sizeof (float);
	  không_thì
#endif
	    avaluep[i] = (char *) &fpr[i];
	}
      không_thì
	{
	  switch (arg_types[i]->type)
	    {
	      case FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		break;

	      case FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		break;
		  
	      case FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		break;
		  
	      case FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		break;

	      default:
		avaluep[i] = (char *) &ar[argn];
		break;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  nếu (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      switch (cif->rtype->type)
        {
        case FFI_TYPE_FLOAT:
          trả_về FFI_TYPE_INT;
        case FFI_TYPE_DOUBLE:
          trả_về FFI_TYPE_UINT64;
        default:
          trả_về cif->rtype->type;
        }
    }
  không_thì
    {
      nếu (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ volatile ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      trả_về cif->rtype->type;
    }
}

#nếu defined(FFI_MIPS_N32)

static void
copy_struct_N32(char *target, unsigned offset, ffi_abi abi, ffi_type *type,
                số_nguyên argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, số_nguyên soft_float)
{
  ffi_type **elt_typep = type->elements;
  trong_khi(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      char *tp;
      char *argp;
      char *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / sizeof(ffi_arg);
      arg_offset = arg_offset % sizeof(ffi_arg);

      argp = (char *)(ar + argn);
      fpp = (char *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      nếu (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(double *)tp = *(double *)fpp;
      không_thì
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / sizeof(ffi_arg);
      arg_offset = arg_offset % sizeof(ffi_arg);
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
số_nguyên
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    void (*fun)(ffi_cif*, void*, void**, void*),
                            void *user_data,
			    void *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  void **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  số_nguyên i, avn, argn;
  số_nguyên soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * sizeof (ffi_arg));
  avaluep = alloca (cif->nargs * sizeof (ffi_arg));

  argn = 0;

  nếu (cif->rstruct_flag)
    {
#nếu _MIPS_SIM==_ABIN32
      rvalue = (void *)(UINT32)ar[0];
#không_thì /* N64 */
      rvalue = (void *)ar[0];
#endif
      argn = 1;
    }
  nếu (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  trong_khi (i < avn)
    {
      nếu (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          nếu ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#nếu defined(__MIPSEB__) || defined(_MIPSEB)
          nếu (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((char *) argp) + sizeof (float);
          không_thì
#endif
            avaluep[i] = (char *) argp;
        }
      không_thì nếu (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (char *) argp;
        }
      không_thì nếu (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (char *) argp;
        }
      không_thì nếu (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          nếu (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  không_thì
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (char *) argp;
        }
      không_thì
        {
          unsigned type = arg_types[i]->type;

          nếu (arg_types[i]->alignment > sizeof(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / sizeof(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          nếu (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  nếu (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          switch (type)
            {
            case FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              break;

            case FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              break;

            case FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              break;

            case FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              break;

            case FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              break;

            case FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              break;

            case FFI_TYPE_STRUCT:
              nếu (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  break;
                }
              /* Else fall through.  */
            default:
              avaluep[i] = (char *) argp;
              break;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, sizeof(ffi_arg)) / sizeof(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  trả_về cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#nếu defined(FFI_MIPS_O32)
extern void ffi_closure_O32(void);
extern void ffi_go_closure_O32(void);
#không_thì
extern void ffi_closure_N32(void);
extern void ffi_go_closure_N32(void);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     void (*fun)(ffi_cif*,void*,void**,void*))
{
  void * fn;

#nếu defined(FFI_MIPS_O32)
  nếu (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#không_thì
#nếu _MIPS_SIM ==_ABIN32
  nếu (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
#không_thì
  nếu (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    trả_về FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (void *)fn;
  closure->cif = cif;
  closure->fun = fun;

  trả_về FFI_OK;
}

#endif /* FFI_CLOSURES */

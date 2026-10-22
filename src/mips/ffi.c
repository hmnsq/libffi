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

#अन्तर्भाव <ffi.h>
#अन्तर्भाव <ffi_common.h>

#अन्तर्भाव <stdint.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdio.h>

#ifdef __GNUC__
#  जर (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  जर defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  अन्यथा
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#अन्यथा
# define FFI_MIPS_STOP_HERE() करा {} पर्यंत(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &चळत[bytes]); \
जर (argp == &चळत[bytes]) \
{ \
  argp = चळत; \
  FFI_MIPS_STOP_HERE(); \
}
#अन्यथा
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

स्थिर शून्य ffi_prep_args(अक्षर *चळत, 
			  extended_cif *ecif,
			  पूर्णांक bytes,
			  पूर्णांक flags)
{
  पूर्णांक i;
  शून्य **p_argv;
  अक्षर *argp, *argp_f;
  ffi_type **p_arg;

  स_रखो(चळत, 0, bytes);

#ifdef FFI_MIPS_N32
  पूर्णांक soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  जर (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      जर (bytes + 16 > 8 * आकार(ffi_arg))
        argp = &चळत[bytes - (8 * आकार(ffi_arg))];
      अन्यथा
        argp = चळत;
      * (अचिन्हित दीर्घ *) argp = (अचिन्हित दीर्घ) ecif->rvalue;
      argp += 16;
    }
  अन्यथा
    {
      जर (bytes > 8 * आकार(ffi_arg))
        argp = &चळत[bytes - (8 * आकार(ffi_arg))];
      अन्यथा
        argp = चळत;
    }
#अन्यथा
  argp = चळत;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  जर ( ecif->cif->rstruct_flag != 0 )
#अन्यथा
  जर ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += आकार(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  साठी (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      माप_प्रकार z;
      अचिन्हित पूर्णांक a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      जर (a < आकार(ffi_arg))
        a = आकार(ffi_arg);
      
      जर ((a - 1) & (अचिन्हित दीर्घ) argp)
	{
	  argp = (अक्षर *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      जर (z <= आकार(ffi_arg))
	{
          पूर्णांक type = (*p_arg)->type;
	  z = आकार(ffi_arg);

          /* The size of a pointer depends on the ABI */
          जर (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	जर (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    निवड (type)
	      {
	      स्थिती FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		थांबा;
	      स्थिती FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		थांबा;
	      पूर्वनिर्धारित:
		थांबा;
	      }
	  }
	  निवड (type)
	    {
	      स्थिती FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		थांबा;

	      स्थिती FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		थांबा;
		  
	      स्थिती FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		थांबा;
		  
	      स्थिती FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		थांबा;
		  
	      स्थिती FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		थांबा;
		  
	      स्थिती FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#अन्यथा
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		थांबा;

#ifdef FFI_MIPS_N32
	      स्थिती FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        जर(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(दशांश *) argp = *(दशांश *)(* p_argv);
		    argp += z;
		    अक्षर *tmp = (शून्य *) (*p_argv);
		    *(दशांश *) argp = *(दशांश *)(tmp+4);
		  }
		अन्यथा
		  स_नकल(argp, *p_argv, (*p_arg)->size);
		थांबा;
#endif
	      /* This can only happen with 64bit slots.  */
	      स्थिती FFI_TYPE_FLOAT:
		*(दशांश *) argp = *(दशांश *)(* p_argv);
		थांबा;

	      /* Handle structures.  */
	      पूर्वनिर्धारित:
		स_नकल(argp, *p_argv, (*p_arg)->size);
		थांबा;
	    }
	}
      अन्यथा
	{
#ifdef FFI_MIPS_O32
	  स_नकल(argp, *p_argv, z);
#अन्यथा
	  {
	    अचिन्हित दीर्घ end = (अचिन्हित दीर्घ) argp + z;
	    अचिन्हित दीर्घ cap = (अचिन्हित दीर्घ) चळत + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    जर (end <= cap)
	      स_नकल(argp, *p_argv, z);
	    अन्यथा
	      {
		अचिन्हित दीर्घ portion = cap - (अचिन्हित दीर्घ)argp;

		स_नकल(argp, *p_argv, portion);
		argp = चळत;
                z -= portion;
		स_नकल(argp, (शून्य*)((अचिन्हित दीर्घ)(*p_argv) + portion),
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

स्थिर पूर्णांक
calc_n32_struct_flags_element(अचिन्हित *flags, ffi_type *e,
			      अचिन्हित *loc, अचिन्हित *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  जर (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      जर (*arg_reg > 7)
	परत 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  परत 0;
}

स्थिर अचिन्हित
calc_n32_struct_flags(पूर्णांक soft_float, ffi_type *arg,
		      अचिन्हित *loc, अचिन्हित *arg_reg)
{
  अचिन्हित flags = 0;
  अचिन्हित index = 0;

  ffi_type *e;

  जर (soft_float)
    परत 0;

  पर्यंत ((e = arg->elements[index]))
    {
      जर (e->type == FFI_TYPE_COMPLEX)
	{
	  जर (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    थांबा;
	  जर (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    थांबा;
	}
      अन्यथा
	जर (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  थांबा;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  परत flags;
}

स्थिर अचिन्हित
calc_n32_return_struct_flags(पूर्णांक soft_float, ffi_type *arg)
{
  अचिन्हित flags;
  अचिन्हित small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  जर (arg->size > 16)
    परत 0;

  जर (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  जर (e->type == FFI_TYPE_COMPLEX)
    {
      पूर्णांक type = e->elements[0]->type;

      जर (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	परत small;

      जर (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  परत small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  अन्यथा
    {
      जर (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	परत small;

      flags = e->type;

      जर (arg->elements[1])
	{
	  e = arg->elements[1];
	  जर (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    परत small;

	  जर (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      परत small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  जर (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  परत flags;
}

#endif

/* Perform machine dependent cif processing */
स्थिर ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, अचिन्हित nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  जर (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      जर (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  निवड ((cif->arg_types)[0]->type)
	    {
	    स्थिती FFI_TYPE_FLOAT:
	    स्थिती FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      थांबा;
	      
	    पूर्वनिर्धारित:
	      थांबा;
	    }

	  जर (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      जर (cif->flags)
		{
		  निवड ((cif->arg_types)[1]->type)
		    {
		    स्थिती FFI_TYPE_FLOAT:
		    स्थिती FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      थांबा;
		      
		    पूर्वनिर्धारित:
		      थांबा;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  जर (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      निवड (cif->rtype->type)
        {
        स्थिती FFI_TYPE_VOID:
        स्थिती FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          थांबा;

        स्थिती FFI_TYPE_SINT64:
        स्थिती FFI_TYPE_UINT64:
        स्थिती FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          थांबा;
      
        स्थिती FFI_TYPE_FLOAT:
        पूर्वनिर्धारित:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          थांबा;
        }
    }
  अन्यथा
    {
      /* FFI_O32 */      
      निवड (cif->rtype->type)
        {
        स्थिती FFI_TYPE_VOID:
        स्थिती FFI_TYPE_STRUCT:
        स्थिती FFI_TYPE_FLOAT:
        स्थिती FFI_TYPE_DOUBLE:
        स्थिती FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  जर (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          थांबा;

        स्थिती FFI_TYPE_SINT64:
        स्थिती FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          थांबा;
      
        पूर्वनिर्धारित:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          थांबा;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    अचिन्हित arg_reg = 0;
    अचिन्हित loc = 0;
    अचिन्हित मोजणी = (cif->nargs < 8) ? cif->nargs : 8;
    अचिन्हित index = 0;

    अचिन्हित struct_flags = 0;
    पूर्णांक soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    जर (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	जर (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    मोजणी = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	अन्यथा
	    cif->rstruct_flag = 0;
      }
    अन्यथा
      cif->rstruct_flag = 0;

    पर्यंत (मोजणी-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	निवड (t->type)
	  {
	  स्थिती FFI_TYPE_FLOAT:
	  स्थिती FFI_TYPE_DOUBLE:
	    जर (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    थांबा;
          स्थिती FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    जर (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    अन्यथा
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		जर (arg_reg >= 8)
		  सुरूठेवा;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            थांबा;

	  स्थिती FFI_TYPE_COMPLEX:
	    निवड (t->elements[0]->type)
	      {
	      स्थिती FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		जर (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अन्यथा
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    जर (arg_reg >= 8)
		        सुरूठेवा;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    जर (arg_reg >= 8)
		        सुरूठेवा;
		  }
		/* passthrough */
	      स्थिती FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      स्थिती FFI_TYPE_SINT32:
	      स्थिती FFI_TYPE_UINT32:
	      स्थिती FFI_TYPE_DOUBLE:
		जर (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अन्यथा
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    जर (arg_reg >= 8)
		        सुरूठेवा;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		थांबा;
	      पूर्वनिर्धारित:
		arg_reg += 2;
		थांबा;
	      }
	    थांबा;

	  स्थिती FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    थांबा;

	  पूर्वनिर्धारित:
	    arg_reg++;
            थांबा;
	  }

	index++;
      }

  /* Set the return type flag */
    निवड (cif->rtype->type)
      {
      स्थिती FFI_TYPE_STRUCT:
	{
	  जर (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  अन्यथा
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  थांबा;
	}
      
      स्थिती FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	थांबा;

      स्थिती FFI_TYPE_POINTER:
	जर (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	अन्यथा
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	थांबा;

      स्थिती FFI_TYPE_FLOAT:
	जर (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    थांबा;
	  }
	/* else fall through */
      स्थिती FFI_TYPE_DOUBLE:
	जर (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	अन्यथा
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	थांबा;

      स्थिती FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	जर (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	अन्यथा
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	थांबा;
      स्थिती FFI_TYPE_COMPLEX:
	{
	  पूर्णांक type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  जर (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      निवड (type)
		{
		स्थिती FFI_TYPE_DOUBLE:
		स्थिती FFI_TYPE_SINT64:
		स्थिती FFI_TYPE_UINT64:
		स्थिती FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  थांबा;
		स्थिती FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  थांबा;
		स्थिती FFI_TYPE_FLOAT:
		पूर्वनिर्धारित:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  अन्यथा
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  थांबा;
	}
      स्थिती FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	थांबा;
      स्थिती FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	थांबा;
      पूर्वनिर्धारित:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	थांबा;
      }
  }
#endif
  परत FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    परत ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    अचिन्हित nfixedargs,
                                    अचिन्हित ntotalargs MAYBE_UNUSED)
{
    परत ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
बाह्य पूर्णांक ffi_call_O32(शून्य (*)(अक्षर *, extended_cif *, पूर्णांक, पूर्णांक), 
			extended_cif *, अचिन्हित, 
			अचिन्हित, अचिन्हित *, शून्य (*)(शून्य), शून्य *closure);

/* Low level routine for calling N32 functions */
बाह्य पूर्णांक ffi_call_N32(शून्य (*)(अक्षर *, extended_cif *, पूर्णांक, पूर्णांक), 
			extended_cif *, अचिन्हित, 
			अचिन्हित, शून्य *, शून्य (*)(शून्य), शून्य *closure);

शून्य ffi_call_int(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, 
	      शून्य **avalue, शून्य *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  जर ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  अन्यथा
    ecif.rvalue = rvalue;
    
  निवड (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    स्थिती FFI_O32:
    स्थिती FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      थांबा;
#endif

#ifdef FFI_MIPS_N32
    स्थिती FFI_N32:
    स्थिती FFI_N32_SOFT_FLOAT:
    स्थिती FFI_N64:
    स्थिती FFI_N64_SOFT_FLOAT:
      {
        पूर्णांक copy_rvalue = 0;
	पूर्णांक copy_offset = 0;
        अक्षर *rvalue_copy = ecif.rvalue;
        जर (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	अन्यथा जर (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#जर defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        जर (copy_rvalue)
          स_नकल(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      थांबा;
#endif

    पूर्वनिर्धारित:
      FFI_ASSERT(0);
      थांबा;
    }
}

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#जर FFI_CLOSURES
#जर defined(FFI_MIPS_O32)
बाह्य शून्य ffi_closure_O32(शून्य);
बाह्य शून्य ffi_go_closure_O32(शून्य);
#अन्यथा
बाह्य शून्य ffi_closure_N32(शून्य);
बाह्य शून्य ffi_go_closure_N32(शून्य);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_data,
		      शून्य *codeloc)
{
  अचिन्हित पूर्णांक *tramp = (अचिन्हित पूर्णांक *) &closure->tramp[0];
  शून्य * fn;
  अक्षर *clear_location = (अक्षर *) codeloc;

#जर defined(FFI_MIPS_O32)
  जर (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    परत FFI_BAD_ABI;
  fn = ffi_closure_O32;
#अन्यथा
#जर _MIPS_SIM ==_ABIN32
  जर (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    परत FFI_BAD_ABI;
#अन्यथा
  जर (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    परत FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#जर defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((अचिन्हित)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((अचिन्हित)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((अचिन्हित)codeloc >> 16);
  /* jr   $25          */
#जर !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#अन्यथा
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((अचिन्हित)codeloc & 0xffff);
#अन्यथा
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((अचिन्हित दीर्घ)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((अचिन्हित दीर्घ)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((अचिन्हित दीर्घ)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((अचिन्हित दीर्घ)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((अचिन्हित दीर्घ)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((अचिन्हित दीर्घ)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((अचिन्हित दीर्घ)fn  & 0xffff);
  /* jr   $25          */
#जर !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#अन्यथा
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((अचिन्हित दीर्घ)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#जर !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#अन्यथा
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  परत FFI_OK;
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
पूर्णांक
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			    शून्य *user_data,
			    शून्य *rvalue, ffi_arg *ar,
			    द्विगुण *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णांक i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * आकार (ffi_arg));
  avaluep = alloca (cif->nargs * आकार (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  जर ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (शून्य *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  जर ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  पर्यंत (i < avn)
    {
      जर (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      जर (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#जर defined(__MIPSEB__) || defined(_MIPSEB)
	  जर (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((अक्षर *) &fpr[i]) + आकार (दशांश);
	  अन्यथा
#endif
	    avaluep[i] = (अक्षर *) &fpr[i];
	}
      अन्यथा
	{
	  निवड (arg_types[i]->type)
	    {
	      स्थिती FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		थांबा;

	      स्थिती FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		थांबा;
		  
	      स्थिती FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		थांबा;
		  
	      स्थिती FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		थांबा;

	      पूर्वनिर्धारित:
		avaluep[i] = (अक्षर *) &ar[argn];
		थांबा;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  जर (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      निवड (cif->rtype->type)
        {
        स्थिती FFI_TYPE_FLOAT:
          परत FFI_TYPE_INT;
        स्थिती FFI_TYPE_DOUBLE:
          परत FFI_TYPE_UINT64;
        पूर्वनिर्धारित:
          परत cif->rtype->type;
        }
    }
  अन्यथा
    {
      जर (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ अस्थिर ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      परत cif->rtype->type;
    }
}

#जर defined(FFI_MIPS_N32)

स्थिर शून्य
copy_struct_N32(अक्षर *target, अचिन्हित offset, ffi_abi abi, ffi_type *type,
                पूर्णांक argn, अचिन्हित arg_offset, ffi_arg *ar,
                ffi_arg *fpr, पूर्णांक soft_float)
{
  ffi_type **elt_typep = type->elements;
  पर्यंत(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      अचिन्हित o;
      अक्षर *tp;
      अक्षर *argp;
      अक्षर *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / आकार(ffi_arg);
      arg_offset = arg_offset % आकार(ffi_arg);

      argp = (अक्षर *)(ar + argn);
      fpp = (अक्षर *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      जर (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(द्विगुण *)tp = *(द्विगुण *)fpp;
      अन्यथा
        स_नकल(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / आकार(ffi_arg);
      arg_offset = arg_offset % आकार(ffi_arg);
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
पूर्णांक
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                            शून्य *user_data,
			    शून्य *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णांक i, avn, argn;
  पूर्णांक soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * आकार (ffi_arg));
  avaluep = alloca (cif->nargs * आकार (ffi_arg));

  argn = 0;

  जर (cif->rstruct_flag)
    {
#जर _MIPS_SIM==_ABIN32
      rvalue = (शून्य *)(UINT32)ar[0];
#अन्यथा /* N64 */
      rvalue = (शून्य *)ar[0];
#endif
      argn = 1;
    }
  जर (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  पर्यंत (i < avn)
    {
      जर (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          जर ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#जर defined(__MIPSEB__) || defined(_MIPSEB)
          जर (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((अक्षर *) argp) + आकार (दशांश);
          अन्यथा
#endif
            avaluep[i] = (अक्षर *) argp;
        }
      अन्यथा जर (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (अक्षर *) argp;
        }
      अन्यथा जर (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (अक्षर *) argp;
        }
      अन्यथा जर (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          जर (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  अन्यथा
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (अक्षर *) argp;
        }
      अन्यथा
        {
          अचिन्हित type = arg_types[i]->type;

          जर (arg_types[i]->alignment > आकार(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / आकार(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          जर (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  जर (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          निवड (type)
            {
            स्थिती FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              थांबा;

            स्थिती FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              थांबा;

            स्थिती FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              थांबा;

            स्थिती FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              थांबा;

            स्थिती FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              थांबा;

            स्थिती FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              थांबा;

            स्थिती FFI_TYPE_STRUCT:
              जर (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  थांबा;
                }
              /* Else fall through.  */
            पूर्वनिर्धारित:
              avaluep[i] = (अक्षर *) argp;
              थांबा;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, आकार(ffi_arg)) / आकार(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  परत cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#जर defined(FFI_MIPS_O32)
बाह्य शून्य ffi_closure_O32(शून्य);
बाह्य शून्य ffi_go_closure_O32(शून्य);
#अन्यथा
बाह्य शून्य ffi_closure_N32(शून्य);
बाह्य शून्य ffi_go_closure_N32(शून्य);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य * fn;

#जर defined(FFI_MIPS_O32)
  जर (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    परत FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#अन्यथा
#जर _MIPS_SIM ==_ABIN32
  जर (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    परत FFI_BAD_ABI;
#अन्यथा
  जर (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    परत FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (शून्य *)fn;
  closure->cif = cif;
  closure->fun = fun;

  परत FFI_OK;
}

#endif /* FFI_CLOSURES */

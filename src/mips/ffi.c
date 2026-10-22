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

#यदि_परिभाषितम् __GNUC__
#  यदि (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#यदि_अन्तः

#यदि_अपरिभाषितम् USE__BUILTIN___CLEAR_CACHE
#  यदि defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  अन्यथा
#    include <sys/cachectl.h>
#  endif
#यदि_अन्तः

#यदि_परिभाषितम् FFI_DEBUG
# define FFI_MIPS_STOP_अत्र() ffi_stop_अत्र()
#अन्यथा
# define FFI_MIPS_STOP_अत्र() कुरु {} यावत्(0)
#यदि_अन्तः

#यदि_परिभाषितम् FFI_MIPS_N32
#परिभाषय FIX_ARGP \
FFI_ASSERT(argp <= &राशिः[अष्टकानि]); \
यदि (argp == &राशिः[अष्टकानि]) \
{ \
  argp = राशिः; \
  FFI_MIPS_STOP_अत्र(); \
}
#अन्यथा
#परिभाषय FIX_ARGP 
#यदि_अन्तः


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

स्थिर शून्य ffi_prep_तर्काः(वर्ण *राशिः, 
			  extended_cif *ecif,
			  पूर्णाङ्क अष्टकानि,
			  पूर्णाङ्क flags)
{
  पूर्णाङ्क i;
  शून्य **p_argv;
  वर्ण *argp, *argp_f;
  ffi_type **p_arg;

  स्मृतिं_पूरय(राशिः, 0, अष्टकानि);

#यदि_परिभाषितम् FFI_MIPS_N32
  पूर्णाङ्क soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  यदि (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      यदि (अष्टकानि + 16 > 8 * परिमाण(ffi_arg))
        argp = &राशिः[अष्टकानि - (8 * परिमाण(ffi_arg))];
      अन्यथा
        argp = राशिः;
      * (अचिह्नित दीर्घ *) argp = (अचिह्नित दीर्घ) ecif->rvalue;
      argp += 16;
    }
  अन्यथा
    {
      यदि (अष्टकानि > 8 * परिमाण(ffi_arg))
        argp = &राशिः[अष्टकानि - (8 * परिमाण(ffi_arg))];
      अन्यथा
        argp = राशिः;
    }
#अन्यथा
  argp = राशिः;
#यदि_अन्तः

  argp_f = argp;

#यदि_परिभाषितम् FFI_MIPS_N32
  यदि ( ecif->cif->rstruct_flag != 0 )
#अन्यथा
  यदि ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#यदि_अन्तः  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += परिमाण(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  प्रति (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      अचिह्नित पूर्णाङ्क a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      यदि (a < परिमाण(ffi_arg))
        a = परिमाण(ffi_arg);
      
      यदि ((a - 1) & (अचिह्नित दीर्घ) argp)
	{
	  argp = (वर्ण *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->आकार;
      यदि (z <= परिमाण(ffi_arg))
	{
          पूर्णाङ्क type = (*p_arg)->type;
	  z = परिमाण(ffi_arg);

          /* The size of a pointer depends on the ABI */
          यदि (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	यदि (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    विकल्प (type)
	      {
	      पक्ष FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		भङ्ग;
	      पक्ष FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		भङ्ग;
	      स्वभाव:
		भङ्ग;
	      }
	  }
	  विकल्प (type)
	    {
	      पक्ष FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		भङ्ग;

	      पक्ष FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT32:
#यदि_परिभाषितम् FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#अन्यथा
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#यदि_अन्तः
		भङ्ग;

#यदि_परिभाषितम् FFI_MIPS_N32
	      पक्ष FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        यदि(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(भिन्न *) argp = *(भिन्न *)(* p_argv);
		    argp += z;
		    वर्ण *tmp = (शून्य *) (*p_argv);
		    *(भिन्न *) argp = *(भिन्न *)(tmp+4);
		  }
		अन्यथा
		  स्मृतिं_प्रतिलिख(argp, *p_argv, (*p_arg)->आकार);
		भङ्ग;
#यदि_अन्तः
	      /* This can only happen with 64bit slots.  */
	      पक्ष FFI_TYPE_FLOAT:
		*(भिन्न *) argp = *(भिन्न *)(* p_argv);
		भङ्ग;

	      /* Handle structures.  */
	      स्वभाव:
		स्मृतिं_प्रतिलिख(argp, *p_argv, (*p_arg)->आकार);
		भङ्ग;
	    }
	}
      अन्यथा
	{
#यदि_परिभाषितम् FFI_MIPS_O32
	  स्मृतिं_प्रतिलिख(argp, *p_argv, z);
#अन्यथा
	  {
	    अचिह्नित दीर्घ अन्तः = (अचिह्नित दीर्घ) argp + z;
	    अचिह्नित दीर्घ cap = (अचिह्नित दीर्घ) राशिः + अष्टकानि;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    यदि (अन्तः <= cap)
	      स्मृतिं_प्रतिलिख(argp, *p_argv, z);
	    अन्यथा
	      {
		अचिह्नित दीर्घ portion = cap - (अचिह्नित दीर्घ)argp;

		स्मृतिं_प्रतिलिख(argp, *p_argv, portion);
		argp = राशिः;
                z -= portion;
		स्मृतिं_प्रतिलिख(argp, (शून्य*)((अचिह्नित दीर्घ)(*p_argv) + portion),
                       z);
	      }
	  }
#यदि_अन्तः
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#यदि_परिभाषितम् FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

स्थिर पूर्णाङ्क
calc_n32_struct_flags_element(अचिह्नित *flags, ffi_type *e,
			      अचिह्नित *loc, अचिह्नित *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  यदि (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      यदि (*arg_reg > 7)
	प्रत्यागम 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_द्वयङ्काः));
    }
  *loc += e->आकार;
  प्रत्यागम 0;
}

स्थिर अचिह्नित
calc_n32_struct_flags(पूर्णाङ्क soft_float, ffi_type *arg,
		      अचिह्नित *loc, अचिह्नित *arg_reg)
{
  अचिह्नित flags = 0;
  अचिह्नित सूचकांक = 0;

  ffi_type *e;

  यदि (soft_float)
    प्रत्यागम 0;

  यावत् ((e = arg->elements[सूचकांक]))
    {
      यदि (e->type == FFI_TYPE_COMPLEX)
	{
	  यदि (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    भङ्ग;
	  यदि (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    भङ्ग;
	}
      अन्यथा
	यदि (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  भङ्ग;
      सूचकांक++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  प्रत्यागम flags;
}

स्थिर अचिह्नित
calc_n32_return_struct_flags(पूर्णाङ्क soft_float, ffi_type *arg)
{
  अचिह्नित flags;
  अचिह्नित small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  यदि (arg->आकार > 16)
    प्रत्यागम 0;

  यदि (arg->आकार > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  यदि (e->type == FFI_TYPE_COMPLEX)
    {
      पूर्णाङ्क type = e->elements[0]->type;

      यदि (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	प्रत्यागम small;

      यदि (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  प्रत्यागम small;
	}

      flags = (type << FFI_FLAG_द्वयङ्काः) + type;
    }
  अन्यथा
    {
      यदि (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	प्रत्यागम small;

      flags = e->type;

      यदि (arg->elements[1])
	{
	  e = arg->elements[1];
	  यदि (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    प्रत्यागम small;

	  यदि (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      प्रत्यागम small;
	    }

	  flags += e->type << FFI_FLAG_द्वयङ्काः;
	}
    }

  यदि (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  प्रत्यागम flags;
}

#यदि_अन्तः

/* Perform machine dependent cif processing */
स्थिर ffi_स्थितिः ffi_prep_cif_machdep_int(ffi_cif *cif, अचिह्नित nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#यदि_परिभाषितम् FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  यदि (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      यदि (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  विकल्प ((cif->arg_types)[0]->type)
	    {
	    पक्ष FFI_TYPE_FLOAT:
	    पक्ष FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      भङ्ग;
	      
	    स्वभाव:
	      भङ्ग;
	    }

	  यदि (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      यदि (cif->flags)
		{
		  विकल्प ((cif->arg_types)[1]->type)
		    {
		    पक्ष FFI_TYPE_FLOAT:
		    पक्ष FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_द्वयङ्काः;
		      भङ्ग;
		      
		    स्वभाव:
		      भङ्ग;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  यदि (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_VOID:
        पक्ष FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_द्वयङ्काः * 2);
          भङ्ग;

        पक्ष FFI_TYPE_SINT64:
        पक्ष FFI_TYPE_UINT64:
        पक्ष FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_द्वयङ्काः * 2);
          भङ्ग;
      
        पक्ष FFI_TYPE_FLOAT:
        स्वभाव:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_द्वयङ्काः * 2);
          भङ्ग;
        }
    }
  अन्यथा
    {
      /* FFI_O32 */      
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_VOID:
        पक्ष FFI_TYPE_STRUCT:
        पक्ष FFI_TYPE_FLOAT:
        पक्ष FFI_TYPE_DOUBLE:
        पक्ष FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_द्वयङ्काः * 2);
	  यदि (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_द्वयङ्काः * 4);
          भङ्ग;

        पक्ष FFI_TYPE_SINT64:
        पक्ष FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_द्वयङ्काः * 2);
          भङ्ग;
      
        स्वभाव:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_द्वयङ्काः * 2);
          भङ्ग;
        }
    }
#यदि_अन्तः

#यदि_परिभाषितम् FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    अचिह्नित arg_reg = 0;
    अचिह्नित loc = 0;
    अचिह्नित गणनम् = (cif->nargs < 8) ? cif->nargs : 8;
    अचिह्नित सूचकांक = 0;

    अचिह्नित struct_flags = 0;
    पूर्णाङ्क soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    यदि (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	यदि (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    गणनम् = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	अन्यथा
	    cif->rstruct_flag = 0;
      }
    अन्यथा
      cif->rstruct_flag = 0;

    यावत् (गणनम्-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[सूचकांक];

	विकल्प (t->type)
	  {
	  पक्ष FFI_TYPE_FLOAT:
	  पक्ष FFI_TYPE_DOUBLE:
	    यदि (!soft_float && सूचकांक < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_द्वयङ्काः);
	    arg_reg++;
	    भङ्ग;
          पक्ष FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    यदि (soft_float || सूचकांक >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    अन्यथा
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_द्वयङ्काः));
		arg_reg++;
		यदि (arg_reg >= 8)
		  अनुवर्त;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_द्वयङ्काः));
		arg_reg++;
	      }
            भङ्ग;

	  पक्ष FFI_TYPE_COMPLEX:
	    विकल्प (t->elements[0]->type)
	      {
	      पक्ष FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		यदि (soft_float || सूचकांक >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अन्यथा
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_द्वयङ्काः));
		    arg_reg++;
		    यदि (arg_reg >= 8)
		        अनुवर्त;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_द्वयङ्काः));
		    arg_reg++;
		    यदि (arg_reg >= 8)
		        अनुवर्त;
		  }
		/* passthrough */
	      पक्ष FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->अष्टकानि += 16;
		/* passthrough */
	      पक्ष FFI_TYPE_SINT32:
	      पक्ष FFI_TYPE_UINT32:
	      पक्ष FFI_TYPE_DOUBLE:
		यदि (soft_float || सूचकांक >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		अन्यथा
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_द्वयङ्काः));
		    arg_reg++;
		    यदि (arg_reg >= 8)
		        अनुवर्त;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_द्वयङ्काः));
		    arg_reg++;
		  }
		भङ्ग;
	      स्वभाव:
		arg_reg += 2;
		भङ्ग;
	      }
	    भङ्ग;

	  पक्ष FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || सूचकांक >= nfixedargs,
						t, &loc, &arg_reg);
	    भङ्ग;

	  स्वभाव:
	    arg_reg++;
            भङ्ग;
	  }

	सूचकांक++;
      }

  /* Set the return type flag */
    विकल्प (cif->rtype->type)
      {
      पक्ष FFI_TYPE_STRUCT:
	{
	  यदि (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  अन्यथा
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_द्वयङ्काः * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_द्वयङ्काः * 8));
	    }
	  भङ्ग;
	}
      
      पक्ष FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	भङ्ग;

      पक्ष FFI_TYPE_POINTER:
	यदि (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_द्वयङ्काः * 8);
	अन्यथा
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_द्वयङ्काः * 8);
	भङ्ग;

      पक्ष FFI_TYPE_FLOAT:
	यदि (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_द्वयङ्काः * 8);
	    भङ्ग;
	  }
	/* else fall through */
      पक्ष FFI_TYPE_DOUBLE:
	यदि (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_द्वयङ्काः * 8);
	अन्यथा
	  cif->flags += cif->rtype->type << (FFI_FLAG_द्वयङ्काः * 8);
	भङ्ग;

      पक्ष FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	यदि (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_द्वयङ्काः * 8);
 	  }
	अन्यथा
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_द्वयङ्काः * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_द्वयङ्काः))
					      << (4 + (FFI_FLAG_द्वयङ्काः * 8));
	  }
	भङ्ग;
      पक्ष FFI_TYPE_COMPLEX:
	{
	  पूर्णाङ्क type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_द्वयङ्काः * 8));
	  यदि (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      विकल्प (type)
		{
		पक्ष FFI_TYPE_DOUBLE:
		पक्ष FFI_TYPE_SINT64:
		पक्ष FFI_TYPE_UINT64:
		पक्ष FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  भङ्ग;
		पक्ष FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  भङ्ग;
		पक्ष FFI_TYPE_FLOAT:
		स्वभाव:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_द्वयङ्काः * 8));
	    }
	  अन्यथा
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_द्वयङ्काः * 8));
	    }
	  भङ्ग;
	}
      पक्ष FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_द्वयङ्काः * 8);
	भङ्ग;
      पक्ष FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_द्वयङ्काः * 8);
	भङ्ग;
      स्वभाव:
	cif->flags += cif->rtype->type << (FFI_FLAG_द्वयङ्काः * 8);
	भङ्ग;
      }
  }
#यदि_अन्तः
  प्रत्यागम FFI_OK;
}

ffi_स्थितिः ffi_prep_cif_machdep(ffi_cif *cif)
{
    प्रत्यागम ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_स्थितिः ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    अचिह्नित nfixedargs,
                                    अचिह्नित ntotalargs MAYBE_अप्रयुक्तम्)
{
    प्रत्यागम ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
बाह्य पूर्णाङ्क ffi_call_O32(शून्य (*)(वर्ण *, extended_cif *, पूर्णाङ्क, पूर्णाङ्क), 
			extended_cif *, अचिह्नित, 
			अचिह्नित, अचिह्नित *, शून्य (*)(शून्य), शून्य *संवृतिः);

/* Low level routine for calling N32 functions */
बाह्य पूर्णाङ्क ffi_call_N32(शून्य (*)(वर्ण *, extended_cif *, पूर्णाङ्क, पूर्णाङ्क), 
			extended_cif *, अचिह्नित, 
			अचिह्नित, शून्य *, शून्य (*)(शून्य), शून्य *संवृतिः);

शून्य ffi_call_int(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, 
	      शून्य **avalue, शून्य *संवृतिः)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  यदि ((rvalue == रिक्त) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->आकार);
  अन्यथा
    ecif.rvalue = rvalue;
    
  विकल्प (cif->abi) 
    {
#यदि_परिभाषितम् FFI_MIPS_O32
    पक्ष FFI_O32:
    पक्ष FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_तर्काः, &ecif, cif->अष्टकानि, 
		   cif->flags, ecif.rvalue, fn, संवृतिः);
      भङ्ग;
#यदि_अन्तः

#यदि_परिभाषितम् FFI_MIPS_N32
    पक्ष FFI_N32:
    पक्ष FFI_N32_SOFT_FLOAT:
    पक्ष FFI_N64:
    पक्ष FFI_N64_SOFT_FLOAT:
      {
        पूर्णाङ्क copy_rvalue = 0;
	पूर्णाङ्क copy_विस्थापनम् = 0;
        वर्ण *rvalue_copy = ecif.rvalue;
        यदि (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->आकार < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	अन्यथा यदि (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#यदि defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_विस्थापनम् = 4;
#यदि_अन्तः
	  }
        ffi_call_N32(ffi_prep_तर्काः, &ecif, cif->अष्टकानि,
                     cif->flags, rvalue_copy, fn, संवृतिः);
        यदि (copy_rvalue)
          स्मृतिं_प्रतिलिख(ecif.rvalue, rvalue_copy + copy_विस्थापनम्, cif->rtype->आकार);
      }
      भङ्ग;
#यदि_अन्तः

    स्वभाव:
      FFI_ASSERT(0);
      भङ्ग;
    }
}

शून्य
ffi_call(ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue, शून्य **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, रिक्त);
}

शून्य
ffi_call_go (ffi_cif *cif, शून्य (*fn)(शून्य), शून्य *rvalue,
	     शून्य **avalue, शून्य *संवृतिः)
{
  ffi_call_int (cif, fn, rvalue, avalue, संवृतिः);
}


#यदि FFI_CLOSURES
#यदि defined(FFI_MIPS_O32)
बाह्य शून्य ffi_संवृतिः_O32(शून्य);
बाह्य शून्य ffi_go_संवृतिः_O32(शून्य);
#अन्यथा
बाह्य शून्य ffi_संवृतिः_N32(शून्य);
बाह्य शून्य ffi_go_संवृतिः_N32(शून्य);
#यदि_अन्तः /* FFI_MIPS_O32 */

ffi_स्थितिः
ffi_prep_संवृतिः_loc (ffi_संवृतिः *संवृतिः,
		      ffi_cif *cif,
		      शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*),
		      शून्य *user_दत्त,
		      शून्य *codeloc)
{
  अचिह्नित पूर्णाङ्क *tramp = (अचिह्नित पूर्णाङ्क *) &संवृतिः->tramp[0];
  शून्य * fn;
  वर्ण *clear_location = (वर्ण *) codeloc;

#यदि defined(FFI_MIPS_O32)
  यदि (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
  fn = ffi_संवृतिः_O32;
#अन्यथा
#यदि _MIPS_SIM ==_ABIN32
  यदि (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
#अन्यथा
  यदि (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
  fn = ffi_संवृतिः_N32;
#यदि_अन्तः /* FFI_MIPS_O32 */

#यदि defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((अचिह्नित)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((अचिह्नित)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((अचिह्नित)codeloc >> 16);
  /* jr   $25          */
#यदि !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#अन्यथा
  tramp[3] = 0x03200009;
#यदि_अन्तः
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((अचिह्नित)codeloc & 0xffff);
#अन्यथा
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((अचिह्नित दीर्घ)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((अचिह्नित दीर्घ)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((अचिह्नित दीर्घ)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((अचिह्नित दीर्घ)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((अचिह्नित दीर्घ)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((अचिह्नित दीर्घ)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((अचिह्नित दीर्घ)fn  & 0xffff);
  /* jr   $25          */
#यदि !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#अन्यथा
  tramp[11] = 0x03200009;
#यदि_अन्तः
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((अचिह्नित दीर्घ)codeloc & 0xffff);

#यदि_अन्तः

  संवृतिः->cif = cif;
  संवृतिः->fun = fun;
  संवृतिः->user_दत्त = user_दत्त;

#यदि !defined(__FreeBSD__)
#यदि_परिभाषितम् USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_आकार);
#अन्यथा
  cacheflush (clear_location, FFI_TRAMPOLINE_आकार, ICACHE);
#यदि_अन्तः
#यदि_अन्तः /* ! __FreeBSD__ */
  प्रत्यागम FFI_OK;
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
पूर्णाङ्क
ffi_संवृतिः_mips_inner_O32 (ffi_cif *cif,
                            शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
			    शून्य *user_दत्त,
			    शून्य *rvalue, ffi_arg *ar,
			    द्विगुण *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णाङ्क i, avn, argn, आलोकितम्_int;

  avalue = alloca (cif->nargs * परिमाण (ffi_arg));
  avaluep = alloca (cif->nargs * परिमाण (ffi_arg));

  आलोकितम्_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  यदि ((cif->flags >> (FFI_FLAG_द्वयङ्काः * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (शून्य *)(uintptr_t)ar[0];
      argn = 1;
      आलोकितम्_int = 1;
    }
  यदि ((cif->flags >> (FFI_FLAG_द्वयङ्काः * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  यावत् (i < avn)
    {
      यदि (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      यदि (i < 2 && !आलोकितम्_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#यदि defined(__MIPSEB__) || defined(_MIPSEB)
	  यदि (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((वर्ण *) &fpr[i]) + परिमाण (भिन्न);
	  अन्यथा
#यदि_अन्तः
	    avaluep[i] = (वर्ण *) &fpr[i];
	}
      अन्यथा
	{
	  विकल्प (arg_types[i]->type)
	    {
	      पक्ष FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		भङ्ग;

	      पक्ष FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		भङ्ग;
		  
	      पक्ष FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		भङ्ग;
		  
	      पक्ष FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		भङ्ग;

	      स्वभाव:
		avaluep[i] = (वर्ण *) &ar[argn];
		भङ्ग;
	    }
	  आलोकितम्_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->आकार, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_दत्त);

  यदि (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      विकल्प (cif->rtype->type)
        {
        पक्ष FFI_TYPE_FLOAT:
          प्रत्यागम FFI_TYPE_INT;
        पक्ष FFI_TYPE_DOUBLE:
          प्रत्यागम FFI_TYPE_UINT64;
        स्वभाव:
          प्रत्यागम cif->rtype->type;
        }
    }
  अन्यथा
    {
      यदि (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ चञ्चल ("move $v1, %0" : : "r"(cif->rtype->आकार));
      }
      प्रत्यागम cif->rtype->type;
    }
}

#यदि defined(FFI_MIPS_N32)

स्थिर शून्य
copy_struct_N32(वर्ण *target, अचिह्नित offset, ffi_abi abi, ffi_type *type,
                पूर्णाङ्क argn, अचिह्नित arg_विस्थापनम्, ffi_arg *ar,
                ffi_arg *fpr, पूर्णाङ्क soft_float)
{
  ffi_type **elt_typep = type->elements;
  यावत्(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      अचिह्नित o;
      वर्ण *tp;
      वर्ण *argp;
      वर्ण *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_विस्थापनम् += o - offset;
      offset = o;
      argn += arg_विस्थापनम् / परिमाण(ffi_arg);
      arg_विस्थापनम् = arg_विस्थापनम् % परिमाण(ffi_arg);

      argp = (वर्ण *)(ar + argn);
      fpp = (वर्ण *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      यदि (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(द्विगुण *)tp = *(द्विगुण *)fpp;
      अन्यथा
        स्मृतिं_प्रतिलिख(tp, argp + arg_विस्थापनम्, elt_type->आकार);

      offset += elt_type->आकार;
      arg_विस्थापनम् += elt_type->आकार;
      elt_typep++;
      argn += arg_विस्थापनम् / परिमाण(ffi_arg);
      arg_विस्थापनम् = arg_विस्थापनम् % परिमाण(ffi_arg);
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
पूर्णाङ्क
ffi_संवृतिः_mips_inner_N32 (ffi_cif *cif, 
			    शून्य (*fun)(ffi_cif*, शून्य*, शून्य**, शून्य*),
                            शून्य *user_दत्त,
			    शून्य *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  शून्य **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  पूर्णाङ्क i, avn, argn;
  पूर्णाङ्क soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * परिमाण (ffi_arg));
  avaluep = alloca (cif->nargs * परिमाण (ffi_arg));

  argn = 0;

  यदि (cif->rstruct_flag)
    {
#यदि _MIPS_SIM==_ABIN32
      rvalue = (शून्य *)(UINT32)ar[0];
#अन्यथा /* N64 */
      rvalue = (शून्य *)ar[0];
#यदि_अन्तः
      argn = 1;
    }
  यदि (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  यावत् (i < avn)
    {
      यदि (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          यदि ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#यदि defined(__MIPSEB__) || defined(_MIPSEB)
          यदि (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((वर्ण *) argp) + परिमाण (भिन्न);
          अन्यथा
#यदि_अन्तः
            avaluep[i] = (वर्ण *) argp;
        }
      अन्यथा यदि (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (वर्ण *) argp;
        }
      अन्यथा यदि (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (वर्ण *) argp;
        }
      अन्यथा यदि (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          यदि (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  अन्यथा
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (वर्ण *) argp;
        }
      अन्यथा
        {
          अचिह्नित type = arg_types[i]->type;

          यदि (arg_types[i]->alignment > परिमाण(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / परिमाण(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          यदि (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  यदि (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          विकल्प (type)
            {
            पक्ष FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              भङ्ग;

            पक्ष FFI_TYPE_STRUCT:
              यदि (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->आकार);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  भङ्ग;
                }
              /* Else fall through.  */
            स्वभाव:
              avaluep[i] = (वर्ण *) argp;
              भङ्ग;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->आकार, परिमाण(ffi_arg)) / परिमाण(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_दत्त);

  प्रत्यागम cif->flags >> (FFI_FLAG_द्वयङ्काः * 8);
}

#यदि_अन्तः /* FFI_MIPS_N32 */

#यदि defined(FFI_MIPS_O32)
बाह्य शून्य ffi_संवृतिः_O32(शून्य);
बाह्य शून्य ffi_go_संवृतिः_O32(शून्य);
#अन्यथा
बाह्य शून्य ffi_संवृतिः_N32(शून्य);
बाह्य शून्य ffi_go_संवृतिः_N32(शून्य);
#यदि_अन्तः /* FFI_MIPS_O32 */

ffi_स्थितिः
ffi_prep_go_संवृतिः (ffi_go_संवृतिः* संवृतिः, ffi_cif* cif,
		     शून्य (*fun)(ffi_cif*,शून्य*,शून्य**,शून्य*))
{
  शून्य * fn;

#यदि defined(FFI_MIPS_O32)
  यदि (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
  fn = ffi_go_संवृतिः_O32;
#अन्यथा
#यदि _MIPS_SIM ==_ABIN32
  यदि (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
#अन्यथा
  यदि (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    प्रत्यागम FFI_BAD_ABI;
#यदि_अन्तः
  fn = ffi_go_संवृतिः_N32;
#यदि_अन्तः /* FFI_MIPS_O32 */

  संवृतिः->tramp = (शून्य *)fn;
  संवृतिः->cif = cif;
  संवृतिः->fun = fun;

  प्रत्यागम FFI_OK;
}

#यदि_अन्तः /* FFI_CLOSURES */

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

#شامل <ffi.h>
#شامل <ffi_common.h>

#شامل <stdint.h>
#شامل <stdlib.h>
#شامل <stdio.h>

#اگر_تعریف_شدہ __GNUC__
#  اگر (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#اگر_ختم

#اگر_غیر_تعریف_شدہ USE__BUILTIN___CLEAR_CACHE
#  اگر defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ورنہ
#    include <sys/cachectl.h>
#  endif
#اگر_ختم

#اگر_تعریف_شدہ FFI_DEBUG
# define FFI_MIPS_STOP_یہاں() ffi_stop_یہاں()
#ورنہ
# define FFI_MIPS_STOP_یہاں() کرو {} جب(0)
#اگر_ختم

#اگر_تعریف_شدہ FFI_MIPS_N32
#تعریف_کرو FIX_ARGP \
FFI_ASSERT(argp <= &ڈھیر[بائٹیں]); \
اگر (argp == &ڈھیر[بائٹیں]) \
{ \
  argp = ڈھیر; \
  FFI_MIPS_STOP_یہاں(); \
}
#ورنہ
#تعریف_کرو FIX_ARGP 
#اگر_ختم


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ساکن خالی ffi_prep_دلائل(حرف *ڈھیر, 
			  extended_cif *ecif,
			  صحیح بائٹیں,
			  صحیح flags)
{
  صحیح i;
  خالی **p_argv;
  حرف *argp, *argp_f;
  ffi_type **p_arg;

  memset(ڈھیر, 0, بائٹیں);

#اگر_تعریف_شدہ FFI_MIPS_N32
  صحیح soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  اگر (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      اگر (بائٹیں + 16 > 8 * حجم(ffi_arg))
        argp = &ڈھیر[بائٹیں - (8 * حجم(ffi_arg))];
      ورنہ
        argp = ڈھیر;
      * (غیرعلامتی طویل *) argp = (غیرعلامتی طویل) ecif->rvalue;
      argp += 16;
    }
  ورنہ
    {
      اگر (بائٹیں > 8 * حجم(ffi_arg))
        argp = &ڈھیر[بائٹیں - (8 * حجم(ffi_arg))];
      ورنہ
        argp = ڈھیر;
    }
#ورنہ
  argp = ڈھیر;
#اگر_ختم

  argp_f = argp;

#اگر_تعریف_شدہ FFI_MIPS_N32
  اگر ( ecif->cif->rstruct_flag != 0 )
#ورنہ
  اگر ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#اگر_ختم  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += حجم(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  برائے (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      غیرعلامتی صحیح a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      اگر (a < حجم(ffi_arg))
        a = حجم(ffi_arg);
      
      اگر ((a - 1) & (غیرعلامتی طویل) argp)
	{
	  argp = (حرف *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->سائز;
      اگر (z <= حجم(ffi_arg))
	{
          صحیح type = (*p_arg)->type;
	  z = حجم(ffi_arg);

          /* The size of a pointer depends on the ABI */
          اگر (type == FFI_TYPE_اشارہ_گر)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	اگر (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    انتخاب (type)
	      {
	      حالت FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		توڑو;
	      حالت FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		توڑو;
	      طےشدہ:
		توڑو;
	      }
	  }
	  انتخاب (type)
	    {
	      حالت FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		توڑو;

	      حالت FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		توڑو;
		  
	      حالت FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		توڑو;
		  
	      حالت FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		توڑو;
		  
	      حالت FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		توڑو;
		  
	      حالت FFI_TYPE_UINT32:
#اگر_تعریف_شدہ FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ورنہ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#اگر_ختم
		توڑو;

#اگر_تعریف_شدہ FFI_MIPS_N32
	      حالت FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        اگر(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(اعشاری *) argp = *(اعشاری *)(* p_argv);
		    argp += z;
		    حرف *tmp = (خالی *) (*p_argv);
		    *(اعشاری *) argp = *(اعشاری *)(tmp+4);
		  }
		ورنہ
		  حافظہ_نقل(argp, *p_argv, (*p_arg)->سائز);
		توڑو;
#اگر_ختم
	      /* This can only happen with 64bit slots.  */
	      حالت FFI_TYPE_FLOAT:
		*(اعشاری *) argp = *(اعشاری *)(* p_argv);
		توڑو;

	      /* Handle structures.  */
	      طےشدہ:
		حافظہ_نقل(argp, *p_argv, (*p_arg)->سائز);
		توڑو;
	    }
	}
      ورنہ
	{
#اگر_تعریف_شدہ FFI_MIPS_O32
	  حافظہ_نقل(argp, *p_argv, z);
#ورنہ
	  {
	    غیرعلامتی طویل انتہا = (غیرعلامتی طویل) argp + z;
	    غیرعلامتی طویل cap = (غیرعلامتی طویل) ڈھیر + بائٹیں;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    اگر (انتہا <= cap)
	      حافظہ_نقل(argp, *p_argv, z);
	    ورنہ
	      {
		غیرعلامتی طویل portion = cap - (غیرعلامتی طویل)argp;

		حافظہ_نقل(argp, *p_argv, portion);
		argp = ڈھیر;
                z -= portion;
		حافظہ_نقل(argp, (خالی*)((غیرعلامتی طویل)(*p_argv) + portion),
                       z);
	      }
	  }
#اگر_ختم
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#اگر_تعریف_شدہ FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

ساکن صحیح
calc_n32_struct_flags_element(غیرعلامتی *flags, ffi_type *e,
			      غیرعلامتی *loc, غیرعلامتی *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  اگر (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      اگر (*arg_reg > 7)
	واپس 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_بٹ));
    }
  *loc += e->سائز;
  واپس 0;
}

ساکن غیرعلامتی
calc_n32_struct_flags(صحیح soft_float, ffi_type *arg,
		      غیرعلامتی *loc, غیرعلامتی *arg_reg)
{
  غیرعلامتی flags = 0;
  غیرعلامتی اشاریہ = 0;

  ffi_type *e;

  اگر (soft_float)
    واپس 0;

  جب ((e = arg->elements[اشاریہ]))
    {
      اگر (e->type == FFI_TYPE_COMPLEX)
	{
	  اگر (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    توڑو;
	  اگر (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    توڑو;
	}
      ورنہ
	اگر (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  توڑو;
      اشاریہ++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  واپس flags;
}

ساکن غیرعلامتی
calc_n32_return_struct_flags(صحیح soft_float, ffi_type *arg)
{
  غیرعلامتی flags;
  غیرعلامتی small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  اگر (arg->سائز > 16)
    واپس 0;

  اگر (arg->سائز > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  اگر (e->type == FFI_TYPE_COMPLEX)
    {
      صحیح type = e->elements[0]->type;

      اگر (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	واپس small;

      اگر (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  واپس small;
	}

      flags = (type << FFI_FLAG_بٹ) + type;
    }
  ورنہ
    {
      اگر (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	واپس small;

      flags = e->type;

      اگر (arg->elements[1])
	{
	  e = arg->elements[1];
	  اگر (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    واپس small;

	  اگر (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      واپس small;
	    }

	  flags += e->type << FFI_FLAG_بٹ;
	}
    }

  اگر (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  واپس flags;
}

#اگر_ختم

/* Perform machine dependent cif processing */
ساکن ffi_حالت ffi_prep_cif_machdep_int(ffi_cif *cif, غیرعلامتی nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#اگر_تعریف_شدہ FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  اگر (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      اگر (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  انتخاب ((cif->arg_types)[0]->type)
	    {
	    حالت FFI_TYPE_FLOAT:
	    حالت FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      توڑو;
	      
	    طےشدہ:
	      توڑو;
	    }

	  اگر (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      اگر (cif->flags)
		{
		  انتخاب ((cif->arg_types)[1]->type)
		    {
		    حالت FFI_TYPE_FLOAT:
		    حالت FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_بٹ;
		      توڑو;
		      
		    طےشدہ:
		      توڑو;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  اگر (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_VOID:
        حالت FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_بٹ * 2);
          توڑو;

        حالت FFI_TYPE_SINT64:
        حالت FFI_TYPE_UINT64:
        حالت FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٹ * 2);
          توڑو;
      
        حالت FFI_TYPE_FLOAT:
        طےشدہ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_بٹ * 2);
          توڑو;
        }
    }
  ورنہ
    {
      /* FFI_O32 */      
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_VOID:
        حالت FFI_TYPE_STRUCT:
        حالت FFI_TYPE_FLOAT:
        حالت FFI_TYPE_DOUBLE:
        حالت FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_بٹ * 2);
	  اگر (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_بٹ * 4);
          توڑو;

        حالت FFI_TYPE_SINT64:
        حالت FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٹ * 2);
          توڑو;
      
        طےشدہ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_بٹ * 2);
          توڑو;
        }
    }
#اگر_ختم

#اگر_تعریف_شدہ FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    غیرعلامتی arg_reg = 0;
    غیرعلامتی loc = 0;
    غیرعلامتی گنتی = (cif->nargs < 8) ? cif->nargs : 8;
    غیرعلامتی اشاریہ = 0;

    غیرعلامتی struct_flags = 0;
    صحیح soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    اگر (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	اگر (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    گنتی = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ورنہ
	    cif->rstruct_flag = 0;
      }
    ورنہ
      cif->rstruct_flag = 0;

    جب (گنتی-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[اشاریہ];

	انتخاب (t->type)
	  {
	  حالت FFI_TYPE_FLOAT:
	  حالت FFI_TYPE_DOUBLE:
	    اگر (!soft_float && اشاریہ < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_بٹ);
	    arg_reg++;
	    توڑو;
          حالت FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    اگر (soft_float || اشاریہ >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ورنہ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٹ));
		arg_reg++;
		اگر (arg_reg >= 8)
		  جاری;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٹ));
		arg_reg++;
	      }
            توڑو;

	  حالت FFI_TYPE_COMPLEX:
	    انتخاب (t->elements[0]->type)
	      {
	      حالت FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		اگر (soft_float || اشاریہ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ورنہ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٹ));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        جاری;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٹ));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        جاری;
		  }
		/* passthrough */
	      حالت FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->بائٹیں += 16;
		/* passthrough */
	      حالت FFI_TYPE_SINT32:
	      حالت FFI_TYPE_UINT32:
	      حالت FFI_TYPE_DOUBLE:
		اگر (soft_float || اشاریہ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ورنہ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_بٹ));
		    arg_reg++;
		    اگر (arg_reg >= 8)
		        جاری;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_بٹ));
		    arg_reg++;
		  }
		توڑو;
	      طےشدہ:
		arg_reg += 2;
		توڑو;
	      }
	    توڑو;

	  حالت FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || اشاریہ >= nfixedargs,
						t, &loc, &arg_reg);
	    توڑو;

	  طےشدہ:
	    arg_reg++;
            توڑو;
	  }

	اشاریہ++;
      }

  /* Set the return type flag */
    انتخاب (cif->rtype->type)
      {
      حالت FFI_TYPE_STRUCT:
	{
	  اگر (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ورنہ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_بٹ * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_بٹ * 8));
	    }
	  توڑو;
	}
      
      حالت FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	توڑو;

      حالت FFI_TYPE_اشارہ_گر:
	اگر (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٹ * 8);
	ورنہ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٹ * 8);
	توڑو;

      حالت FFI_TYPE_FLOAT:
	اگر (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٹ * 8);
	    توڑو;
	  }
	/* else fall through */
      حالت FFI_TYPE_DOUBLE:
	اگر (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٹ * 8);
	ورنہ
	  cif->flags += cif->rtype->type << (FFI_FLAG_بٹ * 8);
	توڑو;

      حالت FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	اگر (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_بٹ * 8);
 	  }
	ورنہ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_بٹ * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_بٹ))
					      << (4 + (FFI_FLAG_بٹ * 8));
	  }
	توڑو;
      حالت FFI_TYPE_COMPLEX:
	{
	  صحیح type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_بٹ * 8));
	  اگر (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      انتخاب (type)
		{
		حالت FFI_TYPE_DOUBLE:
		حالت FFI_TYPE_SINT64:
		حالت FFI_TYPE_UINT64:
		حالت FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  توڑو;
		حالت FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  توڑو;
		حالت FFI_TYPE_FLOAT:
		طےشدہ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_بٹ * 8));
	    }
	  ورنہ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_بٹ * 8));
	    }
	  توڑو;
	}
      حالت FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٹ * 8);
	توڑو;
      حالت FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٹ * 8);
	توڑو;
      طےشدہ:
	cif->flags += cif->rtype->type << (FFI_FLAG_بٹ * 8);
	توڑو;
      }
  }
#اگر_ختم
  واپس FFI_OK;
}

ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    واپس ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    غیرعلامتی nfixedargs,
                                    غیرعلامتی ntotalargs MAYBE_غیر_مستعمل)
{
    واپس ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
خارجی صحیح ffi_call_O32(خالی (*)(حرف *, extended_cif *, صحیح, صحیح), 
			extended_cif *, غیرعلامتی, 
			غیرعلامتی, غیرعلامتی *, خالی (*)(خالی), خالی *احاطہ);

/* Low level routine for calling N32 functions */
خارجی صحیح ffi_call_N32(خالی (*)(حرف *, extended_cif *, صحیح, صحیح), 
			extended_cif *, غیرعلامتی, 
			غیرعلامتی, خالی *, خالی (*)(خالی), خالی *احاطہ);

خالی ffi_call_int(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, 
	      خالی **avalue, خالی *احاطہ)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  اگر ((rvalue == تہی) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->سائز);
  ورنہ
    ecif.rvalue = rvalue;
    
  انتخاب (cif->abi) 
    {
#اگر_تعریف_شدہ FFI_MIPS_O32
    حالت FFI_O32:
    حالت FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_دلائل, &ecif, cif->بائٹیں, 
		   cif->flags, ecif.rvalue, fn, احاطہ);
      توڑو;
#اگر_ختم

#اگر_تعریف_شدہ FFI_MIPS_N32
    حالت FFI_N32:
    حالت FFI_N32_SOFT_FLOAT:
    حالت FFI_N64:
    حالت FFI_N64_SOFT_FLOAT:
      {
        صحیح copy_rvalue = 0;
	صحیح copy_انحراف = 0;
        حرف *rvalue_copy = ecif.rvalue;
        اگر (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->سائز < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ورنہ اگر (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_انحراف = 4;
#اگر_ختم
	  }
        ffi_call_N32(ffi_prep_دلائل, &ecif, cif->بائٹیں,
                     cif->flags, rvalue_copy, fn, احاطہ);
        اگر (copy_rvalue)
          حافظہ_نقل(ecif.rvalue, rvalue_copy + copy_انحراف, cif->rtype->سائز);
      }
      توڑو;
#اگر_ختم

    طےشدہ:
      FFI_ASSERT(0);
      توڑو;
    }
}

خالی
ffi_call(ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue, خالی **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, تہی);
}

خالی
ffi_call_go (ffi_cif *cif, خالی (*fn)(خالی), خالی *rvalue,
	     خالی **avalue, خالی *احاطہ)
{
  ffi_call_int (cif, fn, rvalue, avalue, احاطہ);
}


#اگر FFI_CLOSURES
#اگر defined(FFI_MIPS_O32)
خارجی خالی ffi_احاطہ_O32(خالی);
خارجی خالی ffi_go_احاطہ_O32(خالی);
#ورنہ
خارجی خالی ffi_احاطہ_N32(خالی);
خارجی خالی ffi_go_احاطہ_N32(خالی);
#اگر_ختم /* FFI_MIPS_O32 */

ffi_حالت
ffi_prep_احاطہ_loc (ffi_احاطہ *احاطہ,
		      ffi_cif *cif,
		      خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*),
		      خالی *user_ڈیٹا,
		      خالی *codeloc)
{
  غیرعلامتی صحیح *tramp = (غیرعلامتی صحیح *) &احاطہ->tramp[0];
  خالی * fn;
  حرف *clear_location = (حرف *) codeloc;

#اگر defined(FFI_MIPS_O32)
  اگر (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
  fn = ffi_احاطہ_O32;
#ورنہ
#اگر _MIPS_SIM ==_ABIN32
  اگر (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
#ورنہ
  اگر (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
#اگر_ختم
  fn = ffi_احاطہ_N32;
#اگر_ختم /* FFI_MIPS_O32 */

#اگر defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((غیرعلامتی)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((غیرعلامتی)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((غیرعلامتی)codeloc >> 16);
  /* jr   $25          */
#اگر !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ورنہ
  tramp[3] = 0x03200009;
#اگر_ختم
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((غیرعلامتی)codeloc & 0xffff);
#ورنہ
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((غیرعلامتی طویل)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((غیرعلامتی طویل)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((غیرعلامتی طویل)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((غیرعلامتی طویل)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((غیرعلامتی طویل)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((غیرعلامتی طویل)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((غیرعلامتی طویل)fn  & 0xffff);
  /* jr   $25          */
#اگر !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ورنہ
  tramp[11] = 0x03200009;
#اگر_ختم
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((غیرعلامتی طویل)codeloc & 0xffff);

#اگر_ختم

  احاطہ->cif = cif;
  احاطہ->fun = fun;
  احاطہ->user_ڈیٹا = user_ڈیٹا;

#اگر !defined(__FreeBSD__)
#اگر_تعریف_شدہ USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_سائز);
#ورنہ
  cacheflush (clear_location, FFI_TRAMPOLINE_سائز, ICACHE);
#اگر_ختم
#اگر_ختم /* ! __FreeBSD__ */
  واپس FFI_OK;
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
صحیح
ffi_احاطہ_mips_inner_O32 (ffi_cif *cif,
                            خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
			    خالی *user_ڈیٹا,
			    خالی *rvalue, ffi_arg *ar,
			    دوہرا *fpr)
{
  خالی **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحیح i, avn, argn, نظر_آیا_int;

  avalue = alloca (cif->nargs * حجم (ffi_arg));
  avaluep = alloca (cif->nargs * حجم (ffi_arg));

  نظر_آیا_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  اگر ((cif->flags >> (FFI_FLAG_بٹ * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (خالی *)(uintptr_t)ar[0];
      argn = 1;
      نظر_آیا_int = 1;
    }
  اگر ((cif->flags >> (FFI_FLAG_بٹ * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  جب (i < avn)
    {
      اگر (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      اگر (i < 2 && !نظر_آیا_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
	  اگر (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((حرف *) &fpr[i]) + حجم (اعشاری);
	  ورنہ
#اگر_ختم
	    avaluep[i] = (حرف *) &fpr[i];
	}
      ورنہ
	{
	  انتخاب (arg_types[i]->type)
	    {
	      حالت FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		توڑو;

	      حالت FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		توڑو;
		  
	      حالت FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		توڑو;
		  
	      حالت FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		توڑو;

	      طےشدہ:
		avaluep[i] = (حرف *) &ar[argn];
		توڑو;
	    }
	  نظر_آیا_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->سائز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_ڈیٹا);

  اگر (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      انتخاب (cif->rtype->type)
        {
        حالت FFI_TYPE_FLOAT:
          واپس FFI_TYPE_INT;
        حالت FFI_TYPE_DOUBLE:
          واپس FFI_TYPE_UINT64;
        طےشدہ:
          واپس cif->rtype->type;
        }
    }
  ورنہ
    {
      اگر (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ متلون ("move $v1, %0" : : "r"(cif->rtype->سائز));
      }
      واپس cif->rtype->type;
    }
}

#اگر defined(FFI_MIPS_N32)

ساکن خالی
copy_struct_N32(حرف *ہدف_2, غیرعلامتی offset, ffi_abi abi, ffi_type *type,
                صحیح argn, غیرعلامتی arg_انحراف, ffi_arg *ar,
                ffi_arg *fpr, صحیح soft_float)
{
  ffi_type **elt_typep = type->elements;
  جب(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      غیرعلامتی o;
      حرف *tp;
      حرف *argp;
      حرف *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_انحراف += o - offset;
      offset = o;
      argn += arg_انحراف / حجم(ffi_arg);
      arg_انحراف = arg_انحراف % حجم(ffi_arg);

      argp = (حرف *)(ar + argn);
      fpp = (حرف *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = ہدف_2 + offset;

      اگر (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(دوہرا *)tp = *(دوہرا *)fpp;
      ورنہ
        حافظہ_نقل(tp, argp + arg_انحراف, elt_type->سائز);

      offset += elt_type->سائز;
      arg_انحراف += elt_type->سائز;
      elt_typep++;
      argn += arg_انحراف / حجم(ffi_arg);
      arg_انحراف = arg_انحراف % حجم(ffi_arg);
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
صحیح
ffi_احاطہ_mips_inner_N32 (ffi_cif *cif, 
			    خالی (*fun)(ffi_cif*, خالی*, خالی**, خالی*),
                            خالی *user_ڈیٹا,
			    خالی *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  خالی **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحیح i, avn, argn;
  صحیح soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * حجم (ffi_arg));
  avaluep = alloca (cif->nargs * حجم (ffi_arg));

  argn = 0;

  اگر (cif->rstruct_flag)
    {
#اگر _MIPS_SIM==_ABIN32
      rvalue = (خالی *)(UINT32)ar[0];
#ورنہ /* N64 */
      rvalue = (خالی *)ar[0];
#اگر_ختم
      argn = 1;
    }
  اگر (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  جب (i < avn)
    {
      اگر (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          اگر ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#اگر defined(__MIPSEB__) || defined(_MIPSEB)
          اگر (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((حرف *) argp) + حجم (اعشاری);
          ورنہ
#اگر_ختم
            avaluep[i] = (حرف *) argp;
        }
      ورنہ اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (حرف *) argp;
        }
      ورنہ اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (حرف *) argp;
        }
      ورنہ اگر (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          اگر (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ورنہ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (حرف *) argp;
        }
      ورنہ
        {
          غیرعلامتی type = arg_types[i]->type;

          اگر (arg_types[i]->alignment > حجم(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / حجم(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          اگر (type == FFI_TYPE_اشارہ_گر)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  اگر (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          انتخاب (type)
            {
            حالت FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              توڑو;

            حالت FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              توڑو;

            حالت FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              توڑو;

            حالت FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              توڑو;

            حالت FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              توڑو;

            حالت FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              توڑو;

            حالت FFI_TYPE_STRUCT:
              اگر (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->سائز);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  توڑو;
                }
              /* Else fall through.  */
            طےشدہ:
              avaluep[i] = (حرف *) argp;
              توڑو;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->سائز, حجم(ffi_arg)) / حجم(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_ڈیٹا);

  واپس cif->flags >> (FFI_FLAG_بٹ * 8);
}

#اگر_ختم /* FFI_MIPS_N32 */

#اگر defined(FFI_MIPS_O32)
خارجی خالی ffi_احاطہ_O32(خالی);
خارجی خالی ffi_go_احاطہ_O32(خالی);
#ورنہ
خارجی خالی ffi_احاطہ_N32(خالی);
خارجی خالی ffi_go_احاطہ_N32(خالی);
#اگر_ختم /* FFI_MIPS_O32 */

ffi_حالت
ffi_prep_go_احاطہ (ffi_go_احاطہ* احاطہ, ffi_cif* cif,
		     خالی (*fun)(ffi_cif*,خالی*,خالی**,خالی*))
{
  خالی * fn;

#اگر defined(FFI_MIPS_O32)
  اگر (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
  fn = ffi_go_احاطہ_O32;
#ورنہ
#اگر _MIPS_SIM ==_ABIN32
  اگر (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
#ورنہ
  اگر (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    واپس FFI_BAD_ABI;
#اگر_ختم
  fn = ffi_go_احاطہ_N32;
#اگر_ختم /* FFI_MIPS_O32 */

  احاطہ->tramp = (خالی *)fn;
  احاطہ->cif = cif;
  احاطہ->fun = fun;

  واپس FFI_OK;
}

#اگر_ختم /* FFI_CLOSURES */

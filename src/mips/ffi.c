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

#ضمن <ffi.h>
#ضمن <ffi_common.h>

#ضمن <stdint.h>
#ضمن <stdlib.h>
#ضمن <stdio.h>

#جيڪڏهن_وصف_ٿيل __GNUC__
#  إذا (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_نه_ٿيل USE__BUILTIN___CLEAR_CACHE
#  إذا defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  وإلا
#    include <sys/cachectl.h>
#  endif
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FFI_DEBUG
# define FFI_MIPS_STOP_هتي() ffi_stop_هتي()
#وإلا
# define FFI_MIPS_STOP_هتي() افعل {} طالما(0)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
#وصف_ڪريو FIX_ARGP \
FFI_ASSERT(argp <= &ڍير[بائيٽون]); \
إذا (argp == &ڍير[بائيٽون]) \
{ \
  argp = ڍير; \
  FFI_MIPS_STOP_هتي(); \
}
#وإلا
#وصف_ڪريو FIX_ARGP 
#جيڪڏهن_پورو


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ساكن فراغ ffi_prep_دليلون(حرف *ڍير, 
			  extended_cif *ecif,
			  صحيح بائيٽون,
			  صحيح flags)
{
  صحيح i;
  فراغ **p_argv;
  حرف *argp, *argp_f;
  ffi_type **p_arg;

  memset(ڍير, 0, بائيٽون);

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
  صحيح soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  إذا (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      إذا (بائيٽون + 16 > 8 * حجم(ffi_arg))
        argp = &ڍير[بائيٽون - (8 * حجم(ffi_arg))];
      وإلا
        argp = ڍير;
      * (بنانشان ڊگهو *) argp = (بنانشان ڊگهو) ecif->rvalue;
      argp += 16;
    }
  وإلا
    {
      إذا (بائيٽون > 8 * حجم(ffi_arg))
        argp = &ڍير[بائيٽون - (8 * حجم(ffi_arg))];
      وإلا
        argp = ڍير;
    }
#وإلا
  argp = ڍير;
#جيڪڏهن_پورو

  argp_f = argp;

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
  إذا ( ecif->cif->rstruct_flag != 0 )
#وإلا
  إذا ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#جيڪڏهن_پورو  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += حجم(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  لكل (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      بنانشان صحيح a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      إذا (a < حجم(ffi_arg))
        a = حجم(ffi_arg);
      
      إذا ((a - 1) & (بنانشان ڊگهو) argp)
	{
	  argp = (حرف *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->سائيز;
      إذا (z <= حجم(ffi_arg))
	{
          صحيح type = (*p_arg)->type;
	  z = حجم(ffi_arg);

          /* The size of a pointer depends on the ABI */
          إذا (type == FFI_TYPE_اشارو)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	إذا (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    بدل (type)
	      {
	      حالة FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		اكسر;
	      حالة FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		اكسر;
	      افتراضي:
		اكسر;
	      }
	  }
	  بدل (type)
	    {
	      حالة FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		اكسر;

	      حالة FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		اكسر;
		  
	      حالة FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		اكسر;
		  
	      حالة FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		اكسر;
		  
	      حالة FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		اكسر;
		  
	      حالة FFI_TYPE_UINT32:
#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#وإلا
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#جيڪڏهن_پورو
		اكسر;

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
	      حالة FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        إذا(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(عائم *) argp = *(عائم *)(* p_argv);
		    argp += z;
		    حرف *tmp = (فراغ *) (*p_argv);
		    *(عائم *) argp = *(عائم *)(tmp+4);
		  }
		وإلا
		  يادگيري_نقل(argp, *p_argv, (*p_arg)->سائيز);
		اكسر;
#جيڪڏهن_پورو
	      /* This can only happen with 64bit slots.  */
	      حالة FFI_TYPE_FLOAT:
		*(عائم *) argp = *(عائم *)(* p_argv);
		اكسر;

	      /* Handle structures.  */
	      افتراضي:
		يادگيري_نقل(argp, *p_argv, (*p_arg)->سائيز);
		اكسر;
	    }
	}
      وإلا
	{
#جيڪڏهن_وصف_ٿيل FFI_MIPS_O32
	  يادگيري_نقل(argp, *p_argv, z);
#وإلا
	  {
	    بنانشان ڊگهو پڇاڙي = (بنانشان ڊگهو) argp + z;
	    بنانشان ڊگهو cap = (بنانشان ڊگهو) ڍير + بائيٽون;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    إذا (پڇاڙي <= cap)
	      يادگيري_نقل(argp, *p_argv, z);
	    وإلا
	      {
		بنانشان ڊگهو portion = cap - (بنانشان ڊگهو)argp;

		يادگيري_نقل(argp, *p_argv, portion);
		argp = ڍير;
                z -= portion;
		يادگيري_نقل(argp, (فراغ*)((بنانشان ڊگهو)(*p_argv) + portion),
                       z);
	      }
	  }
#جيڪڏهن_پورو
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

ساكن صحيح
calc_n32_struct_flags_element(بنانشان *flags, ffi_type *e,
			      بنانشان *loc, بنانشان *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  إذا (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      إذا (*arg_reg > 7)
	أرجع 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_بٽ));
    }
  *loc += e->سائيز;
  أرجع 0;
}

ساكن بنانشان
calc_n32_struct_flags(صحيح soft_float, ffi_type *arg,
		      بنانشان *loc, بنانشان *arg_reg)
{
  بنانشان flags = 0;
  بنانشان اشاريو = 0;

  ffi_type *e;

  إذا (soft_float)
    أرجع 0;

  طالما ((e = arg->elements[اشاريو]))
    {
      إذا (e->type == FFI_TYPE_COMPLEX)
	{
	  إذا (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    اكسر;
	  إذا (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    اكسر;
	}
      وإلا
	إذا (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  اكسر;
      اشاريو++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  أرجع flags;
}

ساكن بنانشان
calc_n32_return_struct_flags(صحيح soft_float, ffi_type *arg)
{
  بنانشان flags;
  بنانشان small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  إذا (arg->سائيز > 16)
    أرجع 0;

  إذا (arg->سائيز > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  إذا (e->type == FFI_TYPE_COMPLEX)
    {
      صحيح type = e->elements[0]->type;

      إذا (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	أرجع small;

      إذا (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  أرجع small;
	}

      flags = (type << FFI_FLAG_بٽ) + type;
    }
  وإلا
    {
      إذا (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	أرجع small;

      flags = e->type;

      إذا (arg->elements[1])
	{
	  e = arg->elements[1];
	  إذا (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    أرجع small;

	  إذا (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      أرجع small;
	    }

	  flags += e->type << FFI_FLAG_بٽ;
	}
    }

  إذا (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  أرجع flags;
}

#جيڪڏهن_پورو

/* Perform machine dependent cif processing */
ساكن ffi_حالت ffi_prep_cif_machdep_int(ffi_cif *cif, بنانشان nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#جيڪڏهن_وصف_ٿيل FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  إذا (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      إذا (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  بدل ((cif->arg_types)[0]->type)
	    {
	    حالة FFI_TYPE_FLOAT:
	    حالة FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      اكسر;
	      
	    افتراضي:
	      اكسر;
	    }

	  إذا (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      إذا (cif->flags)
		{
		  بدل ((cif->arg_types)[1]->type)
		    {
		    حالة FFI_TYPE_FLOAT:
		    حالة FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_بٽ;
		      اكسر;
		      
		    افتراضي:
		      اكسر;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  إذا (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      بدل (cif->rtype->type)
        {
        حالة FFI_TYPE_VOID:
        حالة FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_بٽ * 2);
          اكسر;

        حالة FFI_TYPE_SINT64:
        حالة FFI_TYPE_UINT64:
        حالة FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٽ * 2);
          اكسر;
      
        حالة FFI_TYPE_FLOAT:
        افتراضي:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_بٽ * 2);
          اكسر;
        }
    }
  وإلا
    {
      /* FFI_O32 */      
      بدل (cif->rtype->type)
        {
        حالة FFI_TYPE_VOID:
        حالة FFI_TYPE_STRUCT:
        حالة FFI_TYPE_FLOAT:
        حالة FFI_TYPE_DOUBLE:
        حالة FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_بٽ * 2);
	  إذا (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_بٽ * 4);
          اكسر;

        حالة FFI_TYPE_SINT64:
        حالة FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٽ * 2);
          اكسر;
      
        افتراضي:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_بٽ * 2);
          اكسر;
        }
    }
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    بنانشان arg_reg = 0;
    بنانشان loc = 0;
    بنانشان ڳڻپ = (cif->nargs < 8) ? cif->nargs : 8;
    بنانشان اشاريو = 0;

    بنانشان struct_flags = 0;
    صحيح soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    إذا (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	إذا (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ڳڻپ = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	وإلا
	    cif->rstruct_flag = 0;
      }
    وإلا
      cif->rstruct_flag = 0;

    طالما (ڳڻپ-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[اشاريو];

	بدل (t->type)
	  {
	  حالة FFI_TYPE_FLOAT:
	  حالة FFI_TYPE_DOUBLE:
	    إذا (!soft_float && اشاريو < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_بٽ);
	    arg_reg++;
	    اكسر;
          حالة FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    إذا (soft_float || اشاريو >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    وإلا
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٽ));
		arg_reg++;
		إذا (arg_reg >= 8)
		  استمر;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٽ));
		arg_reg++;
	      }
            اكسر;

	  حالة FFI_TYPE_COMPLEX:
	    بدل (t->elements[0]->type)
	      {
	      حالة FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		إذا (soft_float || اشاريو >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		وإلا
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٽ));
		    arg_reg++;
		    إذا (arg_reg >= 8)
		        استمر;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_بٽ));
		    arg_reg++;
		    إذا (arg_reg >= 8)
		        استمر;
		  }
		/* passthrough */
	      حالة FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->بائيٽون += 16;
		/* passthrough */
	      حالة FFI_TYPE_SINT32:
	      حالة FFI_TYPE_UINT32:
	      حالة FFI_TYPE_DOUBLE:
		إذا (soft_float || اشاريو >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		وإلا
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_بٽ));
		    arg_reg++;
		    إذا (arg_reg >= 8)
		        استمر;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_بٽ));
		    arg_reg++;
		  }
		اكسر;
	      افتراضي:
		arg_reg += 2;
		اكسر;
	      }
	    اكسر;

	  حالة FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || اشاريو >= nfixedargs,
						t, &loc, &arg_reg);
	    اكسر;

	  افتراضي:
	    arg_reg++;
            اكسر;
	  }

	اشاريو++;
      }

  /* Set the return type flag */
    بدل (cif->rtype->type)
      {
      حالة FFI_TYPE_STRUCT:
	{
	  إذا (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  وإلا
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_بٽ * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_بٽ * 8));
	    }
	  اكسر;
	}
      
      حالة FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	اكسر;

      حالة FFI_TYPE_اشارو:
	إذا (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٽ * 8);
	وإلا
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٽ * 8);
	اكسر;

      حالة FFI_TYPE_FLOAT:
	إذا (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٽ * 8);
	    اكسر;
	  }
	/* else fall through */
      حالة FFI_TYPE_DOUBLE:
	إذا (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٽ * 8);
	وإلا
	  cif->flags += cif->rtype->type << (FFI_FLAG_بٽ * 8);
	اكسر;

      حالة FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	إذا (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_بٽ * 8);
 	  }
	وإلا
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_بٽ * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_بٽ))
					      << (4 + (FFI_FLAG_بٽ * 8));
	  }
	اكسر;
      حالة FFI_TYPE_COMPLEX:
	{
	  صحيح type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_بٽ * 8));
	  إذا (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      بدل (type)
		{
		حالة FFI_TYPE_DOUBLE:
		حالة FFI_TYPE_SINT64:
		حالة FFI_TYPE_UINT64:
		حالة FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  اكسر;
		حالة FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  اكسر;
		حالة FFI_TYPE_FLOAT:
		افتراضي:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_بٽ * 8));
	    }
	  وإلا
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_بٽ * 8));
	    }
	  اكسر;
	}
      حالة FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_بٽ * 8);
	اكسر;
      حالة FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_بٽ * 8);
	اكسر;
      افتراضي:
	cif->flags += cif->rtype->type << (FFI_FLAG_بٽ * 8);
	اكسر;
      }
  }
#جيڪڏهن_پورو
  أرجع FFI_OK;
}

ffi_حالت ffi_prep_cif_machdep(ffi_cif *cif)
{
    أرجع ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_حالت ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    بنانشان nfixedargs,
                                    بنانشان ntotalargs MAYBE_اڻ_استعمال_ٿيل)
{
    أرجع ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
خارجي صحيح ffi_call_O32(فراغ (*)(حرف *, extended_cif *, صحيح, صحيح), 
			extended_cif *, بنانشان, 
			بنانشان, بنانشان *, فراغ (*)(فراغ), فراغ *گهيرو);

/* Low level routine for calling N32 functions */
خارجي صحيح ffi_call_N32(فراغ (*)(حرف *, extended_cif *, صحيح, صحيح), 
			extended_cif *, بنانشان, 
			بنانشان, فراغ *, فراغ (*)(فراغ), فراغ *گهيرو);

فراغ ffi_call_int(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, 
	      فراغ **avalue, فراغ *گهيرو)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  إذا ((rvalue == عدم) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->سائيز);
  وإلا
    ecif.rvalue = rvalue;
    
  بدل (cif->abi) 
    {
#جيڪڏهن_وصف_ٿيل FFI_MIPS_O32
    حالة FFI_O32:
    حالة FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_دليلون, &ecif, cif->بائيٽون, 
		   cif->flags, ecif.rvalue, fn, گهيرو);
      اكسر;
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل FFI_MIPS_N32
    حالة FFI_N32:
    حالة FFI_N32_SOFT_FLOAT:
    حالة FFI_N64:
    حالة FFI_N64_SOFT_FLOAT:
      {
        صحيح copy_rvalue = 0;
	صحيح copy_وٿي = 0;
        حرف *rvalue_copy = ecif.rvalue;
        إذا (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->سائيز < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	وإلا إذا (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#إذا defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_وٿي = 4;
#جيڪڏهن_پورو
	  }
        ffi_call_N32(ffi_prep_دليلون, &ecif, cif->بائيٽون,
                     cif->flags, rvalue_copy, fn, گهيرو);
        إذا (copy_rvalue)
          يادگيري_نقل(ecif.rvalue, rvalue_copy + copy_وٿي, cif->rtype->سائيز);
      }
      اكسر;
#جيڪڏهن_پورو

    افتراضي:
      FFI_ASSERT(0);
      اكسر;
    }
}

فراغ
ffi_call(ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *گهيرو)
{
  ffi_call_int (cif, fn, rvalue, avalue, گهيرو);
}


#إذا FFI_CLOSURES
#إذا defined(FFI_MIPS_O32)
خارجي فراغ ffi_گهيرو_O32(فراغ);
خارجي فراغ ffi_go_گهيرو_O32(فراغ);
#وإلا
خارجي فراغ ffi_گهيرو_N32(فراغ);
خارجي فراغ ffi_go_گهيرو_N32(فراغ);
#جيڪڏهن_پورو /* FFI_MIPS_O32 */

ffi_حالت
ffi_prep_گهيرو_loc (ffi_گهيرو *گهيرو,
		      ffi_cif *cif,
		      فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
		      فراغ *user_معلومات,
		      فراغ *codeloc)
{
  بنانشان صحيح *tramp = (بنانشان صحيح *) &گهيرو->tramp[0];
  فراغ * fn;
  حرف *clear_location = (حرف *) codeloc;

#إذا defined(FFI_MIPS_O32)
  إذا (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
  fn = ffi_گهيرو_O32;
#وإلا
#إذا _MIPS_SIM ==_ABIN32
  إذا (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
#وإلا
  إذا (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
  fn = ffi_گهيرو_N32;
#جيڪڏهن_پورو /* FFI_MIPS_O32 */

#إذا defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((بنانشان)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((بنانشان)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((بنانشان)codeloc >> 16);
  /* jr   $25          */
#إذا !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#وإلا
  tramp[3] = 0x03200009;
#جيڪڏهن_پورو
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((بنانشان)codeloc & 0xffff);
#وإلا
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((بنانشان ڊگهو)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((بنانشان ڊگهو)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((بنانشان ڊگهو)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((بنانشان ڊگهو)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((بنانشان ڊگهو)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((بنانشان ڊگهو)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((بنانشان ڊگهو)fn  & 0xffff);
  /* jr   $25          */
#إذا !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#وإلا
  tramp[11] = 0x03200009;
#جيڪڏهن_پورو
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((بنانشان ڊگهو)codeloc & 0xffff);

#جيڪڏهن_پورو

  گهيرو->cif = cif;
  گهيرو->fun = fun;
  گهيرو->user_معلومات = user_معلومات;

#إذا !defined(__FreeBSD__)
#جيڪڏهن_وصف_ٿيل USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_سائيز);
#وإلا
  cacheflush (clear_location, FFI_TRAMPOLINE_سائيز, ICACHE);
#جيڪڏهن_پورو
#جيڪڏهن_پورو /* ! __FreeBSD__ */
  أرجع FFI_OK;
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
صحيح
ffi_گهيرو_mips_inner_O32 (ffi_cif *cif,
                            فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
			    فراغ *user_معلومات,
			    فراغ *rvalue, ffi_arg *ar,
			    مزدوج *fpr)
{
  فراغ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحيح i, avn, argn, ڏٺل_int;

  avalue = alloca (cif->nargs * حجم (ffi_arg));
  avaluep = alloca (cif->nargs * حجم (ffi_arg));

  ڏٺل_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  إذا ((cif->flags >> (FFI_FLAG_بٽ * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (فراغ *)(uintptr_t)ar[0];
      argn = 1;
      ڏٺل_int = 1;
    }
  إذا ((cif->flags >> (FFI_FLAG_بٽ * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  طالما (i < avn)
    {
      إذا (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      إذا (i < 2 && !ڏٺل_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#إذا defined(__MIPSEB__) || defined(_MIPSEB)
	  إذا (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((حرف *) &fpr[i]) + حجم (عائم);
	  وإلا
#جيڪڏهن_پورو
	    avaluep[i] = (حرف *) &fpr[i];
	}
      وإلا
	{
	  بدل (arg_types[i]->type)
	    {
	      حالة FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		اكسر;

	      حالة FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		اكسر;
		  
	      حالة FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		اكسر;
		  
	      حالة FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		اكسر;

	      افتراضي:
		avaluep[i] = (حرف *) &ar[argn];
		اكسر;
	    }
	  ڏٺل_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->سائيز, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_معلومات);

  إذا (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      بدل (cif->rtype->type)
        {
        حالة FFI_TYPE_FLOAT:
          أرجع FFI_TYPE_INT;
        حالة FFI_TYPE_DOUBLE:
          أرجع FFI_TYPE_UINT64;
        افتراضي:
          أرجع cif->rtype->type;
        }
    }
  وإلا
    {
      إذا (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ متطاير ("move $v1, %0" : : "r"(cif->rtype->سائيز));
      }
      أرجع cif->rtype->type;
    }
}

#إذا defined(FFI_MIPS_N32)

ساكن فراغ
copy_struct_N32(حرف *هدف_2, بنانشان offset, ffi_abi abi, ffi_type *type,
                صحيح argn, بنانشان arg_وٿي, ffi_arg *ar,
                ffi_arg *fpr, صحيح soft_float)
{
  ffi_type **elt_typep = type->elements;
  طالما(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      بنانشان o;
      حرف *tp;
      حرف *argp;
      حرف *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_وٿي += o - offset;
      offset = o;
      argn += arg_وٿي / حجم(ffi_arg);
      arg_وٿي = arg_وٿي % حجم(ffi_arg);

      argp = (حرف *)(ar + argn);
      fpp = (حرف *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = هدف_2 + offset;

      إذا (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(مزدوج *)tp = *(مزدوج *)fpp;
      وإلا
        يادگيري_نقل(tp, argp + arg_وٿي, elt_type->سائيز);

      offset += elt_type->سائيز;
      arg_وٿي += elt_type->سائيز;
      elt_typep++;
      argn += arg_وٿي / حجم(ffi_arg);
      arg_وٿي = arg_وٿي % حجم(ffi_arg);
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
صحيح
ffi_گهيرو_mips_inner_N32 (ffi_cif *cif, 
			    فراغ (*fun)(ffi_cif*, فراغ*, فراغ**, فراغ*),
                            فراغ *user_معلومات,
			    فراغ *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  فراغ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  صحيح i, avn, argn;
  صحيح soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * حجم (ffi_arg));
  avaluep = alloca (cif->nargs * حجم (ffi_arg));

  argn = 0;

  إذا (cif->rstruct_flag)
    {
#إذا _MIPS_SIM==_ABIN32
      rvalue = (فراغ *)(UINT32)ar[0];
#وإلا /* N64 */
      rvalue = (فراغ *)ar[0];
#جيڪڏهن_پورو
      argn = 1;
    }
  إذا (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  طالما (i < avn)
    {
      إذا (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          إذا ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#إذا defined(__MIPSEB__) || defined(_MIPSEB)
          إذا (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((حرف *) argp) + حجم (عائم);
          وإلا
#جيڪڏهن_پورو
            avaluep[i] = (حرف *) argp;
        }
      وإلا إذا (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (حرف *) argp;
        }
      وإلا إذا (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (حرف *) argp;
        }
      وإلا إذا (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          إذا (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  وإلا
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (حرف *) argp;
        }
      وإلا
        {
          بنانشان type = arg_types[i]->type;

          إذا (arg_types[i]->alignment > حجم(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / حجم(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          إذا (type == FFI_TYPE_اشارو)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  إذا (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          بدل (type)
            {
            حالة FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              اكسر;

            حالة FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              اكسر;

            حالة FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              اكسر;

            حالة FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              اكسر;

            حالة FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              اكسر;

            حالة FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              اكسر;

            حالة FFI_TYPE_STRUCT:
              إذا (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->سائيز);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  اكسر;
                }
              /* Else fall through.  */
            افتراضي:
              avaluep[i] = (حرف *) argp;
              اكسر;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->سائيز, حجم(ffi_arg)) / حجم(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_معلومات);

  أرجع cif->flags >> (FFI_FLAG_بٽ * 8);
}

#جيڪڏهن_پورو /* FFI_MIPS_N32 */

#إذا defined(FFI_MIPS_O32)
خارجي فراغ ffi_گهيرو_O32(فراغ);
خارجي فراغ ffi_go_گهيرو_O32(فراغ);
#وإلا
خارجي فراغ ffi_گهيرو_N32(فراغ);
خارجي فراغ ffi_go_گهيرو_N32(فراغ);
#جيڪڏهن_پورو /* FFI_MIPS_O32 */

ffi_حالت
ffi_prep_go_گهيرو (ffi_go_گهيرو* گهيرو, ffi_cif* cif,
		     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  فراغ * fn;

#إذا defined(FFI_MIPS_O32)
  إذا (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
  fn = ffi_go_گهيرو_O32;
#وإلا
#إذا _MIPS_SIM ==_ABIN32
  إذا (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
#وإلا
  إذا (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    أرجع FFI_BAD_ABI;
#جيڪڏهن_پورو
  fn = ffi_go_گهيرو_N32;
#جيڪڏهن_پورو /* FFI_MIPS_O32 */

  گهيرو->tramp = (فراغ *)fn;
  گهيرو->cif = cif;
  گهيرو->fun = fun;

  أرجع FFI_OK;
}

#جيڪڏهن_پورو /* FFI_CLOSURES */

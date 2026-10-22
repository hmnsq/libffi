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

#అన్తర్భావ <ffi.h>
#అన్తర్భావ <ffi_common.h>

#అన్తర్భావ <stdint.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdio.h>

#నిర్వచితమైతే __GNUC__
#  ఒకవేళ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే USE__BUILTIN___CLEAR_CACHE
#  ఒకవేళ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  లేకపోతే
#    include <sys/cachectl.h>
#  endif
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_DEBUG
# define FFI_MIPS_STOP_ఇక్కడ() ffi_stop_ఇక్కడ()
#లేకపోతే
# define FFI_MIPS_STOP_ఇక్కడ() చేయి {} అయితే(0)
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_MIPS_N32
#నిర్వచించు FIX_ARGP \
FFI_ASSERT(argp <= &దొంతర[బైట్లు]); \
ఒకవేళ (argp == &దొంతర[బైట్లు]) \
{ \
  argp = దొంతర; \
  FFI_MIPS_STOP_ఇక్కడ(); \
}
#లేకపోతే
#నిర్వచించు FIX_ARGP 
#అయితే_ముగింపు


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

స్థిర శూన్య ffi_prep_వాదనలు(అక్షరం *దొంతర, 
			  extended_cif *ecif,
			  పూర్ణాంకం బైట్లు,
			  పూర్ణాంకం flags)
{
  పూర్ణాంకం i;
  శూన్య **p_argv;
  అక్షరం *argp, *argp_f;
  ffi_type **p_arg;

  memset(దొంతర, 0, బైట్లు);

#నిర్వచితమైతే FFI_MIPS_N32
  పూర్ణాంకం soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ఒకవేళ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ఒకవేళ (బైట్లు + 16 > 8 * పరిమాణం(ffi_arg))
        argp = &దొంతర[బైట్లు - (8 * పరిమాణం(ffi_arg))];
      లేకపోతే
        argp = దొంతర;
      * (గుర్తులేని దీర్ఘ *) argp = (గుర్తులేని దీర్ఘ) ecif->rvalue;
      argp += 16;
    }
  లేకపోతే
    {
      ఒకవేళ (బైట్లు > 8 * పరిమాణం(ffi_arg))
        argp = &దొంతర[బైట్లు - (8 * పరిమాణం(ffi_arg))];
      లేకపోతే
        argp = దొంతర;
    }
#లేకపోతే
  argp = దొంతర;
#అయితే_ముగింపు

  argp_f = argp;

#నిర్వచితమైతే FFI_MIPS_N32
  ఒకవేళ ( ecif->cif->rstruct_flag != 0 )
#లేకపోతే
  ఒకవేళ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#అయితే_ముగింపు  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += పరిమాణం(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  కొరకు (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      గుర్తులేని పూర్ణాంకం a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ఒకవేళ (a < పరిమాణం(ffi_arg))
        a = పరిమాణం(ffi_arg);
      
      ఒకవేళ ((a - 1) & (గుర్తులేని దీర్ఘ) argp)
	{
	  argp = (అక్షరం *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->కొలత;
      ఒకవేళ (z <= పరిమాణం(ffi_arg))
	{
          పూర్ణాంకం type = (*p_arg)->type;
	  z = పరిమాణం(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ఒకవేళ (type == FFI_TYPE_సూచిక)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ఒకవేళ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ఎంపిక (type)
	      {
	      స్థితి FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ఆపు;
	      స్థితి FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ఆపు;
	      అప్రమేయ:
		ఆపు;
	      }
	  }
	  ఎంపిక (type)
	    {
	      స్థితి FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ఆపు;

	      స్థితి FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ఆపు;
		  
	      స్థితి FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ఆపు;
		  
	      స్థితి FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ఆపు;
		  
	      స్థితి FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ఆపు;
		  
	      స్థితి FFI_TYPE_UINT32:
#నిర్వచితమైతే FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#లేకపోతే
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#అయితే_ముగింపు
		ఆపు;

#నిర్వచితమైతే FFI_MIPS_N32
	      స్థితి FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ఒకవేళ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(దశాంశం *) argp = *(దశాంశం *)(* p_argv);
		    argp += z;
		    అక్షరం *tmp = (శూన్య *) (*p_argv);
		    *(దశాంశం *) argp = *(దశాంశం *)(tmp+4);
		  }
		లేకపోతే
		  జ్ఞాపకం_నకలు(argp, *p_argv, (*p_arg)->కొలత);
		ఆపు;
#అయితే_ముగింపు
	      /* This can only happen with 64bit slots.  */
	      స్థితి FFI_TYPE_FLOAT:
		*(దశాంశం *) argp = *(దశాంశం *)(* p_argv);
		ఆపు;

	      /* Handle structures.  */
	      అప్రమేయ:
		జ్ఞాపకం_నకలు(argp, *p_argv, (*p_arg)->కొలత);
		ఆపు;
	    }
	}
      లేకపోతే
	{
#నిర్వచితమైతే FFI_MIPS_O32
	  జ్ఞాపకం_నకలు(argp, *p_argv, z);
#లేకపోతే
	  {
	    గుర్తులేని దీర్ఘ చివర = (గుర్తులేని దీర్ఘ) argp + z;
	    గుర్తులేని దీర్ఘ cap = (గుర్తులేని దీర్ఘ) దొంతర + బైట్లు;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ఒకవేళ (చివర <= cap)
	      జ్ఞాపకం_నకలు(argp, *p_argv, z);
	    లేకపోతే
	      {
		గుర్తులేని దీర్ఘ portion = cap - (గుర్తులేని దీర్ఘ)argp;

		జ్ఞాపకం_నకలు(argp, *p_argv, portion);
		argp = దొంతర;
                z -= portion;
		జ్ఞాపకం_నకలు(argp, (శూన్య*)((గుర్తులేని దీర్ఘ)(*p_argv) + portion),
                       z);
	      }
	  }
#అయితే_ముగింపు
      }
      p_argv++;
      argp += z;
      FIX_ARGP;
    }
}

#నిర్వచితమైతే FFI_MIPS_N32

/* The n32 spec says that if "a chunk consists solely of a double 
   float field (but not a double, which is part of a union), it
   is passed in a floating point register. Any other chunk is
   passed in an integer register". This code traverses structure
   definitions and generates the appropriate flags. */

స్థిర పూర్ణాంకం
calc_n32_struct_flags_element(గుర్తులేని *flags, ffi_type *e,
			      గుర్తులేని *loc, గుర్తులేని *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ఒకవేళ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ఒకవేళ (*arg_reg > 7)
	తిరిగి 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_బిట్లు));
    }
  *loc += e->కొలత;
  తిరిగి 0;
}

స్థిర గుర్తులేని
calc_n32_struct_flags(పూర్ణాంకం soft_float, ffi_type *arg,
		      గుర్తులేని *loc, గుర్తులేని *arg_reg)
{
  గుర్తులేని flags = 0;
  గుర్తులేని సూచీ = 0;

  ffi_type *e;

  ఒకవేళ (soft_float)
    తిరిగి 0;

  అయితే ((e = arg->elements[సూచీ]))
    {
      ఒకవేళ (e->type == FFI_TYPE_COMPLEX)
	{
	  ఒకవేళ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ఆపు;
	  ఒకవేళ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ఆపు;
	}
      లేకపోతే
	ఒకవేళ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ఆపు;
      సూచీ++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  తిరిగి flags;
}

స్థిర గుర్తులేని
calc_n32_return_struct_flags(పూర్ణాంకం soft_float, ffi_type *arg)
{
  గుర్తులేని flags;
  గుర్తులేని small = FFI_TYPE_SMALLSTRUCT;
  ffi_type *e;

  /* Returning structures under n32 is a tricky thing.
     A struct with only one or two floating point fields 
     is returned in $f0 (and $f2 if necessary). Any other
     struct results at most 128 bits are returned in $2
     (the first 64 bits) and $3 (remainder, if necessary).
     Larger structs are handled normally. */
  
  ఒకవేళ (arg->కొలత > 16)
    తిరిగి 0;

  ఒకవేళ (arg->కొలత > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ఒకవేళ (e->type == FFI_TYPE_COMPLEX)
    {
      పూర్ణాంకం type = e->elements[0]->type;

      ఒకవేళ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	తిరిగి small;

      ఒకవేళ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  తిరిగి small;
	}

      flags = (type << FFI_FLAG_బిట్లు) + type;
    }
  లేకపోతే
    {
      ఒకవేళ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	తిరిగి small;

      flags = e->type;

      ఒకవేళ (arg->elements[1])
	{
	  e = arg->elements[1];
	  ఒకవేళ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    తిరిగి small;

	  ఒకవేళ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      తిరిగి small;
	    }

	  flags += e->type << FFI_FLAG_బిట్లు;
	}
    }

  ఒకవేళ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  తిరిగి flags;
}

#అయితే_ముగింపు

/* Perform machine dependent cif processing */
స్థిర ffi_స్థితి ffi_prep_cif_machdep_int(ffi_cif *cif, గుర్తులేని nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#నిర్వచితమైతే FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  ఒకవేళ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ఒకవేళ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ఎంపిక ((cif->arg_types)[0]->type)
	    {
	    స్థితి FFI_TYPE_FLOAT:
	    స్థితి FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ఆపు;
	      
	    అప్రమేయ:
	      ఆపు;
	    }

	  ఒకవేళ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ఒకవేళ (cif->flags)
		{
		  ఎంపిక ((cif->arg_types)[1]->type)
		    {
		    స్థితి FFI_TYPE_FLOAT:
		    స్థితి FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_బిట్లు;
		      ఆపు;
		      
		    అప్రమేయ:
		      ఆపు;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ఒకవేళ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ఎంపిక (cif->rtype->type)
        {
        స్థితి FFI_TYPE_VOID:
        స్థితి FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_బిట్లు * 2);
          ఆపు;

        స్థితి FFI_TYPE_SINT64:
        స్థితి FFI_TYPE_UINT64:
        స్థితి FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_బిట్లు * 2);
          ఆపు;
      
        స్థితి FFI_TYPE_FLOAT:
        అప్రమేయ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_బిట్లు * 2);
          ఆపు;
        }
    }
  లేకపోతే
    {
      /* FFI_O32 */      
      ఎంపిక (cif->rtype->type)
        {
        స్థితి FFI_TYPE_VOID:
        స్థితి FFI_TYPE_STRUCT:
        స్థితి FFI_TYPE_FLOAT:
        స్థితి FFI_TYPE_DOUBLE:
        స్థితి FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_బిట్లు * 2);
	  ఒకవేళ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_బిట్లు * 4);
          ఆపు;

        స్థితి FFI_TYPE_SINT64:
        స్థితి FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_బిట్లు * 2);
          ఆపు;
      
        అప్రమేయ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_బిట్లు * 2);
          ఆపు;
        }
    }
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    గుర్తులేని arg_reg = 0;
    గుర్తులేని loc = 0;
    గుర్తులేని లెక్క = (cif->nargs < 8) ? cif->nargs : 8;
    గుర్తులేని సూచీ = 0;

    గుర్తులేని struct_flags = 0;
    పూర్ణాంకం soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ఒకవేళ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    లెక్క = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	లేకపోతే
	    cif->rstruct_flag = 0;
      }
    లేకపోతే
      cif->rstruct_flag = 0;

    అయితే (లెక్క-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[సూచీ];

	ఎంపిక (t->type)
	  {
	  స్థితి FFI_TYPE_FLOAT:
	  స్థితి FFI_TYPE_DOUBLE:
	    ఒకవేళ (!soft_float && సూచీ < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_బిట్లు);
	    arg_reg++;
	    ఆపు;
          స్థితి FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ఒకవేళ (soft_float || సూచీ >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    లేకపోతే
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_బిట్లు));
		arg_reg++;
		ఒకవేళ (arg_reg >= 8)
		  కొనసాగు;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_బిట్లు));
		arg_reg++;
	      }
            ఆపు;

	  స్థితి FFI_TYPE_COMPLEX:
	    ఎంపిక (t->elements[0]->type)
	      {
	      స్థితి FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ఒకవేళ (soft_float || సూచీ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		లేకపోతే
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_బిట్లు));
		    arg_reg++;
		    ఒకవేళ (arg_reg >= 8)
		        కొనసాగు;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_బిట్లు));
		    arg_reg++;
		    ఒకవేళ (arg_reg >= 8)
		        కొనసాగు;
		  }
		/* passthrough */
	      స్థితి FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->బైట్లు += 16;
		/* passthrough */
	      స్థితి FFI_TYPE_SINT32:
	      స్థితి FFI_TYPE_UINT32:
	      స్థితి FFI_TYPE_DOUBLE:
		ఒకవేళ (soft_float || సూచీ >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		లేకపోతే
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_బిట్లు));
		    arg_reg++;
		    ఒకవేళ (arg_reg >= 8)
		        కొనసాగు;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_బిట్లు));
		    arg_reg++;
		  }
		ఆపు;
	      అప్రమేయ:
		arg_reg += 2;
		ఆపు;
	      }
	    ఆపు;

	  స్థితి FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || సూచీ >= nfixedargs,
						t, &loc, &arg_reg);
	    ఆపు;

	  అప్రమేయ:
	    arg_reg++;
            ఆపు;
	  }

	సూచీ++;
      }

  /* Set the return type flag */
    ఎంపిక (cif->rtype->type)
      {
      స్థితి FFI_TYPE_STRUCT:
	{
	  ఒకవేళ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  లేకపోతే
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_బిట్లు * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_బిట్లు * 8));
	    }
	  ఆపు;
	}
      
      స్థితి FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ఆపు;

      స్థితి FFI_TYPE_సూచిక:
	ఒకవేళ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_బిట్లు * 8);
	లేకపోతే
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_బిట్లు * 8);
	ఆపు;

      స్థితి FFI_TYPE_FLOAT:
	ఒకవేళ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_బిట్లు * 8);
	    ఆపు;
	  }
	/* else fall through */
      స్థితి FFI_TYPE_DOUBLE:
	ఒకవేళ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_బిట్లు * 8);
	లేకపోతే
	  cif->flags += cif->rtype->type << (FFI_FLAG_బిట్లు * 8);
	ఆపు;

      స్థితి FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ఒకవేళ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_బిట్లు * 8);
 	  }
	లేకపోతే
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_బిట్లు * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_బిట్లు))
					      << (4 + (FFI_FLAG_బిట్లు * 8));
	  }
	ఆపు;
      స్థితి FFI_TYPE_COMPLEX:
	{
	  పూర్ణాంకం type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_బిట్లు * 8));
	  ఒకవేళ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ఎంపిక (type)
		{
		స్థితి FFI_TYPE_DOUBLE:
		స్థితి FFI_TYPE_SINT64:
		స్థితి FFI_TYPE_UINT64:
		స్థితి FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ఆపు;
		స్థితి FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ఆపు;
		స్థితి FFI_TYPE_FLOAT:
		అప్రమేయ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_బిట్లు * 8));
	    }
	  లేకపోతే
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_బిట్లు * 8));
	    }
	  ఆపు;
	}
      స్థితి FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_బిట్లు * 8);
	ఆపు;
      స్థితి FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_బిట్లు * 8);
	ఆపు;
      అప్రమేయ:
	cif->flags += cif->rtype->type << (FFI_FLAG_బిట్లు * 8);
	ఆపు;
      }
  }
#అయితే_ముగింపు
  తిరిగి FFI_OK;
}

ffi_స్థితి ffi_prep_cif_machdep(ffi_cif *cif)
{
    తిరిగి ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_స్థితి ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    గుర్తులేని nfixedargs,
                                    గుర్తులేని ntotalargs MAYBE_వాడని)
{
    తిరిగి ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
బాహ్య పూర్ణాంకం ffi_call_O32(శూన్య (*)(అక్షరం *, extended_cif *, పూర్ణాంకం, పూర్ణాంకం), 
			extended_cif *, గుర్తులేని, 
			గుర్తులేని, గుర్తులేని *, శూన్య (*)(శూన్య), శూన్య *సంవృతం);

/* Low level routine for calling N32 functions */
బాహ్య పూర్ణాంకం ffi_call_N32(శూన్య (*)(అక్షరం *, extended_cif *, పూర్ణాంకం, పూర్ణాంకం), 
			extended_cif *, గుర్తులేని, 
			గుర్తులేని, శూన్య *, శూన్య (*)(శూన్య), శూన్య *సంవృతం);

శూన్య ffi_call_int(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, 
	      శూన్య **avalue, శూన్య *సంవృతం)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  ఒకవేళ ((rvalue == ఖాళీ) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->కొలత);
  లేకపోతే
    ecif.rvalue = rvalue;
    
  ఎంపిక (cif->abi) 
    {
#నిర్వచితమైతే FFI_MIPS_O32
    స్థితి FFI_O32:
    స్థితి FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_వాదనలు, &ecif, cif->బైట్లు, 
		   cif->flags, ecif.rvalue, fn, సంవృతం);
      ఆపు;
#అయితే_ముగింపు

#నిర్వచితమైతే FFI_MIPS_N32
    స్థితి FFI_N32:
    స్థితి FFI_N32_SOFT_FLOAT:
    స్థితి FFI_N64:
    స్థితి FFI_N64_SOFT_FLOAT:
      {
        పూర్ణాంకం copy_rvalue = 0;
	పూర్ణాంకం copy_స్థానభ్రంశం = 0;
        అక్షరం *rvalue_copy = ecif.rvalue;
        ఒకవేళ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->కొలత < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	లేకపోతే ఒకవేళ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ఒకవేళ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_స్థానభ్రంశం = 4;
#అయితే_ముగింపు
	  }
        ffi_call_N32(ffi_prep_వాదనలు, &ecif, cif->బైట్లు,
                     cif->flags, rvalue_copy, fn, సంవృతం);
        ఒకవేళ (copy_rvalue)
          జ్ఞాపకం_నకలు(ecif.rvalue, rvalue_copy + copy_స్థానభ్రంశం, cif->rtype->కొలత);
      }
      ఆపు;
#అయితే_ముగింపు

    అప్రమేయ:
      FFI_ASSERT(0);
      ఆపు;
    }
}

శూన్య
ffi_call(ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue, శూన్య **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ఖాళీ);
}

శూన్య
ffi_call_go (ffi_cif *cif, శూన్య (*fn)(శూన్య), శూన్య *rvalue,
	     శూన్య **avalue, శూన్య *సంవృతం)
{
  ffi_call_int (cif, fn, rvalue, avalue, సంవృతం);
}


#ఒకవేళ FFI_CLOSURES
#ఒకవేళ defined(FFI_MIPS_O32)
బాహ్య శూన్య ffi_సంవృతం_O32(శూన్య);
బాహ్య శూన్య ffi_go_సంవృతం_O32(శూన్య);
#లేకపోతే
బాహ్య శూన్య ffi_సంవృతం_N32(శూన్య);
బాహ్య శూన్య ffi_go_సంవృతం_N32(శూన్య);
#అయితే_ముగింపు /* FFI_MIPS_O32 */

ffi_స్థితి
ffi_prep_సంవృతం_loc (ffi_సంవృతం *సంవృతం,
		      ffi_cif *cif,
		      శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*),
		      శూన్య *user_దత్తాంశం,
		      శూన్య *codeloc)
{
  గుర్తులేని పూర్ణాంకం *tramp = (గుర్తులేని పూర్ణాంకం *) &సంవృతం->tramp[0];
  శూన్య * fn;
  అక్షరం *clear_location = (అక్షరం *) codeloc;

#ఒకవేళ defined(FFI_MIPS_O32)
  ఒకవేళ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
  fn = ffi_సంవృతం_O32;
#లేకపోతే
#ఒకవేళ _MIPS_SIM ==_ABIN32
  ఒకవేళ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
#లేకపోతే
  ఒకవేళ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
  fn = ffi_సంవృతం_N32;
#అయితే_ముగింపు /* FFI_MIPS_O32 */

#ఒకవేళ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((గుర్తులేని)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((గుర్తులేని)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((గుర్తులేని)codeloc >> 16);
  /* jr   $25          */
#ఒకవేళ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#లేకపోతే
  tramp[3] = 0x03200009;
#అయితే_ముగింపు
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((గుర్తులేని)codeloc & 0xffff);
#లేకపోతే
  /* N64 has a somewhat larger trampoline.  */
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((గుర్తులేని దీర్ఘ)fn >> 48);
  /* lui  $12,high(codeloc) */
  tramp[1] = 0x3c0c0000 | ((గుర్తులేని దీర్ఘ)codeloc >> 48);
  /* ori  $25,mid-high(fn)  */
  tramp[2] = 0x37390000 | (((గుర్తులేని దీర్ఘ)fn >> 32 ) & 0xffff);
  /* ori  $12,mid-high(codeloc)  */
  tramp[3] = 0x358c0000 | (((గుర్తులేని దీర్ఘ)codeloc >> 32) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[4] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[5] = 0x000c6438;
  /* ori  $25,mid-low(fn)  */
  tramp[6] = 0x37390000 | (((గుర్తులేని దీర్ఘ)fn >> 16 ) & 0xffff);
  /* ori  $12,mid-low(codeloc)  */
  tramp[7] = 0x358c0000 | (((గుర్తులేని దీర్ఘ)codeloc >> 16) & 0xffff);
  /* dsll $25,$25,16 */
  tramp[8] = 0x0019cc38;
  /* dsll $12,$12,16 */
  tramp[9] = 0x000c6438;
  /* ori  $25,low(fn)  */
  tramp[10] = 0x37390000 | ((గుర్తులేని దీర్ఘ)fn  & 0xffff);
  /* jr   $25          */
#ఒకవేళ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#లేకపోతే
  tramp[11] = 0x03200009;
#అయితే_ముగింపు
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((గుర్తులేని దీర్ఘ)codeloc & 0xffff);

#అయితే_ముగింపు

  సంవృతం->cif = cif;
  సంవృతం->fun = fun;
  సంవృతం->user_దత్తాంశం = user_దత్తాంశం;

#ఒకవేళ !defined(__FreeBSD__)
#నిర్వచితమైతే USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_కొలత);
#లేకపోతే
  cacheflush (clear_location, FFI_TRAMPOLINE_కొలత, ICACHE);
#అయితే_ముగింపు
#అయితే_ముగింపు /* ! __FreeBSD__ */
  తిరిగి FFI_OK;
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
పూర్ణాంకం
ffi_సంవృతం_mips_inner_O32 (ffi_cif *cif,
                            శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
			    శూన్య *user_దత్తాంశం,
			    శూన్య *rvalue, ffi_arg *ar,
			    ద్విగుణ *fpr)
{
  శూన్య **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  పూర్ణాంకం i, avn, argn, చూసిన_int;

  avalue = alloca (cif->nargs * పరిమాణం (ffi_arg));
  avaluep = alloca (cif->nargs * పరిమాణం (ffi_arg));

  చూసిన_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ఒకవేళ ((cif->flags >> (FFI_FLAG_బిట్లు * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (శూన్య *)(uintptr_t)ar[0];
      argn = 1;
      చూసిన_int = 1;
    }
  ఒకవేళ ((cif->flags >> (FFI_FLAG_బిట్లు * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  అయితే (i < avn)
    {
      ఒకవేళ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ఒకవేళ (i < 2 && !చూసిన_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ఒకవేళ defined(__MIPSEB__) || defined(_MIPSEB)
	  ఒకవేళ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((అక్షరం *) &fpr[i]) + పరిమాణం (దశాంశం);
	  లేకపోతే
#అయితే_ముగింపు
	    avaluep[i] = (అక్షరం *) &fpr[i];
	}
      లేకపోతే
	{
	  ఎంపిక (arg_types[i]->type)
	    {
	      స్థితి FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ఆపు;

	      స్థితి FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ఆపు;
		  
	      స్థితి FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ఆపు;
		  
	      స్థితి FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ఆపు;

	      అప్రమేయ:
		avaluep[i] = (అక్షరం *) &ar[argn];
		ఆపు;
	    }
	  చూసిన_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->కొలత, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_దత్తాంశం);

  ఒకవేళ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ఎంపిక (cif->rtype->type)
        {
        స్థితి FFI_TYPE_FLOAT:
          తిరిగి FFI_TYPE_INT;
        స్థితి FFI_TYPE_DOUBLE:
          తిరిగి FFI_TYPE_UINT64;
        అప్రమేయ:
          తిరిగి cif->rtype->type;
        }
    }
  లేకపోతే
    {
      ఒకవేళ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ అస్థిర ("move $v1, %0" : : "r"(cif->rtype->కొలత));
      }
      తిరిగి cif->rtype->type;
    }
}

#ఒకవేళ defined(FFI_MIPS_N32)

స్థిర శూన్య
copy_struct_N32(అక్షరం *లక్ష్యం_2, గుర్తులేని offset, ffi_abi abi, ffi_type *type,
                పూర్ణాంకం argn, గుర్తులేని arg_స్థానభ్రంశం, ffi_arg *ar,
                ffi_arg *fpr, పూర్ణాంకం soft_float)
{
  ffi_type **elt_typep = type->elements;
  అయితే(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      గుర్తులేని o;
      అక్షరం *tp;
      అక్షరం *argp;
      అక్షరం *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_స్థానభ్రంశం += o - offset;
      offset = o;
      argn += arg_స్థానభ్రంశం / పరిమాణం(ffi_arg);
      arg_స్థానభ్రంశం = arg_స్థానభ్రంశం % పరిమాణం(ffi_arg);

      argp = (అక్షరం *)(ar + argn);
      fpp = (అక్షరం *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = లక్ష్యం_2 + offset;

      ఒకవేళ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ద్విగుణ *)tp = *(ద్విగుణ *)fpp;
      లేకపోతే
        జ్ఞాపకం_నకలు(tp, argp + arg_స్థానభ్రంశం, elt_type->కొలత);

      offset += elt_type->కొలత;
      arg_స్థానభ్రంశం += elt_type->కొలత;
      elt_typep++;
      argn += arg_స్థానభ్రంశం / పరిమాణం(ffi_arg);
      arg_స్థానభ్రంశం = arg_స్థానభ్రంశం % పరిమాణం(ffi_arg);
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
పూర్ణాంకం
ffi_సంవృతం_mips_inner_N32 (ffi_cif *cif, 
			    శూన్య (*fun)(ffi_cif*, శూన్య*, శూన్య**, శూన్య*),
                            శూన్య *user_దత్తాంశం,
			    శూన్య *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  శూన్య **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  పూర్ణాంకం i, avn, argn;
  పూర్ణాంకం soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * పరిమాణం (ffi_arg));
  avaluep = alloca (cif->nargs * పరిమాణం (ffi_arg));

  argn = 0;

  ఒకవేళ (cif->rstruct_flag)
    {
#ఒకవేళ _MIPS_SIM==_ABIN32
      rvalue = (శూన్య *)(UINT32)ar[0];
#లేకపోతే /* N64 */
      rvalue = (శూన్య *)ar[0];
#అయితే_ముగింపు
      argn = 1;
    }
  ఒకవేళ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  అయితే (i < avn)
    {
      ఒకవేళ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ఒకవేళ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ఒకవేళ defined(__MIPSEB__) || defined(_MIPSEB)
          ఒకవేళ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((అక్షరం *) argp) + పరిమాణం (దశాంశం);
          లేకపోతే
#అయితే_ముగింపు
            avaluep[i] = (అక్షరం *) argp;
        }
      లేకపోతే ఒకవేళ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (అక్షరం *) argp;
        }
      లేకపోతే ఒకవేళ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (అక్షరం *) argp;
        }
      లేకపోతే ఒకవేళ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ఒకవేళ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  లేకపోతే
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (అక్షరం *) argp;
        }
      లేకపోతే
        {
          గుర్తులేని type = arg_types[i]->type;

          ఒకవేళ (arg_types[i]->alignment > పరిమాణం(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / పరిమాణం(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ఒకవేళ (type == FFI_TYPE_సూచిక)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ఒకవేళ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ఎంపిక (type)
            {
            స్థితి FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ఆపు;

            స్థితి FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ఆపు;

            స్థితి FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ఆపు;

            స్థితి FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ఆపు;

            స్థితి FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ఆపు;

            స్థితి FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ఆపు;

            స్థితి FFI_TYPE_STRUCT:
              ఒకవేళ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->కొలత);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ఆపు;
                }
              /* Else fall through.  */
            అప్రమేయ:
              avaluep[i] = (అక్షరం *) argp;
              ఆపు;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->కొలత, పరిమాణం(ffi_arg)) / పరిమాణం(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_దత్తాంశం);

  తిరిగి cif->flags >> (FFI_FLAG_బిట్లు * 8);
}

#అయితే_ముగింపు /* FFI_MIPS_N32 */

#ఒకవేళ defined(FFI_MIPS_O32)
బాహ్య శూన్య ffi_సంవృతం_O32(శూన్య);
బాహ్య శూన్య ffi_go_సంవృతం_O32(శూన్య);
#లేకపోతే
బాహ్య శూన్య ffi_సంవృతం_N32(శూన్య);
బాహ్య శూన్య ffi_go_సంవృతం_N32(శూన్య);
#అయితే_ముగింపు /* FFI_MIPS_O32 */

ffi_స్థితి
ffi_prep_go_సంవృతం (ffi_go_సంవృతం* సంవృతం, ffi_cif* cif,
		     శూన్య (*fun)(ffi_cif*,శూన్య*,శూన్య**,శూన్య*))
{
  శూన్య * fn;

#ఒకవేళ defined(FFI_MIPS_O32)
  ఒకవేళ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
  fn = ffi_go_సంవృతం_O32;
#లేకపోతే
#ఒకవేళ _MIPS_SIM ==_ABIN32
  ఒకవేళ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
#లేకపోతే
  ఒకవేళ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    తిరిగి FFI_BAD_ABI;
#అయితే_ముగింపు
  fn = ffi_go_సంవృతం_N32;
#అయితే_ముగింపు /* FFI_MIPS_O32 */

  సంవృతం->tramp = (శూన్య *)fn;
  సంవృతం->cif = cif;
  సంవృతం->fun = fun;

  తిరిగి FFI_OK;
}

#అయితే_ముగింపు /* FFI_CLOSURES */

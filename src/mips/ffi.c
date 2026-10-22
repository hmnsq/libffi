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

#ଅନ୍ତର୍ଭାଵ <ffi.h>
#ଅନ୍ତର୍ଭାଵ <ffi_common.h>

#ଅନ୍ତର୍ଭାଵ <stdint.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ifdef __GNUC__
#  ଯଦି (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  ଯଦି defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ଅନ୍ୟଥା
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#ଅନ୍ୟଥା
# define FFI_MIPS_STOP_HERE() କର {} ଯେତେବେଳେ(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &ଗଦା[bytes]); \
ଯଦି (argp == &ଗଦା[bytes]) \
{ \
  argp = ଗଦା; \
  FFI_MIPS_STOP_HERE(); \
}
#ଅନ୍ୟଥା
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

ସ୍ଥିର ଶୂନ୍ୟ ffi_prep_args(ଅକ୍ଷର *ଗଦା, 
			  extended_cif *ecif,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା bytes,
			  ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଶୂନ୍ୟ **p_argv;
  ଅକ୍ଷର *argp, *argp_f;
  ffi_type **p_arg;

  memset(ଗଦା, 0, bytes);

#ifdef FFI_MIPS_N32
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ଯଦି (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ଯଦି (bytes + 16 > 8 * ଆକାର(ffi_arg))
        argp = &ଗଦା[bytes - (8 * ଆକାର(ffi_arg))];
      ଅନ୍ୟଥା
        argp = ଗଦା;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  ଅନ୍ୟଥା
    {
      ଯଦି (bytes > 8 * ଆକାର(ffi_arg))
        argp = &ଗଦା[bytes - (8 * ଆକାର(ffi_arg))];
      ଅନ୍ୟଥା
        argp = ଗଦା;
    }
#ଅନ୍ୟଥା
  argp = ଗଦା;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  ଯଦି ( ecif->cif->rstruct_flag != 0 )
#ଅନ୍ୟଥା
  ଯଦି ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += ଆକାର(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  ପାଇଁ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ଯଦି (a < ଆକାର(ffi_arg))
        a = ଆକାର(ffi_arg);
      
      ଯଦି ((a - 1) & (unsigned long) argp)
	{
	  argp = (ଅକ୍ଷର *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      ଯଦି (z <= ଆକାର(ffi_arg))
	{
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = (*p_arg)->type;
	  z = ଆକାର(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ଯଦି (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ଯଦି (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    ବାଛ (type)
	      {
	      ସ୍ଥିତି FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		ଭାଙ୍ଗ;
	      ସ୍ଥିତି FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		ଭାଙ୍ଗ;
	      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	      }
	  }
	  ବାଛ (type)
	    {
	      ସ୍ଥିତି FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		ଭାଙ୍ଗ;

	      ସ୍ଥିତି FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ଅନ୍ୟଥା
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		ଭାଙ୍ଗ;

#ifdef FFI_MIPS_N32
	      ସ୍ଥିତି FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ଯଦି(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(ଦଶମିକ *) argp = *(ଦଶମିକ *)(* p_argv);
		    argp += z;
		    ଅକ୍ଷର *tmp = (ଶୂନ୍ୟ *) (*p_argv);
		    *(ଦଶମିକ *) argp = *(ଦଶମିକ *)(tmp+4);
		  }
		ଅନ୍ୟଥା
		  memcpy(argp, *p_argv, (*p_arg)->size);
		ଭାଙ୍ଗ;
#endif
	      /* This can only happen with 64bit slots.  */
	      ସ୍ଥିତି FFI_TYPE_FLOAT:
		*(ଦଶମିକ *) argp = *(ଦଶମିକ *)(* p_argv);
		ଭାଙ୍ଗ;

	      /* Handle structures.  */
	      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		memcpy(argp, *p_argv, (*p_arg)->size);
		ଭାଙ୍ଗ;
	    }
	}
      ଅନ୍ୟଥା
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#ଅନ୍ୟଥା
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) ଗଦା + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ଯଦି (end <= cap)
	      memcpy(argp, *p_argv, z);
	    ଅନ୍ୟଥା
	      {
		unsigned long portion = cap - (unsigned long)argp;

		memcpy(argp, *p_argv, portion);
		argp = ଗଦା;
                z -= portion;
		memcpy(argp, (ଶୂନ୍ୟ*)((unsigned long)(*p_argv) + portion),
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ଯଦି (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ଯଦି (*arg_reg > 7)
	ଫେରନ୍ତୁ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର unsigned
calc_n32_struct_flags(ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  ଯଦି (soft_float)
    ଫେରନ୍ତୁ 0;

  ଯେତେବେଳେ ((e = arg->elements[index]))
    {
      ଯଦି (e->type == FFI_TYPE_COMPLEX)
	{
	  ଯଦି (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ଭାଙ୍ଗ;
	  ଯଦି (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    ଭାଙ୍ଗ;
	}
      ଅନ୍ୟଥା
	ଯଦି (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  ଭାଙ୍ଗ;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ଫେରନ୍ତୁ flags;
}

ସ୍ଥିର unsigned
calc_n32_return_struct_flags(ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float, ffi_type *arg)
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
  
  ଯଦି (arg->size > 16)
    ଫେରନ୍ତୁ 0;

  ଯଦି (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ଯଦି (e->type == FFI_TYPE_COMPLEX)
    {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = e->elements[0]->type;

      ଯଦି (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ଫେରନ୍ତୁ small;

      ଯଦି (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ଫେରନ୍ତୁ small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  ଅନ୍ୟଥା
    {
      ଯଦି (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ଫେରନ୍ତୁ small;

      flags = e->type;

      ଯଦି (arg->elements[1])
	{
	  e = arg->elements[1];
	  ଯଦି (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ଫେରନ୍ତୁ small;

	  ଯଦି (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ଫେରନ୍ତୁ small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  ଯଦି (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ଫେରନ୍ତୁ flags;
}

#endif

/* Perform machine dependent cif processing */
ସ୍ଥିର ffi_status ffi_prep_cif_machdep_int(ffi_cif *cif, unsigned nfixedargs)
{
  cif->flags = 0;
  cif->mips_nfixedargs = nfixedargs;

#ifdef FFI_MIPS_O32
  /* Set the flags necessary for O32 processing.  FFI_O32_SOFT_FLOAT
   * does not have special handling for floating point args.
   */

  ଯଦି (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ଯଦି (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  ବାଛ ((cif->arg_types)[0]->type)
	    {
	    ସ୍ଥିତି FFI_TYPE_FLOAT:
	    ସ୍ଥିତି FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      ଭାଙ୍ଗ;
	      
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      ଭାଙ୍ଗ;
	    }

	  ଯଦି (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ଯଦି (cif->flags)
		{
		  ବାଛ ((cif->arg_types)[1]->type)
		    {
		    ସ୍ଥିତି FFI_TYPE_FLOAT:
		    ସ୍ଥିତି FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      ଭାଙ୍ଗ;
		      
		    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		      ଭାଙ୍ଗ;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ଯଦି (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ବାଛ (cif->rtype->type)
        {
        ସ୍ଥିତି FFI_TYPE_VOID:
        ସ୍ଥିତି FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_SINT64:
        ସ୍ଥିତି FFI_TYPE_UINT64:
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ଭାଙ୍ଗ;
      
        ସ୍ଥିତି FFI_TYPE_FLOAT:
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ଭାଙ୍ଗ;
        }
    }
  ଅନ୍ୟଥା
    {
      /* FFI_O32 */      
      ବାଛ (cif->rtype->type)
        {
        ସ୍ଥିତି FFI_TYPE_VOID:
        ସ୍ଥିତି FFI_TYPE_STRUCT:
        ସ୍ଥିତି FFI_TYPE_FLOAT:
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
        ସ୍ଥିତି FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  ଯଦି (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          ଭାଙ୍ଗ;

        ସ୍ଥିତି FFI_TYPE_SINT64:
        ସ୍ଥିତି FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          ଭାଙ୍ଗ;
      
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          ଭାଙ୍ଗ;
        }
    }
#endif

#ifdef FFI_MIPS_N32
  /* Set the flags necessary for N32 processing */
  {
    unsigned arg_reg = 0;
    unsigned loc = 0;
    unsigned ଗଣନା_2 = (cif->nargs < 8) ? cif->nargs : 8;
    unsigned index = 0;

    unsigned struct_flags = 0;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ଯଦି (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    ଗଣନା_2 = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ଅନ୍ୟଥା
	    cif->rstruct_flag = 0;
      }
    ଅନ୍ୟଥା
      cif->rstruct_flag = 0;

    ଯେତେବେଳେ (ଗଣନା_2-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	ବାଛ (t->type)
	  {
	  ସ୍ଥିତି FFI_TYPE_FLOAT:
	  ସ୍ଥିତି FFI_TYPE_DOUBLE:
	    ଯଦି (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    ଭାଙ୍ଗ;
          ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ଯଦି (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ଅନ୍ୟଥା
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		ଯଦି (arg_reg >= 8)
		  ଜାରି;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_COMPLEX:
	    ବାଛ (t->elements[0]->type)
	      {
	      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ଯଦି (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ଅନ୍ୟଥା
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ଯଦି (arg_reg >= 8)
		        ଜାରି;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ଯଦି (arg_reg >= 8)
		        ଜାରି;
		  }
		/* passthrough */
	      ସ୍ଥିତି FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      ସ୍ଥିତି FFI_TYPE_SINT32:
	      ସ୍ଥିତି FFI_TYPE_UINT32:
	      ସ୍ଥିତି FFI_TYPE_DOUBLE:
		ଯଦି (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ଅନ୍ୟଥା
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ଯଦି (arg_reg >= 8)
		        ଜାରି;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		ଭାଙ୍ଗ;
	      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		arg_reg += 2;
		ଭାଙ୍ଗ;
	      }
	    ଭାଙ୍ଗ;

	  ସ୍ଥିତି FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    ଭାଙ୍ଗ;

	  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	    arg_reg++;
            ଭାଙ୍ଗ;
	  }

	index++;
      }

  /* Set the return type flag */
    ବାଛ (cif->rtype->type)
      {
      ସ୍ଥିତି FFI_TYPE_STRUCT:
	{
	  ଯଦି (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ଅନ୍ୟଥା
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ଭାଙ୍ଗ;
	}
      
      ସ୍ଥିତି FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_POINTER:
	ଯଦି (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ଅନ୍ୟଥା
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_FLOAT:
	ଯଦି (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    ଭାଙ୍ଗ;
	  }
	/* else fall through */
      ସ୍ଥିତି FFI_TYPE_DOUBLE:
	ଯଦି (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ଅନ୍ୟଥା
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ଭାଙ୍ଗ;

      ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ଯଦି (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	ଅନ୍ୟଥା
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_COMPLEX:
	{
	  ପୂର୍ଣ୍ଣସଂଖ୍ୟା type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  ଯଦି (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      ବାଛ (type)
		{
		ସ୍ଥିତି FFI_TYPE_DOUBLE:
		ସ୍ଥିତି FFI_TYPE_SINT64:
		ସ୍ଥିତି FFI_TYPE_UINT64:
		ସ୍ଥିତି FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  ଭାଙ୍ଗ;
		ସ୍ଥିତି FFI_TYPE_FLOAT:
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ଅନ୍ୟଥା
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ଭାଙ୍ଗ;
	}
      ସ୍ଥିତି FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ଭାଙ୍ଗ;
      ସ୍ଥିତି FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	ଭାଙ୍ଗ;
      }
  }
#endif
  ଫେରନ୍ତୁ FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    ଫେରନ୍ତୁ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    ଫେରନ୍ତୁ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_call_O32(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure);

/* Low level routine for calling N32 functions */
ବାହ୍ୟ ପୂର୍ଣ୍ଣସଂଖ୍ୟା ffi_call_N32(ଶୂନ୍ୟ (*)(ଅକ୍ଷର *, extended_cif *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା), 
			extended_cif *, unsigned, 
			unsigned, ଶୂନ୍ୟ *, ଶୂନ୍ୟ (*)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *closure);

ଶୂନ୍ୟ ffi_call_int(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, 
	      ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  extended_cif ecif;

  ecif.cif = cif;
  ecif.avalue = avalue;
  
  /* If the return value is a struct and we don't have a return	*/
  /* value address then we need to make one		        */
  
  ଯଦି ((rvalue == ଖାଲି) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  ଅନ୍ୟଥା
    ecif.rvalue = rvalue;
    
  ବାଛ (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    ସ୍ଥିତି FFI_O32:
    ସ୍ଥିତି FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      ଭାଙ୍ଗ;
#endif

#ifdef FFI_MIPS_N32
    ସ୍ଥିତି FFI_N32:
    ସ୍ଥିତି FFI_N32_SOFT_FLOAT:
    ସ୍ଥିତି FFI_N64:
    ସ୍ଥିତି FFI_N64_SOFT_FLOAT:
      {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା copy_rvalue = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା copy_offset = 0;
        ଅକ୍ଷର *rvalue_copy = ecif.rvalue;
        ଯଦି (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ଅନ୍ୟଥା ଯଦି (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ଯଦି defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        ଯଦି (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      ଭାଙ୍ଗ;
#endif

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      FFI_ASSERT(0);
      ଭାଙ୍ଗ;
    }
}

ଶୂନ୍ୟ
ffi_call(ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue, ଶୂନ୍ୟ **avalue)
{
  ffi_call_int (cif, fn, rvalue, avalue, ଖାଲି);
}

ଶୂନ୍ୟ
ffi_call_go (ffi_cif *cif, ଶୂନ୍ୟ (*fn)(ଶୂନ୍ୟ), ଶୂନ୍ୟ *rvalue,
	     ଶୂନ୍ୟ **avalue, ଶୂନ୍ୟ *closure)
{
  ffi_call_int (cif, fn, rvalue, avalue, closure);
}


#ଯଦି FFI_CLOSURES
#ଯଦି defined(FFI_MIPS_O32)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_O32(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_O32(ଶୂନ୍ୟ);
#ଅନ୍ୟଥା
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_N32(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_N32(ଶୂନ୍ୟ);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*),
		      ଶୂନ୍ୟ *user_data,
		      ଶୂନ୍ୟ *codeloc)
{
  unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *tramp = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *) &closure->tramp[0];
  ଶୂନ୍ୟ * fn;
  ଅକ୍ଷର *clear_location = (ଅକ୍ଷର *) codeloc;

#ଯଦି defined(FFI_MIPS_O32)
  ଯଦି (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
  fn = ffi_closure_O32;
#ଅନ୍ୟଥା
#ଯଦି _MIPS_SIM ==_ABIN32
  ଯଦି (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#ଅନ୍ୟଥା
  ଯଦି (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#ଯଦି defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#ଯଦି !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ଅନ୍ୟଥା
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#ଅନ୍ୟଥା
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
#ଯଦି !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ଅନ୍ୟଥା
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ଯଦି !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#ଅନ୍ୟଥା
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  ଫେରନ୍ତୁ FFI_OK;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
			    ଶୂନ୍ୟ *user_data,
			    ଶୂନ୍ୟ *rvalue, ffi_arg *ar,
			    ଦ୍ୱିଗୁଣ *fpr)
{
  ଶୂନ୍ୟ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * ଆକାର (ffi_arg));
  avaluep = alloca (cif->nargs * ଆକାର (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ଯଦି ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (ଶୂନ୍ୟ *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  ଯଦି ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ଯେତେବେଳେ (i < avn)
    {
      ଯଦି (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ଯଦି (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ଯଦି defined(__MIPSEB__) || defined(_MIPSEB)
	  ଯଦି (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((ଅକ୍ଷର *) &fpr[i]) + ଆକାର (ଦଶମିକ);
	  ଅନ୍ୟଥା
#endif
	    avaluep[i] = (ଅକ୍ଷର *) &fpr[i];
	}
      ଅନ୍ୟଥା
	{
	  ବାଛ (arg_types[i]->type)
	    {
	      ସ୍ଥିତି FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		ଭାଙ୍ଗ;

	      ସ୍ଥିତି FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		ଭାଙ୍ଗ;
		  
	      ସ୍ଥିତି FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		ଭାଙ୍ଗ;

	      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		avaluep[i] = (ଅକ୍ଷର *) &ar[argn];
		ଭାଙ୍ଗ;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  ଯଦି (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      ବାଛ (cif->rtype->type)
        {
        ସ୍ଥିତି FFI_TYPE_FLOAT:
          ଫେରନ୍ତୁ FFI_TYPE_INT;
        ସ୍ଥିତି FFI_TYPE_DOUBLE:
          ଫେରନ୍ତୁ FFI_TYPE_UINT64;
        ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
          ଫେରନ୍ତୁ cif->rtype->type;
        }
    }
  ଅନ୍ୟଥା
    {
      ଯଦି (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ ଅସ୍ଥିର ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      ଫେରନ୍ତୁ cif->rtype->type;
    }
}

#ଯଦି defined(FFI_MIPS_N32)

ସ୍ଥିର ଶୂନ୍ୟ
copy_struct_N32(ଅକ୍ଷର *target, unsigned offset, ffi_abi abi, ffi_type *type,
                ପୂର୍ଣ୍ଣସଂଖ୍ୟା argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float)
{
  ffi_type **elt_typep = type->elements;
  ଯେତେବେଳେ(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      ଅକ୍ଷର *tp;
      ଅକ୍ଷର *argp;
      ଅକ୍ଷର *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / ଆକାର(ffi_arg);
      arg_offset = arg_offset % ଆକାର(ffi_arg);

      argp = (ଅକ୍ଷର *)(ar + argn);
      fpp = (ଅକ୍ଷର *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      ଯଦି (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(ଦ୍ୱିଗୁଣ *)tp = *(ଦ୍ୱିଗୁଣ *)fpp;
      ଅନ୍ୟଥା
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / ଆକାର(ffi_arg);
      arg_offset = arg_offset % ଆକାର(ffi_arg);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    ଶୂନ୍ୟ (*fun)(ffi_cif*, ଶୂନ୍ୟ*, ଶୂନ୍ୟ**, ଶୂନ୍ୟ*),
                            ଶୂନ୍ୟ *user_data,
			    ଶୂନ୍ୟ *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  ଶୂନ୍ୟ **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, avn, argn;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * ଆକାର (ffi_arg));
  avaluep = alloca (cif->nargs * ଆକାର (ffi_arg));

  argn = 0;

  ଯଦି (cif->rstruct_flag)
    {
#ଯଦି _MIPS_SIM==_ABIN32
      rvalue = (ଶୂନ୍ୟ *)(UINT32)ar[0];
#ଅନ୍ୟଥା /* N64 */
      rvalue = (ଶୂନ୍ୟ *)ar[0];
#endif
      argn = 1;
    }
  ଯଦି (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ଯେତେବେଳେ (i < avn)
    {
      ଯଦି (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ଯଦି ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ଯଦି defined(__MIPSEB__) || defined(_MIPSEB)
          ଯଦି (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((ଅକ୍ଷର *) argp) + ଆକାର (ଦଶମିକ);
          ଅନ୍ୟଥା
#endif
            avaluep[i] = (ଅକ୍ଷର *) argp;
        }
      ଅନ୍ୟଥା ଯଦି (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ଅକ୍ଷର *) argp;
        }
      ଅନ୍ୟଥା ଯଦି (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (ଅକ୍ଷର *) argp;
        }
      ଅନ୍ୟଥା ଯଦି (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ଯଦି (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ଅନ୍ୟଥା
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (ଅକ୍ଷର *) argp;
        }
      ଅନ୍ୟଥା
        {
          unsigned type = arg_types[i]->type;

          ଯଦି (arg_types[i]->alignment > ଆକାର(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / ଆକାର(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ଯଦି (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ଯଦି (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          ବାଛ (type)
            {
            ସ୍ଥିତି FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              ଭାଙ୍ଗ;

            ସ୍ଥିତି FFI_TYPE_STRUCT:
              ଯଦି (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  ଭାଙ୍ଗ;
                }
              /* Else fall through.  */
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
              avaluep[i] = (ଅକ୍ଷର *) argp;
              ଭାଙ୍ଗ;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, ଆକାର(ffi_arg)) / ଆକାର(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  ଫେରନ୍ତୁ cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#ଯଦି defined(FFI_MIPS_O32)
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_O32(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_O32(ଶୂନ୍ୟ);
#ଅନ୍ୟଥା
ବାହ୍ୟ ଶୂନ୍ୟ ffi_closure_N32(ଶୂନ୍ୟ);
ବାହ୍ୟ ଶୂନ୍ୟ ffi_go_closure_N32(ଶୂନ୍ୟ);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     ଶୂନ୍ୟ (*fun)(ffi_cif*,ଶୂନ୍ୟ*,ଶୂନ୍ୟ**,ଶୂନ୍ୟ*))
{
  ଶୂନ୍ୟ * fn;

#ଯଦି defined(FFI_MIPS_O32)
  ଯଦି (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#ଅନ୍ୟଥା
#ଯଦି _MIPS_SIM ==_ABIN32
  ଯଦି (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#ଅନ୍ୟଥା
  ଯଦି (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ଫେରନ୍ତୁ FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (ଶୂନ୍ୟ *)fn;
  closure->cif = cif;
  closure->fun = fun;

  ଫେରନ୍ତୁ FFI_OK;
}

#endif /* FFI_CLOSURES */

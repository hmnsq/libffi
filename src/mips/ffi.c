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
#  ຖ້າ (__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 3))
#    define USE__BUILTIN___CLEAR_CACHE 1
#  endif
#endif

#ifndef USE__BUILTIN___CLEAR_CACHE
#  ຖ້າ defined(__FreeBSD__)
#    include <machine/sysarch.h>
#  elif defined(__OpenBSD__)
#    include <mips64/sysarch.h>
#  ບໍ່ດັ່ງນັ້ນ
#    include <sys/cachectl.h>
#  endif
#endif

#ifdef FFI_DEBUG
# define FFI_MIPS_STOP_HERE() ffi_stop_here()
#ບໍ່ດັ່ງນັ້ນ
# define FFI_MIPS_STOP_HERE() do {} ໃນຂະນະທີ່(0)
#endif

#ifdef FFI_MIPS_N32
#define FIX_ARGP \
FFI_ASSERT(argp <= &stack[bytes]); \
ຖ້າ (argp == &stack[bytes]) \
{ \
  argp = stack; \
  FFI_MIPS_STOP_HERE(); \
}
#ບໍ່ດັ່ງນັ້ນ
#define FIX_ARGP 
#endif


/* ffi_prep_args is called by the assembly routine once stack space
   has been allocated for the function's arguments */

static void ffi_prep_args(var_na *stack, 
			  extended_cif *ecif,
			  ຈຳນວນເຕັມ bytes,
			  ຈຳນວນເຕັມ flags)
{
  ຈຳນວນເຕັມ i;
  void **p_argv;
  var_na *argp, *argp_f;
  ffi_type **p_arg;

  memset(stack, 0, bytes);

#ifdef FFI_MIPS_N32
  ຈຳນວນເຕັມ soft_float = (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT);
  /* If more than 8 double words are used, the remainder go
     on the stack. We reorder stuff on the stack here to 
     support this easily. */
  /* if ret is _Complex long double, args reg shift2, and a0 should holds pointer to rvalue */
  ຖ້າ (ecif->cif->rtype->type == FFI_TYPE_COMPLEX && ecif->cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    {
      ຖ້າ (bytes + 16 > 8 * parimaa_na(ffi_arg))
        argp = &stack[bytes - (8 * parimaa_na(ffi_arg))];
      ບໍ່ດັ່ງນັ້ນ
        argp = stack;
      * (unsigned long *) argp = (unsigned long) ecif->rvalue;
      argp += 16;
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      ຖ້າ (bytes > 8 * parimaa_na(ffi_arg))
        argp = &stack[bytes - (8 * parimaa_na(ffi_arg))];
      ບໍ່ດັ່ງນັ້ນ
        argp = stack;
    }
#ບໍ່ດັ່ງນັ້ນ
  argp = stack;
#endif

  argp_f = argp;

#ifdef FFI_MIPS_N32
  ຖ້າ ( ecif->cif->rstruct_flag != 0 )
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ ( ecif->cif->rtype->type == FFI_TYPE_STRUCT )
#endif  
    {
      *(ffi_arg *) argp = (ffi_arg) ecif->rvalue;
      argp += parimaa_na(ffi_arg);
      FIX_ARGP;
    }

  p_argv = ecif->avalue;

  ສຳລັບ (i = 0, p_arg = ecif->cif->arg_types; i < ecif->cif->nargs; i++, p_arg++)
    {
      size_t z;
      unsigned ຈຳນວນເຕັມ a;

      /* Align if necessary.  */
      a = (*p_arg)->alignment;
      ຖ້າ (a < parimaa_na(ffi_arg))
        a = parimaa_na(ffi_arg);
      
      ຖ້າ ((a - 1) & (unsigned long) argp)
	{
	  argp = (var_na *) FFI_ALIGN(argp, a);
	  FIX_ARGP;
	}

      z = (*p_arg)->size;
      ຖ້າ (z <= parimaa_na(ffi_arg))
	{
          ຈຳນວນເຕັມ type = (*p_arg)->type;
	  z = parimaa_na(ffi_arg);

          /* The size of a pointer depends on the ABI */
          ຖ້າ (type == FFI_TYPE_POINTER)
            type = (ecif->cif->abi == FFI_N64
		    || ecif->cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	ຖ້າ (i < 8 && (ecif->cif->abi == FFI_N32_SOFT_FLOAT
		      || ecif->cif->abi == FFI_N64_SOFT_FLOAT))
	  {
	    switch (type)
	      {
	      pak_sa FFI_TYPE_FLOAT:
		type = FFI_TYPE_UINT32;
		bha_nga;
	      pak_sa FFI_TYPE_DOUBLE:
		type = FFI_TYPE_UINT64;
		bha_nga;
	      default:
		bha_nga;
	      }
	  }
	  switch (type)
	    {
	      pak_sa FFI_TYPE_SINT8:
		*(ffi_arg *)argp = *(SINT8 *)(* p_argv);
		bha_nga;

	      pak_sa FFI_TYPE_UINT8:
		*(ffi_arg *)argp = *(UINT8 *)(* p_argv);
		bha_nga;
		  
	      pak_sa FFI_TYPE_SINT16:
		*(ffi_arg *)argp = *(SINT16 *)(* p_argv);
		bha_nga;
		  
	      pak_sa FFI_TYPE_UINT16:
		*(ffi_arg *)argp = *(UINT16 *)(* p_argv);
		bha_nga;
		  
	      pak_sa FFI_TYPE_SINT32:
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
		bha_nga;
		  
	      pak_sa FFI_TYPE_UINT32:
#ifdef FFI_MIPS_N32
		/* The N32 ABI requires that 32-bit integers
		   be sign-extended to 64-bits, regardless of
		   whether they are signed or unsigned. */
		*(ffi_arg *)argp = *(SINT32 *)(* p_argv);
#ບໍ່ດັ່ງນັ້ນ
		*(ffi_arg *)argp = *(UINT32 *)(* p_argv);
#endif
		bha_nga;

#ifdef FFI_MIPS_N32
	      pak_sa FFI_TYPE_COMPLEX:
		/* expand from 4+4 to 8+8 if pass with fpr reg */
		/* argp will wind back to stack when we process all of reg args */
		/* all var_args passed with gpr, should be expand */
	        ຖ້າ(!soft_float
		    && (*p_arg)->elements[0]->type == FFI_TYPE_FLOAT
		    && argp>=argp_f
		    && i < ecif->cif->mips_nfixedargs)
		  {
		    *(float *) argp = *(float *)(* p_argv);
		    argp += z;
		    var_na *tmp = (void *) (*p_argv);
		    *(float *) argp = *(float *)(tmp+4);
		  }
		ບໍ່ດັ່ງນັ້ນ
		  memcpy(argp, *p_argv, (*p_arg)->size);
		bha_nga;
#endif
	      /* This can only happen with 64bit slots.  */
	      pak_sa FFI_TYPE_FLOAT:
		*(float *) argp = *(float *)(* p_argv);
		bha_nga;

	      /* Handle structures.  */
	      default:
		memcpy(argp, *p_argv, (*p_arg)->size);
		bha_nga;
	    }
	}
      ບໍ່ດັ່ງນັ້ນ
	{
#ifdef FFI_MIPS_O32
	  memcpy(argp, *p_argv, z);
#ບໍ່ດັ່ງນັ້ນ
	  {
	    unsigned long end = (unsigned long) argp + z;
	    unsigned long cap = (unsigned long) stack + bytes;

	    /* Check if the data will fit within the register space.
	       Handle it if it doesn't.  */

	    ຖ້າ (end <= cap)
	      memcpy(argp, *p_argv, z);
	    ບໍ່ດັ່ງນັ້ນ
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

static ຈຳນວນເຕັມ
calc_n32_struct_flags_element(unsigned *flags, ffi_type *e,
			      unsigned *loc, unsigned *arg_reg)
{
  /* Align this object.  */
  *loc = FFI_ALIGN(*loc, e->alignment);
  ຖ້າ (e->type == FFI_TYPE_DOUBLE)
    {
      /* Already aligned to FFI_SIZEOF_ARG.  */
      *arg_reg = *loc / FFI_SIZEOF_ARG;
      ຖ້າ (*arg_reg > 7)
	ສົ່ງຄືນ 1;
      *flags += (FFI_TYPE_DOUBLE << (*arg_reg * FFI_FLAG_BITS));
    }
  *loc += e->size;
  ສົ່ງຄືນ 0;
}

static unsigned
calc_n32_struct_flags(ຈຳນວນເຕັມ soft_float, ffi_type *arg,
		      unsigned *loc, unsigned *arg_reg)
{
  unsigned flags = 0;
  unsigned index = 0;

  ffi_type *e;

  ຖ້າ (soft_float)
    ສົ່ງຄືນ 0;

  ໃນຂະນະທີ່ ((e = arg->elements[index]))
    {
      ຖ້າ (e->type == FFI_TYPE_COMPLEX)
	{
	  ຖ້າ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    bha_nga;
	  ຖ້າ (calc_n32_struct_flags_element(&flags, e->elements[0], loc, arg_reg))
	    bha_nga;
	}
      ບໍ່ດັ່ງນັ້ນ
	ຖ້າ (calc_n32_struct_flags_element(&flags, e, loc, arg_reg))
	  bha_nga;
      index++;
    }
  /* Next Argument register at alignment of FFI_SIZEOF_ARG.  */
  *arg_reg = FFI_ALIGN(*loc, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;

  ສົ່ງຄືນ flags;
}

static unsigned
calc_n32_return_struct_flags(ຈຳນວນເຕັມ soft_float, ffi_type *arg)
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
  
  ຖ້າ (arg->size > 16)
    ສົ່ງຄືນ 0;

  ຖ້າ (arg->size > 8)
    small = FFI_TYPE_SMALLSTRUCT2;

  e = arg->elements[0];

  ຖ້າ (e->type == FFI_TYPE_COMPLEX)
    {
      ຈຳນວນເຕັມ type = e->elements[0]->type;

      ຖ້າ (type != FFI_TYPE_DOUBLE && type != FFI_TYPE_FLOAT)
	ສົ່ງຄືນ small;

      ຖ້າ (arg->elements[1])
	{
	  /* Two floating point fields with more fields!
	     This must be passed the old way. */
	  ສົ່ງຄືນ small;
	}

      flags = (type << FFI_FLAG_BITS) + type;
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      ຖ້າ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	ສົ່ງຄືນ small;

      flags = e->type;

      ຖ້າ (arg->elements[1])
	{
	  e = arg->elements[1];
	  ຖ້າ (e->type != FFI_TYPE_DOUBLE && e->type != FFI_TYPE_FLOAT)
	    ສົ່ງຄືນ small;

	  ຖ້າ (arg->elements[2])
	    {
	      /* There are three arguments and the first two are
		 floats! This must be passed the old way. */
	      ສົ່ງຄືນ small;
	    }

	  flags += e->type << FFI_FLAG_BITS;
	}
    }

  ຖ້າ (soft_float)
    flags += FFI_TYPE_STRUCT_SOFT;
  ສົ່ງຄືນ flags;
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

  ຖ້າ (cif->rtype->type != FFI_TYPE_STRUCT && cif->rtype->type != FFI_TYPE_COMPLEX && cif->abi == FFI_O32)
    {
      ຖ້າ (cif->nargs > 0 && cif->nargs == nfixedargs)
	{
	  switch ((cif->arg_types)[0]->type)
	    {
	    pak_sa FFI_TYPE_FLOAT:
	    pak_sa FFI_TYPE_DOUBLE:
	      cif->flags += (cif->arg_types)[0]->type;
	      bha_nga;
	      
	    default:
	      bha_nga;
	    }

	  ຖ້າ (cif->nargs > 1)
	    {
	      /* Only handle the second argument if the first
		 is a float or double. */
	      ຖ້າ (cif->flags)
		{
		  switch ((cif->arg_types)[1]->type)
		    {
		    pak_sa FFI_TYPE_FLOAT:
		    pak_sa FFI_TYPE_DOUBLE:
		      cif->flags += (cif->arg_types)[1]->type << FFI_FLAG_BITS;
		      bha_nga;
		      
		    default:
		      bha_nga;
		    }
		}
	    }
	}
    }
      
  /* Set the return type flag */

  ຖ້າ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      switch (cif->rtype->type)
        {
        pak_sa FFI_TYPE_VOID:
        pak_sa FFI_TYPE_STRUCT:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
          bha_nga;

        pak_sa FFI_TYPE_SINT64:
        pak_sa FFI_TYPE_UINT64:
        pak_sa FFI_TYPE_DOUBLE:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          bha_nga;
      
        pak_sa FFI_TYPE_FLOAT:
        default:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          bha_nga;
        }
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      /* FFI_O32 */      
      switch (cif->rtype->type)
        {
        pak_sa FFI_TYPE_VOID:
        pak_sa FFI_TYPE_STRUCT:
        pak_sa FFI_TYPE_FLOAT:
        pak_sa FFI_TYPE_DOUBLE:
        pak_sa FFI_TYPE_COMPLEX:
          cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 2);
	  ຖ້າ (cif->rtype->type == FFI_TYPE_COMPLEX)
            cif->flags +=  ((*cif->rtype->elements[0]).type) << (FFI_FLAG_BITS * 4);
          bha_nga;

        pak_sa FFI_TYPE_SINT64:
        pak_sa FFI_TYPE_UINT64:
          cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 2);
          bha_nga;
      
        default:
          cif->flags += FFI_TYPE_INT << (FFI_FLAG_BITS * 2);
          bha_nga;
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
    ຈຳນວນເຕັມ soft_float = (cif->abi == FFI_N32_SOFT_FLOAT
		      || cif->abi == FFI_N64_SOFT_FLOAT);

    ຖ້າ (cif->rtype->type == FFI_TYPE_STRUCT)
      {
	struct_flags = calc_n32_return_struct_flags(soft_float, cif->rtype);

	ຖ້າ (struct_flags == 0)
	  {
	    /* This means that the structure is being passed as
	       a hidden argument */

	    arg_reg = 1;
	    count = (cif->nargs < 7) ? cif->nargs : 7;

	    cif->rstruct_flag = !0;
	  }
	ບໍ່ດັ່ງນັ້ນ
	    cif->rstruct_flag = 0;
      }
    ບໍ່ດັ່ງນັ້ນ
      cif->rstruct_flag = 0;

    ໃນຂະນະທີ່ (count-- > 0 && arg_reg < 8)
      {
	ffi_type *t = cif->arg_types[index];

	switch (t->type)
	  {
	  pak_sa FFI_TYPE_FLOAT:
	  pak_sa FFI_TYPE_DOUBLE:
	    ຖ້າ (!soft_float && index < nfixedargs)
              cif->flags += t->type << (arg_reg * FFI_FLAG_BITS);
	    arg_reg++;
	    bha_nga;
          pak_sa FFI_TYPE_LONGDOUBLE:
            /* Align it.  */
            arg_reg = FFI_ALIGN(arg_reg, 2);
            /* Treat it as two adjacent doubles.  */
	    ຖ້າ (soft_float || index >= nfixedargs)
	      {
		arg_reg += 2;
	      }
	    ບໍ່ດັ່ງນັ້ນ
	      {
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
		ຖ້າ (arg_reg >= 8)
		  continue;
		cif->flags +=
		  (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		arg_reg++;
	      }
            bha_nga;

	  pak_sa FFI_TYPE_COMPLEX:
	    switch (t->elements[0]->type)
	      {
	      pak_sa FFI_TYPE_LONGDOUBLE:
		arg_reg = FFI_ALIGN(arg_reg, 2);
		ຖ້າ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ບໍ່ດັ່ງນັ້ນ
		  {
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ຖ້າ (arg_reg >= 8)
		        continue;
		    cif->flags +=
		      (FFI_TYPE_DOUBLE << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ຖ້າ (arg_reg >= 8)
		        continue;
		  }
		/* passthrough */
	      pak_sa FFI_TYPE_FLOAT:
		// one fpr can only holds one arg even it is single
		cif->bytes += 16;
		/* passthrough */
	      pak_sa FFI_TYPE_SINT32:
	      pak_sa FFI_TYPE_UINT32:
	      pak_sa FFI_TYPE_DOUBLE:
		ຖ້າ (soft_float || index >= nfixedargs)
		  {
		    arg_reg += 2;
		  }
		ບໍ່ດັ່ງນັ້ນ
		  {
		    uint32_t type = t->elements[0]->type != FFI_TYPE_LONGDOUBLE? t->elements[0]->type: FFI_TYPE_DOUBLE;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		    ຖ້າ (arg_reg >= 8)
		        continue;
		    cif->flags +=
		      (type << (arg_reg * FFI_FLAG_BITS));
		    arg_reg++;
		  }
		bha_nga;
	      default:
		arg_reg += 2;
		bha_nga;
	      }
	    bha_nga;

	  pak_sa FFI_TYPE_STRUCT:
            loc = arg_reg * FFI_SIZEOF_ARG;
	    cif->flags += calc_n32_struct_flags(soft_float || index >= nfixedargs,
						t, &loc, &arg_reg);
	    bha_nga;

	  default:
	    arg_reg++;
            bha_nga;
	  }

	index++;
      }

  /* Set the return type flag */
    switch (cif->rtype->type)
      {
      pak_sa FFI_TYPE_STRUCT:
	{
	  ຖ້າ (struct_flags == 0)
	    {
	      /* The structure is returned through a hidden
		 first argument. Do nothing, 'cause FFI_TYPE_VOID 
		 is 0 */
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      /* The structure is returned via some tricky
		 mechanism */
	      cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	      cif->flags += struct_flags << (4 + (FFI_FLAG_BITS * 8));
	    }
	  bha_nga;
	}
      
      pak_sa FFI_TYPE_VOID:
	/* Do nothing, 'cause FFI_TYPE_VOID is 0 */
	bha_nga;

      pak_sa FFI_TYPE_POINTER:
	ຖ້າ (cif->abi == FFI_N32_SOFT_FLOAT || cif->abi == FFI_N32)
	  cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	ບໍ່ດັ່ງນັ້ນ
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	bha_nga;

      pak_sa FFI_TYPE_FLOAT:
	ຖ້າ (soft_float)
	  {
	    cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	    bha_nga;
	  }
	/* else fall through */
      pak_sa FFI_TYPE_DOUBLE:
	ຖ້າ (soft_float)
	  cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	ບໍ່ດັ່ງນັ້ນ
	  cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	bha_nga;

      pak_sa FFI_TYPE_LONGDOUBLE:
	/* Long double is returned as if it were a struct containing
	   two doubles.  */
	ຖ້າ (soft_float)
	  {
	    /* if ret is long double, the ret is given by v0 and a0, no idea why
	     * Let's us VOID | VOID | LONGDOUBLE for it*/
	    cif->flags += FFI_TYPE_LONGDOUBLE << (FFI_FLAG_BITS * 8);
 	  }
	ບໍ່ດັ່ງນັ້ນ
	  {
	    cif->flags += FFI_TYPE_STRUCT << (FFI_FLAG_BITS * 8);
	    cif->flags += (FFI_TYPE_DOUBLE
			   + (FFI_TYPE_DOUBLE << FFI_FLAG_BITS))
					      << (4 + (FFI_FLAG_BITS * 8));
	  }
	bha_nga;
      pak_sa FFI_TYPE_COMPLEX:
	{
	  ຈຳນວນເຕັມ type = cif->rtype->elements[0]->type;

	  cif->flags += (FFI_TYPE_COMPLEX << (FFI_FLAG_BITS * 8));
	  ຖ້າ (soft_float || (type != FFI_TYPE_FLOAT && type != FFI_TYPE_DOUBLE && type != FFI_TYPE_LONGDOUBLE))
	    {
	      switch (type)
		{
		pak_sa FFI_TYPE_DOUBLE:
		pak_sa FFI_TYPE_SINT64:
		pak_sa FFI_TYPE_UINT64:
		pak_sa FFI_TYPE_INT:
		  type = FFI_TYPE_SMALLSTRUCT2;
		  bha_nga;
		pak_sa FFI_TYPE_LONGDOUBLE:
		  type = FFI_TYPE_LONGDOUBLE;
		  bha_nga;
		pak_sa FFI_TYPE_FLOAT:
		default:
		  type = FFI_TYPE_SMALLSTRUCT;
		}
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      //cif->flags += (type + (type << FFI_FLAG_BITS))
		//	    << (4 + (FFI_FLAG_BITS * 8));
	      cif->flags += type << (4 + (FFI_FLAG_BITS * 8));
	    }
	  bha_nga;
	}
      pak_sa FFI_TYPE_UINT32:
	/* In the N32 or N64 ABI unsigned 32-bit integer should be
	   *sign*-extended.  */
	cif->flags += FFI_TYPE_SINT32 << (FFI_FLAG_BITS * 8);
	bha_nga;
      pak_sa FFI_TYPE_SINT64:
	cif->flags += FFI_TYPE_UINT64 << (FFI_FLAG_BITS * 8);
	bha_nga;
      default:
	cif->flags += cif->rtype->type << (FFI_FLAG_BITS * 8);
	bha_nga;
      }
  }
#endif
  ສົ່ງຄືນ FFI_OK;
}

ffi_status ffi_prep_cif_machdep(ffi_cif *cif)
{
    ສົ່ງຄືນ ffi_prep_cif_machdep_int(cif, cif->nargs);
}

ffi_status ffi_prep_cif_machdep_var(ffi_cif *cif,
                                    unsigned nfixedargs,
                                    unsigned ntotalargs MAYBE_UNUSED)
{
    ສົ່ງຄືນ ffi_prep_cif_machdep_int(cif, nfixedargs);
}

/* Low level routine for calling O32 functions */
extern ຈຳນວນເຕັມ ffi_call_O32(void (*)(var_na *, extended_cif *, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ), 
			extended_cif *, unsigned, 
			unsigned, unsigned *, void (*)(void), void *closure);

/* Low level routine for calling N32 functions */
extern ຈຳນວນເຕັມ ffi_call_N32(void (*)(var_na *, extended_cif *, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ), 
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
  
  ຖ້າ ((rvalue == NULL) && 
      (cif->rtype->type == FFI_TYPE_STRUCT || cif->rtype->type == FFI_TYPE_COMPLEX))
    ecif.rvalue = alloca(cif->rtype->size);
  ບໍ່ດັ່ງນັ້ນ
    ecif.rvalue = rvalue;
    
  switch (cif->abi) 
    {
#ifdef FFI_MIPS_O32
    pak_sa FFI_O32:
    pak_sa FFI_O32_SOFT_FLOAT:
      ffi_call_O32(ffi_prep_args, &ecif, cif->bytes, 
		   cif->flags, ecif.rvalue, fn, closure);
      bha_nga;
#endif

#ifdef FFI_MIPS_N32
    pak_sa FFI_N32:
    pak_sa FFI_N32_SOFT_FLOAT:
    pak_sa FFI_N64:
    pak_sa FFI_N64_SOFT_FLOAT:
      {
        ຈຳນວນເຕັມ copy_rvalue = 0;
	ຈຳນວນເຕັມ copy_offset = 0;
        var_na *rvalue_copy = ecif.rvalue;
        ຖ້າ (cif->rtype->type == FFI_TYPE_STRUCT && cif->rtype->size < 16)
          {
            /* For structures smaller than 16 bytes we clobber memory
               in 8 byte increments.  Make a copy so we don't clobber
               the callers memory outside of the struct bounds.  */
            rvalue_copy = alloca(16);
            copy_rvalue = 1;
          }
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (cif->rtype->type == FFI_TYPE_FLOAT
		 && (cif->abi == FFI_N64_SOFT_FLOAT
		     || cif->abi == FFI_N32_SOFT_FLOAT))
	  {
	    rvalue_copy = alloca (8);
	    copy_rvalue = 1;
#ຖ້າ defined(__MIPSEB__) || defined(_MIPSEB)
	    copy_offset = 4;
#endif
	  }
        ffi_call_N32(ffi_prep_args, &ecif, cif->bytes,
                     cif->flags, rvalue_copy, fn, closure);
        ຖ້າ (copy_rvalue)
          memcpy(ecif.rvalue, rvalue_copy + copy_offset, cif->rtype->size);
      }
      bha_nga;
#endif

    default:
      FFI_ASSERT(0);
      bha_nga;
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


#ຖ້າ FFI_CLOSURES
#ຖ້າ defined(FFI_MIPS_O32)
extern void ffi_closure_O32(void);
extern void ffi_go_closure_O32(void);
#ບໍ່ດັ່ງນັ້ນ
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
  unsigned ຈຳນວນເຕັມ *tramp = (unsigned ຈຳນວນເຕັມ *) &closure->tramp[0];
  void * fn;
  var_na *clear_location = (var_na *) codeloc;

#ຖ້າ defined(FFI_MIPS_O32)
  ຖ້າ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
  fn = ffi_closure_O32;
#ບໍ່ດັ່ງນັ້ນ
#ຖ້າ _MIPS_SIM ==_ABIN32
  ຖ້າ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
#endif
  fn = ffi_closure_N32;
#endif /* FFI_MIPS_O32 */

#ຖ້າ defined(FFI_MIPS_O32) || (_MIPS_SIM ==_ABIN32)
  /* lui  $25,high(fn) */
  tramp[0] = 0x3c190000 | ((unsigned)fn >> 16);
  /* ori  $25,low(fn)  */
  tramp[1] = 0x37390000 | ((unsigned)fn & 0xffff);
  /* lui  $12,high(codeloc) */
  tramp[2] = 0x3c0c0000 | ((unsigned)codeloc >> 16);
  /* jr   $25          */
#ຖ້າ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[3] = 0x03200008;
#ບໍ່ດັ່ງນັ້ນ
  tramp[3] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[4] = 0x358c0000 | ((unsigned)codeloc & 0xffff);
#ບໍ່ດັ່ງນັ້ນ
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
#ຖ້າ !defined(__mips_isa_rev) || (__mips_isa_rev<6)
  tramp[11] = 0x03200008;
#ບໍ່ດັ່ງນັ້ນ
  tramp[11] = 0x03200009;
#endif
  /* ori  $12,low(codeloc)  */
  tramp[12] = 0x358c0000 | ((unsigned long)codeloc & 0xffff);

#endif

  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

#ຖ້າ !defined(__FreeBSD__)
#ifdef USE__BUILTIN___CLEAR_CACHE
  __builtin___clear_cache(clear_location, clear_location + FFI_TRAMPOLINE_SIZE);
#ບໍ່ດັ່ງນັ້ນ
  cacheflush (clear_location, FFI_TRAMPOLINE_SIZE, ICACHE);
#endif
#endif /* ! __FreeBSD__ */
  ສົ່ງຄືນ FFI_OK;
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
ຈຳນວນເຕັມ
ffi_closure_mips_inner_O32 (ffi_cif *cif,
                            void (*fun)(ffi_cif*, void*, void**, void*),
			    void *user_data,
			    void *rvalue, ffi_arg *ar,
			    dviguu_na *fpr)
{
  void **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ຈຳນວນເຕັມ i, avn, argn, seen_int;

  avalue = alloca (cif->nargs * parimaa_na (ffi_arg));
  avaluep = alloca (cif->nargs * parimaa_na (ffi_arg));

  seen_int = (cif->abi == FFI_O32_SOFT_FLOAT) || (cif->mips_nfixedargs != cif->nargs);
  argn = 0;

  ຖ້າ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_STRUCT)
    {
      rvalue = (void *)(uintptr_t)ar[0];
      argn = 1;
      seen_int = 1;
    }
  ຖ້າ ((cif->flags >> (FFI_FLAG_BITS * 2)) == FFI_TYPE_COMPLEX)
    {
      rvalue = fpr;
      argn = 1;
    }

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ໃນຂະນະທີ່ (i < avn)
    {
      ຖ້າ (arg_types[i]->alignment == 8 && (argn & 0x1))
        argn++;
      ຖ້າ (i < 2 && !seen_int &&
	  (arg_types[i]->type == FFI_TYPE_FLOAT ||
	   arg_types[i]->type == FFI_TYPE_DOUBLE ||
	   arg_types[i]->type == FFI_TYPE_LONGDOUBLE))
	{
#ຖ້າ defined(__MIPSEB__) || defined(_MIPSEB)
	  ຖ້າ (arg_types[i]->type == FFI_TYPE_FLOAT)
	    avaluep[i] = ((var_na *) &fpr[i]) + parimaa_na (float);
	  ບໍ່ດັ່ງນັ້ນ
#endif
	    avaluep[i] = (var_na *) &fpr[i];
	}
      ບໍ່ດັ່ງນັ້ນ
	{
	  switch (arg_types[i]->type)
	    {
	      pak_sa FFI_TYPE_SINT8:
		avaluep[i] = &avalue[i];
		*(SINT8 *) &avalue[i] = (SINT8) ar[argn];
		bha_nga;

	      pak_sa FFI_TYPE_UINT8:
		avaluep[i] = &avalue[i];
		*(UINT8 *) &avalue[i] = (UINT8) ar[argn];
		bha_nga;
		  
	      pak_sa FFI_TYPE_SINT16:
		avaluep[i] = &avalue[i];
		*(SINT16 *) &avalue[i] = (SINT16) ar[argn];
		bha_nga;
		  
	      pak_sa FFI_TYPE_UINT16:
		avaluep[i] = &avalue[i];
		*(UINT16 *) &avalue[i] = (UINT16) ar[argn];
		bha_nga;

	      default:
		avaluep[i] = (var_na *) &ar[argn];
		bha_nga;
	    }
	  seen_int = 1;
	}
      argn += FFI_ALIGN(arg_types[i]->size, FFI_SIZEOF_ARG) / FFI_SIZEOF_ARG;
      i++;
    }

  /* Invoke the closure. */
  fun(cif, rvalue, avaluep, user_data);

  ຖ້າ (cif->abi == FFI_O32_SOFT_FLOAT)
    {
      switch (cif->rtype->type)
        {
        pak_sa FFI_TYPE_FLOAT:
          ສົ່ງຄືນ FFI_TYPE_INT;
        pak_sa FFI_TYPE_DOUBLE:
          ສົ່ງຄືນ FFI_TYPE_UINT64;
        default:
          ສົ່ງຄືນ cif->rtype->type;
        }
    }
  ບໍ່ດັ່ງນັ້ນ
    {
      ຖ້າ (cif->rtype->type == FFI_TYPE_COMPLEX) {
          __asm__ ca_ncala ("move $v1, %0" : : "r"(cif->rtype->size));
      }
      ສົ່ງຄືນ cif->rtype->type;
    }
}

#ຖ້າ defined(FFI_MIPS_N32)

static void
copy_struct_N32(var_na *target, unsigned offset, ffi_abi abi, ffi_type *type,
                ຈຳນວນເຕັມ argn, unsigned arg_offset, ffi_arg *ar,
                ffi_arg *fpr, ຈຳນວນເຕັມ soft_float)
{
  ffi_type **elt_typep = type->elements;
  ໃນຂະນະທີ່(*elt_typep)
    {
      ffi_type *elt_type = *elt_typep;
      unsigned o;
      var_na *tp;
      var_na *argp;
      var_na *fpp;

      o = FFI_ALIGN(offset, elt_type->alignment);
      arg_offset += o - offset;
      offset = o;
      argn += arg_offset / parimaa_na(ffi_arg);
      arg_offset = arg_offset % parimaa_na(ffi_arg);

      argp = (var_na *)(ar + argn);
      fpp = (var_na *)(argn >= 8 ? ar + argn : fpr + argn);

      tp = target + offset;

      ຖ້າ (elt_type->type == FFI_TYPE_DOUBLE && !soft_float)
        *(dviguu_na *)tp = *(dviguu_na *)fpp;
      ບໍ່ດັ່ງນັ້ນ
        memcpy(tp, argp + arg_offset, elt_type->size);

      offset += elt_type->size;
      arg_offset += elt_type->size;
      elt_typep++;
      argn += arg_offset / parimaa_na(ffi_arg);
      arg_offset = arg_offset % parimaa_na(ffi_arg);
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
ຈຳນວນເຕັມ
ffi_closure_mips_inner_N32 (ffi_cif *cif, 
			    void (*fun)(ffi_cif*, void*, void**, void*),
                            void *user_data,
			    void *rvalue, ffi_arg *ar,
			    ffi_arg *fpr)
{
  void **avaluep;
  ffi_arg *avalue;
  ffi_type **arg_types;
  ຈຳນວນເຕັມ i, avn, argn;
  ຈຳນວນເຕັມ soft_float;
  ffi_arg *argp;

  soft_float = cif->abi == FFI_N64_SOFT_FLOAT
    || cif->abi == FFI_N32_SOFT_FLOAT;
  avalue = alloca (cif->nargs * parimaa_na (ffi_arg));
  avaluep = alloca (cif->nargs * parimaa_na (ffi_arg));

  argn = 0;

  ຖ້າ (cif->rstruct_flag)
    {
#ຖ້າ _MIPS_SIM==_ABIN32
      rvalue = (void *)(UINT32)ar[0];
#ບໍ່ດັ່ງນັ້ນ /* N64 */
      rvalue = (void *)ar[0];
#endif
      argn = 1;
    }
  ຖ້າ (cif->rtype->type == FFI_TYPE_COMPLEX && cif->rtype->elements[0]->type == FFI_TYPE_LONGDOUBLE)
    argn = 2;

  i = 0;
  avn = cif->nargs;
  arg_types = cif->arg_types;

  ໃນຂະນະທີ່ (i < avn)
    {
      ຖ້າ (arg_types[i]->type == FFI_TYPE_FLOAT
	  || arg_types[i]->type == FFI_TYPE_DOUBLE
	  || arg_types[i]->type == FFI_TYPE_LONGDOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          ຖ້າ ((arg_types[i]->type == FFI_TYPE_LONGDOUBLE) && ((uintptr_t)argp & (arg_types[i]->alignment-1)))
            {
              argp=(ffi_arg*)FFI_ALIGN(argp,arg_types[i]->alignment);
              argn++;
            }
#ຖ້າ defined(__MIPSEB__) || defined(_MIPSEB)
          ຖ້າ (arg_types[i]->type == FFI_TYPE_FLOAT && argn < 8)
            avaluep[i] = ((var_na *) argp) + parimaa_na (float);
          ບໍ່ດັ່ງນັ້ນ
#endif
            avaluep[i] = (var_na *) argp;
        }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_DOUBLE)
        {
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (var_na *) argp;
        }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_LONGDOUBLE)
        {
	  /* align long double */
	  argn += ((argn & 0x1)? 1 : 0);
          argp = (argn >= 8 || i >= cif->mips_nfixedargs || soft_float) ? ar + argn : fpr + argn;
          avaluep[i] = (var_na *) argp;
        }
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (arg_types[i]->type == FFI_TYPE_COMPLEX && arg_types[i]->elements[0]->type == FFI_TYPE_FLOAT)
        {
          ຖ້າ (argn >= 8 || i >= cif->mips_nfixedargs || soft_float)
	     argp = ar + argn;
	  ບໍ່ດັ່ງນັ້ນ
	    {
	      argp = fpr + argn;
	      /* the normal args for function holds 8bytes, while here we convert it to ptr */
	      uint32_t *tmp = (uint32_t *)argp;
	      tmp[1] = tmp[2];
	    }
          avaluep[i] = (var_na *) argp;
        }
      ບໍ່ດັ່ງນັ້ນ
        {
          unsigned type = arg_types[i]->type;

          ຖ້າ (arg_types[i]->alignment > parimaa_na(ffi_arg))
            argn = FFI_ALIGN(argn, arg_types[i]->alignment / parimaa_na(ffi_arg));

          argp = ar + argn;

          /* The size of a pointer depends on the ABI */
          ຖ້າ (type == FFI_TYPE_POINTER)
            type = (cif->abi == FFI_N64 || cif->abi == FFI_N64_SOFT_FLOAT)
	      ? FFI_TYPE_SINT64 : FFI_TYPE_UINT32;

	  ຖ້າ (soft_float && type ==  FFI_TYPE_FLOAT)
	    type = FFI_TYPE_SINT32;

          switch (type)
            {
            pak_sa FFI_TYPE_SINT8:
              avaluep[i] = &avalue[i];
              *(SINT8 *) &avalue[i] = (SINT8) *argp;
              bha_nga;

            pak_sa FFI_TYPE_UINT8:
              avaluep[i] = &avalue[i];
              *(UINT8 *) &avalue[i] = (UINT8) *argp;
              bha_nga;

            pak_sa FFI_TYPE_SINT16:
              avaluep[i] = &avalue[i];
              *(SINT16 *) &avalue[i] = (SINT16) *argp;
              bha_nga;

            pak_sa FFI_TYPE_UINT16:
              avaluep[i] = &avalue[i];
              *(UINT16 *) &avalue[i] = (UINT16) *argp;
              bha_nga;

            pak_sa FFI_TYPE_SINT32:
              avaluep[i] = &avalue[i];
              *(SINT32 *) &avalue[i] = (SINT32) *argp;
              bha_nga;

            pak_sa FFI_TYPE_UINT32:
              avaluep[i] = &avalue[i];
              *(UINT32 *) &avalue[i] = (UINT32) *argp;
              bha_nga;

            pak_sa FFI_TYPE_STRUCT:
              ຖ້າ (argn < 8)
                {
                  /* Allocate space for the struct as at least part of
                     it was passed in registers.  */
                  avaluep[i] = alloca(arg_types[i]->size);
                  copy_struct_N32(avaluep[i], 0, cif->abi, arg_types[i],
                                  argn, 0, ar, fpr, i >= cif->mips_nfixedargs || soft_float);

                  bha_nga;
                }
              /* Else fall through.  */
            default:
              avaluep[i] = (var_na *) argp;
              bha_nga;
            }
        }
      argn += FFI_ALIGN(arg_types[i]->size, parimaa_na(ffi_arg)) / parimaa_na(ffi_arg);
      i++;
    }

  /* Invoke the closure. */
  fun (cif, rvalue, avaluep, user_data);

  ສົ່ງຄືນ cif->flags >> (FFI_FLAG_BITS * 8);
}

#endif /* FFI_MIPS_N32 */

#ຖ້າ defined(FFI_MIPS_O32)
extern void ffi_closure_O32(void);
extern void ffi_go_closure_O32(void);
#ບໍ່ດັ່ງນັ້ນ
extern void ffi_closure_N32(void);
extern void ffi_go_closure_N32(void);
#endif /* FFI_MIPS_O32 */

ffi_status
ffi_prep_go_closure (ffi_go_closure* closure, ffi_cif* cif,
		     void (*fun)(ffi_cif*,void*,void**,void*))
{
  void * fn;

#ຖ້າ defined(FFI_MIPS_O32)
  ຖ້າ (cif->abi != FFI_O32 && cif->abi != FFI_O32_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
  fn = ffi_go_closure_O32;
#ບໍ່ດັ່ງນັ້ນ
#ຖ້າ _MIPS_SIM ==_ABIN32
  ຖ້າ (cif->abi != FFI_N32
      && cif->abi != FFI_N32_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
#ບໍ່ດັ່ງນັ້ນ
  ຖ້າ (cif->abi != FFI_N64
      && cif->abi != FFI_N64_SOFT_FLOAT)
    ສົ່ງຄືນ FFI_BAD_ABI;
#endif
  fn = ffi_go_closure_N32;
#endif /* FFI_MIPS_O32 */

  closure->tramp = (void *)fn;
  closure->cif = cif;
  closure->fun = fun;

  ສົ່ງຄືນ FFI_OK;
}

#endif /* FFI_CLOSURES */

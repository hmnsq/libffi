/* -----------------------------------------------------------------------
   ffi.c - Copyright (c) 2000, 2007 Software AG
           Copyright (c) 2008 Red Hat, Inc

   S390 Foreign Function Interface

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   ``Software''), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED ``AS IS'', WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
   ----------------------------------------------------------------------- */
/*====================================================================*/
/*                          Includes                                  */
/*                          --------                                  */
/*====================================================================*/

#ਅਨ੍ਤਰ੍ਭਾਵ <ffi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ffi_common.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdint.h>
#ਅਨ੍ਤਰ੍ਭਾਵ "internal.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*ਨਿਰਧਾਰਤ_ਕਰਦਾ*/
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_FPRARGS 4
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_FPRARGS 2
#ਜੇ_ਅੰਤ

/* Round to multiple of 16.  */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਗੇੜਾ_ਮਾਪ(ਮਾਪ) (((ਮਾਪ) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ਬਣਤਰ call_ਚੌਖਟਾ
{
  ਖਾਲੀ *ਪਿੱਠ_chain;
  ਖਾਲੀ *eos;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ gpr_ਦਲੀਲਾਂ[5];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ gpr_save[9];
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ fpr_ਦਲੀਲਾਂ[4];
};

ਬਾਹਰੀ ਖਾਲੀ FFI_HIDDEN ffi_call_SYSV(ਬਣਤਰ call_ਚੌਖਟਾ *, ਬਿਨਾਨਿਸ਼ਾਨੀ, ਖਾਲੀ *,
			             ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *);

ਬਾਹਰੀ ਖਾਲੀ ffi_ਘੇਰਾ_SYSV(ਖਾਲੀ);
ਬਾਹਰੀ ਖਾਲੀ ffi_go_ਘੇਰਾ_SYSV(ਖਾਲੀ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ਸਥਿਰ ਪੂਰਨ
ffi_check_struct_type (ffi_type *arg)
{
  size_t ਮਾਪ = arg->ਮਾਪ;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ਜਦੋਂ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  ਚੋਣ (ਮਾਪ)
    {
      ਹਾਲਤ 1:
        ਵਾਪਸ FFI_TYPE_UINT8;

      ਹਾਲਤ 2:
        ਵਾਪਸ FFI_TYPE_UINT16;

      ਹਾਲਤ 4:
	ਜੇ (arg->type == FFI_TYPE_FLOAT)
          ਵਾਪਸ FFI_TYPE_FLOAT;
	ਨਹੀਂਤਾਂ
	  ਵਾਪਸ FFI_TYPE_UINT32;

      ਹਾਲਤ 8:
	ਜੇ (arg->type == FFI_TYPE_DOUBLE)
          ਵਾਪਸ FFI_TYPE_DOUBLE;
	ਨਹੀਂਤਾਂ
	  ਵਾਪਸ FFI_TYPE_UINT64;

      ਮੂਲ:
	ਰੋਕੋ;
    }

  /* Other structs are passed via a pointer to the data.  */
  ਵਾਪਸ FFI_TYPE_ਸੂਚਕ;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_ਸਥਿਤੀ FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_ਮਾਪ = 0;
  ਪੂਰਨ n_gpr = 0;
  ਪੂਰਨ n_fpr = 0;
  ਪੂਰਨ n_ov = 0;

  ffi_type **ptr;
  ਪੂਰਨ i;

  /* Determine return value handling.  */

  ਚੋਣ (cif->rtype->type)
    {
      /* Void is easy.  */
      ਹਾਲਤ FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	ਰੋਕੋ;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      ਹਾਲਤ FFI_TYPE_STRUCT:
      ਹਾਲਤ FFI_TYPE_COMPLEX:
      ਹਾਲਤ FFI_TYPE_SINT128:
      ਹਾਲਤ FFI_TYPE_UINT128:
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਅੰਤ
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	ਰੋਕੋ;

      /* Floating point values are returned in fpr 0.  */
      ਹਾਲਤ FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	ਰੋਕੋ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ਹਾਲਤ FFI_TYPE_UINT64:
      ਹਾਲਤ FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਹਾਲਤ FFI_TYPE_INT:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_UINT16:
      ਹਾਲਤ FFI_TYPE_SINT16:
      ਹਾਲਤ FFI_TYPE_UINT8:
      ਹਾਲਤ FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	cif->flags = FFI390_RET_INT64;
#ਨਹੀਂਤਾਂ
	cif->flags = FFI390_RET_INT32;
#ਜੇ_ਅੰਤ
	ਰੋਕੋ;

      ਮੂਲ:
        FFI_ASSERT (0);
        ਰੋਕੋ;
    }

  /* Now for the arguments.  */

  ਲਈ (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      ਪੂਰਨ type = (*ptr)->type;

      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ਜੇ (type != FFI_TYPE_ਸੂਚਕ)
	    ਰੋਕੋ;
	  /* fall through */

	ਹਾਲਤ FFI_TYPE_COMPLEX:
	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਅੰਤ
	  type = FFI_TYPE_ਸੂਚਕ;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_ਮਾਪ += ਗੇੜਾ_ਮਾਪ ((*ptr)->ਮਾਪ);
	  ਰੋਕੋ;
	}

      /* Now handle all primitive int/float data types.  */
      ਚੋਣ (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ਨਹੀਂਤਾਂ
	      n_ov += ਆਕਾਰ (ਦੁੱਗਣਾ) / ਆਕਾਰ (ਲੰਮਾ);
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਜੇ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ਨਹੀਂਤਾਂ
	      n_ov++;
	    ਰੋਕੋ;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __s390x__
	  ਹਾਲਤ FFI_TYPE_UINT64:
	  ਹਾਲਤ FFI_TYPE_SINT64:
	    ਜੇ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ਨਹੀਂਤਾਂ
	      n_ov += 2;
	    ਰੋਕੋ;
#ਜੇ_ਅੰਤ

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  ਮੂਲ:
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ਨਹੀਂਤਾਂ
	      n_ov++;
	    ਰੋਕੋ;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->ਬਾਈਟਾਂ = ਗੇੜਾ_ਮਾਪ (n_ov * ਆਕਾਰ (ਲੰਮਾ)) + struct_ਮਾਪ;

  ਵਾਪਸ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ਸਥਿਰ ਖਾਲੀ
ffi_call_int(ffi_cif *cif,
	     ਖਾਲੀ (*fn)(ਖਾਲੀ),
	     ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue,
	     ਖਾਲੀ *ਘੇਰਾ)
{
  ਪੂਰਨ ret_type = cif->flags;
  size_t rsize = 0, ਬਾਈਟਾਂ = cif->ਬਾਈਟਾਂ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਢੇਰ, *p_struct;
  ਬਣਤਰ call_ਚੌਖਟਾ *ਚੌਖਟਾ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *p_ov, *p_gpr;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ *p_fpr;
  ਪੂਰਨ n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ਜੇ (rvalue == ਨਿਰਾ)
    {
      ਜੇ (ret_type & FFI390_RET_ਵਿੱਚ_MEM)
	rsize = cif->rtype->ਮਾਪ;
      ਨਹੀਂਤਾਂ
	ret_type = FFI390_RET_VOID;
    }

  /* The stack space will be filled with those areas:

	dummy structure return		    (highest addresses)
	  FPR argument register save area
	  GPR argument register save area
	stack frame for ffi_call_SYSV
	temporary struct copies
	overflow argument area              (lowest addresses)

     We set up the following pointers:

        p_fpr: bottom of the FPR area (growing upwards)
	p_gpr: bottom of the GPR area (growing upwards)
	p_ov: bottom of the overflow area (growing upwards)
	p_struct: top of the struct copy area (growing downwards)

     All areas are kept aligned to twice the word size.

     Note that we're going to create the stack frame for both
     ffi_call_SYSV _and_ the target function right here.  This
     works because we don't make any function calls with more
     than 5 arguments (indeed only memcpy and ffi_call_SYSV),
     and thus we don't have any stacked outgoing parameters.  */

  ਢੇਰ = alloca (ਬਾਈਟਾਂ + ਆਕਾਰ(ਬਣਤਰ call_ਚੌਖਟਾ) + rsize);
  ਚੌਖਟਾ = (ਬਣਤਰ call_ਚੌਖਟਾ *)(ਢੇਰ + ਬਾਈਟਾਂ);
  ਜੇ (rsize)
    rvalue = ਚੌਖਟਾ + 1;

  /* Link the new frame back to the one from this function.  */
  ਚੌਖਟਾ->ਪਿੱਠ_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)ਢੇਰ;
  p_struct = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *)ਚੌਖਟਾ;
  p_gpr = ਚੌਖਟਾ->gpr_ਦਲੀਲਾਂ;
  p_fpr = ਚੌਖਟਾ->fpr_ਦਲੀਲਾਂ;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ਜੇ (cif->flags & FFI390_RET_ਵਿੱਚ_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  ਲਈ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      ਖਾਲੀ *arg = avalue[i];
      ਪੂਰਨ type = ty->type;
      ffi_arg val;

    restart:
      ਚੋਣ (type)
	{
	ਹਾਲਤ FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_INT:
	ਹਾਲਤ FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  ਜਾਓ do_int;
	ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_UINT64:
	ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	  val = *(UINT64 *)arg;
	  ਜਾਓ do_int;
#ਨਹੀਂਤਾਂ
	  ਜੇ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ਜੇ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ਨਹੀਂਤਾਂ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_DOUBLE:
	  ਜੇ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ਨਹੀਂਤਾਂ
	    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ਨਹੀਂਤਾਂ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#ਜੇ_ਅੰਤ
	    }
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ਜੇ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ਨਹੀਂਤਾਂ
	    p_ov[n_ov++] = val;
	  ਰੋਕੋ;

	ਹਾਲਤ FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ਜੇ (type != FFI_TYPE_ਸੂਚਕ)
	    ਜਾਓ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਅੰਤ
	ਹਾਲਤ FFI_TYPE_COMPLEX:
	ਹਾਲਤ FFI_TYPE_SINT128:
	ਹਾਲਤ FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ਗੇੜਾ_ਮਾਪ (ty->ਮਾਪ);
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (p_struct, arg, ty->ਮਾਪ);
	  val = (uintptr_t)p_struct;
	  ਜਾਓ do_int;

	ਮੂਲ:
	  FFI_ASSERT (0);
	  ਰੋਕੋ;
        }
    }

  ffi_call_SYSV (ਚੌਖਟਾ, ret_type & FFI360_RET_ਨਕਾਬ, rvalue, fn, ਘੇਰਾ);
}

ਖਾਲੀ
ffi_call (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue, ਖਾਲੀ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਨਿਰਾ);
}

ਖਾਲੀ
ffi_call_go (ffi_cif *cif, ਖਾਲੀ (*fn)(ਖਾਲੀ), ਖਾਲੀ *rvalue,
	     ਖਾਲੀ **avalue, ਖਾਲੀ *ਘੇਰਾ)
{
  ffi_call_int(cif, fn, rvalue, avalue, ਘੇਰਾ);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

ਖਾਲੀ FFI_HIDDEN
ffi_ਘੇਰਾ_helper_SYSV (ffi_cif *cif,
			 ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*),
			 ਖਾਲੀ *user_ਅੰਕੜੇ,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *p_gpr,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ *p_fpr,
			 ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *p_ov)
{
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ret_ਬਫਰ;

  ਖਾਲੀ *rvalue = &ret_ਬਫਰ;
  ਖਾਲੀ **avalue;
  ਖਾਲੀ **p_arg;

  ਪੂਰਨ n_gpr = 0;
  ਪੂਰਨ n_fpr = 0;
  ਪੂਰਨ n_ov = 0;

  ffi_type **ptr;
  ਪੂਰਨ i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * ਆਕਾਰ (ਖਾਲੀ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ਜੇ (cif->flags & FFI390_RET_ਵਿੱਚ_MEM)
    rvalue = (ਖਾਲੀ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  ਲਈ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      ਪੂਰਨ deref_struct_ਸੂਚਕ = 0;
      ਪੂਰਨ type = (*ptr)->type;

#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ਜੇ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#ਜੇ_ਅੰਤ

      /* Check how a structure type is passed.  */
      ਜੇ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ਜੇ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_ਸੂਚਕ;
	  ਨਹੀਂਤਾਂ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ਜੇ (type == FFI_TYPE_ਸੂਚਕ)
	    deref_struct_ਸੂਚਕ = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ਜੇ (type == FFI_TYPE_ਸੂਚਕ)
	{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	  type = FFI_TYPE_UINT64;
#ਨਹੀਂਤਾਂ
	  type = FFI_TYPE_UINT32;
#ਜੇ_ਅੰਤ
	}

      /* Now handle all primitive int/float data types.  */
      ਚੋਣ (type)
	{
	  ਹਾਲਤ FFI_TYPE_DOUBLE:
	    ਜੇ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ਨਹੀਂਤਾਂ
	      *p_arg = &p_ov[n_ov],
	      n_ov += ਆਕਾਰ (ਦੁੱਗਣਾ) / ਆਕਾਰ (ਲੰਮਾ);
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_FLOAT:
	    ਜੇ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ਨਹੀਂਤਾਂ
	      *p_arg = (ਅੱਖਰ *)&p_ov[n_ov++] + ਆਕਾਰ (ਲੰਮਾ) - 4;
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_UINT64:
	  ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ਨਹੀਂਤਾਂ
	      *p_arg = &p_ov[n_ov++];
#ਨਹੀਂਤਾਂ
	    ਜੇ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ਨਹੀਂਤਾਂ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#ਜੇ_ਅੰਤ
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_INT:
	  ਹਾਲਤ FFI_TYPE_UINT32:
	  ਹਾਲਤ FFI_TYPE_SINT32:
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ਅੱਖਰ *)&p_gpr[n_gpr++] + ਆਕਾਰ (ਲੰਮਾ) - 4;
	    ਨਹੀਂਤਾਂ
	      *p_arg = (ਅੱਖਰ *)&p_ov[n_ov++] + ਆਕਾਰ (ਲੰਮਾ) - 4;
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_UINT16:
	  ਹਾਲਤ FFI_TYPE_SINT16:
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ਅੱਖਰ *)&p_gpr[n_gpr++] + ਆਕਾਰ (ਲੰਮਾ) - 2;
	    ਨਹੀਂਤਾਂ
	      *p_arg = (ਅੱਖਰ *)&p_ov[n_ov++] + ਆਕਾਰ (ਲੰਮਾ) - 2;
	    ਰੋਕੋ;

	  ਹਾਲਤ FFI_TYPE_UINT8:
	  ਹਾਲਤ FFI_TYPE_SINT8:
	    ਜੇ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ਅੱਖਰ *)&p_gpr[n_gpr++] + ਆਕਾਰ (ਲੰਮਾ) - 1;
	    ਨਹੀਂਤਾਂ
	      *p_arg = (ਅੱਖਰ *)&p_ov[n_ov++] + ਆਕਾਰ (ਲੰਮਾ) - 1;
	    ਰੋਕੋ;

	  ਮੂਲ:
	    FFI_ASSERT (0);
	    ਰੋਕੋ;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ਜੇ (deref_struct_ਸੂਚਕ)
	*p_arg = *(ਖਾਲੀ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_ਅੰਕੜੇ);

  /* Convert the return value.  */
  ਚੋਣ (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      ਹਾਲਤ FFI_TYPE_VOID:
      ਹਾਲਤ FFI_TYPE_STRUCT:
      ਹਾਲਤ FFI_TYPE_COMPLEX:
#ਜੇ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ਹਾਲਤ FFI_TYPE_LONGDOUBLE:
#ਜੇ_ਅੰਤ
	ਰੋਕੋ;

      /* Floating point values are returned in fpr 0.  */
      ਹਾਲਤ FFI_TYPE_FLOAT:
	p_fpr[0] = (ਲੰਮਾ ਲੰਮਾ) *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *) rvalue << 32;
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_DOUBLE:
	p_fpr[0] = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ *) rvalue;
	ਰੋਕੋ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ਹਾਲਤ FFI_TYPE_UINT64:
      ਹਾਲਤ FFI_TYPE_SINT64:
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __s390x__
	p_gpr[0] = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rvalue;
#ਨਹੀਂਤਾਂ
	p_gpr[0] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rvalue)[0],
	p_gpr[1] = ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rvalue)[1];
#ਜੇ_ਅੰਤ
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_ਸੂਚਕ:
      ਹਾਲਤ FFI_TYPE_UINT32:
      ਹਾਲਤ FFI_TYPE_UINT16:
      ਹਾਲਤ FFI_TYPE_UINT8:
	p_gpr[0] = *(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *) rvalue;
	ਰੋਕੋ;

      ਹਾਲਤ FFI_TYPE_INT:
      ਹਾਲਤ FFI_TYPE_SINT32:
      ਹਾਲਤ FFI_TYPE_SINT16:
      ਹਾਲਤ FFI_TYPE_SINT8:
	p_gpr[0] = *(ਨਿਸ਼ਾਨੀਵਾਲਾ ਲੰਮਾ *) rvalue;
	ਰੋਕੋ;

      ਮੂਲ:
        FFI_ASSERT (0);
        ਰੋਕੋ;
    }
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_closure_loc.                                   */
/*                                                                    */
/* Function - Prepare a FFI closure.                                  */
/*                                                                    */
/*====================================================================*/

ffi_ਸਥਿਤੀ
ffi_prep_ਘੇਰਾ_loc (ffi_ਘੇਰਾ *ਘੇਰਾ,
		      ffi_cif *cif,
		      ਖਾਲੀ (*fun) (ffi_cif *, ਖਾਲੀ *, ਖਾਲੀ **, ਖਾਲੀ *),
		      ਖਾਲੀ *user_ਅੰਕੜੇ,
		      ਖਾਲੀ *codeloc)
{
  ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ ਅਚਲ ਸਾਂਚਾ[] = {
    0x0d10,			/* basr %r1,0 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ਨਹੀਂਤਾਂ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#ਜੇ_ਅੰਤ
    0x07f1			/* br %r1 */
  };
  ਖਾਲੀ (*dest)(ਖਾਲੀ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *tramp = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *)&ਘੇਰਾ->tramp;

  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;


#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
  ਜੇ (ffi_tramp_is_present(ਘੇਰਾ))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_ਘੇਰਾ_SYSV;
      ffi_tramp_ਸੈੱਟ_parms (ਘੇਰਾ->ftramp, dest, ਘੇਰਾ);
      ਜਾਓ ਬਾਹਰਲਾ;
    }
#ਜੇ_ਅੰਤ

  ਸਮ੍ਰਿਤੀ_ਨਕਲ (tramp, ਸਾਂਚਾ, ਆਕਾਰ(ਸਾਂਚਾ));
  tramp[2] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)codeloc;
  tramp[3] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)&ffi_ਘੇਰਾ_SYSV;

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਬਾਹਰਲਾ:
#ਜੇ_ਅੰਤ
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;
  ਘੇਰਾ->user_ਅੰਕੜੇ = user_ਅੰਕੜੇ;

  ਵਾਪਸ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_ਸਥਿਤੀ
ffi_prep_go_ਘੇਰਾ (ffi_go_ਘੇਰਾ *ਘੇਰਾ, ffi_cif *cif,
		     ਖਾਲੀ (*fun)(ffi_cif*,ਖਾਲੀ*,ਖਾਲੀ**,ਖਾਲੀ*))
{
  ਜੇ (cif->abi != FFI_SYSV)
    ਵਾਪਸ FFI_BAD_ABI;

  ਘੇਰਾ->tramp = ffi_go_ਘੇਰਾ_SYSV;
  ਘੇਰਾ->cif = cif;
  ਘੇਰਾ->fun = fun;

  ਵਾਪਸ FFI_OK;
}

#ਜੇ defined(FFI_EXEC_STATIC_TRAMP)
ਖਾਲੀ *
ffi_tramp_arch (size_t *tramp_ਮਾਪ, size_t *map_ਮਾਪ)
{
  ਬਾਹਰੀ ਖਾਲੀ *trampoline_ਕੋਡ_ਸਾਰਣੀ;

  *tramp_ਮਾਪ = FFI390_TRAMP_ਮਾਪ;
  *map_ਮਾਪ = FFI390_TRAMP_MAP_ਮਾਪ;
  ਵਾਪਸ &trampoline_ਕੋਡ_ਸਾਰਣੀ;
}
#ਜੇ_ਅੰਤ

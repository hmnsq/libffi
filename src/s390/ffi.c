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

#ಅನ್ತರ್ಭಾವ <ffi.h>
#ಅನ್ತರ್ಭಾವ <ffi_common.h>
#ಅನ್ತರ್ಭಾವ <stdint.h>
#ಅನ್ತರ್ಭಾವ "internal.h"
#ಅನ್ತರ್ಭಾವ <tramp.h>

/*====================== End of Includes =============================*/

/*====================================================================*/
/*                           Defines                                  */
/*                           -------                                  */
/*====================================================================*/

/* Maximum number of GPRs available for argument passing.  */
#define MAX_GPRARGS 5

/* Maximum number of FPRs available for argument passing.  */
#ifdef __s390x__
#define MAX_FPRARGS 4
#ಅನ್ಯಥಾ
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ಸಂರಚನಾ call_frame
{
  ಶೂನ್ಯ *back_chain;
  ಶೂನ್ಯ *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

ಬಾಹ್ಯ ಶೂನ್ಯ FFI_HIDDEN ffi_call_SYSV(ಸಂರಚನಾ call_frame *, unsigned, ಶೂನ್ಯ *,
			             ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *);

ಬಾಹ್ಯ ಶೂನ್ಯ ffi_closure_SYSV(ಶೂನ್ಯ);
ಬಾಹ್ಯ ಶೂನ್ಯ ffi_go_closure_SYSV(ಶೂನ್ಯ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  ಯಾವತ್ (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  ವಿಕಲ್ಪ (size)
    {
      ಪಕ್ಷ 1:
        ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT8;

      ಪಕ್ಷ 2:
        ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT16;

      ಪಕ್ಷ 4:
	ಯದಿ (arg->type == FFI_TYPE_FLOAT)
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_FLOAT;
	ಅನ್ಯಥಾ
	  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT32;

      ಪಕ್ಷ 8:
	ಯದಿ (arg->type == FFI_TYPE_DOUBLE)
          ಪ್ರತ್ಯಾಗಮ FFI_TYPE_DOUBLE;
	ಅನ್ಯಥಾ
	  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_UINT64;

      ಸ್ವಭಾವ:
	ಭಙ್ಗ;
    }

  /* Other structs are passed via a pointer to the data.  */
  ಪ್ರತ್ಯಾಗಮ FFI_TYPE_POINTER;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_prep_cif_machdep.                                   */
/*                                                                    */
/* Function - Perform machine dependent CIF processing.               */
/*                                                                    */
/*====================================================================*/

ffi_status FFI_HIDDEN
ffi_prep_cif_machdep(ffi_cif *cif)
{
  size_t struct_size = 0;
  ಪೂರ್ಣಾಙ್ಕ n_gpr = 0;
  ಪೂರ್ಣಾಙ್ಕ n_fpr = 0;
  ಪೂರ್ಣಾಙ್ಕ n_ov = 0;

  ffi_type **ptr;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* Determine return value handling.  */

  ವಿಕಲ್ಪ (cif->rtype->type)
    {
      /* Void is easy.  */
      ಪಕ್ಷ FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	ಭಙ್ಗ;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಪಕ್ಷ FFI_TYPE_COMPLEX:
      ಪಕ್ಷ FFI_TYPE_SINT128:
      ಪಕ್ಷ FFI_TYPE_UINT128:
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	ಭಙ್ಗ;

      /* Floating point values are returned in fpr 0.  */
      ಪಕ್ಷ FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	ಭಙ್ಗ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ಪಕ್ಷ FFI_TYPE_UINT64:
      ಪಕ್ಷ FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_POINTER:
      ಪಕ್ಷ FFI_TYPE_INT:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪಕ್ಷ FFI_TYPE_UINT8:
      ಪಕ್ಷ FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#ಅನ್ಯಥಾ
	cif->flags = FFI390_RET_INT32;
#endif
	ಭಙ್ಗ;

      ಸ್ವಭಾವ:
        FFI_ASSERT (0);
        ಭಙ್ಗ;
    }

  /* Now for the arguments.  */

  ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      ಪೂರ್ಣಾಙ್ಕ type = (*ptr)->type;

      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  ಯದಿ (type != FFI_TYPE_POINTER)
	    ಭಙ್ಗ;
	  /* fall through */

	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	ಪಕ್ಷ FFI_TYPE_SINT128:
	ಪಕ್ಷ FFI_TYPE_UINT128:
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  ಭಙ್ಗ;
	}

      /* Now handle all primitive int/float data types.  */
      ವಿಕಲ್ಪ (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ಅನ್ಯಥಾ
	      n_ov += ಪರಿಮಾಣ (ದ್ವಿಗುಣ) / ಪರಿಮಾಣ (long);
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಯದಿ (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    ಅನ್ಯಥಾ
	      n_ov++;
	    ಭಙ್ಗ;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  ಪಕ್ಷ FFI_TYPE_UINT64:
	  ಪಕ್ಷ FFI_TYPE_SINT64:
	    ಯದಿ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    ಅನ್ಯಥಾ
	      n_ov += 2;
	    ಭಙ್ಗ;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  ಸ್ವಭಾವ:
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    ಅನ್ಯಥಾ
	      n_ov++;
	    ಭಙ್ಗ;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * ಪರಿಮಾಣ (long)) + struct_size;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ಸ್ಥಿರ ಶೂನ್ಯ
ffi_call_int(ffi_cif *cif,
	     ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ),
	     ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue,
	     ಶೂನ್ಯ *closure)
{
  ಪೂರ್ಣಾಙ್ಕ ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned ವರ್ಣ *ರಾಶಿ, *p_struct;
  ಸಂರಚನಾ call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  ಪೂರ್ಣಾಙ್ಕ n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  ಯದಿ (rvalue == ರಿಕ್ತ)
    {
      ಯದಿ (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      ಅನ್ಯಥಾ
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

  ರಾಶಿ = alloca (bytes + ಪರಿಮಾಣ(ಸಂರಚನಾ call_frame) + rsize);
  frame = (ಸಂರಚನಾ call_frame *)(ರಾಶಿ + bytes);
  ಯದಿ (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)ರಾಶಿ;
  p_struct = (unsigned ವರ್ಣ *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  ಯದಿ (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  ಪ್ರತಿ (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      ಶೂನ್ಯ *arg = avalue[i];
      ಪೂರ್ಣಾಙ್ಕ type = ty->type;
      ffi_arg val;

    restart:
      ವಿಕಲ್ಪ (type)
	{
	ಪಕ್ಷ FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_INT:
	ಪಕ್ಷ FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  ಗಚ್ಛ do_int;
	ಪಕ್ಷ FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_UINT64:
	ಪಕ್ಷ FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  ಗಚ್ಛ do_int;
#ಅನ್ಯಥಾ
	  ಯದಿ (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  ಯದಿ (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  ಅನ್ಯಥಾ
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_DOUBLE:
	  ಯದಿ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  ಅನ್ಯಥಾ
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#ಅನ್ಯಥಾ
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  ಯದಿ (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  ಅನ್ಯಥಾ
	    p_ov[n_ov++] = val;
	  ಭಙ್ಗ;

	ಪಕ್ಷ FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  ಯದಿ (type != FFI_TYPE_POINTER)
	    ಗಚ್ಛ restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#endif
	ಪಕ್ಷ FFI_TYPE_COMPLEX:
	ಪಕ್ಷ FFI_TYPE_SINT128:
	ಪಕ್ಷ FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  ಗಚ್ಛ do_int;

	ಸ್ವಭಾವ:
	  FFI_ASSERT (0);
	  ಭಙ್ಗ;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

ಶೂನ್ಯ
ffi_call (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue, ಶೂನ್ಯ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ರಿಕ್ತ);
}

ಶೂನ್ಯ
ffi_call_go (ffi_cif *cif, ಶೂನ್ಯ (*fn)(ಶೂನ್ಯ), ಶೂನ್ಯ *rvalue,
	     ಶೂನ್ಯ **avalue, ಶೂನ್ಯ *closure)
{
  ffi_call_int(cif, fn, rvalue, avalue, closure);
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_closure_helper_SYSV.                                */
/*                                                                    */
/* Function - Call a FFI closure target function.                     */
/*                                                                    */
/*====================================================================*/

ಶೂನ್ಯ FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*),
			 ಶೂನ್ಯ *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  ಶೂನ್ಯ *rvalue = &ret_buffer;
  ಶೂನ್ಯ **avalue;
  ಶೂನ್ಯ **p_arg;

  ಪೂರ್ಣಾಙ್ಕ n_gpr = 0;
  ಪೂರ್ಣಾಙ್ಕ n_fpr = 0;
  ಪೂರ್ಣಾಙ್ಕ n_ov = 0;

  ffi_type **ptr;
  ಪೂರ್ಣಾಙ್ಕ i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * ಪರಿಮಾಣ (ಶೂನ್ಯ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  ಯದಿ (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (ಶೂನ್ಯ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  ಪ್ರತಿ (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      ಪೂರ್ಣಾಙ್ಕ deref_struct_pointer = 0;
      ಪೂರ್ಣಾಙ್ಕ type = (*ptr)->type;

#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      ಯದಿ (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      ಯದಿ (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  ಯದಿ (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  ಅನ್ಯಥಾ
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  ಯದಿ (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      ಯದಿ (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#ಅನ್ಯಥಾ
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      ವಿಕಲ್ಪ (type)
	{
	  ಪಕ್ಷ FFI_TYPE_DOUBLE:
	    ಯದಿ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ಅನ್ಯಥಾ
	      *p_arg = &p_ov[n_ov],
	      n_ov += ಪರಿಮಾಣ (ದ್ವಿಗುಣ) / ಪರಿಮಾಣ (long);
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_FLOAT:
	    ಯದಿ (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    ಅನ್ಯಥಾ
	      *p_arg = (ವರ್ಣ *)&p_ov[n_ov++] + ಪರಿಮಾಣ (long) - 4;
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_UINT64:
	  ಪಕ್ಷ FFI_TYPE_SINT64:
#ifdef __s390x__
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    ಅನ್ಯಥಾ
	      *p_arg = &p_ov[n_ov++];
#ಅನ್ಯಥಾ
	    ಯದಿ (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    ಅನ್ಯಥಾ
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_INT:
	  ಪಕ್ಷ FFI_TYPE_UINT32:
	  ಪಕ್ಷ FFI_TYPE_SINT32:
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ವರ್ಣ *)&p_gpr[n_gpr++] + ಪರಿಮಾಣ (long) - 4;
	    ಅನ್ಯಥಾ
	      *p_arg = (ವರ್ಣ *)&p_ov[n_ov++] + ಪರಿಮಾಣ (long) - 4;
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_UINT16:
	  ಪಕ್ಷ FFI_TYPE_SINT16:
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ವರ್ಣ *)&p_gpr[n_gpr++] + ಪರಿಮಾಣ (long) - 2;
	    ಅನ್ಯಥಾ
	      *p_arg = (ವರ್ಣ *)&p_ov[n_ov++] + ಪರಿಮಾಣ (long) - 2;
	    ಭಙ್ಗ;

	  ಪಕ್ಷ FFI_TYPE_UINT8:
	  ಪಕ್ಷ FFI_TYPE_SINT8:
	    ಯದಿ (n_gpr < MAX_GPRARGS)
	      *p_arg = (ವರ್ಣ *)&p_gpr[n_gpr++] + ಪರಿಮಾಣ (long) - 1;
	    ಅನ್ಯಥಾ
	      *p_arg = (ವರ್ಣ *)&p_ov[n_ov++] + ಪರಿಮಾಣ (long) - 1;
	    ಭಙ್ಗ;

	  ಸ್ವಭಾವ:
	    FFI_ASSERT (0);
	    ಭಙ್ಗ;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      ಯದಿ (deref_struct_pointer)
	*p_arg = *(ಶೂನ್ಯ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  ವಿಕಲ್ಪ (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      ಪಕ್ಷ FFI_TYPE_VOID:
      ಪಕ್ಷ FFI_TYPE_STRUCT:
      ಪಕ್ಷ FFI_TYPE_COMPLEX:
#ಯದಿ FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ಪಕ್ಷ FFI_TYPE_LONGDOUBLE:
#endif
	ಭಙ್ಗ;

      /* Floating point values are returned in fpr 0.  */
      ಪಕ್ಷ FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned ಪೂರ್ಣಾಙ್ಕ *) rvalue << 32;
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	ಭಙ್ಗ;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ಪಕ್ಷ FFI_TYPE_UINT64:
      ಪಕ್ಷ FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#ಅನ್ಯಥಾ
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_POINTER:
      ಪಕ್ಷ FFI_TYPE_UINT32:
      ಪಕ್ಷ FFI_TYPE_UINT16:
      ಪಕ್ಷ FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	ಭಙ್ಗ;

      ಪಕ್ಷ FFI_TYPE_INT:
      ಪಕ್ಷ FFI_TYPE_SINT32:
      ಪಕ್ಷ FFI_TYPE_SINT16:
      ಪಕ್ಷ FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	ಭಙ್ಗ;

      ಸ್ವಭಾವ:
        FFI_ASSERT (0);
        ಭಙ್ಗ;
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

ffi_status
ffi_prep_closure_loc (ffi_closure *closure,
		      ffi_cif *cif,
		      ಶೂನ್ಯ (*fun) (ffi_cif *, ಶೂನ್ಯ *, ಶೂನ್ಯ **, ಶೂನ್ಯ *),
		      ಶೂನ್ಯ *user_data,
		      ಶೂನ್ಯ *codeloc)
{
  ಸ್ಥಿರ unsigned short ಧ್ರುವ template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#ಅನ್ಯಥಾ
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  ಶೂನ್ಯ (*dest)(ಶೂನ್ಯ);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;


#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
  ಯದಿ (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      ಗಚ್ಛ out;
    }
#endif

  memcpy (tramp, template, ಪರಿಮಾಣ(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     ಶೂನ್ಯ (*fun)(ffi_cif*,ಶೂನ್ಯ*,ಶೂನ್ಯ**,ಶೂನ್ಯ*))
{
  ಯದಿ (cif->abi != FFI_SYSV)
    ಪ್ರತ್ಯಾಗಮ FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  ಪ್ರತ್ಯಾಗಮ FFI_OK;
}

#ಯದಿ defined(FFI_EXEC_STATIC_TRAMP)
ಶೂನ್ಯ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  ಬಾಹ್ಯ ಶೂನ್ಯ *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  ಪ್ರತ್ಯಾಗಮ &trampoline_code_table;
}
#endif

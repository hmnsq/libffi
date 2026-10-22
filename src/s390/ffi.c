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

#அந்தர்பாவ <ffi.h>
#அந்தர்பாவ <ffi_common.h>
#அந்தர்பாவ <stdint.h>
#அந்தர்பாவ "internal.h"
#அந்தர்பாவ <tramp.h>

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
#இல்லையெனில்
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

கட்டமைப்பு call_frame
{
  வெறுமை *back_chain;
  வெறுமை *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

வெளி வெறுமை FFI_HIDDEN ffi_call_SYSV(கட்டமைப்பு call_frame *, unsigned, வெறுமை *,
			             வெறுமை (*fn)(வெறுமை), வெறுமை *);

வெளி வெறுமை ffi_closure_SYSV(வெறுமை);
வெளி வெறுமை ffi_go_closure_SYSV(வெறுமை);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

நிலையான முழு
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  போது (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  தேர்வு (size)
    {
      நிலை 1:
        திரும்பு FFI_TYPE_UINT8;

      நிலை 2:
        திரும்பு FFI_TYPE_UINT16;

      நிலை 4:
	எனில் (arg->type == FFI_TYPE_FLOAT)
          திரும்பு FFI_TYPE_FLOAT;
	இல்லையெனில்
	  திரும்பு FFI_TYPE_UINT32;

      நிலை 8:
	எனில் (arg->type == FFI_TYPE_DOUBLE)
          திரும்பு FFI_TYPE_DOUBLE;
	இல்லையெனில்
	  திரும்பு FFI_TYPE_UINT64;

      இயல்பு:
	நிறுத்து;
    }

  /* Other structs are passed via a pointer to the data.  */
  திரும்பு FFI_TYPE_POINTER;
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
  முழு n_gpr = 0;
  முழு n_fpr = 0;
  முழு n_ov = 0;

  ffi_type **ptr;
  முழு i;

  /* Determine return value handling.  */

  தேர்வு (cif->rtype->type)
    {
      /* Void is easy.  */
      நிலை FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	நிறுத்து;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      நிலை FFI_TYPE_STRUCT:
      நிலை FFI_TYPE_COMPLEX:
      நிலை FFI_TYPE_SINT128:
      நிலை FFI_TYPE_UINT128:
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      நிலை FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	நிறுத்து;

      /* Floating point values are returned in fpr 0.  */
      நிலை FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	நிறுத்து;

      நிலை FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	நிறுத்து;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      நிலை FFI_TYPE_UINT64:
      நிலை FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	நிறுத்து;

      நிலை FFI_TYPE_POINTER:
      நிலை FFI_TYPE_INT:
      நிலை FFI_TYPE_UINT32:
      நிலை FFI_TYPE_SINT32:
      நிலை FFI_TYPE_UINT16:
      நிலை FFI_TYPE_SINT16:
      நிலை FFI_TYPE_UINT8:
      நிலை FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#இல்லையெனில்
	cif->flags = FFI390_RET_INT32;
#endif
	நிறுத்து;

      இயல்பு:
        FFI_ASSERT (0);
        நிறுத்து;
    }

  /* Now for the arguments.  */

  க்கு (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      முழு type = (*ptr)->type;

      தேர்வு (type)
	{
	நிலை FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  எனில் (type != FFI_TYPE_POINTER)
	    நிறுத்து;
	  /* fall through */

	நிலை FFI_TYPE_COMPLEX:
	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  நிறுத்து;
	}

      /* Now handle all primitive int/float data types.  */
      தேர்வு (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    இல்லையெனில்
	      n_ov += அளவு (இரட்டை) / அளவு (long);
	    நிறுத்து;

	  நிலை FFI_TYPE_FLOAT:
	    எனில் (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    இல்லையெனில்
	      n_ov++;
	    நிறுத்து;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  நிலை FFI_TYPE_UINT64:
	  நிலை FFI_TYPE_SINT64:
	    எனில் (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    எனில் (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    இல்லையெனில்
	      n_ov += 2;
	    நிறுத்து;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  இயல்பு:
	    எனில் (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    இல்லையெனில்
	      n_ov++;
	    நிறுத்து;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * அளவு (long)) + struct_size;

  திரும்பு FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

நிலையான வெறுமை
ffi_call_int(ffi_cif *cif,
	     வெறுமை (*fn)(வெறுமை),
	     வெறுமை *rvalue,
	     வெறுமை **avalue,
	     வெறுமை *closure)
{
  முழு ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned எழுத்து *அடுக்கு, *p_struct;
  கட்டமைப்பு call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  முழு n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  எனில் (rvalue == வெற்று)
    {
      எனில் (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      இல்லையெனில்
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

  அடுக்கு = alloca (bytes + அளவு(கட்டமைப்பு call_frame) + rsize);
  frame = (கட்டமைப்பு call_frame *)(அடுக்கு + bytes);
  எனில் (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)அடுக்கு;
  p_struct = (unsigned எழுத்து *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  எனில் (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  க்கு (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      வெறுமை *arg = avalue[i];
      முழு type = ty->type;
      ffi_arg val;

    restart:
      தேர்வு (type)
	{
	நிலை FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_INT:
	நிலை FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  செல் do_int;
	நிலை FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  நிறுத்து;

	நிலை FFI_TYPE_UINT64:
	நிலை FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  செல் do_int;
#இல்லையெனில்
	  எனில் (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  எனில் (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  இல்லையெனில்
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  நிறுத்து;

	நிலை FFI_TYPE_DOUBLE:
	  எனில் (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  இல்லையெனில்
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#இல்லையெனில்
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  நிறுத்து;

	நிலை FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  எனில் (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  இல்லையெனில்
	    p_ov[n_ov++] = val;
	  நிறுத்து;

	நிலை FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  எனில் (type != FFI_TYPE_POINTER)
	    செல் restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	நிலை FFI_TYPE_LONGDOUBLE:
#endif
	நிலை FFI_TYPE_COMPLEX:
	நிலை FFI_TYPE_SINT128:
	நிலை FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  செல் do_int;

	இயல்பு:
	  FFI_ASSERT (0);
	  நிறுத்து;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

வெறுமை
ffi_call (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue, வெறுமை **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, வெற்று);
}

வெறுமை
ffi_call_go (ffi_cif *cif, வெறுமை (*fn)(வெறுமை), வெறுமை *rvalue,
	     வெறுமை **avalue, வெறுமை *closure)
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

வெறுமை FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*),
			 வெறுமை *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  வெறுமை *rvalue = &ret_buffer;
  வெறுமை **avalue;
  வெறுமை **p_arg;

  முழு n_gpr = 0;
  முழு n_fpr = 0;
  முழு n_ov = 0;

  ffi_type **ptr;
  முழு i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * அளவு (வெறுமை *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  எனில் (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (வெறுமை *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  க்கு (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      முழு deref_struct_pointer = 0;
      முழு type = (*ptr)->type;

#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      எனில் (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      எனில் (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  எனில் (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  இல்லையெனில்
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  எனில் (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      எனில் (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#இல்லையெனில்
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      தேர்வு (type)
	{
	  நிலை FFI_TYPE_DOUBLE:
	    எனில் (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    இல்லையெனில்
	      *p_arg = &p_ov[n_ov],
	      n_ov += அளவு (இரட்டை) / அளவு (long);
	    நிறுத்து;

	  நிலை FFI_TYPE_FLOAT:
	    எனில் (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    இல்லையெனில்
	      *p_arg = (எழுத்து *)&p_ov[n_ov++] + அளவு (long) - 4;
	    நிறுத்து;

	  நிலை FFI_TYPE_UINT64:
	  நிலை FFI_TYPE_SINT64:
#ifdef __s390x__
	    எனில் (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    இல்லையெனில்
	      *p_arg = &p_ov[n_ov++];
#இல்லையெனில்
	    எனில் (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    எனில் (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    இல்லையெனில்
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    நிறுத்து;

	  நிலை FFI_TYPE_INT:
	  நிலை FFI_TYPE_UINT32:
	  நிலை FFI_TYPE_SINT32:
	    எனில் (n_gpr < MAX_GPRARGS)
	      *p_arg = (எழுத்து *)&p_gpr[n_gpr++] + அளவு (long) - 4;
	    இல்லையெனில்
	      *p_arg = (எழுத்து *)&p_ov[n_ov++] + அளவு (long) - 4;
	    நிறுத்து;

	  நிலை FFI_TYPE_UINT16:
	  நிலை FFI_TYPE_SINT16:
	    எனில் (n_gpr < MAX_GPRARGS)
	      *p_arg = (எழுத்து *)&p_gpr[n_gpr++] + அளவு (long) - 2;
	    இல்லையெனில்
	      *p_arg = (எழுத்து *)&p_ov[n_ov++] + அளவு (long) - 2;
	    நிறுத்து;

	  நிலை FFI_TYPE_UINT8:
	  நிலை FFI_TYPE_SINT8:
	    எனில் (n_gpr < MAX_GPRARGS)
	      *p_arg = (எழுத்து *)&p_gpr[n_gpr++] + அளவு (long) - 1;
	    இல்லையெனில்
	      *p_arg = (எழுத்து *)&p_ov[n_ov++] + அளவு (long) - 1;
	    நிறுத்து;

	  இயல்பு:
	    FFI_ASSERT (0);
	    நிறுத்து;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      எனில் (deref_struct_pointer)
	*p_arg = *(வெறுமை **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  தேர்வு (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      நிலை FFI_TYPE_VOID:
      நிலை FFI_TYPE_STRUCT:
      நிலை FFI_TYPE_COMPLEX:
#எனில் FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      நிலை FFI_TYPE_LONGDOUBLE:
#endif
	நிறுத்து;

      /* Floating point values are returned in fpr 0.  */
      நிலை FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned முழு *) rvalue << 32;
	நிறுத்து;

      நிலை FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	நிறுத்து;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      நிலை FFI_TYPE_UINT64:
      நிலை FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#இல்லையெனில்
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	நிறுத்து;

      நிலை FFI_TYPE_POINTER:
      நிலை FFI_TYPE_UINT32:
      நிலை FFI_TYPE_UINT16:
      நிலை FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	நிறுத்து;

      நிலை FFI_TYPE_INT:
      நிலை FFI_TYPE_SINT32:
      நிலை FFI_TYPE_SINT16:
      நிலை FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	நிறுத்து;

      இயல்பு:
        FFI_ASSERT (0);
        நிறுத்து;
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
		      வெறுமை (*fun) (ffi_cif *, வெறுமை *, வெறுமை **, வெறுமை *),
		      வெறுமை *user_data,
		      வெறுமை *codeloc)
{
  நிலையான unsigned short மாறிலி template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#இல்லையெனில்
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  வெறுமை (*dest)(வெறுமை);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;


#எனில் defined(FFI_EXEC_STATIC_TRAMP)
  எனில் (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      செல் out;
    }
#endif

  memcpy (tramp, template, அளவு(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  திரும்பு FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     வெறுமை (*fun)(ffi_cif*,வெறுமை*,வெறுமை**,வெறுமை*))
{
  எனில் (cif->abi != FFI_SYSV)
    திரும்பு FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  திரும்பு FFI_OK;
}

#எனில் defined(FFI_EXEC_STATIC_TRAMP)
வெறுமை *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  வெளி வெறுமை *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  திரும்பு &trampoline_code_table;
}
#endif

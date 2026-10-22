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

#شامل <ffi.h>
#شامل <ffi_common.h>
#شامل <stdint.h>
#شامل "internal.h"
#شامل <tramp.h>

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
#وگرنه
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

ساختار call_frame
{
  تهی *back_chain;
  تهی *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

بیرونی تهی FFI_HIDDEN ffi_call_SYSV(ساختار call_frame *, unsigned, تهی *,
			             تهی (*fn)(تهی), تهی *);

بیرونی تهی ffi_closure_SYSV(تهی);
بیرونی تهی ffi_go_closure_SYSV(تهی);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ایستا صحیح
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  وقتی (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  انتخاب (size)
    {
      حالت 1:
        بازگشت FFI_TYPE_UINT8;

      حالت 2:
        بازگشت FFI_TYPE_UINT16;

      حالت 4:
	اگر (arg->type == FFI_TYPE_FLOAT)
          بازگشت FFI_TYPE_FLOAT;
	وگرنه
	  بازگشت FFI_TYPE_UINT32;

      حالت 8:
	اگر (arg->type == FFI_TYPE_DOUBLE)
          بازگشت FFI_TYPE_DOUBLE;
	وگرنه
	  بازگشت FFI_TYPE_UINT64;

      پیشفرض:
	بشکن;
    }

  /* Other structs are passed via a pointer to the data.  */
  بازگشت FFI_TYPE_POINTER;
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
  صحیح n_gpr = 0;
  صحیح n_fpr = 0;
  صحیح n_ov = 0;

  ffi_type **ptr;
  صحیح i;

  /* Determine return value handling.  */

  انتخاب (cif->rtype->type)
    {
      /* Void is easy.  */
      حالت FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	بشکن;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_COMPLEX:
      حالت FFI_TYPE_SINT128:
      حالت FFI_TYPE_UINT128:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالت FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	بشکن;

      /* Floating point values are returned in fpr 0.  */
      حالت FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	بشکن;

      حالت FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	بشکن;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	بشکن;

      حالت FFI_TYPE_POINTER:
      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_UINT8:
      حالت FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#وگرنه
	cif->flags = FFI390_RET_INT32;
#endif
	بشکن;

      پیشفرض:
        FFI_ASSERT (0);
        بشکن;
    }

  /* Now for the arguments.  */

  برای (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      صحیح type = (*ptr)->type;

      انتخاب (type)
	{
	حالت FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  اگر (type != FFI_TYPE_POINTER)
	    بشکن;
	  /* fall through */

	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  بشکن;
	}

      /* Now handle all primitive int/float data types.  */
      انتخاب (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  حالت FFI_TYPE_DOUBLE:
	    اگر (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وگرنه
	      n_ov += اندازه (دوگانه) / اندازه (long);
	    بشکن;

	  حالت FFI_TYPE_FLOAT:
	    اگر (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وگرنه
	      n_ov++;
	    بشکن;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  حالت FFI_TYPE_UINT64:
	  حالت FFI_TYPE_SINT64:
	    اگر (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    اگر (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    وگرنه
	      n_ov += 2;
	    بشکن;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  پیشفرض:
	    اگر (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    وگرنه
	      n_ov++;
	    بشکن;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * اندازه (long)) + struct_size;

  بازگشت FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ایستا تهی
ffi_call_int(ffi_cif *cif,
	     تهی (*fn)(تهی),
	     تهی *rvalue,
	     تهی **avalue,
	     تهی *closure)
{
  صحیح ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned نویسه *پشته, *p_struct;
  ساختار call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  صحیح n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  اگر (rvalue == پوچ)
    {
      اگر (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      وگرنه
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

  پشته = alloca (bytes + اندازه(ساختار call_frame) + rsize);
  frame = (ساختار call_frame *)(پشته + bytes);
  اگر (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)پشته;
  p_struct = (unsigned نویسه *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  اگر (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  برای (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      تهی *arg = avalue[i];
      صحیح type = ty->type;
      ffi_arg val;

    restart:
      انتخاب (type)
	{
	حالت FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  برو do_int;
	حالت FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  برو do_int;
	حالت FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  برو do_int;
	حالت FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  برو do_int;
	حالت FFI_TYPE_INT:
	حالت FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  برو do_int;
	حالت FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  برو do_int;
	حالت FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  بشکن;

	حالت FFI_TYPE_UINT64:
	حالت FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  برو do_int;
#وگرنه
	  اگر (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  اگر (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  وگرنه
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  بشکن;

	حالت FFI_TYPE_DOUBLE:
	  اگر (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  وگرنه
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#وگرنه
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  بشکن;

	حالت FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  اگر (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  وگرنه
	    p_ov[n_ov++] = val;
	  بشکن;

	حالت FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  اگر (type != FFI_TYPE_POINTER)
	    برو restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالت FFI_TYPE_LONGDOUBLE:
#endif
	حالت FFI_TYPE_COMPLEX:
	حالت FFI_TYPE_SINT128:
	حالت FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  برو do_int;

	پیشفرض:
	  FFI_ASSERT (0);
	  بشکن;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

تهی
ffi_call (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue, تهی **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, پوچ);
}

تهی
ffi_call_go (ffi_cif *cif, تهی (*fn)(تهی), تهی *rvalue,
	     تهی **avalue, تهی *closure)
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

تهی FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*),
			 تهی *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  تهی *rvalue = &ret_buffer;
  تهی **avalue;
  تهی **p_arg;

  صحیح n_gpr = 0;
  صحیح n_fpr = 0;
  صحیح n_ov = 0;

  ffi_type **ptr;
  صحیح i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * اندازه (تهی *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  اگر (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (تهی *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  برای (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      صحیح deref_struct_pointer = 0;
      صحیح type = (*ptr)->type;

#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      اگر (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      اگر (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  اگر (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  وگرنه
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  اگر (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      اگر (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#وگرنه
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      انتخاب (type)
	{
	  حالت FFI_TYPE_DOUBLE:
	    اگر (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وگرنه
	      *p_arg = &p_ov[n_ov],
	      n_ov += اندازه (دوگانه) / اندازه (long);
	    بشکن;

	  حالت FFI_TYPE_FLOAT:
	    اگر (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وگرنه
	      *p_arg = (نویسه *)&p_ov[n_ov++] + اندازه (long) - 4;
	    بشکن;

	  حالت FFI_TYPE_UINT64:
	  حالت FFI_TYPE_SINT64:
#ifdef __s390x__
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    وگرنه
	      *p_arg = &p_ov[n_ov++];
#وگرنه
	    اگر (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    وگرنه
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    بشکن;

	  حالت FFI_TYPE_INT:
	  حالت FFI_TYPE_UINT32:
	  حالت FFI_TYPE_SINT32:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (نویسه *)&p_gpr[n_gpr++] + اندازه (long) - 4;
	    وگرنه
	      *p_arg = (نویسه *)&p_ov[n_ov++] + اندازه (long) - 4;
	    بشکن;

	  حالت FFI_TYPE_UINT16:
	  حالت FFI_TYPE_SINT16:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (نویسه *)&p_gpr[n_gpr++] + اندازه (long) - 2;
	    وگرنه
	      *p_arg = (نویسه *)&p_ov[n_ov++] + اندازه (long) - 2;
	    بشکن;

	  حالت FFI_TYPE_UINT8:
	  حالت FFI_TYPE_SINT8:
	    اگر (n_gpr < MAX_GPRARGS)
	      *p_arg = (نویسه *)&p_gpr[n_gpr++] + اندازه (long) - 1;
	    وگرنه
	      *p_arg = (نویسه *)&p_ov[n_ov++] + اندازه (long) - 1;
	    بشکن;

	  پیشفرض:
	    FFI_ASSERT (0);
	    بشکن;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      اگر (deref_struct_pointer)
	*p_arg = *(تهی **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  انتخاب (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      حالت FFI_TYPE_VOID:
      حالت FFI_TYPE_STRUCT:
      حالت FFI_TYPE_COMPLEX:
#اگر FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالت FFI_TYPE_LONGDOUBLE:
#endif
	بشکن;

      /* Floating point values are returned in fpr 0.  */
      حالت FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned صحیح *) rvalue << 32;
	بشکن;

      حالت FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	بشکن;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالت FFI_TYPE_UINT64:
      حالت FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#وگرنه
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	بشکن;

      حالت FFI_TYPE_POINTER:
      حالت FFI_TYPE_UINT32:
      حالت FFI_TYPE_UINT16:
      حالت FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	بشکن;

      حالت FFI_TYPE_INT:
      حالت FFI_TYPE_SINT32:
      حالت FFI_TYPE_SINT16:
      حالت FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	بشکن;

      پیشفرض:
        FFI_ASSERT (0);
        بشکن;
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
		      تهی (*fun) (ffi_cif *, تهی *, تهی **, تهی *),
		      تهی *user_data,
		      تهی *codeloc)
{
  ایستا unsigned short ثابت template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#وگرنه
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  تهی (*dest)(تهی);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;


#اگر defined(FFI_EXEC_STATIC_TRAMP)
  اگر (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      برو out;
    }
#endif

  memcpy (tramp, template, اندازه(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#اگر defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  بازگشت FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     تهی (*fun)(ffi_cif*,تهی*,تهی**,تهی*))
{
  اگر (cif->abi != FFI_SYSV)
    بازگشت FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  بازگشت FFI_OK;
}

#اگر defined(FFI_EXEC_STATIC_TRAMP)
تهی *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  بیرونی تهی *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  بازگشت &trampoline_code_table;
}
#endif

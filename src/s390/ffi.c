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

#অন্তর্ভাব <ffi.h>
#অন্তর্ভাব <ffi_common.h>
#অন্তর্ভাব <stdint.h>
#অন্তর্ভাব "internal.h"
#অন্তর্ভাব <tramp.h>

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
#অন্যথা
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_আকার(আকার_2) (((আকার_2) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

কাঠামো call_frame
{
  শূন্য *back_chain;
  শূন্য *eos;
  unsigned long gpr_args[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_args[4];
};

বাহ্যিক শূন্য FFI_HIDDEN ffi_call_SYSV(কাঠামো call_frame *, unsigned, শূন্য *,
			             শূন্য (*fn)(শূন্য), শূন্য *);

বাহ্যিক শূন্য ffi_closure_SYSV(শূন্য);
বাহ্যিক শূন্য ffi_go_closure_SYSV(শূন্য);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

স্থির পূর্ণসংখ্যা
ffi_check_struct_type (ffi_type *arg)
{
  size_t আকার_2 = arg->আকার_2;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  যখন (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  নির্বাচন (আকার_2)
    {
      ক্ষেত্র 1:
        ফেরত FFI_TYPE_UINT8;

      ক্ষেত্র 2:
        ফেরত FFI_TYPE_UINT16;

      ক্ষেত্র 4:
	যদি (arg->type == FFI_TYPE_FLOAT)
          ফেরত FFI_TYPE_FLOAT;
	অন্যথা
	  ফেরত FFI_TYPE_UINT32;

      ক্ষেত্র 8:
	যদি (arg->type == FFI_TYPE_DOUBLE)
          ফেরত FFI_TYPE_DOUBLE;
	অন্যথা
	  ফেরত FFI_TYPE_UINT64;

      পূর্বনির্ধারিত:
	বিরতি;
    }

  /* Other structs are passed via a pointer to the data.  */
  ফেরত FFI_TYPE_POINTER;
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
  size_t struct_আকার = 0;
  পূর্ণসংখ্যা n_gpr = 0;
  পূর্ণসংখ্যা n_fpr = 0;
  পূর্ণসংখ্যা n_ov = 0;

  ffi_type **ptr;
  পূর্ণসংখ্যা i;

  /* Determine return value handling.  */

  নির্বাচন (cif->rtype->type)
    {
      /* Void is easy.  */
      ক্ষেত্র FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	বিরতি;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      ক্ষেত্র FFI_TYPE_STRUCT:
      ক্ষেত্র FFI_TYPE_COMPLEX:
      ক্ষেত্র FFI_TYPE_SINT128:
      ক্ষেত্র FFI_TYPE_UINT128:
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	বিরতি;

      /* Floating point values are returned in fpr 0.  */
      ক্ষেত্র FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	বিরতি;

      ক্ষেত্র FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	বিরতি;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ক্ষেত্র FFI_TYPE_UINT64:
      ক্ষেত্র FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	বিরতি;

      ক্ষেত্র FFI_TYPE_POINTER:
      ক্ষেত্র FFI_TYPE_INT:
      ক্ষেত্র FFI_TYPE_UINT32:
      ক্ষেত্র FFI_TYPE_SINT32:
      ক্ষেত্র FFI_TYPE_UINT16:
      ক্ষেত্র FFI_TYPE_SINT16:
      ক্ষেত্র FFI_TYPE_UINT8:
      ক্ষেত্র FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#অন্যথা
	cif->flags = FFI390_RET_INT32;
#endif
	বিরতি;

      পূর্বনির্ধারিত:
        FFI_ASSERT (0);
        বিরতি;
    }

  /* Now for the arguments.  */

  জন্য (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      পূর্ণসংখ্যা type = (*ptr)->type;

      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  যদি (type != FFI_TYPE_POINTER)
	    বিরতি;
	  /* fall through */

	ক্ষেত্র FFI_TYPE_COMPLEX:
	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_আকার += ROUND_আকার ((*ptr)->আকার_2);
	  বিরতি;
	}

      /* Now handle all primitive int/float data types.  */
      নির্বাচন (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  ক্ষেত্র FFI_TYPE_DOUBLE:
	    যদি (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    অন্যথা
	      n_ov += আকার (দ্বিগুণ) / আকার (long);
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_FLOAT:
	    যদি (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    অন্যথা
	      n_ov++;
	    বিরতি;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  ক্ষেত্র FFI_TYPE_UINT64:
	  ক্ষেত্র FFI_TYPE_SINT64:
	    যদি (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    যদি (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    অন্যথা
	      n_ov += 2;
	    বিরতি;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  পূর্বনির্ধারিত:
	    যদি (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    অন্যথা
	      n_ov++;
	    বিরতি;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_আকার (n_ov * আকার (long)) + struct_আকার;

  ফেরত FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

স্থির শূন্য
ffi_call_int(ffi_cif *cif,
	     শূন্য (*fn)(শূন্য),
	     শূন্য *rvalue,
	     শূন্য **avalue,
	     শূন্য *closure)
{
  পূর্ণসংখ্যা ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned অক্ষর *স্তূপ, *p_struct;
  কাঠামো call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  পূর্ণসংখ্যা n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  যদি (rvalue == ফাঁকা)
    {
      যদি (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->আকার_2;
      অন্যথা
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

  স্তূপ = alloca (bytes + আকার(কাঠামো call_frame) + rsize);
  frame = (কাঠামো call_frame *)(স্তূপ + bytes);
  যদি (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)স্তূপ;
  p_struct = (unsigned অক্ষর *)frame;
  p_gpr = frame->gpr_args;
  p_fpr = frame->fpr_args;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  যদি (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  জন্য (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      শূন্য *arg = avalue[i];
      পূর্ণসংখ্যা type = ty->type;
      ffi_arg val;

    restart:
      নির্বাচন (type)
	{
	ক্ষেত্র FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_INT:
	ক্ষেত্র FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  যাও do_int;
	ক্ষেত্র FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_UINT64:
	ক্ষেত্র FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  যাও do_int;
#অন্যথা
	  যদি (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  যদি (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  অন্যথা
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  বিরতি;

	ক্ষেত্র FFI_TYPE_DOUBLE:
	  যদি (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  অন্যথা
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#অন্যথা
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  বিরতি;

	ক্ষেত্র FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  যদি (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  অন্যথা
	    p_ov[n_ov++] = val;
	  বিরতি;

	ক্ষেত্র FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  যদি (type != FFI_TYPE_POINTER)
	    যাও restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#endif
	ক্ষেত্র FFI_TYPE_COMPLEX:
	ক্ষেত্র FFI_TYPE_SINT128:
	ক্ষেত্র FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_আকার (ty->আকার_2);
	  memcpy (p_struct, arg, ty->আকার_2);
	  val = (uintptr_t)p_struct;
	  যাও do_int;

	পূর্বনির্ধারিত:
	  FFI_ASSERT (0);
	  বিরতি;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

শূন্য
ffi_call (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue, শূন্য **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, ফাঁকা);
}

শূন্য
ffi_call_go (ffi_cif *cif, শূন্য (*fn)(শূন্য), শূন্য *rvalue,
	     শূন্য **avalue, শূন্য *closure)
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

শূন্য FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*),
			 শূন্য *user_তথ্য,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_বাফার;

  শূন্য *rvalue = &ret_বাফার;
  শূন্য **avalue;
  শূন্য **p_arg;

  পূর্ণসংখ্যা n_gpr = 0;
  পূর্ণসংখ্যা n_fpr = 0;
  পূর্ণসংখ্যা n_ov = 0;

  ffi_type **ptr;
  পূর্ণসংখ্যা i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * আকার (শূন্য *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  যদি (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (শূন্য *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  জন্য (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      পূর্ণসংখ্যা deref_struct_pointer = 0;
      পূর্ণসংখ্যা type = (*ptr)->type;

#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      যদি (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      যদি (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  যদি (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  অন্যথা
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  যদি (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      যদি (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#অন্যথা
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      নির্বাচন (type)
	{
	  ক্ষেত্র FFI_TYPE_DOUBLE:
	    যদি (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    অন্যথা
	      *p_arg = &p_ov[n_ov],
	      n_ov += আকার (দ্বিগুণ) / আকার (long);
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_FLOAT:
	    যদি (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    অন্যথা
	      *p_arg = (অক্ষর *)&p_ov[n_ov++] + আকার (long) - 4;
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_UINT64:
	  ক্ষেত্র FFI_TYPE_SINT64:
#ifdef __s390x__
	    যদি (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    অন্যথা
	      *p_arg = &p_ov[n_ov++];
#অন্যথা
	    যদি (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    যদি (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    অন্যথা
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_INT:
	  ক্ষেত্র FFI_TYPE_UINT32:
	  ক্ষেত্র FFI_TYPE_SINT32:
	    যদি (n_gpr < MAX_GPRARGS)
	      *p_arg = (অক্ষর *)&p_gpr[n_gpr++] + আকার (long) - 4;
	    অন্যথা
	      *p_arg = (অক্ষর *)&p_ov[n_ov++] + আকার (long) - 4;
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_UINT16:
	  ক্ষেত্র FFI_TYPE_SINT16:
	    যদি (n_gpr < MAX_GPRARGS)
	      *p_arg = (অক্ষর *)&p_gpr[n_gpr++] + আকার (long) - 2;
	    অন্যথা
	      *p_arg = (অক্ষর *)&p_ov[n_ov++] + আকার (long) - 2;
	    বিরতি;

	  ক্ষেত্র FFI_TYPE_UINT8:
	  ক্ষেত্র FFI_TYPE_SINT8:
	    যদি (n_gpr < MAX_GPRARGS)
	      *p_arg = (অক্ষর *)&p_gpr[n_gpr++] + আকার (long) - 1;
	    অন্যথা
	      *p_arg = (অক্ষর *)&p_ov[n_ov++] + আকার (long) - 1;
	    বিরতি;

	  পূর্বনির্ধারিত:
	    FFI_ASSERT (0);
	    বিরতি;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      যদি (deref_struct_pointer)
	*p_arg = *(শূন্য **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_তথ্য);

  /* Convert the return value.  */
  নির্বাচন (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      ক্ষেত্র FFI_TYPE_VOID:
      ক্ষেত্র FFI_TYPE_STRUCT:
      ক্ষেত্র FFI_TYPE_COMPLEX:
#যদি FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      ক্ষেত্র FFI_TYPE_LONGDOUBLE:
#endif
	বিরতি;

      /* Floating point values are returned in fpr 0.  */
      ক্ষেত্র FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned পূর্ণসংখ্যা *) rvalue << 32;
	বিরতি;

      ক্ষেত্র FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	বিরতি;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      ক্ষেত্র FFI_TYPE_UINT64:
      ক্ষেত্র FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#অন্যথা
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	বিরতি;

      ক্ষেত্র FFI_TYPE_POINTER:
      ক্ষেত্র FFI_TYPE_UINT32:
      ক্ষেত্র FFI_TYPE_UINT16:
      ক্ষেত্র FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	বিরতি;

      ক্ষেত্র FFI_TYPE_INT:
      ক্ষেত্র FFI_TYPE_SINT32:
      ক্ষেত্র FFI_TYPE_SINT16:
      ক্ষেত্র FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	বিরতি;

      পূর্বনির্ধারিত:
        FFI_ASSERT (0);
        বিরতি;
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
		      শূন্য (*fun) (ffi_cif *, শূন্য *, শূন্য **, শূন্য *),
		      শূন্য *user_তথ্য,
		      শূন্য *codeloc)
{
  স্থির unsigned short ধ্রুবক template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#অন্যথা
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  শূন্য (*dest)(শূন্য);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  যদি (cif->abi != FFI_SYSV)
    ফেরত FFI_BAD_ABI;


#যদি defined(FFI_EXEC_STATIC_TRAMP)
  যদি (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      যাও out;
    }
#endif

  memcpy (tramp, template, আকার(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#যদি defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_তথ্য = user_তথ্য;

  ফেরত FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     শূন্য (*fun)(ffi_cif*,শূন্য*,শূন্য**,শূন্য*))
{
  যদি (cif->abi != FFI_SYSV)
    ফেরত FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  ফেরত FFI_OK;
}

#যদি defined(FFI_EXEC_STATIC_TRAMP)
শূন্য *
ffi_tramp_arch (size_t *tramp_আকার, size_t *map_আকার)
{
  বাহ্যিক শূন্য *trampoline_code_table;

  *tramp_আকার = FFI390_TRAMP_আকার;
  *map_আকার = FFI390_TRAMP_MAP_আকার;
  ফেরত &trampoline_code_table;
}
#endif

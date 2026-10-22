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

#ضمن <ffi.h>
#ضمن <ffi_common.h>
#ضمن <stdint.h>
#ضمن "internal.h"
#ضمن <tramp.h>

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
#وإلا
#define MAX_FPRARGS 2
#endif

/* Round to multiple of 16.  */
#define ROUND_SIZE(size) (((size) + 15) & ~15)

/*===================== End of Defines ===============================*/

/*====================================================================*/
/*                          Externals                                 */
/*                          ---------                                 */
/*====================================================================*/

بنية call_frame
{
  فراغ *back_chain;
  فراغ *eos;
  unsigned long gpr_وسائط[5];
  unsigned long gpr_save[9];
  unsigned long long fpr_وسائط[4];
};

خارجي فراغ FFI_HIDDEN ffi_call_SYSV(بنية call_frame *, unsigned, فراغ *,
			             فراغ (*fn)(فراغ), فراغ *);

خارجي فراغ ffi_closure_SYSV(فراغ);
خارجي فراغ ffi_go_closure_SYSV(فراغ);

/*====================== End of Externals ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_check_struct_type.                                  */
/*                                                                    */
/* Function - Determine if a structure can be passed within a         */
/*            general purpose or floating point register.             */
/*                                                                    */
/*====================================================================*/

ساكن صحيح
ffi_check_struct_type (ffi_type *arg)
{
  size_t size = arg->size;

  /* If the struct has just one element, look at that element
     to find out whether to consider the struct as floating point.  */
  طالما (arg->type == FFI_TYPE_STRUCT
         && arg->elements[0] && !arg->elements[1])
    arg = arg->elements[0];

  /* Structs of size 1, 2, 4, and 8 are passed in registers,
     just like the corresponding int/float types.  */
  بدل (size)
    {
      حالة 1:
        أرجع FFI_TYPE_UINT8;

      حالة 2:
        أرجع FFI_TYPE_UINT16;

      حالة 4:
	إذا (arg->type == FFI_TYPE_FLOAT)
          أرجع FFI_TYPE_FLOAT;
	وإلا
	  أرجع FFI_TYPE_UINT32;

      حالة 8:
	إذا (arg->type == FFI_TYPE_DOUBLE)
          أرجع FFI_TYPE_DOUBLE;
	وإلا
	  أرجع FFI_TYPE_UINT64;

      افتراضي:
	اكسر;
    }

  /* Other structs are passed via a pointer to the data.  */
  أرجع FFI_TYPE_POINTER;
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
  صحيح n_gpr = 0;
  صحيح n_fpr = 0;
  صحيح n_ov = 0;

  ffi_type **ptr;
  صحيح i;

  /* Determine return value handling.  */

  بدل (cif->rtype->type)
    {
      /* Void is easy.  */
      حالة FFI_TYPE_VOID:
	cif->flags = FFI390_RET_VOID;
	اكسر;

      /* Structures, complex, int128, and long double are
         returned via a hidden pointer.  */
      حالة FFI_TYPE_STRUCT:
      حالة FFI_TYPE_COMPLEX:
      حالة FFI_TYPE_SINT128:
      حالة FFI_TYPE_UINT128:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالة FFI_TYPE_LONGDOUBLE:
#endif
	cif->flags = FFI390_RET_STRUCT;
	n_gpr++;  /* We need one GPR to pass the pointer.  */
	اكسر;

      /* Floating point values are returned in fpr 0.  */
      حالة FFI_TYPE_FLOAT:
	cif->flags = FFI390_RET_FLOAT;
	اكسر;

      حالة FFI_TYPE_DOUBLE:
	cif->flags = FFI390_RET_DOUBLE;
	اكسر;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
	cif->flags = FFI390_RET_INT64;
	اكسر;

      حالة FFI_TYPE_POINTER:
      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_UINT8:
      حالة FFI_TYPE_SINT8:
	/* These are to be extended to word size.  */
#ifdef __s390x__
	cif->flags = FFI390_RET_INT64;
#وإلا
	cif->flags = FFI390_RET_INT32;
#endif
	اكسر;

      افتراضي:
        FFI_ASSERT (0);
        اكسر;
    }

  /* Now for the arguments.  */

  لكل (ptr = cif->arg_types, i = cif->nargs;
       i > 0;
       i--, ptr++)
    {
      صحيح type = (*ptr)->type;

      بدل (type)
	{
	حالة FFI_TYPE_STRUCT:
	  type = ffi_check_struct_type (*ptr);
	  إذا (type != FFI_TYPE_POINTER)
	    اكسر;
	  /* fall through */

	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
#endif
	  type = FFI_TYPE_POINTER;
	  /* If we pass via pointer, we must reserve space to copy
	     its data for proper call-by-value semantics.  */
	  struct_size += ROUND_SIZE ((*ptr)->size);
	  اكسر;
	}

      /* Now handle all primitive int/float data types.  */
      بدل (type)
	{
	  /* The first MAX_FPRARGS floating point arguments
	     go in FPRs, the rest overflow to the stack.  */

	  حالة FFI_TYPE_DOUBLE:
	    إذا (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وإلا
	      n_ov += حجم (مزدوج) / حجم (long);
	    اكسر;

	  حالة FFI_TYPE_FLOAT:
	    إذا (n_fpr < MAX_FPRARGS)
	      n_fpr++;
	    وإلا
	      n_ov++;
	    اكسر;

	  /* On 31-bit machines, 64-bit integers are passed in GPR pairs,
	     if one is still available, or else on the stack.  If only one
	     register is free, skip the register (it won't be used for any
	     subsequent argument either).  */

#ifndef __s390x__
	  حالة FFI_TYPE_UINT64:
	  حالة FFI_TYPE_SINT64:
	    إذا (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    إذا (n_gpr < MAX_GPRARGS)
	      n_gpr += 2;
	    وإلا
	      n_ov += 2;
	    اكسر;
#endif

	  /* Everything else is passed in GPRs (until MAX_GPRARGS
	     have been used) or overflows to the stack.  */

	  افتراضي:
	    إذا (n_gpr < MAX_GPRARGS)
	      n_gpr++;
	    وإلا
	      n_ov++;
	    اكسر;
        }
    }

  /* Total stack space as required for overflow arguments
     and temporary structure copies.  */

  cif->bytes = ROUND_SIZE (n_ov * حجم (long)) + struct_size;

  أرجع FFI_OK;
}

/*======================== End of Routine ============================*/

/*====================================================================*/
/*                                                                    */
/* Name     - ffi_call.                                               */
/*                                                                    */
/* Function - Call the FFI routine.                                   */
/*                                                                    */
/*====================================================================*/

ساكن فراغ
ffi_call_int(ffi_cif *cif,
	     فراغ (*fn)(فراغ),
	     فراغ *rvalue,
	     فراغ **avalue,
	     فراغ *closure)
{
  صحيح ret_type = cif->flags;
  size_t rsize = 0, bytes = cif->bytes;
  unsigned حرف *مكدس, *p_struct;
  بنية call_frame *frame;
  unsigned long *p_ov, *p_gpr;
  unsigned long long *p_fpr;
  صحيح n_fpr, n_gpr, n_ov, i, n;
  ffi_type **arg_types;

  FFI_ASSERT (cif->abi == FFI_SYSV);

  /* If we don't have a return value, we need to fake one.  */
  إذا (rvalue == عدم)
    {
      إذا (ret_type & FFI390_RET_IN_MEM)
	rsize = cif->rtype->size;
      وإلا
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

  مكدس = alloca (bytes + حجم(بنية call_frame) + rsize);
  frame = (بنية call_frame *)(مكدس + bytes);
  إذا (rsize)
    rvalue = frame + 1;

  /* Link the new frame back to the one from this function.  */
  frame->back_chain = __builtin_frame_address (0);

  /* Fill in all of the argument stuff.  */
  p_ov = (unsigned long *)مكدس;
  p_struct = (unsigned حرف *)frame;
  p_gpr = frame->gpr_وسائط;
  p_fpr = frame->fpr_وسائط;
  n_fpr = n_gpr = n_ov = 0;

  /* If we returning a structure then we set the first parameter register
     to the address of where we are returning this structure.  */
  إذا (cif->flags & FFI390_RET_IN_MEM)
    p_gpr[n_gpr++] = (uintptr_t) rvalue;

  /* Now for the arguments.  */
  arg_types = cif->arg_types;
  لكل (i = 0, n = cif->nargs; i < n; ++i)
    {
      ffi_type *ty = arg_types[i];
      فراغ *arg = avalue[i];
      صحيح type = ty->type;
      ffi_arg val;

    restart:
      بدل (type)
	{
	حالة FFI_TYPE_SINT8:
	  val = *(SINT8 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT8:
	  val = *(UINT8 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_SINT16:
	  val = *(SINT16 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT16:
	  val = *(UINT16 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_INT:
	حالة FFI_TYPE_SINT32:
	  val = *(SINT32 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_UINT32:
	  val = *(UINT32 *)arg;
	  اذهب do_int;
	حالة FFI_TYPE_POINTER:
	  val = *(uintptr_t *)arg;
	do_int:
	  *(n_gpr < MAX_GPRARGS ? p_gpr + n_gpr++ : p_ov + n_ov++) = val;
	  اكسر;

	حالة FFI_TYPE_UINT64:
	حالة FFI_TYPE_SINT64:
#ifdef __s390x__
	  val = *(UINT64 *)arg;
	  اذهب do_int;
#وإلا
	  إذا (n_gpr == MAX_GPRARGS-1)
	    n_gpr = MAX_GPRARGS;
	  إذا (n_gpr < MAX_GPRARGS)
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[0],
	    p_gpr[n_gpr++] = ((UINT32 *) arg)[1];
	  وإلا
	    p_ov[n_ov++] = ((UINT32 *) arg)[0],
	    p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	  اكسر;

	حالة FFI_TYPE_DOUBLE:
	  إذا (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = *(UINT64 *) arg;
	  وإلا
	    {
#ifdef __s390x__
	      p_ov[n_ov++] = *(UINT64 *) arg;
#وإلا
	      p_ov[n_ov++] = ((UINT32 *) arg)[0],
	      p_ov[n_ov++] = ((UINT32 *) arg)[1];
#endif
	    }
	  اكسر;

	حالة FFI_TYPE_FLOAT:
	  val = *(UINT32 *)arg;
	  إذا (n_fpr < MAX_FPRARGS)
	    p_fpr[n_fpr++] = (UINT64)val << 32;
	  وإلا
	    p_ov[n_ov++] = val;
	  اكسر;

	حالة FFI_TYPE_STRUCT:
          /* Check how a structure type is passed.  */
	  type = ffi_check_struct_type (ty);
	  /* Some structures are passed via a type they contain.  */
	  إذا (type != FFI_TYPE_POINTER)
	    اذهب restart;
	  /* ... otherwise, passed by reference.  fallthru.  */

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
	حالة FFI_TYPE_LONGDOUBLE:
#endif
	حالة FFI_TYPE_COMPLEX:
	حالة FFI_TYPE_SINT128:
	حالة FFI_TYPE_UINT128:
	  /* Complex, int128 and long double types are passed via reference. */
	  p_struct -= ROUND_SIZE (ty->size);
	  memcpy (p_struct, arg, ty->size);
	  val = (uintptr_t)p_struct;
	  اذهب do_int;

	افتراضي:
	  FFI_ASSERT (0);
	  اكسر;
        }
    }

  ffi_call_SYSV (frame, ret_type & FFI360_RET_MASK, rvalue, fn, closure);
}

فراغ
ffi_call (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue, فراغ **avalue)
{
  ffi_call_int(cif, fn, rvalue, avalue, عدم);
}

فراغ
ffi_call_go (ffi_cif *cif, فراغ (*fn)(فراغ), فراغ *rvalue,
	     فراغ **avalue, فراغ *closure)
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

فراغ FFI_HIDDEN
ffi_closure_helper_SYSV (ffi_cif *cif,
			 فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*),
			 فراغ *user_data,
			 unsigned long *p_gpr,
			 unsigned long long *p_fpr,
			 unsigned long *p_ov)
{
  unsigned long long ret_buffer;

  فراغ *rvalue = &ret_buffer;
  فراغ **avalue;
  فراغ **p_arg;

  صحيح n_gpr = 0;
  صحيح n_fpr = 0;
  صحيح n_ov = 0;

  ffi_type **ptr;
  صحيح i;

  /* Allocate buffer for argument list pointers.  */
  p_arg = avalue = alloca (cif->nargs * حجم (فراغ *));

  /* If we returning a structure, pass the structure address
     directly to the target function.  Otherwise, have the target
     function store the return value to the GPR save area.  */
  إذا (cif->flags & FFI390_RET_IN_MEM)
    rvalue = (فراغ *) p_gpr[n_gpr++];

  /* Now for the arguments.  */
  لكل (ptr = cif->arg_types, i = cif->nargs; i > 0; i--, p_arg++, ptr++)
    {
      صحيح deref_struct_pointer = 0;
      صحيح type = (*ptr)->type;

#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      /* 16-byte long double is passed like a struct.  */
      إذا (type == FFI_TYPE_LONGDOUBLE)
	type = FFI_TYPE_STRUCT;
#endif

      /* Check how a structure type is passed.  */
      إذا (type == FFI_TYPE_STRUCT || type == FFI_TYPE_COMPLEX)
	{
	  إذا (type == FFI_TYPE_COMPLEX)
	    type = FFI_TYPE_POINTER;
	  وإلا
	    type = ffi_check_struct_type (*ptr);

	  /* If we pass the struct via pointer, remember to
	     retrieve the pointer later.  */
	  إذا (type == FFI_TYPE_POINTER)
	    deref_struct_pointer = 1;
	}

      /* Pointers are passed like UINTs of the same size.  */
      إذا (type == FFI_TYPE_POINTER)
	{
#ifdef __s390x__
	  type = FFI_TYPE_UINT64;
#وإلا
	  type = FFI_TYPE_UINT32;
#endif
	}

      /* Now handle all primitive int/float data types.  */
      بدل (type)
	{
	  حالة FFI_TYPE_DOUBLE:
	    إذا (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وإلا
	      *p_arg = &p_ov[n_ov],
	      n_ov += حجم (مزدوج) / حجم (long);
	    اكسر;

	  حالة FFI_TYPE_FLOAT:
	    إذا (n_fpr < MAX_FPRARGS)
	      *p_arg = &p_fpr[n_fpr++];
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (long) - 4;
	    اكسر;

	  حالة FFI_TYPE_UINT64:
	  حالة FFI_TYPE_SINT64:
#ifdef __s390x__
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr++];
	    وإلا
	      *p_arg = &p_ov[n_ov++];
#وإلا
	    إذا (n_gpr == MAX_GPRARGS-1)
	      n_gpr = MAX_GPRARGS;
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = &p_gpr[n_gpr], n_gpr += 2;
	    وإلا
	      *p_arg = &p_ov[n_ov], n_ov += 2;
#endif
	    اكسر;

	  حالة FFI_TYPE_INT:
	  حالة FFI_TYPE_UINT32:
	  حالة FFI_TYPE_SINT32:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (long) - 4;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (long) - 4;
	    اكسر;

	  حالة FFI_TYPE_UINT16:
	  حالة FFI_TYPE_SINT16:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (long) - 2;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (long) - 2;
	    اكسر;

	  حالة FFI_TYPE_UINT8:
	  حالة FFI_TYPE_SINT8:
	    إذا (n_gpr < MAX_GPRARGS)
	      *p_arg = (حرف *)&p_gpr[n_gpr++] + حجم (long) - 1;
	    وإلا
	      *p_arg = (حرف *)&p_ov[n_ov++] + حجم (long) - 1;
	    اكسر;

	  افتراضي:
	    FFI_ASSERT (0);
	    اكسر;
        }

      /* If this is a struct passed via pointer, we need to
	 actually retrieve that pointer.  */
      إذا (deref_struct_pointer)
	*p_arg = *(فراغ **)*p_arg;
    }


  /* Call the target function.  */
  (fun) (cif, rvalue, avalue, user_data);

  /* Convert the return value.  */
  بدل (cif->rtype->type)
    {
      /* Void is easy, and so is struct.  */
      حالة FFI_TYPE_VOID:
      حالة FFI_TYPE_STRUCT:
      حالة FFI_TYPE_COMPLEX:
#إذا FFI_TYPE_LONGDOUBLE != FFI_TYPE_DOUBLE
      حالة FFI_TYPE_LONGDOUBLE:
#endif
	اكسر;

      /* Floating point values are returned in fpr 0.  */
      حالة FFI_TYPE_FLOAT:
	p_fpr[0] = (long long) *(unsigned صحيح *) rvalue << 32;
	اكسر;

      حالة FFI_TYPE_DOUBLE:
	p_fpr[0] = *(unsigned long long *) rvalue;
	اكسر;

      /* Integer values are returned in gpr 2 (and gpr 3
	 for 64-bit values on 31-bit machines).  */
      حالة FFI_TYPE_UINT64:
      حالة FFI_TYPE_SINT64:
#ifdef __s390x__
	p_gpr[0] = *(unsigned long *) rvalue;
#وإلا
	p_gpr[0] = ((unsigned long *) rvalue)[0],
	p_gpr[1] = ((unsigned long *) rvalue)[1];
#endif
	اكسر;

      حالة FFI_TYPE_POINTER:
      حالة FFI_TYPE_UINT32:
      حالة FFI_TYPE_UINT16:
      حالة FFI_TYPE_UINT8:
	p_gpr[0] = *(unsigned long *) rvalue;
	اكسر;

      حالة FFI_TYPE_INT:
      حالة FFI_TYPE_SINT32:
      حالة FFI_TYPE_SINT16:
      حالة FFI_TYPE_SINT8:
	p_gpr[0] = *(signed long *) rvalue;
	اكسر;

      افتراضي:
        FFI_ASSERT (0);
        اكسر;
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
		      فراغ (*fun) (ffi_cif *, فراغ *, فراغ **, فراغ *),
		      فراغ *user_data,
		      فراغ *codeloc)
{
  ساكن unsigned short ثابت template[] = {
    0x0d10,			/* basr %r1,0 */
#ifndef __s390x__
    0x9801, 0x1006,		/* lm %r0,%r1,6(%r1) */
#وإلا
    0xeb01, 0x100e, 0x0004,	/* lmg %r0,%r1,14(%r1) */
#endif
    0x07f1			/* br %r1 */
  };
  فراغ (*dest)(فراغ);
  unsigned long *tramp = (unsigned long *)&closure->tramp;

  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;


#إذا defined(FFI_EXEC_STATIC_TRAMP)
  إذا (ffi_tramp_is_present(closure))
    {
      /* Initialize the static trampoline's parameters. */
      dest = ffi_closure_SYSV;
      ffi_tramp_set_parms (closure->ftramp, dest, closure);
      اذهب out;
    }
#endif

  memcpy (tramp, template, حجم(template));
  tramp[2] = (unsigned long)codeloc;
  tramp[3] = (unsigned long)&ffi_closure_SYSV;

#إذا defined(FFI_EXEC_STATIC_TRAMP)
out:
#endif
  closure->cif = cif;
  closure->fun = fun;
  closure->user_data = user_data;

  أرجع FFI_OK;
}

/*======================== End of Routine ============================*/

/* Build a Go language closure.  */

ffi_status
ffi_prep_go_closure (ffi_go_closure *closure, ffi_cif *cif,
		     فراغ (*fun)(ffi_cif*,فراغ*,فراغ**,فراغ*))
{
  إذا (cif->abi != FFI_SYSV)
    أرجع FFI_BAD_ABI;

  closure->tramp = ffi_go_closure_SYSV;
  closure->cif = cif;
  closure->fun = fun;

  أرجع FFI_OK;
}

#إذا defined(FFI_EXEC_STATIC_TRAMP)
فراغ *
ffi_tramp_arch (size_t *tramp_size, size_t *map_size)
{
  خارجي فراغ *trampoline_code_table;

  *tramp_size = FFI390_TRAMP_SIZE;
  *map_size = FFI390_TRAMP_MAP_SIZE;
  أرجع &trampoline_code_table;
}
#endif
